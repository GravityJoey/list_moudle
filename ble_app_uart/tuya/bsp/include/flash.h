/*
 * flash.h
 *
 *  Created on: 2017-9-13
 *      Author: echo
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "../../../tuya/types.h"

#define SD_FLASH_VERSION	1

#if SD_FLASH_VERSION
void tuya_sd_flash_sys_event_handler(uint32_t sys_evt);
#endif

void ty_flash_erase_sector(u32 addr);
void ty_flash_read_page(u32 addr, u32 len, u8 *buf);
void ty_flash_write_page(u32 addr, u32 len, u8 *buf);

#endif