/***********************************************************
*  File: simple_flash_app.h 
*  Author: nzy
*  Date: 20160817
***********************************************************/
#ifndef _SIMPLE_FLASH_APP_H
    #define _SIMPLE_FLASH_APP_H
    
    #include "port.h"
	
	#include "simple_flash.h"
	
	#include "simple_flash_common.h"
	

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef  __SIMPLE_FLASH_APP_GLOBALS
    #define __SIMPLE_FLASH_APP_EXT
#else
    #define __SIMPLE_FLASH_APP_EXT extern
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#define PARTITION_SIZE (1 << 12) /* 4KB */
#define FLH_BLOCK_SZ PARTITION_SIZE

// flash map 
#ifdef NRF52832_PORTING
#define SIMPLE_FLASH_START 0x00065000
#define SIMPLE_FLASH_SIZE 0x8000 // 32K

#define SIMPLE_FLASH_SWAP_START 0x0006D000 
#define SIMPLE_FLASH_SWAP_SIZE 0x2000 // 8K
#endif

/***********************************************************
*************************variable define********************
***********************************************************/


/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: simple_flash_app_init
*  Input: none
*  Output: 
*  Return: none
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
OPERATE_RET simple_flash_app_init(VOID);

/***********************************************************
*  Function: simple_flash_app_uninit
*  Input: none
*  Output: 
*  Return: none
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
VOID simple_flash_app_uninit(VOID);

/***********************************************************
*  Function: sf_app_write
*  description: flash write method
*  Input: name->data name
*         data->data
*         len->data len
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
OPERATE_RET sf_app_write(CONST CHAR *name,CONST BYTE *data,CONST UINT len);

/***********************************************************
*  Function: sf_app_read
*  description: read method
*  Input: name->data name
*         data->write data
*         len->data len
*  Output: data->read data,need free
*          len->read data len 
*  Return: OPERATE_RET
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
OPERATE_RET sf_app_read(CONST CHAR *name,BYTE **data,UINT *len);

/***********************************************************
*  Function: sf_app_fuzzy_read
*  description: fuzzy read method
*  Input: fuzzy_name->fuzzy name
*         index->find index
*         data->read data
*         len->data len
*  Output: data->read data,need free
*          len->read data len 
*          index->the fine index +1
*  Return: OPERATE_RET
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
OPERATE_RET sf_app_fuzzy_read(CONST CHAR *fuzzy_name,UINT *index,\
                              BYTE **data,UINT *len);

/***********************************************************
*  Function: sf_app_format
*  Input: none
*  Output: none
*  Return: sf_ret
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
OPERATE_RET sf_app_format(VOID);

/***********************************************************
*  Function: sf_fuzzy_delete
*  description: simple flash data delete method
*  Input: fuzzy_name->delete data name
*  Output: none
*  Return: sf_ret
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
OPERATE_RET sf_app_fuzzy_delete(const CHAR *fuzzy_name);

/***********************************************************
*  Function: sf_delete
*  description: data delete method
*  Input: name->delete data name
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
__SIMPLE_FLASH_APP_EXT \
OPERATE_RET sf_app_delete(CONST CHAR *name);

#ifdef __cplusplus
}
#endif

#endif


