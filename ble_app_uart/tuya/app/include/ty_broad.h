/*
 * ty_broad.h
 *
 *  Created on: 2016-12-30
 *      Author: Tom
 */

#ifndef _TY_BROAD_H_
#define _TY_BROAD_H_

#include "../../utils/include/mutli_tsf_protocol.h"
#include "tuya_config.h"
#include "../../port/include/port.h"
#include "../../../tuya/types.h"
#include "string.h"

#define TY_OTA_ENABLE

//==========================================
//#define TY_SPP_MODE             //uart example
#define TY_SELF_MODE          //pwm led

#define CURTAIN_LOW_POWER

#ifdef TY_SPP_MODE
#define TY_DEVICE_PID			"1r5jpMkT" //TLS8266 1r5jpMkT//(ooo g6yEKoTC)//ZRQ5tzcb
#endif
#ifdef TY_SELF_MODE
#define TY_DEVICE_PID			"ZYoJfwDn" //9UnXzZbO//ZYoJfwDn//nodric single plug FfSOpKQH //YIBO x8z56uzY//kCEU855e
#endif

#define TY_FIRMWARE_NAME		"wistar_curtain"//nordic_52832_wistar_curtain" 	//"ty_ble_nod" 	//"ty_ble_spp"
#define TY_FIRMWARE_VER			"1.1.0" 		//"1.0.2" 		//"1.0.0"

#define SKIP_GPIO_TEST					0

//===========================================

//#define WEIZHI_BLE_MODULE
#define TUYA_BLE_MODULE

#define  PWM_HIGH_ACTIVE

//v1.2
#define soft_ver_high       0x01
#define soft_ver_low        0x02
//v1.0
#define protocol_ver_high   0x01
#define protocol_ver_low    0x00

///////////////////////PWM//////////////////////////////////////
#define P_PWM_MODE       0  //正向
#define N_PWM_MODE       1  //反向
#define PWM_BASE_CLK     2000000 //2MHZ
#define PWM_OUT_CLK      300    //300HZ
#define PWM_BASE        (PWM_BASE_CLK/PWM_OUT_CLK)
#define PWM_CNT_BASE     26//26*(0~255)

#ifdef TUYA_BLE_MODULE

#ifdef TLS8266_PORTING
#define PWM_G            GPIO_PWM3		
#define PWM_B            GPIO_PWM4			
#define PWM_R            GPIO_PWM2	
#define PWM_CW           GPIO_PWM1
#define PWM_WW           GPIO_PWM0			
#define PWMID_G          3			
#define PWMID_B          4			
#define PWMID_R          2	
#define PWMID_CW         1
#define PWMID_WW         0
#endif

#ifdef NRF52832_PORTING
#define PWM_G            GPIO_PWM3		
#define PWM_B            GPIO_PWM4			
#define PWM_R            GPIO_PWM2	
#define PWM_CW           GPIO_PWM1
#define PWM_WW           GPIO_PWM0	
		
#define PWMID_G          3			
#define PWMID_B          4			
#define PWMID_R          2	
#define PWMID_CW         1
#define PWMID_WW         0
#endif
#endif

#ifdef WEIZHI_BLE_MODULE
#define PWM_G            GPIO_PWM4		
#define PWM_B            GPIO_GP1			
#define PWM_R            GPIO_PWM3	
#define PWM_CW           GPIO_PWM2
#define PWM_WW           GPIO_PWM0			
#define PWMID_G          4			
#define PWMID_B          1			
#define PWMID_R          3	
#define PWMID_CW         2
#define PWMID_WW         0
#endif

typedef enum{
    LED_SWITCH_ID = 1,
    WORK_MODE_ID,
    BRIGHT_ID,
    TEMP_ID,
    CLOR_ID,
    SCENE_NUM_ID,
    ROUGUANG_SCENE_ID,
    BINFENG_SCENE_ID,
    XUANCAI_SCENE_ID,
    BANLAN_SCENE_ID,
    END_ID_NUM
}TY_LED_ID;

typedef enum{
    WHILTE_MODE = 0,
    COLOR_MODE,
    SCENE_MODE,
    SCENE1_MODE,
    SCENE2_MODE,
    SCENE3_MODE,
    SCENE4_MODE,
    MAX_MODE_NUM,
}TY_LED_MODE;

typedef enum{
    BRIGHT_POS = 0,
    TEMP_POS,
    FREQ_POS,
    SCENE_NUM_POS,
}SCENE_NUM_POS_E;

typedef enum{
    R_POS = 0,
    G_POS,
    B_POS,
    W_POS,
}COLOR_NUM_POS_E;

typedef struct{
    unsigned char led_switch;
    unsigned char work_mode;
    unsigned char working_mode;
    unsigned char rgb_value[4];
    unsigned int bright_value;
    unsigned int temp_value;
    unsigned char scene_temp[4];/*bright-temp-freq-num*/
    unsigned char scene_data[6][3];
    unsigned char clor_data[44];
    unsigned char clor_len;
    unsigned char report_flg[END_ID_NUM - 1];
    unsigned char scene_data_change_flag;
}led_state_s;
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
#define TICK_BASE_MS           1000
#define IRQ_TIME0_INTERVAL     1000  //1ms

#define AIR_FRAME_MAX    256

#define H_ID_LEN            20
#define P_ID_LEN            8
#define D_ID_LEN            16
#define MAC_LEN             12
#define AUTH_KEY_LEN        32
#define PASS_KEY_LEN        6
#define SESSION_LEN         16
#define RANDOM_LEN          6

#define TY_HEART_MSG_TYPE       0
#define TY_SEARCH_PID_TYPE      1
#define TY_CK_MCU_TYPE          2
#define TY_REPORT_WORK_STATE_TYPE  3
#define TY_REBOOT_TYPE             4
#define TY_SEND_CMD_TYPE           6
#define TY_SEND_STATUS_TYPE        7
#define QUERY_STATUS            8

typedef enum{
    IN_PUT = 0,
    OUT_PUT,
}PIN_MODE;

typedef enum{
    OUTPUT_LOW = 0,
    OUTPUT_HIGH,
}OUTPUT_LEVE;

typedef enum{
    TIMER_FIRST = 0,
    TIMER_LED_INDEX,
#ifdef TY_OTA_ENABLE
    TIMER_OTA_TIMEOUT,
#endif
    TIMER_REPORT_STATE,
    TIMER_CHECK_STATE,
#ifdef TY_SELF_MODE
    TIMER_RGB_INDX,
    TIMER_SAVE_LED_PARA,
#endif
#ifdef TY_SPP_MODE
    TIMER_MCU_HEART,
    TIMER_QUERY,
#endif
	TIMER_REBOOT,
	TIMER_SCAN,
	TIMER_SCAN_TEST,
    TIMER_ID_MAX,
}TIMER_ID_NUM;

typedef enum{
    DISABLE = 0,
    ENABLE,
}TY_FLAG;

typedef enum{
    UNBONDING = 0,
    BONDING_UNCONN,
    BONDING_CONN,
    UNKNOW_STATE
}BLE_STATE;

typedef struct{
    u8 timer_enable;
    u8 timer_flag;
    u32 timer_set_cnt;
    u32 timer_cnt;
}t_timer_tlm;

typedef struct{
    u8 recv_len;
    u8 recv_data[AIR_FRAME_MAX];
    u8 de_encrypt_buf[AIR_FRAME_MAX];
}r_air_recv_packet;

typedef struct{
    u8 session_key[SESSION_LEN];    //16
    u8 d_id[D_ID_LEN];//16
    u8 d_id_flag;//1
    u8 pass_key[PASS_KEY_LEN];//6
    u8 pass_key_flag;//1
    u8 reset_cnt;//1
}b_ble_para;

#ifdef TY_OTA_ENABLE
typedef enum{
    NORMAL_STATUS = 0,
    OTA_STARTING,
    OTA_END,
    OTA_ERROR,
}o_ota_status;
#endif

typedef enum{
    USER_TEST_ID = 0,
#ifdef TY_SELF_MODE
    USER_LED_PARA_ID,
#endif
    USER_PARA_ID,
}u_user_para_id;

void ty_para_init(void);
int ty_first_timer_proc(void);
void ty_ev_terminate_proc(void);

#endif /* _TY_BROAD_H_ */
