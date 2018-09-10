

#include "../include/ty_broad.h"
#include "../../module/include/tuya_ota.h"
#include "../../module/include/tuya_event.h"
#include "../../module/include/tuya_factory_test.h"
#include "../../port/include/port.h"
#include "ble_gap.h"

extern r_air_recv_packet air_recv_packet;
extern u8 ty_factory_flag;
extern u8 ty_ble_state;
#ifdef TY_SELF_MODE
extern led_state_s led_state;
#endif
#ifdef TY_OTA_ENABLE
extern u8 ota_status;
#endif
extern int led_timer_proc(void);
extern void ty_air_send(u8 len,u8 *buf,u8 type);
extern void tuya_erase_new_firmware_bank(void);

u8 ty_led_flag = 0;

#define  OTA_LED_INDEX      PWMID_WW
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void ty_user_init(void)
{
	
	//ty_pwm_all_init();
#ifdef TY_SELF_MODE  
    ty_pwm_init(PWM_R,PWMID_R,0);
    ty_pwm_init(PWM_G,PWMID_G,0);
    ty_pwm_init(PWM_B,PWMID_B,1);
    ty_pwm_init(PWM_WW,PWMID_WW,0); 
#ifdef WEIZHI_BLE_MODULE
	ty_pwm_init(PWM_CW,PWMID_CW,0); 
#endif
    led_state.led_switch = 1;
    led_state.work_mode = WHILTE_MODE;
    led_state.rgb_value[R_POS] = 0;
    led_state.rgb_value[G_POS] = 0;
    led_state.rgb_value[B_POS] = 0;
    led_state.rgb_value[W_POS] = 0;
    led_state.bright_value = 0;
    ty_set_pwm(PWMID_WW,led_state.rgb_value[W_POS]);
    ty_set_pwm(PWMID_R,led_state.rgb_value[R_POS]);
    ty_set_pwm(PWMID_G,led_state.rgb_value[G_POS]);
    ty_set_pwm(PWMID_B,led_state.rgb_value[B_POS]); 
#ifdef WEIZHI_BLE_MODULE
	ty_set_pwm(PWMID_CW,0);
#endif
#endif

#ifdef TY_SPP_MODE
    ty_pwm_init(PWM_WW,PWMID_WW,0);
#endif
    /*ty_gpio_init(PWM_G,OUT_PUT,0);
    ty_gpio_out_level(PWM_G,OUTPUT_HIGH);
    ty_gpio_init(GPIO_PD5,IN_PUT,PULLUP_10K);
    ty_gpio_init(GPIO_PD4,IN_PUT,PULLUP_10K);*/
    ty_hw_timer_init();
    ty_para_init();
    ty_timer_start(TIMER_FIRST,500);  

    //ty_uart_send(5,"12345");
	
	tuya_user_app_init();
	
	//TODO echo修改为正式
	tuya_start_factory_test_scan(FACTORY_TEST);
}
	
void ty_uart_proc(u8 len,u8 *buf)
{	
    if(ty_factory_flag){
        ty_factory_test(len,buf);
    }
    else{
#ifdef TY_SPP_MODE
        ty_uart_protocol_proc(len, buf);
#else ifdef TY_SELF_MODE
        weishida_uart_protocol_proc(len, buf);
#endif
    }
    return;
}

void ty_ble_proc(u8 len,u8 *buf)
{
    ty_air_proc(len,buf);
    return;
}

void ty_ota_proc(u8 len,u8 *buf)
{
    ty_air_proc_ota(len,buf);
    return;
}
//////return -1->stop 0->loop other->timer
int ty_timer_proc(u8 timer_id)
{
    int ret = 0;
    u8 temp = 0;
    
    switch(timer_id)
    {
        case TIMER_FIRST:
			printf("TIMER_FIRST\r\n");
            ret = ty_first_timer_proc();
            return ret;
        break;

        case TIMER_LED_INDEX:
            if(NORMAL_STATUS == ota_status){
                if(ty_ble_state == BONDING_UNCONN)  return -1;
                if(ty_led_flag){
                    ty_led_flag = 0;
                    ty_set_pwm(PWMID_WW,0);
                }
                else{
                    ty_set_pwm(PWMID_WW,30);
                    ty_led_flag = 1;
                }
            }
            else if(OTA_STARTING == ota_status){
                if(ty_led_flag){
                    ty_led_flag = 0;
                    ty_set_pwm(OTA_LED_INDEX,0);
                    return 1900;
                }
                else{
                    ty_set_pwm(OTA_LED_INDEX,30);
                    ty_led_flag = 1;
                    return 100;
                }
            }
            else if(OTA_END == ota_status){
                ota_status = NORMAL_STATUS;
                ty_ble_discon();
                return  -1;
            }
            else if(OTA_ERROR == ota_status){
                ota_status = NORMAL_STATUS;
#ifdef TY_SELF_MODE
                ty_timer_start(TIMER_RGB_INDX,6);
#endif
                tuya_erase_new_firmware_bank();
                return  -1;
            }
        break;

        case TIMER_REPORT_STATE:
            temp = ty_report_mux_packet(FRM_STAT_REPORT);
            if(0 == temp){
                return -1;
            }
            else
            {
                return 0;
            } 
            //ty_air_send(air_recv_packet.de_encrypt_buf[0],&air_recv_packet.de_encrypt_buf[1],FRM_STAT_ACK);
        break;

        case TIMER_CHECK_STATE:
            temp = ty_report_mux_packet(FRM_ALL_DP_QUERY_RESP);
            if(0 == temp){
                return -1;
            }
            else
            {
                return 0;
            } 
            //ty_air_send(air_recv_packet.de_encrypt_buf[0],&air_recv_packet.de_encrypt_buf[1],FRM_ALL_DP_QUERY_RESP);
            //return -1;
        break;
                   
#ifdef TY_SELF_MODE
        case TIMER_RGB_INDX:
            ret = led_timer_proc();
            return ret;
        break;

        case TIMER_SAVE_LED_PARA:
            save_led_data();
            return -1;
        break;
#endif  

#ifdef TY_OTA_ENABLE
        case TIMER_OTA_TIMEOUT:
           ty_ble_discon(); 
           return -1;
        break;
#endif 

#ifdef TY_SPP_MODE
        case TIMER_MCU_HEART:
            ty_uart_protocol_send(TY_HEART_MSG_TYPE,NULL,0);
            return 3000;
        break;
		case TIMER_QUERY:
			ty_uart_protocol_send(QUERY_STATUS,NULL,0); 
			return -1;
		break;
#endif
		case TIMER_REBOOT:
        	ty_ble_discon();
        	return -1;
        break;
		case TIMER_SCAN:
			ty_ble_scan_stop();
		
			//test time out
			scan_type_t type = tuya_factory_test_scan_type_get();
		
			if(type == FACTORY_TEST)
			{
				tuya_factory_test_cb_fun cb = tuya_factory_test_get_cb();
				if(cb != NULL)
				{
					cb(0,0);
				}
			}
			else if(type == AUTH_RF_TEST)
			{
				//auth rf test fail
				tuya_factory_test_auth_rf_resp(0,NULL);
			}
			
			return -1;
		
			break;
		case TIMER_SCAN_TEST:

			ty_timer_stop(TIMER_SCAN);
			ty_ble_scan_stop();
		
			s8 rssi = tuya_auth_rf_test_calc_avg();
			tuya_factory_test_auth_rf_resp(1,rssi);
		
			tuya_auth_rf_test_begin_stats_flag_set(FALSE);
		
			return -1;

			break;
        default:
            break;
    }
    return 0;
}

void ty_ble_event_callback(u16 evt)
{
	switch (evt)
    {
        case BLE_GAP_EVT_DISCONNECTED:
			//tuya_porting
			ty_ev_terminate_proc();
		break;
	}
}

void ty_main_loop(void)
{
    ty_timer_loop();
	
	tuya_event_process();
}


