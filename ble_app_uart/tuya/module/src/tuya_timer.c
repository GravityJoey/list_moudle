/*
 * tuya_timer.c
 *  Created on: 2017-9-6
 *      Author: echo
 */
#include "../include/tuya_timer.h"

static t_timer_tlm timer_tlm[TIMER_ID_MAX];

///////////////////////////timer config/////////////////////////////////////
void ty_timer_start(u8 timer_id,u32 time_ms_cnt)
{
    if(timer_id >= TIMER_ID_MAX)
        return;
    timer_tlm[timer_id].timer_enable = ENABLE;
    timer_tlm[timer_id].timer_set_cnt = time_ms_cnt;
    timer_tlm[timer_id].timer_cnt = 0;
    timer_tlm[timer_id].timer_flag = DISABLE;
    return;
}

void ty_timer_stop(u8 timer_id)
{
    if(timer_id >= TIMER_ID_MAX)
        return;
    timer_tlm[timer_id].timer_enable = DISABLE;
    timer_tlm[timer_id].timer_flag = DISABLE;
    return;
}
void ty_timer_stop_all(void)
{
    u8 i = 0;

    for(i = 0;i < TIMER_ID_MAX;i++)
        ty_timer_stop(i);
}

void ty_timer_loop(void)
{
    u8 i = 0;
    int ret = 0;

    for(i = 0;i < TIMER_ID_MAX;i++){
        if(ENABLE == timer_tlm[i].timer_flag){
            timer_tlm[i].timer_flag = DISABLE;
            ret = ty_timer_proc(i);
            if(-1 == ret){
                timer_tlm[i].timer_enable = DISABLE;
            }
            else if(0 == ret){
                timer_tlm[i].timer_enable = ENABLE;
            }
            else{
                timer_tlm[i].timer_enable = ENABLE;
                timer_tlm[i].timer_set_cnt = ret;
            }
            timer_tlm[i].timer_cnt  = 0;
            return;
        }
    }
}

void ty_timer_update(void)
{
    u8 i = 0;

    for(i = 0;i < TIMER_ID_MAX;i++){
        if(ENABLE == timer_tlm[i].timer_enable){
            if((timer_tlm[i].timer_cnt += TUYA_TIMER_INTEVAL_MS) >= timer_tlm[i].timer_set_cnt){
                timer_tlm[i].timer_flag = ENABLE;
                timer_tlm[i].timer_enable = DISABLE;
            }
        }
    }
}
