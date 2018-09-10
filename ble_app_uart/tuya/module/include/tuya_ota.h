/*
 * tuya_ota.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_OTA_H_
#define TUYA_OTA_H_

#include "../../app/include/ty_broad.h"

//#include "../../../proj_lib/ble/service/ble_ll_ota.h"

#ifdef TY_OTA_ENABLE
/*OTA new firmware bin is stored in 0x40000=64<<12
  OTA boot_flag is stored in 0x6a000=106<<12*/
#define	    NEW_FW_ADR	                    0x40000//(64<<12)
#define	    OTA_FLG_ADR			            0x73000// (115<<12)
#define     TELINK_OTA_BOOT_FLAG             (165)
u8 ota_status = NORMAL_STATUS;
u32 index_pagkt = 0;
u8 ota_end_cmd[4] = {0xaa,0x55,0xff,0x00};

void tuya_ota_proc(u8 *t_data, u8 len);
#endif

#endif /* TUYA_OTA_H_ */
