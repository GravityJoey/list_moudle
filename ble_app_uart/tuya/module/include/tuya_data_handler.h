/*
 * tuya_data_handler.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_DATA_HANDLER_H_
#define TUYA_DATA_HANDLER_H_

#include "../../app/include/ty_broad.h"


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

void ty_air_proc(u8 len,u8 *buf);

#ifdef TY_SELF_MODE
void self_cmd_proc(klv_node_s *list);
void weishida_uart_protocol_proc(u8 len,u8 *pData);
#endif

#ifdef TY_SPP_MODE
void ty_uart_protocol_proc(u8 len,u8 *pData);
void spp_cmd_proc(klv_node_s *list);
#endif


#endif /* TUYA_DATA_HANDLER_H_ */
