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



static NJ_INT16 set_previous_selection(NJ_CLASS *iwnn, NJ_RESULT *result);
static NJ_INT16 set_learn_word_info(NJ_CLASS *iwnn, NJ_LEARN_WORD_INFO *lword, NJ_RESULT *result);



NJ_EXTERN NJ_INT16 njx_select(NJ_CLASS *iwnn, NJ_RESULT *r_result) {
    NJ_INT16 ret;
    NJ_DIC_SET *dics;


    if (iwnn == NULL) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_SELECT, NJ_ERR_PARAM_ENV_NULL);
    }
    dics = &(iwnn->dic_set);

    if (dics->rHandle[NJ_MODE_TYPE_HENKAN] == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_SELECT, NJ_ERR_NO_RULEDIC);
    }

    
    if ( r_result != NULL ) {
        
        ret = set_previous_selection(iwnn, r_result);
        if (ret < 0) {
            return ret; 
        }
    } else {
        
        set_previous_selection(iwnn, NULL);
    }
    return 0;   
}

NJ_EXTERN NJ_INT16 njx_init(NJ_CLASS *iwnn) {

    if (iwnn == NULL) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_INIT, NJ_ERR_PARAM_ENV_NULL);
    }

    
    set_previous_selection(iwnn, NULL);
    return 0;
}

NJ_EXTERN NJ_INT16 njx_get_candidate(NJ_CLASS *iwnn, NJ_RESULT *result, NJ_CHAR *buf, NJ_UINT16 buf_size) {
    NJ_INT16 ret;


    if (iwnn == NULL) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_CANDIDATE, NJ_ERR_PARAM_ENV_NULL);
    }
    if (result == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_CANDIDATE, NJ_ERR_PARAM_RESULT_NULL);
    }

    if ((buf == NULL) || (buf_size == 0)) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_CANDIDATE, NJ_ERR_BUFFER_NOT_ENOUGH);
    }

    switch (NJ_GET_RESULT_OP(result->operation_id)) {
    case NJ_OP_SEARCH:
        ret = njd_get_candidate(iwnn, result, buf, buf_size);
        break;

    default:
        
        ret = NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_CANDIDATE, NJ_ERR_INVALID_RESULT); 
        break;
    }
    
    return ret;
}

NJ_EXTERN NJ_INT16 njx_get_stroke(NJ_CLASS *iwnn, NJ_RESULT *result, NJ_CHAR *buf, NJ_UINT16 buf_size) {
    NJ_INT16 ret;


    if (iwnn == NULL) {
        
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_STROKE, NJ_ERR_PARAM_ENV_NULL);
    }
    if (result == NULL) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_STROKE, NJ_ERR_PARAM_RESULT_NULL);
    }

    if ((buf == NULL) || (buf_size == 0)) {
        return NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_STROKE, NJ_ERR_BUFFER_NOT_ENOUGH);
    }

    switch (NJ_GET_RESULT_OP(result->operation_id)) {
    case NJ_OP_SEARCH:
        ret = njd_get_stroke(iwnn, result, buf, buf_size);
        break;

    default:
        
        ret = NJ_SET_ERR_VAL(NJ_FUNC_NJ_GET_STROKE, NJ_ERR_INVALID_RESULT); 
        break;
    }
    return ret;
}


static NJ_INT16 set_previous_selection(NJ_CLASS *iwnn, NJ_RESULT *result) {
    NJ_INT16   ret;
    NJ_PREVIOUS_SELECTION_INFO *prev_info = &(iwnn->previous_selection);


    if (result == NULL) {
        prev_info->count = 0;
   } else {
        ret = set_learn_word_info(iwnn, &(prev_info->selection_data), result);
        if (ret < 0) {
            
            return ret; 
        }

        prev_info->count = 1;
    }
    
    return 0;
}

static NJ_INT16 set_learn_word_info(NJ_CLASS *iwnn, NJ_LEARN_WORD_INFO *lword, NJ_RESULT *result) 
{
    NJ_INT16 ret;
    NJ_DIC_SET *dics = &(iwnn->dic_set);



#if 0
    
    ret = njx_get_stroke(iwnn, result, lword->yomi, sizeof(lword->yomi));
    if (ret < 0) {
        return ret; 
    }
    lword->yomi_len = (NJ_UINT8)ret;
    ret = njx_get_candidate(iwnn, result, lword->hyouki, sizeof(lword->hyouki));
    if (ret < 0) {
        return ret; 
    }
    lword->hyouki_len = (NJ_UINT8)ret;
#else
    lword->yomi[0] = 0x0000;
    lword->yomi_len = 0;
    lword->hyouki[0] = 0x0000;
    lword->hyouki_len = 0;
#endif

    
    lword->f_hinsi = NJ_GET_FPOS_FROM_STEM(&(result->word));
    lword->stem_b_hinsi = NJ_GET_BPOS_FROM_STEM(&(result->word));
    lword->b_hinsi = NJ_GET_BPOS_FROM_STEM(&(result->word));

    
    ret = njd_r_get_hinsi(dics->rHandle[NJ_MODE_TYPE_HENKAN], NJ_HINSI_TANKANJI_F);
    if ((ret != 0) && (lword->f_hinsi == (NJ_UINT16)ret)) {
        ret = njd_r_get_hinsi(dics->rHandle[NJ_MODE_TYPE_HENKAN], NJ_HINSI_CHIMEI_F);
        if (ret != 0) {
            lword->f_hinsi = (NJ_UINT16)ret;
        }
    }

    
    ret = njd_r_get_hinsi(dics->rHandle[NJ_MODE_TYPE_HENKAN], NJ_HINSI_TANKANJI_B);
    if ((ret != 0) && (lword->b_hinsi == (NJ_UINT16)ret)) {
        ret = njd_r_get_hinsi(dics->rHandle[NJ_MODE_TYPE_HENKAN], NJ_HINSI_CHIMEI_B);
        if (ret != 0) {
            lword->b_hinsi = (NJ_UINT16)ret;
        }
    }

    
    ret = njd_r_get_hinsi(dics->rHandle[NJ_MODE_TYPE_HENKAN], NJ_HINSI_TANKANJI_B);
    if ((ret != 0) && (lword->stem_b_hinsi == (NJ_UINT16)ret)) {
        ret = njd_r_get_hinsi(dics->rHandle[NJ_MODE_TYPE_HENKAN], NJ_HINSI_CHIMEI_B);
        if (ret != 0) {
            lword->stem_b_hinsi = (NJ_UINT16)ret;
        }
    }

    return 0;

}
