/*
 * tuya_factory_test.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#include "../include/tuya_factory_test.h"

#include "../../port/include/port.h"

#include "app_timer.h"


extern u8 factory_test_reset;

extern u8 ty_factory_flag;

APP_TIMER_DEF(reset_timer_id);

void reset_packet_proc_handler(void * p_context)
{
	factory_test_reset = 1;
	
	ty_factory_flag = 1;

	ty_ble_discon();
}

void ty_factory_test(u8 len,u8 *pData)
{
    u8 ck_sum=0,i=0,temp=0;
    u8 *alloc_buf = NULL;
    static u8 if_enter = 0;

    ck_sum = check_sum(pData,len-1);
    if((0x55 == pData[0])&&(0xaa == pData[1])&&\
        (ck_sum == pData[len-1])){
    	if(0x00 == pData[3]){
    		u8 confirm_firmware = 1;
    		ty_uart_protocol_send(0x00, &confirm_firmware,1);
			ty_timer_stop(TIMER_FIRST);
			if_enter = 1;
    	}else if(1 == if_enter){
			switch(pData[3])
			{
				case 0x01:
					alloc_buf = ty_malloc(70);
					ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_H_ID, &alloc_buf[40], H_ID_LEN);
					//alloc_buf[40] = 0;//my add
					if(0x01 == alloc_buf[40]){
						alloc_buf[0] = '{';
						alloc_buf[1] = '\"';
						memcpy(&alloc_buf[2],"ret",3);
						alloc_buf[5] = '\"';

						alloc_buf[6] = ':';
						memcpy(&alloc_buf[7],"true",4);

						alloc_buf[11] = ',';
						alloc_buf[12] = '\"';
						memcpy(&alloc_buf[13],"hid",3);
						alloc_buf[16] = '\"';
						alloc_buf[17] = ':';
						alloc_buf[18] = '\"';

						memcpy(&alloc_buf[19],&alloc_buf[41],19);
						alloc_buf[38] = '\"';
						alloc_buf[39] = '}';
						ty_uart_protocol_send(0x01, alloc_buf,40);
					}
					else
					{
						alloc_buf[0] = '{';
						alloc_buf[1] = '\"';
						memcpy(&alloc_buf[2],"ret",3);
						alloc_buf[5] = '\"';

						alloc_buf[6] = ':';
						memcpy(&alloc_buf[7],"true",4);

						alloc_buf[11] = ',';
						alloc_buf[12] = '\"';
						memcpy(&alloc_buf[13],"hid",3);
						alloc_buf[16] = '\"';
						alloc_buf[17] = ':';
						alloc_buf[18] = '\"';
						alloc_buf[19] = '\"';
						alloc_buf[20] = '}';
						ty_uart_protocol_send(0x01, alloc_buf,21);
					}
					ty_free(alloc_buf);
				break;

				case 0x02:
					alloc_buf = ty_malloc(60);
					//test_gpio(&alloc_buf[50]);
					gpio_test_run(&alloc_buf[50]);
					alloc_buf[0] = '{';
					alloc_buf[1] = '\"';
					memcpy(&alloc_buf[2],"ret",3);
					alloc_buf[5] = '\"';
					alloc_buf[6] = ':';
#if SKIP_GPIO_TEST
					alloc_buf[50] = 0;//
#endif
					if(0 == alloc_buf[50]){
						memcpy(&alloc_buf[7],"true",4);
						alloc_buf[11] = '}';
						ty_uart_protocol_send(0x02, alloc_buf,12);
					}
					else{
						memcpy(&alloc_buf[7],"false",5);
						alloc_buf[12] = '}';
						ty_uart_protocol_send(0x02, alloc_buf,13);
					}
					ty_free(alloc_buf);
				break;

				case 0x03:
					alloc_buf = ty_malloc(80);
					memcpy(&alloc_buf[60],&pData[59],16);
					if(0 == memcmp(&pData[87+23],"true",4)){
						alloc_buf[76] = 0x01;
					}
					else if(0 == memcmp(&pData[87+23],"false",5)){
						alloc_buf[76] = 0x0;
					}
					
					ty_ble_scan_stop();
					
					ty_flash_write(NV_USER_MODULE,NV_USER_ITEM_AUZ_KEY, &pData[17], AUTH_KEY_LEN);
					ty_flash_write(NV_USER_MODULE,NV_USER_ITEM_D_ID, &alloc_buf[60], D_ID_LEN+1);
					ty_flash_write(NV_USER_MODULE,NV_USER_ITEM_MAC, &pData[84], MAC_LEN);

					//ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_AUZ_KEY, alloc_buf, AUTH_KEY_LEN);
					//ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_D_ID, &alloc_buf[40], D_ID_LEN);
					
					//TODO skip by echo÷
					memcpy(alloc_buf,&pData[17],AUTH_KEY_LEN);
					memcpy(&alloc_buf[40],&alloc_buf[60],D_ID_LEN);
					
					if((0 == memcmp(alloc_buf,&pData[17],AUTH_KEY_LEN))&&\
						(0 == memcmp(&alloc_buf[40],&pData[59],16))){
						alloc_buf[0] = '{';
						alloc_buf[1] = '\"';
						memcpy(&alloc_buf[2],"ret",3);
						alloc_buf[5] = '\"';

						alloc_buf[6] = ':';
						memcpy(&alloc_buf[7],"true",4);
						alloc_buf[11] = '}';
						ty_uart_protocol_send(0x03, alloc_buf,12);
					}
					else{
						alloc_buf[0] = '{';
						alloc_buf[1] = '\"';
						memcpy(&alloc_buf[2],"ret",3);
						alloc_buf[5] = '\"';

						alloc_buf[6] = ':';
						memcpy(&alloc_buf[7],"false",5);
						alloc_buf[12] = '}';
						ty_uart_protocol_send(0x03, alloc_buf,13);
					}
					ty_free(alloc_buf);
				break;

				case 0x04:
					i = 0;
					alloc_buf = ty_malloc(80);
					alloc_buf[i++] = 0x55;
					alloc_buf[i++] = 0xaa;
					alloc_buf[i++] = 0x0;
					alloc_buf[i++] = 0x04;//type;
					alloc_buf[i++] = 0;
					i++;//len
					alloc_buf[i++] = '{';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"ret",3);
					i += 3;
					alloc_buf[i++] = '\"';

					alloc_buf[i++] = ':';
					memcpy(&alloc_buf[i],"true",4);
					i += 4;
					alloc_buf[i++] = ',';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"auzKey",6);
					i += 6;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_AUZ_KEY, &alloc_buf[i], AUTH_KEY_LEN);
					i += AUTH_KEY_LEN;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ',';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"hid",3);
					i += 3;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_H_ID, &alloc_buf[100], H_ID_LEN);
					memcpy(&alloc_buf[i],&alloc_buf[101],19);
					i += 19;
					alloc_buf[i++] = '\"';

					alloc_buf[i++] = ',';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"uuid",4);
					i += 4;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_D_ID, &alloc_buf[i], D_ID_LEN+1);
					i += 16;
					temp = alloc_buf[i];
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ',';
					
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"mac",3);
					i += 3;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_MAC, &alloc_buf[i], MAC_LEN);
//					memcpy(&alloc_buf[i],"402c004C23BC",12);
					i += MAC_LEN;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ',';
					
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"firmName",8);
					i += 8;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],TY_FIRMWARE_NAME,strlen(TY_FIRMWARE_NAME));
					i+=strlen(TY_FIRMWARE_NAME);
					alloc_buf[i++] = '\"';

					alloc_buf[i++] = ',';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"firmVer",7);
					i+=7;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],TY_FIRMWARE_VER,strlen(TY_FIRMWARE_VER));
					i+=strlen(TY_FIRMWARE_VER);
					alloc_buf[i++] = '\"';

					alloc_buf[i++] = ',';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"prod_test",9);
					i+=9;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					if(0x1 == temp){
						memcpy(&alloc_buf[i],"true",4);
						i += 4;
					}
					else if(0x0 == temp){
						memcpy(&alloc_buf[i],"false",5);
						i += 5;
					}

					alloc_buf[i++] = '}';
					alloc_buf[5] = i-6;
					alloc_buf[i] = check_sum(alloc_buf,i);
					i++;
					ty_uart_send(i,alloc_buf);
					ty_free(alloc_buf);
				break;

				case 0x05://reset				
				
					ty_factory_flag = 0;	
				
//					bsp_board_led_off(3);
				
					ty_uart_protocol_send(0x05,NULL,0);
				
					#if 1
				
					ty_factory_flag = 0;		
				
					app_timer_create(&reset_timer_id,APP_TIMER_MODE_SINGLE_SHOT,reset_packet_proc_handler);
					app_timer_start(reset_timer_id,APP_TIMER_TICKS(1000,APP_TIMER_PRESCALER),NULL);
				
					#else
				
					ty_timer_start(TIMER_FIRST,1000);
				
					factory_test_reset = 1;
					#endif
					
				break;
				
				case 0x06:
					i = 0;
					alloc_buf = ty_malloc(60);
					alloc_buf[i++] = '{';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"ret",3);
					i+=3;
					alloc_buf[i++] = '\"';

					alloc_buf[i++] = ':';
					memcpy(&alloc_buf[i],"true",4);
					i+=4;

					alloc_buf[i++] = ',';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"firmName",8);
					i+=8;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],TY_FIRMWARE_NAME,strlen(TY_FIRMWARE_NAME));
					i+=strlen(TY_FIRMWARE_NAME);
					alloc_buf[i++] = '\"';

					alloc_buf[i++] = ',';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],"firmVer",7);
					i+=7;
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = ':';
					alloc_buf[i++] = '\"';
					memcpy(&alloc_buf[i],TY_FIRMWARE_VER,strlen(TY_FIRMWARE_VER));
					i+=strlen(TY_FIRMWARE_VER);
					alloc_buf[i++] = '\"';
					alloc_buf[i++] = '}';
					ty_uart_protocol_send(0x06, alloc_buf,i);
					ty_free(alloc_buf);
				break;

				case 0x07://write hid
					alloc_buf = ty_malloc(50);
					memcpy(&alloc_buf[1],&pData[14],19);
					alloc_buf[0] = 0x01;	
				
					ty_flash_write(NV_USER_MODULE,NV_USER_ITEM_H_ID, alloc_buf, H_ID_LEN);
					//ty_flash_read(NV_USER_MODULE,NV_USER_ITEM_H_ID, &alloc_buf[30], H_ID_LEN);
				
					bsp_board_led_off(2);
				
					//TODO skip by echo
					memcpy(alloc_buf+30,alloc_buf,H_ID_LEN);
				
					if(0 == memcmp(&alloc_buf[30],alloc_buf,H_ID_LEN)){
						//save 76000 for mac
						//flash_erase_sector(CFG_MAC_ADDRESS);
						memcpy(alloc_buf,&pData[21],12);
						for(i =0;i<6;i++){
							alloc_buf[30+i] = alloc_buf[10-i*2] - 0x30;
							alloc_buf[30+i] <<=4;
							alloc_buf[30+i] += (alloc_buf[11-i*2] - 0x30);
						}

						//ty_set_mac_adress(&alloc_buf[30]);//ADD BY ECHO
						//nv_flashWrite(CFG_MAC_ADDRESS,6,&alloc_buf[30]);

						alloc_buf[0] = '{';
						alloc_buf[1] = '\"';
						memcpy(&alloc_buf[2],"ret",3);
						alloc_buf[5] = '\"';

						alloc_buf[6] = ':';
						memcpy(&alloc_buf[7],"true",4);
						alloc_buf[11] = '}';
						ty_uart_protocol_send(0x07, alloc_buf,12);
					}
					else{
						alloc_buf[0] = '{';
						alloc_buf[1] = '\"';
						memcpy(&alloc_buf[2],"ret",3);
						alloc_buf[5] = '\"';

						alloc_buf[6] = ':';
						memcpy(&alloc_buf[7],"false",5);
						alloc_buf[12] = '}';
						ty_uart_protocol_send(0x07, alloc_buf,13);
					}
					ty_free(alloc_buf);
				break;
					
				case 0x08://rf test
					
					//Òì²½ËÑË÷²¢»Ø¸´
					tuya_start_factory_test_scan(AUTH_RF_TEST);

					break;

				default:
					break;
			}
    	}
    }
}

#ifdef TY_SPP_MODE
#define TEST_GPIO_NUM     7

void test_gpio(u8 *buf)
{
	#if 0
	u32 gpio_pin[TEST_GPIO_NUM]={GPIO_PWM0,GPIO_PWM1,GPIO_PWM2,\
                GPIO_PWM3,GPIO_PWM4,GPIO_PE7,GPIO_PF1};
	
    u8 temp = 0,j = 0,i = 0,index = 1,flag = 0;

    buf[0] = 0;
    for(j = 0; j < 7;j++){
        //gpio_init();
        gpio_set_func(GPIO_PWM0, AS_GPIO);
        gpio_set_func(GPIO_PWM1, AS_GPIO);
        gpio_set_func(GPIO_PWM2, AS_GPIO);
        gpio_set_func(GPIO_PWM3, AS_GPIO);
        gpio_set_func(GPIO_PWM4, AS_GPIO);
        gpio_set_func(GPIO_PE7, AS_GPIO);
        gpio_set_func(GPIO_PF1, AS_GPIO);
        gpio_set_output_en(GPIO_PWM0, 0);
        gpio_set_output_en(GPIO_PWM1, 0);
        gpio_set_output_en(GPIO_PWM2, 0);
        gpio_set_output_en(GPIO_PWM3, 0);
        gpio_set_output_en(GPIO_PWM4, 0);
        gpio_set_output_en(GPIO_PE7, 0);
        gpio_set_output_en(GPIO_PF1, 0);
        gpio_set_input_en(GPIO_PWM0, 1);
        gpio_set_input_en(GPIO_PWM1, 1);
        gpio_set_input_en(GPIO_PWM2, 1);
        gpio_set_input_en(GPIO_PWM3, 1);
        gpio_set_input_en(GPIO_PWM4, 1);
        gpio_set_input_en(GPIO_PE7, 1);
        gpio_set_input_en(GPIO_PF1, 1);
        gpio_setup_up_down_resistor(GPIO_PWM0,PM_PIN_PULLUP_10K);
        gpio_setup_up_down_resistor(GPIO_PWM1,PM_PIN_PULLUP_10K);
        gpio_setup_up_down_resistor(GPIO_PWM2,PM_PIN_PULLUP_10K);
        gpio_setup_up_down_resistor(GPIO_PWM3,PM_PIN_PULLUP_10K);
        gpio_setup_up_down_resistor(GPIO_PWM4,PM_PIN_PULLUP_10K);
        gpio_setup_up_down_resistor(GPIO_PE7,PM_PIN_PULLUP_10K);
        gpio_setup_up_down_resistor(GPIO_PF1,PM_PIN_PULLUP_10K);
        gpio_set_input_en(gpio_pin[j], 0);
        gpio_set_output_en(gpio_pin[j], 1);
        gpio_write(gpio_pin[j], 0);
        for(i =0;i<7;i++){
            if(j!=i){
                temp = gpio_read(gpio_pin[i]);
                if((0 == j)&&(3 == i)){
                    flag = 1;
                }
                else if((1 == j)&&(5 == i)){
                    flag = 1;
                }
                else if((2 == j)&&((4 == i)||(6 == i))){
                    flag = 1;
                }
                else if((3 == j)&&(0 == i)){
                    flag = 1;
                }
                else if((4 == j)&&((2 == i)||(6 == i))){
                    flag = 1;
                }
                else if((5 == j)&&(1 == i)){
                    flag = 1;
                }
                else if((6 == j)&&((2 == i)||(4 == i))){
                    flag = 1;
                }
                else{
                    if((0 == temp)&&(0 == check_num(buf,i+0x30))){
                        buf[0]++;
                        buf[index++] = i + 0x30;
                    }
                }
                if(flag){
                    flag = 0;
                    if((1 == temp)&&(0 == check_num(buf,i+0x30))){
                        buf[0]++;
                        buf[index++] = i + 0x30;
                    }
                }
            }
        }
    }
	#endif
}
#endif

#ifdef TY_SELF_MODE

#define TEST_GPIO_NUM     4
u32 gpio_pin[TEST_GPIO_NUM]={GPIO_PWM0,GPIO_PWM2, GPIO_PWM3,GPIO_PWM4};

void test_gpio(u8 *buf)
{
	
#if 1
	
    u8 temp = 0,j = 0,i = 0,index = 1,flag = 0;

    buf[0] = 0;
	
	ty_pwm_all_uninit();
	
    for(j=0;j<TEST_GPIO_NUM;j++){
		tuya_gpio_set_output_en(GPIO_PWM0, 0);
		tuya_gpio_set_output_en(GPIO_PWM2, 0);
		tuya_gpio_set_output_en(GPIO_PWM3, 0);
		tuya_gpio_set_output_en(GPIO_PWM4, 0);
		tuya_gpio_set_input_en(GPIO_PWM0, 1,1);
		tuya_gpio_set_input_en(GPIO_PWM2, 1,1);
		tuya_gpio_set_input_en(GPIO_PWM3, 1,1);
		tuya_gpio_set_input_en(GPIO_PWM4, 1,1);

        tuya_gpio_set_input_en(gpio_pin[j], 0, 0);
        tuya_gpio_set_output_en(gpio_pin[j], 1);
        tuya_gpio_write(gpio_pin[j], 0);
		
        for(i =0;i<TEST_GPIO_NUM;i++){
            if(j!=i){
                temp = tuya_gpio_read(gpio_pin[i]);
				
				/*
				memset(tp_buf,0x0,32);
				sprintf(tp_buf,"\r\npin%d:0,pin%d:%d\r\n",gpio_pin[j],gpio_pin[i],temp);
				ty_uart_send(strlen(tp_buf),tp_buf);
				*/
				
                if((0 == j)&&(2 == i)){
                    flag = 1;
                }
                else if((1 == j)&&(3 == i)){
                    flag = 1;
                }
                else if((2 == j)&&(0 == i)){
                    flag = 1;
                }
                else if((3 == j)&&(1 == i)){
                    flag = 1;
                }
                else{
                    if((0 == temp)&&(0 == check_num(buf,i+0x30))){//Ã»ÓÐÁ¬Í¨µÄ¹Ü½ÅÄ¬ÈÏÉÏÀ­£¬Ó¦¸ÃÎª¸ß¡£µÍËµÃ÷³ö´í¡£
                        buf[0]++;
                        buf[index++] = i + 0x30;
						
						ty_uart_send(strlen("       err1r\r\n"),"       err1r\r\n");
						
                    }
                }
                if(flag){
                    flag = 0;
                    if((1 == temp)&&(0 == check_num(buf,i+0x30))){//Á¬Í¨µÄ¹Ü½ÅÄ¬ÈÏÎª»á±»ÖÃµÍ£¬Ó¦¸ÃÎªµÍ¡£¸ßËµÃ÷³ö´í¡£
                        buf[0]++;
                        buf[index++] = i + 0x30;
						
						ty_uart_send(strlen("       err2r\r\n"),"       err2r\r\n");
                    }
                }
            }
        }
    }
#endif
	
}
#endif

///////////////////////////////////////////////gpio test ////////////////////////////////////////////////
#define CNT_OF(_x)			(sizeof(_x)/sizeof(_x[0]))
#define ALLCNT_OF(_x)		(CNT_OF(_x))*2

//TODO TEST real gpio pin
#if 1

u8 gpio_test_matrix[][2]={
	{16,19},
	{12,2},
	{11,3},
	{14,5},
	{15,4},
};

#else

u8 gpio_test_matrix[][2]={
	{GPIO_PWM0,GPIO_PWM3},
	{GPIO_PWM2,GPIO_PWM4},
};

#endif

void gpio_test_init_onece(void)
{
	u8 i;
	for(i = 0;i< CNT_OF(gpio_test_matrix);i++)
	{		
		tuya_gpio_set_output_en(gpio_test_matrix[i][0], 0);
		tuya_gpio_set_output_en(gpio_test_matrix[i][1], 0);

		tuya_gpio_set_input_en(gpio_test_matrix[i][0], 1,1);
		tuya_gpio_set_input_en(gpio_test_matrix[i][1], 1,1);
	}
}

void gpio_test_run(u8 *buf)
{
	u8 i,j,temp,index = 1;
	
	//change to One-dimensional array 
	u8 *convert_gpio_test_pin = (u8*)gpio_test_matrix; 
	for(i = 0;i<ALLCNT_OF(gpio_test_matrix);i++)
	{
		//init every time
		gpio_test_init_onece();
		
		//output high
		tuya_gpio_set_input_en(convert_gpio_test_pin[i], 0, 0);
        tuya_gpio_set_output_en(convert_gpio_test_pin[i], 1);
        tuya_gpio_write(convert_gpio_test_pin[i], 0);
		
		for(j = 0;j<ALLCNT_OF(gpio_test_matrix);j++)
		{
			if(j!=i)//not same pin
			{
				temp = tuya_gpio_read(convert_gpio_test_pin[j]);
				
				u8 val = ((u8)(i/2) == (u8)(j/2));//ÅÐ¶ÏÊÇ·ñÎªÁ¬Í¨×é
				
				if(val)
				{
                    if((1 == temp)&&(0 == check_num(buf,i+0x30))){//Á¬Í¨µÄ¹Ü½ÅÄ¬ÈÏÎª»á±»ÖÃµÍ£¬Ó¦¸ÃÎªµÍ¡£¸ßËµÃ÷³ö´í¡£
                        buf[0]++;
                        buf[index++] = i + 0x30;
						
						ty_uart_send(strlen("       err2r\r\n"),"       err2r\r\n");
                    }
				}
				else
				{
					if((0 == temp)&&(0 == check_num(buf,i+0x30))){//Ã»ÓÐÁ¬Í¨µÄ¹Ü½ÅÄ¬ÈÏÉÏÀ­£¬Ó¦¸ÃÎª¸ß¡£µÍËµÃ÷³ö´í¡£
                        buf[0]++;
                        buf[index++] = i + 0x30;
						
						ty_uart_send(strlen("       err1r\r\n"),"       err1r\r\n");
						
                    }
				}
			}
		}
	}
}

////////////////////////////////////////////gpio test end////////////////////////////////////////////////

////////////////////////////////////////fac_test_cmd_resp api////////////////////////////////////////////
void tuya_factory_test_auth_rf_resp(BOOL suc_flag,s8 rssi)
{
	u8 *alloc_buf = NULL;
	alloc_buf = ty_malloc(60);
	memset(alloc_buf,0x0,60);

	if(suc_flag)
	{
		sprintf(alloc_buf,"{\"ret\":true,\"rssi\":\"%d\"}",rssi);
	}
	else
	{
		sprintf(alloc_buf,"{\"ret\":flase}");
	}

	ty_uart_protocol_send(0x08, alloc_buf,strlen(alloc_buf));
	
	ty_free(alloc_buf);
}

////////////////////////////////dev scan static/////////////////////////////////

static data_stats_t auth_rf_test_rssi_stats={0};

static BOOL auth_rf_test_begin_stats_flag = FALSE;

BOOL tuya_auth_rf_test_begin_stats_flag_get(void)
{
	return auth_rf_test_begin_stats_flag;
}

void tuya_auth_rf_test_begin_stats_flag_set(BOOL flag)
{
	auth_rf_test_begin_stats_flag = flag;
}

void tuya_auth_rf_test_rssi_stats_add(s8 rssi)
{
	if( auth_rf_test_rssi_stats.index >= MAX_BUFFER_DATA )
	{
		s8 old_avg = tuya_auth_rf_test_calc_avg();
		tuya_auth_rf_test_rssi_stats_reset();
		
		auth_rf_test_rssi_stats.data[0] = old_avg;
	}
	auth_rf_test_rssi_stats.data[auth_rf_test_rssi_stats.index++] = (u8)rssi;
}

void tuya_auth_rf_test_rssi_stats_reset(void)
{
	memset(&auth_rf_test_rssi_stats,0x0,sizeof(data_stats_t));//reset stats data
}

s8 tuya_auth_rf_test_calc_avg(void)
{
	s16 sum = 0;
	u8 i = 0;
	for(i = 0 ; i < auth_rf_test_rssi_stats.length ; i++)
	{
		sum += (s8)(auth_rf_test_rssi_stats.data[i]);
	}
	
	return (s8)(sum/(auth_rf_test_rssi_stats.length));
}

////////////////////////////////dev scan static end/////////////////////////////////



////////////////////////////////////////dev scan api/////////////////////////////////////////////////////

#define FACTORY_TEST_BEACON_NAME		"ty_mdev"
#define AUTH_RF_TEST_BEACON_NAME		"ty_prod"

static scan_type_t scan_test_type = FACTORY_TEST;

scan_type_t tuya_factory_test_scan_type_get(void)
{
	return scan_test_type;
}

void tuya_start_factory_test_scan(scan_type_t test_type)
{
	scan_test_type = test_type;
	ty_ble_scan_stop();
	ty_ble_scan_start();
	ty_timer_start(TIMER_SCAN,5000); //scan time out 5s
}

//NEED refator???

static tuya_factory_test_cb_fun cb_fun = NULL;

void tuya_factorytest_on_scanrsp(u8 *dev_name, u8 dev_name_len, s8 dev_rssi)
{
	u8 tp_buf[32]={0};
	memcpy(tp_buf,dev_name,dev_name_len);
	
	//int rssi = -((((u16)-1)-dev_rssi)+1);
	
	int rssi = dev_rssi;
	
	/////////////////////////////
	if(strlen(tp_buf) > 1)
	{
//		printf("dev_name:%s,\tdev_rssi:%d\r\n",tp_buf,rssi);
	}
	
	/////////////////////////////
	
	scan_type_t type = tuya_factory_test_scan_type_get();
	
	if(type == FACTORY_TEST)//factory test
	{
		if( (rssi > -70) && (memcmp(dev_name,FACTORY_TEST_BEACON_NAME,dev_name_len) == 0) )
		{
			tuya_factory_test_cb_fun cb = tuya_factory_test_get_cb();
			if(cb != NULL)
			{
				ty_timer_stop(TIMER_SCAN);
				ty_ble_scan_stop();
				cb(1,rssi);
			}
		}
	}
	else if(type == AUTH_RF_TEST)//auth rf test
	{
		if((memcmp(dev_name,AUTH_RF_TEST_BEACON_NAME,dev_name_len) == 0) )
		{
			if(tuya_auth_rf_test_begin_stats_flag_get())
			{
				tuya_auth_rf_test_rssi_stats_add(rssi);
			}
			else
			{
				//begin calc
				tuya_auth_rf_test_begin_stats_flag_set(TRUE);
				
				tuya_auth_rf_test_rssi_stats_reset();
				
				ty_timer_start(TIMER_SCAN_TEST,1000);
				
				tuya_auth_rf_test_rssi_stats_add(rssi);//add frist one
			}
			
		}
	}

}

tuya_factory_test_cb_fun tuya_factory_test_get_cb(void)
{
	return cb_fun;
}

u8 tuya_user_app_regist_factory_test_cb(tuya_factory_test_cb_fun cb)
{
	cb_fun = cb;
}