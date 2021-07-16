/** @file ssi_comms.h */

/*==========================================================
 * Copyright 2021 QuickLogic Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *==========================================================*/

#ifndef SSI_COMMS_H_
#define SSI_COMMS_H_
#include "app_config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#ifndef SSI_JSON_CONFIG_VERSION
#define SSI_JSON_CONFIG_VERSION    (1)     /* 2 => Use enhance SSI protocol, 1 => use original SSI protocol */
#endif //SSI_JSON_CONFIG_VERSION
#define SSI_SYNC_DATA              (0xFF)
#define SSI_HEADER_SIZE            (9)     ///< SSI v2 header size in bytes
#define SSI_MAX_CHANNELS           (4)
#define SSI_CHANNEL_DEFAULT        (0)

#define CONNECT_STRING "connect"
#define CONNECT_CHARS 7
#define DISCONNECT_STRING "disconnect"
#define DISCONNECT_CHARS 10
#define TOTAL_CHARS 11

typedef size_t (*uart_rw)(uint8_t*, const size_t);

typedef struct
{
    uart_rw ssi_read;
    uart_rw ssi_write;
    bool initialized;
    volatile bool connected;
} ssi_io_funcs_t;

void ssi_init(ssi_io_funcs_t* p_interface);
bool ssi_connected(void);
void ssi_try_connect(void);
void ssi_try_disconnect(void);

void ssi_seqnum_init(uint8_t channel);
void ssi_seqnum_reset(uint8_t channel);
uint32_t ssi_seqnum_update(uint8_t channel);
uint32_t ssi_seqnum_get(uint8_t channel);
uint8_t ssi_payload_checksum_get(uint8_t *p_data, uint16_t len);

void ssiv2_publish_sensor_data(uint8_t channel, uint8_t* p_source, int ilen);
void ssiv1_publish_sensor_data(uint8_t* buffer, int size);
#endif /* SSI_COMMS_H_ */
