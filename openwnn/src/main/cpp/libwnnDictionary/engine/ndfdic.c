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

#define DATA_SIZE (10)
#define DATA_OFFSET_FHINSI          (0) 
#define DATA_OFFSET_BHINSI          (1) 
#define DATA_OFFSET_HINDO           (2) 
#define DATA_OFFSET_CANDIDATE       (3) 
#define DATA_OFFSET_CANDIDATE_LEN   (5) 
#define DATA_OFFSET_YOMI            (6) 
#define DATA_OFFSET_YOMI_LEN        (9) 

#define YOMINASI_DIC_FREQ_DIV 63  

#define DATA_FHINSI(x)                                                  \
    ( (NJ_UINT16)(0x01FF &                                              \
                  (((NJ_UINT16)*((x)+DATA_OFFSET_FHINSI  ) << 1) |      \
                   (           *((x)+DATA_OFFSET_FHINSI+1) >> 7))) )
#define DATA_BHINSI(x)                                                  \
    ( (NJ_UINT16)(0x01FF &                                              \
                  (((NJ_UINT16)*((x)+DATA_OFFSET_BHINSI  ) << 2) |      \
                   (           *((x)+DATA_OFFSET_BHINSI+1) >> 6))) )
#define DATA_HINDO(x)                                                   \
    ((NJ_HINDO)(0x003F & ((NJ_UINT16)*((x)+DATA_OFFSET_HINDO))))
#define DATA_CANDIDATE(x)                                               \
    ((NJ_UINT32)(0x000FFFFF &                                           \
                 (((NJ_UINT32)*((x)+DATA_OFFSET_CANDIDATE)   << 12) |   \
                  ((NJ_UINT32)*((x)+DATA_OFFSET_CANDIDATE+1) <<  4) |   \
                  (           *((x)+DATA_OFFSET_CANDIDATE+2) >>  4))))
#define DATA_CANDIDATE_SIZE(x)                                          \
    ((NJ_UINT8)((*((x)+DATA_OFFSET_CANDIDATE_LEN)   << 4) |             \
                (*((x)+DATA_OFFSET_CANDIDATE_LEN+1) >> 4)))
#define DATA_YOMI(x) \
    ((NJ_UINT32)(0x000FFFFF &                                           \
                 (((NJ_UINT32)*((x)+DATA_OFFSET_YOMI)   << 16) |        \
                  ((NJ_UINT32)*((x)+DATA_OFFSET_YOMI+1) <<  8) |        \
                  (           *((x)+DATA_OFFSET_YOMI+2)      ))))
#define DATA_YOMI_SIZE(x)                       \
    ((NJ_UINT8)((*((x)+DATA_OFFSET_YOMI_LEN))))

#define YOMI_INDX_TOP_ADDR(h) ((NJ_UINT8*)((h)+NJ_INT32_READ((h)+0x1C)))
#define YOMI_INDX_CNT(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x20)))
#define YOMI_INDX_BYTE(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x22)))
#define STEM_AREA_TOP_ADDR(h) ((NJ_UINT8*)((h)+NJ_INT32_READ((h)+0x24)))
#define STRS_AREA_TOP_ADDR(h) ((NJ_UINT8*)((h)+NJ_INT32_READ((h)+0x28)))
#define YOMI_AREA_TOP_ADDR(h) ((NJ_UINT8*)((h)+NJ_INT32_READ((h)+0x2C)))

#define NO_CONV_FLG ((NJ_UINT32) 0x00080000L)

#define HINSI_OFFSET (7)

#define CURRENT_INFO_SET (NJ_UINT8)(0x10)

static NJ_UINT16 search_data(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset);
static NJ_UINT16 convert_to_yomi(NJ_DIC_HANDLE hdl, NJ_UINT8 *index, NJ_UINT16 len, NJ_CHAR *yomi, NJ_UINT16 size);
static NJ_UINT16 yomi_strcmp_forward(NJ_DIC_HANDLE hdl, NJ_UINT8 *data, NJ_CHAR *yomi);

static NJ_UINT16 search_data(NJ_SEARCH_CONDITION *condition, NJ_SEARCH_LOCATION_SET *loctset)
{
    NJ_UINT32 offset;
    NJ_UINT8 *data;
    NJ_UINT16 i, j;
    NJ_UINT16 hindo;
    NJ_UINT8 hit_flg;
    NJ_UINT8 *tmp_hinsi = NULL;


    offset = loctset->loct.current;
    data = STEM_AREA_TOP_ADDR(loctset->loct.handle) + offset;

    if (GET_LOCATION_STATUS(loctset->loct.status) != NJ_ST_SEARCH_NO_INIT) {
        data += DATA_SIZE;
        offset += DATA_SIZE;

        
        if (data >= STRS_AREA_TOP_ADDR(loctset->loct.handle)) {
            
            loctset->loct.status = NJ_ST_SEARCH_END;
            return 0;
        }
    }

    
    tmp_hinsi = condition->hinsi.fore;
    condition->hinsi.fore = condition->hinsi.yominasi_fore;
    
    i = (STRS_AREA_TOP_ADDR(loctset->loct.handle) - data) / DATA_SIZE;
    for (j = 0; j < i; j++) {
        
        if (njd_connect_test(condition, DATA_FHINSI(data), DATA_BHINSI(data))) {
            
            hit_flg = 0;

            if (condition->operation == NJ_CUR_OP_LINK) {
                
                hit_flg = 1;
            } else {
                

                
                if (yomi_strcmp_forward(loctset->loct.handle, data, condition->yomi)) {
                    
                    hit_flg = 1;
                }
            }

            if (hit_flg) {
                
                loctset->loct.current_info = CURRENT_INFO_SET;
                loctset->loct.current = offset;
                loctset->loct.status = NJ_ST_SEARCH_READY;
                hindo = DATA_HINDO(STEM_AREA_TOP_ADDR(loctset->loct.handle) + loctset->loct.current);
                loctset->cache_freq = CALCULATE_HINDO(hindo, loctset->dic_freq.base, 
                                                      loctset->dic_freq.high, YOMINASI_DIC_FREQ_DIV);

                
                condition->hinsi.fore = tmp_hinsi;
                return 1;
            }
        }
        
        data += DATA_SIZE;
        offset += DATA_SIZE;
    }
    
    loctset->loct.status = NJ_ST_SEARCH_END;
    
    condition->hinsi.fore = tmp_hinsi;
    return 0;
}

static NJ_UINT16 convert_to_yomi(NJ_DIC_HANDLE hdl, NJ_UINT8 *index, NJ_UINT16 len, NJ_CHAR *yomi, NJ_UINT16 size)
{
    NJ_UINT8  *wkc;
    NJ_CHAR   *wky;
    NJ_UINT16 i, idx, yib, ret;
    NJ_UINT16 j, char_len;


    
    wkc = YOMI_INDX_TOP_ADDR(hdl);

    
    yib = YOMI_INDX_BYTE(hdl);

    
    if (NJ_CHAR_ILLEGAL_DIC_YINDEX(yib)) {
        
        return 0;
    }

    
    ret = 0;
    wky = yomi;
    for (i = 0; i < len; i++) {
        idx = (NJ_UINT16)((*index - 1) * yib);  
        if (yib == 2) {         
            char_len = UTL_CHAR(wkc + idx);
            
            if (((ret + char_len + NJ_TERM_LEN) * sizeof(NJ_CHAR)) > size) {
                return (size / sizeof(NJ_CHAR));
            }
            for (j = 0; j < char_len; j++) {
                NJ_CHAR_COPY(wky, wkc + idx + j);
                wky++;
                ret++;
            }
        } else {                
            
            if (((ret + 1 + NJ_TERM_LEN) * sizeof(NJ_CHAR)) > size) { 
                return (size / sizeof(NJ_CHAR)); 
            }
            *wky++ = (NJ_CHAR)(*(wkc + idx));  
            ret++; 
        }
        index++;
    }
    *wky = NJ_CHAR_NUL;
    return ret;
}

static NJ_UINT16 yomi_strcmp_forward(NJ_DIC_HANDLE hdl, NJ_UINT8 *data, NJ_CHAR *yomi)
{
    NJ_UINT8 *area;
    NJ_CHAR  *stroke;
    NJ_CHAR   buf[NJ_MAX_LEN + NJ_TERM_LEN];
    NJ_UINT16 ylen, dic_ylen, j, size;


    
    size = sizeof(buf);
    stroke = buf;

    
    area = YOMI_AREA_TOP_ADDR(hdl) + DATA_YOMI(data);

    if (YOMI_INDX_CNT(hdl) == 0) {      
        
        dic_ylen = DATA_YOMI_SIZE(data) / sizeof(NJ_CHAR);

        
        if (size < ((dic_ylen + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
            return 0;
        }
        for (j = 0; j < dic_ylen; j++) {
            NJ_CHAR_COPY(stroke, area); 
            stroke++;
            area += sizeof(NJ_CHAR);
        }
        *stroke = NJ_CHAR_NUL;
    } else {                            
        
        dic_ylen = convert_to_yomi(hdl, area, DATA_YOMI_SIZE(data), stroke, size);

        
        if (size < ((dic_ylen + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
            return 0;
        }
    }

    
    ylen = nj_strlen(yomi);

    
    if (dic_ylen < ylen) {
        
        return 0;
    }

    
    if (nj_strncmp(yomi, buf, ylen) == 0) {
        
        return 1;
    }
    return 0;
}

NJ_INT16 njd_f_search_word(NJ_SEARCH_CONDITION *con, NJ_SEARCH_LOCATION_SET *loctset)
{
    NJ_UINT16 ret;

    switch (con->operation) {
    case NJ_CUR_OP_LINK:
        
        
        if ((con->hinsi.yominasi_fore == NULL) ||
            (con->hinsi.foreSize == 0)) {
            loctset->loct.status = NJ_ST_SEARCH_END;
            return 0;
        }
        break;
    case NJ_CUR_OP_FORE:
        
        
        if (NJ_CHAR_STRLEN_IS_0(con->yomi)) {
            loctset->loct.status = NJ_ST_SEARCH_END;
            return 0;
        }

        
        if ((con->hinsi.yominasi_fore == NULL) ||
            (con->hinsi.foreSize == 0)) {
            loctset->loct.status = NJ_ST_SEARCH_END;
            return 0;
        }
        break;
    default:
        
        loctset->loct.status = NJ_ST_SEARCH_END;
        return 0;
    } 

    
    if (con->mode != NJ_CUR_MODE_FREQ) {
        
        loctset->loct.status = NJ_ST_SEARCH_END;
        return 0;
    }

    
    if ((GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_NO_INIT)
        || (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_READY)) {
        
        ret = search_data(con, loctset);
        if (ret < 1) {
            
            loctset->loct.status = NJ_ST_SEARCH_END;
        }
        return ret;
    } else {
        
        loctset->loct.status = NJ_ST_SEARCH_END; 
        return 0; 
    }
}

NJ_INT16 njd_f_get_word(NJ_SEARCH_LOCATION_SET *loctset, NJ_WORD *word)
{
    NJ_UINT8 *data;
    NJ_CHAR  stroke[NJ_MAX_LEN + NJ_TERM_LEN];
    NJ_INT16 yomilen, kouholen;


    
    if (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_END) {
        return 0; 
    }

    
    data = STEM_AREA_TOP_ADDR(loctset->loct.handle) + loctset->loct.current;

    NJ_SET_YLEN_TO_STEM(word, 1);

    
    word->stem.loc = loctset->loct;                                     
    yomilen = njd_f_get_stroke(word, stroke, sizeof(stroke));
    if (yomilen <= 0) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_F_GET_WORD, NJ_ERR_INVALID_RESULT); 
    }
    word->stem.info1 = yomilen;
    word->stem.info1 |= (NJ_UINT16)(DATA_FHINSI(data) << HINSI_OFFSET); 
    word->stem.info2 = (NJ_UINT16)(DATA_BHINSI(data) << HINSI_OFFSET);  
    kouholen = (NJ_UINT16)DATA_CANDIDATE_SIZE(data)/sizeof(NJ_CHAR);
    if (kouholen == 0) {
        
        kouholen = yomilen;
    }
    word->stem.info2 |= kouholen;                                       
    word->stem.hindo = CALCULATE_HINDO(DATA_HINDO(data), loctset->dic_freq.base, 
                                       loctset->dic_freq.high, YOMINASI_DIC_FREQ_DIV); 

    
    word->stem.type = 0;

    return 1;
}

NJ_INT16 njd_f_get_stroke(NJ_WORD *word, NJ_CHAR *stroke, NJ_UINT16 size) {
    NJ_SEARCH_LOCATION *loc;
    NJ_UINT8 *area, *data;
    NJ_UINT16 len;
    NJ_UINT32 j;

    if (NJ_GET_YLEN_FROM_STEM(word) == 0) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_F_GET_STROKE, NJ_ERR_INVALID_RESULT); 
    }


    
    loc = &word->stem.loc;
    data = STEM_AREA_TOP_ADDR(loc->handle) + loc->current;

    
    area = YOMI_AREA_TOP_ADDR(loc->handle) + DATA_YOMI(data);

    if (YOMI_INDX_CNT(loc->handle) == 0) {      
        
        len = DATA_YOMI_SIZE(data)/sizeof(NJ_CHAR);

        
        if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_F_GET_STROKE, NJ_ERR_BUFFER_NOT_ENOUGH); 
        }

        for (j = 0; j < len; j++) {
            NJ_CHAR_COPY(stroke, area); 
            stroke++;
            area += sizeof(NJ_CHAR);
        }
        *stroke = NJ_CHAR_NUL;
    } else {                                    
        
        len = convert_to_yomi(loc->handle, area, DATA_YOMI_SIZE(data), stroke, size);

        
        if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_F_GET_STROKE, NJ_ERR_BUFFER_NOT_ENOUGH); 
        }
    }
    return len;
}

NJ_INT16 njd_f_get_candidate(NJ_WORD *word, NJ_CHAR *candidate, NJ_UINT16 size)
{
    NJ_SEARCH_LOCATION *loc;
    NJ_UINT8 *data, *area;
    NJ_CHAR   work[NJ_MAX_LEN + NJ_TERM_LEN];
    NJ_UINT16 len, j;



    
    loc = &word->stem.loc;
    data = STEM_AREA_TOP_ADDR(loc->handle) + loc->current;

    
    len = DATA_CANDIDATE_SIZE(data)/sizeof(NJ_CHAR);
    if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_F_GET_CANDIDATE, NJ_ERR_BUFFER_NOT_ENOUGH); 
    }

    
    if (len == 0) {     
        
        area = YOMI_AREA_TOP_ADDR(loc->handle) + DATA_YOMI(data);
        if (YOMI_INDX_CNT(loc->handle) == 0) {  
            
            len = DATA_YOMI_SIZE(data)/sizeof(NJ_CHAR);

            
            if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
                return NJ_SET_ERR_VAL(NJ_FUNC_NJD_F_GET_STROKE, NJ_ERR_BUFFER_NOT_ENOUGH); 
            }
            for (j = 0; j < len; j++) {
                NJ_CHAR_COPY(candidate + j, area);   
                area += sizeof(NJ_CHAR);
            }
            candidate[len] = NJ_CHAR_NUL;
            return len;
        } else {                                        
            
            len = convert_to_yomi(loc->handle, area, DATA_YOMI_SIZE(data), work, size);

            
            if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
                return NJ_SET_ERR_VAL(NJ_FUNC_NJD_F_GET_CANDIDATE, NJ_ERR_BUFFER_NOT_ENOUGH); 
            }
        }

        if (DATA_CANDIDATE(data) & NO_CONV_FLG) {       
            nje_convert_hira_to_kata(work, candidate, len);
        } else {                                        
            for (j = 0; j < len; j++) {
                candidate[j] = work[j];
            }
        }
    } else {            
        
        area = STRS_AREA_TOP_ADDR(loc->handle) + DATA_CANDIDATE(data);
        for (j = 0; j < len; j++) {
            NJ_CHAR_COPY(candidate + j, area);
            area += sizeof(NJ_CHAR);
        }
    }

    candidate[len] = NJ_CHAR_NUL;
    return len;
}
