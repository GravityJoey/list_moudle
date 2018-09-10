/***********************************************************
*  File: simple_flash_app.h 
*  Author: nzy
*  Date: 20160817
***********************************************************/
#define __PSM_FLASH_GLOBALS

#include "simple_flash_app.h"
#include "simple_flash.h"
//#include "mem_pool.h"
//#include "adapter_platform.h"
//#include "flash_api.h"

#include "ble_flash.h"
#include "simple_flash_common.h"


/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
*************************variable define********************
***********************************************************/
static sf_hand_s *hand = NULL;
//static flash_t obj;

/***********************************************************
*************************function define********************
***********************************************************/
static sf_ret flash_read(sf_uint32_t addr, sf_uint8_t *dst, sf_uint32_t size)
{
    if(NULL == dst) {
        return SF_INVALID_PARM;
    }

    //flash_stream_read(&obj, addr, size, dst);
	
	memcpy((sf_uint32_t*)dst,(sf_uint32_t*)addr,size);
	
    return SF_OK;
}

static sf_ret flash_write(sf_uint32_t addr, sf_uint8_t *src, sf_uint32_t size)
{
    if(NULL == src) {
        return SF_INVALID_PARM;
    }
	
	//flash_stream_write(&obj, addr, size, src);
	
	u32 wordsCounts = CEIL_DIV(size,sizeof(uint32_t));
	
	//flash_page_erase((uint32_t *)addr);
	
	for(int i=0;i<wordsCounts;i++)
	{
		uint32_t word = *((uint32_t*)(src)+i);
		
		ble_flash_word_write((uint32_t*)addr,word);
		
		addr+=4;
	}
	
    return SF_OK;
}

static sf_ret flash_erase(sf_uint32_t addr, sf_uint32_t size)
{
    USHORT start_sec = (addr/PARTITION_SIZE);
    USHORT end_sec = ((addr+size-1)/PARTITION_SIZE);

    INT i = 0;
    for(i = start_sec;i <= end_sec;i++) {
        //flash_erase_sector(&obj, PARTITION_SIZE*i);
		ble_flash_page_erase(i);
    }
	
    return SF_OK;
}

/***********************************************************
*  Function: simple_flash_app_init
*  Input: none
*  Output: 
*  Return: none
***********************************************************/
OPERATE_RET simple_flash_app_init(VOID)
{
    if(hand) {
        return OPRT_OK;
    }

    sf_hook_s hook;
    hook.malloc_fn = ty_malloc;
    hook.free_fn = ty_free;
    sf_init_hooks(&hook);

	sf_flash_cfg_s cfg;
	cfg.start_addr = SIMPLE_FLASH_START; // user physical flash start address 
	cfg.flash_sz = SIMPLE_FLASH_SIZE; // user flash size
	cfg.block_sz = FLH_BLOCK_SZ; // flash block size
	cfg.swap_start_addr = SIMPLE_FLASH_SWAP_START; // swap flash start address
	cfg.swap_flash_sz = SIMPLE_FLASH_SWAP_SIZE; // swap flash size    
	cfg.sf_read = flash_read;
	cfg.sf_write = flash_write;
	cfg.sf_erase = flash_erase;
	cfg.sf_crc32 = NULL;

	sf_ret ret = SF_OK;
	ret = create_flash_hand_and_init(&hand, &cfg);
	if (ret != SF_OK) {
		return OPRT_COM_ERROR;
	}

    return OPRT_OK;
}

/***********************************************************
*  Function: simple_flash_app_uninit
*  Input: none
*  Output: 
*  Return: none
***********************************************************/
VOID simple_flash_app_uninit(VOID)
{
    if(NULL == hand) {
        return;
    }

    delete_flash_hand(hand);
    hand = NULL;
}

/***********************************************************
*  Function: sf_app_write
*  description: flash write method
*  Input: name->data name
*         data->data
*         len->data len
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET sf_app_write(CONST CHAR *name,CONST BYTE *data,CONST UINT len)
{
    sf_ret ret;

    ret = sf_write(hand,name,data,len);
    if(SF_OK != ret) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

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
OPERATE_RET sf_app_read(CONST CHAR *name,BYTE **data,UINT *len)
{
    sf_ret ret;

    ret = sf_read(hand,name,data,len);
    if(SF_OK != ret && ret != SF_NO_FIND) {
        return OPRT_COM_ERROR;
    }else if(ret == SF_NO_FIND) {
        return OPRT_NOT_FOUND_DEV;
    }

    return OPRT_OK;
}

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
OPERATE_RET sf_app_fuzzy_read(CONST CHAR *fuzzy_name,UINT *index,\
                              BYTE **data,UINT *len)
{
    sf_ret ret;

    ret = sf_fuzzy_read(hand,fuzzy_name,index,data,len);
    if(SF_OK != ret && ret != SF_NO_FIND) {
        return OPRT_COM_ERROR;
    }else if(ret == SF_NO_FIND) {
        return OPRT_NOT_FOUND_DEV;
    }

    return OPRT_OK;
}

/***********************************************************
*  Function: sf_delete
*  description: data delete method
*  Input: name->delete data name
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET sf_app_delete(CONST CHAR *name)
{
    sf_ret ret;

    ret = sf_delete(hand,name);
    if(SF_OK != ret) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/***********************************************************
*  Function: sf_fuzzy_delete
*  description: simple flash data delete method
*  Input: fuzzy_name->delete data name
*  Output: none
*  Return: sf_ret
***********************************************************/
OPERATE_RET sf_app_fuzzy_delete(const CHAR *fuzzy_name)
{
    sf_ret ret;
    ret = sf_fuzzy_delete(hand,fuzzy_name);
    if(SF_OK != ret) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/***********************************************************
*  Function: sf_app_format
*  Input: none
*  Output: none
*  Return: sf_ret
***********************************************************/
OPERATE_RET sf_app_format(VOID)
{
    sf_ret ret;

    ret = sf_format(hand);
    if(SF_OK != ret) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}


