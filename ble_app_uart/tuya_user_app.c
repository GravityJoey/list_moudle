/*
 * tuya_user_app.c
 *
 *  Created on: 2017-9-22
 *      Author: echo
 */
 
#include "tuya_user_app.h"

#include "nrf_gpio.h"
#include "bsp.h"

/***********************************************************
*************************micro define***********************
***********************************************************/


/***********************************************************
*************************variable define********************
***********************************************************/


/***********************************************************
*************************function define********************
***********************************************************/

	
////////////////////////////////////////////////////////////
OPERATE_RET tuya_user_app_init(void)
{
	//add your custom cmd
	//cmd data ref must add
	//tran type,dp id, dp type,dp len,data ref,recv handler
	extern int switch_on_off_flag;
	extern curtain_run_state_t run_state;
	extern int run_percent;
	extern curtain_border_t border;
	extern int run_location;
	extern BOOL run_direction;
	extern BOOL hand_start;
	extern int mortor_state;
	extern BOOL border_state;
	extern int curtain_mortor_type;
	extern int dc_control_type;
    extern int ac_control_type;
    extern week_time_t tuya_time;
    extern u8 dp_action_arry[][5];

//	tuya_user_cmd_add(REPORT_DELIVERY,PLUG_SWITCH_ID,DT_BOOL,DT_BOOL_LEN,&switch_on_off_flag,handle_switch_1);

	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_RUN_ID,DT_ENUM,DT_ENUM_LEN,&run_state,curtain_run);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_PERCENT_ID,DT_VALUE,DT_VALUE_LEN,&run_percent,curtain_percent);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_BORDER_ID,DT_ENUM,DT_ENUM_LEN,&border,curtain_border);
	
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_LOCATION_ID,DT_VALUE,DT_VALUE_LEN,&run_location,curtain_location);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_DIRECTION_ID,DT_BOOL,DT_BOOL_LEN,&run_direction,curtain_direction);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_HAND_ID,DT_BOOL,DT_BOOL_LEN,&hand_start,curtain_hand);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_STATE_ID,DT_ENUM,DT_ENUM_LEN,&mortor_state,curtain_state);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_BORDER_STATE_ID,DT_ENUM,DT_ENUM_LEN,&border_state,curtain_border_state);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_TYPE_ID,DT_ENUM,DT_ENUM_LEN,&curtain_mortor_type,curtain_type);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_DC_CONTROL_ID,DT_ENUM,DT_ENUM_LEN,&dc_control_type,curtain_DC_control);
	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_AC_CONTROL_ID,DT_ENUM,DT_ENUM_LEN,&ac_control_type,curtain_AC_control);

    tuya_user_cmd_add(REPORT_DELIVERY,TIME_MODIFY_ID,DT_STRING,DT_STR_MAX,NULL,time_modify);
    tuya_user_cmd_add(REPORT_DELIVERY,TIMER_ACTION_ID,DT_STRING,DT_STR_MAX,&dp_action_arry,timer_action_manu);

//	tuya_user_cmd_add(REPORT_DELIVERY,CURTAIN_REPORT_ID,DT_STRING,DT_STR_MAX,NULL,curtain_report);

	//regist factory test 
	tuya_user_app_regist_factory_test_cb(on_factory_test_complete);
	
	nrf_gpio_cfg_output(BT_INDICATE_LED);
	nrf_gpio_cfg_output(PLUG_INDICATE_LED);
	nrf_gpio_cfg_output(PLUG_IO);
	
	uint32_t err_code;
	
	err_code = bsp_event_to_button_action_assign(PLUG_BTN_ID,
											 BSP_BUTTON_ACTION_RELEASE,
											 BSP_EVENT_PLUG_KEY);
	
	err_code = bsp_event_to_button_action_assign(PLUG_BTN_ID,
											 BSP_BUTTON_ACTION_LONG_PUSH,
											 BSP_EVENT_PLUG_KEY_LONG);
	
	return OPRT_OK;
}
