/*
 * tuya_common.h
 *
 *  Created on: 2017-9-22
 *      Author: echo
 */
 
#ifndef TUYA_COMMON_H_
#define TUYA_COMMON_H_

#include "./tuya/app/include/ty_broad.h"

#define INCREMENT(x,n) (x+n)

typedef INT OPERATE_RET; 
#define OPRT_COMMON_START 0 
#define OPRT_OK 					INCREMENT(OPRT_COMMON_START,0)
#define OPRT_COM_ERROR 				INCREMENT(OPRT_COMMON_START,1)	
#define OPRT_INVALID_PARM 			INCREMENT(OPRT_COMMON_START,2)
#define OPRT_MALLOC_FAILED 			INCREMENT(OPRT_COMMON_START,3)
#define OPRT_ERROR_INTERNAL			INCREMENT(OPRT_COMMON_START,4)
#define OPRT_ERROR_NOT_FOUND		INCREMENT(OPRT_COMMON_START,5)
#define OPRT_COMMON_END 			OPRT_ERROR_NOT_FOUND 

#endif