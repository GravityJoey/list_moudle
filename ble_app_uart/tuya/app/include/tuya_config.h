/*
 * tuya_config.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_CONFIG_H_
#define TUYA_CONFIG_H_

#include "ty_broad.h"

#include "../../port/include/port.h"
#include "../../module/include/tuya_aes.h"
#include "../../module/include/tuya_data_exchange.h"
#include "../../module/include/tuya_data_handler.h"
#include "../../module/include/tuya_factory_test.h"
#include "../../module/include/tuya_ota.h"
#include "../../module/include/tuya_timer.h"

#include "../../utils/include/tuya_utils.h"
#include "../../utils/include/aes.h"
#include "../../utils/include/mutli_tsf_protocol.h"

int ty_timer_proc(u8 timer_id);

#endif /* TUYA_CONFIG_H_ */
