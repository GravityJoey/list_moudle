/*
 * tuya_ota.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#include "../include/tuya_ota.h"
#include "../../port/include/port.h"

#ifdef TY_OTA_ENABLE

void ty_air_proc_ota(u8 len,u8 *buf)
{
    if(OTA_STARTING == ota_status){
        tuya_ota_proc(buf,len);
        return;
    }
}

void tuya_OTASaveData(u32 adr, u8 *data)
{
    ty_flash_write_page(NEW_FW_ADR + adr, 16, data);
}

void tuya_OTALoadData(u32 adr, u8 *data)
{
    ty_flash_read_page(NEW_FW_ADR + adr, 16, data);
}

void tuya_OTASetBootFlag(void)
{
	ty_flash_erase_sector(OTA_FLG_ADR);
	u32 flag = TELINK_OTA_BOOT_FLAG;
	ty_flash_write_page(OTA_FLG_ADR, 4, (u8 *)&flag);
}

void tuya_erase_new_firmware_bank(void)
{
    //25*4K(0X1000)=100K  START 0X40000
    u8 i = 0;
    for(i = 0; i < 25; i++)
    {
        ty_flash_erase_sector(NEW_FW_ADR + 0x1000*i);
    }
}

u16 ty_ota_crc(u8 *pbuf, int len) {

	static u16 poly[2] = {0, 0xa001};            //0x8005 <==> 0xa001
	u16 crc = 0xffff;
	u8 ds;
	int i, j;

	for(j = len; j > 0; j--) {
		ds = *pbuf++;

		for(i = 0; i < 8; i++) {
			crc = (crc >> 1) ^ poly[(crc ^ ds) & 1];
			ds = ds >> 1;
		}
	}

	return crc;
}

void tuya_ota_proc(u8 *t_data, u8 len)
{
    u16 index_num = 0,ota_crc = 0,ck_crc = 0;
    u8 *alloc_buf = NULL;
    u32 i_firmware = 0;

    if(0 != memcmp(ota_end_cmd,t_data,4)){
        ty_timer_start(TIMER_OTA_TIMEOUT,10*1000);
        index_num = t_data[0];
        index_num <<= 8;
        index_num += t_data[1];
        if(index_num == index_pagkt){
            i_firmware = index_pagkt*16;
            //if((i_firmware&0xfff) == 0)
                //flash_erase_sector(NEW_FW_ADR + i_firmware);
            tuya_OTASaveData(i_firmware,&t_data[2]);
            ota_crc = t_data[18];
            ota_crc <<= 8;
            ota_crc += t_data[19];
            alloc_buf = ty_malloc(16);
            tuya_OTALoadData(i_firmware,alloc_buf);
            ck_crc = ty_ota_crc(alloc_buf,16);
            if(ck_crc == ota_crc){
                index_pagkt++;
            }
            else{
                goto ota_over_error;
            }
            ty_free(alloc_buf);
        }
        else{
            if(index_pagkt>index_num)   return;
            //err && end ota
ota_over_error:
            index_pagkt = 0;
            alloc_buf  = ty_malloc(4);
            memcpy(alloc_buf,ota_end_cmd,4);
            alloc_buf[3] = 0x02;
            ty_ble_notify(4,alloc_buf);
            ota_status = OTA_ERROR;
            ty_free(alloc_buf);
            ty_timer_start(TIMER_LED_INDEX,100);
        }
    }
    else{
        tuya_OTASetBootFlag();
        alloc_buf = ty_malloc(4);
        memcpy(alloc_buf,ota_end_cmd,4);
        alloc_buf[3] = 0x01;
        ty_ble_notify(4,alloc_buf);
        ty_free(alloc_buf);
        ota_status = OTA_END;
        index_pagkt = 0;
        ty_timer_start(TIMER_LED_INDEX,100);
    }
}
#endif
