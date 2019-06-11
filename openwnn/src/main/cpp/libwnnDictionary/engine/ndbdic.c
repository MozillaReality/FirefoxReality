/*
 * Copyright (C) 2008-2012  OMRON SOFTWARE Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nj_lib.h"
#include "nj_err.h"
#include "nj_ext.h"
#include "nj_dic.h"
#include "njd.h"


#define NODE_TERM(x) ((NJ_UINT8)(0x80 & (*(x))))
#define NODE_LEFT_EXIST(x) ((NJ_UINT8)(0x40 & (*(x))))
#define NODE_DATA_EXIST(x) ((NJ_UINT8)(0x20 & (*(x))))
#define NODE_IDX_EXIST(x) ((NJ_UINT8)(0x10 & (*(x))))
#define NODE_IDX_CNT(x) ((NJ_UINT8)((0x0f & (*(x))) + 2))

#define STEM_TERMINETER(x) ((NJ_UINT8)(0x80 & (*(x))))

#define STEM_NO_CONV_FLG(x) ((NJ_UINT8)(0x40 & (*(x))))

#define TERM_BIT (1)            
#define INDEX_BIT (8)           

#define APPEND_YOMI_FLG(h) ((NJ_UINT8)(0x80 & (*((h) + 0x1C))))
#define HINSI_NO_TOP_ADDR(h) ((NJ_UINT8*)((h) + NJ_INT32_READ((h) + 0x1D)))
#define FHINSI_NO_CNT(h) ((NJ_INT16)(NJ_INT16_READ((h) + 0x21)))
#define BHINSI_NO_CNT(h) ((NJ_INT16)(NJ_INT16_READ((h) + 0x23)))
#define HINSI_NO_BYTE(h) ((NJ_UINT8)(*((h) + 0x25)))
#define HINDO_NO_TOP_ADDR(h) ((NJ_UINT8*)((h) + NJ_INT32_READ((h) + 0x26)))
#define HINDO_NO_CNT(h) ((NJ_UINT8)(*((h) + 0x2A)))
#define STEM_AREA_TOP_ADDR(h) ((NJ_UINT8*)((h) + NJ_INT32_READ((h) + 0x2B)))
#define BIT_CANDIDATE_LEN(h) ((NJ_UINT8)(*((h) + 0x2F)))
#define BIT_FHINSI(h) ((NJ_UINT8)(*((h) + 0x30)))
#define BIT_BHINSI(h) ((NJ_UINT8)(*((h) + 0x31)))
#define BIT_HINDO_LEN(h) ((NJ_UINT8)(*((h) + 0x32)))
#define BIT_MUHENKAN_LEN(h) ((NJ_UINT8)(*((h) + 0x33)))
#define BIT_YOMI_LEN(h) ((NJ_UINT8)(*((h) + 0x35)))
#define YOMI_INDX_TOP_ADDR(h) ((NJ_UINT8*)((h) + NJ_INT32_READ((h) + 0x42)))
#define YOMI_INDX_CNT(h) ((NJ_INT16)(*((h) + 0x46)))
#define YOMI_INDX_SIZE(h) ((NJ_INT8)(*((h) + 0x47)))
#define NODE_AREA_TOP_ADDR(h) ((NJ_UINT8*)((h) + NJ_INT32_READ((h) + 0x48)))
#define BIT_NODE_AREA_DATA_LEN(h) ((NJ_UINT8)(*((h) + 0x4C)))
#define BIT_NODE_AREA_LEFT_LEN(h) ((NJ_UINT8)(*((h) + 0x4D)))
#define NODE_AREA_MID_ADDR(h) ((NJ_UINT32)(NJ_INT32_READ((h) + 0x4E)))
#define CAND_IDX_AREA_TOP_ADDR(h) ((NJ_UINT8*)((h) + NJ_INT32_READ((h) + 0x52)))
#define CAND_IDX_AREA_CNT(h) ((NJ_UINT32)(((NJ_INT32_READ((h) + 0x56)) >> 8) & 0x00FFFFFF))
#define CAND_IDX_AREA_SIZE(h) ((NJ_UINT8)(*((h) + 0x59)))

#define WORD_LEN(x) ((NJ_UINT16)(0x007F & (x)))

#define CURRENT_INFO_SET ((NJ_UINT8)(0x10))

#define COMP_DIC_FREQ_DIV 63      

#define LOC_CURRENT_NO_ENTRY  0xffffffffU

typedef struct {
    NJ_UINT16 stem_size;        
    NJ_UINT16 term;             
    NJ_UINT16 no_conv_flg;      
    NJ_HINDO hindo;             
    NJ_UINT16 hindo_jitu;       
    NJ_UINT16 candidate_size;   
    NJ_UINT16 yomi_size;        
    NJ_UINT16 fhinsi;           
    NJ_UINT16 bhinsi;           
    NJ_UINT16 fhinsi_jitu;      
    NJ_UINT16 bhinsi_jitu;      
} STEM_DATA_SET;

static NJ_INT16 get_stem_next(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data);
static void get_stem_word(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data, STEM_DATA_SET *stem_set, NJ_UINT8 check);
static void get_stem_cand_data(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data, STEM_DATA_SET *stem_set);
static NJ_UINT16 get_stem_yomi_data(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data,STEM_DATA_SET *stem_set);
static NJ_UINT16 get_stem_yomi_size(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data, NJ_UINT16 yomi_size);
static NJ_UINT16 get_stem_yomi_string(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data, NJ_CHAR *yomi, NJ_UINT16 yomi_pos, NJ_UINT16 yomi_size, NJ_UINT16 size);
static NJ_INT16 search_node(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset);
static NJ_INT16 bdic_search_data(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset);
static NJ_INT16 bdic_search_fore_data(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset);

static NJ_HINDO get_stem_hindo(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data);

static NJ_INT16 search_node2(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset,
                             NJ_UINT16 hidx);
static NJ_INT16 bdic_search_fore_data2(NJ_SEARCH_CONDITION *condition,
                                       NJ_SEARCH_LOCATION_SET *loctset, NJ_UINT16 hidx);
static NJ_INT16 search_yomi_node(NJ_UINT8 operation, NJ_UINT8 *node,
                                 NJ_UINT8 *now, NJ_UINT16 idx_no,
                                 NJ_CHAR  *yomi, NJ_UINT16 yomilen,
                                 NJ_UINT8 *root, NJ_UINT8 *node_mid,
                                 NJ_UINT16 bit_left, NJ_UINT16 bit_data,
                                 NJ_UINT8 *data_top,
                                 NJ_INT16 ytbl_cnt, NJ_UINT16 y,
                                 NJ_UINT8 *ytbl_top, NJ_CACHE_INFO *storebuf,
                                 NJ_UINT8 **con_node, NJ_UINT32 *data_offset);
static NJ_INT16 get_node_bottom(NJ_CHAR *yomi, NJ_UINT8 *now, NJ_UINT8 *node_mid,
                                NJ_UINT8 *data_top, NJ_UINT16 bit_left,
                                NJ_UINT16 bit_data, NJ_UINT32 top,
                                NJ_DIC_HANDLE handle, NJ_UINT32 *ret_bottom);
static NJ_INT16 bdic_get_next_data(NJ_UINT8 *data_top, NJ_UINT8 *data_end,
                                   NJ_SEARCH_LOCATION_SET *loctset,
                                   NJ_SEARCH_CACHE *psrhCache, NJ_UINT16 abIdx);
static NJ_INT16 bdic_get_word_freq(NJ_UINT8 *data_top, NJ_SEARCH_LOCATION_SET *loctset,
                                   NJ_SEARCH_CACHE *psrhCache, NJ_UINT16 abIdx);

static NJ_HINDO get_stem_hindo(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data)
{
    NJ_UINT8 flg_bit;
    NJ_UINT16 data;
    NJ_UINT16 pos, j, bit_all;


    
    flg_bit = BIT_MUHENKAN_LEN(hdl);
    if (NJ_GET_DIC_FMT(hdl) != NJ_DIC_FMT_KANAKAN) {
        flg_bit++;  
    }

    if (BIT_HINDO_LEN(hdl)) {
        
        bit_all = (NJ_UINT16)(TERM_BIT + flg_bit);
        pos = (NJ_UINT16)(bit_all >> 3);
        data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));
        
        
        j = (NJ_UINT16)(bit_all & 0x0007);

        return GET_BITFIELD_16(data, j, BIT_HINDO_LEN(hdl));
    } else {
        
        return 0;
    }
}

static NJ_INT16 get_stem_next(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data)
{
    NJ_UINT8 flg_bit;
    NJ_UINT16 data;
    NJ_UINT16 pos, j, bit_all;
    NJ_UINT16 stem_size, cand_bit, yomi_bit;
    NJ_UINT16 candidate_size, yomi_size;


    
    flg_bit = BIT_MUHENKAN_LEN(hdl);
    if (NJ_GET_DIC_FMT(hdl) != NJ_DIC_FMT_KANAKAN) {
        flg_bit++;  
    }

    
    
    bit_all = (NJ_UINT16)(TERM_BIT + flg_bit + 
                          BIT_HINDO_LEN(hdl) + 
                          BIT_FHINSI(hdl) + 
                          BIT_BHINSI(hdl));
    pos = (NJ_UINT16)(bit_all >> 3);
    data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));

    
    j = (NJ_UINT16)(bit_all & 0x0007);
    cand_bit = BIT_CANDIDATE_LEN(hdl);
    
    candidate_size = GET_BITFIELD_16(data, j, cand_bit);
    bit_all += cand_bit;

    
    if (APPEND_YOMI_FLG(hdl) && STEM_TERMINETER(stem_data)) {
        
        
        pos = (NJ_UINT16)(bit_all >> 3);
        data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));

        
        j = (NJ_UINT16)(bit_all & 0x0007);
        yomi_bit = BIT_YOMI_LEN(hdl);
        
        yomi_size = GET_BITFIELD_16(data, j, yomi_bit);
        bit_all += yomi_bit;
    } else {
        yomi_size = 0;  
    }

    
    stem_size = GET_BIT_TO_BYTE(bit_all);

    
    stem_size += candidate_size;

    
    stem_size += yomi_size;

    
    return stem_size;
}

static void get_stem_word(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data, STEM_DATA_SET *stem_set, NJ_UINT8 check)
{
    NJ_UINT8 flg_bit;
    NJ_UINT16 data;
    NJ_UINT16 pos, j, bit_all = 0;
    NJ_UINT16 bit;
    NJ_UINT16 dpos = 0;
    NJ_INT16 next;
    NJ_UINT8 b;
    NJ_UINT8 *wkc;

   
    
    flg_bit = BIT_MUHENKAN_LEN(hdl);
    if (NJ_GET_DIC_FMT(hdl) != NJ_DIC_FMT_KANAKAN) {
        flg_bit++;  
    }

    if (BIT_HINDO_LEN(hdl)) {
        
        bit_all = (NJ_UINT16)(TERM_BIT + flg_bit);
        pos = (NJ_UINT16)(bit_all >> 3);
        data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));
        
        
        j = (NJ_UINT16)(bit_all & 0x0007);

        stem_set->hindo = GET_BITFIELD_16(data, j, BIT_HINDO_LEN(hdl));
    } else {
        
        stem_set->hindo = 0;
    }
    
    stem_set->hindo_jitu = (NJ_UINT16)(*(HINDO_NO_TOP_ADDR(hdl) + stem_set->hindo));

    if (BIT_FHINSI(hdl)) {
        
        
        bit_all = (NJ_UINT16)(TERM_BIT + flg_bit + BIT_HINDO_LEN(hdl));
        pos = (NJ_UINT16)(bit_all >> 3);
        data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));
        
        
        j = (NJ_UINT16)(bit_all & 0x0007);
        
        stem_set->fhinsi = GET_BITFIELD_16(data, j, BIT_FHINSI(hdl));
    } else {
        stem_set->fhinsi = 0;
    }

    
    b = HINSI_NO_BYTE(hdl);
    wkc = (NJ_UINT8*)(HINSI_NO_TOP_ADDR(hdl) + (b * (NJ_UINT16)(stem_set->fhinsi)));

    
    if (b == 2) {
        stem_set->fhinsi_jitu = (NJ_UINT16)(NJ_INT16_READ(wkc));
    } else {
        stem_set->fhinsi_jitu = (NJ_UINT16)*wkc;
    }
    
    if (BIT_BHINSI(hdl)) {
        
        
        bit_all = (NJ_UINT16)(TERM_BIT + flg_bit + BIT_HINDO_LEN(hdl) + BIT_FHINSI(hdl));
        pos = (NJ_UINT16)(bit_all >> 3);
        data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));
        
        
        j = (NJ_UINT16)(bit_all & 0x0007);
        
        stem_set->bhinsi = GET_BITFIELD_16(data, j, BIT_BHINSI(hdl));
    } else {
        stem_set->bhinsi = 0;
    }
    
    wkc = (NJ_UINT8*)(HINSI_NO_TOP_ADDR(hdl)
                      + (b * (FHINSI_NO_CNT(hdl) + (NJ_UINT16)(stem_set->bhinsi))));
    
    if (b == 2) {
        stem_set->bhinsi_jitu = (NJ_UINT16)(NJ_INT16_READ(wkc));
    } else {
        stem_set->bhinsi_jitu = (NJ_UINT16)*wkc;
    }

    
    if (check != 1) {
        
        
        bit_all = (NJ_UINT16)(TERM_BIT + flg_bit + 
                              BIT_HINDO_LEN(hdl) + 
                              BIT_FHINSI(hdl) + 
                              BIT_BHINSI(hdl));
        pos = (NJ_UINT16)(bit_all >> 3);
        data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));

        
        j = (NJ_UINT16)(bit_all & 0x0007);
        bit = BIT_CANDIDATE_LEN(hdl);
        
        stem_set->candidate_size = GET_BITFIELD_16(data, j, bit);
        bit_all += bit;
    }
    
    if (check == 0) {
        stem_set->yomi_size = 0;

        
        if (APPEND_YOMI_FLG(hdl) && STEM_TERMINETER(stem_data)) {
            pos = (NJ_UINT16)(bit_all >> 3);
            data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));

            
            j = (NJ_UINT16)(bit_all & 0x0007);
            bit = BIT_YOMI_LEN(hdl);
            
            stem_set->yomi_size = GET_BITFIELD_16(data, j, bit);
            bit_all += bit;

            
            
            dpos = GET_BIT_TO_BYTE(bit_all);
            dpos += stem_set->candidate_size;
            
        } else if (APPEND_YOMI_FLG(hdl)) {
            while (!(STEM_TERMINETER(stem_data))) {
                next = get_stem_next(hdl, stem_data);
                stem_data += next;
            }
            
            dpos = get_stem_yomi_data(hdl, stem_data, stem_set);
        }

        if (stem_set->yomi_size) {
           
            stem_set->yomi_size = get_stem_yomi_size(hdl, stem_data + dpos, stem_set->yomi_size);
        }
    }
}

static void get_stem_cand_data(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data, STEM_DATA_SET *stem_set)
{
    NJ_UINT8 flg_bit;
    NJ_UINT16 data;
    NJ_UINT16 pos, j, bit_all;
    NJ_UINT16 cand_bit, yomi_bit;


    
    flg_bit = BIT_MUHENKAN_LEN(hdl);
    if (NJ_GET_DIC_FMT(hdl) != NJ_DIC_FMT_KANAKAN) {
        flg_bit++;  
    }

    
    
    bit_all = (NJ_UINT16)(TERM_BIT + flg_bit + 
                          BIT_HINDO_LEN(hdl) + 
                          BIT_FHINSI(hdl) + 
                          BIT_BHINSI(hdl));
    pos = (NJ_UINT16)(bit_all >> 3);
    data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));

    
    cand_bit = BIT_CANDIDATE_LEN(hdl);
    j = (NJ_UINT16)(bit_all & 0x0007);
    
    stem_set->candidate_size = GET_BITFIELD_16(data, j, cand_bit);
    bit_all += cand_bit;

    
    if (APPEND_YOMI_FLG(hdl) && STEM_TERMINETER(stem_data)) {
        
        yomi_bit = BIT_YOMI_LEN(hdl);
        bit_all += yomi_bit;
    }

    
    stem_set->stem_size = GET_BIT_TO_BYTE(bit_all);
}

static NJ_UINT16 get_stem_yomi_data(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data,STEM_DATA_SET *stem_set)
{
    NJ_UINT16 flg_bit;
    NJ_UINT16 data;
    NJ_UINT16 cand_bit, yomi_bit;
    NJ_UINT16 pos, j, bit_all;
    NJ_UINT16 yomi_pos;
    NJ_UINT16 candidate_size;


    
    flg_bit = BIT_MUHENKAN_LEN(hdl);
    if (NJ_GET_DIC_FMT(hdl) != NJ_DIC_FMT_KANAKAN) {
        flg_bit++;  
    }

    
    
    bit_all = (NJ_UINT16)(TERM_BIT + flg_bit + BIT_HINDO_LEN(hdl) + 
                          BIT_FHINSI(hdl) + BIT_BHINSI(hdl));
    pos = (NJ_UINT16)(bit_all >> 3);
    data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));

    
    j = (NJ_UINT16)(bit_all & 0x0007);

    cand_bit = BIT_CANDIDATE_LEN(hdl);
    candidate_size = GET_BITFIELD_16(data, j, cand_bit);

    
    bit_all += cand_bit;

    
    if (APPEND_YOMI_FLG(hdl) && STEM_TERMINETER(stem_data)) {
        
        
        pos = (NJ_UINT16)(bit_all >> 3);
        data = (NJ_UINT16)(NJ_INT16_READ(stem_data + pos));

        
        j = (NJ_UINT16)(bit_all & 0x0007);
        yomi_bit = BIT_YOMI_LEN(hdl);
        
        stem_set->yomi_size = GET_BITFIELD_16(data, j, yomi_bit);
        bit_all += yomi_bit;
    } else {
        stem_set->yomi_size = 0;  
    }

    
    
    yomi_pos = GET_BIT_TO_BYTE(bit_all);
    yomi_pos += candidate_size;

    return yomi_pos;
}

static NJ_UINT16 get_stem_yomi_size(NJ_DIC_HANDLE hdl, NJ_UINT8 *ydata, NJ_UINT16 yomi_size)
{
    NJ_INT16 ytbl_cnt;
    NJ_INT8 ysize;
    NJ_UINT8 *ytbl_top;
    NJ_UINT8 *ytbl;
    NJ_UINT8 yidx;
    NJ_UINT16 i;
    NJ_UINT16 len;


    
    ytbl_cnt = YOMI_INDX_CNT(hdl);

    if (ytbl_cnt) {
    ysize = YOMI_INDX_SIZE(hdl); 
    ytbl_top = YOMI_INDX_TOP_ADDR(hdl);

        len = 0;
        for (i = 0; i < yomi_size; i++) {
            if (ysize == 2) {
                
                yidx = *(ydata+i);
                ytbl = ytbl_top + ((yidx-1) * ysize);
                len += UTL_CHAR(ytbl);
                
            } else {
                
                len++;
            }
        }
        
        return len * sizeof(NJ_CHAR);
    } else {
        
        return yomi_size;
    }
}

static NJ_UINT16 get_stem_yomi_string(NJ_DIC_HANDLE hdl, NJ_UINT8 *stem_data, NJ_CHAR *yomi, NJ_UINT16 yomi_pos, NJ_UINT16 yomi_size, NJ_UINT16 size)
{
    NJ_INT16 ytbl_cnt;
    NJ_INT8 ysize;
    NJ_UINT8 *ytbl_top, *ytbl;
    NJ_UINT8 *ydata;
    NJ_UINT8 yidx;
    NJ_UINT16 i;
    NJ_UINT16 copy_len;
    NJ_UINT16 char_len;


    
    ytbl_cnt = YOMI_INDX_CNT(hdl);
    ysize    = YOMI_INDX_SIZE(hdl);      
    ytbl_top = YOMI_INDX_TOP_ADDR(hdl);

    
    ydata = stem_data + yomi_pos;

    if (ytbl_cnt) {
        copy_len = 0;
        for (i = 0; i < yomi_size; i++) {
            
            yidx = *(ydata + i);
            ytbl = ytbl_top + ((yidx - 1) * ysize);
            if (ysize == 2) {
                
                char_len = UTL_CHAR(ytbl); 
                if (((copy_len + char_len + NJ_TERM_LEN) * sizeof(NJ_CHAR)) > size) {
                    return size;
                }
                while (char_len > 0) {
                    NJ_CHAR_COPY(yomi + copy_len, ytbl);
                    copy_len++;
                    char_len--;
                    ytbl += sizeof(NJ_CHAR);
                }
            } else {
                
                if (((copy_len + 1 + NJ_TERM_LEN) * sizeof(NJ_CHAR)) > size) {
                    return size; 
                }
                
                *(yomi + copy_len) = (NJ_CHAR)(*ytbl);
                copy_len++;
            }
        }
    } else {
        if ((yomi_size + (NJ_TERM_LEN * sizeof(NJ_CHAR))) > size) {
            return size; 
        }
        
        nj_memcpy((NJ_UINT8*)yomi, ydata, yomi_size);
        copy_len = yomi_size / sizeof(NJ_CHAR);
    }

    
    *(yomi + copy_len) = NJ_CHAR_NUL;

    
    return copy_len;
}

static NJ_INT16 search_node(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset)
{
    NJ_UINT8 *root, *now, *node, *node_mid;
    NJ_UINT8 index;
    NJ_UINT8 *byomi;
    NJ_UINT8 *wkc;
    NJ_UINT8 idx_no;
    NJ_INT16 idx;
    NJ_INT16 char_size;
    NJ_INT16 left, right, mid;  
    NJ_INT16 ytbl_cnt;
    NJ_UINT16 c, d;
    NJ_UINT8  c1 = 0, c2 = 0;
    NJ_UINT16 y;
    NJ_UINT16 ysize = (condition->ylen * sizeof(NJ_CHAR));
    NJ_UINT8 *ytbl_top;
    NJ_UINT16 idx_cnt;
    NJ_UINT16 nd_index;
    NJ_UINT16 bit_left, bit_data;
    NJ_UINT32 data_offset;
    NJ_UINT16 data;
    NJ_UINT16 pos, j, bit_all, bit_tmp, bit_idx;
    NJ_UINT32 data_l;
    NJ_UINT8 restart_flg = 0;
    NJ_UINT8 bottom_flg = 0;
    NJ_UINT8 *data_top, *stem_data;
    NJ_UINT16 hindo, hindo_max;
    NJ_UINT32 current,hindo_max_data, bottom, next;


    node = NULL;        

    byomi = (NJ_UINT8*)(condition->yomi); 

    
    root = NODE_AREA_TOP_ADDR(loctset->loct.handle);

    
    node_mid = root + NODE_AREA_MID_ADDR(loctset->loct.handle);
    now = node_mid;

    
    idx_no = 0;
    idx_cnt = 1;

    bit_left = BIT_NODE_AREA_LEFT_LEN(loctset->loct.handle);
    bit_data = BIT_NODE_AREA_DATA_LEN(loctset->loct.handle);

    ytbl_cnt = YOMI_INDX_CNT(loctset->loct.handle);
    y = YOMI_INDX_SIZE(loctset->loct.handle);    
    ytbl_top = YOMI_INDX_TOP_ADDR(loctset->loct.handle);
    
    data_top = STEM_AREA_TOP_ADDR(loctset->loct.handle);

    
    if ((condition->operation == NJ_CUR_OP_FORE) &&
        NJ_CHAR_STRLEN_IS_0(condition->yomi)) {

        ysize = 0;

        
        node = root;
    }

    
    while (ysize > 0) {
        if (ytbl_cnt != 0) {
            char_size = UTL_CHAR(byomi) * sizeof(NJ_CHAR);
            if (char_size > 2) {
                loctset->loct.status = NJ_ST_SEARCH_END_EXT;
                return 0;   
            }

            if (char_size == 2) {        
                if (y == 1) {
                    return 0;   
                }
                c1 = *byomi;
                c2 = *(byomi + 1);
                c = (NJ_UINT16)((c1 << 8) | c2);
            } else {                    
                
                c1 = *byomi;
                c2 = 0x00;
                c = (NJ_UINT16)(*byomi);
            }

            idx = -1;
            left = 0;                                   
            right = ytbl_cnt;   

            if (y == 2) {
                while (left <= right) {
                    mid = (left + right) >> 1;
                    wkc = ytbl_top + (mid << 1);

                    if (c1 == *wkc) {
                        if (c2 == *(wkc + 1)) {
                            idx = (NJ_UINT16)(mid + 1);
                            break;
                        }
                        if (c2 < *(wkc + 1)) {
                            right = mid - 1;
                        } else {
                            left = mid + 1;
                        }
                    } else if (c1 < *wkc) {
                        right = mid - 1;
                    } else {
                        left = mid + 1;
                    }
                }
            } else {
                while (left <= right) {
                    mid = (left + right) >> 1;
                    wkc = ytbl_top + (mid * y);
                    d = (NJ_UINT16)(*wkc);
                    if (c == d) {
                        idx = (NJ_UINT16)(mid + 1);
                        break;
                    }
                    if (c < d) {
                        right = mid - 1;
                    } else {
                        left = mid + 1;
                    }
                }
            }

            if (idx < 0) {
                loctset->loct.status = NJ_ST_SEARCH_END_EXT;
                return 0;       
            }
            index = (NJ_UINT8)idx;
        } else {
            index = *byomi;
            char_size = 1;       
        }

        byomi += char_size;       
        ysize -= char_size;

        while (now < data_top) {
            if (NODE_IDX_EXIST(now)) {
                bit_idx = 8;
                idx_cnt = NODE_IDX_CNT(now);
            } else {
                bit_idx = 4;
                idx_cnt = 1;
            }
            bit_all = bit_idx;

            
            if (NODE_LEFT_EXIST(now)) {
                bit_all += bit_left;
            }

            
            if (NODE_DATA_EXIST(now)) {
                bit_all += bit_data;
            }
            
            bit_tmp = bit_all;

            
            bit_all += (NJ_UINT16)(idx_no << 3);

            
            pos = (NJ_UINT16)(bit_all >> 3);
            
            data = (NJ_UINT16)(NJ_INT16_READ(now + pos));

            
            j = (NJ_UINT16)(bit_all & 0x0007);
            
            nd_index = GET_BITFIELD_16(data, j, INDEX_BIT);
            if (index == (NJ_UINT8)nd_index) {
                
                break;
            } else {
                if ((!NODE_TERM(now)) && (index > (NJ_UINT8)nd_index) && (idx_no == 0)) {
                    
                    now += GET_BIT_TO_BYTE(bit_tmp + (idx_cnt * 8));
                    if (now == node_mid) {
                        loctset->loct.status = NJ_ST_SEARCH_END_EXT;
                        return 0;
                    }
                    continue;   
                } else {
                    if ((now == node_mid) && (restart_flg == 0) &&
                        (index < (NJ_UINT8)nd_index) && (idx_no == 0) &&
                        (root != node_mid)) {
                        now = root;
                        idx_no = 0;
                        restart_flg = 1;
                        continue;       
                    }
                    loctset->loct.status = NJ_ST_SEARCH_END_EXT;
                    return 0;
                }
            }
        }

        if ( (idx_cnt > (NJ_UINT16)(idx_no + 1))) {
            if (ysize == 0) {
                if (condition->operation == NJ_CUR_OP_FORE) {
                    
                    node = now;
                    break;
                }
                loctset->loct.status = NJ_ST_SEARCH_END;
                return 0;   
            }
            idx_no++;
            continue;
        }
        node = now;     
        idx_no = 0;     

        if (ysize == 0) {
            break;
        } else {
            if (!(NODE_LEFT_EXIST(now))) {
                loctset->loct.status = NJ_ST_SEARCH_END_EXT;
                return 0;       
            }
        }

        if (NODE_IDX_EXIST(now)) {
            bit_idx = 8;
        } else {
            bit_idx = 4;
        }
        pos = (NJ_UINT16)(bit_idx >> 3);
        data_l = (NJ_UINT32)(NJ_INT32_READ(now + pos));

        
        j = (NJ_UINT16)(bit_idx & 0x0007);

        now += GET_BITFIELD_32(data_l, j, bit_left);
    }

    
    now = node; 

    
    if ((node == NULL) || !(NODE_DATA_EXIST(node))) {

        if ((condition->operation == NJ_CUR_OP_FORE) && 
            (node != NULL)) {
            while (!NODE_DATA_EXIST(node)) {
                if (!(NODE_LEFT_EXIST(node))) {
                    loctset->loct.status = NJ_ST_SEARCH_END;
                    return 0;   
                }
                
                if (NODE_IDX_EXIST(node)) {
                    bit_idx = 8;
                } else {
                    bit_idx = 4;
                }
                pos = (NJ_UINT16)(bit_idx >> 3);
                data_l = (NJ_UINT32)(NJ_INT32_READ(node + pos));
                
                
                j = (NJ_UINT16)(bit_idx & 0x0007);
                node += GET_BITFIELD_32(data_l, j, bit_left);
            }
        } else {
            loctset->loct.status = NJ_ST_SEARCH_END;
            return 0;   
        }
    }

    if (NODE_IDX_EXIST(node)) {
        bit_idx = 8;
    } else {
        bit_idx = 4;
    }
    
    
    if (NODE_LEFT_EXIST(node)) {
        bit_all = bit_idx + bit_left;
    } else {
        bit_all = bit_idx;
    }
    
    pos = (NJ_UINT16)(bit_all >> 3);
    data_l = (NJ_UINT32)(NJ_INT32_READ(node + pos));
    
    
    j = (NJ_UINT16)(bit_all & 0x0007);
    data_offset = GET_BITFIELD_32(data_l, j, bit_data);

    loctset->loct.top = data_offset;
    loctset->loct.current = 0;

    if (condition->operation == NJ_CUR_OP_FORE) {
        
        bottom = loctset->loct.top;

        if (NJ_CHAR_STRLEN_IS_0(condition->yomi)) {
            node = node_mid;

        } else {
            
            node = now;
            if (NODE_LEFT_EXIST(node)) {
                if (NODE_IDX_EXIST(node)) {
                    bit_all = 8;
                } else {
                    bit_all = 4;
                }
                
                pos = (NJ_UINT16)(bit_all >> 3);
                data_l = (NJ_UINT32)(NJ_INT32_READ(node + pos));
                
                
                j = (NJ_UINT16)(bit_all & 0x0007);
                node += GET_BITFIELD_32(data_l, j, bit_left);

            } else {
                bottom_flg = 1;
            }
        }

        if (!bottom_flg) {
            while (node < data_top) {
                
                if (!NODE_TERM(node)) {
                    
                    if (NODE_IDX_EXIST(node)) {
                        bit_all = 8; 
                        idx_cnt = NODE_IDX_CNT(node); 
                    } else {
                        bit_all = 4;
                        idx_cnt = 1;
                    }
                    
                    
                    if (NODE_LEFT_EXIST(node)) {
                        bit_all += bit_left;
                    }
                    
                    
                    if (NODE_DATA_EXIST(node)) {
                        bit_all += bit_data;
                    }
                    
                    
                    node += GET_BIT_TO_BYTE(bit_all + (idx_cnt * 8));
                } else {
                    
                    if (!NODE_LEFT_EXIST(node)) {
                        
                        if (NODE_DATA_EXIST(node)) {
                            
                            if (NODE_IDX_EXIST(node)) {
                                bit_all = 8;
                            } else {
                                bit_all = 4;
                            }

                            pos = (NJ_UINT16)(bit_all >> 3);
                            data_l = (NJ_UINT32)(NJ_INT32_READ(node + pos));
                            
                            
                            j = (NJ_UINT16)(bit_all & 0x0007);
                            data_offset = GET_BITFIELD_32(data_l, j, bit_data);
                            
                            bottom = data_offset;
                            break;
                        } else {
                            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_DIC_BROKEN); 
                        }

                    } else {
                        
                        if (NODE_IDX_EXIST(node)) {
                            bit_all = 8;
                        } else {
                            bit_all = 4;
                        }
                        
                        pos = (NJ_UINT16)(bit_all >> 3);
                        data_l = (NJ_UINT32)(NJ_INT32_READ(node + pos));

                        
                        j = (NJ_UINT16)(bit_all & 0x0007);
                        
                        
                        node += GET_BITFIELD_32(data_l, j, bit_left);
                    }
                }
            }
        }

        stem_data = data_top + bottom;
        
        while (!(STEM_TERMINETER(stem_data))) {
            next = get_stem_next(loctset->loct.handle, stem_data);
            stem_data += next;
        }
        loctset->loct.bottom = (NJ_UINT32)(stem_data - data_top);
        
        
        stem_data = data_top + loctset->loct.top;
        
        hindo = (NJ_UINT16) *((NJ_UINT8*)(HINDO_NO_TOP_ADDR(loctset->loct.handle)
                                          + get_stem_hindo(loctset->loct.handle, stem_data)));
        
        hindo_max = hindo;
        hindo_max_data = 0;

        if (condition->mode == NJ_CUR_MODE_FREQ) {

            
            j = get_stem_next(loctset->loct.handle, stem_data);
            current = j;
            stem_data += j;

            while (stem_data <= (data_top + loctset->loct.bottom)) {
                
                
                hindo = (NJ_UINT16) *((NJ_UINT8*)(HINDO_NO_TOP_ADDR(loctset->loct.handle)
                                                  + get_stem_hindo(loctset->loct.handle, stem_data)));
                
                
                if (hindo > hindo_max) {
                    hindo_max = hindo;
                    hindo_max_data = current; 
                }

                
                j = get_stem_next(loctset->loct.handle, stem_data);
                current += j;
                stem_data += j;
            }
        }
        loctset->cache_freq = CALCULATE_HINDO(hindo_max, loctset->dic_freq.base, 
                                              loctset->dic_freq.high, COMP_DIC_FREQ_DIV);
        loctset->loct.current = hindo_max_data;

    }

    return 1;   
}

static NJ_INT16 bdic_search_data(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset)
{
    NJ_UINT8 *data, *data_end;
    NJ_INT16 i, current = 0;
    NJ_UINT16 hindo;


    data = STEM_AREA_TOP_ADDR(loctset->loct.handle);
    data += loctset->loct.top + loctset->loct.current;

    if (GET_LOCATION_STATUS(loctset->loct.status) != NJ_ST_SEARCH_NO_INIT) {

        if (STEM_TERMINETER(data)) {
            
            loctset->loct.status = NJ_ST_SEARCH_END;
            return 0;
        }

        
        i = get_stem_next(loctset->loct.handle, data);

        data += i;
        current += i;
    }

    if (NJ_GET_DIC_FMT(loctset->loct.handle) == NJ_DIC_FMT_KANAKAN) {
        data_end = loctset->loct.handle
            + NJ_DIC_COMMON_HEADER_SIZE 
            + NJ_INT32_READ(loctset->loct.handle + NJ_DIC_POS_DATA_SIZE)
            + NJ_INT32_READ(loctset->loct.handle + NJ_DIC_POS_EXT_SIZE) 
            - NJ_DIC_ID_LEN;
    } else {
        data_end = CAND_IDX_AREA_TOP_ADDR(loctset->loct.handle);
    }

    if (data < data_end) {
        
        loctset->loct.status = NJ_ST_SEARCH_READY;
        loctset->loct.current += current;
        hindo = (NJ_UINT16) *((NJ_UINT8*)(HINDO_NO_TOP_ADDR(loctset->loct.handle) + 
                                          get_stem_hindo(loctset->loct.handle, data)));
        loctset->cache_freq = CALCULATE_HINDO(hindo, loctset->dic_freq.base, 
                                              loctset->dic_freq.high, COMP_DIC_FREQ_DIV);
        return 1;
    }
    
    loctset->loct.status = NJ_ST_SEARCH_END; 
    return 0; 
}

static NJ_INT16 bdic_search_fore_data(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset)
{
    NJ_UINT8 *data, *data_top, *bottom, *data_end;
    NJ_INT16 i = 0;
    NJ_INT16 hindo = 0;
    NJ_INT16 hindo_max = -1;
    NJ_UINT8 no_hit = 0;
    NJ_UINT32 current = loctset->loct.current;
    NJ_UINT8 *current_org;
    NJ_UINT32 hindo_data = 0;


    
    if (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_NO_INIT) {
        loctset->loct.status = NJ_ST_SEARCH_READY;
        loctset->loct.current_info = CURRENT_INFO_SET;
        return 1;
    }

    
    data_top = STEM_AREA_TOP_ADDR(loctset->loct.handle);

    
    data = data_top + loctset->loct.top + loctset->loct.current;

    
    current_org = data;

    
    bottom = data_top + loctset->loct.bottom;

    if (NJ_GET_DIC_FMT(loctset->loct.handle) == NJ_DIC_FMT_KANAKAN) {
        data_end = loctset->loct.handle
            + NJ_DIC_COMMON_HEADER_SIZE 
            + NJ_INT32_READ(loctset->loct.handle + NJ_DIC_POS_DATA_SIZE)
            + NJ_INT32_READ(loctset->loct.handle + NJ_DIC_POS_EXT_SIZE) 
            - NJ_DIC_ID_LEN;
    } else {
        data_end = CAND_IDX_AREA_TOP_ADDR(loctset->loct.handle);
    }

    if (condition->mode == NJ_CUR_MODE_FREQ) {
        

        
        while (data < data_end) {
            
            i = get_stem_next(loctset->loct.handle, data);
            data += i;
            current += i;
            
            
            if (data > bottom) {
                if (loctset->cache_freq == 0) {
                    
                    loctset->loct.status = NJ_ST_SEARCH_END;
                    return 0;
                } else if (no_hit == 1) {
                    
                    loctset->loct.status = NJ_ST_SEARCH_END;
                    return 0;
                }
                
                loctset->cache_freq -= 1;
                
                
                data = data_top + loctset->loct.top;
                current = 0;

                no_hit = 1;
            }

            
            if ((hindo_max != -1) && (data == current_org)) {
                loctset->loct.status = NJ_ST_SEARCH_READY;
                loctset->loct.current_info = CURRENT_INFO_SET;
                loctset->loct.current = hindo_data;
                loctset->cache_freq = hindo_max;
                return 1;
            }
        
            
            hindo = (NJ_INT16) *((NJ_UINT8*)(HINDO_NO_TOP_ADDR(loctset->loct.handle) + get_stem_hindo(loctset->loct.handle, data)));
            
            hindo = CALCULATE_HINDO(hindo, loctset->dic_freq.base, 
                                    loctset->dic_freq.high, COMP_DIC_FREQ_DIV);

            
            if (hindo == loctset->cache_freq) {
                loctset->loct.status = NJ_ST_SEARCH_READY;
                loctset->loct.current_info = CURRENT_INFO_SET;
                loctset->loct.current = current;
                return 1;
            }

            if (hindo < loctset->cache_freq) {
                if (((hindo == hindo_max) && (current < hindo_data)) || 
                    (hindo > hindo_max)) {
                    hindo_max = hindo;
                    hindo_data = current;
                }
            }
        }
    } else {
        

        
        i = get_stem_next(loctset->loct.handle, data);
        data += i;
        current += i;
        
        
        if (data > bottom) {
            
            loctset->loct.status = NJ_ST_SEARCH_END;
            return 0;
        }

        
        hindo = (NJ_INT16) *((NJ_UINT8*)(HINDO_NO_TOP_ADDR(loctset->loct.handle)
                                         + get_stem_hindo(loctset->loct.handle, data)));
        loctset->cache_freq = CALCULATE_HINDO(hindo, loctset->dic_freq.base, 
                                              loctset->dic_freq.high, COMP_DIC_FREQ_DIV);
        loctset->loct.status = NJ_ST_SEARCH_READY;
        loctset->loct.current_info = CURRENT_INFO_SET;
        loctset->loct.current = current;
        return 1;
    }
    
    loctset->loct.status = NJ_ST_SEARCH_END; 
    return 0; 
}

NJ_INT16 njd_b_search_word(NJ_SEARCH_CONDITION *con, NJ_SEARCH_LOCATION_SET *loctset)
{
    NJ_INT16 ret;
    NJ_DIC_INFO *pdicinfo;
    NJ_UINT16 hIdx;



    
    switch (con->operation) {
    case NJ_CUR_OP_COMP:
        
        if (con->mode != NJ_CUR_MODE_FREQ) {
            
            loctset->loct.status = NJ_ST_SEARCH_END_EXT;
            return 0;
        }
        break;
    case NJ_CUR_OP_FORE:
        
        if (APPEND_YOMI_FLG(loctset->loct.handle) == 0) {
            loctset->loct.status = NJ_ST_SEARCH_END_EXT;
            return 0;
        }

        if ((NJ_GET_DIC_TYPE_EX(loctset->loct.type, loctset->loct.handle) != NJ_DIC_TYPE_CUSTOM_COMPRESS)
            && NJ_CHAR_STRLEN_IS_0(con->yomi)) {
            loctset->loct.status = NJ_ST_SEARCH_END_EXT;
            return 0;       
        }
        break;
    default:
        
        loctset->loct.status = NJ_ST_SEARCH_END_EXT;
        return 0;
    }

    if (con->ylen > NJ_GET_MAX_YLEN(loctset->loct.handle)) {
        loctset->loct.status = NJ_ST_SEARCH_END_EXT;
        return 0;
    }

    if (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_NO_INIT) {
        

        switch (con->operation) {
        case NJ_CUR_OP_COMP:
            ret = search_node(con, loctset);
            if (ret < 1) {
                return ret;
            }
            ret = bdic_search_data(con, loctset);
            if (ret < 1) {
                
                loctset->loct.status = NJ_ST_SEARCH_END;
            }
            break;
        case NJ_CUR_OP_FORE:
            pdicinfo = con->ds->dic;
            for (hIdx = 0; (hIdx < NJ_MAX_DIC) && (pdicinfo->handle != loctset->loct.handle); hIdx++) {
                pdicinfo++;
            }

            if (hIdx == NJ_MAX_DIC) {
                
                loctset->loct.status = NJ_ST_SEARCH_END; 
                return 0; 
            }

            if ((con->ds->dic[hIdx].srhCache == NULL) || (con->ylen == 0) ||
                !(con->ds->mode & 0x0001)) {
                ret = search_node(con, loctset);
                if (ret < 1) {
                    return ret;
                }
                ret = bdic_search_fore_data(con, loctset);
            } else {
                ret = search_node2(con, loctset, hIdx);
                if (ret == NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_NOT_ENOUGH)) {
                    
                    NJ_SET_CACHEOVER_TO_SCACHE(con->ds->dic[hIdx].srhCache);
                    ret = search_node2(con, loctset, hIdx);
                }
                if (ret < 1) {
                    return ret;
                }
                ret = bdic_search_fore_data2(con, loctset, hIdx);
            }
            if (ret < 1) {
                
                loctset->loct.status = NJ_ST_SEARCH_END; 
            }
            break;
        default:
            loctset->loct.status = NJ_ST_SEARCH_END_EXT; 
            return 0; 
        }
    } else if (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_READY) {

        switch (con->operation) {
        case NJ_CUR_OP_COMP:
            ret = bdic_search_data(con, loctset);
            if (ret < 1) {
                
                loctset->loct.status = NJ_ST_SEARCH_END;
            }
            break;
        case NJ_CUR_OP_FORE:
            pdicinfo = con->ds->dic;
            for (hIdx = 0; (hIdx < NJ_MAX_DIC) && (pdicinfo->handle != loctset->loct.handle); hIdx++) {
                pdicinfo++;
            }

            if (hIdx == NJ_MAX_DIC) {
                
                loctset->loct.status = NJ_ST_SEARCH_END; 
                return 0; 
            }

            if ((con->ds->dic[hIdx].srhCache == NULL) || (con->ylen == 0) ||
                !(con->ds->mode & 0x0001)) {
                ret = bdic_search_fore_data(con, loctset);
            } else {
                ret = bdic_search_fore_data2(con, loctset, hIdx);
            }
            if (ret < 1) {
                
                loctset->loct.status = NJ_ST_SEARCH_END;
            }
            break;
        default:
            loctset->loct.status = NJ_ST_SEARCH_END; 
            return 0; 
        }
    } else {
        loctset->loct.status = NJ_ST_SEARCH_END; 
        return 0; 
    }
    return ret;
}

NJ_INT16 njd_b_get_word(NJ_SEARCH_LOCATION_SET *loctset, NJ_WORD *word)
{
    NJ_UINT8 *data;
    STEM_DATA_SET stem_set;
    NJ_UINT8 check;



    
    if (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_END) {
        return 0; 
    }

	if (GET_LOCATION_OPERATION(loctset->loct.status) == NJ_CUR_OP_FORE) {
        data = STEM_AREA_TOP_ADDR(loctset->loct.handle);
        data += loctset->loct.top + loctset->loct.current;

        
        check = 0;
    } else {
        

        
        data = STEM_AREA_TOP_ADDR(loctset->loct.handle);
        data += loctset->loct.top + loctset->loct.current;

        
        check = 2;
    }

    
    get_stem_word(loctset->loct.handle, data, &stem_set, check);

    if (GET_LOCATION_OPERATION(loctset->loct.status) == NJ_CUR_OP_FORE) {
        word->stem.info1 = (NJ_UINT16)(stem_set.yomi_size / sizeof(NJ_CHAR));
    }
    word->stem.info1 = WORD_LEN(word->stem.info1);              
    word->stem.info1 |= (NJ_UINT16)(stem_set.fhinsi_jitu << 7); 

    if (check != 1) {
        if (stem_set.candidate_size == 0) {
            
            if (GET_LOCATION_OPERATION(loctset->loct.status) == NJ_CUR_OP_FORE) {
                word->stem.info2 = (NJ_UINT16)(stem_set.yomi_size / sizeof(NJ_CHAR));
            } else {
                
                word->stem.info2 = (NJ_UINT16)NJ_GET_YLEN_FROM_STEM(word);
            }
        } else {
            
            word->stem.info2 = (NJ_UINT16)(stem_set.candidate_size / sizeof(NJ_CHAR));
        }
    } else {
        
        word->stem.info2 = (NJ_UINT16)NJ_GET_YLEN_FROM_STEM(word);
    }

    word->stem.info2 = WORD_LEN(word->stem.info2);                      
    word->stem.info2 |= (NJ_UINT16)(stem_set.bhinsi_jitu << 7);         
    word->stem.hindo = CALCULATE_HINDO(stem_set.hindo_jitu, loctset->dic_freq.base, 
                                       loctset->dic_freq.high, COMP_DIC_FREQ_DIV); 
    word->stem.loc = loctset->loct;                                     

    return 1;
}

NJ_INT16 njd_b_get_candidate(NJ_WORD *word, NJ_CHAR *candidate, NJ_UINT16 size)
{
    NJ_SEARCH_LOCATION *loc;
    NJ_CHAR  *wkc, *cand;
    NJ_UINT8  *wkd;
    NJ_UINT8 *data;
    NJ_UINT8 *data_org;
    NJ_UINT16 len, j;
    STEM_DATA_SET stem_set;
    NJ_INT16  next;
    NJ_UINT16 yomi_pos;
    NJ_CHAR   ybuf[NJ_MAX_LEN + NJ_TERM_LEN];



    
    if ((GET_LOCATION_OPERATION(word->stem.loc.status) == NJ_CUR_OP_COMP) || 
        (GET_LOCATION_OPERATION(word->stem.loc.status) == NJ_CUR_OP_FORE)) {
        
        
        loc = &word->stem.loc;
        data = STEM_AREA_TOP_ADDR(loc->handle);
        data += loc->top + loc->current;
        
        
        get_stem_cand_data(loc->handle, data, &stem_set);
        len = stem_set.candidate_size / sizeof(NJ_CHAR);

    } else {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_CANDIDATE, NJ_ERR_INVALID_RESULT); 
    }

    if (len == 0) {     
        data_org = data;

        if (GET_LOCATION_OPERATION(word->stem.loc.status) == NJ_CUR_OP_COMP) {
            
            len = WORD_LEN(word->stem.info1);   
            if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
                return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_CANDIDATE, NJ_ERR_BUFFER_NOT_ENOUGH);
            }
            wkc = word->yomi;
        } else {


            
            while (!(STEM_TERMINETER(data))) {
                next = get_stem_next(loc->handle, data);
                data += next;
            }

            
            yomi_pos = get_stem_yomi_data(loc->handle, data, &stem_set);

            
            wkc = ybuf;
            len = get_stem_yomi_string(loc->handle, data, wkc, 
                                       yomi_pos, stem_set.yomi_size,
                                       size);

            
            if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
                return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_CANDIDATE, NJ_ERR_BUFFER_NOT_ENOUGH);
            }
        }

        if (STEM_NO_CONV_FLG(data_org) == 0) {  
            cand = candidate;
            for (j = 0; j < len; j++) {
                *cand++ = *wkc++;
            }
            *cand = NJ_CHAR_NUL;
        } else {                                
            nje_convert_hira_to_kata(wkc, candidate, len);
        }

    } else {            
        
        if (size < (stem_set.candidate_size + (NJ_TERM_LEN*sizeof(NJ_CHAR)))) {
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_CANDIDATE, NJ_ERR_BUFFER_NOT_ENOUGH);
        }
        wkc = candidate;
        wkd = data + stem_set.stem_size;
        for (j = 0; j < len; j++) {
            NJ_CHAR_COPY(wkc, wkd);
            wkd += sizeof(NJ_CHAR);
            wkc++;
        }
        *wkc = NJ_CHAR_NUL;
    }

    return len;
}

NJ_INT16 njd_b_get_stroke(NJ_WORD *word, NJ_CHAR *stroke, NJ_UINT16 size)
{
    NJ_SEARCH_LOCATION *loc;
    NJ_UINT8 *data;
    NJ_INT16 len;
    NJ_INT16 next;
    NJ_UINT16 yomi_pos;
    STEM_DATA_SET stem_set;



    
    if (GET_LOCATION_OPERATION(word->stem.loc.status) == NJ_CUR_OP_FORE) {
        if (NJ_GET_YLEN_FROM_STEM(word) == 0) {
            
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_STROKE, NJ_ERR_INVALID_RESULT); 
        }

        
        loc = &word->stem.loc;
        
        data = STEM_AREA_TOP_ADDR(loc->handle);
        data += loc->top + loc->current;

    } else {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_STROKE, NJ_ERR_INVALID_RESULT); 
    }

    
    while (!(STEM_TERMINETER(data))) {
        next = get_stem_next(loc->handle, data);
        data += next;
    }
    
    
    yomi_pos = get_stem_yomi_data(loc->handle, data, &stem_set);
    if (stem_set.yomi_size == 0) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_STROKE, NJ_ERR_INVALID_RESULT); 
    }

    
    len = get_stem_yomi_string(loc->handle, data, stroke, 
                               yomi_pos, stem_set.yomi_size,
                               size);

    
    if (size < (NJ_UINT16)((len+NJ_TERM_LEN)*sizeof(NJ_CHAR))) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_GET_STROKE, NJ_ERR_BUFFER_NOT_ENOUGH);
    }

    *(stroke + len) = NJ_CHAR_NUL;
    return len;
}

static NJ_INT16 search_node2(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset, NJ_UINT16 hidx)
{
    NJ_UINT8 *root, *now, *node, *node_mid;
    NJ_CHAR  *yomi;

    NJ_INT16 ytbl_cnt;
    NJ_UINT16 y;
    NJ_UINT8 *ytbl_top;

    NJ_UINT16 bit_left, bit_data;
    NJ_UINT32 data_offset;
    NJ_UINT16 j;
    NJ_UINT8 *data_top, *stem_data;
    NJ_UINT16 hindo, hindo_max, hindo_tmp;
    NJ_UINT32 current, hindo_max_data, hindo_tmp_data;

    
    NJ_SEARCH_CACHE *psrhCache = condition->ds->dic[hidx].srhCache;
    NJ_CHAR  *key;
    NJ_UINT8 cmpflg;
    NJ_UINT8 endflg;
    NJ_UINT16 abPtrIdx;
    NJ_UINT16 key_len;
    NJ_UINT16 i, l, m;
    NJ_UINT16 abIdx;
    NJ_UINT16 abIdx_current;
    NJ_UINT16 abIdx_old;
    NJ_UINT16 addcnt = 0;
    NJ_CHAR   char_tmp[NJ_MAX_LEN + NJ_TERM_LEN];
    NJ_UINT16 tmp_len;
    NJ_UINT16 endIdx;
    NJ_INT16 ret;
    NJ_UINT8 *con_node;
    NJ_UINT16 yomi_clen;
    NJ_UINT8 aimai_flg = 0x01;
    NJ_CHAR  key_tmp[NJ_MAX_CHAR_LEN + NJ_TERM_LEN];
    NJ_CACHE_INFO tmpbuff;


    if (NJ_GET_CACHEOVER_FROM_SCACHE(psrhCache)) {
        aimai_flg = 0x00;
    }

    node = NULL;                

    yomi = condition->yomi;     

    
    root = NODE_AREA_TOP_ADDR(loctset->loct.handle);

    
    node_mid = root + NODE_AREA_MID_ADDR(loctset->loct.handle);
    now = node_mid;

    bit_left = BIT_NODE_AREA_LEFT_LEN(loctset->loct.handle);
    bit_data = BIT_NODE_AREA_DATA_LEN(loctset->loct.handle);

    ytbl_cnt = YOMI_INDX_CNT(loctset->loct.handle);
    y = YOMI_INDX_SIZE(loctset->loct.handle);    
    ytbl_top = YOMI_INDX_TOP_ADDR(loctset->loct.handle);

    data_top = STEM_AREA_TOP_ADDR(loctset->loct.handle);

    
    endflg = 0x00;
    cmpflg = 0x00;
    abPtrIdx = 0;
    key = condition->ds->keyword;

    
    yomi_clen = condition->yclen;
    for (i = 0; i < yomi_clen; i++) {
        
        abPtrIdx = i;

        
        if (!cmpflg) {  
            
            if (((abPtrIdx != 0) && (psrhCache->keyPtr[abPtrIdx] == 0))
                || (psrhCache->keyPtr[abPtrIdx + 1] == 0)) {
                
                cmpflg = 0x01;
            } else {
                
            }
        }

        addcnt = 0;
        if (cmpflg) {   
            
            if (abPtrIdx == 0) {
                
                abIdx = 0;
                
                nj_charncpy(key_tmp, yomi, 1);
                key_len = nj_strlen(key_tmp);

                node = NULL;
                now = node_mid;
                psrhCache->keyPtr[0] = 0;

                
                ret = search_yomi_node(condition->operation,
                                       node, now, 0, key_tmp, key_len,
                                       root, node_mid, bit_left, bit_data,
                                       data_top, ytbl_cnt, y, ytbl_top,
                                       &tmpbuff,
                                       &con_node, &data_offset);

                if (ret < 0) {
                    
                } else {
                    

                    
                    psrhCache->storebuff[abIdx] = tmpbuff;

                    
                    now = con_node;
                    
                    psrhCache->storebuff[abIdx].top = data_offset;

                    if (condition->operation == NJ_CUR_OP_FORE) {
                        ret = get_node_bottom(key_tmp, now, node_mid, data_top,
                                              bit_left, bit_data,
                                              psrhCache->storebuff[abIdx].top,
                                              loctset->loct.handle,
                                              &(psrhCache->storebuff[abIdx].bottom));
                        if (ret < 0) {
                            
                            return ret; 
                        }
                    }
                    addcnt++;
                    abIdx++;
                }

                if ((condition->charset != NULL) && aimai_flg) {
                    
                    for (l = 0; l < condition->charset->charset_count; l++) {
                        
                        if (nj_charncmp(key, condition->charset->from[l], 1) == 0) {
                            
                            nj_strcpy(char_tmp, condition->charset->to[l]);
                            tmp_len = nj_strlen(char_tmp);

                            node = NULL;
                            now = node_mid;

                            
                            ret = search_yomi_node(condition->operation,
                                                   node, now, 0, char_tmp, tmp_len,
                                                   root, node_mid, bit_left, bit_data,
                                                   data_top, ytbl_cnt, y, ytbl_top,
                                                   &tmpbuff,
                                                   &con_node, &data_offset);

                            if (ret < 0) {
                                
                            } else {
                                

                                
                                if (abIdx >= NJ_SEARCH_CACHE_SIZE) {
                                    psrhCache->keyPtr[abPtrIdx+1] = 0;
                                    return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_NOT_ENOUGH);
                                }

                                
                                psrhCache->storebuff[abIdx] = tmpbuff;

                                
                                now = con_node;
                                
                                psrhCache->storebuff[abIdx].top = data_offset;

                                if (condition->operation == NJ_CUR_OP_FORE) {
                                    ret = get_node_bottom(key_tmp, now,
                                                          node_mid, data_top,
                                                          bit_left, bit_data,
                                                          psrhCache->storebuff[abIdx].top,
                                                          loctset->loct.handle,
                                                          &(psrhCache->storebuff[abIdx].bottom));
                                    if (ret < 0) {
                                        
                                        return ret; 
                                    }
                                }
                                addcnt++;
                                abIdx++;
                            }
                        }
                    }
                }
                psrhCache->keyPtr[abPtrIdx + 1] = abIdx;
            } else {
                nj_charncpy(key_tmp, yomi, 1); 
                key_len = nj_strlen(key_tmp);

                if (psrhCache->keyPtr[abPtrIdx] == psrhCache->keyPtr[abPtrIdx - 1]) {
                    
                    psrhCache->keyPtr[abPtrIdx+1] = psrhCache->keyPtr[abPtrIdx-1];
                    endflg = 0x01;
                } else {
                    endIdx = psrhCache->keyPtr[abPtrIdx];
                    abIdx_old = psrhCache->keyPtr[abPtrIdx - 1];

                    if (NJ_GET_CACHEOVER_FROM_SCACHE(psrhCache)) {
                        abIdx = psrhCache->keyPtr[abPtrIdx - 1];
                        psrhCache->keyPtr[abPtrIdx] = abIdx;
                    } else {
                        abIdx = psrhCache->keyPtr[abPtrIdx];
                    }

                    if ((abIdx > NJ_SEARCH_CACHE_SIZE) || (abIdx_old >= NJ_SEARCH_CACHE_SIZE)
                        || (endIdx > NJ_SEARCH_CACHE_SIZE)) {
                        
                        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_BROKEN); 
                    }

                    for (m = abIdx_old; m < endIdx; m++) {
                        node = psrhCache->storebuff[m].node;
                        now = psrhCache->storebuff[m].now;

                        if ((node == now) && (psrhCache->storebuff[m].idx_no == 0)) {
                            continue;
                        }

                        
                        ret = search_yomi_node(condition->operation,
                                               node, now, psrhCache->storebuff[m].idx_no,
                                               key_tmp, key_len, root,
                                               node_mid, bit_left, bit_data,
                                               data_top, ytbl_cnt, y, ytbl_top,
                                               &tmpbuff,
                                               &con_node, &data_offset);

                        if (ret < 0) {
                            
                        } else {
                            

                            
                            if (abIdx >= NJ_SEARCH_CACHE_SIZE) {
                                psrhCache->keyPtr[abPtrIdx+1] = 0;
                                return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_NOT_ENOUGH);
                            }

                            
                            psrhCache->storebuff[abIdx] = tmpbuff;
                            
                            
                            now = con_node;

                            psrhCache->storebuff[abIdx].top = data_offset;

                            if (condition->operation == NJ_CUR_OP_FORE) {
                                ret = get_node_bottom(key_tmp, now, node_mid, data_top,
                                                      bit_left, bit_data,
                                                      psrhCache->storebuff[abIdx].top,
                                                      loctset->loct.handle,
                                                      &(psrhCache->storebuff[abIdx].bottom));

                                if (ret < 0) {
                                    
                                    return ret; 
                                }
                            }
                            addcnt++;
                            abIdx++;
                        }

                        if ((condition->charset != NULL) && aimai_flg) {
                            
                            for (l = 0; l < condition->charset->charset_count; l++) {
                                
                                if (nj_charncmp(key, condition->charset->from[l], 1) == 0) {
                                    
                                    nj_strcpy(char_tmp, condition->charset->to[l]);

                                    tmp_len = nj_strlen(char_tmp);

                                    node = psrhCache->storebuff[m].node;
                                    now = psrhCache->storebuff[m].now;

                                    
                                    ret = search_yomi_node(condition->operation,
                                                           node, now,
                                                           psrhCache->storebuff[m].idx_no,
                                                           char_tmp, tmp_len,
                                                           root, node_mid,
                                                           bit_left, bit_data, data_top,
                                                           ytbl_cnt, y, ytbl_top,
                                                           &tmpbuff,
                                                           &con_node, &data_offset);

                                    if (ret < 0) {
                                        
                                    } else {
                                        

                                        
                                        if (abIdx >= NJ_SEARCH_CACHE_SIZE) {
                                            psrhCache->keyPtr[abPtrIdx+1] = 0;
                                            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_NOT_ENOUGH);
                                        }

                                        
                                        psrhCache->storebuff[abIdx] = tmpbuff;
                                        
                                        
                                        now = con_node;

                                        psrhCache->storebuff[abIdx].top = data_offset;

                                        if (condition->operation == NJ_CUR_OP_FORE) {
                                            ret = get_node_bottom(key_tmp, now, node_mid,
                                                                  data_top, bit_left, bit_data,
                                                                  psrhCache->storebuff[abIdx].top,
                                                                  loctset->loct.handle,
                                                                  &(psrhCache->storebuff[abIdx].bottom));
                                            if (ret < 0) {
                                                
                                                return ret; 
                                            }
                                        }
                                        addcnt++;
                                        abIdx++;
                                    }
                                }
                            }
                        }
                    }
                    psrhCache->keyPtr[abPtrIdx + 1] = abIdx;
                }
            }
        }
        yomi += UTL_CHAR(yomi);
        key  += UTL_CHAR(key);
    }
    
    if ((addcnt == 0) && (psrhCache->keyPtr[yomi_clen - 1] == psrhCache->keyPtr[yomi_clen])) {
        endflg = 0x01;
    }

    if (endflg) {               
        loctset->loct.status = NJ_ST_SEARCH_END;
        return 0;
    }

    loctset->loct.current = 0;


    
    abPtrIdx = condition->yclen;

    
    abIdx = psrhCache->keyPtr[abPtrIdx];
    abIdx_old = psrhCache->keyPtr[abPtrIdx - 1];
    if ((abIdx > NJ_SEARCH_CACHE_SIZE) || (abIdx_old >= NJ_SEARCH_CACHE_SIZE)) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_BROKEN); 
    }

    if (condition->mode == NJ_CUR_MODE_FREQ) {
        hindo_max = 0;
        hindo_max_data = 0;
        abIdx_current = abIdx_old;

        
        stem_data = data_top + psrhCache->storebuff[abIdx_current].top;

        hindo = (NJ_UINT16) *((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle) +
                                           get_stem_hindo(loctset->loct.handle, stem_data)));

        hindo_tmp = 0;
        hindo_tmp_data = 0;
        current = 0;

        
        while (stem_data <= (data_top + psrhCache->storebuff[abIdx_current].bottom)) {
            
            if (hindo > hindo_tmp) {
                hindo_tmp = hindo;
                hindo_tmp_data = current;
            }

            
            j = get_stem_next(loctset->loct.handle, stem_data);
            current += j;
            stem_data += j;

            
            hindo = (NJ_UINT16) *((NJ_UINT8 *) (HINDO_NO_TOP_ADDR(loctset->loct.handle) +
                                                get_stem_hindo(loctset->loct.handle, stem_data)));

        }

        
        psrhCache->storebuff[abIdx_current].current = hindo_tmp_data;

        
        if (hindo_tmp > hindo_max) {
            hindo_max = hindo_tmp;
            hindo_max_data = hindo_tmp_data;
        }
    } else {
        
        abIdx_current = abIdx_old; 

        
        stem_data = data_top + psrhCache->storebuff[abIdx_current].top; 

        hindo = (NJ_UINT16) *((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle) 
                                           + get_stem_hindo(loctset->loct.handle, stem_data)));

        hindo_max = hindo; 
        hindo_max_data = 0; 
    }

    
    loctset->loct.top = psrhCache->storebuff[abIdx_current].top;
    loctset->loct.bottom = psrhCache->storebuff[abIdx_current].bottom;

    loctset->cache_freq = CALCULATE_HINDO(hindo_max, loctset->dic_freq.base,
                                          loctset->dic_freq.high, COMP_DIC_FREQ_DIV);
    loctset->loct.current = hindo_max_data;
    loctset->loct.current_cache = (NJ_UINT8)abIdx_current;

    
    psrhCache->viewCnt = 1;
    NJ_SET_AIMAI_TO_SCACHE(psrhCache);

    return 1; 
}

static NJ_INT16 search_yomi_node(NJ_UINT8 operation, NJ_UINT8 *node, NJ_UINT8 *now,
                                 NJ_UINT16 idx_no, NJ_CHAR  *yomi, NJ_UINT16 yomilen,
                                 NJ_UINT8 * root, NJ_UINT8 * node_mid,
                                 NJ_UINT16 bit_left, NJ_UINT16 bit_data,
                                 NJ_UINT8 * data_top,
                                 NJ_INT16 ytbl_cnt, NJ_UINT16 y, NJ_UINT8 * ytbl_top,
                                 NJ_CACHE_INFO * storebuf,
                                 NJ_UINT8 ** con_node,
                                 NJ_UINT32 * data_offset)
{

    NJ_UINT8 index;
    NJ_UINT8 *wkc;
    NJ_UINT8 *byomi;
    NJ_INT16 idx;
    NJ_INT16 char_size;
    NJ_INT16 left, right, mid; 
    NJ_UINT16 c, d;
    NJ_UINT8 c1 = 0, c2 = 0;
    NJ_UINT16 ysize = yomilen * sizeof(NJ_CHAR);
    NJ_UINT16 idx_cnt;
    NJ_UINT16 nd_index;
    NJ_UINT16 data;
    NJ_UINT16 pos, j, bit_all, bit_tmp, bit_idx;
    NJ_UINT32 data_l;
    NJ_UINT8 restart_flg = 0;


    *con_node = NULL;

    
    idx_cnt = 1;
    storebuf->idx_no = 0;

    byomi = (NJ_UINT8*)yomi;

    
    while (ysize > 0) {
        if (ytbl_cnt != 0) {
            char_size = UTL_CHAR(byomi) * sizeof(NJ_CHAR);
            if (char_size > 2) {
                return -1;  
            }


            
            if (char_size == 2) { 
                if (y == 1) {
                    return -1;  
                }
                c1 = *byomi;
                c2 = *(byomi + 1);
                c = (NJ_UINT16)((c1 << 8) | c2);
            } else {            
                
                c1 = *byomi;
                c2 = 0x00;
                c = (NJ_UINT16)(*byomi);
            }

            idx = -1;
            left = 0;           
            right = ytbl_cnt;   

            if (y == 2) {
                while (left <= right) {
                    mid = (left + right) >> 1;
                    wkc = ytbl_top + (mid << 1);

                    if (c1 == *wkc) {
                        if (c2 == *(wkc + 1)) {
                            idx = (NJ_UINT16) (mid + 1);
                            break;
                        }
                        if (c2 < *(wkc + 1)) {
                            right = mid - 1;
                        } else {
                            left = mid + 1;
                        }
                    } else if (c1 < *wkc) {
                        right = mid - 1;
                    } else {
                        left = mid + 1;
                    }
                }
            } else {
                while (left <= right) {
                    mid = (left + right) >> 1;
                    wkc = ytbl_top + (mid * y);
                    d = (NJ_UINT16) (*wkc);
                    if (c == d) {
                        idx = (NJ_UINT16) (mid + 1);
                        break;
                    }
                    if (c < d) {
                        right = mid - 1;
                    } else {
                        left = mid + 1;
                    }
                }
            }

            if (idx < 0) {
                return -1;      
            }
            index = (NJ_UINT8) idx;
        } else {
            index = *byomi;
            char_size = 1;       
        }

        byomi += char_size;       
        ysize -= char_size;

        while (now < data_top) {
            if (NODE_IDX_EXIST(now)) {
                bit_idx = 8;
                idx_cnt = NODE_IDX_CNT(now);
            } else {
                bit_idx = 4;
                idx_cnt = 1;
            }
            bit_all = bit_idx;

            
            if (NODE_LEFT_EXIST(now)) {
                bit_all += bit_left;
            }

            
            if (NODE_DATA_EXIST(now)) {
                bit_all += bit_data;
            }
            
            bit_tmp = bit_all;

            
            bit_all += (NJ_UINT16) (idx_no << 3);

            pos = (NJ_UINT16) (bit_all >> 3);
            
            data = (NJ_UINT16) (NJ_INT16_READ(now + pos));

            j = (NJ_UINT16) (bit_all & 0x0007);

            nd_index = GET_BITFIELD_16(data, j, INDEX_BIT);
            if (index == (NJ_UINT8) nd_index) {
                
                break;
            } else {
                if ((!NODE_TERM(now)) && (index > (NJ_UINT8) nd_index) && (idx_no == 0)) {
                    
                    now += GET_BIT_TO_BYTE(bit_tmp + (idx_cnt * 8));
                    if (now == node_mid) {

                        return -1;
                    }
                    continue;   
                } else {
                    if ((now == node_mid) && (restart_flg == 0)
                        && (index < (NJ_UINT8) nd_index) && (idx_no == 0)
                        && (root != node_mid)) {
                        now = root;
                        idx_no = 0;
                        restart_flg = 1;
                        continue;       
                    }
                    return -1;
                }
            }
        }

        if ( (idx_cnt > (NJ_UINT16) (idx_no + 1))) {
            if (ysize == 0) {
                if (operation == NJ_CUR_OP_FORE) {
                    
                    storebuf->node = now;
                    storebuf->now = now;
                    storebuf->idx_no = idx_no + 1;
                    node = now;
                    break;
                }
                return -2;      
            }
            idx_no++;
            continue;
        }
        
        node = now;             
        storebuf->node = now;
        idx_no = 0;             

        if (ysize == 0) {
            *con_node = now;
        } else {
            if (!(NODE_LEFT_EXIST(now))) {
                return -1; 
            }
        }

        if (NODE_LEFT_EXIST(now)) {
            if (NODE_IDX_EXIST(now)) {
                bit_idx = 8;
            } else {
                bit_idx = 4;
            }
            pos = (NJ_UINT16) (bit_idx >> 3);
            data_l = (NJ_UINT32) (NJ_INT32_READ(now + pos));

            
            j = (NJ_UINT16) (bit_idx & 0x0007);

            now += GET_BITFIELD_32(data_l, j, bit_left);
            storebuf->now = now;
        } else {
            storebuf->now = now;
        }
    }


    
    if (*con_node == NULL) {
        *con_node = now;
    }

    
    if ((node == NULL) || !(NODE_DATA_EXIST(node))) {

        if ((operation == NJ_CUR_OP_FORE) && (node != NULL)) {
            while (!NODE_DATA_EXIST(node)) {
                if (!(NODE_LEFT_EXIST(node))) {
                    
                    return -2;  
                }

                if (NODE_IDX_EXIST(node)) {
                    bit_idx = 8;
                } else {
                    bit_idx = 4;
                }
                pos = (NJ_UINT16) (bit_idx >> 3);
                data_l = (NJ_UINT32) (NJ_INT32_READ(node + pos));

                
                j = (NJ_UINT16) (bit_idx & 0x0007);
                node += GET_BITFIELD_32(data_l, j, bit_left);
            }
        } else {
            return -2;          
        }
    }

    if (NODE_IDX_EXIST(node)) {
        bit_idx = 8;
    } else {
        bit_idx = 4;
    }

    
    if (NODE_LEFT_EXIST(node)) {
        bit_all = bit_idx + bit_left;
    } else {
        bit_all = bit_idx;
    }

    pos = (NJ_UINT16) (bit_all >> 3);
    data_l = (NJ_UINT32) (NJ_INT32_READ(node + pos));

    
    j = (NJ_UINT16) (bit_all & 0x0007);
    *data_offset = GET_BITFIELD_32(data_l, j, bit_data);

    return 1;
}

static NJ_INT16 get_node_bottom(NJ_CHAR * yomi, NJ_UINT8 * now, NJ_UINT8 * node_mid,
                                NJ_UINT8 * data_top, NJ_UINT16 bit_left, NJ_UINT16 bit_data,
                                NJ_UINT32 top, NJ_DIC_HANDLE handle,
                                NJ_UINT32 * ret_bottom)
{
    NJ_UINT8 *node;
    NJ_UINT16 idx_cnt;
    NJ_UINT32 data_offset;
    NJ_UINT16 pos, j, bit_all;
    NJ_UINT32 data_l;
    NJ_UINT8 bottom_flg = 0;
    NJ_UINT8 *stem_data;
    NJ_UINT32 bottom, next;


    
    bottom = top;

    if (NJ_CHAR_STRLEN_IS_0(yomi)) {
        node = node_mid;

    } else {
        
        node = now;
        if (NODE_LEFT_EXIST(node)) {
            
            if (NODE_IDX_EXIST(node)) {
                bit_all = 8;
            } else {
                bit_all = 4;
            }

            pos = (NJ_UINT16) (bit_all >> 3);
            data_l = (NJ_UINT32) (NJ_INT32_READ(node + pos));

            
            j = (NJ_UINT16) (bit_all & 0x0007);
            node += GET_BITFIELD_32(data_l, j, bit_left);

        } else {
            bottom_flg = 1;
        }
    }

    
    if (!bottom_flg) {
        while (node < data_top) {
            
            if (!NODE_TERM(node)) {
                
                if (NODE_IDX_EXIST(node)) {
                    bit_all = 8;
                    idx_cnt = NODE_IDX_CNT(node);
                } else {
                    bit_all = 4;
                    idx_cnt = 1;
                }

                
                if (NODE_LEFT_EXIST(node)) {
                    bit_all += bit_left;
                }

                
                if (NODE_DATA_EXIST(node)) {
                    bit_all += bit_data;
                }

                
                node += GET_BIT_TO_BYTE(bit_all + (idx_cnt * 8));
            } else {
                
                if (!NODE_LEFT_EXIST(node)) {
                    
                    if (NODE_DATA_EXIST(node)) {
                        
                        if (NODE_IDX_EXIST(node)) {
                            bit_all = 8;
                        } else {
                            bit_all = 4;
                        }

                        pos = (NJ_UINT16) (bit_all >> 3);
                        data_l = (NJ_UINT32) (NJ_INT32_READ(node + pos));

                        
                        j = (NJ_UINT16) (bit_all & 0x0007);
                        data_offset = GET_BITFIELD_32(data_l, j, bit_data);
                        
                        bottom = data_offset;
                        break;
                    } else {
                        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_DIC_BROKEN); 
                    }

                } else {
                    
                    if (NODE_IDX_EXIST(node)) {
                        bit_all = 8;
                    } else {
                        bit_all = 4;
                    }

                    pos = (NJ_UINT16) (bit_all >> 3);
                    data_l = (NJ_UINT32) (NJ_INT32_READ(node + pos));

                    
                    j = (NJ_UINT16) (bit_all & 0x0007);

                    
                    node += GET_BITFIELD_32(data_l, j, bit_left);
                }
            }
        }
    }

    stem_data = data_top + bottom;

    while (!(STEM_TERMINETER(stem_data))) {
        next = get_stem_next(handle, stem_data);
        stem_data += next;
    }
    *ret_bottom = (NJ_UINT32) (stem_data - data_top);

    return 1;
}

static NJ_INT16 bdic_search_fore_data2(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset, NJ_UINT16 hidx)
{
    NJ_UINT8 *data, *data_top, *bottom, *data_end;
    NJ_INT16 i = 0;
    NJ_INT16 hindo = 0;
    NJ_UINT32 current = loctset->loct.current;


    NJ_SEARCH_CACHE *psrhCache = condition->ds->dic[hidx].srhCache;

    NJ_UINT16 top_abIdx;
    NJ_UINT16 bottom_abIdx;
    NJ_UINT16 count_abIdx;
    NJ_UINT16 current_abIdx;
    NJ_UINT16 old_abIdx;
    NJ_UINT8 freq_flag = 0;
    NJ_INT16 save_hindo = 0;
    NJ_UINT16 save_abIdx = 0;
    NJ_UINT16 abPtrIdx;
    NJ_UINT16 m;
    NJ_INT16 ret;
    NJ_INT16 loop_check;

    NJ_UINT16 abIdx;
    NJ_UINT16 abIdx_old;
    NJ_UINT16 hindo_max, hindo_tmp;
    NJ_UINT32 hindo_max_data, hindo_tmp_data;
    NJ_UINT16 abIdx_current;



    
    if (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_NO_INIT) {
        loctset->loct.status = NJ_ST_SEARCH_READY;
        loctset->loct.current_info = CURRENT_INFO_SET;
        return 1;
    }

    if (NJ_GET_AIMAI_FROM_SCACHE(psrhCache)) {
        NJ_UNSET_AIMAI_TO_SCACHE(psrhCache);
        
        data_top = STEM_AREA_TOP_ADDR(loctset->loct.handle);
        if (condition->operation == NJ_CUR_OP_FORE) {
            if (condition->ylen) {              
                
                abPtrIdx = condition->yclen;

                
                abIdx = psrhCache->keyPtr[abPtrIdx];
                abIdx_old = psrhCache->keyPtr[abPtrIdx - 1];
                if ((abIdx > NJ_SEARCH_CACHE_SIZE) || (abIdx_old >= NJ_SEARCH_CACHE_SIZE)) {
                    
                    return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_BROKEN); 
                }

                if (condition->mode == NJ_CUR_MODE_FREQ) {
                    hindo_max = 0;
                    hindo_max_data = 0;
                    abIdx_current = abIdx_old;

                    for (m = abIdx_old; m < abIdx; m++) {
                        
                        data = data_top + psrhCache->storebuff[m].top;

                        hindo = (NJ_UINT16) *((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle) +
                                                           get_stem_hindo(loctset->loct.handle, data)));

                        hindo_tmp = 0;
                        hindo_tmp_data = 0;
                        current = 0;

                        
                        while (data <= (data_top + psrhCache->storebuff[m].bottom)) {
                            
                            if (hindo > hindo_tmp) {
                                hindo_tmp = hindo;
                                hindo_tmp_data = current;
                            }

                            
                            i = get_stem_next(loctset->loct.handle, data);
                            current += i;
                            data += i;

                            
                            hindo = (NJ_UINT16) *((NJ_UINT8 *) (HINDO_NO_TOP_ADDR(loctset->loct.handle) +
                                                                get_stem_hindo(loctset->loct.handle, data)));

                        }

                        
                        psrhCache->storebuff[m].current = hindo_tmp_data;

                        
                        if (hindo_tmp > hindo_max) {
                            hindo_max = hindo_tmp;
                            hindo_max_data = hindo_tmp_data;
                            abIdx_current = m;
                        }
                    }
                } else {
                    
                    abIdx_current = abIdx_old; 

                    
                    data = data_top + psrhCache->storebuff[abIdx_current].top; 

                    hindo = (NJ_UINT16) *((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle) 
                                                       + get_stem_hindo(loctset->loct.handle, data)));

                    hindo_max = hindo; 
                    hindo_max_data = 0; 
                }

                
                loctset->loct.top = psrhCache->storebuff[abIdx_current].top;
                loctset->loct.bottom = psrhCache->storebuff[abIdx_current].bottom;

                loctset->cache_freq = CALCULATE_HINDO(hindo_max, loctset->dic_freq.base,
                                                      loctset->dic_freq.high, COMP_DIC_FREQ_DIV);
                loctset->loct.current = hindo_max_data;
                loctset->loct.current_cache = (NJ_UINT8)abIdx_current;

                
                psrhCache->viewCnt = 1;
            } else {
                
                data = data_top + loctset->loct.top; 

                hindo = (NJ_UINT16) *((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle) + 
                                                   get_stem_hindo(loctset->loct.handle, data)));

                hindo_max = hindo; 
                hindo_max_data = 0; 

                if (condition->mode == NJ_CUR_MODE_FREQ) { 

                    
                    i = get_stem_next(loctset->loct.handle, data); 
                    current = i; 
                    data += i; 

                    
                    while (data <= (data_top + loctset->loct.bottom)) { 

                        
                        hindo = (NJ_UINT16)*((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle) + 
                                                          get_stem_hindo(loctset->loct.handle, data)));

                        
                        if (hindo > hindo_max) { 
                            hindo_max = hindo; 
                            hindo_max_data = current; 
                        }

                        
                        i = get_stem_next(loctset->loct.handle, data); 
                        current += i; 
                        data += i; 
                    }
                }
                loctset->cache_freq = CALCULATE_HINDO(hindo_max, 
                                                      loctset->dic_freq.base,
                                                      loctset->dic_freq.high, COMP_DIC_FREQ_DIV);
                loctset->loct.current = hindo_max_data; 
            }
        }
        return 1;
    }

    
    data_top = STEM_AREA_TOP_ADDR(loctset->loct.handle);

    
    data = data_top + loctset->loct.top + loctset->loct.current;


    
    bottom = data_top + loctset->loct.bottom;

    if (NJ_GET_DIC_FMT(loctset->loct.handle) == NJ_DIC_FMT_KANAKAN) {
        data_end = loctset->loct.handle
            + NJ_DIC_COMMON_HEADER_SIZE
            + NJ_INT32_READ(loctset->loct.handle + NJ_DIC_POS_DATA_SIZE)
            + NJ_INT32_READ(loctset->loct.handle + NJ_DIC_POS_EXT_SIZE)
            - NJ_DIC_ID_LEN;
    } else {
        data_end = CAND_IDX_AREA_TOP_ADDR(loctset->loct.handle);
    }

    if (condition->mode == NJ_CUR_MODE_FREQ) {

        
        abPtrIdx = condition->yclen;

        
        bottom_abIdx = psrhCache->keyPtr[abPtrIdx];
        top_abIdx = psrhCache->keyPtr[abPtrIdx - 1];
        if ((bottom_abIdx > NJ_SEARCH_CACHE_SIZE) || (top_abIdx >= NJ_SEARCH_CACHE_SIZE)) {
            
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_B_SEARCH_WORD, NJ_ERR_CACHE_BROKEN); 
        }

        
        count_abIdx = bottom_abIdx - top_abIdx;
        if (!count_abIdx) {
            loctset->loct.status = NJ_ST_SEARCH_END; 
            return 0; 
        }

        old_abIdx = loctset->loct.current_cache;

        loop_check = 0;

        
        ret = bdic_get_next_data(data_top, data_end, loctset, psrhCache, old_abIdx);

        if (ret == loctset->cache_freq) {
            
            psrhCache->viewCnt++;
            if (psrhCache->viewCnt <= NJ_CACHE_VIEW_CNT) {
                
                loctset->loct.status = NJ_ST_SEARCH_READY;
                loctset->loct.current_info = CURRENT_INFO_SET;
                loctset->loct.current = psrhCache->storebuff[old_abIdx].current;
                loctset->loct.current_cache = (NJ_UINT8)old_abIdx;
                return 1;
            } else {
                
                freq_flag = 1;
                psrhCache->viewCnt = 0;
            }
        } else {
            if (ret == -1) {
                
                loop_check++;
            }
            save_hindo = ret;
            save_abIdx = old_abIdx;
        }

        
        current_abIdx = old_abIdx + 1;
        if (current_abIdx >= bottom_abIdx) {
            
            current_abIdx = top_abIdx;
        }

        while (loop_check != count_abIdx) {

            
            ret = bdic_get_word_freq(data_top, loctset, psrhCache, current_abIdx);

            if ((ret == loctset->cache_freq) &&
                (loctset->loct.top == psrhCache->storebuff[current_abIdx].top) &&
                (loctset->loct.current == psrhCache->storebuff[current_abIdx].current)) {
                ret = bdic_get_next_data(data_top, data_end, loctset, psrhCache, current_abIdx);
            }

            if (ret == loctset->cache_freq) {
                
                loctset->loct.status = NJ_ST_SEARCH_READY;
                loctset->loct.current_info = CURRENT_INFO_SET;
                loctset->loct.top = psrhCache->storebuff[current_abIdx].top;
                loctset->loct.bottom = psrhCache->storebuff[current_abIdx].bottom;
                loctset->loct.current = psrhCache->storebuff[current_abIdx].current;
                loctset->loct.current_cache = (NJ_UINT8)current_abIdx;
                psrhCache->viewCnt = 1;
                return 1;

            } else {
                if (ret == -1) {
                    
                    loop_check++;
                }
                if (save_hindo < ret) {
                    
                    save_hindo = ret;
                    save_abIdx = current_abIdx;
                }
            }

            
            current_abIdx++;
            if (current_abIdx >= bottom_abIdx) {
                
                current_abIdx = top_abIdx;
            }

            
            if (current_abIdx == old_abIdx) {
                if (freq_flag == 1) {
                    
                    loctset->loct.status = NJ_ST_SEARCH_READY;
                    loctset->loct.current_info = CURRENT_INFO_SET;
                    loctset->loct.top = psrhCache->storebuff[current_abIdx].top;
                    loctset->loct.bottom = psrhCache->storebuff[current_abIdx].bottom;
                    loctset->loct.current = psrhCache->storebuff[current_abIdx].current;
                    loctset->loct.current_cache = (NJ_UINT8)current_abIdx;
                    psrhCache->viewCnt = 1;
                    return 1;
                } else if (save_hindo != -1) {
                    
                    loctset->cache_freq = save_hindo;
                    loctset->loct.status = NJ_ST_SEARCH_READY;
                    loctset->loct.current_info = CURRENT_INFO_SET;
                    loctset->loct.top = psrhCache->storebuff[save_abIdx].top;
                    loctset->loct.bottom = psrhCache->storebuff[save_abIdx].bottom;
                    loctset->loct.current = psrhCache->storebuff[save_abIdx].current;
                    loctset->loct.current_cache = (NJ_UINT8)save_abIdx;
                    psrhCache->viewCnt = 1;
                    return 1;
                }
            }
        }
    } else {
        

        
        i = get_stem_next(loctset->loct.handle, data); 
        data += i; 
        current += i; 

        
        if (data > bottom) { 
            
            loctset->loct.status = NJ_ST_SEARCH_END; 
            return 0; 
        }

        
        hindo = (NJ_INT16)*((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle) 
                                         + get_stem_hindo(loctset->loct.handle, data)));
        loctset->cache_freq = CALCULATE_HINDO(hindo, loctset->dic_freq.base, 
                                              loctset->dic_freq.high, COMP_DIC_FREQ_DIV);
        loctset->loct.status = NJ_ST_SEARCH_READY; 
        loctset->loct.current_info = CURRENT_INFO_SET; 
        loctset->loct.current = current; 
        return 1; 
    }
    
    loctset->loct.status = NJ_ST_SEARCH_END;
    return 0;
}

static NJ_INT16 bdic_get_next_data(NJ_UINT8 *data_top, NJ_UINT8 *data_end,
                                   NJ_SEARCH_LOCATION_SET *loctset,
                                   NJ_SEARCH_CACHE *psrhCache,
                                   NJ_UINT16 abIdx)
{
    NJ_UINT8 *data, *bottom;
    NJ_INT16 i = 0;
    NJ_INT16 hindo = 0;
    NJ_INT16 hindo_max = -1;
    NJ_UINT8 no_hit = 0;
    NJ_UINT32 current = psrhCache->storebuff[abIdx].current;
    NJ_UINT8 *current_org;
    NJ_UINT32 hindo_data = 0;
    NJ_INT16 freq_org = loctset->cache_freq;


    if (psrhCache->storebuff[abIdx].current == LOC_CURRENT_NO_ENTRY) {
        return (-1); 
    }

    
    data = data_top + psrhCache->storebuff[abIdx].top + psrhCache->storebuff[abIdx].current;

    
    current_org = data;

    
    bottom = data_top + psrhCache->storebuff[abIdx].bottom;

    

    
    while (data < data_end) {
        
        i = get_stem_next(loctset->loct.handle, data);
        data += i;
        current += i;

        
        if (data > bottom) {
            if ((freq_org == 0) || (no_hit == 1)) {
                
                psrhCache->storebuff[abIdx].current = LOC_CURRENT_NO_ENTRY;
                return -1;
            }
            
            freq_org -= 1;

            
            data = data_top + psrhCache->storebuff[abIdx].top;
            current = 0;

            no_hit = 1;
        }

        
        if ((hindo_max != -1) && (data == current_org)) {
            psrhCache->storebuff[abIdx].current = hindo_data;
            return hindo_max;
        }

        
        hindo = (NJ_INT16)*((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle)
                                         + get_stem_hindo(loctset->loct.handle, data)));
        
        hindo = CALCULATE_HINDO(hindo, loctset->dic_freq.base, loctset->dic_freq.high, COMP_DIC_FREQ_DIV);

        
        if (hindo == freq_org) {
            psrhCache->storebuff[abIdx].current = current;
            return hindo;
        }

        if (hindo < freq_org) {
            if ((hindo > hindo_max) || ((hindo == hindo_max) && (current < hindo_data))) {
                hindo_max = hindo;
                hindo_data = current;
            }
        }
    }

    
    psrhCache->storebuff[abIdx].current = LOC_CURRENT_NO_ENTRY; 
    return -1; 
}

static NJ_INT16 bdic_get_word_freq(NJ_UINT8 * data_top, NJ_SEARCH_LOCATION_SET * loctset,
                                   NJ_SEARCH_CACHE * psrhCache, NJ_UINT16 abIdx)
{
    NJ_UINT8 *data;
    NJ_INT16 hindo = 0;


    if (psrhCache->storebuff[abIdx].current != LOC_CURRENT_NO_ENTRY) {
        
        data = data_top + psrhCache->storebuff[abIdx].top + psrhCache->storebuff[abIdx].current;

        
        hindo = (NJ_INT16)*((NJ_UINT8 *)(HINDO_NO_TOP_ADDR(loctset->loct.handle)
                                         + get_stem_hindo(loctset->loct.handle, data)));
        
        hindo = CALCULATE_HINDO(hindo, loctset->dic_freq.base, loctset->dic_freq.high, COMP_DIC_FREQ_DIV);

    } else {
        
        hindo = -1;
    }

    return hindo;
}
