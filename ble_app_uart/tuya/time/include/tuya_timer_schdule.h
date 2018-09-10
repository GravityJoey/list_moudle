/*
 * tuya_timer_schdule.h
 *  Created on: 2018-4-27
 *      Author: louxu
 */
#include "tuya_time.h"
#include "../../module/include/tuya_data_handler.h"



int timer_action_manu(UINT16 dp_data_len, BYTE *dp_data);

int timer_action_run(pLINK_SS prt);

void timer_action_schdule();

