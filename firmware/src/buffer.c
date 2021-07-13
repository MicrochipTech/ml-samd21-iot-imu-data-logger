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
#include "buffer.h"

void buffer_init(struct sensor_buffer_t *buffer) {
    memset(buffer, 0, sizeof(struct sensor_buffer_t));
    buffer->_mask = SNSR_BUF_LEN - 1;
}

// Only the reader should call this function, and ONLY if overrun has already occurred
// Furthermore, reader should not be able to interrupt writer to call this function
void buffer_reset(struct sensor_buffer_t *buffer) {
    buffer->overrun = true; // Freeze buffering from the writer while we reset things
    buffer->readIdx = 0;
    buffer->writeIdx = 0;
    buffer->underrun = false;
    buffer->overrun = false;
}

buffer_size_t buffer_read(struct sensor_buffer_t *buffer, buffer_data_t dst[][SNSR_NUM_AXES], buffer_size_t framecount) {
    buffer_size_t availframes = buffer_get_read_frames(buffer);
    buffer_data_t *ptr;
    buffer_size_t rdcnt = buffer_get_read_buffer(buffer, &ptr);
    bool underrun = framecount > availframes;
    
    // Block from reading until this flag is cleared
//    if (buffer->underrun)
//        return 0;
    
    if (underrun)
        framecount = availframes;
    
    if (rdcnt >= framecount) {
        memcpy(dst[0], ptr, framecount*sizeof(buffer_frame_t));
    }
    else {
        memcpy(dst[0], ptr, rdcnt*sizeof(buffer_frame_t));
        memcpy(dst[rdcnt], buffer->data[0], (framecount - rdcnt)*sizeof(buffer_frame_t));
    }
    
    buffer_advance_read_index(buffer, framecount);
    buffer->underrun = underrun;
    return framecount;
}

buffer_size_t buffer_write(struct sensor_buffer_t *buffer, buffer_data_t data[][SNSR_NUM_AXES], buffer_size_t framecount) {   
    buffer_size_t availframes = buffer_get_write_frames(buffer);
    buffer_data_t *ptr;
    buffer_size_t wrcnt = buffer_get_write_buffer(buffer, &ptr);
    bool overrun = framecount > availframes;
    
    // Block from writing until this flag is cleared
    if (buffer->overrun)
        return 0;
    
    if (overrun)
        framecount = availframes;
    
    if (wrcnt >= framecount) {
        memcpy(ptr, data[0], framecount*sizeof(buffer_frame_t));
    }
    else {
        memcpy(ptr, data[0], wrcnt*sizeof(buffer_frame_t));
        memcpy(buffer->data[0], data[wrcnt], (framecount - wrcnt)*sizeof(buffer_frame_t));
    }
    
    buffer_advance_write_index(buffer, framecount);
    buffer->overrun = overrun;
    return framecount;
}

buffer_size_t buffer_get_read_frames(struct sensor_buffer_t *buffer) {
    return (buffer->writeIdx - buffer->readIdx) & buffer->_mask;
}

buffer_size_t buffer_get_write_frames(struct sensor_buffer_t *buffer) {
    return (buffer->readIdx - buffer->writeIdx - 1) & buffer->_mask;
}

buffer_size_t buffer_get_read_buffer(struct sensor_buffer_t *buffer, buffer_data_t **ptr) {
    buffer_size_t writeIdx = buffer->writeIdx;
    buffer_size_t readIdx = buffer->readIdx;
    
    *ptr = buffer->data[readIdx];
    
    if (writeIdx < readIdx) {
        return SNSR_BUF_LEN - readIdx;
    }
    else {
        return writeIdx - readIdx;
    }
}

buffer_size_t buffer_get_write_buffer(struct sensor_buffer_t *buffer, buffer_data_t **ptr) {
    buffer_size_t readIdx = buffer->readIdx;
    buffer_size_t writeIdx = buffer->writeIdx;
    
    *ptr = buffer->data[writeIdx];
    
    if (readIdx <= writeIdx) {
        return SNSR_BUF_LEN - writeIdx - (readIdx == 0);
    }
    else {
        return readIdx - writeIdx - 1;
    }
}

bool buffer_advance_read_index(struct sensor_buffer_t *buffer, buffer_size_t framecount) {
    buffer_size_t availframes = buffer_get_read_frames(buffer);
    buffer_size_t newIdx;
    
    // If underrun is already flagged, don't attempt to read any more data
//    if (buffer->underrun)
//        return false;
    
    // Note we advance the read index like the user asks regardless of underrun
    newIdx = (buffer->readIdx + framecount) & buffer->_mask;
    
    __buffer_sync();
    buffer->readIdx = newIdx;
    
    return !(buffer->underrun = availframes < framecount);
}

bool buffer_advance_write_index(struct sensor_buffer_t *buffer, buffer_size_t framecount) {
    buffer_size_t availframes = buffer_get_write_frames(buffer);
    buffer_size_t newIdx;
    
    // If overrun is already flagged, don't attempt to write any more data
    if (buffer->overrun)
        return false;
    
    // Note we advance the write index like the user asks regardless of overrun
    newIdx = (buffer->writeIdx + framecount) & buffer->_mask;
    
    __buffer_sync();
    buffer->writeIdx = newIdx;
    
    /* Flag overrun condition */
    /* (Note for simplicity this declares overrun 1 frame before overrun actually occurs) */    
    return !(buffer->overrun = availframes < framecount);
}