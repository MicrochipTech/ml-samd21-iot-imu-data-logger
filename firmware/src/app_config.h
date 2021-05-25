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

// *****************************************************************************
// *****************************************************************************
// Section: User configurable application level parameters
// *****************************************************************************
// *****************************************************************************

// Set to true to have firmware dump data to uart in ascii format
#define DATA_LOGGER_BUILD       true

// Set to true to have firmware dump data to uart in form suitable for MPLAB DV plugin
#define DATA_VISUALIZER_BUILD   false

#define MPDV_START_OF_FRAME     0xA5U   // Frame header byte for MPLAB DV

// IMU sampling rate in Hz
// For BMI160:
//  Use one of 25, 50, 100, 200, 400, 800, or 1600
// For ICM42688:
//  Use one of 25, 50, 100, 200, 500
// !NB! Increasing the sample rate above 200Hz with all 6 axes may cause buffer overruns
// Change at your own risk!
#define SNSR_SAMPLE_RATE        100

// Accelerometer range in Gs
#define SNSR_ACCEL_RANGE        16

// Gyro range in DPS
#define SNSR_GYRO_RANGE         2000

#define SNSR_BUF_LEN            256

// stream sensor data type natively
#define SNSR_DATA_TYPE          int16_t

// Define which axes from the IMU to use
#define SNSR_USE_ACCEL_X        true
#define SNSR_USE_ACCEL_Y        true
#define SNSR_USE_ACCEL_Z        true
#define SNSR_USE_GYRO_X         true
#define SNSR_USE_GYRO_Y         true
#define SNSR_USE_GYRO_Z         true

// LED tick rate parameters
#define TICK_RATE_FAST          100
#define TICK_RATE_SLOW          500

// Switch debounce time (default value based on 48MHz clock rate)
#define SW_DEBOUNCE_INTERVAL   730000L

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

#if (DATA_LOGGER_BUILD && DATA_VISUALIZER_BUILD)
    #error "Only one of DATA_LOGGER_BUILD or DATA_VISUALIZER_BUILD may be set"
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
#define MIKRO_EIC_PIN   EIC_PIN_12

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* APP_CONFIG_H */

