/*
 * tuya_event.h
 *
 *  Created on: 2017-9-11
 *      Author: echo
 */

#ifndef TUYA_EVENT_H_
#define TUYA_EVENT_H_

#include "../../../tuya/types.h"
#include "../../utils/include/utility.h"

#define TUYA_EVT_MAX_NUM 		50

#pragma anon_unions

enum
{
	TUYA_EVT_SEND_SUCCESS = 0,
	TUYA_EVT_SEND_FAIL = -1,
};

typedef struct _tag_tuya_event_hdr
{
  u16 event_id;
  //u16 event_data_len;
} tuya_event_hdr_t;

typedef struct _tag_tuya_event_save_flash
{
	u8 module;
	u8 id;
	u16 data_length;
	void *data;
}tuya_event_save_flash_t;

typedef struct _tag_tuya_event
{
	tuya_event_hdr_t hdr;
	union
	{
		tuya_event_save_flash_t saveFlash_evt;
	};
}tuya_event_t;

enum
{
	TUYA_FLASH_SAVE_EVT = 0,
	TUYA_FLASH_READ_EVT,
	TUYA_FLASH_ERASE_EVT
};


u8 tuya_event_process(void);

u8 tuya_event_send(tuya_event_t *evt);

#endif