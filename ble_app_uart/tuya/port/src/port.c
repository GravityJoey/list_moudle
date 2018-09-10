/*
 * port.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */


#include "../../types.h"
#include "nrf_gpio.h"
#include "../../app/include/ty_broad.h"
#include "../include/port.h"
#include "../../module/include/tuya_event.h"

#ifdef TLS8266_PORTING
#include "../../../proj_lib/ble/ble_ll.h"
#include "../../../proj_lib/ble/blt_config.h"
#include "../../../proj_lib/ble/ll_whitelist.h"
#endif

#ifdef NRF52832_PORTING
#include "ble_nus.h"
#include "ble_advertising.h"
#include "tuya_fds.h"
#include "app_timer.h"
#include "../../bsp/include/flash.h"
#include "../../utils/include/ev_buffer.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"
#endif

////////////////////////////////////////////////////////////////////

void ty_ble_notify(u8 len,u8 *buf)
{
#ifdef TLS8266_PORTING
    /*0x11->notify att handle*/
    bls_att_pushNotifyData(13, buf, len);
#endif
	
#ifdef NRF52832_PORTING
	extern ble_nus_t	m_nus;
    ble_nus_string_send(&m_nus, buf, len);
#endif	
}

void ty_set_adv_enable(u8 en)
{
#ifdef TLS8266_PORTING
    //enable & disable
    bls_ll_setAdvEnable(en);
#endif
	
#ifdef NRF52832_PORTING
	if(en)
	{
		ble_advertising_start(BLE_ADV_MODE_FAST);
	}
	else
	{
		sd_ble_gap_adv_stop();
	}
#endif
}

void ty_set_adv_data(u8 len,u8 *data)
{
#ifdef TLS8266_PORTING
    bls_ll_setAdvData(data,len);
#endif
	
#ifdef NRF52832_PORTING
	//sd_ble_gap_adv_data_set(data, len, NULL, 0);
#endif
}

void ty_set_adv_param(u16 interval)
{
#ifdef TLS8266_PORTING
    u8 status = bls_ll_setAdvParam( interval, interval, \
                                    ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, \
                                    0,  NULL,  BLT_ENABLE_ADV_37, ADV_FP_NONE);
    if(status != BLE_SUCCESS)
    {
        write_reg8(0x8000, 0x11);
        while(1);
    }
#endif
#ifdef NRF52832_PORTING
#endif
}

void ty_set_device_name(u8 len,u8 *buf)
{
#ifdef TLS8266_PORTING
    bls_att_setDeviceName(buf,len);
#endif
	
#ifdef NRF52832_PORTING
	ble_gap_conn_sec_mode_t sec_mode;
	
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	
	sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) buf, len);
#endif
}

void ty_set_scanrsp_data(u8 len,u8 *data)
{
#ifdef TLS8266_PORTING
    bls_ll_setScanRspData(data,len);
#endif
	
#ifdef NRF52832_PORTING
	sd_ble_gap_adv_data_set(NULL, 0, data, len);//only actually in change pass key flag
#endif
}

void ty_ble_scan_start(void)
{
	(void) sd_ble_gap_scan_stop();
	
	#define SCAN_INTERVAL               0x00A0                                        /**< Determines scan interval in units of 0.625 millisecond. */
	#define SCAN_WINDOW                 0x0050                                        /**< Determines scan window in units of 0.625 millisecond. */
	#define SCAN_TIMEOUT                0
	
	static const ble_gap_scan_params_t m_scan_params =
	{
		.active   = 1,
		.interval = SCAN_INTERVAL,
		.window   = SCAN_WINDOW,
		.timeout  = SCAN_TIMEOUT,
		#if (NRF_SD_BLE_API_VERSION == 2)
			.selective   = 0,
			.p_whitelist = NULL,
		#endif
		#if (NRF_SD_BLE_API_VERSION == 3)
			.use_whitelist = 0,
		#endif
	};

    sd_ble_gap_scan_start(&m_scan_params);
}

void ty_ble_scan_stop(void)
{
	(void) sd_ble_gap_scan_stop();
}

void ty_ble_scan_callback(void *metaData)
{
	ble_evt_t * p_ble_evt = (ble_evt_t*)metaData;
	
	const ble_gap_evt_t   * const p_gap_evt = &p_ble_evt->evt.gap_evt;
	ret_code_t                    err_code;
	
	const ble_gap_evt_adv_report_t *p_adv_report = &p_gap_evt->params.adv_report;
	
	data_t   adv_data;
	data_t   dev_name;
	
	adv_data.p_data     = (uint8_t *)p_adv_report->data;
	adv_data.data_len   = p_adv_report->dlen;
	
	err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
							&adv_data,
							&dev_name);
	//TODO need rr_code macro
	if(err_code == 0)//parese name suc
	{
		//!!!!! must call scan process fun
		tuya_factorytest_on_scanrsp(dev_name.p_data,dev_name.data_len,p_adv_report->rssi);
	}
}

//TODO reset or not?
void ty_ble_discon(void)
{
#ifdef TLS8266_PORTING
    irq_disable();
    start_reboot();
    //cpu_reset();
#endif
	
#ifdef NRF52832_PORTING
	NVIC_SystemReset();
#endif	
}

void ty_set_mac_adress(u8 *data)
{
#ifdef TLS8266_PORTING
	nv_flashWrite(CFG_MAC_ADDRESS,6,data);
#endif
}

void ty_ble_data_callback(u8 len,u8 *data)
{
	ty_ble_proc(len,data);
}

////////////////////////////

void ty_irq_disable()
{
#ifdef NRF52832_PORTING
	__disable_irq() ;
#endif
}

void ty_irq_enable()
{
#ifdef NRF52832_PORTING
	__enable_irq() ;
#endif
}

////////////////////////////////////////////////////////////////////

void ty_uart_init(void)
{
	//TODO change Telink app.c to portable version
}

void ty_uart_send(u8 len,u8 *buf)
{
#ifdef TLS8266_PORTING
    hci_send_data (0x01, buf, len);
    return;
#endif

	int i;
#ifdef NRF52832_PORTING
	for(i = 0;i < len;i++)
	{
		app_uart_put(buf[i]);//echo note here must be "i" 
	}
#endif
}

void ty_uart_data_callback(u8 len,u8 *data)
{		
    ty_uart_proc(len,data);	
}

////////////////////////////////////////////////////////////////////
//TODO return status
void ty_flash_write(u8 module, u8 id, u8 *buf, u16 len)
{
#ifdef TLS8266_PORTING
	if(module == NV_USER_MODULE3)
	{
		if(SUCCESS != nv_write(NV_USER_MODULE3, id, len, buf)){
			nv_resetModule(NV_USER_MODULE3);
			nv_write(NV_USER_MODULE3, id, len, buf);
		}
	}
	else
	{
		nv_write(module, id, len, buf);
	}
#endif
	
#ifdef NRF52832_PORTING
	
#if FDS_PORTING
	
	#if 0
	
	tuya_fds_find_and_delete(GET_FILE_ID(module,id),GET_RECORD_KEY(module,id));
	tuya_fds_write_record(GET_FILE_ID(module,id),GET_RECORD_KEY(module,id),(uint32_t*)buf,CEIL_DIV(len,4));//除4向上取整

	#else
	
	tuya_event_t evt;
	evt.hdr.event_id = TUYA_FLASH_SAVE_EVT;
	
	u8 *rcvbuf = ty_malloc(len);
	memcpy(rcvbuf,buf,len);
	
	evt.saveFlash_evt.data = rcvbuf;
	evt.saveFlash_evt.data_length = len;
	evt.saveFlash_evt.id = id;
	evt.saveFlash_evt.module = module;
	
	tuya_event_send(&evt);
	#endif
	
#else

	
	tuya_event_t evt;
	evt.hdr.event_id = TUYA_FLASH_SAVE_EVT;
	
	u8 *rcvbuf = ty_malloc(len);
	memcpy(rcvbuf,buf,len);
	
	evt.saveFlash_evt.data = rcvbuf;
	evt.saveFlash_evt.data_length = len;
	evt.saveFlash_evt.id = id;
	evt.saveFlash_evt.module = module;
	
	tuya_event_send(&evt);
	
	printf("ty_flash_write\r\n");
	

	//nv_write(module,id,len,buf);
	
	/*
	ty_flash_erase_sector(MODULE_ID_FLASH_ADDR(module,id));
	ty_flash_write_page(MODULE_ID_FLASH_ADDR(module,id),len,buf);
	*/
#endif
	
#endif
}

void ty_flash_read(u8 module, u8 id, u8 *buf, u16 len)
{
#ifdef TLS8266_PORTING
	nv_read(module, id, len, buf);
#endif
	
#ifdef NRF52832_PORTING

#if FDS_PORTING
	u8 *tp_buf = ty_malloc(len);
	tuya_fds_read_record(GET_FILE_ID(module,id),GET_RECORD_KEY(module,id),(uint32_t *)tp_buf);//先读到临时buf,防止读取4字节对齐数据,导致内存越界
	memcpy(buf,tp_buf,len);
	ty_free((u8*)tp_buf);
#else
	//nv_read(module,id,len,buf);
	
	u32 addr = MODULE_ID_FLASH_ADDR(module,id);
	
	printf("addr:%x ",addr);

	ty_flash_read_page(addr,len,buf);
#endif
	
#endif
	
	printf("read nv buf:%s\r\n",buf);
}

////////////////////////////////////////////////////////////////////

void ty_loop_timer_handler(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	ty_timer_update();
}

void ty_hw_timer_init(void)
{
#ifdef TLS8266_PORTING
    //enable timer0 interrupt
    reg_irq_mask |= FLD_IRQ_TMR0_EN;
    reg_tmr0_tick = 0;
    reg_tmr0_capt = CLOCK_SYS_CLOCK_1US * IRQ_TIME0_INTERVAL;
    reg_tmr_ctrl |= FLD_TMR0_EN;
#endif
	
#ifdef NRF52832_PORTING
	APP_TIMER_DEF(tuya_loop_timer_id);
	app_timer_create(&tuya_loop_timer_id,APP_TIMER_MODE_REPEATED,ty_loop_timer_handler);
	app_timer_start(tuya_loop_timer_id,PORTING_TIMER_INTERVAL,NULL);
#endif
}

//////////////////////////////////////////////////////////////////////

void ty_gpio_init(u32 gpio_pin,u8 in_out,u32 up_down)
{
#ifdef TLS8266_PORTING
    gpio_set_func(gpio_pin,AS_GPIO);
    if(IN_PUT == in_out){
        gpio_set_output_en(gpio_pin, 0);
        gpio_set_input_en(gpio_pin, 1);
        gpio_setup_up_down_resistor(gpio_pin,up_down);
    }
    else if(OUT_PUT == in_out){
        gpio_set_input_en(gpio_pin, 0);
        gpio_set_output_en(gpio_pin, 1);
    }
#endif
}

void ty_gpio_out_level(u32 gpio_pin,u8 low_high)
{
#ifdef TLS8266_PORTING
    if(OUTPUT_LOW == low_high){
        gpio_write(gpio_pin, OUTPUT_LOW);
    }
    else if(OUTPUT_HIGH == low_high){
        gpio_write(gpio_pin, OUTPUT_HIGH);
    }
#endif
}

void tuya_gpio_set_func(u32 pin, u32 func)
{
#ifdef NRF52832_PORTING
#endif	
}

void tuya_gpio_set_output_en(u32 pin, u32 value)
{
#ifdef NRF52832_PORTING
	
	if(value)
	{
		nrf_gpio_cfg_output(pin);
	}
	else
	{
		nrf_gpio_cfg_default(pin);
	}
	
#endif	
}

void tuya_gpio_set_input_en(u32 pin, u32 value,u8 pull_type)
{
#ifdef NRF52832_PORTING
	if(value)
	{
		if(pull_type == 1)
			nrf_gpio_cfg_input(pin,NRF_GPIO_PIN_PULLUP);
		else if(pull_type == 0)
			nrf_gpio_cfg_input(pin,NRF_GPIO_PIN_NOPULL);
		else
			nrf_gpio_cfg_input(pin,NRF_GPIO_PIN_PULLDOWN);
	}
	else
	{
		nrf_gpio_cfg_default(pin);
	}
#endif
}

void tuya_gpio_setup_up_down_resistor(u32 gpio, u32 up_down)
{
#ifdef NRF52832_PORTING	
#endif
}

u32 tuya_gpio_read(u32 pin)
{
#ifdef NRF52832_PORTING
	return nrf_gpio_pin_read(pin);
#endif
}

void tuya_gpio_write(u32 pin, u32 value)
{
#ifdef NRF52832_PORTING
	if(value)
	{
		nrf_gpio_pin_set(pin);
	}
	else
	{
		nrf_gpio_pin_clear(pin);
	}
#endif
}

///////////////////////////////////////////////////////////

static nrf_drv_pwm_t m_pwm = NRF_DRV_PWM_INSTANCE(0);

// This is for tracking PWM instances being used, so we can unintialize only
// the relevant ones when switching from one demo to another.
#define USED_PWM(idx) (1UL << idx)
static uint8_t m_used = 0;


static uint16_t const              m_demo1_top  = 10000;
static uint16_t const              m_demo1_step = 500;
static uint8_t                     m_demo1_phase;
static nrf_pwm_values_individual_t m_demo1_seq_values;
static nrf_pwm_sequence_t const    m_demo1_seq =
{
    .values.p_individual = &m_demo1_seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(m_demo1_seq_values),
    .repeats             = 0,
    .end_delay           = 0
};

static void demo1_handler(nrf_drv_pwm_evt_type_t event_type)
{
    if (event_type == NRF_DRV_PWM_EVT_FINISHED)
    {
		/*
        uint8_t channel    = m_demo1_phase >> 1;
        bool    down       = m_demo1_phase & 1;
        bool    next_phase = false;

        uint16_t * p_channels = (uint16_t *)&m_demo1_seq_values;
        uint16_t   value      = p_channels[channel];
			
		static int i = 0;
		value = (i < 20) ? (i * m_demo1_step) : (m_demo1_top - (i - 20) * m_demo1_step);
		i++;
		
		if( i == 40)
		{
				i = 0;
		}
		
		p_channels[0] = value;
		p_channels[1] = m_demo1_top - value;
		p_channels[2] = value;
		p_channels[3] = m_demo1_top - value;
		
		nrf_delay_ms(25);
		*/
    }
}

void ty_pwm_all_init(void)
{
	uint32_t                   err_code;
    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            BSP_PWM_0 | NRF_DRV_PWM_PIN_INVERTED, // channel 0
            BSP_PWM_1 | NRF_DRV_PWM_PIN_INVERTED, // channel 1
            BSP_PWM_3 | NRF_DRV_PWM_PIN_INVERTED, // channel 2
            BSP_PWM_2 | NRF_DRV_PWM_PIN_INVERTED  // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = m_demo1_top,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    err_code = nrf_drv_pwm_init(&m_pwm, &config0, demo1_handler);
    APP_ERROR_CHECK(err_code);

    m_demo1_seq_values.channel_0 = 0;
    m_demo1_seq_values.channel_1 = 0;
    m_demo1_seq_values.channel_2 = 0;
    m_demo1_seq_values.channel_3 = 0;
    m_demo1_phase                = 0;

    nrf_drv_pwm_simple_playback(&m_pwm, &m_demo1_seq, 1,
                                NRF_DRV_PWM_FLAG_LOOP);
}

//TODO echo 测试是否可用
void ty_pwm_all_uninit(void)
{
	nrf_drv_pwm_stop(&m_pwm,1);
	nrf_drv_pwm_uninit(&m_pwm);
}

void ty_pwm_init(u32 gpio_pin,u8 pwmid,u8 invert)
{
#ifdef TLS8266_PORTING
    gpio_set_func(gpio_pin,AS_PWM);
	pwm_clk(2000000);
	/*set pwm0 mode to NORMAL MODE*/
	pwm_set_mode(pwmid,NORMAL_MODE,0);
	/*Set PWM0_output_frequency = 2khz and high_time_duty_ratio = 20%*/
	/*max_tick = pwm_module_clock / PWM0_output_frequency = 2MHz / 2kHz = 1000 */
	/*cmp_tick = max_tick * high_time_duty_ratio = 1000 * 20% = 200 */
    pwm_set_duty(pwmid, PWM_BASE, 30);
	/*enable PWM0 output*/
	pwm_start(pwmid);
#endif
	
#ifdef NRF52832_PORTING
#endif
}

#ifdef PWM_HIGH_ACTIVE
void ty_set_pwm(u8 pwmid,u8 cnt)
{
#ifdef TLS8266_PORTING
    u8 pwm_cnt = 0;

#ifdef WEIZHI_BLE_MODULE
    if(PWMID_B == pwmid){
        if(0 == cnt){
            pwm_set_duty(pwmid, PWM_BASE , PWM_BASE);
        }
        else{
            pwm_cnt = 255 - cnt;
            pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
        }
    }
    else{
        pwm_cnt = cnt;
        pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
    }
#endif
#ifdef TUYA_BLE_MODULE
    if(PWMID_CW == pwmid){
        if(0 == cnt){
            pwm_set_duty(pwmid, PWM_BASE , PWM_BASE);
        }
        else{
            pwm_cnt = 255 - cnt;
            pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
        }
    }
    else{
        pwm_cnt = cnt;
        pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
    }
#endif
#endif
#ifdef NRF52832_PORTING
	//uint16_t * p_channels = (uint16_t *)&m_demo1_seq_values;
	//p_channels[pwmid] = (m_demo1_top/256)*cnt;
#endif
}
#else
void ty_set_pwm(u8 pwmid,u8 cnt)
{
#ifdef TLS8266_PORTING
    u8 pwm_cnt = 0;

#ifdef WEIZHI_BLE_MODULE
    if(PWMID_B == pwmid){
        pwm_cnt = cnt;
        pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
    }
    else{
        if(0 == cnt){
            pwm_set_duty(pwmid, PWM_BASE , PWM_BASE);
        }
        else{
            pwm_cnt = 255-cnt;
            pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
        }
    }
#endif
#ifdef TUYA_BLE_MODULE
    if(PWMID_CW == pwmid){
        pwm_cnt = cnt;
        pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
    }
    else{
        if(0 == cnt){
            pwm_set_duty(pwmid, PWM_BASE , PWM_BASE);
        }
        else{
            pwm_cnt = 255-cnt;
            pwm_set_duty(pwmid, PWM_BASE , PWM_CNT_BASE*pwm_cnt);
        }
    }
#endif
	
#endif
	
#ifdef NRF52832_PORTING
	//uint16_t * p_channels = (uint16_t *)&m_demo1_seq_values;
	//p_channels[pwmid] = (m_demo1_top/256)*cnt;
#endif
}
#endif
///////////////////////////////////////////////////////////////
#define USE_EV_BUFFER	0

u8 ty_mem_init(void)
{
#ifdef NRF52832_PORTING
	#if USE_EV_BUFFER
	ev_buf_init();
	#else
	
	#endif
#endif
}

u8 *ty_malloc(u16 size)
{
#ifdef NRF52832_PORTING
	#if USE_EV_BUFFER
	return ev_buf_allocate(size);
	#else
	u8 *ptr = malloc(size); 
	
	if(ptr)
		memset(ptr,0x0,size);//allocate buffer need init
	
	return ptr;
	#endif
#endif
}

void ty_free(u8 *ptr)
{
#ifdef NRF52832_PORTING
	#if USE_EV_BUFFER
	ev_buf_free(ptr);
	#else
	return free(ptr);
	#endif
#endif	
}
