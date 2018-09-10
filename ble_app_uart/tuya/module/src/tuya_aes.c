/*
 * tuya_aes.c
 *
 *  Created on: 2017-9-6
 *      Author: echo
 */

#include "../include/tuya_aes.h"

extern b_ble_para ble_para;

//补全为16字节对齐，用于aes block encode，16字节为一个block
//假如len过大，会覆盖到后续buf node地址，bug here
u8 Add_Pkcs(u8 *p, u8 len)
{
    u8 *pkcs = NULL;
    u8 i = 0, cz = 16-len%16;

    pkcs = (u8*)ty_malloc(16);
    memset(pkcs, 0, sizeof(pkcs));
    for( i=0; i<cz; i++ ) {
        pkcs[i]=cz;
    }
    memcpy(p + len, pkcs, cz);
    ty_free(pkcs);
    return (len + cz);
}

void ty_aes_encrypt(u8 *in_buf,u8 in_len,u8 *out_buf)
{
    u8 i = 0,tmplen = 0;

    u8 *alloc_buf = ty_malloc(in_len*2+1+16);//申请空间需要多申请最大16字节空间

    if(alloc_buf != NULL){
        hextoascii(in_buf,in_len,alloc_buf);
        tmplen = Add_Pkcs(alloc_buf,in_len*2+1);
        for(i = 0;i < tmplen;i += 16) {
            AES128_ECB_encrypt(&alloc_buf[i],ble_para.session_key,&out_buf[1+i]);
        }
        ty_free(alloc_buf);
        out_buf[0] = tmplen;
    }
}

void ty_aes_decrypt(u8 *in_buf,u8 in_len,u8 *out_buf)
{
    if((in_len%16) != 0)  {
        return;
    }
    u8 i = 0;
    u8 *alloc_buf = ty_malloc(in_len);
    if(alloc_buf != NULL){
        for(i = 0;i < in_len;i += 16) {
            AES128_ECB_decrypt(in_buf+i,ble_para.session_key,alloc_buf+i);
        }
        asciitohex(alloc_buf,out_buf);
        ty_free(alloc_buf);
    }
}
