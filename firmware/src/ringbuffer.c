/*******************************************************************************
  Buffering Interface Source File

  Company:
    Microchip Technology Inc.

  File Name:
    ringbuffer.c

  Summary:
    This file contains the ring buffer API used for generic buffering

  Description:

  Notes:

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

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "ringbuffer.h"

int8_t ringbuffer_init(ringbuffer_t *ringbuffer, uint8_t *buffer, ringbuffer_size_t len) {
    // Check for power of 2
    if ( ((len - 1) & len) != 0 )
        return 0;

    memset(buffer, 0, sizeof(ringbuffer_t));
    ringbuffer->len = len;
    ringbuffer->data = buffer;
    ringbuffer->_mask = len - 1;

    return 0;
}

// Only the reader should call this function, and ONLY if overrun has already occurred
// Furthermore, reader should not be able to interrupt writer to call this function
void ringbuffer_reset(ringbuffer_t *ringbuffer) {
    ringbuffer->readIdx = 0;
    ringbuffer->writeIdx = 0;
}

ringbuffer_size_t ringbuffer_read(ringbuffer_t *ringbuffer, uint8_t *dst, ringbuffer_size_t bytecount) {
    ringbuffer_size_t availbytes = ringbuffer_get_read_bytes(ringbuffer);
    uint8_t *ptr;
    ringbuffer_size_t rdcnt = ringbuffer_get_read_buffer(ringbuffer, &ptr);
    bool underrun = bytecount > availbytes;

    if (underrun)
        bytecount = availbytes;

    if (rdcnt >= bytecount) {
        memcpy(dst, ptr, bytecount);
    }
    else {
        memcpy(dst, ptr, rdcnt);
        memcpy(dst + rdcnt, ringbuffer->data, (bytecount - rdcnt));
    }

    ringbuffer_advance_read_index(ringbuffer, bytecount);
    return bytecount;
}

ringbuffer_size_t ringbuffer_write(ringbuffer_t *ringbuffer, uint8_t *src, ringbuffer_size_t bytecount) {
    ringbuffer_size_t availbytes = ringbuffer_get_write_bytes(ringbuffer);
    uint8_t *ptr;
    ringbuffer_size_t wrcnt = ringbuffer_get_write_buffer(ringbuffer, &ptr);
    bool overrun = bytecount > availbytes;

    if (overrun)
        bytecount = availbytes;

    if (wrcnt >= bytecount) {
        memcpy(ptr, src, bytecount);
    }
    else {
        memcpy(ptr, src, wrcnt);
        memcpy(ringbuffer->data, src + wrcnt, (bytecount - wrcnt));
    }

    ringbuffer_advance_write_index(ringbuffer, bytecount);
    return bytecount;
}

ringbuffer_size_t ringbuffer_get_read_bytes(ringbuffer_t *ringbuffer) {
    return (ringbuffer->writeIdx - ringbuffer->readIdx) & ringbuffer->_mask;
}

ringbuffer_size_t ringbuffer_get_write_bytes(ringbuffer_t *ringbuffer) {
    return (ringbuffer->readIdx - ringbuffer->writeIdx - 1) & ringbuffer->_mask;
}

ringbuffer_size_t ringbuffer_get_read_buffer(ringbuffer_t *ringbuffer, uint8_t **ptr) {
    ringbuffer_size_t writeIdx = ringbuffer->writeIdx;
    ringbuffer_size_t readIdx = ringbuffer->readIdx;

    *ptr = ringbuffer->data + readIdx;

    if (writeIdx < readIdx) {
        return ringbuffer->len - readIdx;
    }
    else {
        return writeIdx - readIdx;
    }
}

ringbuffer_size_t ringbuffer_get_write_buffer(ringbuffer_t *ringbuffer, uint8_t **ptr) {
    ringbuffer_size_t readIdx = ringbuffer->readIdx;
    ringbuffer_size_t writeIdx = ringbuffer->writeIdx;

    *ptr = ringbuffer->data + writeIdx;

    if (readIdx <= writeIdx) {
        return ringbuffer->len - writeIdx - (readIdx == 0);
    }
    else {
        return readIdx - writeIdx - 1;
    }
}

bool ringbuffer_advance_read_index(ringbuffer_t *ringbuffer, ringbuffer_size_t bytecount) {
    ringbuffer_size_t readIdx = ringbuffer->readIdx;
    ringbuffer_size_t availbytes = (ringbuffer->writeIdx - readIdx) & ringbuffer->_mask;
    ringbuffer_size_t newIdx;

    // Note we advance the read index like the user asks regardless of underrun
    newIdx = (readIdx + bytecount) & ringbuffer->_mask;

    __ringbuffer_sync();
    ringbuffer->readIdx = newIdx;

    return availbytes >= bytecount;
}

bool ringbuffer_advance_write_index(ringbuffer_t *ringbuffer, ringbuffer_size_t nbytes) {
    ringbuffer_size_t writeIdx = ringbuffer->writeIdx;
    ringbuffer_size_t availbytes = (ringbuffer->readIdx - writeIdx - 1) & ringbuffer->_mask;
    ringbuffer_size_t newIdx;

    // Note we advance the write index like the user asks regardless of overrun
    newIdx = (writeIdx + nbytes) & ringbuffer->_mask;

    // Additionally, must make sure to flush any unfinished reads of writeIdx since
    // it may take more than one cycle to access
    __ringbuffer_sync();
    ringbuffer->writeIdx = newIdx;

    /* Flag overrun condition */
    /* (Note for simplicity this declares overrun 1 byte before overrun actually occurs) */
    return availbytes >= nbytes;
}
