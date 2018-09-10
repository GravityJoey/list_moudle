/*
 * tuya_led.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_LED_H_
#define TUYA_LED_H_

#include "../../app/include/ty_broad.h"

#include "../include/tuya_scene.h"

#ifdef TY_SELF_MODE

extern u8 scene_n_rgb[5];
extern u8 scene_n_rgb_time[5];
extern u8 scene_cnt;
extern u8 up_down;
extern u8 cur_sene_num;
extern u8 d_value_rgb[5];
extern u8 rgb_up_down_flag[5];

extern r_air_recv_packet air_recv_packet;

extern frm_trsmitr_proc_s ty_trsmitr_proc;

int led_timer_proc(void);
void save_led_data(void);
void load_led_data(void);
void report_state(u8 flag);

#endif

#endif /* TUYA_LED_H_ */
