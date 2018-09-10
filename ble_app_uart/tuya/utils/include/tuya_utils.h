/*
 * tuya_utils.h
 *
 *  Created on: 2017-9-5
 *      Author: 123
 */

#ifndef TUYA_UTILS_H_
#define TUYA_UTILS_H_

#include "../../types.h"

u8 check_sum(u8 *pbuf,u8 len);

u8 check_num(u8 *buf,u8 num);

void hextoascii(u8 *hexbuf,u8 len,u8 *ascbuf);

void asciitohex(u8 *ascbuf,u8 *hexbuf);

void made_session_key(u8 *input_buf,u8 len,u8 *output_buf);

u8 char_2_ascii(u8 data);

void str_to_hex(u8 *str_buf,u8 str_len,u8 *hex_buf);

u32 adv_report_parse(u8 type, data_t * p_advdata, data_t * p_typedata);

void ConverseArrayToBigEndian(BYTE *srcArray,u32 *desBeData);

void ConverseArrayToLittleEndian(BYTE *srcArray,u32 *desLeData);


#endif /* TUYA_UTILS_H_ */
