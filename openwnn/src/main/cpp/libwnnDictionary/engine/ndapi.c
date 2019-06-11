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

#define NJ_DIC_UNCOMP_EXT_HEADER_SIZE   0x002C      
#define CREATE_DIC_TYPE_USER            0           

#define GET_HYOKI_INDEX_OFFSET(cnt)                             \
    (NJ_LEARN_DIC_HEADER_SIZE + NJ_INDEX_SIZE * ((cnt)+1))

#define GET_DATA_AREA_OFFSET(cnt)                               \
    (NJ_LEARN_DIC_HEADER_SIZE + NJ_INDEX_SIZE * ((cnt)+1) * 2)
#define GET_EXT_DATA_AREA_OFFSET(cnt)                                   \
    (NJ_LEARN_DIC_HEADER_SIZE + NJ_INDEX_SIZE * ((cnt)+1) * 2 + LEARN_DIC_QUE_SIZE * (cnt))

#define MIN_SIZE_OF_USER_DIC                                            \
    (NJ_LEARN_DIC_HEADER_SIZE + NJ_USER_QUE_SIZE + 2 * (NJ_INDEX_SIZE * (1+1)) + 4)
#define GET_MAX_WORD_NUM_IN_USER_DIC(size)                              \
    (((size) - NJ_LEARN_DIC_HEADER_SIZE - (2 * NJ_INDEX_SIZE) - 4)      \
     / (NJ_USER_QUE_SIZE + 2 * NJ_INDEX_SIZE))


static NJ_INT16 check_search_cursor(NJ_CLASS *iwnn, NJ_CURSOR *cursor);
static NJ_INT16 search_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_UINT8 comp_flg, NJ_UINT8 *exit_flag);
static void set_operation_id(NJ_SEARCH_LOCATION *dicinfo, NJ_UINT8 reverse, NJ_RESULT *result);
static NJ_INT16 get_word_and_search_next_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_RESULT *result, NJ_UINT8 comp_flg);

static NJ_INT16 njd_check_dic(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle);

static NJ_INT16 check_search_cursor(NJ_CLASS *iwnn, NJ_CURSOR *cursor) {
    NJ_UINT16 i;
    NJ_DIC_INFO *dicinfo;
    NJ_SEARCH_LOCATION_SET *loctset;


    if (cursor->cond.ds == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_PARAM_DIC_NULL);
    }


    for (i = 0; i < NJ_MAX_DIC; i++) {
        loctset = &(cursor->loctset[i]);
        dicinfo = &(cursor->cond.ds->dic[i]);
        
        
        njd_init_search_location_set(loctset);

        if (dicinfo->handle != NULL) {
            
            
            
            if (
                (dicinfo->dic_freq[NJ_MODE_TYPE_HENKAN].high > DIC_FREQ_HIGH) ) {
                return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_DIC_FREQ_INVALID);
            }

            
            loctset->loct.handle        = dicinfo->handle;
            loctset->loct.type          = dicinfo->type;
            loctset->loct.current_info  = 0x10;  
            loctset->loct.status        = NJ_ST_SEARCH_NO_INIT;
            loctset->dic_freq           = dicinfo->dic_freq[NJ_MODE_TYPE_HENKAN];
        }
    }

    if (cursor->cond.yomi == NULL) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_PARAM_YOMI_NULL);
    } 

    if (cursor->cond.ylen > NJ_MAX_LEN) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_YOMI_TOO_LONG);
    }

    if (cursor->cond.operation == NJ_CUR_OP_LINK) {
        
    } else if (cursor->cond.kanji != NULL) {
        
        if (nj_strlen(cursor->cond.kanji) > NJ_MAX_RESULT_LEN) {
            return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_CANDIDATE_TOO_LONG);
        }
    }

    switch (cursor->cond.operation) {
    case NJ_CUR_OP_COMP:
    case NJ_CUR_OP_FORE:
    case NJ_CUR_OP_LINK:
        break;
    default:
        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_PARAM_OPERATION);
    }

    switch (cursor->cond.mode) {
    case NJ_CUR_MODE_FREQ:
    case NJ_CUR_MODE_YOMI:
        break;
    default:
        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_PARAM_MODE);
    }

    return 0;
}

static NJ_INT16 search_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_UINT8 comp_flg,
                            NJ_UINT8 *exit_flag) {
    NJ_UINT32 dic_type;
    NJ_INT16 i;
    NJ_INT16 ret = 0;
    NJ_INT16 flag = 0;
    NJ_SEARCH_LOCATION_SET *loctset;


    *exit_flag = 1;
    for (i = 0; i < NJ_MAX_DIC; i++) {
        loctset = &(cursor->loctset[i]);

        if (loctset->loct.handle == NULL) {
            continue;   
        }

        dic_type = NJ_GET_DIC_TYPE_EX(loctset->loct.type, loctset->loct.handle);
#ifdef IWNN_ERR_CHECK
        if (iwnn->err_check_flg == 12) {
            dic_type = 0x11111111;
        }
#endif 
        switch (dic_type) {
        case NJ_DIC_TYPE_JIRITSU:                       
        case NJ_DIC_TYPE_FZK:                           
        case NJ_DIC_TYPE_TANKANJI:                      
        case NJ_DIC_TYPE_STDFORE:                       
        case NJ_DIC_TYPE_CUSTOM_COMPRESS:               
        case NJ_DIC_TYPE_FORECONV:                      
            ret = njd_b_search_word(&cursor->cond, loctset);
            break;
        case NJ_DIC_TYPE_USER:                          
        case NJ_DIC_TYPE_CUSTOM_INCOMPRESS:             
            ret = njd_l_search_word(iwnn, &cursor->cond, loctset, comp_flg);
            break;

        case NJ_DIC_TYPE_YOMINASHI:                     
            ret = njd_f_search_word(&cursor->cond, loctset);
            break;

        default:
            return NJ_SET_ERR_VAL(NJ_FUNC_SEARCH_WORD, NJ_ERR_DIC_TYPE_INVALID);
        }
        if (ret < 0) {
            return ret;
        }
        if (ret == 0) {
            if ((GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_END)
                && (*exit_flag == 1)) {
                *exit_flag = 0;
            }
            
            loctset->loct.status = NJ_ST_SEARCH_END;
        
        } else {
            flag = 1;
            *exit_flag = 0;
        }
    }
    return flag;
}

static NJ_INT16 get_word_and_search_next_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_RESULT *result,
                                              NJ_UINT8 comp_flg) {
    NJ_INT16  ret = -1;
    NJ_INT32  i, next, first;
    NJ_WORD   tmp_word;
    NJ_RESULT tmp_result;
    NJ_CHAR   tmp_stroke[NJ_MAX_LEN + NJ_TERM_LEN];
    NJ_CHAR   result_stroke[NJ_MAX_LEN + NJ_TERM_LEN];
    NJ_INT32  j, max_len = 0;
    NJ_UINT32 dic_type;
    NJ_SEARCH_LOCATION_SET *loctset;


    next = -1;
    first= 0;
    
    njd_init_word(&tmp_word);

    result->word = tmp_word;
    tmp_result.word = tmp_word;
    
    for (i = 0; i < NJ_MAX_DIC; i++) {
        loctset = &(cursor->loctset[i]);
        if ((loctset->loct.handle == NULL) ||
            (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_END) ||
            (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_END_EXT)) {
            continue;
        }

        dic_type = NJ_GET_DIC_TYPE_EX(loctset->loct.type, loctset->loct.handle);

        switch (dic_type) {
        case NJ_DIC_TYPE_JIRITSU:               
        case NJ_DIC_TYPE_FZK:                   
        case NJ_DIC_TYPE_TANKANJI:              
        case NJ_DIC_TYPE_STDFORE:               
        case NJ_DIC_TYPE_CUSTOM_COMPRESS:       
        case NJ_DIC_TYPE_FORECONV:              
            tmp_word.yomi = cursor->cond.yomi;
            tmp_word.stem.info1 = cursor->cond.ylen;
            tmp_result.word.yomi = cursor->cond.yomi;
            tmp_result.word.stem.info1 = cursor->cond.ylen;
            break;
        default:
            break;
        }

        loctset->loct.status |= SET_LOCATION_OPERATION(cursor->cond.operation);
        if (cursor->cond.mode == NJ_CUR_MODE_FREQ) {
            if ((cursor->cond.ds->mode & (NJ_CACHE_MODE_VALID)) &&
                (cursor->cond.ds->dic[i].srhCache != NULL) &&
                (NJ_GET_AIMAI_FROM_SCACHE(cursor->cond.ds->dic[i].srhCache)) &&
                (cursor->cond.operation == NJ_CUR_OP_FORE)) {
                first = 1;

                ret = njd_get_word_data(iwnn, cursor->cond.ds, loctset, (NJ_UINT16)i, &tmp_result.word);
                if (ret < 0) {
                    return ret; 
                }

                ret = njd_get_stroke(iwnn, &tmp_result, tmp_stroke, sizeof(tmp_stroke));
                if (ret <= 0) {
                    if ((ret == 0) || (NJ_GET_ERR_CODE(ret) == NJ_ERR_BUFFER_NOT_ENOUGH)) { 
                        return NJ_SET_ERR_VAL(NJ_FUNC_GET_WORD_AND_SEARCH_NEXT_WORD, NJ_ERR_INVALID_RESULT); 
                    } else {
                        return ret; 
                    }
                }
                for (j = 0; j < cursor->cond.ylen; j++) {
                    if (cursor->cond.yomi[j] != tmp_stroke[j]) {
                        break;
                    }
                }

                switch (dic_type) {
                case NJ_DIC_TYPE_JIRITSU:                       
                case NJ_DIC_TYPE_FZK:                           
                case NJ_DIC_TYPE_TANKANJI:                      
                case NJ_DIC_TYPE_STDFORE:                       
                case NJ_DIC_TYPE_CUSTOM_COMPRESS:               
                case NJ_DIC_TYPE_FORECONV:                      
                    ret = njd_b_search_word(&cursor->cond, loctset);
                    break;

                case NJ_DIC_TYPE_USER:                          
                case NJ_DIC_TYPE_CUSTOM_INCOMPRESS:             
                    ret = njd_l_search_word(iwnn, &cursor->cond, loctset, comp_flg);
                    break;

                default:
                    return NJ_SET_ERR_VAL(NJ_FUNC_GET_WORD_AND_SEARCH_NEXT_WORD, NJ_ERR_DIC_TYPE_INVALID); 
                }

                if (ret < 0) {
                    return ret; 
                }
            } else {
                ret = njd_get_word_data(iwnn, cursor->cond.ds, loctset, (NJ_UINT16)i, &tmp_result.word);
                if (ret < 0) {
                    return ret; 
                }
                j = cursor->cond.ylen;
            }

            if ((j > max_len) ||
                ((j == max_len) && (loctset->cache_freq > result->word.stem.hindo)) ||
                (next == -1)) {
                
                set_operation_id(&(loctset->loct), 0, result);

                result->word = tmp_result.word;

                next = i;  
                max_len = j;
            }

        } else {
            
            ret = njd_get_word_data(iwnn, cursor->cond.ds, loctset, (NJ_UINT16)i, &(tmp_result.word));
            if (ret < 0) {
                return ret; 
            }

            
            ret = njd_get_stroke(iwnn, &tmp_result, tmp_stroke, sizeof(tmp_stroke));
            if (ret <= 0) {
                if ((ret == 0) || (NJ_GET_ERR_CODE(ret) == NJ_ERR_BUFFER_NOT_ENOUGH)) { 
                    return NJ_SET_ERR_VAL(NJ_FUNC_GET_WORD_AND_SEARCH_NEXT_WORD, NJ_ERR_INVALID_RESULT); 
                } else {
                    return ret; 
                }
            }
            if ((next == -1) || (nj_strcmp(result_stroke, tmp_stroke) > 0)) {
                
                set_operation_id(&(loctset->loct), 0, result);

                result->word = tmp_result.word;

                next = i;  
                nj_strcpy(result_stroke, tmp_stroke);
            }
        }
    }

    
    if (next == -1) {
        return 0;
    }

    loctset = &(cursor->loctset[next]);
    if ((!first) ||
        ((loctset->loct.handle != NULL) &&
         (cursor->cond.ds->dic[next].srhCache == NULL))) {
        dic_type = NJ_GET_DIC_TYPE_EX(loctset->loct.type, loctset->loct.handle);

        
        switch (dic_type) {
        case NJ_DIC_TYPE_JIRITSU:                       
        case NJ_DIC_TYPE_FZK:                           
        case NJ_DIC_TYPE_TANKANJI:                      
        case NJ_DIC_TYPE_STDFORE:                       
        case NJ_DIC_TYPE_CUSTOM_COMPRESS:               
        case NJ_DIC_TYPE_FORECONV:                      
            ret = njd_b_search_word(&cursor->cond, loctset);
            break;

        case NJ_DIC_TYPE_USER:                          
        case NJ_DIC_TYPE_CUSTOM_INCOMPRESS:             
            ret = njd_l_search_word(iwnn, &cursor->cond, loctset, comp_flg);
            break;

        case NJ_DIC_TYPE_YOMINASHI:                     
            ret = njd_f_search_word(&cursor->cond, loctset);
            break;

        default:
            return NJ_SET_ERR_VAL(NJ_FUNC_GET_WORD_AND_SEARCH_NEXT_WORD, NJ_ERR_DIC_TYPE_INVALID); 
        }
    }

    if (ret < 0) {
        return ret; 
    }
    return 1;
}

NJ_INT16 njd_get_word_data(NJ_CLASS *iwnn, NJ_DIC_SET *dicset, NJ_SEARCH_LOCATION_SET *loctset, NJ_UINT16 dic_idx, NJ_WORD *word) {
    NJ_INT16 ret = 0;
    NJ_UINT32 dic_type;


    
    if (GET_LOCATION_STATUS(loctset->loct.status) == NJ_ST_SEARCH_END) {
        return 0; 
    }
    
    if (loctset->loct.handle == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_WORD_DATA, NJ_ERR_DIC_TYPE_INVALID); 
    }
    
    dic_type = NJ_GET_DIC_TYPE_EX(loctset->loct.type, loctset->loct.handle);

    switch (dic_type) {
    case NJ_DIC_TYPE_JIRITSU:                   
    case NJ_DIC_TYPE_FZK:                       
    case NJ_DIC_TYPE_TANKANJI:                  
    case NJ_DIC_TYPE_STDFORE:                   
    case NJ_DIC_TYPE_CUSTOM_COMPRESS:           
    case NJ_DIC_TYPE_FORECONV:                  
        ret = njd_b_get_word(loctset, word);
        break;

    case NJ_DIC_TYPE_USER:                      
    case NJ_DIC_TYPE_CUSTOM_INCOMPRESS:         
        ret = njd_l_get_word(iwnn, loctset, word);
        break;

    case NJ_DIC_TYPE_YOMINASHI:                 
        ret = njd_f_get_word(loctset, word);
        break;

    default:
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_WORD_DATA, NJ_ERR_DIC_TYPE_INVALID); 
    }
    return ret;
}

static void set_operation_id(NJ_SEARCH_LOCATION *dicinfo, NJ_UINT8 reverse, NJ_RESULT *result) {
    NJ_UINT16 dictype;
    NJ_UINT32 type;

    if (dicinfo->handle == NULL) {
        
        dictype = NJ_DIC_STATIC; 
        return; 
    }

    type = NJ_GET_DIC_TYPE_EX(NJ_GET_DIC_INFO(dicinfo), dicinfo->handle);

    
    switch (type) {
    case NJ_DIC_TYPE_JIRITSU:                   
    case NJ_DIC_TYPE_FZK:                       
    case NJ_DIC_TYPE_TANKANJI:                  
    case NJ_DIC_TYPE_STDFORE:                   
    case NJ_DIC_TYPE_YOMINASHI:                 

    case NJ_DIC_TYPE_FORECONV:                  
        dictype = NJ_DIC_STATIC;
        break;

    case NJ_DIC_TYPE_CUSTOM_INCOMPRESS:         
    case NJ_DIC_TYPE_CUSTOM_COMPRESS:           
        dictype = NJ_DIC_CUSTOMIZE;
        break;

    case NJ_DIC_TYPE_USER:                      
        dictype = NJ_DIC_USER;
        break;

    default:
        
        dictype = NJ_DIC_STATIC; 
    }

    
    result->operation_id =
        (NJ_UINT16)((NJ_UINT16)NJ_OP_SEARCH | (NJ_UINT16)NJ_FUNC_SEARCH | dictype);
}

static NJ_INT16 njd_search_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_UINT8 comp_flg,
                         NJ_UINT8 *exit_flag) {
    NJ_INT16 ret;


    ret = check_search_cursor(iwnn, cursor);
    if (ret != 0) {
        return ret;
    }
    
    return search_word(iwnn, cursor, comp_flg, exit_flag);
}

static NJ_INT16 njd_get_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_RESULT *result,
                      NJ_UINT8 comp_flg) {

    NJ_INT16    ret;


    ret = get_word_and_search_next_word(iwnn, cursor, result, comp_flg);

    return ret;
}

NJ_INT16 njd_get_stroke(NJ_CLASS *iwnn, NJ_RESULT *result, NJ_CHAR *stroke, NJ_UINT16 size) {
    NJ_INT16 ret = 0;
    NJ_UINT16 len;
    NJ_UINT32 dictype;


    if (result->word.stem.loc.handle == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_STROKE, NJ_ERR_INVALID_RESULT);
    }

    dictype = NJ_GET_DIC_TYPE_EX(result->word.stem.loc.type, result->word.stem.loc.handle);

    switch (dictype) {
    case NJ_DIC_TYPE_JIRITSU:                   
    case NJ_DIC_TYPE_FZK:                       
    case NJ_DIC_TYPE_TANKANJI:                  
    case NJ_DIC_TYPE_STDFORE:                   
    case NJ_DIC_TYPE_CUSTOM_COMPRESS:           
    case NJ_DIC_TYPE_FORECONV:                  
        if (GET_LOCATION_OPERATION(result->word.stem.loc.status) != NJ_CUR_OP_COMP) {
            ret = njd_b_get_stroke(&result->word, stroke, size);
        } else {
            len = NJ_GET_YLEN_FROM_STEM(&result->word);

            if (size < ((len + NJ_TERM_LEN) * sizeof(NJ_CHAR))) {
                return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_STROKE,
                                      NJ_ERR_BUFFER_NOT_ENOUGH);
            }
            if (len == 0) {
                return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_STROKE, 
                                      NJ_ERR_INVALID_RESULT);
            }
            nj_strncpy(stroke, result->word.yomi, len);
            *(stroke + len) = NJ_CHAR_NUL;
            return len;
        }
        break;

    case NJ_DIC_TYPE_USER:                      
    case NJ_DIC_TYPE_CUSTOM_INCOMPRESS:         
        ret = njd_l_get_stroke(iwnn, &result->word, stroke, size);
        break;

    case NJ_DIC_TYPE_YOMINASHI:                 
        ret = njd_f_get_stroke(&result->word, stroke, size);
        break;

    default:
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_STROKE, NJ_ERR_DIC_TYPE_INVALID); 
    }
    
    if (ret == 0) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_STROKE, NJ_ERR_INVALID_RESULT); 
    }
    return ret;
}


NJ_INT16 njd_get_candidate(NJ_CLASS *iwnn, NJ_RESULT *result,
                           NJ_CHAR *candidate, NJ_UINT16 size) {
    NJ_INT16 ret = 0;
    NJ_UINT32 dictype;


    if (result->word.stem.loc.handle == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_CANDIDATE, NJ_ERR_INVALID_RESULT); 
    }

    dictype = NJ_GET_DIC_TYPE_EX(result->word.stem.loc.type, result->word.stem.loc.handle);

    switch (dictype) {
    case NJ_DIC_TYPE_JIRITSU:                   
    case NJ_DIC_TYPE_FZK:                       
    case NJ_DIC_TYPE_TANKANJI:                  
    case NJ_DIC_TYPE_STDFORE:                   
    case NJ_DIC_TYPE_CUSTOM_COMPRESS:           
    case NJ_DIC_TYPE_FORECONV:                  
        ret = njd_b_get_candidate(&result->word, candidate, size);
        break;

    case NJ_DIC_TYPE_USER:                      
    case NJ_DIC_TYPE_CUSTOM_INCOMPRESS:         
        ret = njd_l_get_candidate(iwnn, &result->word, candidate, size);
        break;

    case NJ_DIC_TYPE_YOMINASHI:                 
        ret = njd_f_get_candidate(&result->word, candidate, size);
        break;

    default:
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_CANDIDATE, NJ_ERR_DIC_TYPE_INVALID); 
    }
    
    if (ret == 0) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_GET_CANDIDATE, NJ_ERR_INVALID_RESULT); 
    }
    return ret;
}


static NJ_INT16 njd_check_dic(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle) {
    NJ_UINT8 *addr;
    NJ_UINT32 datasize, extsize;
    NJ_UINT32 version;
    NJ_UINT32 type;


    addr = handle;

    
    if (NJ_INT32_READ(addr) != NJ_DIC_IDENTIFIER) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
    }
    addr += sizeof(NJ_UINT32);

    
    version = NJ_INT32_READ(addr);
    if ((version != NJ_DIC_VERSION1) && (version != NJ_DIC_VERSION2) && 
        (version != NJ_DIC_VERSION2_1) && (version != NJ_DIC_VERSION3)) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
    }
    addr += sizeof(NJ_UINT32);

    
    type = NJ_INT32_READ(addr);
    addr += sizeof(NJ_UINT32);

    
    datasize = NJ_INT32_READ(addr);
    addr += sizeof(NJ_UINT32);

    
    extsize = NJ_INT32_READ(addr);
    addr += sizeof(NJ_UINT32);

    
    if (NJ_INT32_READ(addr) > (NJ_MAX_LEN * sizeof(NJ_CHAR))) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
    }
    addr += sizeof(NJ_UINT32);

    
    if (NJ_INT32_READ(addr) > (NJ_MAX_RESULT_LEN * sizeof(NJ_CHAR))) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
    }

    
    addr += (extsize + datasize);
    if (NJ_INT32_READ(addr) != NJ_DIC_IDENTIFIER) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
    }

    
    switch (type) {

    case NJ_DIC_TYPE_JIRITSU:                   
    case NJ_DIC_TYPE_FZK:                       
    case NJ_DIC_TYPE_TANKANJI:                  
    case NJ_DIC_TYPE_CUSTOM_COMPRESS:           
    case NJ_DIC_TYPE_STDFORE:                   
        
        if (version != (NJ_UINT32)NJ_DIC_VERSION2) {
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
        }
        break;

    case NJ_DIC_TYPE_RULE:                      
        
        if (version != (NJ_UINT32)NJ_DIC_VERSION2_1) {
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
        }
        break;

    case NJ_DIC_TYPE_YOMINASHI:                 
        
        if (version != (NJ_UINT32)NJ_DIC_VERSION1) {
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
        }
        break;

    case NJ_DIC_TYPE_USER:                      
        
        if (version != (NJ_UINT32)NJ_DIC_VERSION2) {
            return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_FORMAT_INVALID);
        }
        return njd_l_check_dic(iwnn, handle);

    default:
        return NJ_SET_ERR_VAL(NJ_FUNC_NJD_CHECK_DIC, NJ_ERR_DIC_TYPE_INVALID);
    }
    return 0;
}


NJ_EXTERN NJ_INT16 njx_search_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor) {

    NJ_SEARCH_CACHE     *pCache;
    NJ_CHAR             *p_yomi, *p_key;
    NJ_UINT16           initst, inited;
    NJ_UINT16           clrcnt, diccnt;
    NJ_UINT16           kw_len;
    NJ_UINT16           cacheOverKeyPtr;

    NJ_UINT8 exit_flag;                         
    NJ_UINT8 cnt;
    NJ_DIC_HANDLE dhdl;
    NJ_PREVIOUS_SELECTION_INFO *prev_info = &(iwnn->previous_selection);


    if (iwnn == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_SEARCH_WORD, NJ_ERR_PARAM_ENV_NULL);
    }
    if (cursor == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_SEARCH_WORD, NJ_ERR_PARAM_CURSOR_NULL);
    }

    
    cursor->cond.hinsi.fore = NULL;
    cursor->cond.hinsi.foreSize = 0;
    cursor->cond.hinsi.foreFlag = 0;
    cursor->cond.hinsi.rear = NULL;
    cursor->cond.hinsi.rearSize = 0;
    cursor->cond.hinsi.rearFlag = 0;

    
    if (cursor->cond.yomi == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_PARAM_YOMI_NULL);
    }
    cursor->cond.ylen = nj_strlen(cursor->cond.yomi);
    cursor->cond.yclen = nj_charlen(cursor->cond.yomi);

    
    if (cursor->cond.ds == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_PARAM_DIC_NULL);
    }

    
    cursor->cond.ds->mode = NJ_CACHE_MODE_VALID;

    p_yomi = cursor->cond.yomi;
    p_key  = cursor->cond.ds->keyword;

    for (clrcnt = 0; clrcnt < cursor->cond.yclen; clrcnt++) {
        if (nj_charncmp(p_yomi, p_key, 1) != 0) {
            break;
        }
        p_yomi += NJ_CHAR_LEN(p_yomi);
        p_key  += NJ_CHAR_LEN(p_key);
    }
    if (clrcnt != 0) {
        initst = clrcnt + 1;
    } else {
        initst = 0;
    }

    kw_len = nj_charlen(cursor->cond.ds->keyword);
    if (kw_len >= cursor->cond.yclen) {
      inited = kw_len + 1;
    } else {
      inited = cursor->cond.yclen + 1;
    }

    for (diccnt = 0; diccnt < NJ_MAX_DIC; diccnt++) {
        pCache = cursor->cond.ds->dic[diccnt].srhCache;
        if (pCache != NULL) {
            
            if (NJ_GET_CACHEOVER_FROM_SCACHE(pCache)) {
                
                for (cacheOverKeyPtr = 0; cacheOverKeyPtr < kw_len; cacheOverKeyPtr++) {
                    if (pCache->keyPtr[cacheOverKeyPtr] == pCache->keyPtr[cacheOverKeyPtr + 1] ) {
                        break;
                    }
                }
                cacheOverKeyPtr++;

                
                if (cacheOverKeyPtr < initst) {
                    clrcnt = cacheOverKeyPtr;
                } else {
                    clrcnt = initst;
                }
                for (; clrcnt < inited; clrcnt++) {
                    pCache->keyPtr[clrcnt] = 0x0000;
                }
                
                for (clrcnt = 1; clrcnt < inited; clrcnt++ ) {
                    if ((pCache->keyPtr[clrcnt - 1] > pCache->keyPtr[clrcnt]) &&
                        (pCache->keyPtr[clrcnt] != 0)) {
                        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_CACHE_BROKEN); 
                    }
                }
                NJ_UNSET_CACHEOVER_TO_SCACHE(pCache);
            } else {
                for (clrcnt = initst; clrcnt < inited; clrcnt++) {
                    pCache->keyPtr[clrcnt] = 0x0000;
                }
                
                for (clrcnt = 1; clrcnt < inited; clrcnt++ ) {
                    if ((pCache->keyPtr[clrcnt - 1] > pCache->keyPtr[clrcnt]) &&
                        (pCache->keyPtr[clrcnt] != 0)) {
                        return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_CACHE_BROKEN);
                    }
                }
            }
        }
    }

    
    nj_strcpy(cursor->cond.ds->keyword, cursor->cond.yomi);

    for (cnt = 0; cnt < NJ_MAX_DIC; cnt++) {
        dhdl = cursor->cond.ds->dic[cnt].handle;
        
        if (dhdl != NULL) {
            if ((cursor->cond.ds->dic[cnt].dic_freq[NJ_MODE_TYPE_HENKAN].base
                 > cursor->cond.ds->dic[cnt].dic_freq[NJ_MODE_TYPE_HENKAN].high)) {
                    return NJ_SET_ERR_VAL(NJ_FUNC_CHECK_SEARCH_CURSOR, NJ_ERR_DIC_FREQ_INVALID);
                }
        }
    }

    if( prev_info->count == 0 ) {
        cursor->cond.hinsi.yominasi_fore = NULL;
    } else {
        int prev_hinsi = prev_info->selection_data.b_hinsi;

        
        njd_r_get_connect(cursor->cond.ds->rHandle[NJ_MODE_TYPE_HENKAN], prev_hinsi,
                          0, &(cursor->cond.hinsi.yominasi_fore));
        njd_r_get_count(cursor->cond.ds->rHandle[NJ_MODE_TYPE_HENKAN],
                        &(cursor->cond.hinsi.foreSize), &(cursor->cond.hinsi.rearSize));
    }

    return njd_search_word(iwnn, cursor, 0, &exit_flag);
}


NJ_EXTERN NJ_INT16 njx_get_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_RESULT *result) {
    NJ_INT16  ret;


    
    if (iwnn == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_WORD, NJ_ERR_PARAM_ENV_NULL);
    }
    if (cursor == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_WORD, NJ_ERR_PARAM_CURSOR_NULL);
    }
    if (result == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_WORD, NJ_ERR_PARAM_RESULT_NULL);
    }

    ret = njd_get_word(iwnn, cursor, result, 0);

    return ret;
}



NJ_EXTERN NJ_INT16 njx_check_dic(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle, NJ_UINT8 restore, NJ_UINT32 size) {


    if (iwnn == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_CHECK_DIC, NJ_ERR_PARAM_ENV_NULL);
    }

    if (handle == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_CHECK_DIC, NJ_ERR_DIC_HANDLE_NULL);
    }

    
    
    if (size <= NJ_DIC_COMMON_HEADER_SIZE) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_CHECK_DIC, NJ_ERR_AREASIZE_INVALID);
    }

    
    
    if (size != (NJ_DIC_COMMON_HEADER_SIZE
                 + NJ_INT32_READ(handle + NJ_DIC_POS_DATA_SIZE)
                 + NJ_INT32_READ(handle + NJ_DIC_POS_EXT_SIZE))) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_CHECK_DIC, NJ_ERR_AREASIZE_INVALID);
    }

    return njd_check_dic(iwnn, handle);
}

NJ_INT16 njd_init_search_location_set(NJ_SEARCH_LOCATION_SET* loctset)
{

    loctset->cache_freq         = 0;
    loctset->dic_freq.base      = 0;
    loctset->dic_freq.high      = 0;
    loctset->loct.type          = NJ_DIC_H_TYPE_NORMAL;
    loctset->loct.handle        = NULL;
    loctset->loct.current_info  = 0x10;  
    loctset->loct.current       = 0;
    loctset->loct.top           = 0;
    loctset->loct.bottom        = 0;
    loctset->loct.current_cache = 0;
    loctset->loct.status        = NJ_ST_SEARCH_NO_INIT;

    return 1;
}

NJ_INT16 njd_init_word(NJ_WORD* word)
{

    word->yomi                  = NULL;
    word->stem.info1            = 0;
    word->stem.info2            = 0;
    word->stem.hindo            = 0;
    word->fzk.info1             = 0;
    word->fzk.info2             = 0;
    word->fzk.hindo             = 0;

    word->stem.loc.handle       = NULL;
    word->stem.loc.type         = NJ_DIC_H_TYPE_NORMAL;
    word->stem.loc.current      = 0;
    word->stem.loc.top          = 0;
    word->stem.loc.bottom       = 0;
    word->stem.loc.current_cache= 0;
    word->stem.loc.current_info = 0x10;  
    word->stem.loc.status       = NJ_ST_SEARCH_NO_INIT;

    return 1;
}
