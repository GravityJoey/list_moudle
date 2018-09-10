
#pragma once

#include "../../types.h"
#include "boards.h"

#define FLASH_4K_PAGE_NUM              16
#define FLASH_PAGE_SIZE                256

#define NV_MAX_SECTION_BLOCK           4

#define NV_SECTION_SIZE                (FLASH_PAGE_SIZE * FLASH_4K_PAGE_NUM)

#define NV_ALIGN_LENTH(len)                ( ( ((len) % 4 ) == 0) ? len : ( ((len)/4 + 1) * 4 ) )

#define END_PAGE_FOR_NV         (START_PAGE_FOR_NV + (NV_SECTION_SIZE * NV_MAX_SECTION_BLOCK))


#define NV_HEADER_TABLE_SIZE           10

#define INVALID_NV_VALUE               0xFF

#define MOUDLES_START_ADDR(modules)   (START_PAGE_FOR_NV + NV_SECTION_SIZE * (modules))

#define MOUDLES_AVALIABLE_SIZE(usedSize)  (FLASH_4K_PAGE_NUM * FLASH_PAGE_SIZE - (usedSize))

#define PAGE_AVALIABLE_SIZE(offset)       (FLASH_PAGE_SIZE - (offset % FLASH_PAGE_SIZE))

#define PAGE_HEADER_SISE  (sizeof(page_header_t))
#define NV_HEADER_SIZE  (sizeof(nv_header_t))

#define READ_BYTES_PER_TIME                 16

typedef enum {
	NV_BLE_ITEM_LL,
	NV_BLE_ITEM_GAP,
	NV_BLE_ITEM_BOND_TBL,
	NV_BLE_ITEM_SM,
	NV_BLE_ITEM_NUM,
} nv_bleItemId_t;

/*
typedef enum {
	NV_BASIC_WHITE_LIST,
	NV_BASIC_ITEM_NUM,
} nv_basicItemId_t;

typedef enum {
	NV_USER_ITEM_H_ID,
	NV_USER_ITEM_AUZ_KEY,
	NV_USER_ITEM_D_ID,
} nv_userItemId_t;

typedef enum {
	NV_USER_ITEM_RESET_CNT,
} nv_user2ItemId_t;

typedef enum {
	NV_USER_ITEM_PASS_KEY
} nv_user3ItemId_t;
*/


typedef struct {
	u16 offset;
	u16 len;
	u8 id;
} nv_header_t;


typedef struct {
	u16 usedSize;
	nv_header_t hdrList[NV_HEADER_TABLE_SIZE];
} page_header_t;

typedef struct {
	/* addr of header */
	u16 usedSize;
	u16 nvOffset;
	nv_header_t hdrInfo;
} item_info_t;


/** @addtogroup  TELINK_BLE_HAL TELINK HAL Stack
 *  @{
 */

/** @addtogroup  HAL_NV_Module NV Flash Module
 *  @{
 */

/** @addtogroup  HAL_NV_Module_Constant NV Flash Constants
 *  @{
 */

/**
*  @brief  Definition of starting position of NV flash module (100K by default)
*/
//#define START_PAGE_FOR_NV       (400 * FLASH_PAGE_SIZE)

#define START_PAGE_FOR_NV 		  (0x70000)

//#define START_PAGE_FOR_NV       (NRF_FICR->CODEPAGESIZE)*(NRF_FICR->CODESIZE - 100)

/**
*  @brief  Definition of constant position for parameters needed by Telink chip (need be pre burned during production )
*/
/*The block start from 0x76000~0x76fff is used to store the parameters which don't need to be calibrated */

/*0x76000~0x7603f: MAC ddress in the following format: 0x ZZ-YY-A4-C1-38-XX-WW-VV, 
  *where 0xA4-C1-38-XX-WW-VV is used for BLE mac address (0xZZYY is used only by zigbee address)
  * VV is the LSB and ZZ is the MSB.*/
#define CFG_MAC_ADDRESS              (0x76000)

/*0x76040~0x7617f is reserved for Telink future use*/
#define CFG_TELINK_NO_CALI_RESERVED1   (0x76040)
#define CFG_TELINK_NO_CALI_RESERVED2   (0x76080)
#define CFG_TELINK_NO_CALI_RESERVED3   (0x760c0)
#define CFG_TELINK_NO_CALI_RESERVED4   (0x76100)
#define CFG_TELINK_NO_CALI_RESERVED5   (0x76140)

/*0x76180~0x76fff is for vendor specific use, to store parameters which don't need to 
 * be calibrated, can be defined by user*/

/*The block start from 0x77000~0x77fff is used to store the parameters which need to be calibrated */

/*0x77000~0x7703f, 1 byte, used for frequency offset calibration*/
#define CFG_FREQUENCY_OFFSET         (0x77000)

/*0x77040~0x7707F, 2 byte, 0x77040 for TP_GAIN_0, 0x77041 for TP_GAIN_1*/
#define CFG_TP_GAIN                  (0x77040)

/*Not supported for current SDK*/
#define CFG_ADC_CALIBRATION           (0x77080) 

/*Not supported for current SDK*/
#define CFG_BATERRY_DETECT            (0x770C0)

/*0x77100~0x77239 is reserved for Telink future use*/
#define CFG_TELINK_CALI_RESERVED1   (0x77100)
#define CFG_TELINK_CALI_RESERVED2   (0x77140)
#define CFG_TELINK_CALI_RESERVED3   (0x77180)
#define CFG_TELINK_CALI_RESERVED4   (0x771C0)
#define CFG_TELINK_CALI_RESERVED5   (0x77200)


/** @} end of group HAL_NV_Module_Constant */


/** @addtogroup  HAL_NV_Module_Types NV Flash Types
 *  @{
 */

/**
 *  @brief  Specific purpose statement for different NV flash modules,
 *    each module occupies 4K memory.
 *
 *  @details By default, the BLE stack has allocated  three modules:
 *    - the first module is used for basic use.
 *    - the second module is used for ble stack use.
 *    - the third module is used for user application.
 *
 *  @details So if user need a NV memory size less than 4K, user can used the third
 *    module directly. Otherwise, user can declare some additional modules,
 *    ex, NV_USER_MODULE2=3, NV_USER_MODULE3=4,....
 *    All the modules specified should be declared before NV_MAX_MOULDS
 */

/*
typedef enum {
	NV_USER_MODULE = 0,     //!< Used for user application
	NV_USER_MODULE1,
	NV_USER_MODULE2,
	NV_USER_MODULE3,
	NV_MAX_MOULDS,            //!< Represent the number of modules used
} nv_module_t;
*/

/**
 *  @brief  Definition of nv_read/nv_write function return value type
 */
typedef enum nv_sts_e {
	NV_INVALID_MODULS = 1,      //!< Module input has not been declared yet
	NV_INVALID_ID ,                     //!< ID input is not valid
	NV_ITEM_NOT_FOUND,           //!< ID not found for nv_read operation in this module
	NV_NOT_ENOUGH_SAPCE,       //!< No enough space for nv_write operation in this module
	NV_ITEM_LEN_NOT_MATCH,    //!< Length input doesn't meet the actual length stored
	NV_CHECK_SUM_ERROR,           //!< Check sum wrong for nv_read operation
} nv_sts_t;
/** @} end of group HAL_NV_Module_Types */



/** @addtogroup  HAL_NV_Module_Functions NV Flash APIs
 *  @{
 */
/**
  * @brief      Read NV Data
  *
  * @param[in]  modules - NV module used (4k memory each module)
  * @param[in]  id        - Specify different ID for all items needed to be stored in the same module (max 9)
  * @param[in]  len      - Length of data to be read (max 4K)
  * @param[in]  buf      - Buffer to store data read from NV
  *
  * @return     Status
  */
nv_sts_t nv_read(u8 modules, u8 id, u16 len, u8 *buf);

/**
  * @brief      Write NV Data
  *
  * @param[in]  modules - NV module used (4k memory each module)
  * @param[in]  id        - Specify different ID for all items needed to be stored in the same module (max 9)
  * @param[in]  len      - Length of data to be write (max 4K)
  * @param[in]  buf      - Buffer to store data to be written into NV
  *
  * @return     Status
  */
nv_sts_t nv_write(u8 modules, u8 id, u16 len, u8 *buf);

/**
  * @brief      Read NV Data for NV_USER_MODULE.
  *
  * @details   If user need a NV memory size less than 4K, user can use this API instead of nv_read
  *
  * @param[in]  id        - Specify different ID for all items needed to be stored in the same module (max 9)
  * @param[in]  len      - Length of data to be read (max 4K)
  * @param[in]  buf      - Buffer to store data read from NV
  *
  * @return     Status
  */
#define NOT_TUYA_PORTING_API 0
#if NOT_TUYA_PORTING_API
#define ty_flash_read(NV_USER_MODULE,id, buf, len)              nv_read(NV_USER_MODULE, id, len, buf)
#define NV_USER1_ITEM_LOAD(id, buf, len)              nv_read(NV_USER_MODULE1, id, len, buf)
#define NV_USER2_ITEM_LOAD(id, buf, len)              nv_read(NV_USER_MODULE2, id, len, buf)
#define NV_USER3_ITEM_LOAD(id, buf, len)              nv_read(NV_USER_MODULE3, id, len, buf)
#endif

/**
  * @brief      Write NV Data for NV_USER_MODULE.
  *
  *@details    If user need a NV memory size less than 4K, user can use this API instead of nv_write
  *
  * @param[in]  modules - NV module used (4k memory each module)
  * @param[in]  id        - Specify different ID for all items needed to be stored in the same module (max 9)
  * @param[in]  len      - Length of data to be write (max 4K)
  * @param[in]  buf      - Buffer to store data to be written into NV
  *
  * @return     Status
  */

#if NOT_TUYA_PORTING_API
#define NV_USER_ITEM_SAVE(id, buf, len)              nv_write(NV_USER_MODULE, id, len, buf)
#define NV_USER1_ITEM_SAVE(id, buf, len)              nv_write(NV_USER_MODULE1, id, len, buf)
#define NV_USER2_ITEM_SAVE(id, buf, len)              nv_write(NV_USER_MODULE2, id, len, buf)
#define NV_USER3_ITEM_SAVE(id, buf, len)              do{	\
															if(SUCCESS != nv_write(NV_USER_MODULE3, id, len, buf)){	\
																nv_resetModule(NV_USER_MODULE3);	\
																nv_write(NV_USER_MODULE3, id, len, buf);	\
															}	\
													  }while(0)
#endif


#define NV_ERASE_RESET_CNT()  				flash_erase_sector(MOUDLES_START_ADDR(NV_USER_MODULE1))
/** @} end of group HAL_NV_Module_Functions */


/** @} end of group HAL_NV_Module */

/** @} end of group TELINK_BLE_HAL */


/*Functions for internal use */

nv_sts_t nv_init(u8 rst);
nv_sts_t nv_resetModule(u8 modules);
nv_sts_t nv_resetAll(void);
void nv_flashWrite(u32 startAddr, u16 len, u8 *buf);

