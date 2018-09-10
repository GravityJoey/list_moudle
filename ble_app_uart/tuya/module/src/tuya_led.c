/*
 * tuya_led.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#include "../include/tuya_led.h"

#ifdef TY_SELF_MODE

led_state_s led_state;

int led_timer_proc(void)
{
    u8 i = 0;
    int time_ret = 0;

    scene_cnt++;
    if(0 != led_state.led_switch){
        if(led_state.work_mode != led_state.working_mode){
            led_state.working_mode = led_state.work_mode;
            switch(led_state.work_mode){
                case WHILTE_MODE:
                    led_state.rgb_value[R_POS] = 0;
                    led_state.rgb_value[G_POS] = 0;
                    led_state.rgb_value[B_POS] = 0;
                    led_state.rgb_value[W_POS] = (u8)led_state.bright_value;
                    ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
                    ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
                    ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]);
                    ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
                break;

                case COLOR_MODE:
                case SCENE_MODE:
                    led_state.rgb_value[R_POS] = led_state.scene_data[0][0];
                    led_state.rgb_value[G_POS] = led_state.scene_data[0][1];
                    led_state.rgb_value[B_POS] = led_state.scene_data[0][2];
                    led_state.rgb_value[W_POS] = 0;
                    scene_n_rgb[R_POS] = led_state.scene_data[0][0];
                    scene_n_rgb[G_POS] = led_state.scene_data[0][1];
                    scene_n_rgb[B_POS] = led_state.scene_data[0][2];
                    ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
                    ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
                    ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]);
                    ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
                break;

                case SCENE1_MODE:
                case SCENE2_MODE:
                case SCENE3_MODE:
                case SCENE4_MODE:
                    if(SCENE1_MODE == led_state.work_mode){
                        up_down = up;
                    }
                    scene_cnt = 0;
                    cur_sene_num = 0;
                    led_state.rgb_value[R_POS] = led_state.scene_data[
                    cur_sene_num][0];
                    led_state.rgb_value[G_POS] = led_state.scene_data[
                    cur_sene_num][1];
                    led_state.rgb_value[B_POS] = led_state.scene_data[
                    cur_sene_num][2];
                    led_state.rgb_value[W_POS] = 0;
                    if(SCENE4_MODE == led_state.work_mode){
                        cur_sene_num++;
                        time_ret = scene4_up_time();
                    }
                    else{
                        time_ret = updata_scene_time();
                        //time_ret = (5 + (16*led_state.scene_temp[FREQ_POS]/100))*500;
                    }
                    ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
                    ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
                    ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]);
                    ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
                    return time_ret;
                break;
                default:
                break;
            }
        }
        else{
            switch(led_state.work_mode){
                case WHILTE_MODE:
                    led_state.rgb_value[R_POS] = 0;
                    led_state.rgb_value[G_POS] = 0;
                    led_state.rgb_value[B_POS] = 0;
                    led_state.rgb_value[W_POS] = (u8)led_state.bright_value;;
                    ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
                    ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
                    ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]);
                    ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
                break;

                case COLOR_MODE:
                case SCENE_MODE:
                    led_state.rgb_value[R_POS] = led_state.scene_data[0][0];
                    led_state.rgb_value[G_POS] = led_state.scene_data[0][1];
                    led_state.rgb_value[B_POS] = led_state.scene_data[0][2];
                    led_state.rgb_value[W_POS] = 0;
                    ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
                    ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
                    ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]);
                    ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
                break;

                case SCENE1_MODE:
                case SCENE2_MODE:
                case SCENE3_MODE:
                case SCENE4_MODE:
                    if(0 == led_state.scene_data_change_flag){
                        for(i = 0;i<3;i++){
                            if(scene_n_rgb_time[i] == 0){
                                if(SCENE4_MODE !=led_state.work_mode){
                                    scene_n_rgb[i] = 0;
                                }
                            }
                            else{
                                if((scene_cnt% scene_n_rgb_time[i]) == 0){
                                    if(SCENE4_MODE !=led_state.work_mode){
                                        if(change_scene_rgb(led_state.work_mode,i)){
                                            scene_cnt = 0;
                                            break;
                                        }
                                    }
                                    else{
                                        time_ret = change_scene_rgb(led_state.work_mode,i);
                                        if(0 != time_ret){
                                            scene_cnt = 0;
                                            ty_set_pwm(PWMID_R,scene_n_rgb[R_POS]);
                                            ty_set_pwm(PWMID_G,scene_n_rgb[G_POS]);
                                            ty_set_pwm(PWMID_B,scene_n_rgb[B_POS]);
                                            ty_set_pwm(PWMID_WW,0);
                                            return time_ret;
                                        }
                                    }
                                }
                            }
                        }
                        ty_set_pwm(PWMID_R,scene_n_rgb[R_POS]);
                        ty_set_pwm(PWMID_G,scene_n_rgb[G_POS]);
                        ty_set_pwm(PWMID_B,scene_n_rgb[B_POS]);
                        ty_set_pwm(PWMID_WW,0);
                    }
                    else{
                        led_state.scene_data_change_flag = 0;
                        if(SCENE_MODE == led_state.work_mode){
                            up_down = up;
                        }
                        scene_cnt = 0;
                        led_state.rgb_value[R_POS] = led_state.scene_data[0][0];
                        led_state.rgb_value[G_POS] = led_state.scene_data[0][1];
                        led_state.rgb_value[B_POS] = led_state.scene_data[0][2];
                        led_state.rgb_value[W_POS] = 0;
                        if(SCENE4_MODE == led_state.work_mode){
                            time_ret = scene4_up_time();
                        }
                        else{
                            time_ret = updata_scene_time();
                            //time_ret = (5 + (16*led_state.scene_temp[FREQ_POS]/100))*500;
                        }
                        ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
                        ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
                        ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]);
                        ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
                    }
                    return time_ret;
                break;

                default:
                break;
            }
        }
    }
    return 0;
}

void save_led_data(void)
{
    u8 *p_buf = NULL,i = 0,j = 0;

    p_buf = ty_malloc(110);
    if(p_buf == NULL)   return;

    p_buf[0] = led_state.led_switch;
    p_buf[1] = led_state.work_mode;
    memcpy(&p_buf[2],led_state.rgb_value,4);
    p_buf[6] = (((led_state.bright_value)>>0)&0xff);
    p_buf[7] = (((led_state.bright_value)>>8)&0xff);
    p_buf[8] = (((led_state.bright_value)>>16)&0xff);
    p_buf[9] = (((led_state.bright_value)>>24)&0xff);

    p_buf[10] = (((led_state.temp_value)>>0)&0xff);
    p_buf[11] = (((led_state.temp_value)>>8)&0xff);
    p_buf[12] = (((led_state.temp_value)>>16)&0xff);
    p_buf[13] = (((led_state.temp_value)>>24)&0xff);

    memcpy(&p_buf[14],led_state.scene_temp,4);
    for(;i < 6;i++,j+=3){
        memcpy(&p_buf[18+j],&led_state.scene_data[i],3);
    }
    memcpy(&p_buf[18+j],led_state.clor_data,44);
    p_buf[62+j] = led_state.clor_len;
    memcpy(&p_buf[63+j],led_state.report_flg,END_ID_NUM - 1);
    p_buf[109] =  0x01;
    ty_flash_write(NV_USER_MODULE2,USER_LED_PARA_ID, p_buf, 110);
    ty_free(p_buf);
}

void load_led_data(void)
{
    u8 *p_buf = NULL,i = 0,j = 0;

    p_buf = ty_malloc(110);
    if(p_buf == NULL)   return;
    memset(p_buf,0xff,110);
    ty_flash_read(NV_USER_MODULE2,USER_LED_PARA_ID, p_buf, 110);
    if(p_buf[109] == 0xff){
        led_state.led_switch = 1;
        led_state.work_mode = WHILTE_MODE;
        led_state.rgb_value[R_POS] = 0;
        led_state.rgb_value[G_POS] = 0;
        led_state.rgb_value[B_POS] = 0;
        led_state.rgb_value[W_POS] = 20;
        led_state.bright_value = 20;
        ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
        ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
        ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
        ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]);
        ty_free(p_buf);
        return;
    }
    led_state.led_switch = p_buf[0];
    led_state.work_mode = p_buf[1];
    memcpy(led_state.rgb_value,&p_buf[2],4);
    led_state.bright_value = p_buf[6];
    led_state.bright_value += (p_buf[7]<<8);
    led_state.bright_value += (p_buf[8]<<16);
    led_state.bright_value += (p_buf[9]<<24);

    led_state.temp_value = p_buf[10];
    led_state.temp_value += (p_buf[11]<<8);
    led_state.temp_value += (p_buf[12]<<16);
    led_state.temp_value += (p_buf[13]<<24);
    memcpy(led_state.scene_temp,&p_buf[14],4);
    for(;i < 6;i++,j+=3){
        memcpy(&led_state.scene_data[i],&p_buf[18+j],3);
    }
    memcpy(led_state.clor_data,&p_buf[18+j],44);
    led_state.clor_len = p_buf[62+j];
    memcpy(led_state.report_flg,&p_buf[63+j],END_ID_NUM - 1);
    ty_free(p_buf);
}

void report_state(u8 flag)
{
    klv_node_s *list = NULL;
    u8 ret = 0,send_flg = 0;
    u32 temp_int = 0;

    if(1 == flag){
        for(ret = 0;ret<4;ret++){
            led_state.report_flg[ret] = 1;
        }
        ret = 0;
    }

    if(1 == led_state.report_flg[LED_SWITCH_ID -1])
    {
		//extern int switch_on_off_flag;
        list =  make_klv_list(list,LED_SWITCH_ID,DT_BOOL,&led_state.led_switch,DT_BOOL_LEN);
        if(NULL == list) {
           return;
        }
		send_flg = 1;
    }

    if(1 == led_state.report_flg[WORK_MODE_ID -1])
    {
        list = make_klv_list(list,WORK_MODE_ID,DT_ENUM,&led_state.work_mode,DT_ENUM_LEN);
        if(NULL == list) {
            return;
        }
		send_flg = 1;
    }

    if(1 == led_state.report_flg[BRIGHT_ID -1])
    {
        temp_int = led_state.bright_value;
        list = make_klv_list(list,BRIGHT_ID,DT_VALUE,&temp_int,DT_VALUE_LEN);
        if(NULL == list) {
            return;
        }
        send_flg = 1;
    }

    if(1 == led_state.report_flg[TEMP_ID-1])
    {
        temp_int = led_state.temp_value;
        list =  make_klv_list(list,TEMP_ID,DT_VALUE,&temp_int,DT_VALUE_LEN);
        if(NULL == list) {
            return;
        }
        send_flg = 1;
    }

    if(1 == led_state.report_flg[CLOR_ID-1])
    {
        list =  make_klv_list(list,CLOR_ID,DT_STRING,led_state.clor_data,led_state.clor_len);
        if(NULL == list) {
            return;
        }
        send_flg = 1;
    }

    if(1 == led_state.report_flg[SCENE_NUM_ID-1])
    {
        list =  make_klv_list(list,SCENE_NUM_ID,DT_STRING,led_state.clor_data,  led_state.clor_len);
        if(NULL == list) {
            return;
        }
        send_flg = 1;
    }

    if(1 == led_state.report_flg[ROUGUANG_SCENE_ID-1])
    {
        list =  make_klv_list(list,ROUGUANG_SCENE_ID,DT_STRING,led_state.clor_data,led_state.clor_len);
        if(NULL == list) {
            return;
        }
        send_flg = 1;
    }

    if(1 == led_state.report_flg[BINFENG_SCENE_ID-1])
    {
        list = make_klv_list(list,BINFENG_SCENE_ID,DT_STRING,led_state.clor_data,led_state.clor_len);
        if(NULL == list) {
            return;
        }
        send_flg = 1;
    }

    if(1 == led_state.report_flg[XUANCAI_SCENE_ID-1])
    {
        list =  make_klv_list(list,XUANCAI_SCENE_ID,DT_STRING,led_state.clor_data,led_state.clor_len);
        if(NULL == list) {
            return;
        }
        send_flg = 1;
    }

    if(1 == led_state.report_flg[BANLAN_SCENE_ID-1])
    {
        list =  make_klv_list(list,BANLAN_SCENE_ID,DT_STRING,led_state.clor_data,led_state.clor_len);
        if(NULL == list) {
           return;
        }
        send_flg = 1;
    }

    if(1 == send_flg){
        unsigned char *data;
        unsigned int da_len;
        ret = klvlist_2_data(list,&data,&da_len,0);
        if(MTP_OK != ret) {
			free_klv_list(list);
	        return;
        }
	    air_recv_packet.recv_len = da_len;
        memcpy(air_recv_packet.recv_data,data,da_len);
        ty_aes_encrypt(air_recv_packet.recv_data,da_len,air_recv_packet.de_encrypt_buf);
        trsmitr_init(&ty_trsmitr_proc);
        if(0 == flag){
            ty_timer_start(TIMER_REPORT_STATE,16);
            ty_timer_start(TIMER_SAVE_LED_PARA,5000);
        }
        else if(1 == flag){
            ty_timer_start(TIMER_CHECK_STATE,16);
        }
		free_klv_list(list);
	    ty_free(data);
    }
}
#endif
