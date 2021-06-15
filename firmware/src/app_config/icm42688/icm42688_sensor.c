/*******************************************************************************
  Sensor Driver Interface Source File

  Company:
    Microchip Technology Inc.

  File Name:
    sensor.c

  Summary:
    This file defines a simplified interface API for configuring the IMU sensor

  Description:
    None
 *******************************************************************************/

/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "definitions.h"
#include "sensor.h"
#include "Icm426xxDriver_HL.h"

#if SNSR_SAMPLE_RATE_UNIT == SNSR_SAMPLE_RATE_UNIT_HZ
#define __SNSRSAMPLERATEMACRO(x, y) ICM426XX_ ## x ## _CONFIG0_ODR_ ## y ## _ ## HZ
#else
#define __SNSRSAMPLERATEMACRO(x, y) ICM426XX_ ## x ## _CONFIG0_ODR_ ## y ## _ ## KHZ
#endif
#define _SNSRSAMPLERATEEXPR(x, y) __SNSRSAMPLERATEMACRO(x, y)
#define _GET_IMU_SAMPLE_RATE_MACRO(x) _SNSRSAMPLERATEEXPR(x, SNSR_SAMPLE_RATE)

#define __SNSRACCELRANGEMACRO(x) ICM426XX_ACCEL_CONFIG0_FS_SEL_ ## x ## g
#define _SNSRACCELRANGEEXPR(x) __SNSRACCELRANGEMACRO(x)
#define _GET_IMU_ACCEL_RANGE_MACRO() _SNSRACCELRANGEEXPR(SNSR_ACCEL_RANGE)

#define __SNSRGYRORANGEMACRO(x) ICM426XX_GYRO_CONFIG0_FS_SEL_ ## x ## dps
#define _SNSRGYRORANGEEXPR(x) __SNSRGYRORANGEMACRO(x)
#define _GET_IMU_GYRO_RANGE_MACRO() _SNSRGYRORANGEEXPR(SNSR_GYRO_RANGE)

static struct sensor_buffer_t * l_snsr_buffer = NULL;

uint64_t inv_icm426xx_get_time_us(void) {
    return snsr_read_timer_us();
}

void inv_icm426xx_sleep_us(uint32_t us) {
    snsr_sleep_us(us);
}

// Handle callback from inv_icm426xx_get_data_from_registers
void icm42688_sensor_event_cb(inv_icm426xx_sensor_event_t * event) {
    if (l_snsr_buffer == NULL) {
        return;
    }
    
    /* Convert sensor data to buffer type and write to buffer */
    buffer_data_t data[1][SNSR_NUM_AXES];
    buffer_data_t *ptr = &data[0][0];
#if SNSR_USE_ACCEL_X
    *ptr++ = (buffer_data_t) event->accel[0];
#endif
#if SNSR_USE_ACCEL_Y
    *ptr++ = (buffer_data_t) event->accel[1];
#endif
#if SNSR_USE_ACCEL_Z
    *ptr++ = (buffer_data_t) event->accel[2];
#endif
#if SNSR_USE_GYRO_X
    *ptr++ = (buffer_data_t) event->gyro[0];
#endif
#if SNSR_USE_GYRO_Y
    *ptr++ = (buffer_data_t) event->gyro[1];
#endif
#if SNSR_USE_GYRO_Z
    *ptr++ = (buffer_data_t) event->gyro[2];
#endif
    buffer_write(l_snsr_buffer, data, 1);
}

int icm42688_spi_read (struct inv_icm426xx_serif * serif, uint8_t reg, uint8_t * rbuffer, uint32_t rlen) {
    int rval = 0;
    
    reg = 0x80 | (reg & 0x7F); // Set Read/Write bit in MSB (1 for Read)
    
    MIKRO_CS_Clear();
    
    if(!SERCOM0_SPI_Write(&reg, 1)) {
        rval = -1;
    }
    else if(!SERCOM0_SPI_Read(rbuffer, rlen)) {
        rval = -1;
    }
    
    MIKRO_CS_Set();
    
    return rval;
}

int icm42688_spi_write (struct inv_icm426xx_serif * serif, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen) {
    int rval = 0;
    uint8_t *ptr = (uint8_t *) wbuffer; // ignore const (we promise we won't change it)
    uint8_t data[2];
    
    for (int i=0; i<wlen; i++) 
    {
        data[0] = reg++ & 0x7F;
        // data[0] |= (0 << 7); // Set Read/Write bit in MSB (0 for Write)
        data[1] = *ptr++;
        MIKRO_CS_Clear();
        if(!SERCOM0_SPI_Write(data, 2)) {
            rval = -1;
            break;
        }
        MIKRO_CS_Set();
    }    
    
    MIKRO_CS_Set();
    return rval;
}

int icm42688_sensor_init(struct sensor_device_t *sensor) {    
    /* Init ICM */
    memset(&sensor->serif, 0, sizeof(sensor->serif));
    sensor->serif.context   = 0;        /* no need */
    sensor->serif.read_reg  = icm42688_spi_read;
    sensor->serif.write_reg = icm42688_spi_write;
    sensor->serif.max_read  = SNSR_COM_BUF_SIZE-1;
    sensor->serif.max_write = SNSR_COM_BUF_SIZE-1;
    sensor->serif.serif_type = ICM426XX_UI_SPI4;
    
    sensor->status = SNSR_STATUS_OK;

    // Init and disable FIFO
    sensor->status = inv_icm426xx_init(&sensor->device, &sensor->serif, icm42688_sensor_event_cb);
    sensor->status |= inv_icm426xx_configure_fifo(&sensor->device, INV_ICM426XX_FIFO_DISABLED);

    uint8_t who_am_i;
    sensor->status |= inv_icm426xx_get_who_am_i(&sensor->device, &who_am_i);
    if (who_am_i != ICM_WHOAMI)
        sensor->status |= INV_ERROR;
    
    return sensor->status;
}

int icm42688_sensor_set_config(struct sensor_device_t *sensor) {
    /* Configure ICM */
    // No sync clock - disable CLKIN
    sensor->status |= inv_icm426xx_enable_clkin_rtc(&sensor->device, false);

    // Set sampling parameters
    sensor->status |= inv_icm426xx_set_accel_fsr(&sensor->device, _GET_IMU_ACCEL_RANGE_MACRO()); //ICM426XX_ACCEL_CONFIG0_FS_SEL_2g);
    sensor->status |= inv_icm426xx_set_gyro_fsr(&sensor->device, _GET_IMU_GYRO_RANGE_MACRO()); //ICM426XX_GYRO_CONFIG0_FS_SEL_2000dps);
    sensor->status |= inv_icm426xx_set_accel_frequency(&sensor->device, _GET_IMU_SAMPLE_RATE_MACRO(ACCEL)); //ICM426XX_ACCEL_CONFIG0_ODR_100_HZ);
    sensor->status |= inv_icm426xx_set_gyro_frequency(&sensor->device, _GET_IMU_SAMPLE_RATE_MACRO(GYRO)); //ICM426XX_GYRO_CONFIG0_ODR_100_HZ);

    // Low Noise Mode
    sensor->status |= inv_icm426xx_enable_accel_low_noise_mode(&sensor->device);
    sensor->status |= inv_icm426xx_enable_gyro_low_noise_mode(&sensor->device);
    
    // Note DRDY interrupt is set up by default in inv_init function

    return sensor->status;
}

int icm42688_sensor_read(struct sensor_device_t *sensor, struct sensor_buffer_t *buffer) {    
    l_snsr_buffer = buffer; // Set module scoped buffer pointer
    sensor->status = inv_icm426xx_get_data_from_registers(&sensor->device);
    l_snsr_buffer = NULL;
    
    return sensor->status;
}