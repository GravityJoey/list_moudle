/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>



#include "tuya_fds.h"
#include "fds.h"
#include "port.h"
#include "bsp.h"
#include "app.h"
#include "nrf_delay.h"
#include "ble_gap.h"
#include "weishida_protocol.h"


#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for placing the application in low power state while waiting for events.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**@brief Application main function.
 */
	
static u8 rdata[20+1]={0};

#ifdef CURTAIN_LOW_POWER

#endif

int main(void)
{
	bsp_nrf_init();
	
	ty_mem_init();
	
	ble_nrf_int();

	//tuya_fds_init();
	
	ty_user_init();

    wistar_uart_init();//fix the factory can not work without easy dma

    local_time_init();

	nrf_gpio_cfg(18,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT,
			NRF_GPIO_PIN_PULLUP,NRF_GPIO_PIN_S0S1,NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_set(18);
	nrf_gpio_cfg_input(20, NRF_GPIO_PIN_PULLUP);

    // Enter main loop.
    for (;;)
    {
		ty_main_loop();
        power_manage();
    }
}


/**
 * @}
 */
