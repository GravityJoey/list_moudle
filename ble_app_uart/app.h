/*
 * app.h
 *
 *  Created on: 2017-9-11
 *      Author: echo
 */

#ifndef APP_H_
#define APP_H_

#define MAX_UART_DATA_LENGTH	128

typedef struct _tag_uart_packet_t{
	uint8_t len;
	uint8_t data[MAX_UART_DATA_LENGTH];
}uart_packet_t;

#endif