/*
 * tuya_data_api.c
 *
 *  Created on: 2017-9-25
 *      Author: echo
 */
#include "tuya_user_app.h"
#include "tuya_data_api.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
 
#define GET_TOTAL_CMD_COUNT			((sizeof(user_cmd_map))/(sizeof(user_cmd_map[0])))
	
/***********************************************************
*************************variable define********************
***********************************************************/

static tuya_user_cmd_map_t user_cmd_map[MAX_DP_COUNTS] = {0};
static tuya_user_cmd_pak_t cmd_pak = {user_cmd_map , 0};

/***********************************************************
*************************function define********************
***********************************************************/

static OPERATE_RET tuya_user_cmd_report(int flag);

/////////////////////////////////////////////////////////////

OPERATE_RET tuya_user_cmd_add(dp_tranport_type_t dp_tran_type,UINT8 dp_id, UINT8 dp_type, UINT8 dp_data_len, void *dp_data_ref, tuya_cmd_handle_fun handler)
{
	if(cmd_pak.cmd_index >= MAX_DP_COUNTS)
	{
		return OPRT_COM_ERROR;
	}
	
	cmd_pak.cmd_map[cmd_pak.cmd_index].dp_tran_type 	= dp_tran_type;
	cmd_pak.cmd_map[cmd_pak.cmd_index].dp_id 			= dp_id;
	cmd_pak.cmd_map[cmd_pak.cmd_index].dp_type 			= dp_type;
	cmd_pak.cmd_map[cmd_pak.cmd_index].dp_data_len 		= dp_data_len;
	cmd_pak.cmd_map[cmd_pak.cmd_index].dp_data_ref 		= dp_data_ref;
	cmd_pak.cmd_map[cmd_pak.cmd_index].handler 			= handler;
	cmd_pak.cmd_index++;
	
	return OPRT_OK;
}

UINT8 tuya_user_cmd_get_count(void)
{
	return cmd_pak.cmd_index;
}

void tuya_user_self_cmd_handle(UINT8 dp_id,UINT8 dp_type,UINT16 dp_data_len,BYTE *dp_data)
{
	int i = 0;
	
#define TUYA_DEBUG 1
#if TUYA_DEBUG
	printf("dp_id:%d\r\ndp_tpye:%d\r\n",dp_id,dp_type);
	printf("dp_data_len:%d\r\ndp_data:",dp_data_len);

	for(i = 0 ; i<dp_data_len ; i++)
	{
		printf("%0X",dp_data[i]);
	}
	printf("\r\n");
#endif
	
	/////////////////////////////////////////////////////////////
	for(i = 0 ; i < tuya_user_cmd_get_count() ; i++)
	{
		if(dp_id == cmd_pak.cmd_map[i].dp_id)
		{
			if(cmd_pak.cmd_map[i].dp_tran_type == DELIVERY || cmd_pak.cmd_map[i].dp_tran_type == REPORT_DELIVERY)
			{
				if(cmd_pak.cmd_map[i].handler != NULL)
				{
					cmd_pak.cmd_map[i].handler(dp_data_len,dp_data);
					cmd_pak.cmd_map[i].dp_data_len = dp_data_len;
				}
			}
			break;
		}
	}
	
}

OPERATE_RET tuya_user_cmd_status_report(void)
{
	return tuya_user_cmd_report(0);
}

OPERATE_RET tuya_user_cmd_RspAllDP(void)
{
	return tuya_user_cmd_report(1);
}

OPERATE_RET tuya_user_cmd_status_report_one(UINT8 dp_id)
{
	return tuya_user_cmd_report_one(dp_id);
}

static OPERATE_RET tuya_user_cmd_report(int flag)
{
	klv_node_s *list = NULL;
	u8 ret = 0,send_flg = 0;
	int i = 0;
	extern r_air_recv_packet air_recv_packet;
	extern frm_trsmitr_proc_s ty_trsmitr_proc;
	
	for(i = 0 ; i < tuya_user_cmd_get_count() ; i++)
	{
		if(cmd_pak.cmd_map[i].dp_tran_type == REPORT || cmd_pak.cmd_map[i].dp_tran_type == REPORT_DELIVERY)
		{
			list =  make_klv_list(list,cmd_pak.cmd_map[i].dp_id,cmd_pak.cmd_map[i].dp_type,cmd_pak.cmd_map[i].dp_data_ref,cmd_pak.cmd_map[i].dp_data_len);
			if(NULL == list) {
			   return OPRT_MALLOC_FAILED;
			}
			send_flg = 1;
		}
	}

	if(1 == send_flg){
		unsigned char *data;
		unsigned int da_len;
		ret = klvlist_2_data(list,&data,&da_len,0);
		if(MTP_OK != ret) {
			free_klv_list(list);
			return OPRT_COM_ERROR;
		}
		air_recv_packet.recv_len = da_len;
		memcpy(air_recv_packet.recv_data,data,da_len);
		ty_aes_encrypt(air_recv_packet.recv_data,da_len,air_recv_packet.de_encrypt_buf);
		trsmitr_init(&ty_trsmitr_proc);
		if(0 == flag){
			ty_timer_start(TIMER_REPORT_STATE,20);
		}
		else if(1 == flag){
			ty_timer_start(TIMER_CHECK_STATE,16);
		}
		free_klv_list(list);
		ty_free(data);
	}
	
	return OPRT_OK;
}

static OPERATE_RET tuya_user_cmd_report_one(UINT8 dp_id)
{
	int flag = 1;
	klv_node_s *list = NULL;
	u8 ret = 0,send_flg = 0;
	int i = 0;
	extern r_air_recv_packet air_recv_packet;
	extern frm_trsmitr_proc_s ty_trsmitr_proc;
	
	for(i = 0 ; i < tuya_user_cmd_get_count() ; i++)
	{
		if(cmd_pak.cmd_map[i].dp_id == dp_id)
		{
			if(cmd_pak.cmd_map[i].dp_tran_type == REPORT || cmd_pak.cmd_map[i].dp_tran_type == REPORT_DELIVERY)
			{
				list =  make_klv_list(list,cmd_pak.cmd_map[i].dp_id,cmd_pak.cmd_map[i].dp_type,cmd_pak.cmd_map[i].dp_data_ref,cmd_pak.cmd_map[i].dp_data_len);
				if(NULL == list) {
				   return OPRT_MALLOC_FAILED;
				}
				send_flg = 1;
			}
			break;
		}
	}

	if(1 == send_flg){
		unsigned char *data;
		unsigned int da_len;
		ret = klvlist_2_data(list,&data,&da_len,0);
		if(MTP_OK != ret) {
			free_klv_list(list);
			return OPRT_COM_ERROR;
		}
		air_recv_packet.recv_len = da_len;
		memcpy(air_recv_packet.recv_data,data,da_len);
		ty_aes_encrypt(air_recv_packet.recv_data,da_len,air_recv_packet.de_encrypt_buf);
		trsmitr_init(&ty_trsmitr_proc);
		if(0 == flag){
			ty_timer_start(TIMER_REPORT_STATE,16);
		}
		else if(1 == flag){
			ty_timer_start(TIMER_CHECK_STATE,16);
		}
		free_klv_list(list);
		ty_free(data);
	}
	
	return OPRT_OK;
}

extern u8 timing_cnt;
OPERATE_RET tuya_timer_report(UINT8 dp_id)
{
	int flag = 1;
	klv_node_s *list = NULL;
	u8 ret = 0,send_flg = 0;
	int i = 0;
	extern r_air_recv_packet air_recv_packet;
	extern frm_trsmitr_proc_s ty_trsmitr_proc;
	
	for(i = 0 ; i < tuya_user_cmd_get_count() ; i++)
	{
		if(cmd_pak.cmd_map[i].dp_id == TIMER_ACTION_ID)
		{
			if(cmd_pak.cmd_map[i].dp_tran_type == REPORT || cmd_pak.cmd_map[i].dp_tran_type == REPORT_DELIVERY)
			{
				list =  make_klv_list(list,cmd_pak.cmd_map[i].dp_id,cmd_pak.cmd_map[i].dp_type,cmd_pak.cmd_map[i].dp_data_ref,timing_cnt * 10);
				if(NULL == list) {
				   return OPRT_MALLOC_FAILED;
				}
				send_flg = 1;
			}
			break;
		}
	}

	if(1 == send_flg){
		unsigned char *data;
		unsigned int da_len;
		ret = klvlist_2_data(list,&data,&da_len,0);
		if(MTP_OK != ret) {
			free_klv_list(list);
			return OPRT_COM_ERROR;
		}
		air_recv_packet.recv_len = da_len;
		memcpy(air_recv_packet.recv_data,data,da_len);
		ty_aes_encrypt(air_recv_packet.recv_data,da_len,air_recv_packet.de_encrypt_buf);
		trsmitr_init(&ty_trsmitr_proc);
		if(0 == flag){
			ty_timer_start(TIMER_REPORT_STATE,16);
		}
		else if(1 == flag){
			ty_timer_start(TIMER_CHECK_STATE,16);
		}
		free_klv_list(list);
		ty_free(data);
	}
	
	return OPRT_OK;
}