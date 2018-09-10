/*
 * tuya_factory_test.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_FACTORY_TEST_H_
#define TUYA_FACTORY_TEST_H_

#include "../../app/include/ty_broad.h"

typedef enum{
	FACTORY_TEST = 0,
	AUTH_RF_TEST,
}scan_type_t;

#define MAX_BUFFER_DATA			(20)
typedef struct _tag_data_stats_t{
	union
	{
	u8 length;
	u8 index;
	};
	u8 data[MAX_BUFFER_DATA];
}data_stats_t;

////////////////

BOOL tuya_auth_rf_test_begin_stats_flag_get(void);

void tuya_auth_rf_test_begin_stats_flag_set(BOOL flag);

s8 tuya_auth_rf_test_calc_avg(void);

void tuya_auth_rf_test_rssi_stats_add(s8 rssi);

void tuya_auth_rf_test_rssi_stats_reset(void);
	
////////////////


void tuya_factory_test_auth_rf_resp(BOOL suc_flag,s8 rssi);

scan_type_t tuya_factory_test_scan_type_get(void);

void tuya_start_factory_test_scan(scan_type_t test_type);

/////////////////


typedef void (*tuya_factory_test_cb_fun)(BOOL success_flag,INT8 rssi);

u8 tuya_user_app_regist_factory_test_cb(tuya_factory_test_cb_fun cb);

tuya_factory_test_cb_fun tuya_factory_test_get_cb(void);

void tuya_factorytest_on_scanrsp(u8 *dev_name, u8 dev_name_len, s8 dev_rssi);

void ty_factory_test(u8 len,u8 *pData);

void test_gpio(u8 *buf);

void gpio_test_run(u8 *buf);

#endif /* TUYA_FACTORY_TEST_H_ */
