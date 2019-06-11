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

#ifndef _NJX_LIB_H_
#define _NJX_LIB_H_


#define NJD_MAX_CONNECT_CNT     6

typedef struct {
    NJ_UINT16 f_hinsi;                                  
    NJ_UINT16 b_hinsi;                                  
    NJ_UINT8  yomi_len;                                 
    NJ_UINT8  hyouki_len;                               
    NJ_CHAR   yomi[NJ_MAX_LEN +NJ_TERM_LEN];           
    NJ_CHAR   hyouki[NJ_MAX_RESULT_LEN + NJ_TERM_LEN]; 
    NJ_UINT16 stem_b_hinsi;                             
    NJ_UINT8  fzk_yomi_len;                             
} NJ_LEARN_WORD_INFO;


typedef struct word_que {
    NJ_UINT16  entry;           
    NJ_UINT8   type;            
    NJ_UINT16  mae_hinsi;       
    NJ_UINT16  ato_hinsi;       
    NJ_UINT8   yomi_len;        
    NJ_UINT8   hyouki_len;      
    NJ_UINT8   yomi_byte;       
    NJ_UINT8   hyouki_byte;     
    NJ_UINT8   next_flag;       
} NJ_WQUE;


typedef struct {
    NJ_LEARN_WORD_INFO  selection_data;                             
    NJ_UINT8            count;                                      
} NJ_PREVIOUS_SELECTION_INFO;

typedef struct {
    
    
    
    
    NJ_WQUE que_tmp;

    
    
    
    
    NJ_PREVIOUS_SELECTION_INFO previous_selection;

    
    
    
    
    NJ_CHAR learn_string_tmp[NJ_MAX_RESULT_LEN + NJ_TERM_LEN];
    
    NJ_CHAR muhenkan_tmp[NJ_MAX_RESULT_LEN + NJ_TERM_LEN];

    
    
    
    NJ_DIC_SET dic_set;         

    struct {
        NJ_UINT8   commit_status;
        NJ_UINT16  save_top;
        NJ_UINT16  save_bottom;
        NJ_UINT16  save_count;
    } learndic_status;

} NJ_CLASS;

#endif 
