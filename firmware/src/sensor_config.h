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
#ifndef SENSOR_CONFIG_H
#define	SENSOR_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// *****************************************************************************
// *****************************************************************************
// Section: User configurable sensor parameters
// *****************************************************************************
// *****************************************************************************

/* IMU Sensor type defined at a project level */
#if !defined(SNSR_TYPE_BMI160) && !defined(SNSR_TYPE_ICM42688)
#define SNSR_TYPE_BMI160    1
#define SNSR_TYPE_ICM42688  0
#endif

#define SNSR_SAMPLE_RATE_UNIT_HZ    0
#define SNSR_SAMPLE_RATE_UNIT_KHZ   1

// high level sensor configuration defined in app_config header
#include "app_config.h"

// *****************************************************************************
// *****************************************************************************
// Section: Internal defines for sensor library
// *****************************************************************************
// *****************************************************************************

#define SNSR_COM_BUF_SIZE   1024

// Provide sensor specific functions
#if SNSR_TYPE_BMI160
    #define sensor_init        bmi160_sensor_init
    #define sensor_set_config  bmi160_sensor_set_config
    #define sensor_read        bmi160_sensor_read
#elif SNSR_TYPE_ICM42688
    #define sensor_init        icm42688_sensor_init
    #define sensor_set_config  icm42688_sensor_set_config
    #define sensor_read        icm42688_sensor_read
#endif

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* SENSOR_CONFIG_H */

