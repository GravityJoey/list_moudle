/*
 * tuya_user_app_api.h
 *
 *  Created on: 2017-9-25
 *      Author: echo
 */
 
#ifndef TUYA_USER_APP_API_H_
#define TUYA_USER_APP_API_H_
#include "tuya_data_api.h"
#include "weishida_device_api.h"
#include "weishida_protocol.h"

void plug_switch(void);

void handle_switch_1(UINT8 dp_data_len,BYTE *dp_data);

void on_factory_test_complete(BOOL success_flag, INT8 rssi);

void curtain_run(UINT16 dp_data_len, BYTE *dp_data);

void curtain_percent(UINT16 dp_data_len, BYTE *dp_data);

void curtain_border(UINT16 dp_data_len, BYTE *dp_data);

void curtain_location(UINT16 dp_data_len, BYTE *dp_data);

void curtain_direction(UINT16 dp_data_len, BYTE *dp_data);

void curtain_hand(UINT16 dp_data_len, BYTE *dp_data);

void curtain_state(UINT16 dp_data_len, BYTE *dp_data);

void curtain_border_state(UINT16 dp_data_len, BYTE *dp_data);

void curtain_DC_control(UINT16 dp_data_len, BYTE *dp_data);

void curtain_AC_control(UINT16 dp_data_len, BYTE *dp_data);

void curtain_type(UINT16 dp_data_len, BYTE *dp_data);

void curtain_report(void);


#endif