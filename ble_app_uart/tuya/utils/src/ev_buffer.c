/**************************************************************************************************
  Filename:       ev_buffer.c
  Revised:        $Date: 2013-05-27 $

  Description:    Implementation of Telink EV Buffer Module
**************************************************************************************************/

#include "../include/ev_buffer.h"
#include "../include/mempool.h"
#include "../include/utility.h"
#include "../../types.h"
#include "../../port/include/port.h"
//#include "../../../proj/tl_common.h"
#ifdef WIN32
#include <malloc.h>
#endif

#define assert(ignore) ((void) 0)

#if (MODULE_BUFM_ENABLE)
/**************************** Private Variable Definitions *******************/
typedef struct {
	ev_bufItem_t *qHead;
	u16 size;
	u8 availBufNum;
	u8 reserved;
} ev_buf_groups_t;


typedef struct bufm_vars {
	ev_buf_groups_t bufGroups[DEFAULT_BUFFER_GROUP_NUM];
} ev_buf_vars_t;


/**
 *  @brief  Global variables definition for different ev_buf nums
 */
#if    EV_BUF_3_GROUP
MEMPOOL_DECLARE(size_0_pool, size_0_mem, BUFFER_GROUP_0, BUFFER_NUM_IN_GROUP0);
MEMPOOL_DECLARE(size_1_pool, size_1_mem, BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1);
MEMPOOL_DECLARE(size_2_pool, size_2_mem, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2);

ev_buf_vars_t ev_buf_vs = {
		{
			{(ev_bufItem_t*)&size_0_pool, BUFFER_GROUP_0, BUFFER_NUM_IN_GROUP0, 0}, \
			{(ev_bufItem_t*)&size_1_pool, BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1, 0},  \
			{(ev_bufItem_t*)&size_2_pool, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2, 0}
		}
};
ev_buf_vars_t *ev_buf_v = &ev_buf_vs;
const u32 ev_buf_mem_addr[DEFAULT_BUFFER_GROUP_NUM] = {(const u32)size_0_mem, (const u32)size_1_mem, (const u32)size_2_mem};


#elif EV_BUF_4_GROUP
MEMPOOL_DECLARE(size_0_pool, size_0_mem, BUFFER_GROUP_0, BUFFER_NUM_IN_GROUP0);
MEMPOOL_DECLARE(size_1_pool, size_1_mem, BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1);
MEMPOOL_DECLARE(size_2_pool, size_2_mem, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2);
MEMPOOL_DECLARE(size_3_pool, size_3_mem, BUFFER_GROUP_3, BUFFER_NUM_IN_GROUP3);


ev_buf_vars_t ev_buf_vs = {{{&size_0_pool, BUFFER_NUM_IN_GROUP0, BUFFER_GROUP_0}, \
		{&size_1_pool, BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1,0 }, \
		{&size_2_pool, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2, 0}, \
		{&size_3_pool,BUFFER_GROUP_3,  BUFFER_NUM_IN_GROUP3, 0}
	}
};

ev_buf_vars_t *ev_buf_v = &ev_buf_vs;
u32 ev_buf_mem_addr[DEFAULT_BUFFER_GROUP_NUM] = {size_0_mem, size_1_mem, size_2_mem, size_3_mem};


#elif EV_BUF_5_GROUP
MEMPOOL_DECLARE(size_0_pool, size_0_mem, BUFFER_GROUP_0, BUFFER_NUM_IN_GROUP0);
MEMPOOL_DECLARE(size_1_pool, size_1_mem, BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1);
MEMPOOL_DECLARE(size_2_pool, size_2_mem, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2);
MEMPOOL_DECLARE(size_3_pool, size_3_mem, BUFFER_GROUP_3, BUFFER_NUM_IN_GROUP3);
MEMPOOL_DECLARE(size_4_pool, size_4_mem, BUFFER_GROUP_4, BUFFER_NUM_IN_GROUP4);

ev_buf_vars_t ev_buf_vs = {{{&size_0_pool, BUFFER_GROUP_0, BUFFER_NUM_IN_GROUP0, 0}, \
		{&size_1_pool,BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1, 0}, \
		{&size_2_pool, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2, 0}, \
		{&size_3_pool, BUFFER_GROUP_3, BUFFER_NUM_IN_GROUP3, 0}, \
		{&size_4_pool, BUFFER_GROUP_4, BUFFER_NUM_IN_GROUP4, 0}
	}
};

ev_buf_vars_t *ev_buf_v = &ev_buf_vs;
u32 ev_buf_mem_addr[DEFAULT_BUFFER_GROUP_NUM] = {size_0_mem, size_1_mem, size_2_mem, size_3_mem, size_4_mem};

#elif EV_BUF_6_GROUP
MEMPOOL_DECLARE(size_0_pool, size_0_mem, BUFFER_GROUP_0, BUFFER_NUM_IN_GROUP0);
MEMPOOL_DECLARE(size_1_pool, size_1_mem, BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1);
MEMPOOL_DECLARE(size_2_pool, size_2_mem, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2);
MEMPOOL_DECLARE(size_3_pool, size_3_mem, BUFFER_GROUP_3, BUFFER_NUM_IN_GROUP3);
MEMPOOL_DECLARE(size_4_pool, size_4_mem, BUFFER_GROUP_4, BUFFER_NUM_IN_GROUP4);
MEMPOOL_DECLARE(size_5_pool, size_5_mem, BUFFER_GROUP_5, BUFFER_NUM_IN_GROUP5);

ev_buf_vars_t ev_buf_vs = {{{&size_0_pool, BUFFER_GROUP_0, BUFFER_NUM_IN_GROUP0, 0}, \
		{&size_1_pool, BUFFER_GROUP_1, BUFFER_NUM_IN_GROUP1, 0}, \
		{&size_2_pool, BUFFER_GROUP_2, BUFFER_NUM_IN_GROUP2, 0}, \
		{&size_3_pool, BUFFER_GROUP_3, BUFFER_NUM_IN_GROUP3, 0}, \
		{&size_4_pool, BUFFER_GROUP_4, BUFFER_NUM_IN_GROUP4, 0}, \
		{&size_5_pool, BUFFER_GROUP_5, BUFFER_NUM_IN_GROUP5, 0}
	}
};

ev_buf_vars_t *ev_buf_v = &ev_buf_vs;
u32 ev_buf_mem_addr[DEFAULT_BUFFER_GROUP_NUM] = {size_0_mem, size_1_mem, size_2_mem, size_3_mem, size_4_mem, size_5_mem};

#endif //end of (#if  EV_BUF_3_GROUP)


/*********************************************************************
 * @fn      ev_buf_isExisted
 *
 * @brief   Return whether the buffer is in the available buffer
 *
 * @param   pd - the pointer of a data, which is previously allocated
 *
 * @return  Pointer of bufferItem
 */
u8 ev_buf_isExisted(u8 index, mem_block_t *block) {
	mem_pool_t *pool = (mem_pool_t *)ev_buf_v->bufGroups[index].qHead;
	mem_block_t *curBlock = pool->free_list;

	while(curBlock) {
		if(block == curBlock) {
			return 1;
		}

		curBlock = curBlock->next_block;
	}

	return 0;
}

u8 *ev_buf_retriveMempoolHeader(u8 *pd) {
	return pd - (OFFSETOF(ev_bufItem_t, data) - OFFSETOF(mem_block_t, data));
}


/*********************************************************************
 * @fn      ev_buf_reset
 *
 * @brief   Reset the EV Buffer module
 *
 * @param   None
 *
 * @return  None
 */
void ev_buf_reset(void) {
	int i;
	mem_pool_t *pool;

	/*Pay attention that  ev_buf_reset should be called in ev_buf_init otherwise ev_buf_v may be changed during executation*/
	for(i = 0; i < DEFAULT_BUFFER_GROUP_NUM; i++) {
		pool = mempool_init((mem_pool_t*)ev_buf_v->bufGroups[i].qHead, (void*)ev_buf_mem_addr[i], ev_buf_v->bufGroups[i].size, ev_buf_v->bufGroups[i].availBufNum);
	}
}

/*********************************************************************
 * @fn      ev_buf_init
 *
 * @brief   Initialize the EV Buffer module
 *
 * @param   None
 *
 * @return  None
 */

/*Monitor EV BUF use num during stack running*/
#define TEST_EV_BUF_USE_NUM 0
#if TEST_EV_BUF_USE_NUM
u8 gg_ev_buf_min[6] = {0};
#endif
void ev_buf_init(void) {
#if TEST_EV_BUF_USE_NUM

	for(int i = 0; i < 6; i++) {
		gg_ev_buf_min[i] = 30;
	}

#endif
	ev_buf_reset();
}



/*********************************************************************
 * @fn      ev_buf_allocate
 *
 * @brief   Allocate an available buffer according to the requested size
 *          The allocated buffer will have only three kind of size, defined
 *          in @ref EV_BUFFER_CONSTANT
 *
 * @param   size - requested size
 *
 * @return  Pointer to an allocated buffer.
 *          NULL means the there is no available buffer.
 */

#if DEBUG_EV_BUFFER_FREE
u8 *my_ev_buf_allocate(u16 size, int line, char *file) {
#else
u8 *ev_buf_allocate(u16 size) {
#endif
	u8 index, i;
	ev_bufItem_t *pNewBuf;

	if(size > MAX_BUFFER_SIZE) {
		/* the size parameter is wrong */
		return NULL;
	}

	ty_irq_disable();

	/* find related the buffer blocks */
	for(i = 0; i < DEFAULT_BUFFER_GROUP_NUM; i++) {
		if(size <= ev_buf_v->bufGroups[i].size - OFFSETOF(ev_bufItem_t, data)) {
			index = i;
			break;
		}
	}

	if(!ev_buf_v->bufGroups[index].availBufNum) {
		/* no available buffer */
		ty_irq_enable();
#if DEBUG_EV_BUFFER_FREE
            printf("EV ALLO Fail , size=%d, index=%d, l:%d, %s\n", size, index, line, file);
#endif
		return NULL;
	}

	ev_buf_v->bufGroups[index].availBufNum--;
	
	//FIXME 有时，即使有可用空间，对应freelist也为空。内存分配bug
	if(((mem_pool_t *)ev_buf_v->bufGroups[index].qHead)->free_list == 0)
	{
		return NULL;
	}

#if TEST_EV_BUF_USE_NUM

	if((ev_buf_v->bufGroups[index].availBufNum) < gg_ev_buf_min[index]) {
		gg_ev_buf_min[index] = (ev_buf_v->bufGroups[index].availBufNum);
	}

#endif


	u8 *temp = (u8 *)mempool_alloc((mem_pool_t *)ev_buf_v->bufGroups[index].qHead);
	if(temp == NULL)
	{
		ev_buf_v->bufGroups[index].availBufNum++;
		return NULL;
	}
	
	pNewBuf = (ev_bufItem_t *)(temp - 4);
	pNewBuf->groupIndex = index;

#if DEBUG_EV_BUFFER_FREE

	//printf("EV ALLO l:%d, %s\n", line, file);
	//printf("E_A %d, %c, %c, %c\n", line, file[3], file[4], file[5]);
	/*Test every buffer group one by one*/
	//printf("A %d %d %x \n", ev_buf_v->bufGroups[index].availBufNum, line, (u32)(pNewBuf->data));


#endif

	ty_irq_enable();

	return pNewBuf->data;
}


/*********************************************************************
 * @fn      ev_buf_free
 *
 * @brief   Free the specified buffer
 *
 * @param   pBuf - the pointer to the specified buffer to free.
 *
 * @return  status
 */
#if DEBUG_EV_BUFFER_FREE
u8 AA_file_name[30];
volatile u32 AA_line_num = 0;
buf_sts_t my_ev_buf_free(u8 *pBuf, int line, char *file) { //test june
#else
buf_sts_t ev_buf_free(u8 *pBuf) {
#endif
	//u8 r;
	u16 buf_size = 0;
	ev_bufItem_t *pDelBuf;

	if(!pBuf) {
#if DEBUG_EV_BUFFER_FREE
		AA_line_num = line;
		memcpy(AA_file_name, (u8 *)file, 30);
#endif

		assert(0);

		return BUFFER_INVALID_PARAMETER;
	}

	ty_irq_disable();

	pDelBuf = ev_buf_getHead(pBuf);

	/*Check if index is in appropriate range*/
       if(pDelBuf->groupIndex >= DEFAULT_BUFFER_GROUP_NUM){
      
#if DEBUG_EV_BUFFER_FREE
		AA_line_num = line;
		memcpy(AA_file_name, (u8 *)file, 30);
#endif
            assert(0);
       } 

	/* check whether the buffer is duplicated release */
	if(ev_buf_isExisted(pDelBuf->groupIndex, (mem_block_t *)pDelBuf)) {
#if DEBUG_EV_BUFFER_FREE
		AA_line_num = line;
		memcpy(AA_file_name, (u8 *)file, 30);
#endif

		assert(0);
		ty_irq_enable();
		return BUFFER_DUPLICATE_FREE;
	}

	mempool_free((mem_pool_t *)ev_buf_v->bufGroups[pDelBuf->groupIndex].qHead, ev_buf_retriveMempoolHeader(pBuf));
	ev_buf_v->bufGroups[pDelBuf->groupIndex].availBufNum++;


#if (__LOG_RT_ENABLE__)

	if(pDelBuf->groupIndex == 2) {
		//log_event(TR_T_BUF_FREE);
	}

#endif

	buf_size = ev_buf_v->bufGroups[pDelBuf->groupIndex].size;
	memset(pBuf, 0, (buf_size - OFFSETOF(ev_bufItem_t, data)));

#if DEBUG_EV_BUFFER_FREE

	//printf("EV FREE l:%d, %s\n", line, file);
	//printf("E_F %d, %c, %c, %c\n", line, file[3], file[4], file[5]);
	/*Test every buffer group one by one*/
	#if 0
	if(pDelBuf->groupIndex == 1) {
		printf("F %d %d %x \n", ev_buf_v->bufGroups[pDelBuf->groupIndex].availBufNum, line, (u32)(pBuf));
	}
	#endif

#endif

	ty_irq_enable();

	return BUFFER_SUCC;
}


/*********************************************************************
 * @fn      ev_buf_getHead
 *
 * @brief   Get the header pointer of a buffer item
 *
 * @param   pd - the pointer of a data, which is previously allocated
 *
 * @return  Pointer of bufferItem
 */
ev_bufItem_t *ev_buf_getHead(u8 *pd) {
	return (ev_bufItem_t *)(pd - OFFSETOF(ev_bufItem_t, data));
}

/**
 * @brief       Get available buffer num for dedicated group
 *
 * @param       index - ev buffer goupe index
 *
 * @return      Availble buffer num in this group
 */
u8 ev_buf_getAvailNum(u8 index) {
	return ev_buf_v->bufGroups[index].availBufNum;
}


/*********************************************************************
 * @fn      ev_buf_isAddrValid
 *
 * @brief   Check if the address is a part of the ev buffer address
 *
 * @param   pBuf - the memory address to be checked
 *
 * @return    check result
 */
buf_sts_t ev_buf_isAddrValid(u8 *pBuf) {
	u32 ev_buf_startAddr = (u32)(size_0_mem);

#if EV_BUF_3_GROUP
	u32 ev_buf_endAddr = (u32)(size_2_mem) + sizeof(size_2_mem);

#elif EV_BUF_4_GROUP
	u32 ev_buf_endAddr = (u32)(size_3_mem) + sizeof(size_3_mem);

#elif EV_BUF_5_GROUP
	u32 ev_buf_endAddr = (u32)(size_4_mem) + sizeof(size_4_mem);

#elif EV_BUF_6_GROUP
	u32 ev_buf_endAddr = (u32)(size_5_mem) + sizeof(size_5_mem);
#endif

	if(((u32)pBuf >= ev_buf_startAddr) && ((u32)pBuf <= ev_buf_endAddr)) {
		return BUFFER_SUCC;

	}

	else {
		return BUFFER_ADDR_INVALID;
	}


}


/*********************************************************************
* @fn      ev_buffer_remain
*
 * @brief   Interface to get remaining dynamic buffer number
 *
 * @param   buff_len:	the kind of buffer you want check
 *
 * @return  the number of buffers remaining
 */
u8 ev_buffer_remain(u8 buff_len){
	if(buff_len<=SMALL_BUFFER){
		return ev_buf_getAvailNum(0);
	}else if(buff_len<=MEDIUM_BUFFER){
		return ev_buf_getAvailNum(1);
	}else if(buff_len<=LARGE_BUFFER){
		return ev_buf_getAvailNum(2);
	}
	return 0;
}

#endif  /* MODULE_BUFM_ENABLE */






