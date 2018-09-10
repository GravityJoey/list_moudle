/*
 * tuya_fds.c
 *
 *  Created on: 2017-9-9
 *      Author: echo
 */
 
#include "tuya_fds.h"
#include "nrf_delay.h"
 
/****************************fds***********************************/
static volatile uint8_t write_flag = 0;
static volatile uint8_t fds_init_flag = 0;

//fds handler
void tuya_fds_evt_handler(fds_evt_t const * const p_fds_evt)
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                //FDS��ʼ���ɹ�
                printf("FDS init successfully! \r\n");
				printf("\r\n");
				fds_init_flag = 1;
            }
            break;
		case FDS_EVT_WRITE:
			if (p_fds_evt->result == FDS_SUCCESS)
			{
				write_flag=1;
			}
			break;
        default:
            break;
    }
}

//fds init
ret_code_t tuya_fds_init (void)
{
		//ע��FDS�¼�������ڵ���fds_init()����֮ǰ��һ��Ҫ��ע��
	  ret_code_t ret = fds_register(tuya_fds_evt_handler);
		if (ret != FDS_SUCCESS)
		{
				return ret;	
		}
		ret = fds_init();
		if (ret != FDS_SUCCESS)
		{
				return ret;
		}
		
		while (fds_init_flag==0);
		return NRF_SUCCESS;		
}

/**
* @param file id @ 0x0001~0xBFFF,
* @param key 	 @ 0x0001~0xBFFF,
**/
ret_code_t tuya_fds_write_record(uint16_t file_id,uint16_t key,uint32_t *data,uint8_t length)
{
		fds_record_t        record;
		fds_record_desc_t   record_desc;
		fds_record_chunk_t  record_chunk;
		
		//д������ݣ�����2�����֣���������ָ��ָ���д������ݣ��������ݳ���
		record_chunk.p_data         = data;
		record_chunk.length_words   = length;
		
		//д��ļ�¼����Ҫ�����ļ�ID����¼key�����ݿ�
		record.file_id              = file_id;
		record.key              		= key;
		record.data.p_chunks        = &record_chunk;
		record.data.num_chunks      = 1;
				
		ret_code_t ret = fds_record_write(&record_desc, &record);
		if (ret != FDS_SUCCESS)
		{
				return ret;
		}
		
		while (write_flag==0);
		
		//printf("Writing Record ID = %d \r\n",record_desc.record_id);
		//printf("\r\n");
		
		//printf("write file_id:%d\r\n",file_id);
		return NRF_SUCCESS;
}

ret_code_t tuya_fds_find_and_delete(uint16_t file_id,uint16_t key)
{
		fds_record_desc_t   record_desc;
		fds_find_token_t    ftok;
	
		ftok.page=0;
		ftok.p_addr=NULL;
		// Loop and find records with same ID and rec key and mark them as deleted. 
		while (fds_record_find(file_id, key, &record_desc, &ftok) == FDS_SUCCESS)
		{
			fds_record_delete(&record_desc);
			//uart_send_fifo_flag = 0;
		
			//printf("Deleted record ID: %d \r\n",record_desc.record_id);
			//while (uart_send_fifo_flag==0);
		}
		// call the garbage collector to empty them, don't need to do this all the time, this is just for demonstration
		ret_code_t ret = fds_gc();
		if (ret != FDS_SUCCESS)
		{
				return ret;
		}
		return NRF_SUCCESS;
}

ret_code_t tuya_fds_read_record(uint16_t file_id,uint16_t key,uint32_t *read_data)
{
		fds_flash_record_t  flash_record;
		fds_record_desc_t   record_desc;
		fds_find_token_t    ftok ={0};   //��ʼ��Ϊ0����ͷ��ʼ���ҡ�
		uint32_t err_code;
		
		//ʹ�ø����ļ�¼key���Ҹ����ļ��е�����ƥ��ļ�¼
		while (fds_record_find(file_id, key, &record_desc, &ftok) == FDS_SUCCESS)
		{
				//��֮ǰҪ�ȴ򿪼�¼
			  err_code = fds_record_open(&record_desc, &flash_record);
				if ( err_code != FDS_SUCCESS)
				{
					return err_code;		
				}
				
				//printf("Found Record ID = %d\r\n",record_desc.record_id);
				printf("Data = ");
				
				memset(read_data,0x0,4*flash_record.p_header->tl.length_words);
				memcpy(read_data,flash_record.p_data,4*(flash_record.p_header->tl.length_words));
				uint32_t *data;
				data = (uint32_t *) flash_record.p_data;
				
				
				for (uint8_t i=0;i<flash_record.p_header->tl.length_words;i++)
				{
					printf("0x%8x ",data[i]);
				}
				printf("\r\n");
				

				// �����һ���ǵùرռ�¼
				err_code = fds_record_close(&record_desc);
				if (err_code != FDS_SUCCESS)
				{
					return err_code;	
				}
		}
		return NRF_SUCCESS;
}
/******************************************************************/
