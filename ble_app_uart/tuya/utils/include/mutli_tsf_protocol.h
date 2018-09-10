/***********************************************************
*  File: mutli_tsf_protocol.h 
*  Author: nzy
*  Date: 20160727
***********************************************************/
#ifndef __MUTLI_TSF_PROTOCOL_H
	#define __MUTLI_TSF_PROTOCOL_H
    
#ifdef __cplusplus
	extern "C" {
#endif

#ifdef  __MUTLI_TSF_PROTOCOL_GLOBALS
	#define __MUTLI_TSF_PROTOCOL_EXT
#else
	#define __MUTLI_TSF_PROTOCOL_EXT extern
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#define SNGL_PKG_TRSFR_LMT 20 // single package transfer limit

#define FRM_TYPE_OFFSET (0x0f << 4)
#define FRM_SEQ_OFFSET (0x0f << 0)

// frame total len 
typedef unsigned int frame_total_t;

// frame subpackage num
typedef unsigned int frame_subpkg_num_t;

// frame type
typedef unsigned char frame_type_t;
#define FRM_QRY_DEV_INFO_REQ 0
#define FRM_QRY_DEV_INFO_RESP 0
#define PAIR_REQ 1
#define PAIR_RESP 1
#define FRM_CMD_SEND 2
#define FRM_CMD_ACK 2
#define FRM_STAT_REPORT 3
#define FRM_STAT_ACK 3
#define FRM_ALL_DP_QUERY 4
#define FRM_ALL_DP_QUERY_RESP 4
#define FRM_LOGIN_KEY_REQ 5
#define FRM_LOGIN_KEY_RESP 5
#define FRM_UNBONDING_REQ 6
#define FRM_UNBONDING_RESP 6
#define FRM_OTA_START_REQ    0x0c 
#define FRM_OTA_START_RESP    0x0c


// frame sequence
typedef unsigned char frame_seq_t;
#define FRAME_SEQ_LMT 20

// frame subpackage len
typedef unsigned char frame_subpkg_len_t;

// frame package description 
typedef unsigned char frm_pkg_desc_t;
#define FRM_PKG_INIT 0 // frame package init
#define FRM_PKG_FIRST 1 // frame package first
#define FRM_PKG_MIDDLE 2 // frame package middle
#define FRM_PKG_END 3 // frame package end

// mutil tsf ret code
typedef int mtp_ret;
#define MTP_OK 0
#define MTP_INVALID_PARAM 1
#define MTP_COM_ERROR 2
#define MTP_TRSMITR_CONTINUE 3
#define MTP_TRSMITR_ERROR 4
#define MTP_MALLOC_ERR 5

// frame transmitter process
typedef struct {
	frame_total_t total;//4
	frame_type_t type;//1
	frame_seq_t seq;//1
	frm_pkg_desc_t pkg_desc;//1
	frame_subpkg_num_t subpkg_num;//4
	unsigned int pkg_trsmitr_cnt; // package process count
	frame_subpkg_len_t subpkg_len;//1
	unsigned char subpkg[SNGL_PKG_TRSFR_LMT];
}frm_trsmitr_proc_s;

// dp type describe
typedef unsigned char dp_type;
#define DT_RAW 0
#define DT_BOOL 1
#define DT_VALUE 2
#define DT_STRING 3
#define DT_ENUM 4
#define DT_BITMAP 5
#define DT_LMT 6

#define DT_VALUE_LEN 4 // int
#define DT_BOOL_LEN 1
#define DT_ENUM_LEN 1
#define DT_BITMAP_MAX 4 // 1/2/4
#define DT_STR_MAX 255
#define DT_RAW_MAX 255


typedef struct s_klv_node{
    struct s_klv_node *next;
    unsigned char id;
    dp_type type;
    unsigned char len;
    unsigned char data[0];
}klv_node_s;


/***********************************************************
*************************variable define********************
***********************************************************/


/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: make_klv_list
*  description: 
*  Input: 
*  Output: 
*  Return: 
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
klv_node_s *make_klv_list(klv_node_s *list,unsigned char id,dp_type type,\
                          void *data,unsigned char len);

/***********************************************************
*  Function: free_klv_list
*  description: 
*  Input: list
*  Output: 
*  Return: 
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
void free_klv_list(klv_node_s *list);

/***********************************************************
*  Function: klvlist_2_data
*  description: 
*  Input: list
*  Output: data len
*  Return: 
*  Note:data need free.
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret klvlist_2_data(klv_node_s *list,unsigned char **data,unsigned int *len,unsigned char type);

/***********************************************************
*  Function: data_2_klvlist
*  description: 
*  Input: data len
*  Output: list
*  Return: 
*  Note: list need to call free_klv_list to free.
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret data_2_klvlist(unsigned char *data,unsigned int len,klv_node_s **list,unsigned char type);

/***********************************************************
*  Function: create_trsmitr_init
*  description: create a transmitter and initialize
*  Input: none
*  Output: 
*  Return: transmitter handle
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frm_trsmitr_proc_s *create_trsmitr_init(void);

/***********************************************************
*  Function: trsmitr_init
*  description: init a transmitter
*  Input: transmitter handle
*  Output: 
*  Return: 
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
void trsmitr_init(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: delete_trsmitr
*  description: delete transmitter
*  Input: transmitter handle
*  Output: 
*  Return: 
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
void delete_trsmitr(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_total_len
*  description: get a transmitter total data len
*  Input: transmitter handle
*  Output: 
*  Return: frame_total_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frame_total_t get_trsmitr_frame_total_len(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_type
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: frame_type_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frame_type_t get_trsmitr_frame_type(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_seq
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: frame_seq_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frame_seq_t get_trsmitr_frame_seq(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_subpkg_len
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: frame_subpkg_len_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frame_subpkg_len_t get_trsmitr_subpkg_len(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_subpkg
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: subpackage buf
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
unsigned char *get_trsmitr_subpkg(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: trsmitr_send_pkg_encode
*  description: frm_trsmitr->transmitter handle
*               type->frame type
*               buf->data buf
*               len->data len
*  Input: 
*  Output: 
*  Return: MTP_OK->buf send up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from encode data len and encode data by calling method 
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret trsmitr_send_pkg_encode(frm_trsmitr_proc_s *frm_trsmitr, frame_type_t type, unsigned char *buf, unsigned int len);

/***********************************************************
*  Function: trsmitr_recv_pkg_decode
*  description: frm_trsmitr->transmitter handle
*               raw_data->raw encode data
*               raw_data_len->raw encode data len
*  Input: 
*  Output: 
*  Return: MTP_OK->buf receive up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from decode data len and decode data by calling method 
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret trsmitr_recv_pkg_decode(frm_trsmitr_proc_s *frm_trsmitr, unsigned char *raw_data, unsigned char raw_data_len);


#ifdef __cplusplus
}
#endif
#endif


