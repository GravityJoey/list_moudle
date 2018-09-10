/*
 * tuya_data_exchange.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#include "../include/tuya_data_exchange.h"

extern u8 ty_ble_send_flag;
extern r_air_recv_packet air_recv_packet;
extern frm_trsmitr_proc_s ty_trsmitr_proc;

u8 ty_report_mux_packet(u8 type)
{
    mtp_ret ret;
    u8 send_len = 0;
    u8 *p_buf = NULL;

	ty_ble_send_flag = 1;
    ret = trsmitr_send_pkg_encode(&ty_trsmitr_proc,type, &air_recv_packet.de_encrypt_buf[1], air_recv_packet.de_encrypt_buf[0]);
    if(MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
		ty_ble_send_flag = 0;
        return 0;
    }
    send_len = get_trsmitr_subpkg_len(&ty_trsmitr_proc);
    p_buf = (u8*)ty_malloc(send_len);
    memcpy(p_buf,get_trsmitr_subpkg(&ty_trsmitr_proc),send_len);
    ty_ble_notify(send_len,p_buf);
    ty_free(p_buf);
    if(MTP_OK == ret)
    {
	    ty_ble_send_flag = 0;
		return 0;
	}
    else
	{
		return 1;
	}
}

void ty_air_send(u8 len,u8 *buf,u8 type)
{
    mtp_ret ret;
    u8 send_len = 0;
    u8 *p_buf = NULL;
	
	//tuya_log(0x3C,len,buf,type);
	
    trsmitr_init(&ty_trsmitr_proc);
    do
    {
        ret = trsmitr_send_pkg_encode(&ty_trsmitr_proc,type, buf, len);
        if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
            return;
        }
        send_len = get_trsmitr_subpkg_len(&ty_trsmitr_proc);
        p_buf = ty_malloc(send_len);
        memcpy(p_buf,get_trsmitr_subpkg(&ty_trsmitr_proc),send_len);
        ty_ble_notify(send_len,p_buf);
        ty_free(p_buf);
    } while (ret == MTP_TRSMITR_CONTINUE);
	
}

void ty_uart_protocol_send(u8 type,u8 *pdata,u8 len)
{
    u8 i = 0;
    u8 *alloc_buf = ty_malloc(7+len);
    alloc_buf[0] = 0x55;
    alloc_buf[1] = 0xaa;
    alloc_buf[2] = 0x0;
    alloc_buf[3] = type;
    alloc_buf[4] = 0;
    alloc_buf[5] = len;
    for(i=0;i<len;i++)
        alloc_buf[6+i] = pdata[i];
    alloc_buf[6+len] = check_sum(alloc_buf,6+len);
    ty_uart_send(7+len,alloc_buf);
    ty_free(alloc_buf);
}
