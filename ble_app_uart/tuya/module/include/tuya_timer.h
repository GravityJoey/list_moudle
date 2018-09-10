/*
 * tuya_timer.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_TIMER_H_
#define TUYA_TIMER_H_

#include "../../app/include/ty_broad.h"

void ty_timer_start(u8 timer_id,u32 time_ms_cnt);
void ty_timer_stop(u8 timer_id);
void ty_timer_stop_all(void);
void ty_timer_loop(void);
void ty_timer_update(void);

#endif /* TUYA_TIMER_H_ */
