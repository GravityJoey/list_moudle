/*
 * tuya_event.c
 *
 *  Created on: 2017-9-11
 *      Author: echo
 */
 
#include "../include/tuya_event.h"
#include "tuya_fds.h"
#include "../../port/include/port.h"
#include "nrf_delay.h"

MYFIFO_INIT(tuya_evt_fifo, TUYA_EVT_MAX_NUM, sizeof(tuya_event_t));

u8 rdata[128+1]={0};

u8 tuya_event_process(void)
{
	if(my_fifo_get(&tuya_evt_fifo) == 0)
    {
        return 0;
    }
    tuya_event_t *evt = (tuya_event_t*)my_fifo_get(&tuya_evt_fifo);
	
	switch(evt->hdr.event_id)
	{
		case TUYA_FLASH_SAVE_EVT:
			u8 id = evt->saveFlash_evt.id;
			u8 module = evt->saveFlash_evt.module;
			u16 len = evt->saveFlash_evt.data_length;
			u8 *buf = evt->saveFlash_evt.data;
		
			/*
			tuya_fds_find_and_delete(GET_FILE_ID(module,id),GET_RECORD_KEY(module,id));
			tuya_fds_write_record(GET_FILE_ID(module,id),GET_RECORD_KEY(module,id),(uint32_t*)buf,CEIL_DIV(len,4));//除4向上取整
			tuya_fds_read_record(GET_FILE_ID(module,id),GET_RECORD_KEY(module,id),rdata);
			*/
			do{
		
			ty_flash_erase_sector(MODULE_ID_FLASH_ADDR(module,id));
			ty_flash_write_page(MODULE_ID_FLASH_ADDR(module,id),len,buf);
		
			nrf_delay_ms(5);
		
			ty_flash_read(module,id,rdata,len);
		
			}while(memcmp(rdata,buf,len) != 0);
		
			if(module == 0 && id == 0)
				printf("HID:%s\r\n",rdata);
		
			if(module == 0 && id == 1)
			{
				printf("AUZ:%s\r\n",rdata);
			}
			
			if(module == NV_USER_MODULE && id == NV_USER_ITEM_D_ID)
				printf("DID:%s\r\n",rdata);
		
			ty_free(buf);
			break;
		case TUYA_FLASH_READ_EVT:
			break;
		case TUYA_FLASH_ERASE_EVT:
			break;
	}
	
	 my_fifo_pop(&tuya_evt_fifo);
}

u8 tuya_event_send(tuya_event_t *evt)
{
	u8 *p = my_fifo_wptr (&tuya_evt_fifo);
    if (!p)
    {
        return TUYA_EVT_SEND_FAIL;
    }
	
	memcpy(p,(u8*)evt,sizeof(tuya_event_t));
	
    my_fifo_next (&tuya_evt_fifo);
	
	return TUYA_EVT_SEND_SUCCESS;
}