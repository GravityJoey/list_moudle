/*
 * tuya_user_app_api.c
 *
 *  Created on: 2017-9-22
 *      Author: echo
 */
#include "./tuya/app/include/ty_broad.h"
#include "tuya_user_app.h"
#include "tuya_user_app_api.h"
#include "app_timer.h"


#include "nrf_gpio.h"

APP_TIMER_DEF(weishida_resend_timer_id);
 
int switch_on_off_flag = 0;
curtain_run_state_t run_state = 0;
int run_percent = 0;
curtain_border_t border = 0;
bool set_percent = 0;

extern BOOL border_top_limit_set_flg;
extern BOOL border_bottom_limit_set_flg;

void plug_op(BOOL on_off_flag)
{
	if(on_off_flag)
	{
		printf("switch on\r\n");
		switch_on_off_flag = 1;
		//TODO
		nrf_gpio_pin_set(PLUG_IO);
		nrf_gpio_pin_clear(PLUG_INDICATE_LED);
	}
	else
	{
		printf("switch off\r\n");
		switch_on_off_flag = 0;
		//TODO
		nrf_gpio_pin_clear(PLUG_IO);
		nrf_gpio_pin_set(PLUG_INDICATE_LED);
	}
}

void plug_switch(void)
{
	if(switch_on_off_flag)
		plug_op(0);
	else
		plug_op(1);
	tuya_user_cmd_status_report();
}

void handle_switch_1(UINT8 dp_data_len, BYTE *dp_data)
{
	plug_op(dp_data[0]);
}

void weishida_timer_stop()
{
	app_timer_stop(weishida_resend_timer_id);
}

void curtain_run(UINT16 dp_data_len, BYTE *dp_data)
{
	run_state = dp_data[0];
	weishida_device_run(dp_data[0]);
//	app_timer_create(&weishida_resend_timer_id, APP_TIMER_MODE_REPEATED, weishida_device_run);
//	app_timer_start(weishida_resend_timer_id, APP_TIMER_TICKS(400,APP_TIMER_PRESCALER),dp_data[0]);
}

void curtain_percent(UINT16 dp_data_len, BYTE *dp_data)
{
	weishida_device_get_border_state();
	set_percent = 1;
	ConverseArrayToBigEndian(dp_data,(u32*)&run_percent);
}

void curtain_border(UINT16 dp_data_len, BYTE *dp_data)
{
	border = dp_data[0];
	weishida_device_control_border(dp_data[0]);
}

extern int run_location;
extern BOOL run_direction;
extern BOOL hand_start;
extern int dc_control_type;
extern int ac_control_type;

void curtain_location(UINT16 dp_data_len, BYTE *dp_data)
{
	weishida_device_get_position();
}

void curtain_direction(UINT16 dp_data_len, BYTE *dp_data)
{
	run_direction = dp_data[0];
//	weishida_device_set_motor_direct(run_direction);
	weishida_device_get_motor_direct();
}

void curtain_hand(UINT16 dp_data_len, BYTE *dp_data)
{
	hand_start = dp_data[0];
	weishida_device_set_handstart(hand_start);
//	weishida_device_get_handstart();
}

void curtain_state(UINT16 dp_data_len, BYTE *dp_data)
{
	weishida_device_get_state();
}

void curtain_border_state(UINT16 dp_data_len, BYTE *dp_data)
{
	weishida_device_get_border_state();
}

void curtain_DC_control(UINT16 dp_data_len, BYTE *dp_data)
{
	dc_control_type = dp_data[0];   //app send different data compared to the motor
//	weishida_device_get_DC_control_mode();
	weishida_device_set_DC_control_mode(dc_control_type);
}

void curtain_AC_control(UINT16 dp_data_len, BYTE *dp_data)
{
	ac_control_type = dp_data[0];  //app send different data compared to the motor
//	bsp_board_led_on(1);
//	weishida_device_get_AC_control_mode();
	weishida_device_set_AC_control_mode(ac_control_type);	
}

void curtain_type(UINT16 dp_data_len, BYTE *dp_data)
{
	weishida_device_get_device_type();
}

void curtain_report(void)
{

}





BOOL prod_test_flag = 0;
/////////////////////
void on_factory_test_complete(BOOL success_flag, INT8 rssi)
{
	//DO NOTHING
	if(success_flag)
	{
		printf("\r\nfactory test success£º%d,rssi: %d\r\n",success_flag,rssi);
		prod_test_flag = 1;
	}
	else
	{
		printf("\r\nfactory test timeout£º%d\r\n",success_flag);
		prod_test_flag = 0;
	}
	
}
