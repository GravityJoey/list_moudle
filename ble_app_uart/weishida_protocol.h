/*
 * weishida_protocol.h
 *
 *  Created on: 2018-03-30
 *      Author: echo
 */
 
#include "common.h"
 
//little-endian

#define PREAMBLES			(0x55)
#define SLAVE_ADDR			(0xFEFE)

#define MAX_DATA_LEN		(64)
#define MAX_RAW_DATA_LEN	(128)

enum{
	ERROR_NULL			= -1,
	ERROR_PREAMBLES		= -2,
	ERROR_CRC			= -3,
};

typedef struct _tag_curtains_data_t{
		u8 opcode;
        u8 cmd;
		u8 payload_len;
		u8 payload[MAX_DATA_LEN];
}curtains_data_t;

typedef struct _tag_raw_data_t{
	u8 len;
	u8 data[MAX_RAW_DATA_LEN];
}raw_data_t;

int weishida_protocol_packet_encode(curtains_data_t *packet_data,raw_data_t *raw_data);

int weishida_protocol_packet_decode(curtains_data_t *packet_data,raw_data_t *raw_data);