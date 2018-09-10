/***********************************************************
*  File: simple_flash.c 
*  Author: nzy
*  Date: 20160804
***********************************************************/
#define __SIMPLE_FLASH_GLOBALS
#include "simple_flash.h"
//#include "sys_adapter.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#undef NULL
#define NULL (void *)0
static void *(*sf_malloc)(size_t sz) = malloc;
static void (*sf_free)(void *ptr) = free;

#define PAGE_SIZE_LOW_LMT 64

#define GLOBAL_PAGE_ID(sfhand,blk_id,bpg_id) (blk_id*(sfhand->block_pages)+bpg_id)
#define PHY_USER_FLASH_ADDR(sfhand,blk_id,bpg_id) (sfhand->cfg.start_addr + GLOBAL_PAGE_ID(sfhand,blk_id,bpg_id)*(sfhand->page_sz))
#define PHY_USER_SWAP_FLASH_ADDR(sfhand,swap_blk_id) (sfhand->cfg.swap_start_addr + (swap_blk_id*sfhand->cfg.block_sz))
#define BPG_ID_FROM_GPID(sfhand,gpid) ((gpid) % (sfhand->block_pages))
#define BLOCK_ID_FROM_ADDR(sfhand,addr) ((addr - sfhand->cfg.start_addr) / sfhand->cfg.block_sz)
#define BLOCK_ID_FROM_GPID(sfhand,gpid) ((gpid) / (sfhand->block_pages))

#define PR_DEBUG

/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/
static sf_ret __sf_block_write_direct(const sf_hand_s *hand,const sf_uint32_t addr,\
                                      const sf_uint8_t *data,const sf_uint32_t len);
void delete_flash_hand(const sf_hand_s *hand);
static sf_ret __sf_block_read(const sf_hand_s *hand,const sf_uint32_t verify,\
                              const sf_uint16_t block_id,sf_uint8_t *data,const sf_uint32_t len);
static sf_ret __sf_swap_block_read(const sf_hand_s *hand,const sf_uint32_t verify,\
                                   const sf_uint16_t swap_block_id,sf_uint8_t *data,const sf_uint32_t len);
sf_ret __swap_block_data_restore(sf_hand_s *hand,sf_uint8_t *blk_buf,const sf_uint32_t blk_buf_len);
static sf_ret __sf_free_page(sf_hand_s *hand,const sf_uint16_t block_id,const sf_uint8_t bpg_id);

/***********************************************************
*  Function: sf_init_hooks
*  description: register memory method
*  Input: hooks->memory method
*  Output: none
*  Return: none
***********************************************************/
void sf_init_hooks(sf_hook_s *hooks)
{
    if (!hooks) {
        sf_malloc = malloc;
        sf_free = free;
        return;
    }

    sf_malloc = (hooks->malloc_fn)?hooks->malloc_fn:malloc;
    sf_free   = (hooks->free_fn)?hooks->free_fn:free;
}

static sf_uint32_t sum32(void *data, sf_uint16_t len)
{
	sf_uint32_t sum = 0;
	sf_uint8_t *tmp = (sf_uint8_t *)data;

	int i;
	for (i = 0; i < len; i++) {
		sum += tmp[i];
	}

	return sum;
}

static sf_ret __make_sf_index(sf_hand_s *hand,sf_uint16_t block_id,\
                              sf_uint8_t bpg_id,sf_index_s **index)
{
    sf_index_s *tmp_index = (struct s_sf_index *)sf_malloc(sizeof(struct s_sf_index));
    if(NULL == tmp_index) {
        return SF_MALLOC_ERR;
    }

    sf_ret ret;
    ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,block_id,bpg_id),\
                            (sf_char_t *)tmp_index,sizeof(struct s_sf_index));
    if(SF_OK != ret) {
        sf_free(tmp_index);
        return ret;
    }

    int len = sizeof(struct s_sf_index)+tmp_index->name_len+tmp_index->element_num*sizeof(sf_data_element_s);
    if(len > hand->page_sz) {
        sf_free(tmp_index);
        return SF_PG_SIZE_NOT_ENOUGH;
    }

    *index = (struct s_sf_index *)sf_malloc(len);
    sf_free(tmp_index);
    if(NULL == *index) {
        return SF_MALLOC_ERR;
    }

    ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,block_id,bpg_id),(sf_uint8_t *)(*index),len);
    if(SF_OK != ret) {
        sf_free(*index);
        return ret;
    }
	(*index)->element = (sf_data_element_s *)((sf_uint8_t *)(*index) + sizeof(struct s_sf_index) + (*index)->name_len);
    
    return SF_OK;
}

static void __sf_index_cache_delete(sf_hand_s *hand,const sf_uint8_t *name)
{
    int i;

    for(i = 0;i < hand->cache_cnt;i++) {
        if((strlen(name) == hand->idx_cache[i]->name_len) && \
           (0 == strcmp(name,hand->idx_cache[i]->name))) {
            break;
        }
    }

    if(i >= hand->cache_cnt) { // not find
        return;
    }

    hand->cache_cnt--;
    if(hand->up_index > i || hand->up_index >=  hand->cache_cnt) { // update index
        hand->up_index--;
    }

    // delete the index cache
    sf_free(hand->idx_cache[i]);
    for(;i < (hand->cache_cnt-1);i++) {
        hand->idx_cache[i] = hand->idx_cache[i+i];
    }
}

static void __sf_index_cache_delete_by_idx(sf_hand_s *hand,const sf_index_s *index)
{
    int i;

    for(i = 0;i < hand->cache_cnt;i++) {
        if(index == hand->idx_cache[i]) {
            break;
        }
    }

    if(i >= hand->cache_cnt) { // not find
        return;
    }

    if(hand->up_index > i || hand->up_index >=  hand->cache_cnt) { // update index
        hand->up_index--;
    }

    // delete the index cache
    sf_free(hand->idx_cache[i]);
    for(;i < (hand->cache_cnt-1);i++) {
        hand->idx_cache[i] = hand->idx_cache[i+1];
    }
	hand->cache_cnt--;
}

// drop_or_up == 0 when new index insert if cache buffer is full then drop it.
// drop_or_up == other when new index insert if cache buffer is full then instead of the oldest one.
static void __sf_index_cache_insert(sf_hand_s *hand,const sf_index_s *index,\
                                    const sf_uint8_t drop_or_up)
{
    if(hand->cache_cnt < INDEX_CACHE_LMT) {
		hand->idx_cache[hand->cache_cnt++] = (sf_index_s *)index;
    }else {
        if(0 == drop_or_up) { // drop it 
            return;
        }

        // sf_free(hand->idx_cache[hand->up_index]->element);
        sf_free(hand->idx_cache[hand->up_index]);
		hand->idx_cache[hand->up_index++] = (sf_index_s *)index;
        if(hand->up_index >= INDEX_CACHE_LMT) {
            hand->up_index = 0;
        }
    }
}
    
static sf_ret __make_index_cache(sf_hand_s *hand,sf_block_mag_s *blk_mag)
{
    sf_ret ret;
    int i,j;
    
    for(i = 0;i < blk_mag->pg_bm_len;i++) {
        if(blk_mag->pg_bm[i]) {
            for(j = 0;j < 8;j++) {
                if(0 == (blk_mag->pg_bm[i] & (1 << j))) {
                    continue;
                }
            
                sf_index_s *index;
                ret = __make_sf_index(hand,blk_mag->block_id,(i*8+j),&index);
                #if 0
                if(SF_OK != ret && SF_PG_SIZE_NOT_ENOUGH) {
                    goto err_ret;
                }else if(SF_PG_SIZE_NOT_ENOUGH == ret) {// unvalid index
                    __sf_free_page(hand,blk_mag->block_id,(i*8+j));
                    continue;
                }
                #else
                if(SF_OK != ret) {
                    printf("%s:%d ret:%d",__FILE__,__LINE__,ret);
                    goto err_ret;
                }
                #endif
                
                __sf_index_cache_insert(hand,index,0);
                hand->sf_index_cnt++;
                
                // count free page                    
                hand->free_pages--;
                sf_uint32_t gpid = GLOBAL_PAGE_ID(hand,blk_mag->block_id,(i*8+j));
                hand->pg_bm[gpid/8] |= 1 << (gpid % 8);
                
                sf_uint16_t l,k;
                for(l = 0;l < index->element_num;l++) {
                    for(k = index->element[l].start_page_id;k <= index->element[l].end_page_id;k++) {
                        hand->free_pages--;
                        gpid = GLOBAL_PAGE_ID(hand,index->element[l].block_id,k);
                        hand->pg_bm[gpid/8] |= 1 << (gpid % 8);
                    }
                }
            }
        }
    }

    return SF_OK;

err_ret:
    for(i = 0;i < hand->cache_cnt;i++) {
        sf_free(hand->idx_cache[i]);
        hand->idx_cache[i] = NULL;
    }
    
    return ret;
}

static sf_ret __sf_find_index(sf_hand_s *hand,const sf_uint8_t *name,sf_index_s **index)
{
    if(NULL == hand || NULL == name) {
        return SF_INVALID_PARM;
    }

    int i;
    for(i = 0;i < hand->cache_cnt;i++) {
        if((strlen(name)+1 == hand->idx_cache[i]->name_len) && \
			(0 == strcmp(name, hand->idx_cache[i]->name))) {
			if(index) {
                *index = hand->idx_cache[i];
            }
            return SF_OK;
        }
    }

    if(hand->sf_index_cnt <= hand->cache_cnt) {
        return SF_NO_FIND;
    }

    sf_uint8_t *read_buf = sf_malloc(hand->page_sz);
    if(NULL == read_buf) {
        return SF_MALLOC_ERR;
    }

    sf_ret ret;
    for(i = 0;i < hand->block_nums;i++) { // find the index from flash
        ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,i,0),read_buf,hand->page_sz);
        if(SF_OK != ret) {
            sf_free(read_buf);
            return ret;
        }

        sf_block_mag_s *block_mag = (sf_block_mag_s *)read_buf;
        int j,k;
        for(j= 0;j < block_mag->pg_bm_len;j++) {
            if(block_mag->pg_bm[j]) {
                for(k = 0;k < 8;k++) {
                    if(block_mag->pg_bm[j] & (1 << k)) {
                        sf_uint8_t bpg_id = j*8 + k;
                        int n;
                        for(n = 0;n < hand->cache_cnt;n++) {
                            if(bpg_id == hand->idx_cache[n]->bpg_id) {
                                break;
                            }
                        }

                        sf_index_s *tmp_index;
                        if(n >= hand->cache_cnt) { // not find in cache
                            ret = __make_sf_index(hand,block_mag->block_id,bpg_id,&tmp_index);
                            if(SF_OK != ret && SF_PG_SIZE_NOT_ENOUGH != ret) {
                                sf_free(read_buf);
                                return ret;
                            }else if(SF_PG_SIZE_NOT_ENOUGH == ret) {// unvalid index
                                #if 1
                                printf("%s:%d i:%d block_mag->pg_bm[%d]=%x\n",\
                                       __FILE__,__LINE__,i,j,block_mag->pg_bm[j]);
                                #endif
                                __sf_free_page(hand,block_mag->block_id,bpg_id);
                                continue;
                            }

                            if((strlen(name)+1 == tmp_index->name_len) && \
			                   (0 == strcmp(name, tmp_index->name))) {
                                __sf_index_cache_insert(hand,tmp_index,1); // update the cache
                                if(index) {
                                    *index = tmp_index;
                                }
                                sf_free(read_buf);
                                return SF_OK;
                            }

                            sf_free(tmp_index);
                        }
                    }
                }
            }
        }
    }

    sf_free(read_buf);
    return SF_NO_FIND;
}

static sf_ret __sf_fuzzy_find_index(sf_hand_s *hand,const sf_uint8_t *fuzzy_name,\
                                    sf_uint32_t *gpid,sf_index_s **index)
{
    if(NULL == hand || NULL == fuzzy_name) {
        return SF_INVALID_PARM;
    }

    sf_uint8_t *read_buf = sf_malloc(hand->page_sz);
    if(NULL == read_buf) {
        return SF_MALLOC_ERR;
    }

    sf_uint16_t tmp_block_id = BLOCK_ID_FROM_GPID(hand,*gpid);
    sf_uint8_t tmp_bpg_id = BPG_ID_FROM_GPID(hand,*gpid);
    
    sf_ret ret;
    sf_uint16_t tmp_seq = 0;
    int i;
    for(i = tmp_block_id;i < hand->block_nums;i++) { // find the index from flash
        ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,i,0),read_buf,hand->page_sz);
        if(SF_OK != ret) {
            sf_free(read_buf);
            return ret;
        }

        sf_block_mag_s *block_mag = (sf_block_mag_s *)read_buf;
        int j,k;
        for(j= (tmp_bpg_id/8);j < block_mag->pg_bm_len;j++) {
            if(0 == block_mag->pg_bm[j]) {
                continue;
            }
            
            for(k = (tmp_bpg_id%8);k < 8;k++) {
                if(0 == (block_mag->pg_bm[j] & (1 << k))) {
                    continue;
                }

                sf_index_s *tmp_index = NULL;
                sf_uint8_t bpg_id = j*8 + k;
                ret = __make_sf_index(hand,block_mag->block_id,bpg_id,&tmp_index);
                if(SF_OK != ret && SF_PG_SIZE_NOT_ENOUGH != ret) {
                    sf_free(read_buf);
                    return ret;
                }else if(SF_PG_SIZE_NOT_ENOUGH == ret) {// unvalid index
                    #if 1
                    printf("%s:%d i:%d block_mag->pg_bm[%d]=%x\n",\
                           __FILE__,__LINE__,i,j,block_mag->pg_bm[j]);
                    #endif
                    __sf_free_page(hand,block_mag->block_id,bpg_id);
                    continue;
                }

                if((0 == strncmp(fuzzy_name, tmp_index->name,strlen(fuzzy_name)))) {
                    sf_free(read_buf);
                    *index = tmp_index;
                    *gpid = GLOBAL_PAGE_ID(hand,i,bpg_id)+1;
                    return SF_OK;
                }
                sf_free(tmp_index);
            }
			tmp_bpg_id += (tmp_bpg_id - tmp_bpg_id % 8);
        }
		tmp_bpg_id = 0;
    }

    sf_free(read_buf);
    return SF_NO_FIND;
}


/***********************************************************
*  Function: create_flash_hand_and_init
*  description: 
*  Input: cfg-> simple flash config
*  Output: ohand-> simple flash hand
*  Return: sf_ret
*  Note: if(cfg->sf_crc32 == NULL) then "use sum32 to verify 
*        data validity internally" 
***********************************************************/
sf_ret create_flash_hand_and_init(sf_hand_s **ohand,const sf_flash_cfg_s *cfg)
{
    if(NULL == ohand || \
       NULL == cfg) {
        return SF_INVALID_PARM;
    }

    if(NULL == cfg->sf_erase || \
       NULL == cfg->sf_read || \
       NULL == cfg->sf_write || \
       cfg->flash_sz < cfg->block_sz || \
       cfg->swap_flash_sz < cfg->block_sz) {
        return SF_COM_ERR;
    }

    sf_hand_s *hand = sf_malloc(sizeof(sf_hand_s));
    if(NULL == hand) {
        return SF_MALLOC_ERR;
    }
    memset(hand,0,sizeof(sf_hand_s));

    memcpy(&hand->cfg,cfg,sizeof(sf_flash_cfg_s));
    if(NULL == hand->cfg.sf_crc32) {
        hand->cfg.sf_crc32 = sum32;
    }
    
    hand->block_nums = hand->cfg.flash_sz / hand->cfg.block_sz;
    hand->swap_block_nums = hand->cfg.swap_flash_sz / hand->cfg.block_sz;
    
    // Select the appropriate page size
    sf_uint16_t pg_bm_len = 0;
    sf_uint16_t i = 1;
    do {
        hand->page_sz = PAGE_SIZE_LOW_LMT * i;
        hand->block_pages = hand->cfg.block_sz / hand->page_sz;

        pg_bm_len = hand->page_sz - sizeof(sf_block_mag_s);
        i *= 2;
    }while((pg_bm_len*8) < hand->block_pages);

    hand->flash_pages = hand->block_nums * hand->block_pages; 
    hand->free_pages = hand->flash_pages - hand->block_nums; // each block have a manage page in the first page

    hand->pg_bm_len = hand->flash_pages / 8;
    hand->pg_bm_len += (hand->flash_pages % 8) ? 1 : 0;
    hand->pg_bm = sf_malloc(hand->pg_bm_len);
    if(NULL == hand->pg_bm) {
        sf_free(hand);
        return SF_MALLOC_ERR;
    }
    memset(hand->pg_bm,0,hand->pg_bm_len);

    // init page bitmp
    for(i = 0;i < hand->block_nums;i++) {
        sf_uint32_t gpid = GLOBAL_PAGE_ID(hand, i, 0);
		hand->pg_bm[gpid / 8] |= 1 << (gpid % 8);
    }

    // verify flash block and init
    sf_uint8_t *read_buf = sf_malloc(hand->cfg.block_sz);
    if(NULL == read_buf) {
        delete_flash_hand(hand);
        return SF_MALLOC_ERR;
    }

	sf_ret ret;
    // swap flash area restore
    ret = __swap_block_data_restore(hand,read_buf,hand->cfg.block_sz);
    if(SF_OK != ret) {
        goto err_ret;
    }

    // data restore
    for(i = 0;i < hand->block_nums;i++) {
        ret = __sf_block_read(hand,1,i,read_buf,hand->cfg.block_sz);
        if(SF_OK != ret && SF_BLOCK_UNVALID != ret) {
            goto err_ret;
        }
        
        sf_block_mag_s *blk_mag = (sf_block_mag_s *)read_buf;
        // verify 
        if(SF_BLOCK_UNVALID == ret) {
            memset(read_buf,0xff,hand->cfg.block_sz);
            blk_mag->magic = SIMPLE_BF_MAGIC;
            blk_mag->block_id = i;
            blk_mag->wr_count = 0;
            blk_mag->pg_bm_len = hand->block_pages / 8;
            blk_mag->pg_bm_len += (hand->block_pages % 8) ? 1 : 0;
            memset(blk_mag->pg_bm,0,blk_mag->pg_bm_len);
            blk_mag->crc32 = hand->cfg.sf_crc32(read_buf+8,hand->cfg.block_sz-8);

            ret = __sf_block_write_direct(hand,PHY_USER_FLASH_ADDR(hand,i,0),read_buf,hand->cfg.block_sz);
            if(SF_OK != ret) {
                goto err_ret;
            }
            continue;
        }

        // make index cache
        ret = __make_index_cache(hand,blk_mag);
        if(SF_OK != ret) {
            goto err_ret;
        }
    }

    sf_free(read_buf);
    *ohand = hand;

    return SF_OK;

err_ret:
    sf_free(read_buf);
    delete_flash_hand(hand);
    return ret;
}

/***********************************************************
*  Function: delete_flash_hand
*  description: 
*  Input: hand->simple flash hand
*  Output: 
*  Return: sf_ret
***********************************************************/
void delete_flash_hand(const sf_hand_s *hand)
{
    if(NULL == hand) {
        return;
    }

    int i;
    for(i = 0;i < hand->cache_cnt;i++) {
        sf_free(hand->idx_cache[i]);
    }
    
    sf_free(hand->pg_bm);
    sf_free((void *)hand);
}

static sf_ret __sf_block_write_direct(const sf_hand_s *hand,const sf_uint32_t addr,\
                                      const sf_uint8_t *data,const sf_uint32_t len)
{
    if(NULL == hand || NULL == data) {
        return SF_INVALID_PARM;
    }

    if(((addr+len) > (addr + hand->cfg.block_sz - (addr % hand->cfg.block_sz))) || \
       ((addr+len) > (hand->cfg.start_addr + hand->cfg.flash_sz))) { // is span block
        return SF_INVALID_PARM;
    }

    sf_ret ret;
    ret = hand->cfg.sf_erase(addr,len);
    if(SF_OK != ret) {
        return ret;
    }

	ret = hand->cfg.sf_write((sf_uint32_t)addr, (sf_uint8_t *)data, len);

    return ret;
}

static sf_ret __sf_swap_block_write(sf_hand_s *hand,const sf_uint8_t *data,const sf_uint32_t len)
{
    if(NULL == hand || NULL == data || \
       (len > hand->cfg.block_sz)) {
        return SF_INVALID_PARM;
    }

    sf_ret ret;
    ret = hand->cfg.sf_erase(PHY_USER_SWAP_FLASH_ADDR(hand,hand->swap_blk_idx),len);
    if(SF_OK != ret) {
        hand->swap_blk_idx++;
        if(hand->swap_blk_idx >= hand->swap_block_nums) { // choose the next block 
            hand->swap_blk_idx = 0;
        }
        return ret;
    }

	ret = hand->cfg.sf_write(PHY_USER_SWAP_FLASH_ADDR(hand,hand->swap_blk_idx), (sf_uint8_t *)data, len);    
    hand->swap_blk_idx++;
    if(hand->swap_blk_idx >= hand->swap_block_nums) { // choose the next block
        hand->swap_blk_idx = 0;
    }

    return ret;
}

// index and all element must in the same block
static sf_ret __sf_block_write(sf_hand_s *hand,const sf_index_s *index,\
                               const sf_uint8_t *data,const sf_uint32_t len,\
                               const sf_data_element_s *element,const sf_uint16_t element_num,\
                               sf_uint8_t *blk_buf,const sf_uint32_t blk_buf_len)
{
    if((NULL == hand) || \
       ((NULL == index) && (NULL == data || 0== len || NULL == element || 0 == element_num)) || \
       (NULL == blk_buf || blk_buf_len < hand->cfg.block_sz)) {
        return SF_INVALID_PARM;
    }

    int i;    
    sf_uint32_t tmp_len = 0;
    sf_uint16_t block_id = 0;

    if(index || (element && len != 0)) {
        if(element && len != 0) {            
            block_id = element[0].block_id;
            
            for(i = 0;i < element_num;i++) {
                if(block_id != element[i].block_id) {
                    return SF_COM_ERR;
                }
                tmp_len += ((element[i].end_page_id - element[i].start_page_id) + 1) * hand->page_sz;
            }

            if(tmp_len < len) {
                return SF_COM_ERR;
            }
        }

        if(index) {
            if((element && len != 0) && index->block_id != block_id) {
                return SF_COM_ERR;
            }else {
                block_id = index->block_id;
            }
        }
    }
    
    sf_ret ret;
    ret = __sf_block_read(hand,1,block_id,blk_buf,hand->cfg.block_sz);
    if(SF_OK != ret) {
        return ret;
    }

    sf_block_mag_s *block_mag = (sf_block_mag_s *)blk_buf;

    // index copy and set bit in the manage block 
    if(index) {
        block_mag->pg_bm[index->bpg_id/8] |= 1 << (index->bpg_id%8);
        memcpy(blk_buf+index->bpg_id*hand->page_sz,index,\
               sizeof(sf_index_s)+index->name_len+(index->element_num*sizeof(sf_data_element_s)));
    }

    // user data copy
    if(element && len != 0) {
        sf_uint32_t offset = 0;
        int j = 0;
        
        for(i = 0;i < element_num;i++) {
            for(j = element[i].start_page_id; j <= element[i].end_page_id;j++) {
                if((offset + hand->page_sz) < len) {
                    memcpy(blk_buf+(j*hand->page_sz),data+offset,hand->page_sz);
                }else {
                    memcpy(blk_buf+(j*hand->page_sz),data+offset,len-offset);
                }

                offset += hand->page_sz;
                if(offset >= len) {
                    break;
                }
            }
        }
    }

    // update block magange info 
    block_mag->wr_count += 1;
    block_mag->crc32 = hand->cfg.sf_crc32(blk_buf+8,hand->cfg.block_sz-8);
    
    // save to flash  
    ret = __sf_block_write_direct(hand,PHY_USER_FLASH_ADDR(hand,block_id,0),blk_buf,hand->cfg.block_sz);
    if(SF_OK != ret) {
        return ret;
    }
    
    return ret;
}

static sf_ret __sf_block_read(const sf_hand_s *hand,const sf_uint32_t verify,\
                              const sf_uint16_t block_id,sf_uint8_t *data,const sf_uint32_t len)
{
    if(NULL == hand || \
       NULL == data || \
       len < hand->cfg.block_sz) {
        return SF_INVALID_PARM;
    }

    sf_ret ret;
    ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,block_id,0),data,hand->cfg.block_sz);
    if(SF_OK != ret) {
        return ret;
    }

    if(verify) {
        sf_block_mag_s *block_mag = (sf_block_mag_s *)data;
        if((block_mag->magic != SIMPLE_BF_MAGIC) || \
           (block_mag->block_id != block_id) || \
           (block_mag->crc32 != hand->cfg.sf_crc32(data+8,hand->cfg.block_sz-8))) {
            return SF_BLOCK_UNVALID;
        }
    }

    return SF_OK;
}

static sf_ret __sf_swap_block_read(const sf_hand_s *hand,const sf_uint32_t verify,\
                                   const sf_uint16_t swap_block_id,sf_uint8_t *data,const sf_uint32_t len)
{
    if(NULL == hand || \
       NULL == data || \
       len < hand->cfg.block_sz) {
        return SF_INVALID_PARM;
    }

    sf_ret ret;
    ret = hand->cfg.sf_read(PHY_USER_SWAP_FLASH_ADDR(hand,swap_block_id),data,hand->cfg.block_sz);
    if(SF_OK != ret) {
        return ret;
    }

    if(verify) {
        sf_block_mag_s *block_mag = (sf_block_mag_s *)data;
        if((block_mag->magic != SIMPLE_BF_MAGIC) || \
           (block_mag->crc32 != hand->cfg.sf_crc32(data+8,hand->cfg.block_sz-8))) {
            return SF_BLOCK_UNVALID;
        }
    }

    return SF_OK;
}

static sf_ret __sf_alloc_page(sf_hand_s *hand,sf_uint16_t *block_id,sf_uint8_t *bpg_id)
{
    if(NULL == hand || NULL == block_id || NULL == bpg_id) {
        return SF_INVALID_PARM;
    }

    if(0 == hand->free_pages) {
        return SF_NOT_ENOUGH_PAGE;
    }

    sf_uint32_t alloc_start_gpid = hand->alloc_gpid_index;

    do {
        if(!(hand->pg_bm[alloc_start_gpid/8] & (1 << (alloc_start_gpid%8)))) {
            hand->pg_bm[alloc_start_gpid/8] |= (1 << (alloc_start_gpid%8));
            
            *block_id = BLOCK_ID_FROM_GPID(hand,alloc_start_gpid);
            *bpg_id = BPG_ID_FROM_GPID(hand,alloc_start_gpid);
            
            hand->alloc_gpid_index = alloc_start_gpid+1;
            
            hand->free_pages--;
            if(hand->alloc_gpid_index >= hand->flash_pages) {
                hand->alloc_gpid_index = 0;
            }

            return SF_OK;
        }

        alloc_start_gpid++;
        if(alloc_start_gpid >= hand->flash_pages) {
            alloc_start_gpid = 0;
        }
    }while((alloc_start_gpid != hand->alloc_gpid_index));

    return SF_COM_ERR;
}

static sf_ret __sf_free_page(sf_hand_s *hand,const sf_uint16_t block_id,const sf_uint8_t bpg_id)
{
    if((NULL == hand) || \
       (block_id >= hand->block_nums) || \
       (bpg_id >= hand->block_pages)) {
        return SF_INVALID_PARM;
    }
    
    sf_uint32_t gpid = GLOBAL_PAGE_ID(hand,block_id,bpg_id);
    hand->free_pages++;

    hand->pg_bm[gpid/8] &= ~(1 << (gpid%8));

    return SF_OK;
}

static sf_ret __sf_alloc_data_elem_by_len(sf_hand_s *hand,const sf_uint32_t len,\
                                          sf_data_element_s **element,sf_uint16_t *element_num)
{
    if(NULL == hand || NULL == element || NULL == element_num) {
        return SF_INVALID_PARM;
    }

    sf_uint32_t need_pages = (len / hand->page_sz) + ((len % hand->page_sz) ? 1 : 0);
    if(need_pages > hand->free_pages) {
        return SF_NOT_ENOUGH_PAGE;
    }

    // count how many elements need?
    sf_uint32_t free_pages_cnt = 0;
    sf_uint16_t element_cnt = 0;
    sf_uint16_t last_free_bid = 0xffff;
    sf_uint8_t find_free_start = 0;
    sf_uint32_t alloc_start_gpid = hand->alloc_gpid_index;

    do {
        if(hand->pg_bm[alloc_start_gpid/8] == 0xff) {
            alloc_start_gpid += (8 - (alloc_start_gpid % 8));
            if(alloc_start_gpid >= hand->flash_pages) {
                alloc_start_gpid -= hand->flash_pages;
            }
            continue;
        }

        sf_uint32_t i = 0;
        for(i = (alloc_start_gpid % 8);i < 8;i++,alloc_start_gpid++) {            
            if(free_pages_cnt >= need_pages) {
                break;
            }
            
            if(!(hand->pg_bm[alloc_start_gpid/8] & (1 << (alloc_start_gpid%8)))) {
                free_pages_cnt++;

                // span block process
                if(0xffff == last_free_bid) {
                    last_free_bid = BLOCK_ID_FROM_GPID(hand,alloc_start_gpid);
                }else if(BLOCK_ID_FROM_GPID(hand,alloc_start_gpid) != last_free_bid) {
                    last_free_bid = BLOCK_ID_FROM_GPID(hand,alloc_start_gpid);
                    element_cnt++;
                    find_free_start = 1;

                    continue;
                }

                //  search sequence pages
                if(0 == find_free_start) { 
                    find_free_start = 1;
                    element_cnt++;
                }
            }else {
                if(find_free_start) {
                    find_free_start = 0;
                }
            }
        }

        if(free_pages_cnt >= need_pages) {
            break;
        }

        if(alloc_start_gpid >= hand->flash_pages) {
            alloc_start_gpid -= hand->flash_pages;
        }
    }while((alloc_start_gpid != hand->alloc_gpid_index));

    sf_data_element_s *tmp_element = (sf_data_element_s *)sf_malloc(sizeof(sf_data_element_s)*element_cnt);
    if(NULL == tmp_element) {
        return SF_MALLOC_ERR;
    }

    free_pages_cnt = 0;
    element_cnt = 0;
    last_free_bid = 0xffff;
    find_free_start = 0;
    alloc_start_gpid = hand->alloc_gpid_index;

    do {
        if(hand->pg_bm[alloc_start_gpid/8] == 0xff) {
            alloc_start_gpid += (8 - (alloc_start_gpid % 8));
            if(alloc_start_gpid >= hand->flash_pages) {
                alloc_start_gpid -= hand->flash_pages;
            }
            continue;
        }

        sf_uint32_t i = 0;
        for(i = (alloc_start_gpid % 8);i < 8;i++,alloc_start_gpid++) {
            if(free_pages_cnt >= need_pages) {
                if(alloc_start_gpid >= hand->flash_pages) {
                    alloc_start_gpid -= hand->flash_pages;
                }
                hand->alloc_gpid_index = alloc_start_gpid;

                *element = tmp_element;
                *element_num = element_cnt;
                return SF_OK;
            }
            
            if(!(hand->pg_bm[alloc_start_gpid/8] & (1 << (alloc_start_gpid%8)))) {
                hand->pg_bm[alloc_start_gpid/8] |= (1 << (alloc_start_gpid%8));                
                free_pages_cnt++;
                hand->free_pages--;

                // span block process
                if(0xffff == last_free_bid) {
                    last_free_bid = BLOCK_ID_FROM_GPID(hand,alloc_start_gpid);
                }else if(BLOCK_ID_FROM_GPID(hand,alloc_start_gpid) != last_free_bid) {
                    last_free_bid = BLOCK_ID_FROM_GPID(hand,alloc_start_gpid);
                    element_cnt++;
                    find_free_start = 1;

                    tmp_element[element_cnt-1].block_id = last_free_bid;
                    tmp_element[element_cnt-1].start_page_id = BPG_ID_FROM_GPID(hand,alloc_start_gpid);
                    tmp_element[element_cnt-1].end_page_id = BPG_ID_FROM_GPID(hand,alloc_start_gpid);

                    continue;
                }

                //  search sequence pages
                if(0 == find_free_start) { 
                    find_free_start = 1;
                    element_cnt++;

                    tmp_element[element_cnt-1].block_id = last_free_bid;
                    tmp_element[element_cnt-1].start_page_id = BPG_ID_FROM_GPID(hand,alloc_start_gpid);
                    tmp_element[element_cnt-1].end_page_id = BPG_ID_FROM_GPID(hand,alloc_start_gpid);
                }else {
                    tmp_element[element_cnt-1].end_page_id = BPG_ID_FROM_GPID(hand,alloc_start_gpid);
                }
            }else {
                if(find_free_start) {
                    find_free_start = 0;
                }
            }
        }

        if(alloc_start_gpid >= hand->flash_pages) {
            alloc_start_gpid -= hand->flash_pages;
        }
    }while((alloc_start_gpid != hand->alloc_gpid_index));

    return SF_COM_ERR;
}

static sf_ret __sf_free_data_by_elem(sf_hand_s *hand,sf_data_element_s *element,sf_uint16_t element_num)
{
    if(NULL == hand || NULL == element) {
        return SF_INVALID_PARM;
    }
    
    int i,j;
    for(i = 0;i < element_num;i++) {
        for(j = element[i].start_page_id; j <= element[i].end_page_id;j++) {
            __sf_free_page(hand,element[i].block_id,j);
        }
    }

    return SF_OK;
}

static sf_ret __sf_block_backup(sf_hand_s *hand,const sf_uint16_t block_id,\
                                sf_uint8_t *blk_buf,const sf_uint16_t blk_buf_len)
{
    if(NULL == hand || \
       NULL == blk_buf || \
       blk_buf_len < hand->cfg.block_sz) {
        return SF_INVALID_PARM;
    }

    sf_ret ret;
    ret = __sf_block_read(hand,1,block_id,blk_buf,hand->cfg.block_sz);
    if(SF_OK != ret) {
        return ret;
    }

    ret = __sf_swap_block_write(hand,blk_buf,hand->cfg.block_sz);
    if(SF_OK != ret) {
        return ret;
    }

    return SF_OK;
}

sf_ret __sf_delete(sf_hand_s *hand,const sf_index_s *index) 
{
    if(NULL == hand || NULL == index) {
        return SF_INVALID_PARM;
    }

    sf_uint8_t *data = sf_malloc(hand->cfg.block_sz);
    if(NULL == data) {
        return SF_MALLOC_ERR;
    }

    // old block backup
    sf_ret ret;
    ret = __sf_block_backup(hand,index->block_id,data,hand->cfg.block_sz);
    if(SF_OK != ret) {
        sf_free(data);
        return ret;
    }

    sf_block_mag_s *block_mag = (sf_block_mag_s *)data;
    block_mag->pg_bm[index->bpg_id/8] &= ~(1 << (index->bpg_id%8)); // clear the bit in the block manage
    block_mag->wr_count += 1;
    block_mag->crc32 = hand->cfg.sf_crc32(data+8,hand->cfg.block_sz-8);

    // save to flash  
    ret = __sf_block_write_direct(hand,PHY_USER_FLASH_ADDR(hand,index->block_id,0),data,hand->cfg.block_sz);
	if (SF_OK != ret) {
		sf_free(data);
		return ret;
	}

	// delete the index block
	ret = __sf_free_page(hand, index->block_id, index->bpg_id);
	if (SF_OK != ret) {
		sf_free(data);
		return ret;
	}

	// delete the data index
	ret = __sf_free_data_by_elem(hand, index->element, index->element_num); // delete the data element
	if (SF_OK != ret) {
		sf_free(data);
		return ret;
	}

	__sf_index_cache_delete_by_idx(hand, index); // delete the cache
	hand->sf_index_cnt--;
    sf_free(data);

    return SF_OK;
}

/***********************************************************
*  Function: sf_delete
*  description: simple flash data delete method
*  Input: hand->simple flash hand
*         name->delete data name
*  Output: none
*  Return: sf_ret
***********************************************************/
sf_ret sf_delete(sf_hand_s *hand,const sf_char_t *name)
{
    if(NULL == hand || NULL == name) {
        return SF_INVALID_PARM;
    }

    if(strlen(name) > DATA_NAME_LMT) {
        return SF_COM_ERR;
    }

    sf_ret ret;
    sf_index_s *index;
    ret = __sf_find_index(hand,name,&index);
    if(SF_OK != ret) {
        return ret;
    }

    ret = __sf_delete(hand,index);
    
    return ret;
}

/***********************************************************
*  Function: sf_fuzzy_delete
*  description: simple flash data delete method
*  Input: hand->simple flash hand
*         fuzzy_name->delete data name
*  Output: none
*  Return: sf_ret
***********************************************************/
sf_ret sf_fuzzy_delete(sf_hand_s *hand,const sf_char_t *fuzzy_name)
{
    if(NULL == hand || NULL == fuzzy_name) {
        return SF_INVALID_PARM;
    }

    if(strlen(fuzzy_name) > DATA_NAME_LMT) {
        return SF_COM_ERR;
    }

    sf_ret ret = SF_OK;
    sf_uint8_t *read_buf = sf_malloc(hand->page_sz);
    if(NULL == read_buf) {
        return SF_MALLOC_ERR;
    }

    int i;
    for(i = 0;i < hand->block_nums;i++) { // find the index from flash
        ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,i,0),read_buf,hand->page_sz);
        if(SF_OK != ret) {
            sf_free(read_buf);
            return ret;
        }

        sf_block_mag_s *block_mag = (sf_block_mag_s *)read_buf;
        int j,k;
        for(j= 0;j < block_mag->pg_bm_len;j++) {
            if(0 == block_mag->pg_bm[j]) {
                continue;
            }
            
            for(k = 0;k < 8;k++) {
                if(0 == (block_mag->pg_bm[j] & (1 << k))) {
                    continue;
                }

                sf_index_s *tmp_index = NULL;
                sf_uint8_t bpg_id = j*8 + k;
                ret = __make_sf_index(hand,block_mag->block_id,bpg_id,&tmp_index);
                if(SF_OK != ret && SF_PG_SIZE_NOT_ENOUGH != ret) {
                    sf_free(read_buf);
                    return ret;
                }else if(SF_PG_SIZE_NOT_ENOUGH == ret) {// unvalid index
                    #if 1
                    printf("%s:%d i:%d block_mag->pg_bm[%d]=%x\n",\
                           __FILE__,__LINE__,i,j,block_mag->pg_bm[j]);
                    #endif
                    __sf_free_page(hand,block_mag->block_id,bpg_id);
                    continue;
                }

                if((0 == strncmp(fuzzy_name, tmp_index->name,strlen(fuzzy_name)))) {
                    __sf_delete(hand,tmp_index);
                    sf_free(tmp_index);
                    continue;
                }
                sf_free(tmp_index);
            }
        }
    }

    sf_free(read_buf);
    return SF_OK;
}

typedef struct {
    sf_uint16_t block_id;
    sf_uint8_t *data;
    sf_uint32_t len;
    sf_data_element_s *element;
    sf_uint16_t element_num;
}blk_write_s;

/***********************************************************
*  Function: sf_write
*  description: simple flash write method
*  Input: hand->simple flash hand
*         name->data name
*         data->data
*         len->data len
*  Output: none
*  Return: sf_ret
***********************************************************/
sf_ret sf_write(sf_hand_s *hand,const sf_char_t *name,\
                const sf_uint8_t *data,const sf_uint32_t len)
{
    if(NULL == hand || NULL == name || NULL == data) {
        return SF_INVALID_PARM;
    }

    if(strlen(name) > DATA_NAME_LMT) {
        return SF_COM_ERR;
    }

    sf_ret ret;
    sf_index_s *index;
    ret = __sf_find_index(hand,name,&index);
    if(SF_OK != ret && SF_NO_FIND != ret) {
        return ret;
    }

    // if already have the same name index object then delete it first
    if(SF_OK == ret) {
        ret = __sf_delete(hand,index);
        if(SF_OK != ret) {
            return ret;
        }
    }

    // alloc page for index
    sf_uint16_t block_id = 0;
    sf_uint8_t bpg_id = 0;
    ret = __sf_alloc_page(hand,&block_id,&bpg_id);
    if(SF_OK != ret) {
        return ret;
    }

    sf_data_element_s *element = NULL;
    sf_uint16_t element_num = 0;
    ret = __sf_alloc_data_elem_by_len(hand,len,&element,&element_num);
    if(SF_OK != ret) {
        __sf_free_page(hand,block_id,bpg_id);
        return ret;
    }

    int index_len = sizeof(sf_index_s)+strlen(name)+1+element_num*sizeof(sf_data_element_s);
    if(index_len > hand->page_sz) {
        __sf_free_page(hand,block_id,bpg_id);
        __sf_free_data_by_elem(hand,element,element_num);
        return SF_PG_SIZE_NOT_ENOUGH;
    }

    // alloc index
    index = (sf_index_s *)sf_malloc(index_len);
    if(NULL == index) {
        sf_free(element);
        __sf_free_page(hand,block_id,bpg_id);
        __sf_free_data_by_elem(hand,element,element_num);
        return SF_MALLOC_ERR;
    }

    // index copy
    index->crc32 = hand->cfg.sf_crc32((sf_uint8_t *)data,len);
    index->data_len = len;
    index->block_id = block_id;
    index->bpg_id = bpg_id;
    index->element_num = element_num;
    index->element = (sf_data_element_s *)((sf_uint8_t *)index + sizeof(sf_index_s)+(strlen(name)+1));
    index->name_len = strlen(name)+1;
    memcpy(index->name,name,index->name_len);
    memcpy(index->element,element,element_num*sizeof(sf_data_element_s));

    sf_free(element);
    element = index->element;

    // how many block need to write
    sf_uint16_t blocks = 1;
    if(index->block_id != element[0].block_id) {
        blocks++;
    }

    int i = 0;
    for(i = 0;i < element_num-1;i++) {
        if(element[i].block_id != element[i+1].block_id) {
            blocks++;
        }
    }

    // make block write manage 
    blk_write_s *blk_wr = sf_malloc(sizeof(blk_write_s)*blocks);
    if(NULL == blk_wr) {
        __sf_free_page(hand,block_id,bpg_id);
        __sf_free_data_by_elem(hand,element,element_num);
        sf_free(index);
        return SF_MALLOC_ERR;
    }
    memset(blk_wr,0,sizeof(blk_write_s)*blocks);

    sf_uint16_t blk_off = 0;
    if(index->block_id != element[0].block_id) {
        blk_off++;
    }

    sf_uint16_t fir_data_blk = blk_off;
    for(i = 0;i < element_num;i++) {
        if(NULL == blk_wr[blk_off].data || \
           NULL == blk_wr[blk_off].element) {
            blk_wr[blk_off].block_id = element[i].block_id;
            if(blk_off == fir_data_blk) {
                blk_wr[blk_off].data = (sf_uint8_t *)data;
            }else {
                blk_wr[blk_off].data = blk_wr[blk_off-1].data + blk_wr[blk_off-1].len;
            }
            blk_wr[blk_off].element = &(element[i]);
        }

        blk_wr[blk_off].len += (element[i].end_page_id - element[i].start_page_id + 1) * hand->page_sz;
        blk_wr[blk_off].element_num++;

        if(i < element_num-1) {
            if(element[i].block_id != element[i+1].block_id) {
                blk_off++;
            }
        }else { // cut more data len
			blk_wr[blk_off].len -= (len % hand->page_sz) ? (hand->page_sz - (len % hand->page_sz)) : 0;
        }
    }

    sf_uint8_t *blk_buf = sf_malloc(hand->cfg.block_sz);
    if(NULL == blk_buf) {
        ret = SF_MALLOC_ERR;
        goto err_ret;
    }

    // span block write
    for(i = blocks;i != 0;i--) {
        ret = __sf_block_backup(hand,blk_wr[i-1].block_id,blk_buf,hand->cfg.block_sz);
        if(SF_OK != ret) {
            goto err_ret;
        }
        
        if(i != 1) {
            ret = __sf_block_write(hand,NULL,blk_wr[i-1].data,blk_wr[i-1].len,\
                                   blk_wr[i-1].element,blk_wr[i-1].element_num,\
                                   blk_buf,hand->cfg.block_sz);
            if(SF_OK != ret) {
                goto err_ret;
            }
        }else {
            ret = __sf_block_write(hand,index,blk_wr[i-1].data,blk_wr[i-1].len,\
                                   blk_wr[i-1].element,blk_wr[i-1].element_num,\
                                   blk_buf,hand->cfg.block_sz);
            if(SF_OK != ret) {
                goto err_ret;
            }
        }
    }

    // index insert
    __sf_index_cache_insert(hand,index,1);
    hand->sf_index_cnt++;

    sf_free(blk_wr);
    sf_free(blk_buf);
	return SF_OK;

err_ret:
    __sf_free_page(hand,block_id,bpg_id);
    __sf_free_data_by_elem(hand,element,element_num);
    sf_free(blk_wr);
    sf_free(index);
    if(blk_buf) {
        sf_free(blk_buf);
    }

    return ret;
}

static sf_ret __sf_read_from_index(sf_hand_s *hand,const sf_index_s *index,\
                                   sf_uint8_t **data,sf_uint32_t *len)
{
    sf_ret ret;
    sf_uint8_t *data_buf = sf_malloc(index->data_len);
    if(NULL == data_buf) {
        return SF_MALLOC_ERR;
    }

    int i,j;
    sf_uint32_t offset = 0;
    
    for(i = 0; i < index->element_num;i++) {
        for(j = index->element[i].start_page_id; j <= index->element[i].end_page_id;j++) {
            if((offset+hand->page_sz) <= index->data_len) {
                ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,index->element[i].block_id,j),\
                                        data_buf+offset,hand->page_sz);
                offset += hand->page_sz;
            }else {
                ret = hand->cfg.sf_read(PHY_USER_FLASH_ADDR(hand,index->element[i].block_id,j),\
                                        data_buf+offset,(index->data_len-offset));
                offset += (index->data_len-offset);
                break;
            }
        }
    }

    if(index->crc32 != hand->cfg.sf_crc32(data_buf,index->data_len)) {
        sf_free(data_buf);
        return SF_CRC_ERR;
    }

    *data = data_buf;
    if(len) {
        *len = index->data_len;
    }
    
	return SF_OK;
}

/***********************************************************
*  Function: sf_read
*  description: simple flash read method
*  Input: hand->simple flash hand
*         name->data name
*         data->write data
*         len->data len
*  Output: data->read data,need call sf_mem_free to free
*          len->read data len 
*  Return: sf_ret
***********************************************************/
sf_ret sf_read(sf_hand_s *hand,const sf_char_t *name,\
               sf_uint8_t **data,sf_uint32_t *len)
{
    //if(name) {
    //    PR_DEBUG("name:%s len:%d",name,strlen(name));
    //}
    
    if(NULL == hand || NULL == name || NULL == data) {
        return SF_INVALID_PARM;
    }

    if(strlen(name) > DATA_NAME_LMT) {
        return SF_COM_ERR;
    }
    
    sf_ret ret;
    sf_index_s *index;
    ret = __sf_find_index(hand,name,&index);
    if(SF_OK != ret) {
        PR_DEBUG("name:%s ret:%d",name,ret);
        return ret;
    }

    ret = __sf_read_from_index(hand,index,data,len);
    if(SF_OK != ret) {
        PR_DEBUG("ret:%d",ret);
        return ret;
    }
    
	return SF_OK;
}

/***********************************************************
*  Function: sf_fuzzy_read
*  description: simple flash read method
*  Input: hand->simple flash hand
*         fuzzy_name->fuzzy_name data name
*         data->write data
*         len->data len
*         gpid->read the fuzzy name index start gpid
*  Output: data->read data,need call sf_mem_free to free
*          len->read data len
*          gpid->the find index gpid +1
*  Return: sf_ret
***********************************************************/
sf_ret sf_fuzzy_read(sf_hand_s *hand,const sf_char_t *fuzzy_name,sf_uint32_t *gpid,\
                     sf_uint8_t **data,sf_uint32_t *len)
{
    if(NULL == hand || NULL == fuzzy_name || NULL == data) {
        return SF_INVALID_PARM;
    }

    sf_ret ret = SF_OK;
    sf_index_s *index;
    
    ret = __sf_fuzzy_find_index(hand,fuzzy_name,gpid,&index);
    if(SF_OK != ret) {
        return ret;
    }

    ret = __sf_read_from_index(hand,index,data,len);
    sf_free(index);
    if(SF_OK != ret) {
        return ret;
    }

    return SF_OK;
}


sf_ret __swap_block_data_restore(sf_hand_s *hand,sf_uint8_t *blk_buf,const sf_uint32_t blk_buf_len)
{
    if(NULL == hand) {
        return SF_INVALID_PARM;
    }

    sf_block_mag_s *swap_blk_mag = sf_malloc(sizeof(sf_block_mag_s));
    if(NULL == swap_blk_mag) {
        return SF_MALLOC_ERR;
    }

    sf_ret ret;
    int i;
    for(i = 0;i < hand->swap_block_nums;i++) {
        // verify swap block is ok
        ret = __sf_swap_block_read(hand,1,i,blk_buf,hand->cfg.block_sz);
        if(SF_BLOCK_UNVALID != ret && SF_OK != ret) {
            sf_free(swap_blk_mag);
            return ret;
        }
        if(SF_BLOCK_UNVALID == ret) {
            continue;
        }

        ret = hand->cfg.sf_read(PHY_USER_SWAP_FLASH_ADDR(hand,i),(sf_uint8_t *)swap_blk_mag,sizeof(sf_block_mag_s));
        if(SF_OK != ret) {
            sf_free(swap_blk_mag);
            return ret;
        }

        // verify user block is ok
        ret =  __sf_block_read(hand,1,swap_blk_mag->block_id,blk_buf,hand->cfg.block_sz);
        if(SF_OK == ret) {
            sf_block_mag_s *user_blkm = (sf_block_mag_s *)blk_buf;
            if(user_blkm->wr_count >= swap_blk_mag->wr_count) { // is need restore?
                continue;
            }
        }

        // read swap block again
        ret = __sf_swap_block_read(hand,0,i,blk_buf,hand->cfg.block_sz);
        if(SF_OK != ret) {
            sf_free(swap_blk_mag);
            return ret;
        }

        ret = __sf_block_write_direct(hand,PHY_USER_FLASH_ADDR(hand,swap_blk_mag->block_id,0),\
                                      blk_buf,hand->cfg.block_sz);
        if(SF_OK != ret) {
            sf_free(swap_blk_mag);
            return ret;
        }
    }
	sf_free(swap_blk_mag);

    return SF_OK;
}

/***********************************************************
*  Function: sf_mem_free
*  description: simple flash memory free
*  Input: mem->memory to free
*  Output: none
*  Return: sf_ret
*  Note: "sf_read" output data maybe use this method to free 
*        data memory
***********************************************************/
void sf_mem_free(void *mem)
{
    sf_free(mem);
}

/***********************************************************
*  Function: sf_format
*  description: simple flash format
*  Input: hand
*  Output: none
*  Return: sf_ret
***********************************************************/
sf_ret sf_format(sf_hand_s *hand)
{
    if(NULL == hand) {
        return SF_INVALID_PARM;
    }
    
    int i;
	sf_ret ret;
    // swap flash area erase
    for(i = 0;i < hand->swap_block_nums;i++) {
        ret = hand->cfg.sf_erase(PHY_USER_SWAP_FLASH_ADDR(hand,i),hand->cfg.block_sz);
        if(SF_OK != ret) {
            return ret;
        }
    }

    sf_uint8_t *buf = sf_malloc(hand->cfg.block_sz);
    if(NULL == buf) {
        return SF_MALLOC_ERR;
    }
    memset(buf,0xff,hand->cfg.block_sz);

    // data format
    for(i = 0;i < hand->block_nums;i++) {
        sf_block_mag_s *blk_mag = (sf_block_mag_s *)buf;
        
        blk_mag->magic = SIMPLE_BF_MAGIC;
        blk_mag->block_id = i;
        blk_mag->wr_count = 0;
        blk_mag->pg_bm_len = hand->block_pages / 8;
        blk_mag->pg_bm_len += (hand->block_pages % 8) ? 1 : 0;
        memset(blk_mag->pg_bm,0,blk_mag->pg_bm_len);
        blk_mag->crc32 = hand->cfg.sf_crc32(buf+8,hand->cfg.block_sz-8);
        
        ret = __sf_block_write_direct(hand,PHY_USER_FLASH_ADDR(hand,i,0),buf,hand->cfg.block_sz);
        if(SF_OK != ret) {
            sf_free(buf);
            return ret;
        }
    }
    sf_free(buf);

    return SF_OK;
}

