/* Microchip Technology Inc. and its subsidiaries.  You may use this software
 * and any derivatives exclusively with Microchip products.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
 * TERMS.
 */

/*
 * File:
 * Author:
 * Comments:
 * Revision history:
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef APP_CONFIG_H
#define	APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// *****************************************************************************
// *****************************************************************************
// Section: Enumeration of available data streaming formats
// *****************************************************************************
// *****************************************************************************
// Disable all data streaming
#define DATA_STREAMER_FORMAT_NONE       0

// Dump data to uart in ascii format
#define DATA_STREAMER_FORMAT_ASCII      1

// Dump data to uart in form suitable for MPLAB DV plugin
#define DATA_STREAMER_FORMAT_MDV        2

// Dump data to uart in form suitable for SensiMLs Data Capture Lab (simple stream format)
#define DATA_STREAMER_FORMAT_SMLSS      3

// *****************************************************************************
// *****************************************************************************
// Section: User configurable application level parameters
// *****************************************************************************
// *****************************************************************************

// Data streaming formatting selection
#ifndef DATA_STREAMER_FORMAT
#define DATA_STREAMER_FORMAT    DATA_STREAMER_FORMAT_ASCII
#endif

// IMU sampling rate in units of SNSR_SAMPLE_RATE_UNIT
// For BMI160:
//  - set SNSR_SAMPLE_RATE to one of: 25, 50, 100, 200, 400, 800, 1600
// For ICM42688:
//  - set SNSR_SAMPLE_RATE to one of: 25, 50, 100, 200, 500, 1000, 2000, 4000, 8000, 16000
// !NB! Increasing the sample rate above 500Hz (this may be lower for non MDV formats)
// with all 6 axes may cause buffer overruns
//  - Change at your own risk!
#define SNSR_SAMPLE_RATE        100

// Accelerometer range in Gs
// Either sensor supports one of: 2, 4, 8, 16
#define SNSR_ACCEL_RANGE        16

// Gyro range in DPS
// For ICM42688 use one of: 16, 31, 62, 125, 250, 500, 1000, 2000
// For BMI160 use one of: 125, 250, 500, 1000, 2000
#define SNSR_GYRO_RANGE         2000

// Define which axes from the IMU to use
#define SNSR_USE_ACCEL          true
#define SNSR_USE_GYRO           true

// Size of sensor buffer in samples (must be power of 2)
#define SNSR_BUF_LEN            128

// Type used to store and stream sensor samples
#define SNSR_DATA_TYPE          int16_t

// Frame header byte for MPLAB DV
#define MDV_START_OF_FRAME      0xA5U

// SensiML specific parameters
#if (DATA_STREAMER_FORMAT == DATA_STREAMER_FORMAT_SMLSS)
#define SML_MAX_CONFIG_STRLEN   256
#define SNSR_SAMPLES_PER_PACKET 8  // must be factor of SNSR_BUF_LEN
#define SSI_JSON_CONFIG_VERSION 2  // 2 => Use enhance SSI protocol,
                                   // 1 => use original SSI protocol
#else
#define SNSR_SAMPLES_PER_PACKET 1
#endif

// LED tick rate periods in ms
#define TICK_RATE_FAST          100
#define TICK_RATE_SLOW          500

// *****************************************************************************
// *****************************************************************************
// Section: Defines derived from user config parameters
// *****************************************************************************
// *****************************************************************************
#define SNSR_NUM_AXES   (3*SNSR_USE_ACCEL + 3*SNSR_USE_GYRO)

/* Define whether multiple sensors types are being used */
#if (SNSR_USE_ACCEL && SNSR_USE_GYRO)
    #define MULTI_SENSOR 1
#else
    #define MULTI_SENSOR 0
#endif

// The way the buffering works the following condition must be enforced
#if (SNSR_BUF_LEN % SNSR_SAMPLES_PER_PACKET) > 0
#error "SNSR_SAMPLES_PER_PACKET must be a factor of SNSR_BUF_LEN"
#endif

// Provide the functions needed by sensor module
#define snsr_read_timer_us read_timer_us
#define snsr_read_timer_ms read_timer_ms
#define snsr_sleep_ms      sleep_ms
#define snsr_sleep_us      sleep_us

#define STREAM_FORMAT_IS(X) (defined(DATA_STREAMER_FORMAT_ ## X) && (DATA_STREAMER_FORMAT_ ## X == DATA_STREAMER_FORMAT))

#ifdef SNSR_TYPE_BMI160
#define SNSR_NAME "bmi160"
#elif SNSR_TYPE_ICM42688
#define SNSR_NAME "icm42688"
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Platform generic macros for portability
// *****************************************************************************
// *****************************************************************************
#define __nullop__()        do {} while (0)
#define LED_BLUE_On         LED_BLUE_Clear
#define LED_BLUE_Off        LED_BLUE_Set
//#define LED_BLUE_Toggle     __nullop__
#define LED_GREEN_On        LED_GREEN_Clear
#define LED_GREEN_Off       LED_GREEN_Set
//#define LED_GREEN_Toggle    __nullop__
#define LED_RED_On          LED_RED_Clear
#define LED_RED_Off         LED_RED_Set
//#define LED_RED_Toggle      __nullop__
#define LED_YELLOW_On       LED_YELLOW_Clear
#define LED_YELLOW_Off      LED_YELLOW_Set
//#define LED_YELLOW_Toggle   __nullop__
#define LED_ALL_On()        do { LED_YELLOW_On(); LED_GREEN_On(); LED_RED_On(); LED_BLUE_On(); } while (0)
#define LED_ALL_Off()       do { LED_YELLOW_Off(); LED_GREEN_Off(); LED_RED_Off(); LED_BLUE_Off(); } while (0)
#define LED_STATUS_On       LED_YELLOW_On
#define LED_STATUS_Off      LED_YELLOW_Off
#define LED_STATUS_Toggle   LED_YELLOW_Toggle

// Map CS pin for SPI
//#define MIKRO_CS_Clear      MIKRO_CS_Clear
//#define MIKRO_CS_Set        MIKRO_CS_Set

// UART stubs
#define UART_RX_DATA        SERCOM5_REGS->USART_INT.SERCOM_DATA
#define UART_IsRxReady      SERCOM5_USART_ReceiverIsReady
#define UART_RXC_Enable()   { SERCOM5_REGS->USART_INT.SERCOM_INTENSET |= (uint8_t)(SERCOM_USART_INT_INTENSET_RXC_Msk); }

// Device init / management
//#define SYS_Initialize   SYS_Initialize
//#define SYS_Tasks        SYS_Tasks

// Sensor external interrupt
#define MIKRO_INT_CallbackRegister(cb) EIC_CallbackRegister(EIC_PIN_12, cb, (uintptr_t) NULL)

// uS Timer
#define TC_TimerStart                   TC3_TimerStart
#define TC_TimerGet_us                  TC3_Timer16bitCounterGet
#define TC_TimerCallbackRegister(cb)    TC3_TimerCallbackRegister(cb, (uintptr_t) NULL)
size_t __attribute__(( unused )) UART_Read(uint8_t *ptr, const size_t nbytes);
size_t __attribute__(( unused )) UART_Write(uint8_t *ptr, const size_t nbytes);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

typedef SNSR_DATA_TYPE snsr_data_t;
typedef SNSR_DATA_TYPE snsr_dataframe_t[SNSR_NUM_AXES];
typedef SNSR_DATA_TYPE snsr_datapacket_t[SNSR_NUM_AXES*SNSR_SAMPLES_PER_PACKET];

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* APP_CONFIG_H */
