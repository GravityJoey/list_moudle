/*
 * simple_flash_common.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */
#ifndef SIMPLE_FLASH_COMMON_H_
#define SIMPLE_FLASH_COMMON_H_

#define INCREMENT(x,n) (x+n)

#define INT 		int
#define VOID  		void
#define CONST		const
#define UINT		unsigned int
#define CHAR		char
#define BYTE		unsigned char
#define USHORT		unsigned short

typedef INT OPERATE_RET; 
#define OPRT_COMMON_START					0
#define OPRT_OK INCREMENT					(OPRT_COMMON_START,0)              
#define OPRT_COM_ERROR 						INCREMENT(OPRT_COMMON_START,1)
#define OPRT_INVALID_PARM 					INCREMENT(OPRT_COMMON_START,2)
#define OPRT_MALLOC_FAILED 					INCREMENT(OPRT_COMMON_START,3)   
#define OPRT_NOT_FOUND_DEV 					INCREMENT(OPRT_COMMON_START,4)  
#define OPRT_COMMON_END 					OPRT_NOT_FOUND_DEV 

#endif