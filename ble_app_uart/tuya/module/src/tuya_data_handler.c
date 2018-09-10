/*
 * tuya_data_handler.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */
#include "../include/tuya_data_handler.h"
#include "../../port/include/port.h"
#include "../include/tuya_led.h"
#include "../../utils/include/tuya_utils.h"
#include "stdlib.h"

#include "app_timer.h"
#include "ble_gap.h"

#include "weishida_protocol.h"
#include "weishida_device_api.h"

#include "tuya_data_api.h"

extern u8 p_id[P_ID_LEN];

extern u8 ty_ble_send_flag;
extern r_air_recv_packet air_recv_packet;
extern frm_trsmitr_proc_s ty_trsmitr_proc;
extern u8 ty_ble_state;
extern u8 pair_rand[6];
extern b_ble_para ble_para;
extern u8 tbl_scanRsp[28];

extern u8 ota_status;

APP_TIMER_DEF(data_reset_timer_id);

void data_reset_timer_handler(void * p_context)
{
	ty_ble_discon();
}

void ty_make_session_key(void)
{
    u8 *p_buf = NULL;

    p_buf = ty_malloc(12);
    memset(ble_para.session_key,0,16);
    memcpy(p_buf,pair_rand,6);
    memcpy(&p_buf[6],ble_para.pass_key,6);
    made_session_key(p_buf,12,ble_para.session_key);
    ty_free(p_buf);
    return;
}

void ty_air_proc(u8 len,u8 *buf)
{
    mtp_ret ret;
    static u8 offset = 0;
    klv_node_s *list = NULL;
    u8 payload_len = 0;
    u16 rand_value = 0;
    u8 *p_buf = NULL;
    u8 *alloc_buf = NULL;
	
	//tuya_log(0x3c,len,buf);

	if(1 == ty_ble_send_flag)	return;
    p_buf = ty_malloc(70);
    ret = trsmitr_recv_pkg_decode(&ty_trsmitr_proc, buf, len);
    if(MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
        air_recv_packet.recv_len = 0;
        memset(air_recv_packet.recv_data,0,AIR_FRAME_MAX);
        ty_free(p_buf);
        return;
    }

    if(FRM_PKG_FIRST == ty_trsmitr_proc.pkg_desc){
        air_recv_packet.recv_len = 0;
        memset(air_recv_packet.recv_data,0,AIR_FRAME_MAX);
        offset = 0;
    }
    memcpy(air_recv_packet.recv_data+offset,get_trsmitr_subpkg(&ty_trsmitr_proc),get_trsmitr_subpkg_len(&ty_trsmitr_proc));
    offset += get_trsmitr_subpkg_len(&ty_trsmitr_proc);
    air_recv_packet.recv_len = offset;
    if(ret == MTP_OK)
    {
        offset = 0;
        if((UNBONDING == ty_ble_state)&&(FRM_QRY_DEV_INFO_REQ != ty_trsmitr_proc.type)\
            &&(PAIR_REQ != ty_trsmitr_proc.type)&&(FRM_LOGIN_KEY_REQ != ty_trsmitr_proc.type)){
            ty_free(p_buf);
            return;
        }
        switch(ty_trsmitr_proc.type)
        {
            case FRM_QRY_DEV_INFO_REQ:
                rand_value = rand();
                pair_rand[0] = (u8)(rand_value&0xff);
                pair_rand[1] = (u8)((rand_value>>8)&0xff);
                rand_value = rand();
                pair_rand[2] = (u8)(rand_value&0xff);
                pair_rand[3] = (u8)((rand_value>>8)&0xff);
                rand_value = rand();
                pair_rand[4] = (u8)(rand_value&0xff);
                pair_rand[5] = (u8)((rand_value>>8)&0xff);
                p_buf[0] = soft_ver_high;
                payload_len++;
                p_buf[1] = soft_ver_low;
                payload_len++;
                p_buf[2] = protocol_ver_high;
                payload_len++;
                p_buf[3] = protocol_ver_low;
                payload_len++;
                p_buf[4] = 0x00;
                payload_len++;
                memcpy(&p_buf[5],pair_rand,6);
                payload_len += 6;
                if(0x0 == ble_para.pass_key_flag){
                    alloc_buf = ty_malloc(AUTH_KEY_LEN);
                    ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_AUZ_KEY, alloc_buf, AUTH_KEY_LEN);//TODO need flash op
                    memcpy(&p_buf[11],alloc_buf,AUTH_KEY_LEN);
                    ty_free(alloc_buf);
                    payload_len += AUTH_KEY_LEN;
                }
                else
                {
                    ty_make_session_key();
                }
                ty_air_send(payload_len,p_buf,FRM_QRY_DEV_INFO_RESP);
            break;

            case PAIR_REQ:
#if 1
                alloc_buf = ty_malloc(air_recv_packet.recv_len);
                if(alloc_buf != NULL){
                    for(u8 i = 0;i < air_recv_packet.recv_len;i += 16) {
                        AES128_ECB_decrypt(air_recv_packet.recv_data+i,ble_para.session_key,alloc_buf+i);
                    }
                    asciitohex(alloc_buf,air_recv_packet.de_encrypt_buf);
                    ty_free(alloc_buf);
                }
#else
                //嵌套太深，导致栈溢出
                ty_aes_decrypt(air_recv_packet.recv_data,air_recv_packet.recv_len,air_recv_packet.de_encrypt_buf);
#endif
                if((0 == memcmp(&air_recv_packet.de_encrypt_buf[1],ble_para.d_id,D_ID_LEN))&&\
                    (0 == memcmp(&air_recv_packet.de_encrypt_buf[17],ble_para.pass_key,PASS_KEY_LEN))){
                    p_buf[0] = 0x00;
                    ty_ble_state = BONDING_CONN;
                }
                else{
                    p_buf[0] = 0x01;
                }
                
                ty_aes_encrypt(p_buf,1,air_recv_packet.de_encrypt_buf);
                ty_air_send(air_recv_packet.de_encrypt_buf[0],&air_recv_packet.de_encrypt_buf[1],PAIR_RESP);
                if(p_buf[0]){    ty_ble_discon();}
#ifdef TY_SELF_MODE
				load_led_data();
                ty_timer_start(TIMER_RGB_INDX,6);
#endif
#ifdef TY_SPP_MODE
                ty_set_pwm(PWMID_WW,30);
								ty_uart_protocol_send(TY_REPORT_WORK_STATE_TYPE,&ty_ble_state,1);
#endif
                ty_timer_stop(TIMER_LED_INDEX);
				//ty_timer_start(TIMER_QUERY,200);//BY ECHO
            break;

            case FRM_CMD_SEND:
                ty_aes_decrypt(air_recv_packet.recv_data,air_recv_packet.recv_len,air_recv_packet.de_encrypt_buf);
                ret = data_2_klvlist(&air_recv_packet.de_encrypt_buf[1],air_recv_packet.de_encrypt_buf[0],&list,0);
                if(MTP_OK != ret) {
                    ty_free(p_buf);
                    return ;
                }
#ifdef   TY_SPP_MODE
                spp_cmd_proc(list);
#elif defined(TY_SELF_MODE)
                //self_cmd_proc(list);
				plug_self_cmd_proc(list);
#endif
                p_buf[0] = 0x00;
                ty_aes_encrypt(p_buf,1,air_recv_packet.de_encrypt_buf);
                ty_air_send(air_recv_packet.de_encrypt_buf[0],&air_recv_packet.de_encrypt_buf[1],FRM_CMD_ACK);
#ifdef  TY_SELF_MODE
                //report_state(0);
//				tuya_user_cmd_status_report();
				tuya_user_cmd_status_report_one(list->id);
				free_klv_list(list);
#endif
            break;

            case FRM_ALL_DP_QUERY:
#ifdef  TY_SELF_MODE
				tuya_user_cmd_RspAllDP();
                //report_state(1);
#endif
            break;

            case FRM_LOGIN_KEY_REQ:
                if(ENABLE != ble_para.pass_key_flag){
                    ble_para.pass_key_flag = ENABLE;
                    memcpy(ble_para.pass_key,air_recv_packet.recv_data,PASS_KEY_LEN);
                    tbl_scanRsp[2] |= 0x80;
                    ty_set_scanrsp_data(sizeof(tbl_scanRsp),tbl_scanRsp);//need to change adv data
                    p_buf[0] = 0;
                    ty_make_session_key();
                    alloc_buf = ty_malloc(7);
                    memcpy(alloc_buf,ble_para.pass_key,PASS_KEY_LEN);
                    alloc_buf[PASS_KEY_LEN] = ble_para.pass_key_flag;
                    ty_flash_write(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, alloc_buf, PASS_KEY_LEN+1);
					//ty_flash_read(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, alloc_buf, PASS_KEY_LEN+1);

					//TODO skip by echo
					
                    memcpy(ble_para.pass_key,alloc_buf,PASS_KEY_LEN);
                    ble_para.pass_key_flag = alloc_buf[PASS_KEY_LEN];
                    ty_free(alloc_buf);

                    ty_air_send(1,p_buf,FRM_LOGIN_KEY_RESP);
					
					//app_timer_create(&data_reset_timer_id,APP_TIMER_MODE_SINGLE_SHOT,data_reset_timer_handler);
					//app_timer_start(data_reset_timer_id,APP_TIMER_TICKS(1000,APP_TIMER_PRESCALER),NULL);
                }
                else
                {
                    p_buf[0] = 1;
                    ty_air_send(1,p_buf,FRM_LOGIN_KEY_RESP);
                    ty_ble_discon();
                }
            break;

            case FRM_UNBONDING_REQ:
                alloc_buf = ty_malloc(7);
                ble_para.pass_key_flag = 0x0;
                memcpy(alloc_buf,ble_para.pass_key,PASS_KEY_LEN);
                alloc_buf[PASS_KEY_LEN] = ble_para.pass_key_flag;
                ty_flash_write(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, alloc_buf,PASS_KEY_LEN+1);
                ty_free(alloc_buf);
                p_buf[0] = 0x00;
                ty_air_send(1,p_buf,FRM_UNBONDING_RESP);
			
				app_timer_create(&data_reset_timer_id,APP_TIMER_MODE_SINGLE_SHOT,data_reset_timer_handler);
				app_timer_start(data_reset_timer_id,APP_TIMER_TICKS(2000,APP_TIMER_PRESCALER),NULL);
            break;

            case FRM_OTA_START_REQ:
#ifdef TY_OTA_ENABLE
                p_buf[0] = 0x00;
                ty_air_send(1,p_buf,FRM_OTA_START_RESP);
                ota_status = OTA_STARTING;
                ty_timer_stop_all();
                ty_timer_start(TIMER_LED_INDEX,100);
                ty_timer_start(TIMER_OTA_TIMEOUT,10*1000);
#ifdef TY_SELF_MODE
                ty_set_pwm(PWMID_R,0);
                ty_set_pwm(PWMID_G,0);
                ty_set_pwm(PWMID_B,0);
                ty_set_pwm(PWMID_WW,0);
#endif
#endif
            break;

            default:
                break;
        }
    }
    ty_free(p_buf);
}

#ifdef TY_SPP_MODE

void ty_uart_protocol_proc(u8 len,u8 *pData)
{
	static char isFirstStart = 0;
    u8 ck_sum=0;
    mtp_ret ret;
    klv_node_s *list = NULL;
    u8 *p_buf = NULL,*alloc_buf = NULL;
    unsigned int da_len;
	static int isFirstBoot = FALSE;

    ck_sum = check_sum(&pData[0],len-1);
    if((0x55 == pData[0])&&(0xaa == pData[1])&&\
        (ck_sum == pData[len-1])){
        switch(pData[3])
        {
            case TY_HEART_MSG_TYPE:
                //if(0 == pData[6]){
                //    ty_uart_protocol_send(TY_SEARCH_PID_TYPE,NULL,0);
                //}
            	if((0 == pData[6]) || (0 == isFirstStart)){
            		isFirstStart = 1;
            		ty_uart_protocol_send(TY_SEARCH_PID_TYPE,NULL,0);
            	}
            break;

            case TY_SEARCH_PID_TYPE:
                memcpy(p_id,&pData[6],8);
                //mcu_ver_max = pData[14];
                //mcu_ver_mid = pData[15];
                //mcu_ver_min = pData[16];
                ty_uart_protocol_send(TY_CK_MCU_TYPE,NULL,0);
				memcpy(&tbl_scanRsp[4],p_id,P_ID_LEN);
				ty_set_scanrsp_data(sizeof(tbl_scanRsp),tbl_scanRsp);
            break;

            case TY_CK_MCU_TYPE:
                /*if(0 == pData[5]){
                    mcu_work_mode = 1;
                }
                else{
                    mcu_work_mode = 2;
                }*/
                ty_uart_protocol_send(TY_REPORT_WORK_STATE_TYPE,&ty_ble_state,1);

				ty_timer_start(TIMER_QUERY,200);
            break;

            case TY_REBOOT_TYPE:
				if(TRUE == isFirstBoot)return;
				isFirstBoot = true;
                alloc_buf = ty_malloc(7);
                memcpy(alloc_buf,ble_para.pass_key,PASS_KEY_LEN);
                ble_para.pass_key_flag = 0x00;
                alloc_buf[6] = ble_para.pass_key_flag;
				ty_flash_write(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, alloc_buf,PASS_KEY_LEN+1);
                ty_free(alloc_buf);
                ty_uart_protocol_send(TY_REBOOT_TYPE,NULL,0);
                ty_timer_start(TIMER_REBOOT,500);
            break;

            case TY_SEND_STATUS_TYPE:
                ret = data_2_klvlist(&pData[6],pData[5],&list,1);
                if(MTP_OK != ret) {
                    return ;
                }
				
				//FIXME 内存覆盖bug，DP解析本身有问题。bug find 20170922
				u8 *buf = 0x200030CD;
				if(buf[0] == 0x66)
				{
					buf++;
				}
				
                ret = klvlist_2_data(list,&p_buf,&da_len,0);
				
				free_klv_list(list);
                if(MTP_OK != ret) {
                    return;
                }
                ty_aes_encrypt(p_buf,(u8)da_len,air_recv_packet.de_encrypt_buf);
                ty_free(p_buf);
				trsmitr_init(&ty_trsmitr_proc);
                ty_timer_start(TIMER_REPORT_STATE,6);
            break;

            default:
                break;
        }
    }
}
#endif


#ifdef TY_SELF_MODE

int run_location = 0;
BOOL run_direction = 0;
int current_direction = 0;
BOOL hand_start = 0;
int mortor_state = 0;
BOOL border_state = 0;
int curtain_mortor_type = 0;
int dc_control_type = 0;
int ac_control_type = 0;
extern BOOL border_top_limit_set_flg;
extern BOOL border_bottom_limit_set_flg;
extern bool set_percent;
extern int run_percent;

void tuya_user_cmd_status_report_one_info(u8 cmd)
{
	switch(cmd)	{
		case CMD_POSITON:
			tuya_user_cmd_status_report_one(CURTAIN_LOCATION_ID);
			break;
		case CMD_DIRECT:
			tuya_user_cmd_status_report_one(CURTAIN_DIRECTION_ID);
			break;
		case CMD_HAND_START:
			tuya_user_cmd_status_report_one(CURTAIN_HAND_ID);
			break;
		case CMD_GET_STATE:
			tuya_user_cmd_status_report_one(CURTAIN_STATE_ID);
			break;
		case CMD_GET_BORDER_STATE:
			tuya_user_cmd_status_report_one(CURTAIN_BORDER_STATE_ID);
			break;
		case CMD_SWITCH_CONTROL_DC_TYPE:
			tuya_user_cmd_status_report_one(CURTAIN_DC_CONTROL_ID);
			break;
		case CMD_SWITCH_CONTROL_AC_TYPE:
			tuya_user_cmd_status_report_one(CURTAIN_AC_CONTROL_ID);
			break;
		case CMD_DEVICE_TYPE:
			tuya_user_cmd_status_report_one(CURTAIN_TYPE_ID);
			break;
	}
}

/*
void tuya_user_cmd_status_report_info(void)
{
	tuya_user_cmd_status_report_one(CURTAIN_LOCATION_ID);
	tuya_user_cmd_status_report_one(CURTAIN_DIRECTION_ID);
	tuya_user_cmd_status_report_one(CURTAIN_HAND_ID);
	tuya_user_cmd_status_report_one(CURTAIN_STATE_ID);
	tuya_user_cmd_status_report_one(CURTAIN_BORDER_STATE_ID);
}*/
BOOL data_send_flag = 0;

void read_info_report(curtains_data_t *packet_data)
{
	switch(packet_data->cmd){
		case CMD_POSITON:
			run_location = packet_data->payload[1];
			break;

		case CMD_DIRECT:
			current_direction = packet_data->payload[1];
			if (run_direction != current_direction) {
				weishida_device_set_motor_direct(run_direction);
				data_send_flag = 1;
			}
			break;

		case CMD_HAND_START:
			
			break;

		case CMD_GET_STATE:
			mortor_state = packet_data->payload[1];
			break;

		case CMD_GET_BORDER_STATE:
			border_state = packet_data->payload[1];
			if(border_state) {
				border_top_limit_set_flg = 1;
				border_bottom_limit_set_flg = 1;
				if(set_percent) {
					weishida_device_start_percent(run_percent);
					data_send_flag = 1;
					set_percent = 0;
				}
			}
			break;

		case CMD_DEVICE_TYPE:
			curtain_mortor_type = packet_data->payload[1];
			break;
		
		default:
        	break;
	}
	tuya_user_cmd_status_report_one_info(packet_data->cmd);
}

void write_info_report(curtains_data_t *packet_data)
{
/*	switch(packet_data->cmd){
		case CMD_DIRECT:
			tuya_user_cmd_status_report();
			break;

		case CMD_HAND_START:
			tuya_user_cmd_status_report();
			break;

		case CMD_SWITCH_CONTROL_DC_TYPE:
			tuya_user_cmd_status_report();
			break;

		case CMD_SWITCH_CONTROL_AC_TYPE:
			tuya_user_cmd_status_report();
			break;
		
		default:
        	break;
	}*/
	tuya_user_cmd_status_report_one_info(packet_data->cmd);
}

void control_info_report(curtains_data_t *packet_data)
{
	switch(packet_data->cmd){
		case CMD_OPEN:
		case CMD_CLOSE:
			weishida_device_get_position();
			APP_TIMER_DEF(get_position_timer_id);
            app_timer_create(&get_position_timer_id, APP_TIMER_MODE_SINGLE_SHOT, weishida_device_get_state);
            app_timer_start(get_position_timer_id, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
			data_send_flag = 1;
			break;
		case CMD_SET_TOP_BORDER:
		case CMD_SET_BOTTOM_BORDER:
//		case CMD_DELETE_ALL_BORDER:
//			weishida_device_get_border_state();
//			data_send_flag = 1;
			break;
		default:
			break;
	}
}


void notify_info_report(curtains_data_t *packet_data)
{
    
    static int FirstBoot = FALSE;
    u8 *alloc_buf = NULL;
	switch(packet_data->cmd){
		case CMD_DEVICE_STATE:
			run_location = packet_data->payload[1];
			current_direction = packet_data->payload[2];
			hand_start = packet_data->payload[3];
			mortor_state = packet_data->payload[4];
			border_state = packet_data->payload[8];
			if(border_state) {
				border_top_limit_set_flg = 1;
				border_bottom_limit_set_flg = 1;
			}

//            weishida_device_get_border_state();
            tuya_user_cmd_status_report_one(CURTAIN_BORDER_STATE_ID);

            APP_TIMER_DEF(report_location_timer_id);
            app_timer_create(&report_location_timer_id, APP_TIMER_MODE_SINGLE_SHOT, tuya_user_cmd_status_report_one);
            app_timer_start(report_location_timer_id, APP_TIMER_TICKS(50, APP_TIMER_PRESCALER), CURTAIN_LOCATION_ID);

            APP_TIMER_DEF(report_mortor_timer_id);
            app_timer_create(&report_mortor_timer_id, APP_TIMER_MODE_SINGLE_SHOT, tuya_user_cmd_status_report_one);
            app_timer_start(report_mortor_timer_id, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), CURTAIN_STATE_ID);

//			tuya_user_cmd_status_report();
			break;

		case CMD_ERROR_INFO:
//			bsp_board_led_on(1);
			break;

		case CMD_RESET:
		    if(TRUE == FirstBoot)return;
			FirstBoot = true;
            alloc_buf = ty_malloc(7);
            memcpy(alloc_buf,ble_para.pass_key,PASS_KEY_LEN);
            ble_para.pass_key_flag = 0x00;
            alloc_buf[6] = ble_para.pass_key_flag;
			ty_flash_write(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, alloc_buf,PASS_KEY_LEN+1);
            ty_free(alloc_buf);
            ty_uart_protocol_send(TY_REBOOT_TYPE,NULL,0);
            ty_timer_start(TIMER_REBOOT,500);
		    break;
		
		default:
			break;
	}
}

extern BOOL uart_init_flag;
void weishida_uart_protocol_proc(u8 len,u8 *pData)
{
    uint32_t ret = 0;
	raw_data_t uart_raw_data;
	curtains_data_t *packet_data;

	data_send_flag = 0;
	uart_raw_data.len = len;
	memcpy(uart_raw_data.data, pData, len);

    ret = weishida_protocol_packet_decode(packet_data, &uart_raw_data);

	switch(packet_data->opcode) {
		case OPCODE_READ:
			read_info_report(packet_data);
			break;

		case OPCODE_WRITE:
			write_info_report(packet_data);
			break;

		case OPCODE_CONTROL:
			control_info_report(packet_data);
			break;

		case OPCODE_NOTIFY:
			notify_info_report(packet_data);
			break;

		default:
        	break;
	}
#ifdef CURTAIN_LOW_POWER
    if(ret == 0 && data_send_flag == 0)
        wistar_uart_close();
#endif
}


extern led_state_s led_state;

void list_to_scene(klv_node_s *node)
{
    u8 data_tmp = 0,i = 0;

    led_state.scene_data_change_flag = 1;
    memcpy((void *restrict)led_state.clor_data,(const void *restrict)node->data,node->len);
    led_state.clor_len = node->len;
    data_tmp = node->len;
    str_to_hex((u8*)node->data,8,led_state.scene_temp);
    data_tmp -= 8;
    do{
        if(data_tmp >= 6){
            data_tmp -= 6;
            str_to_hex(&node->data[8+i*6],6,led_state.scene_data[i]);
            i++;
        }
        else{
            data_tmp = 0;
        }
    }while(data_tmp);
}

void plug_self_cmd_proc(klv_node_s *list)
{
    klv_node_s *node = list;
    u8 data_tmp = 0,i=0;

    while(node){
		tuya_user_self_cmd_handle(node->id,node->type,node->len,node->data);
        node = node->next;
    }
}

void self_cmd_proc(klv_node_s *list)
{
    klv_node_s *node = list;
    u8 data_tmp = 0,i=0;

    for(i = 0;i<(END_ID_NUM-1);i++){
        led_state.report_flg[i] = 0;
    }
    while(node){
        switch(node->id){
            case LED_SWITCH_ID:
                if(0x0 == node->data[0]){
                    ty_set_pwm(PWMID_R,0);
                    ty_set_pwm(PWMID_WW,0);
                    ty_set_pwm(PWMID_G,0);
                    ty_set_pwm(PWMID_B,0);
                }
                led_state.led_switch = node->data[0];
                led_state.report_flg[LED_SWITCH_ID -1] = 1;
            break;

            case WORK_MODE_ID:
                led_state.work_mode = node->data[0];
                led_state.report_flg[WORK_MODE_ID -1] = 1;
            break;

            case BRIGHT_ID:
                led_state.bright_value = node->data[3];
                data_tmp = (u8)led_state.bright_value;
                if(WHILTE_MODE != led_state.work_mode){
                    ty_set_pwm(PWMID_R,data_tmp);
                    ty_set_pwm(PWMID_WW,data_tmp);
                    ty_set_pwm(PWMID_G,data_tmp);
                    ty_set_pwm(PWMID_B,0);
                }
                led_state.report_flg[BRIGHT_ID -1] = 1;
            break;

            case TEMP_ID:
                led_state.temp_value = node->data[3];
                data_tmp = led_state.temp_value;
                ty_set_pwm(PWMID_R,255 - data_tmp);
                ty_set_pwm(PWMID_WW,data_tmp);
                ty_set_pwm(PWMID_G,255 - data_tmp);
                ty_set_pwm(PWMID_B,0);
                led_state.report_flg[TEMP_ID -1] = 1;
            break;

            case CLOR_ID:
                memcpy((void *restrict)led_state.clor_data,(const void *restrict)node->data,node->len);
                led_state.clor_len = node->len;
                led_state.scene_temp[SCENE_NUM_POS] = 1;
                str_to_hex((u8*)node->data,6,(u8*)&led_state.scene_data[0]);
                led_state.report_flg[CLOR_ID-1] = 1;
            break;

            case SCENE_NUM_ID:
                memcpy((void *restrict)led_state.clor_data,(const void *restrict)node->data,node->len);
                led_state.scene_temp[SCENE_NUM_POS] = 1;
                str_to_hex((u8*)node->data,6,(u8*)&led_state.scene_data[0]);
                led_state.report_flg[SCENE_NUM_ID -1] = 1;
            break;

            case ROUGUANG_SCENE_ID:
                if(node->len >= 14){
                    list_to_scene(node);
                    led_state.report_flg[ROUGUANG_SCENE_ID -1] = 1;
                }
            break;

            case BINFENG_SCENE_ID:
                if(node->len >= 14){
                    list_to_scene(node);
                    led_state.report_flg[BINFENG_SCENE_ID -1] = 1;
					//ty_uart_send(strlen("1111\r\n"),"1111\r\n");
                }
            break;

            case XUANCAI_SCENE_ID:
                if(node->len >= 14){
                    list_to_scene(node);
                    led_state.report_flg[XUANCAI_SCENE_ID -1] = 1;
                }
            break;

            case BANLAN_SCENE_ID:
                if(node->len >= 14){
                    list_to_scene(node);
                    led_state.report_flg[BANLAN_SCENE_ID -1] = 1;
                }
            break;

            default:
            break;
        }
        node = node->next;
    }
}
#endif

#ifdef TY_SPP_MODE
void spp_cmd_proc(klv_node_s *list)
{
    klv_node_s *node = list;
    mtp_ret ret;
    unsigned char *data = NULL;
    unsigned int da_len;

    while(node){
        switch(node->type){
            case DT_RAW:
            case DT_BOOL:
            case DT_VALUE:
            case DT_STRING:
            case DT_ENUM:
            case DT_BITMAP:
                ret = klvlist_2_data(node,&data,&da_len,1);//TODO 复合型 DP bug
                if(MTP_OK != ret) {
                    return;
                }
            break;

            default:
                break;
        }
        node = node->next;
    }
	
    ty_uart_protocol_send(TY_SEND_CMD_TYPE,data,da_len);
    ty_free(data);
}
#endif
