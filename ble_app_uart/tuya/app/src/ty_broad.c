#include "../include/ty_broad.h"
#include "../../port/include/port.h"

extern u8 ota_status;

u8 p_id[P_ID_LEN] = TY_DEVICE_PID;

#ifdef TY_SPP_MODE
static u8 ty_devicename[] = "TY_Spp";
#endif
#ifdef TY_SELF_MODE
static u8 ty_devicename[] = "TY_Curtain";
#endif

u8 tbl_scanRsp[28] = {0x1B,0xFF,0x02,0x59};
u8 pair_rand[6]={0};

u8 ty_factory_flag = 0;

r_air_recv_packet air_recv_packet;

frm_trsmitr_proc_s ty_trsmitr_proc;

b_ble_para ble_para;

u8 ty_ble_state = UNKNOW_STATE;

u8 ty_ble_send_flag = 0;

void ty_ev_terminate_proc(void)
{
    ty_timer_stop_all();
    if(ENABLE == ble_para.pass_key_flag){
        ty_ble_state = BONDING_UNCONN;                   
        //ty_timer_start(TIMER_LED_INDEX,500);
    }
    else{
        ty_ble_state = UNBONDING;
        ty_timer_start(TIMER_LED_INDEX,100);
    }
#ifdef TY_SELF_MODE
    ty_timer_start(TIMER_RGB_INDX,6);
#endif
    if(ota_status != NORMAL_STATUS) ota_status = NORMAL_STATUS;
}

void ty_para_init(void)
{
    u8 *p_buf = NULL;
    u8 index = 0;
	
    p_buf = ty_malloc(30);   
    ty_flash_read(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, p_buf, 7);
    memcpy(ble_para.pass_key,p_buf,PASS_KEY_LEN);
    ble_para.pass_key_flag = p_buf[6];
    ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_D_ID, ble_para.d_id, D_ID_LEN);
    ty_flash_read(NV_USER_MODULE1,NV_USER_ITEM_RESET_CNT, &ble_para.reset_cnt, 1);
	
	u8 tp_buf[64]={0};
	ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_H_ID, tp_buf, H_ID_LEN);
	printf("tp_buf:%s\r\n",tp_buf);
	
	ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_AUZ_KEY, tp_buf, AUTH_KEY_LEN);
	printf("tp_buf:%s\r\n",tp_buf);
	
	//ty_uart_send(AUTH_KEY_LEN,tp_buf);
	
    if((ble_para.reset_cnt++) >= 3){
        ble_para.pass_key_flag = 0;
        ble_para.reset_cnt = 0;
        memcpy(p_buf,ble_para.pass_key,PASS_KEY_LEN);
        p_buf[6] = ble_para.pass_key_flag;
        ty_flash_write(NV_USER_MODULE3,NV_USER_ITEM_PASS_KEY, p_buf, 7);
        ty_flash_write(NV_USER_MODULE1,NV_USER_ITEM_RESET_CNT, &ble_para.reset_cnt, 1);
    }

    if(ENABLE == ble_para.pass_key_flag){
        ty_ble_state = BONDING_UNCONN;
        //ty_timer_start(TIMER_LED_INDEX,700);
    }
    else{
        ty_ble_state = UNBONDING;
        ty_timer_start(TIMER_LED_INDEX,100);
    }  
    p_buf[0] = 0x02;
    p_buf[1] = 0x01;
    p_buf[2] = 0x05;
    p_buf[3] = 0x03;
    p_buf[4] = 0x02;
    p_buf[5] = 0x01;
    p_buf[6] = 0xA2;

    ty_set_adv_data(7,p_buf);

    ty_free(p_buf);
    if(ENABLE == ble_para.pass_key_flag)
    {
        tbl_scanRsp[2] |= 0x80;
    }
    else
    {
        tbl_scanRsp[2] &= 0x7F;
    }
    index = 4;
    memcpy(&tbl_scanRsp[index],p_id,P_ID_LEN);
    index += P_ID_LEN;
    memcpy(&tbl_scanRsp[index],ble_para.d_id,D_ID_LEN);
    index += D_ID_LEN;
	#ifdef TY_SELF_MODE

    ty_set_scanrsp_data(sizeof(tbl_scanRsp),tbl_scanRsp);

    #endif

    ty_set_adv_param(30*1000/625);

    ty_factory_flag = 1;

#ifdef TY_SPP_MODE
    ty_timer_start(TIMER_MCU_HEART,800);
#endif
    ty_set_device_name(sizeof(ty_devicename),ty_devicename);
	
	ty_set_adv_enable(1);
}

u8 factory_test_reset = 0;
int ty_first_timer_proc(void)
{
    if(factory_test_reset == 1)
        ty_ble_discon();
                
    if(1 == ty_factory_flag){
        ty_factory_flag = 0;
        ty_flash_write(NV_USER_MODULE1,NV_USER_ITEM_RESET_CNT, &ble_para.reset_cnt, 1);
        
#ifdef CURTAIN_LOW_POWER
        wistar_uart_close();
#endif

#ifdef TY_SELF_MODE
        if(UNBONDING != ty_ble_state){
            load_led_data();
            ty_timer_start(TIMER_RGB_INDX,6);
        }
#endif
        return 5000;//start 5s as reset the count
    }
    else{
        ble_para.reset_cnt = 0;
        ty_flash_write(NV_USER_MODULE1,NV_USER_ITEM_RESET_CNT, &ble_para.reset_cnt, 1);
        return -1;
    }
}