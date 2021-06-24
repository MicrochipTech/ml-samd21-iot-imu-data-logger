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

// *****************************************************************************
// *****************************************************************************
// Section: Enumeration of available data streaming formats
// *****************************************************************************
// *****************************************************************************
#define DATA_STREAMER_FORMAT_UNKNOWN    0

// Dump data to uart in ascii format
#define DATA_STREAMER_FORMAT_ASCII      1

// Dump data to uart in form suitable for MPLAB DV plugin
#define DATA_STREAMER_FORMAT_MDV        2

// Dump data to uart in form suitable for SensiMLs Data Capture Lab (simple stream format)
#define DATA_STREAMER_FORMAT_SMLSS      3

// Disable all data streaming
#define DATA_STREAMER_FORMAT_NONE       4

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
//  - set SNSR_SAMPLE_RATE_UNIT to SNSR_SAMPLE_RATE_UNIT_HZ
//  - set SNSR_SAMPLE_RATE to one of: 25, 50, 100, 200, 400, 800, or 1600
// For ICM42688 < 1kHz range:
//  - set SNSR_SAMPLE_RATE_UNIT to SNSR_SAMPLE_RATE_UNIT_HZ
//  - set SNSR_SAMPLE_RATE to one of: 25, 50, 100, 200, or 500
// For ICM42688 >= 1kHz range:
//  - set SNSR_SAMPLE_RATE_UNIT to SNSR_SAMPLE_RATE_UNIT_KHZ
//  - set SNSR_SAMPLE_RATE to one of: 1, 2, 4, 8, or 16
// !NB! Increasing the sample rate above 200Hz with all 6 axes when using one
// of the data logging builds may cause buffer overruns - Change at your own risk!
#define SNSR_SAMPLE_RATE        100
#define SNSR_SAMPLE_RATE_UNIT   SNSR_SAMPLE_RATE_UNIT_HZ // HZ or KHZ

// Accelerometer range in Gs
#define SNSR_ACCEL_RANGE        16

// Gyro range in DPS
#define SNSR_GYRO_RANGE         2000

// Define which axes from the IMU to use
#define SNSR_USE_ACCEL_X        true
#define SNSR_USE_ACCEL_Y        true
#define SNSR_USE_ACCEL_Z        true
#define SNSR_USE_GYRO_X         true
#define SNSR_USE_GYRO_Y         true
#define SNSR_USE_GYRO_Z         true

// Size of sensor buffer in samples
#define SNSR_BUF_LEN            64

// Type used to store and stream sensor samples
#define SNSR_DATA_TYPE          int16_t

// Frame header byte for MPLAB DV
#define MDV_START_OF_FRAME     0xA5U

// SensiML specific parameters
#if (DATA_STREAMER_FORMAT == DATA_STREAMER_FORMAT_SMLSS)
#define SNSR_SAMPLES_PER_PACKET 6
#endif

// LED tick rate parameters
#define TICK_RATE_FAST          100
#define TICK_RATE_SLOW          500

// *****************************************************************************
// *****************************************************************************
// Section: Defines derived from user config parameters
// *****************************************************************************
// *****************************************************************************
#define SNSR_NUM_AXES   (SNSR_USE_ACCEL_X + SNSR_USE_ACCEL_Y + SNSR_USE_ACCEL_Z \
                            + SNSR_USE_GYRO_X + SNSR_USE_GYRO_Y + SNSR_USE_GYRO_Z)

/* Define whether multiple sensors types are being used */
#define SNSR_USE_GYRO   (SNSR_USE_GYRO_X || SNSR_USE_GYRO_Y || SNSR_USE_GYRO_Z)
#define SNSR_USE_ACCEL  (SNSR_USE_ACCEL_X || SNSR_USE_ACCEL_Y || SNSR_USE_ACCEL_Z)
#if (SNSR_USE_ACCEL && SNSR_USE_GYRO)
    #define MULTI_SENSOR 1
#else
    #define MULTI_SENSOR 0
#endif

#if !defined(DATA_STREAMER_FORMAT) || (DATA_STREAMER_FORMAT == DATA_STREAMER_FORMAT_UNKNOWN)
    #error "No DATA_STREAMER_FORMAT type has been set"
#endif

// Provide the functions needed by sensor module
#define snsr_read_timer_us read_timer_us
#define snsr_read_timer_ms read_timer_ms
#define snsr_sleep_ms      sleep_ms
#define snsr_sleep_us      sleep_us

// Some convenience macros
#define LED_BLUE_On     LED_BLUE_Clear
#define LED_BLUE_Off    LED_BLUE_Set
#define LED_GREEN_On    LED_GREEN_Clear
#define LED_GREEN_Off   LED_GREEN_Set
#define LED_RED_On      LED_RED_Clear
#define LED_RED_Off     LED_RED_Set
#define LED_YELLOW_On   LED_YELLOW_Clear
#define LED_YELLOW_Off  LED_YELLOW_Set
#define LED_ALL_On()    do { LED_YELLOW_On(); LED_GREEN_On(); LED_RED_On(); LED_BLUE_On(); } while (0)
#define LED_ALL_Off()   do { LED_YELLOW_Off(); LED_GREEN_Off(); LED_RED_Off(); LED_BLUE_Off(); } while (0)

#define STREAM_FORMAT_IS(X) (defined(DATA_STREAMER_FORMAT_ ## X) && (DATA_STREAMER_FORMAT_ ## X == DATA_STREAMER_FORMAT))

#define SNSR_SAMPLE_RATE_UNIT_STR ((SNSR_SAMPLE_RATE_UNIT == SNSR_SAMPLE_RATE_UNIT_KHZ) ? "kHz" : "Hz")
#ifdef SNSR_TYPE_BMI160
#define SNSR_NAME "bmi160"
#elif SNSR_TYPE_ICM42688
#define SNSR_NAME "icm42688"
#endif

// Macros for portability
#define TC_TimerCallbackRegister(cb) TC3_TimerCallbackRegister(cb, (uintptr_t) NULL)
#define MIKRO_INT_CallbackRegister(cb) EIC_CallbackRegister(EIC_PIN_12, cb, (uintptr_t) NULL)
#define TC_TimerStart TC3_TimerStart
#define TC_TimerGet TC3_Timer16bitCounterGet
#define TC_TimerCallbackRegister(cb) TC3_TimerCallbackRegister(cb, (uintptr_t) NULL)

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* APP_CONFIG_H */
