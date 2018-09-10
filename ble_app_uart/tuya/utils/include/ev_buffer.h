/**************************************************************************************************
  Filename:       ev_buffer.h
  Revised:        $Date: 2013-05-27 $

  Description:    Interface of Telink EV Buffer Module
**************************************************************************************************/

#pragma once

#define MODULE_BUFM_ENABLE  1

#if (MODULE_BUFM_ENABLE)

#include "../../types.h"


//for ev buffer debug
#define DEBUG_EV_BUFFER_FREE       0

#define BUFFER_HDR_SIZE                  8
/** @addtogroup  TELINK_COMMON_MODULE TELINK Common Module
 *  @{
 */

/** @addtogroup  EV_BUFFER EV Buffer
 *  @{
 */

/** @defgroup EV_BUFFER_CONSTANT EV Buffer Constants
 *  @{
 */


/** @addtogroup ev_buffer_groups EV Buffer Groups
 * Definition the length of each buffer group
 * @{
 */

/**
 * @brief Default buffer groups num,should be defined in app_config.h,
 * only one of following options can be enabled
 * #define EV_BUF_3_GROUP  1
 * #define EV_BUF_4_GROUP  0
 * #define EV_BUF_5_GROUP  0
 * #define EV_BUF_6_GROUP  0
 */


/**
 * @brief Default buffer number in each group (it is valid only if it is in the
 *  range of buffer groups num, ex, BUFFER_GROUP_4 is not available if
 *  EV_BUF_3_GROUP is enabled
 */
#if EV_BUF_3_GROUP

/*Followed should be defined in app_config.h*/
#if 0
#define DEFAULT_BUFFER_GROUP_NUM     3
#define BUFFER_GROUP_0                          32
#define BUFFER_GROUP_1                          72
#define BUFFER_GROUP_2                          120
#define MAX_BUFFER_SIZE                         (BUFFER_GROUP_2-8)

#define BUFFER_NUM_IN_GROUP0             12
#define BUFFER_NUM_IN_GROUP1             20
#define BUFFER_NUM_IN_GROUP2             15
#endif

#elif EV_BUF_4_GROUP
/*Followed should be defined in app_config.h*/
#if 0
#define DEFAULT_BUFFER_GROUP_NUM     4
#define BUFFER_GROUP_0                          32
#define BUFFER_GROUP_1                          72
#define BUFFER_GROUP_2                          120
#define BUFFER_GROUP_3                          1040//To be set by user, DMA_RX_BUFFER_SIZE+4+8 = 1028+4(UartLen)+8
#define MAX_BUFFER_SIZE                         (BUFFER_GROUP_3-8)

#define BUFFER_NUM_IN_GROUP0             12
#define BUFFER_NUM_IN_GROUP1             20
#define BUFFER_NUM_IN_GROUP2             8//15
#define BUFFER_NUM_IN_GROUP3             1 //To be set by user
#endif

#elif EV_BUF_5_GROUP
/*Followed should be defined in app_config.h*/
#if 0
#define DEFAULT_BUFFER_GROUP_NUM     5
#define BUFFER_GROUP_0                          32
#define BUFFER_GROUP_1                          72
#define BUFFER_GROUP_2                          120
#define BUFFER_GROUP_3                          0 //To be set by user
#define BUFFER_GROUP_4                          0 //To be set by user
#define MAX_BUFFER_SIZE                         (BUFFER_GROUP_4-8)

#define BUFFER_NUM_IN_GROUP0             12
#define BUFFER_NUM_IN_GROUP1             20
#define BUFFER_NUM_IN_GROUP2             15
#define BUFFER_NUM_IN_GROUP3             0 //To be set by user
#define BUFFER_NUM_IN_GROUP4             0 //To be set by user
#endif

#elif EV_BUF_6_GROUP
/*Followed should be defined in app_config.h*/
#if 0
#define DEFAULT_BUFFER_GROUP_NUM     6
#define BUFFER_GROUP_0                          32
#define BUFFER_GROUP_1                          72
#define BUFFER_GROUP_2                          120
#define BUFFER_GROUP_3                          0 //To be set by user
#define BUFFER_GROUP_4                          0 //To be set by user
#define BUFFER_GROUP_5                          0 //To be set by user
#define MAX_BUFFER_SIZE                         (BUFFER_GROUP_5-8)

#define BUFFER_NUM_IN_GROUP0             12
#define BUFFER_NUM_IN_GROUP1             20
#define BUFFER_NUM_IN_GROUP2             15
#define BUFFER_NUM_IN_GROUP3             0 //To be set by user
#define BUFFER_NUM_IN_GROUP4             0 //To be set by user
#define BUFFER_NUM_IN_GROUP5             0 //To be set by user
#endif

#else //EV_BUF_3_GROUP by default
#define EV_BUF_3_GROUP                          1
#define DEFAULT_BUFFER_GROUP_NUM     			3
#define BUFFER_GROUP_0                          40
#define BUFFER_GROUP_1                          80
#define BUFFER_GROUP_2                          200
#define MAX_BUFFER_SIZE                         (BUFFER_GROUP_2-BUFFER_HDR_SIZE)

#define BUFFER_NUM_IN_GROUP0             20
#define BUFFER_NUM_IN_GROUP1             10
#define BUFFER_NUM_IN_GROUP2             5
#endif


/** @addtogroup ev_buffer_typical_size EV Buffer Typical Application Size
 * Definition default buffer size for different typical usage
 * @{
 */
#define SMALL_BUFFER                     32 //For 24-byte length data, need 24+8=32 byte size buffer
#define MEDIUM_BUFFER                    72
#define LARGE_BUFFER                     192 //For 80-byte length data, need 192+8=200 byte size buffer

/** @} end of group ev_buffer_typical_size */

/** @} end of group EV_BUFFER_CONSTANT */


/** @defgroup EV_BUFFER_TYPE EV Buffer Types
 *  @{
 */

/**
 *  @brief Definition of a buffer item, it is internal used.
 */
typedef struct ev_bufItem {
	struct ev_bufItem *next;
	u32 groupIndex;
	u8 data[1];
} ev_bufItem_t;


/**
 *  @brief Definiton error code of EV buffer operation
 */
typedef enum buf_sts_e {
	// SUCCESS always be ZERO
	BUFFER_SUCC,
	BUFFER_INVALID_PARAMETER = 1,  //!< Invalid parameter passed to the buffer API
	BUFFER_DUPLICATE_FREE,          //!< The same buffer is freed more than once
	BUFFER_ADDR_INVALID,               //!< Addr is not available in ev_buf address
} buf_sts_t;

/**  @} end of group EV_BUFFER_TYPE */


/** @defgroup EV_BUFFER_FUNCTIONS EV Buffer API
 *  @brief Function declaration of EV Buffer module
 *  @{
 */

/**
 * @brief       Reset the EV Buffer module
 *
 * @param       None
 *
 * @return      None
 */
void ev_buf_reset(void);

/**
 * @brief       Initialize the EV Buffer module
 *
 * @param       None
 *
 * @return      None
 */
void ev_buf_init(void);

/**
 * @brief       Allocate an available buffer according to the requested size
 *              The allocated buffer will have only three kind of size, defined
 *              in @ref EV_BUFFER_CONSTANT
 *
 * @param       size - The requested size
 *
 * @return      Pointer to an allocated buffer.
 *              NULL means the there is no available buffer.
 */
#if DEBUG_EV_BUFFER_FREE
u8 *my_ev_buf_allocate(u16 size, int line, char *file);
#else
u8 *ev_buf_allocate(u16 size);
#endif

/**
 * @brief       Free the specified buffer
 *
 * @param       pBuf - the pointer to the specified buffer to free.
 *
 * @return      Status.
 */

#if DEBUG_EV_BUFFER_FREE
buf_sts_t my_ev_buf_free(u8 *pBuf, int line, char *file);
#else
buf_sts_t ev_buf_free(u8 *pBuf);
#endif

#if DEBUG_EV_BUFFER_FREE
#define ev_buf_allocate(x) my_ev_buf_allocate(x,__LINE__,__FILE__)
#define ev_buf_free(x) my_ev_buf_free(x,__LINE__,__FILE__)
#endif
/**
 * @brief       Get the header of a buffer item
 *
 * @param       pd - the pointer of a data, which is previously allocated
 *
 * @return      Pointer of bufferItem
 */
ev_bufItem_t *ev_buf_getHead(u8 *pd);

/**
 * @brief       Get available buffer num for dedicated group
 *
 * @param       index - ev buffer goupe index
 *
 * @return      Availble buffer num in this group
 */
u8 ev_buf_getAvailNum(u8 index);

/*********************************************************************
 * @fn      ev_buf_isAddrValid
 *
 * @brief   Check if the address is a part of the ev buffer address
 *
 * @param   pBuf - the memory address to be checked
 *
 * @return    check result
 */
buf_sts_t ev_buf_isAddrValid(u8 *pBuf);



/*********************************************************************
* @fn      ev_buffer_remain
*
 * @brief   Interface to get remaining dynamic buffer number
 *
 * @param   buff_len:	the kind of buffer you want check
 *
 * @return  the number of buffers remaining
 */
u8 ev_buffer_remain(u8 buff_len);


/**  @} end of group EV_BUFFER_FUNCTIONS */

/**  @} end of group EV_BUFFER */

/**  @} end of group TELINK_COMMON_MODULE */

#endif

