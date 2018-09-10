/***********************************************************
*  File: mutli_tsf_protocol.c
*  Author: nzy
*  Date: 20160727
***********************************************************/
#define __MUTLI_TSF_PROTOCOL_GLOBALS
#include "../include/mutli_tsf_protocol.h"
#include "../../port/include/port.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define MTP_DEBUG 1

#if MTP_DEBUG
#define MTP_PR_DEBUG(_fmt_, ...) \
	printf("[mtp:dbg]%s:%d "_fmt_"\n\r", __FILE__, __LINE__, ##__VA_ARGS__)
#define MTP_PR_DEBUG_RAW(_fmt_, ...) \
	printf(_fmt_, ##__VA_ARGS__)
#else
#define MTP_PR_DEBUG(...)
#define MTP_PR_DEBUG_RAW(_fmt_, ...)
#endif

#define MTP_PR_NOTICE(_fmt_, ...) \
	printf("[mtp:notice]%s:%d "_fmt_"\n\r", __FILE__, __LINE__, ##__VA_ARGS__)
#define MTP_PR_ERR(_fmt_, ...) \
	printf("[mtp:err]%s:%d "_fmt_"\n\r", __FILE__, __LINE__, ##__VA_ARGS__)

#undef NULL 
#define NULL (void *)0
/***********************************************************
*************************variable define********************
***********************************************************/
static frame_seq_t frame_seq = 0;

/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: create_trsmitr_init
*  description: create a transmitter and initialize
*  Input: none
*  Output: 
*  Return: transmitter handle
***********************************************************/
frm_trsmitr_proc_s *create_trsmitr_init(void)
{
	frm_trsmitr_proc_s *frm_trsmitr = NULL;

	frm_trsmitr = (frm_trsmitr_proc_s *)ty_malloc(sizeof(frm_trsmitr_proc_s));
	if ((void *)0 == frm_trsmitr) {
		return (void *)0;
	}
	memset(frm_trsmitr, 0, sizeof(frm_trsmitr_proc_s));

	return frm_trsmitr;
}

/***********************************************************
*  Function: trsmitr_init
*  description: init a transmitter
*  Input: transmitter handle
*  Output: 
*  Return: 
***********************************************************/
void trsmitr_init(frm_trsmitr_proc_s *frm_trsmitr)
{
	memset(frm_trsmitr, 0, sizeof(frm_trsmitr_proc_s));
}

/***********************************************************
*  Function: delete_trsmitr
*  description: delete transmitter
*  Input: transmitter handle
*  Output: 
*  Return: 
***********************************************************/
void delete_trsmitr(frm_trsmitr_proc_s *frm_trsmitr)
{
	free(frm_trsmitr);
}

/***********************************************************
*  Function: get_trsmitr_frame_total_len
*  description: get a transmitter total data len
*  Input: transmitter handle
*  Output: 
*  Return: frame_total_t
***********************************************************/
frame_total_t get_trsmitr_frame_total_len(frm_trsmitr_proc_s *frm_trsmitr)
{
	return frm_trsmitr->total;
}

/***********************************************************
*  Function: get_trsmitr_frame_type
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: frame_type_t
***********************************************************/
frame_type_t get_trsmitr_frame_type(frm_trsmitr_proc_s *frm_trsmitr)
{
	return frm_trsmitr->type;
}

/***********************************************************
*  Function: get_trsmitr_frame_seq
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: frame_seq_t
***********************************************************/
frame_seq_t get_trsmitr_frame_seq(frm_trsmitr_proc_s *frm_trsmitr)
{
	return frm_trsmitr->seq;
}

/***********************************************************
*  Function: get_trsmitr_subpkg_len
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: frame_subpkg_len_t
***********************************************************/
frame_subpkg_len_t get_trsmitr_subpkg_len(frm_trsmitr_proc_s *frm_trsmitr)
{
	return frm_trsmitr->subpkg_len;
}

/***********************************************************
*  Function: get_trsmitr_subpkg
*  description: 
*  Input: transmitter handle
*  Output: 
*  Return: subpackage buf
***********************************************************/
unsigned char *get_trsmitr_subpkg(frm_trsmitr_proc_s *frm_trsmitr)
{
	return frm_trsmitr->subpkg;
}

static frame_seq_t get_frame_seq(void)
{
	return (frame_seq >= FRAME_SEQ_LMT) ? 0 : frame_seq++;
}

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
mtp_ret trsmitr_send_pkg_encode(frm_trsmitr_proc_s *frm_trsmitr, frame_type_t type, unsigned char *buf, unsigned int len)
{
	if (((void *)0) == frm_trsmitr) {
		return MTP_INVALID_PARAM;
	}

	if (FRM_PKG_INIT == frm_trsmitr->pkg_desc) {
		frm_trsmitr->total = len;
		frm_trsmitr->type = type;
		frm_trsmitr->seq = get_frame_seq();
		frm_trsmitr->subpkg_num = 0;
		frm_trsmitr->pkg_trsmitr_cnt = 0;
	}

	if (frm_trsmitr->subpkg_num >= 0x10000000 || len >= 0x10000000) {
		return MTP_COM_ERROR;
	}

	unsigned char sunpkg_offset = 0;

	// package code
	// subpackage num encode
	int i;
	unsigned int tmp = 0;
	tmp = frm_trsmitr->subpkg_num;
	for (i = 0; i < 4; i++) {
		frm_trsmitr->subpkg[sunpkg_offset] = tmp % 0x80;
		if ((tmp / 0x80)) {
			frm_trsmitr->subpkg[sunpkg_offset] |= 0x80;
		}
		sunpkg_offset++;
		tmp /= 0x80;
        if(0 == tmp) {
            break;
        }
	}

	// the first package include the frame total len
	if (0 == frm_trsmitr->subpkg_num) {
		// frame len encode
		tmp = len;
		for (i = 0; i < 4; i++) {
			frm_trsmitr->subpkg[sunpkg_offset] = tmp % 0x80;
			if ((tmp / 0x80)) {
				frm_trsmitr->subpkg[sunpkg_offset] |= 0x80;
			}
			sunpkg_offset++;
			tmp /= 0x80;
            if(0 == tmp) {
                break;
            }
		}

		// frame type and frame seq
		frm_trsmitr->subpkg[sunpkg_offset++] = (frm_trsmitr->type << 0x04) | (frm_trsmitr->seq & 0x0f);
	}
	
	// frame data transfer
	unsigned char send_data = (SNGL_PKG_TRSFR_LMT - sunpkg_offset);
	if ((len - frm_trsmitr->pkg_trsmitr_cnt) < send_data) {
		send_data = len - frm_trsmitr->pkg_trsmitr_cnt;
	}

	memcpy(&(frm_trsmitr->subpkg[sunpkg_offset]), buf + frm_trsmitr->pkg_trsmitr_cnt, send_data);
	frm_trsmitr->subpkg_len = sunpkg_offset+send_data;
	
	frm_trsmitr->pkg_trsmitr_cnt += send_data;
	if (0 == frm_trsmitr->subpkg_num) {
		frm_trsmitr->pkg_desc = FRM_PKG_FIRST;
	}else {
		frm_trsmitr->pkg_desc = FRM_PKG_MIDDLE;
	}

	if (frm_trsmitr->pkg_trsmitr_cnt < frm_trsmitr->total) {
		frm_trsmitr->subpkg_num++;
		return MTP_TRSMITR_CONTINUE;
	}

	frm_trsmitr->pkg_desc = FRM_PKG_END;
	return MTP_OK;
}

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
mtp_ret trsmitr_recv_pkg_decode(frm_trsmitr_proc_s *frm_trsmitr, unsigned char *raw_data, unsigned char raw_data_len)
{
	if (NULL == raw_data || \
        (raw_data_len > SNGL_PKG_TRSFR_LMT) || \
        NULL == frm_trsmitr) {
		return MTP_INVALID_PARAM;
	}

	if (FRM_PKG_INIT == frm_trsmitr->pkg_desc) {
		frm_trsmitr->total = 0;
		frm_trsmitr->type = 0;
		frm_trsmitr->seq = 0;
		frm_trsmitr->pkg_trsmitr_cnt = 0;
	}

	unsigned char sunpkg_offset = 0;
	// package code
	// subpackage num decode
	int i;
	unsigned int multiplier = 1;
	unsigned char digit;
	frame_subpkg_num_t subpkg_num = 0;

	for (i = 0; i < 4; i++) {
		digit = raw_data[sunpkg_offset++];
		subpkg_num += (digit & 0x7f) * multiplier;
		multiplier *= 0x80;

		if (0 == (digit & 0x80)) {
			break;
		}
	}

    if(0 == subpkg_num) {
        frm_trsmitr->total = 0;
		frm_trsmitr->type = 0;
		frm_trsmitr->seq = 0;
		frm_trsmitr->pkg_trsmitr_cnt = 0;
        frm_trsmitr->pkg_desc = FRM_PKG_FIRST;
    }else {
        frm_trsmitr->pkg_desc = FRM_PKG_MIDDLE;
    }

	if (frm_trsmitr->subpkg_num >= 0x10000000) {
		return MTP_COM_ERROR;
	}
	// is receive the subpackage num valid?
	if (frm_trsmitr->pkg_desc != FRM_PKG_FIRST) {
		if (subpkg_num < frm_trsmitr->subpkg_num) {
			return MTP_TRSMITR_ERROR;
		}else if (subpkg_num == frm_trsmitr->subpkg_num){
			return MTP_TRSMITR_CONTINUE;
		}

		if (subpkg_num - frm_trsmitr->subpkg_num > 1) {
			return MTP_TRSMITR_ERROR;
		}
	}
	frm_trsmitr->subpkg_num = subpkg_num;

	if (0 == frm_trsmitr->subpkg_num) {
		// frame len decode
		multiplier = 1;
		for (i = 0; i < 4; i++) {
			digit = raw_data[sunpkg_offset++];
			frm_trsmitr->total += (digit & 0x7f) * multiplier;
			multiplier *= 0x80;

			if (0 == (digit & 0x80)) {
				break;
			}
		}

		if (frm_trsmitr->total >= 0x10000000) {
			return MTP_COM_ERROR;
		}

		// frame type and frame seq decode 
		frm_trsmitr->type = (raw_data[sunpkg_offset] & FRM_TYPE_OFFSET) >> 4;
		frm_trsmitr->seq = raw_data[sunpkg_offset++] & FRM_SEQ_OFFSET;
	}

    unsigned char recv_data = raw_data_len - sunpkg_offset;
    if((frm_trsmitr->total - frm_trsmitr->pkg_trsmitr_cnt) < recv_data) {
        recv_data = frm_trsmitr->total - frm_trsmitr->pkg_trsmitr_cnt;
    }

    // decode data cp to transmitter subpackage buf
    memcpy(frm_trsmitr->subpkg, &raw_data[sunpkg_offset], recv_data);
    frm_trsmitr->subpkg_len = recv_data;    
    frm_trsmitr->pkg_trsmitr_cnt += recv_data;

	if (frm_trsmitr->pkg_trsmitr_cnt < frm_trsmitr->total) {
		return MTP_TRSMITR_CONTINUE;
	}
	frm_trsmitr->pkg_desc = FRM_PKG_END;
	return MTP_OK;
}

/***********************************************************
*  Function: free_klv_list
*  description: 
*  Input: list
*  Output: 
*  Return: 
***********************************************************/
void free_klv_list(klv_node_s *list)
{
	if (NULL == list) {
		return;
	}

	klv_node_s *node = list;
	klv_node_s *next_node = NULL;

	do {
		next_node = node->next;
		//free(node);
        ty_free((u8*)node);
		node = next_node;
	} while (node);
}

/***********************************************************
*  Function: make_klv_list
*  description: 
*  Input: 
*  Output: 
*  Return: 
***********************************************************/
klv_node_s *make_klv_list(klv_node_s *list,unsigned char id,dp_type type,\
                          void *data,unsigned char len)
{
    if(NULL == data || type >= DT_LMT) {
        return NULL;
    }

    if(DT_VALUE == type && DT_VALUE_LEN != len) {
        goto err_ret;
    }else if(DT_BITMAP == type && len > DT_BITMAP_MAX) {
        goto err_ret;
    }else if(DT_BOOL == type && DT_BOOL_LEN != len) {
        goto err_ret;
    }else if(DT_ENUM == type && DT_ENUM_LEN != len) {
        goto err_ret;
    }

    //klv_node_s *node = (klv_node_s *)malloc(sizeof(klv_node_s)+len);
    klv_node_s *node = (klv_node_s *)ty_malloc(sizeof(klv_node_s)+len);
	if (NULL == node) {
        goto err_ret;
    }
	memset(node, 0, sizeof(klv_node_s));

    node->id = id;
    node->len = len;
    node->type = type;

    if(DT_VALUE == type || \
       DT_BITMAP == type) { // change to big-end
        unsigned int tmp = *(unsigned int *)data;
        unsigned char shift = 0;
        
        int i;
        for(i = 0;i < len;i++) {
            node->data[len-i-1] = (tmp >> shift) & 0xff;
            shift += 8;
        }
    }else {
        memcpy((void *restrict)node->data,(unsigned char*)data,len);
    }
    node->next = list;
    
    return node;

err_ret:
    free_klv_list(list);
    return NULL;
}

/***********************************************************
*  Function: klvlist_2_data
*  description: 
*  Input: 
*  Output: 
*  Return: 
***********************************************************/
mtp_ret klvlist_2_data(klv_node_s *list,unsigned char **data,unsigned int *len,unsigned char type)
{
    if(NULL == list || NULL == data || NULL == len) {
        return MTP_INVALID_PARAM;
    }

    klv_node_s *node = list;

    // count data len 
    unsigned int mk_data_len = 0;
    while(node) {
        mk_data_len += sizeof(klv_node_s)+node->len-sizeof(struct s_klv_node *);
        node = node->next;
    }

    unsigned char *mk_data = (unsigned char *)ty_malloc(mk_data_len);
    if(NULL == mk_data) {
        return MTP_MALLOC_ERR;
    }

    // fill data 
    unsigned int offset = 0;
    node = list;
    while(node) {
        mk_data[offset++] = node->id;
        mk_data[offset++] = node->type;
        if(1 == type){
            mk_data[offset++] = 0;
            mk_data[offset++] = node->len;
        }
        else
        {
            mk_data[offset++] = node->len;
        }
        memcpy((void *restrict)&mk_data[offset],(const void *restrict)node->data,node->len);
        offset += node->len;
        node = node->next;
    }
    *len = offset;
    *data = mk_data;
    
    return MTP_OK;
}

/***********************************************************
*  Function: data_2_klvlist
*  description: 
*  Input: 
*  Output: 
*  Return: 
***********************************************************/
mtp_ret data_2_klvlist(unsigned char *data,unsigned int len,klv_node_s **list,unsigned char type)
{
    if(NULL == data || NULL == list) {
        return MTP_INVALID_PARAM;
    }

    unsigned int offset = 0;
    klv_node_s *klv_list = NULL;
    klv_node_s *node = NULL;
    do {
        // not full klv
        if((len-offset) < (sizeof(klv_node_s)-sizeof(struct s_klv_node *))) {
            break;
        }
        
        node = (klv_node_s *)ty_malloc(sizeof(klv_node_s)+data[(2+offset)]);
        if(NULL == node) {
            free_klv_list(klv_list);
            return MTP_MALLOC_ERR;
        }
        memset(node,0,sizeof(sizeof(klv_node_s)));

        node->id = data[offset++];
        node->type = data[offset++];
        if(1 == type){
            offset++;
            node->len = data[offset++];
        }
        else{
            node->len = data[offset++];
        }       
        if((len-offset) < node->len) { // is remain data len enougn?
            free_klv_list(klv_list);
            return MTP_COM_ERROR;
        }
        
        memcpy((void *restrict)node->data,&data[offset],node->len);
        offset += node->len;

        node->next = klv_list;
        klv_list = node;
    }while(offset < len);

    if(NULL == klv_list) {
        return MTP_COM_ERROR;
    }
    *list = klv_list;
    return MTP_OK;
}

#if 0
typedef struct {
	unsigned char len;
	unsigned char buf[20];
}package_p_s;

package_p_s package_buf[1024];
unsigned char data_buf[500];
unsigned char recv_data_buf[500];

int main(void)
{
	unsigned int i;
	for (i = 0; i < sizeof(data_buf); i++){
		data_buf[i] = i & 0xff;
	}

    // data send and receive sample
	frm_trsmitr_proc_s *trsmitr_proc = create_trsmitr_init();
	if ((void *)0 == trsmitr_proc) {
		return -1;
	}

	unsigned int cnt = 0;
	mtp_ret ret;
	do {
		ret = trsmitr_send_pkg_encode(trsmitr_proc, PAIR_REQ, data_buf, sizeof(data_buf));
		if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
			return -2;
		}

		package_buf[cnt].len = get_trsmitr_subpkg_len(trsmitr_proc);
		memcpy(package_buf[cnt].buf,get_trsmitr_subpkg(trsmitr_proc),get_trsmitr_subpkg_len(trsmitr_proc));
        cnt++;
	} while (ret == MTP_TRSMITR_CONTINUE);

    unsigned char decode[20];
    unsigned char decode_len = 0;
    unsigned int offset = 0;
    trsmitr_init(trsmitr_proc);
    for(i = 0;i < cnt;i++) {
        ret = trsmitr_recv_pkg_decode(trsmitr_proc, package_buf[i].buf, package_buf[i].len);
		if (MTP_OK != ret && MTP_TRSMITR_CONTINUE != ret) {
			return -3;
		}
        memcpy(recv_data_buf+offset,get_trsmitr_subpkg(trsmitr_proc),get_trsmitr_subpkg_len(trsmitr_proc));
		offset += get_trsmitr_subpkg_len(trsmitr_proc);

        if(ret == MTP_OK) {
            break;
        }
    }

    // make klv list test 
    klv_node_s *list = NULL;
    int test = -100;
    list = make_klv_list(list,1,DT_VALUE,&test,sizeof(int));
    if(NULL == list) {
        return -4;
    }

    unsigned char bl = 1;
    list = make_klv_list(list,2,DT_BOOL,&bl,sizeof(unsigned char));
    if(NULL == list) {
        return -4;
    }

    unsigned char raw[5] = {1,2,3,4,5};
    list = make_klv_list(list,3,DT_RAW,raw,sizeof(raw));
    if(NULL == list) {
        return -4;
    }

    // klvlist_2_data test
    unsigned char *data;
    unsigned int da_len;
    ret = klvlist_2_data(list,&data,&da_len);
    if(MTP_OK != ret) {
        return -5;
    }
    free_klv_list(list);
    list = NULL;
    
    // data_2_klvlist test
    ret = data_2_klvlist(data,da_len,&list);
    if(MTP_OK != ret) {
        return -6;
    }

    free_klv_list(list);
    free(data);
}
#endif

