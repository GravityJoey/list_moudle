/*
 * tuya_user_app.h
 *
 *  Created on: 2017-9-22
 *      Author: echo
 */
 
#ifndef USER_APP_H_
#define USER_APP_H_

#include "tuya_common.h"
#include "tuya_user_config.h"
#include "tuya_user_app_api.h"
#include "tuya/time/include/tuya_timer_schdule.h"

#define PLUG_SWITCH_ID			    (1)

#define CURTAIN_RUN_ID		        (101)
#define CURTAIN_PERCENT_ID	        (102)
#define CURTAIN_BORDER_ID	        (104)

#define CURTAIN_LOCATION_ID	        (112)
#define CURTAIN_DIRECTION_ID	    (103)
#define CURTAIN_HAND_ID	            (105)
#define CURTAIN_STATE_ID	        (106)
#define CURTAIN_BORDER_STATE_ID	    (111)
#define CURTAIN_DC_CONTROL_ID	    (108)
#define CURTAIN_AC_CONTROL_ID	    (107)
#define CURTAIN_TYPE_ID	            (109)

#define CURTAIN_REPORT_ID	        (110)

#define TIME_MODIFY_ID	            (113)
#define TIMER_ACTION_ID	            (114)


#define PLUG_IO					(4)
#define PLUG_BTN_ID				(2)//gpio 15
#define BT_INDICATE_LED			(2)
#define PLUG_INDICATE_LED		(3)
	
OPERATE_RET tuya_user_app_init(void);

#endif