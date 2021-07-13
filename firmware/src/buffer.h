/*******************************************************************************
  Sensor Buffering Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    buffer.h

  Summary:
    This file contains the ring buffer API used for buffering sensor data

  Description:
    This file implements a buffering interface for a 2d array of a single data 
    type with statically allocated memory. The buffer has the behavior that on 
    overrun condition, new incoming data will be ignored until the buffer_reset 
    function is called.
 
  Notes:
    - The API provided here is strictly designed for a single reader thread and 
      single writer thread; other uses will cause race conditions.
    - It's further assumed that the reader will *never* interrupt the writer to 
      call buffer_reset - this will cause a race condition
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

#ifndef BUFFER_H
#define	BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "sensor_config.h"

#ifndef SNSR_NUM_AXES
#error "SNSR_NUM_AXES must be defined"
#endif

#ifndef SNSR_BUF_LEN
#error "SNSR_BUF_LEN must be defined"
#endif

#if (((SNSR_BUF_LEN-1) & SNSR_BUF_LEN) != 0) 
#error "SNSR_BUF_LEN must be a power of 2"
#endif

#ifndef SNSR_DATA_TYPE
#error "SNSR_DATA_TYPE must be defined"
#endif

// Define the memory barrier directive to use
// This directive ensures that all data memory operations complete *before* 
// the updating of the read or write index
#if defined(__arm__)
    // Full compiler/memory barrier
#   define __buffer_sync()        asm volatile ("dsb" ::: "memory")
#elif defined(__GNUC__)
    // This directive only ensures the *compiler* doesn't reorder data memory operations before 
    // the updating of the read or write index
    // - this is enough on platforms that don't do out of order execution
#   define __buffer_sync()        asm volatile ("" ::: "memory")
#else
#   define __buffer_sync()        do {} while (0)
#   warning "buffer.h:: No memory/compiler barrier defined; correct concurrent operation not guaranteed"
#endif

#ifdef	__cplusplus
extern "C" {
#endif
    
typedef SNSR_DATA_TYPE buffer_data_t;

typedef buffer_data_t buffer_frame_t[SNSR_NUM_AXES];

// !!!NB!!!
// buffer_size_t MUST be set to a type whose size is <= the data bus width
// this makes loads and stores of the read/write index atomic
// Otherwise, we'd have to get interrupt masking involved..
#if defined(__AVR__)
// For AVR assume 8-bit
#   if (SNSR_BUF_LEN > (1 << 8))
#       error "currently, max buffer length is 256 for 8-bit AVR"
#   endif
typedef uint8_t buffer_size_t;
#else
// For anything else assume 32-bit
#   if ! defined (__arm__)    
#       warning "buffer.h:: Unsure about architecture, assuming 32-bit accesses are atomic"    
#   endif
#   if (SNSR_BUF_LEN > (1 << 32))
#       error "Max buffer size is 2^32"
#   endif
typedef uint32_t buffer_size_t;
#endif

struct sensor_buffer_t {
    buffer_data_t data[SNSR_BUF_LEN][SNSR_NUM_AXES];
    volatile buffer_size_t writeIdx;
    volatile buffer_size_t readIdx;
    buffer_size_t _mask;
    volatile bool overrun;
    volatile bool underrun;
};

void buffer_init(struct sensor_buffer_t *buffer);

void buffer_reset(struct sensor_buffer_t *buffer);

buffer_size_t buffer_read(struct sensor_buffer_t *buffer, buffer_data_t dst[][SNSR_NUM_AXES], buffer_size_t framecount);

buffer_size_t buffer_write(struct sensor_buffer_t *buffer, buffer_data_t data[][SNSR_NUM_AXES], buffer_size_t framecount);

buffer_size_t buffer_get_read_frames(struct sensor_buffer_t *buffer);

buffer_size_t buffer_get_write_frames(struct sensor_buffer_t *buffer);

buffer_size_t buffer_get_read_buffer(struct sensor_buffer_t *buffer, buffer_data_t **ptr);

buffer_size_t buffer_get_write_buffer(struct sensor_buffer_t *buffer, buffer_data_t **ptr);

bool buffer_advance_write_index(struct sensor_buffer_t *buffer, buffer_size_t framecount);

bool buffer_advance_read_index(struct sensor_buffer_t *buffer, buffer_size_t framecount);

#ifdef	__cplusplus
}
#endif

#endif	/* BUFFER_H */

