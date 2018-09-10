/*
 * tuya_scene.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#include "../include/tuya_scene.h"


#ifdef TY_SELF_MODE

u8 scene_n_rgb[5]={0};
u8 scene_n_rgb_time[5]={0};
u8 scene_cnt = 0;
u8 up_down = up_down_default;
u8 cur_sene_num = 0;
u8 d_value_rgb[5];
u8 rgb_up_down_flag[5];

////ret ms
int updata_scene_time(void)
{
    int ret = 0;

    if(led_state.rgb_value[R_POS] > led_state.rgb_value[G_POS]){
        if(led_state.rgb_value[R_POS] > led_state.rgb_value[B_POS]){
            scene_n_rgb_time[R_POS] = 1;
            if(0 != led_state.rgb_value[G_POS]){
                scene_n_rgb_time[G_POS] = led_state.rgb_value[R_POS]/led_state.rgb_value[G_POS];
            }
            else{
                scene_n_rgb_time[G_POS]= 0;
            }
            if(0 != led_state.rgb_value[B_POS]){
                scene_n_rgb_time[B_POS] = led_state.rgb_value[R_POS]/led_state.rgb_value[B_POS];
            }
            else{
                scene_n_rgb_time[B_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[R_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[R_POS];
        }
        else{
            scene_n_rgb_time[B_POS] = 1;
            if(0 != led_state.rgb_value[G_POS]){
                scene_n_rgb_time[G_POS] = led_state.rgb_value[B_POS]/led_state.rgb_value[G_POS];
            }
            else{
                scene_n_rgb_time[G_POS]= 0;
            }
            if(0 != led_state.rgb_value[R_POS]){
                scene_n_rgb_time[R_POS] = led_state.rgb_value[B_POS]/led_state.rgb_value[R_POS];
            }
            else{
                scene_n_rgb_time[R_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[B_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[B_POS];
        }
    }
    else{
        if(led_state.rgb_value[G_POS] > led_state.rgb_value[B_POS]){
            scene_n_rgb_time[G_POS] = 1;
            if(0 != led_state.rgb_value[B_POS]){
                scene_n_rgb_time[B_POS] = led_state.rgb_value[G_POS]/led_state.rgb_value[B_POS];
            }
            else{
                scene_n_rgb_time[B_POS]= 0;
            }
            if(0 != led_state.rgb_value[R_POS]){
                scene_n_rgb_time[R_POS] = led_state.rgb_value[G_POS]/led_state.rgb_value[R_POS];
            }
            else{
                scene_n_rgb_time[R_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[G_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[G_POS];
        }
        else{
            scene_n_rgb_time[B_POS] = 1;
            if(0 != led_state.rgb_value[G_POS]){
                scene_n_rgb_time[G_POS] = led_state.rgb_value[B_POS]/led_state.rgb_value[G_POS];
            }
            else{
                scene_n_rgb_time[G_POS]= 0;
            }
            if(0 != led_state.rgb_value[R_POS]){
                scene_n_rgb_time[R_POS] = led_state.rgb_value[B_POS]/led_state.rgb_value[R_POS];
            }
            else{
                scene_n_rgb_time[R_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[B_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/led_state.rgb_value[B_POS];
        }
    }
    return ret;
}


int scene4_up_time(void)
{
    u8 i =0;
    int ret = 0;

    for(i = 0;i<3;i++){
        if(led_state.rgb_value[i] > led_state.scene_data[cur_sene_num][i]){
            rgb_up_down_flag[i] = down;
            d_value_rgb[i] = led_state.rgb_value[i] - led_state.scene_data[cur_sene_num][i];
        }
        else if(led_state.rgb_value[i] == led_state.scene_data[cur_sene_num][i]){
            rgb_up_down_flag[i] = un_change;
            d_value_rgb[i] = 0;
        }
        else{
            rgb_up_down_flag[i] = up;
            d_value_rgb[i] = led_state.scene_data[cur_sene_num][i] - led_state.rgb_value[i];
        }
    }

    if(d_value_rgb[R_POS] >= d_value_rgb[G_POS]){
        if(d_value_rgb[R_POS] >= d_value_rgb[B_POS]){
            scene_n_rgb_time[R_POS] = 1;
            if(0 != d_value_rgb[G_POS]){
                scene_n_rgb_time[G_POS] = d_value_rgb[R_POS]/d_value_rgb[G_POS];
            }
            else{
                scene_n_rgb_time[G_POS]= 0;
            }
            if(0 != d_value_rgb[B_POS]){
                scene_n_rgb_time[B_POS] = d_value_rgb[R_POS]/d_value_rgb[B_POS];
            }
            else{
                scene_n_rgb_time[B_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[R_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[R_POS];
        }
        else{
            scene_n_rgb_time[B_POS] = 1;
            if(0 != d_value_rgb[G_POS]){
                scene_n_rgb_time[G_POS] = d_value_rgb[B_POS]/d_value_rgb[G_POS];
            }
            else{
                scene_n_rgb_time[G_POS]= 0;
            }
            if(0 != d_value_rgb[R_POS]){
                scene_n_rgb_time[R_POS] = d_value_rgb[B_POS]/d_value_rgb[R_POS];
            }
            else{
                scene_n_rgb_time[R_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[B_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[B_POS];
        }
    }
    else{
        if(d_value_rgb[G_POS] >= d_value_rgb[B_POS]){
            scene_n_rgb_time[G_POS] = 1;
            if(0 != d_value_rgb[B_POS]){
                scene_n_rgb_time[B_POS] = d_value_rgb[G_POS]/d_value_rgb[B_POS];
            }
            else{
                scene_n_rgb_time[B_POS]= 0;
            }
            if(0 != d_value_rgb[R_POS]){
                scene_n_rgb_time[R_POS] = d_value_rgb[G_POS]/d_value_rgb[R_POS];
            }
            else{
                scene_n_rgb_time[R_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[G_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[G_POS];
        }
        else{
            scene_n_rgb_time[B_POS] = 1;
            if(0 != d_value_rgb[G_POS]){
                scene_n_rgb_time[G_POS] = d_value_rgb[B_POS]/d_value_rgb[G_POS];
            }
            else{
                scene_n_rgb_time[G_POS]= 0;
            }
            if(0 != d_value_rgb[R_POS]){
                scene_n_rgb_time[R_POS] = d_value_rgb[B_POS]/d_value_rgb[R_POS];
            }
            else{
                scene_n_rgb_time[R_POS]= 0;
            }
            //ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[B_POS]*1000;
            ret = (1000 + (20*led_state.scene_temp[FREQ_POS]))/d_value_rgb[B_POS];
        }
    }
    return ret;
}

int change_scene_rgb(u8 num,u8 i)
{
    int ret = 0;

    switch(num){
        case SCENE_MODE:
            if(up == up_down){
                if(scene_n_rgb[i] == 0){
                    up_down = down;
                    scene_n_rgb[R_POS] = 0;
                    scene_n_rgb[G_POS] = 0;
                    scene_n_rgb[B_POS] = 0;
                    return 1;
                }
                else{
                    scene_n_rgb[i]--;
                }
            }
            else{
                if(scene_n_rgb[i] >= led_state.rgb_value[i]){
                    up_down = up;
                    scene_n_rgb[R_POS] = led_state.rgb_value[R_POS];
                    scene_n_rgb[G_POS] = led_state.rgb_value[G_POS];
                    scene_n_rgb[B_POS] = led_state.rgb_value[B_POS];
                    return 1;
                }
                else{
                    scene_n_rgb[i]++;
                }
            }
        break;

        case SCENE3_MODE:
            if(scene_n_rgb[i] == 0){
                scene_n_rgb[R_POS] = led_state.rgb_value[R_POS];
                scene_n_rgb[G_POS] = led_state.rgb_value[G_POS];
                scene_n_rgb[B_POS] = led_state.rgb_value[B_POS];
                return 1;
            }
            else{
                scene_n_rgb[i]--;
            }
        break;

        case SCENE2_MODE:
            if(scene_n_rgb[i] == 0){
                cur_sene_num++;
                if(cur_sene_num >= led_state.scene_temp[SCENE_NUM_POS]){
                    cur_sene_num = 0;
                }
                led_state.rgb_value[R_POS] = led_state.scene_data[cur_sene_num][0];
                led_state.rgb_value[G_POS] = led_state.scene_data[cur_sene_num][1];
                led_state.rgb_value[B_POS] = led_state.scene_data[cur_sene_num][2];
                scene_n_rgb[R_POS] = led_state.rgb_value[R_POS];
                scene_n_rgb[G_POS] = led_state.rgb_value[G_POS];
                scene_n_rgb[B_POS] = led_state.rgb_value[B_POS];
                updata_scene_time();
                return 1;
            }
            else{
                scene_n_rgb[i]--;
            }
        break;

        case SCENE4_MODE:
            if(up == rgb_up_down_flag[i]){
                if(scene_n_rgb[i] >= led_state.scene_data[cur_sene_num][i]){
                    led_state.rgb_value[R_POS] = led_state.scene_data[cur_sene_num][0];
                    led_state.rgb_value[G_POS] = led_state.scene_data[cur_sene_num][1];
                    led_state.rgb_value[B_POS] = led_state.scene_data[cur_sene_num][2];
                    cur_sene_num++;
                    if(cur_sene_num >= led_state.scene_temp[SCENE_NUM_POS]){
                        cur_sene_num = 0;
                    }
                    ret = scene4_up_time();
                    scene_n_rgb[R_POS] = led_state.rgb_value[R_POS];
                    scene_n_rgb[G_POS] = led_state.rgb_value[G_POS];
                    scene_n_rgb[B_POS] = led_state.rgb_value[B_POS];
                    return ret;
                }
                else{
                    scene_n_rgb[i]++;
                }
            }
            else{
                if(scene_n_rgb[i] <= led_state.scene_data[cur_sene_num][i])
                {
                    led_state.rgb_value[R_POS] = led_state.scene_data[
                    cur_sene_num][0];
                    led_state.rgb_value[G_POS] = led_state.scene_data[
                    cur_sene_num][1];
                    led_state.rgb_value[B_POS] = led_state.scene_data[
                    cur_sene_num][2];
                    cur_sene_num++;
                    if(cur_sene_num >= led_state.scene_temp[SCENE_NUM_POS]){
                        cur_sene_num = 0;
                    }
                    ret = scene4_up_time();
                    scene_n_rgb[R_POS] = led_state.rgb_value[R_POS];
                    scene_n_rgb[G_POS] = led_state.rgb_value[G_POS];
                    scene_n_rgb[B_POS] = led_state.rgb_value[B_POS];
                    return ret;
                }
                else{
                    scene_n_rgb[i]--;
                }
            }
        break;

        default:
        break;
    }
    return 0;
}
#endif
