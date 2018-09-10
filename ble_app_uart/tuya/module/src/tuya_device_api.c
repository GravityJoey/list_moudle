/*
 * tuya_device_api.c
 *
 *  Created on: 2017-10-11
 *      Author: echo
 */
#include "../../port/include/port.h"
#include "../include/tuya_device_api.h"

void tuya_device_unbond(void)
{
	printf("tuya_device_unbond\r\n");
	u8 *alloc_buf = NULL;
	
	extern b_ble_para ble_para;
	alloc_buf = ty_malloc(7);
	ble_para.pass_key_flag = 0x0;
	memcpy(alloc_buf,ble_para.pass_key,PASS_KEY_LEN);
	alloc_buf[PASS_KEY_LEN] = ble_para.pass_key_flag;
	ty_flash_write(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, alloc_buf,PASS_KEY_LEN+1);
	ty_free(alloc_buf);
}

void tuya_device_reboot(u32 time_ms)
{
	ty_timer_start(TIMER_REBOOT,time_ms);
}