/*
 * weishida_device_api.c
 *
 *  Created on: 2018-04-02 
 *      Author: echo
 */	
#include "../../app/include/ty_broad.h"
#include "weishida_protocol.h"
#include "weishida_device_api.h"
#include "nrf_delay.h"
	
#include <string.h>
#include <stdio.h>

BOOL uart_init_flag = 0;

/*#if 0
#define UART_SEND(data,len)		printf("UART_DATA:");\
							    for(u8 i=0;i<len;i++)	\
							    {	\
							        printf("%02x ",data[i]);	\
							    }	\
							    printf("\r\n");	
#else*/
#ifdef CURTAIN_LOW_POWER
#define UART_SEND(data,len)	  wistar_uart_init();   \
                              ty_uart_send(len, data);
#else 
#define UART_SEND(data,len)	  ty_uart_send(len, data);
#endif

static raw_data_t raw_data={0};

static curtains_data_t curtains_data={0};

void weishida_device_run(curtain_run_state_t state)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_CONTROL;
	
	switch(state)
	{
		case CURTAIN_UP:
			curtains_data.cmd = CMD_OPEN;
			break;
		case CURTAIN_DOWN:
			curtains_data.cmd = CMD_CLOSE;
			break;
		case CURTAIN_STOP:
			curtains_data.cmd = CMD_STOP;
			break;
		default:
			break;
	}
	
	curtains_data.payload_len = 0;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_start_percent(u8 percent)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_CONTROL;
	
	curtains_data.cmd = CMD_PERCENT;
	
	curtains_data.payload[0] = percent;
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

BOOL border_top_limit_set_flg = 0;
BOOL border_bottom_limit_set_flg = 0;

void weishida_device_control_border(curtain_border_t border_op)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_CONTROL;
	curtains_data.payload_len = 1;
	
	switch(border_op)
	{
		case SET_TOP_LIMIT:
			border_top_limit_set_flg = 1;
			curtains_data.cmd = CMD_SET_TOP_BORDER;
			curtains_data.payload[0] = 1;
			break;
		case DEL_TOP_LIMIT:
			border_top_limit_set_flg = 0;
			curtains_data.cmd = CMD_SET_TOP_BORDER;
			curtains_data.payload[0] = 0;
			break;
		case SET_BOTTOM_LIMIT:
			border_bottom_limit_set_flg = 1;
			curtains_data.cmd = CMD_SET_BOTTOM_BORDER;
			curtains_data.payload[0] = 1;
			break;
		case DEL_BOTTOM_LIMIT:
			border_bottom_limit_set_flg = 0;
			curtains_data.cmd = CMD_SET_BOTTOM_BORDER;
			curtains_data.payload[0] = 0;
			break;
		case DEL_BOTH_LIMIT:
		    border_top_limit_set_flg = 0;
            border_bottom_limit_set_flg = 0;
			curtains_data.cmd = CMD_DELETE_ALL_BORDER;
			
			curtains_data.payload_len = 0;//NO PAYLOAD
			break;
	}
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_led_control(connection_state_t state)
{
	//not use now
}

/////

void weishida_device_device_id(u16 id)
{
	//TODO
}

u8 weishida_device_get_position(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_POSITON;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
} 

void weishida_device_set_motor_direct(BOOL opposite)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_WRITE;
	
	curtains_data.cmd = CMD_DIRECT;
	
	curtains_data.payload[0] = 1;

	curtains_data.payload[1] = opposite;//ret data length 
	
	curtains_data.payload_len = 2;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_get_motor_direct(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_DIRECT;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_set_handstart(BOOL not_handstart)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_WRITE;
	curtains_data.cmd = CMD_HAND_START;
	curtains_data.payload[0] = 1;
	
	curtains_data.payload[1] = not_handstart;
	
	curtains_data.payload_len = 2;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_get_handstart(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_HAND_START;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_get_state(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_GET_STATE;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_get_border_state(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_GET_BORDER_STATE;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_set_DC_control_mode(DC_control_type_t type)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_WRITE;
	curtains_data.cmd = CMD_SWITCH_CONTROL_DC_TYPE;
	curtains_data.payload[0] = 1;
	
	curtains_data.payload[1] = type;
	
	curtains_data.payload_len = 2;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_get_DC_control_mode(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_SWITCH_CONTROL_DC_TYPE;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_set_AC_control_mode(AC_control_type_t type)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_WRITE;
	curtains_data.cmd = CMD_SWITCH_CONTROL_AC_TYPE;
	curtains_data.payload[0] = 1;
	
	curtains_data.payload[1] = type;
	
	curtains_data.payload_len = 2;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_get_AC_control_mode(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_SWITCH_CONTROL_AC_TYPE;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}

void weishida_device_get_device_type(void)
{
	memset(&curtains_data,0x0,sizeof(curtains_data_t));
	memset(&raw_data,0x0,sizeof(raw_data));
	
	curtains_data.opcode = OPCODE_READ;
	
	curtains_data.cmd = CMD_DEVICE_TYPE;
	
	curtains_data.payload[0] = 1;//ret data length 
	
	curtains_data.payload_len = 1;
	
	weishida_protocol_packet_encode(&curtains_data,&raw_data);
	UART_SEND(raw_data.data,raw_data.len);
}
