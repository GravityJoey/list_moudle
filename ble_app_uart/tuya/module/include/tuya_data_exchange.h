/*
 * tuya_data_exchange.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef TUYA_DATA_EXCHANGE_H_
#define TUYA_DATA_EXCHANGE_H_

#include "../../app/include/ty_broad.h"

u8 ty_report_mux_packet(u8 type);
void ty_air_send(u8 len,u8 *buf,u8 type);
void ty_uart_protocol_send(u8 type,u8 *pdata,u8 len);

#endif /* TUYA_DATA_EXCHANGE_H_ */
