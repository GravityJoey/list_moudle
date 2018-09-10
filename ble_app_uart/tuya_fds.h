/*
 * tuya_fds.h
 *
 *  Created on: 2017-9-9
 *      Author: echo
 */
#ifndef TUYA_FDS_H_
#define TUYA_FDS_H_

#include "fds.h"

ret_code_t tuya_fds_init(void);
ret_code_t tuya_fds_read_record(uint16_t file_id,uint16_t key,uint32_t *data);
ret_code_t tuya_fds_write_record(uint16_t file_id,uint16_t key,uint32_t *data,uint8_t length);

#endif
