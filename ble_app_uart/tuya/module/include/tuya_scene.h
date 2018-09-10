/*
 * tuya_sence.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_SCENE_H_
#define TUYA_SCENE_H_

#include "../../app/include/ty_broad.h"

#ifdef TY_SELF_MODE

enum{
    up = 0,
    down,
    un_change,
    up_down_default
};

extern led_state_s led_state;

int updata_scene_time(void);
int scene4_up_time(void);
int change_scene_rgb(u8 num,u8 i);

#endif

#endif /* TUYA_SCENE_H_ */
