/*
 * tuya_aes.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_AES_H_
#define TUYA_AES_H_

#include "../../app/include/ty_broad.h"
#include "../../../tuya/types.h"

void ty_aes_encrypt(u8 *in_buf,u8 in_len,u8 *out_buf);

void ty_aes_decrypt(u8 *in_buf,u8 in_len,u8 *out_buf);

#endif /* TUYA_AES_H_ */
