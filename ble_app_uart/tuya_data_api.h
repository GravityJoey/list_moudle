/*
 * tuya_data_api.h
 *
 *  Created on: 2017-9-25
 *      Author: echo
 */
 
#ifndef TUYA_DATA_API_H_
#define TUYA_DATA_API_H_

#include "tuya_common.h"

#define MAX_DP_COUNTS		(15)

typedef enum _tag_dp_tranport_type_t{
	REPORT = 0,
	DELIVERY,
	REPORT_DELIVERY
}dp_tranport_type_t;

typedef void (*tuya_cmd_handle_fun)(UINT8 dp_data_len,BYTE *dp_data);

typedef struct _tag_tuya_user_cmd_map_t
{
	dp_tranport_type_t		dp_tran_type;
	UINT8 					dp_id;
	UINT8 					dp_type;
	UINT8					dp_data_len;
	void					*dp_data_ref;
	tuya_cmd_handle_fun	handler;
}tuya_user_cmd_map_t;

typedef struct _tag_tuya_user_cmd_pak_t
{
	tuya_user_cmd_map_t 	*cmd_map;
	UINT8 					cmd_index;
}tuya_user_cmd_pak_t;

UINT8 tuya_user_cmd_get_count(void);

OPERATE_RET tuya_user_cmd_add(dp_tranport_type_t dp_tran_type,UINT8 dp_id, UINT8 dp_type, UINT8 dp_data_len, void *dp_data_ref, tuya_cmd_handle_fun handler);

OPERATE_RET tuya_user_cmd_status_report(void);

OPERATE_RET tuya_user_cmd_status_report_one(UINT8 dp_id);

OPERATE_RET tuya_user_cmd_RspAllDP(void);

OPERATE_RET tuya_timer_report(UINT8 dp_id);

#endif