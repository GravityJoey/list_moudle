/*
 * flash.c
 *
 *  Created on: 2017-9-13
 *      Author: echo
 */
#include "../include/flash.h"
#include "../../port/include/port.h"

static volatile uint8_t op_suc_flag = 0;

#if REGISTER_VERSION
static void flash_page_erase(uint32_t * page_address);
static void flash_word_write(uint32_t * address, uint32_t value);
#endif
 
void ty_flash_erase_sector(u32 addr)
{
	#if REGISTER_VERSION
	flash_page_erase((uint32_t *)addr);
	#endif
	
	#if SD_FLASH_VERSION
	uint8_t pageNum = (uint8_t)((u32)addr/4096);
	
	uint32_t ret;
	
	ret = sd_flash_page_erase(pageNum);
	
	if(ret == NRF_SUCCESS)
	{
		while(op_suc_flag == 0)
		{
			if(op_suc_flag < 0)//error
			{
				break;
			}
		}
	}
	
	//printf("Eret:%d\r\n",ret);
	#endif
}

void ty_flash_read_page(u32 addr, u32 len, u8 *buf)
{	
	memcpy((uint32_t *)buf,(uint32_t *)addr,len);
}

static __align(4) u8 savbuf[128] = {0};//must be 4 bytes aligned 

void ty_flash_write_page(u32 addr, u32 len, u8 *buf)
{	
	u32 wordsCounts = CEIL_DIV(len,sizeof(uint32_t));
	
	#if SD_FLASH_VERSION
	
	op_suc_flag = 0;
	
	memset(savbuf,0x0,128);
	
	//__align(4) u8 *savbuf = ty_malloc(len);// dynamic allocate
	
	memcpy(savbuf,buf,len);
	
	uint32_t ret;

	while(NRF_ERROR_BUSY == sd_flash_write((uint32_t *)(addr),(uint32_t*)(savbuf),wordsCounts));//repeat do it
	
	//printf("Wret:%d\r\n",ret);
	
	if(ret == NRF_SUCCESS)
	{
		while(op_suc_flag == 0)
		{
			if(op_suc_flag < 0)//error
			{
				break;
			}
		}
	}
	else
	{
		
	}
	
	//ty_free(savbuf);
	#endif
	
	#if REGISTER_VERSION 
	for(int i=0;i<wordsCounts;i++)
	{
		uint32_t word = *((uint32_t*)(buf)+i);
		
		flash_word_write((uint32_t*)addr,word);
		
		addr+=4;
	}
	#endif
}

#if SD_FLASH_VERSION
void tuya_sd_flash_sys_event_handler(uint32_t sys_evt)
{
	switch (sys_evt)
	{
		case NRF_EVT_FLASH_OPERATION_SUCCESS:
			op_suc_flag = 1;
			break;

		case NRF_EVT_FLASH_OPERATION_ERROR:
			op_suc_flag = -1;
			break;
	}

}
#endif

#if REGISTER_VERSION
//////////////////////////////

/** ²Á³ýflashµÄÒ³
 * ²Á³ýÒ»¸öÒ³Ê±Ö»ÐèÏòERASEPAGE¼Ä´æÆ÷Ð´Èë¸ÃÒ³µÚÒ»¸ö×ÖµÄµØÖ·¼´¿É
 */
static void flash_page_erase(uint32_t * page_address)
{
	ty_irq_disable();
	
    // ÉèÖÃ¼Ä´æÆ÷CONFIG.WEN = 2 Ê¹ÄÜFLASHµÄ²Á³ý²Ù×÷
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);
    // µÈ´ýNVMC(·ÇÒ×Ê§´æ´¢Æ÷¿ØÖÆÆ÷)×¼±¸¾ÍÐ÷
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // ²Á³ýÒ³ ²Á³ýÒ»¸öÒ³Ê±Ö»ÐèÏòERASEPAGE¼Ä´æÆ÷Ð´Èë¸ÃÒ³µÚÒ»¸ö×ÖµÄµØÖ·¼´¿É
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;
		
    // µÈ´ýNVMC(·ÇÒ×Ê§´æ´¢Æ÷¿ØÖÆÆ÷)×¼±¸¾ÍÐ÷
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // ÉèÖÃ¼Ä´æÆ÷CONFIG.WEN = 0 ¹Ø±Õ²Á³ý£¬¼´FLASH´¦ÓÚÖ»¶Á×´Ì¬
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
    
		// µÈ´ýNVMC(·ÇÒ×Ê§´æ´¢Æ÷¿ØÖÆÆ÷)×¼±¸¾ÍÐ÷
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
	
	ty_irq_enable();
}


/** ÏòFLASHÖ¸¶¨µØÖ·Ð´ÈëÒ»¸ö×Ö
 */
static void flash_word_write(uint32_t * address, uint32_t value)
{
    // ÉèÖÃ¼Ä´æÆ÷CONFIG.WEN = 1 Ê¹ÄÜFLASHµÄÐ´²Ù×÷
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);

    // µÈ´ýNVMC(·ÇÒ×Ê§´æ´¢Æ÷¿ØÖÆÆ÷)×¼±¸¾ÍÐ÷
	  while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
    //Ïò¸ÃµØÖ·Ð´ÈëÒ»¸ö×Övalue
    *address = value;

    // µÈ´ýNVMC(·ÇÒ×Ê§´æ´¢Æ÷¿ØÖÆÆ÷)×¼±¸¾ÍÐ÷
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // ÉèÖÃ¼Ä´æÆ÷CONFIG.WEN = 0 £¬¹Ø±ÕÐ´¹¦ÄÜ
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    // µÈ´ýNVMC(·ÇÒ×Ê§´æ´¢Æ÷¿ØÖÆÆ÷)×¼±¸¾ÍÐ÷
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}

#endif