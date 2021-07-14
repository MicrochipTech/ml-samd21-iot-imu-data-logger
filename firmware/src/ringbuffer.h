/*******************************************************************************
Buffering Interface Header File

Company:
Microchip Technology Inc.

File Name:
ringbuffer.h

Summary:
This file contains the ring buffer API used for generic buffering

Notes:
    - The API provided here is strictly designed for a single reader thread and
      single writer thread; other uses will cause race conditions.
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

#ifndef RINGBUFFER_H
#define	RINGBUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Define the compiler/memory fence directive to use
// This directive ensures that all data memory operations complete before
// the updating of the read or write index
#if defined(__GNUC__)
#   if defined(__arm__)
    // Full compiler/memory barrier
#   define __ringbuffer_sync()        __asm__ volatile ("dsb" ::: "memory")
#   else
    // This directive only ensures the *compiler* doesn't reorder data memory operations before
    // the updating of the read or write index
    // - this is enough on platforms that don't do out of order execution
#   define __ringbuffer_sync()        __asm__ volatile ("" ::: "memory")
#   endif //if defined(__arm__)
#else
#   define __ringbuffer_sync()        do {} while (0)
#   warning "ringbuffer.h:: No memory barrier defined; thread safety not guaranteed"
#endif

#ifdef	__cplusplus
extern "C" {
#endif

// !!!NB!!!
// ringbuffer_size_t MUST be set to a type whose size is <= the data bus width
// this makes loads and stores of the read/write index atomic
// Otherwise, we'd have to get interrupt masking involved..
// AVR, PIC10/12/14/16/18
#if defined(__AVR__) || defined(__XC8)
    typedef uint8_t ringbuffer_size_t;
// PIC24, dsPIC
#elif defined(__dsPIC30__) || defined(__XC16)
    typedef uint16_t ringbuffer_size_t;
// SAM, PIC32C, PIC32M
#elif defined (__arm__) || defined(__XC32)
    typedef uint32_t ringbuffer_size_t;
#else
#   warning "ringbuffer.h:: Unsure about architecture, assuming 32-bit accesses are atomic"
    typedef uint32_t ringbuffer_size_t;
#endif

typedef struct ring_buffer {
    volatile ringbuffer_size_t writeIdx;
    volatile ringbuffer_size_t readIdx;
    ringbuffer_size_t len;
    ringbuffer_size_t _mask;
    uint8_t *data;
} ringbuffer_t;

int8_t ringbuffer_init(ringbuffer_t *ringbuffer, uint8_t *buffer, ringbuffer_size_t len);

void ringbuffer_reset(ringbuffer_t *ringbuffer);

ringbuffer_size_t ringbuffer_read(ringbuffer_t *ringbuffer, uint8_t *dst, ringbuffer_size_t nbytes);

ringbuffer_size_t ringbuffer_write(ringbuffer_t *ringbuffer, uint8_t *src, ringbuffer_size_t nbytes);

ringbuffer_size_t ringbuffer_get_read_bytes(ringbuffer_t *ringbuffer);

ringbuffer_size_t ringbuffer_get_write_bytes(ringbuffer_t *ringbuffer);

ringbuffer_size_t ringbuffer_get_read_buffer(ringbuffer_t *ringbuffer, uint8_t **ptr);

ringbuffer_size_t ringbuffer_get_write_buffer(ringbuffer_t *ringbuffer, uint8_t **ptr);

bool ringbuffer_advance_write_index(ringbuffer_t *ringbuffer, ringbuffer_size_t nbytes);

bool ringbuffer_advance_read_index(ringbuffer_t *ringbuffer, ringbuffer_size_t nbytes);

#ifdef	__cplusplus
}
#endif

#endif	/* RINGBUFFER_H */
