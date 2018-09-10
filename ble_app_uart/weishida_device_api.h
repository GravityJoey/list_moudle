/*
 * weishida_device_api.h
 *
 *  Created on: 2018-04-02 
 *      Author: echo
 */
#ifndef WEISHIDA_DEVICE_API_H_
#define WEISHIDA_DEVICE_API_H_

#include "common.h"
 
typedef enum{
	OPCODE_READ 	= 0x01,
	OPCODE_WRITE	= 0x02,
	OPCODE_CONTROL 	= 0x03,
	OPCODE_NOTIFY 	= 0x04,
}curtain_cmd_type_t;

////////////////opcode define///////////////
//control cmd
#define	CMD_OPEN								(0x01)
#define	CMD_CLOSE								(0x02)
#define	CMD_STOP								(0x03)
#define	CMD_PERCENT								(0x04)
#define	CMD_SET_TOP_BORDER						(0x05)
#define	CMD_SET_BOTTOM_BORDER					(0x06)
#define	CMD_DELETE_ALL_BORDER					(0x07)

//read/write cmd
#define CMD_ID_L								(0x00)
#define CMD_ID_H								(0x01)
#define CMD_POSITON								(0x02)
#define CMD_DIRECT								(0x03)
#define CMD_HAND_START							(0x04)
#define CMD_GET_STATE							(0x05)
#define CMD_GET_BORDER_STATE					(0x09)
#define CMD_SWITCH_CONTROL_DC_TYPE				(0x27)
#define CMD_SWITCH_CONTROL_AC_TYPE				(0x28)
#define CMD_DEVICE_TYPE							(0xF0)

//notify cmd
#define CMD_DEVICE_STATE						(0x02)
#define CMD_ERROR_INFO							(0x03)
#define CMD_RESET   							(0x04)

//////////////opcode define end/////////////

typedef enum{
	CURTAIN_UP = 0,
	CURTAIN_DOWN = 1,
	CURTAIN_STOP = 2,
}curtain_run_state_t;
 
typedef enum{
	SET_TOP_LIMIT 		= 0x00,
	DEL_TOP_LIMIT		= 0x01,
	SET_BOTTOM_LIMIT  	= 0x02,
	DEL_BOTTOM_LIMIT  	= 0x03,
	DEL_BOTH_LIMIT		= 0x04,
}curtain_border_t;

typedef enum{
	UNBONDED = 0x00,
	BONDING = 0x01,
	BONDED = 0x02,
	PAIRNG = 0x03,
}connection_state_t;

typedef enum{
	CURTAIN_NORMAL_STOP = 0x00,
	CURTAIN_OPEN,
	CURTAIN_CLOSE,
	CURTAIN_ERROR_STOP,
}curtain_state_t;

typedef enum{
	DC_DOUBLE_BOUNCE = 0x01,//DEFAULT
	DC_DOUBLE_NO_BOUNCE,
	DC_SMART,
	DC_SINGLE_CIRCLE,
}DC_control_type_t;
	
typedef enum{
	AC_DOUBLE_NO_BOUNCE = 0x01,//DEFAULT
	AC_HOTEL_MODE,
	AC_DOUBLE_BOUNCE,
	AC_SINGLE_CIRCLE,
}AC_control_type_t;

typedef struct{
	u8 cur_position;
	BOOL opposite_dir;
	BOOL not_handstart;
	curtain_state_t curtain_motor_state;
	u8 dummy[3];
	BOOL is_set_border;
}curtain_report_t;

typedef enum{
	ERROR_UNKNOW_UART_DATA = 0x01,
}curtain_report_error_t;

///control cmd

void weishida_device_run(curtain_run_state_t state);

void weishida_device_start_percent(u8 percent);

void weishida_device_control_border(curtain_border_t border_op);

void weishida_device_led_control(connection_state_t state);


///read/write cmd,----recv/send
void weishida_device_device_id(u16 id);

u8 weishida_device_get_position(void);

void weishida_device_set_motor_direct(BOOL opposite);//get direct before 

void weishida_device_get_motor_direct(void);

void weishida_device_set_handstart(BOOL not_handstart);

void weishida_device_get_handstart(void);

void weishida_device_get_state(void);

void weishida_device_get_border_state(void);


void weishida_device_set_DC_control_mode(DC_control_type_t type);

void weishida_device_get_DC_control_mode(void);

void weishida_device_set_AC_control_mode(AC_control_type_t type);

void weishida_device_get_AC_control_mode(void);


void weishida_device_get_device_type(void);

//notify cmd,----recv
void weishida_device_report_info(curtain_report_t report_data);

void weishida_device_report_error(curtain_report_error_t report_error_data);

#endif

