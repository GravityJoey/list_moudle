/*
 * tuya_timer_schdule.c
 *  Created on: 2018-4-27
 *      Author: louxu
 */

#include "../include/tuya_timer_schdule.h"
#include "weishida_device_api.h"

pLINK_S date_action_list = NULL;
extern week_time_t tuya_time;

u8 dp_action_decode(pLINK_S link_ptr, BYTE *dp_data)
{
    link_ptr->data.en_status = dp_data[1] - 0x30;
    printf("en_status:%d\n", link_ptr->data.en_status);

    link_ptr->data.action = dp_data[3] - 0x30;
    printf("action_dp_act:%d\n", link_ptr->data.action);

    for(int i = 4; i < 10; i++) {
        if(isdigit(dp_data[i])) {
            dp_data[i] = dp_data[i] - 0x30;
        }
        else if(islower(dp_data[i])) {
            dp_data[i] = dp_data[i] - 0x61 + 10;
        }
        else if(isuper(dp_data[i])) {
            dp_data[i] = dp_data[i] - 0x41 + 10;
        }
    }
    link_ptr->data.loop = dp_data[4]*16 + dp_data[5];
    printf("action_loop:%x\n", link_ptr->data.loop);

    link_ptr->data.local_time = ((dp_data[6]*16+dp_data[7])*256) + (dp_data[8]*16+dp_data[9]);
    printf("action_time: %d\n", link_ptr->data.local_time);

    return 0;
}

BOOL time_earlier(pLINK_S a, pLINK_S b)
{
    if(a->data.local_time < b->data.local_time)
        return 1;
    else 
        return 0;
}

pLINK_S timer_action_pop(pLINK_SS ptr)
{
    pLINK_S ptr_t = NULL;
    ptr_t = *ptr;
    *ptr = (*ptr)->next;

    return ptr_t;
}

void timer_action_add(pLINK_SS list_head, pLINK_S item)
{
    pLINK_S ptr = *list_head;
    pLINK_S prev = NULL;

    item->next = NULL;

    if(ptr == NULL) {
        ptr = item;
        *list_head = ptr;
        printf("add the first action :%x\n", item);
    }
    else {
        for( ; ; ) {
            if(time_earlier(item, ptr)) {
                item->next = ptr;
                ptr = item;
                if(prev != NULL) {
                    prev->next = ptr;
                }
                else {
                ptr->next = *list_head;
                *list_head = ptr;
                }
                printf("add the action1 :%x\n", item);
                break;
             }
             else {
                if(ptr->next == NULL) {
                    ptr ->next = item;
                    printf("add the action2: %x\n", item);
                    break;
                }
                else {
                    prev = ptr;
                    ptr = ptr->next;
                }
             }
         }
     }
}

int timer_action_delete_all(pLINK_SS list_head)
{
    pLINK_S ptr = *list_head;
    pLINK_S ptr_t = NULL;

    for(;ptr != NULL;) {
        ptr_t = ptr;
        ptr = ptr->next;
        ty_free(ptr_t);
    }
    *list_head = NULL;

    return 0;
}

/*
int timer_action_delete(pLINK_SS list_head, pLINK_S item)
{
    pLINK_S ptr = *list_head;
    pLINK_S prev = NULL;
    pLINK_S ptr_t = NULL;

    if(ptr == NULL) {
        printf("can not find the action1\n");
        return -1;
    }
    else {
        for(;;) {
            if(memcmp(&(ptr->data), &(item->data), sizeof(DP_ACTION)) == 0) {
                if(prev == NULL) {
                    ptr_t = ptr;
                    ptr = ptr->next;
                    *list_head = ptr;
                    ty_free(ptr_t);
                    printf("delete the action1:%x\n", item);
                    return 0;
                }
                else {
                    prev->next = ptr->next;
                    ty_free(ptr);
                    printf("delete the action2: %x\n", item);
                    return 0;
                }
            } 
            else {
                if(ptr->next == NULL) {
                    printf("can not find the action2\n");
                    return -1;
                }
                else {
                    prev = ptr;
                    ptr = ptr->next;
                }
            }
        }
    }
    return -1;
}*/

int timer_action_run(pLINK_SS prt)
{
    pLINK_S prt_run = *prt;
    u8 act = prt_run->data.action - 1;

    curtain_run(NULL, &act);

    return 0;
}

u8 dp_action_arry[][10] = {0};
u8 timing_cnt = 0;

int timer_action_manu(UINT16 dp_data_len, BYTE *dp_data)
{
    if(dp_data_len % 10 == 0) {
        timing_cnt = dp_data_len / 10;
        printf("timing_cnt:%d\n", timing_cnt);
    }
    else {
        printf("timing data error\n");
        return -1;
    }

    timer_action_delete_all(&date_action_list);

    for(int i = 0; i < timing_cnt; i++) {
        for(int j = 0; j < 10; j++) {
            dp_action_arry[i][j] = dp_data[j];
        }
        pLINK_S ptr = ty_malloc(sizeof(LINK_S));
        ptr->next = NULL;

        ptr->data.action_id = i;
        dp_action_decode(ptr, dp_data);

        timer_action_add(&date_action_list, ptr);

        dp_data += 10;
    }

    printf("timing data end\n");

    return 0;
}

void timer_action_schdule()
{
    u32 ret = 0;
    pLINK_S ptr = date_action_list;

    for(;ptr != NULL;) {
        if(tuya_time.time_min == ptr->data.local_time
            && ptr->data.en_status == 1) {
            if(ptr->data.loop != 0) {
                if((WEEK_BIT(tuya_time.week) & ptr->data.loop) != 0) {
                     printf("timer_action1_1\n");
                    ret = timer_action_run(&ptr);
                }
            }
            else {
                printf("timer_action_2\n");
                ret = timer_action_run(&ptr);
                ptr->data.en_status = 0;
                dp_action_arry[ptr->data.action_id][1] = ptr->data.en_status + 0x30;
                tuya_timer_report(TIMER_REPORT_ID);
            }
        }
        ptr = ptr->next;
    }
}




