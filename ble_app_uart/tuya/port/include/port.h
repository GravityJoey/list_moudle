/*
 * port.h
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#ifndef PORT_H_
#define PORT_H_

#include "../../app/include/ty_broad.h"
#include "../../../tuya/types.h"

#define NRF52832_PORTING

#define TUYA_TIMER_INTEVAL_MS			20

#define FDS_PORTING						0

#ifdef NRF52832_PORTING
#define APP_TIMER_PRESCALER             0

//注意 module 和 id均为0的情况
#define	GET_FILE_ID(module,id)			((((module)+1)*1000)+(((id+1))*100))
#define	GET_RECORD_KEY(module,id)		((((module)+1)*1000)+(((id+1))*100))
#define CEIL_DIV(A,B)					(((A)+(B)-1)/(B))

#define BASE_FLASH_ADDR					(0x60000)
#define MODULE_ID_FLASH_ADDR(module,id)	(BASE_FLASH_ADDR+((module+1)*(0x4000))+((id+1)*(0x1000)))

#define PORTING_TIMER_INTERVAL			APP_TIMER_TICKS(TUYA_TIMER_INTEVAL_MS,APP_TIMER_PRESCALER)

#define GPIO_PWM3		19
#define GPIO_PWM4		20	
#define GPIO_PWM2		18
#define GPIO_PWM1		17
#define GPIO_PWM0		22

#define BSP_PWM_0		GPIO_PWM1
#define BSP_PWM_1		GPIO_PWM2
#define BSP_PWM_2		GPIO_PWM3
#define BSP_PWM_3		GPIO_PWM4

#endif

#ifdef TLS8266_PORTING
#define GPIO_PWM3		3
#define GPIO_PWM4		4	
#define GPIO_PWM2		2
#define GPIO_PWM1		1
#define GPIO_PWM0		0
#endif

typedef enum {
	NV_BASIC_WHITE_LIST,
	NV_BASIC_ITEM_NUM,
} nv_basicItemId_t;

typedef enum {
	NV_USER_ITEM_H_ID,
	NV_USER_ITEM_AUZ_KEY,
	NV_USER_ITEM_D_ID,
	NV_USER_ITEM_MAC,
} nv_userItemId_t;

typedef enum {
	NV_USER_ITEM_RESET_CNT,
} nv_user2ItemId_t;

typedef enum {
	NV_USER_ITEM_PASS_KEY
} nv_user3ItemId_t;

typedef enum {
	NV_USER_MODULE = 0,     //!< Used for user application
	NV_USER_MODULE1,
	NV_USER_MODULE2,
	NV_USER_MODULE3,
	NV_MAX_MOULDS,            //!< Represent the number of modules used
} nv_module_t;

void ty_hw_timer_init(void);

void ty_gpio_init(u32 gpio_pin,u8 in_out,u32 up_down);

void ty_gpio_out_level(u32 gpio_pin,u8 low_high);

void tuya_gpio_set_func(u32 pin, u32 func);

void tuya_gpio_set_output_en(u32 pin, u32 value);

void tuya_gpio_set_input_en(u32 pin, u32 value,u8 pull_type);

void tuya_gpio_setup_up_down_resistor(u32 gpio, u32 up_down);

void tuya_gpio_write(u32 pin, u32 value);

u32 tuya_gpio_read(u32 pin);

void ty_pwm_all_init(void);

void ty_pwm_all_uninit(void);

void ty_pwm_init(u32 gpio_pin,u8 pwmid,u8 invert);

void ty_set_pwm(u8 pwmid,u8 cnt);

void ty_uart_init(void);

void ty_uart_send(u8 len,u8 *buf);

void ty_uart_data_callback(u8 len,u8 *data);

void ty_flash_write(u8 module, u8 id, u8 *buf, u16 len);

void ty_flash_read(u8 module, u8 id, u8 *buf, u16 len);

void ty_ble_notify(u8 len,u8 *buf);

void ty_set_adv_enable(u8 en);

void ty_set_adv_data(u8 len,u8 *data);

void ty_set_adv_param(u16 interval);

void ty_set_device_name(u8 len,u8 *buf);

void ty_set_scanrsp_data(u8 len,u8 *data);

void ty_set_mac_adress(u8 *data);

void ty_ble_discon(void);

void ty_ble_data_callback(u8 len,u8 *data);

void ty_ble_event_callback(u16 evt);

void ty_ble_scan_start(void);

void ty_ble_scan_stop(void);

void ty_ble_scan_callback(void *metaData);


void ty_user_init(void);
void ty_main_loop(void);

//
void ty_flash_erase_sector(u32 addr);

void ty_flash_read_page(u32 addr, u32 len, u8 *buf);

void ty_flash_write_page(u32 addr, u32 len, u8 *buf);

void ty_irq_disable(void);

void ty_irq_enable(void);

u8 ty_mem_init(void);

u8 *ty_malloc(u16 size);

void ty_free(u8 *ptr);

#endif /* PORT_H_ */
