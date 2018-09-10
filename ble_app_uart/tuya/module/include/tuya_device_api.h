/*
 * tuya_device_api.h
 *
 *  Created on: 2017-10-11
 *      Author: echo
 */
#include "../../port/include/port.h"

#ifndef TUYA_DEVICE_API_H_
#define TUYA_DEVICE_API_H_

void tuya_device_unbond(void);

void tuya_device_reboot(u32 time_ms);

#endif