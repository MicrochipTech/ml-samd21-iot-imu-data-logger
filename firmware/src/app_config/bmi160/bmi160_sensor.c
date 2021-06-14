/*******************************************************************************
  Sensor Driver Interface Source File

  Company:
    Microchip Technology Inc.

  File Name:
    bmi160_sensor.c

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

#include <stdint.h>
#include <string.h>
#include "sensor.h"
#include "bmi160.h"
#include "definitions.h"

// Macro function to get the proper Macro defines corresponding to SNSR_SAMPLE_RATE
#if (SNSR_SAMPLE_RATE_UNIT == SNSR_SAMPLE_RATE_UNIT_KHZ)
    #error "BMI160 driver doesn't support sample rate units in kHZ; use SNSR_SAMPLE_RATE_UNIT_HZ instead"
#endif
#define __SNSRSAMPLERATEMACRO(x, y) BMI160_ ## x ## _ODR_ ## y ## HZ
#define _SNSRSAMPLERATEEXPR(x, y) __SNSRSAMPLERATEMACRO(x, y)
#define _GET_IMU_SAMPLE_RATE_MACRO(x) _SNSRSAMPLERATEEXPR(x, SNSR_SAMPLE_RATE)

#define __SNSRACCELRANGEMACRO(x) BMI160_ACCEL_RANGE_ ## x ## G
#define _SNSRACCELRANGEEXPR(x) __SNSRACCELRANGEMACRO(x)
#define _GET_IMU_ACCEL_RANGE_MACRO() _SNSRACCELRANGEEXPR(SNSR_ACCEL_RANGE)

#define __SNSRGYRORANGEMACRO(x) BMI160_GYRO_RANGE_ ## x ## _DPS
#define _SNSRGYRORANGEEXPR(x) __SNSRGYRORANGEMACRO(x)
#define _GET_IMU_GYRO_RANGE_MACRO() _SNSRGYRORANGEEXPR(SNSR_GYRO_RANGE)

int8_t bmi160_i2c_read (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    if(SERCOM1_I2C_WriteRead((uint16_t) dev_addr, &reg_addr, 1, data, (uint32_t) len)) {
        while(SERCOM1_I2C_IsBusy());
        return BMI160_OK;
    }
    else {
        return BMI160_E_COM_FAIL;        
    }
}

int8_t bmi160_i2c_write (uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    static uint8_t buff [SNSR_COM_BUF_SIZE];
    
    if ((len + 1) > SNSR_COM_BUF_SIZE)
        return BMI160_E_COM_FAIL;
    
    buff[0] = reg_addr;
    memcpy(&buff[1], data, len);
    
    if (SERCOM1_I2C_Write((uint16_t) dev_addr, buff, (uint32_t) len + 1)) {
        while(SERCOM1_I2C_IsBusy());
        return BMI160_OK;
    } else {
        return BMI160_E_COM_FAIL;
    }
}

int bmi160_sensor_read(struct sensor_device_t *sensor, struct sensor_buffer_t *buffer)
{
    /* Read bmi160 sensor data */
    struct bmi160_sensor_data accel;
    struct bmi160_sensor_data gyro;
    int status;
    
    status = bmi160_get_sensor_data(BMI160_ACCEL_SEL | BMI160_GYRO_SEL, &accel, &gyro, &sensor->device);
    if (status != BMI160_OK)
        return status;
    
    /* Convert sensor data to buffer type and write to buffer */
    buffer_frame_t frame;
    buffer_data_t *ptr = frame;
#if SNSR_USE_ACCEL_X
    *ptr++ = (buffer_data_t) accel.x;
#endif
#if SNSR_USE_ACCEL_Y
    *ptr++ = (buffer_data_t) accel.y;
#endif
#if SNSR_USE_ACCEL_Z
    *ptr++ = (buffer_data_t) accel.z;
#endif
#if SNSR_USE_GYRO_X
    *ptr++ = (buffer_data_t) gyro.x;
#endif
#if SNSR_USE_GYRO_Y
    *ptr++ = (buffer_data_t) gyro.y;
#endif
#if SNSR_USE_GYRO_Z
    *ptr++ = (buffer_data_t) gyro.z;
#endif
    buffer_write(buffer, &frame, 1);
    
    return status;
}

int bmi160_sensor_init(struct sensor_device_t *sensor) {
    sensor->status = BMI160_OK;
    
    /* Initialize BMI160 */
    sensor->device.id = BMI160_I2C_ADDR;
    sensor->device.interface = BMI160_I2C_INTF;
    sensor->device.read = bmi160_i2c_read;
    sensor->device.write = bmi160_i2c_write;
    sensor->device.delay_ms = snsr_sleep_ms;
    
    sensor->status = bmi160_init(&sensor->device);
    
    return sensor->status;
}

int bmi160_sensor_set_config(struct sensor_device_t *sensor) {
    int8_t errcode = BMI160_OK;
    
    /* Select the Output data rate, range of accelerometer sensor */
    sensor->device.accel_cfg.odr = _GET_IMU_SAMPLE_RATE_MACRO(ACCEL); //BMI160_ACCEL_ODR_100HZ;
    sensor->device.accel_cfg.range = _GET_IMU_ACCEL_RANGE_MACRO();//BMI160_ACCEL_RANGE_2G;
    sensor->device.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
    
    /* Select the power mode of accelerometer sensor */
    sensor->device.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;
    
    /* Select the Output data rate, range of Gyroscope sensor */
    sensor->device.gyro_cfg.odr = _GET_IMU_SAMPLE_RATE_MACRO(GYRO); //BMI160_GYRO_ODR_100HZ;
    sensor->device.gyro_cfg.range = _GET_IMU_GYRO_RANGE_MACRO(); //BMI160_GYRO_RANGE_2000_DPS;
    sensor->device.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

    /* Select the power mode of Gyroscope sensor */
    sensor->device.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;     

    /* Set the sensor configuration */
    errcode = bmi160_set_sens_conf(&sensor->device);
    
    /* Configure sensor interrupt */
    struct bmi160_int_settg int_config;
    
    /* Select the Interrupt channel/pin */
    int_config.int_channel = BMI160_INT_CHANNEL_1;// Interrupt channel/pin 1

    /* Select the Interrupt type */
    int_config.int_type = BMI160_ACC_GYRO_DATA_RDY_INT;// Choosing data ready interrupt
    
    /* Select the interrupt channel/pin settings */
    int_config.int_pin_settg.output_en = BMI160_ENABLE;// Enabling interrupt pins to act as output pin
    int_config.int_pin_settg.output_mode = BMI160_DISABLE;// Choosing push-pull mode for interrupt pin
    int_config.int_pin_settg.output_type = BMI160_DISABLE;// Choosing active low output
    int_config.int_pin_settg.edge_ctrl = BMI160_ENABLE;// Choosing edge triggered output
    int_config.int_pin_settg.input_en = BMI160_DISABLE;// Disabling interrupt pin to act as input
    int_config.int_pin_settg.latch_dur = BMI160_LATCH_DUR_NONE;// non-latched output
            
    /* Set the data ready interrupt */
    errcode |= bmi160_set_int_config(&int_config, &sensor->device);
    
    return errcode;
}