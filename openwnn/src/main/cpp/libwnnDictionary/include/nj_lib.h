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

#define NJ_VERSION "iWnn Version 1.1.2"

#ifndef _NJ_LIB_H_
#define _NJ_LIB_H_

typedef signed char    NJ_INT8;
typedef unsigned char  NJ_UINT8;
typedef signed short   NJ_INT16;
typedef unsigned short NJ_UINT16;
typedef signed long    NJ_INT32;
typedef unsigned long  NJ_UINT32;

typedef unsigned short   NJ_CHAR;

#define NJ_CHAR_NUL  0x0000

#define NJ_TERM_LEN  1
#define NJ_TERM_SIZE (NJ_TERM_LEN)

#ifndef NULL
#define NULL 0
#endif

#ifdef NJ_STACK_CHECK_FILE
typedef NJ_VOID VOID;
#endif 

#ifndef NJ_CHAR_WAVE_DASH_BIG
#define NJ_CHAR_WAVE_DASH_BIG   0xFF5E 
#endif 
#ifndef NJ_CHAR_WAVE_DASH_SMALL
#define NJ_CHAR_WAVE_DASH_SMALL 0x007E 
#endif 

typedef NJ_INT16 NJ_HINDO;      

#define NJ_INDEX_SIZE      2

#define NJ_LEARN_DIC_HEADER_SIZE   72

#ifndef NJ_MAX_DIC
#define NJ_MAX_DIC 20
#endif 

#ifndef NJ_MAX_CHARSET
#define NJ_MAX_CHARSET 200
#endif 

#ifndef NJ_SEARCH_CACHE_SIZE
#define NJ_SEARCH_CACHE_SIZE   200
#endif 
 
#ifndef NJ_CACHE_VIEW_CNT
#define NJ_CACHE_VIEW_CNT       2
#endif 


#ifndef NJ_MAX_RESULT_LEN
#define NJ_MAX_RESULT_LEN  50
#endif 

#ifndef NJ_MAX_LEN
#define NJ_MAX_LEN          50
#endif 

#ifndef NJ_MAX_KEYWORD
#define NJ_MAX_KEYWORD (NJ_MAX_RESULT_LEN + NJ_TERM_LEN)
#endif 

#ifndef NJ_MAX_PHRASE
#define NJ_MAX_PHRASE       NJ_MAX_LEN
#endif 

#ifndef NJ_MAX_PHR_CONNECT
#define NJ_MAX_PHR_CONNECT      5
#endif 

#ifndef NJ_MAX_USER_LEN
#define NJ_MAX_USER_LEN         50
#endif 

#ifndef NJ_MAX_USER_KOUHO_LEN
#define NJ_MAX_USER_KOUHO_LEN   50
#endif 

#ifndef NJ_MAX_USER_COUNT
#define NJ_MAX_USER_COUNT       100
#endif 

#define NJ_USER_QUE_SIZE        (((NJ_MAX_USER_LEN + NJ_MAX_USER_KOUHO_LEN) * sizeof(NJ_CHAR)) + 5)
#define NJ_USER_DIC_SIZE        ((NJ_USER_QUE_SIZE + NJ_INDEX_SIZE + NJ_INDEX_SIZE) * NJ_MAX_USER_COUNT + NJ_INDEX_SIZE  + NJ_INDEX_SIZE + NJ_LEARN_DIC_HEADER_SIZE + 4)

typedef NJ_UINT8 * NJ_DIC_HANDLE;

typedef struct {
    NJ_UINT16 base;         
    NJ_UINT16 high;         
} NJ_DIC_FREQ;

typedef struct {
    NJ_UINT32  current;     
    NJ_UINT32  top;         
    NJ_UINT32  bottom;      
    NJ_UINT8  *node;        
    NJ_UINT8  *now;         
    NJ_UINT16  idx_no;      
} NJ_CACHE_INFO;

typedef struct {
    NJ_UINT8      statusFlg;                        
#define NJ_STATUSFLG_CACHEOVER ((NJ_UINT8)0x01)     
#define NJ_STATUSFLG_AIMAI     ((NJ_UINT8)0x02)     
#define NJ_STATUSFLG_HINDO     ((NJ_UINT8)0x04)     
    NJ_UINT8      viewCnt;                          
    NJ_UINT16     keyPtr[NJ_MAX_KEYWORD];           
    NJ_CACHE_INFO storebuff[NJ_SEARCH_CACHE_SIZE];  
} NJ_SEARCH_CACHE;

#define NJ_GET_CACHEOVER_FROM_SCACHE(s) ((s)->statusFlg & NJ_STATUSFLG_CACHEOVER)
#define NJ_GET_AIMAI_FROM_SCACHE(s)     ((s)->statusFlg & NJ_STATUSFLG_AIMAI)
#define NJ_SET_CACHEOVER_TO_SCACHE(s)   ((s)->statusFlg |= NJ_STATUSFLG_CACHEOVER)
#define NJ_SET_AIMAI_TO_SCACHE(s)       ((s)->statusFlg |= NJ_STATUSFLG_AIMAI)
#define NJ_UNSET_CACHEOVER_TO_SCACHE(s) ((s)->statusFlg &= ~NJ_STATUSFLG_CACHEOVER) 
#define NJ_UNSET_AIMAI_TO_SCACHE(s)     ((s)->statusFlg &= ~NJ_STATUSFLG_AIMAI)


typedef struct {
    NJ_UINT8           type;            
#define NJ_DIC_H_TYPE_NORMAL   0x00     
    NJ_UINT8           limit;           

    NJ_DIC_HANDLE       handle;         

#define NJ_MODE_TYPE_MAX  1   
    
    NJ_DIC_FREQ         dic_freq[NJ_MODE_TYPE_MAX];
#define NJ_MODE_TYPE_HENKAN  0   

    NJ_SEARCH_CACHE *   srhCache;       
} NJ_DIC_INFO;


typedef struct {
    NJ_DIC_INFO   dic[NJ_MAX_DIC];           
    NJ_DIC_HANDLE  rHandle[NJ_MODE_TYPE_MAX]; 

    
    NJ_UINT16           mode;
#define NJ_CACHE_MODE_NONE          0x0000    
#define NJ_CACHE_MODE_VALID         0x0001    

    
    NJ_CHAR             keyword[NJ_MAX_KEYWORD];
} NJ_DIC_SET;

typedef struct {
    NJ_UINT16  charset_count;               
    NJ_CHAR    *from[NJ_MAX_CHARSET];       
    NJ_CHAR    *to[NJ_MAX_CHARSET];         
} NJ_CHARSET;


typedef struct {

    NJ_UINT8 operation;          
#define NJ_CUR_OP_COMP      0    
#define NJ_CUR_OP_FORE      1    
#define NJ_CUR_OP_LINK      2    

    NJ_UINT8 mode;               
#define NJ_CUR_MODE_FREQ    0    
#define NJ_CUR_MODE_YOMI    1    

    NJ_DIC_SET *ds;              

    struct {
        NJ_UINT8 *fore;          
        NJ_UINT16 foreSize;      
        NJ_UINT16 foreFlag;      
        NJ_UINT8 *rear;          
        NJ_UINT16 rearSize;      
        NJ_UINT16 rearFlag;      
        NJ_UINT8 *yominasi_fore; 
    } hinsi;

    NJ_CHAR  *yomi;
    NJ_UINT16 ylen;       
    NJ_UINT16 yclen;      
    NJ_CHAR  *kanji;      

    NJ_CHARSET *charset;  

} NJ_SEARCH_CONDITION;

typedef struct {
    NJ_DIC_HANDLE  handle;        
    NJ_UINT32      current;       
    NJ_UINT32      top;           
    NJ_UINT32      bottom;        
    NJ_UINT32      relation[NJ_MAX_PHR_CONNECT];   
    NJ_UINT8       current_cache; 
    NJ_UINT8       current_info;  
    NJ_UINT8       status;       
    NJ_UINT8       type;     
} NJ_SEARCH_LOCATION;

typedef struct {
    NJ_HINDO           cache_freq;   
    NJ_DIC_FREQ        dic_freq;     
    NJ_SEARCH_LOCATION loct;         
} NJ_SEARCH_LOCATION_SET;

typedef struct {
    NJ_SEARCH_CONDITION cond;                   
    NJ_SEARCH_LOCATION_SET loctset[NJ_MAX_DIC]; 
} NJ_CURSOR;


typedef struct {
    NJ_UINT8 hinsi_group;          
#define NJ_HINSI_MEISI          0    
#define NJ_HINSI_JINMEI         1    
#define NJ_HINSI_MEISI_NO_CONJ  2    
#define NJ_HINSI_CHIMEI         2    
#define NJ_HINSI_KIGOU          3    

    NJ_CHAR  yomi[NJ_MAX_LEN + NJ_TERM_LEN];         
    NJ_CHAR  kouho[NJ_MAX_RESULT_LEN + NJ_TERM_LEN]; 

    
    struct {
        NJ_UINT16  yomi_len;    
        NJ_UINT16  kouho_len;   
        NJ_UINT32  hinsi;       
        NJ_UINT32  attr;        
        NJ_INT16   freq;        
    } stem;

    
    struct {
        NJ_UINT16  yomi_len;    
        NJ_UINT16  kouho_len;   
        NJ_UINT32  hinsi;       
        NJ_INT16   freq;        
    } fzk;

    NJ_INT16   connect;         

} NJ_WORD_INFO;

typedef struct {
    NJ_CHAR  *yomi;   

    
    struct NJ_STEM {
        NJ_UINT16  info1;       
        NJ_UINT16  info2;       
        NJ_HINDO   hindo;       
        NJ_SEARCH_LOCATION loc; 
        NJ_UINT8   type;        
    } stem;

    
    struct NJ_FZK {
        NJ_UINT16  info1;       
        NJ_UINT16  info2;       
        NJ_HINDO   hindo;       
    } fzk;
} NJ_WORD;

#define NJ_GET_FPOS_FROM_STEM(s) ((NJ_UINT16)((s)->stem.info1 >> 7))
#define NJ_GET_BPOS_FROM_STEM(s) ((NJ_UINT16)((s)->stem.info2 >> 7))


#define NJ_SET_FPOS_TO_STEM(s,v) ((s)->stem.info1 = ((s)->stem.info1 & 0x007F) | (NJ_UINT16)((v) << 7))
#define NJ_GET_YLEN_FROM_STEM(s) ((NJ_UINT8)((s)->stem.info1 & 0x7F))
#define NJ_GET_KLEN_FROM_STEM(s) ((NJ_UINT8)((s)->stem.info2 & 0x7F))
#define NJ_SET_YLEN_TO_STEM(s,v) ((s)->stem.info1 = ((s)->stem.info1 & 0xFF80) | (NJ_UINT16)((v) & 0x7F))
#define NJ_SET_BPOS_TO_STEM(s,v) ((s)->stem.info2 = ((s)->stem.info2 & 0x007F) | (NJ_UINT16)((v) << 7))
#define NJ_SET_KLEN_TO_STEM(s,v) ((s)->stem.info2 = ((s)->stem.info2 & 0xFF80) | (NJ_UINT16)((v) & 0x7F))

#define NJ_GET_YLEN_FROM_FZK(f) ((NJ_UINT8)((f)->fzk.info1 & 0x7F))
#define NJ_GET_BPOS_FROM_FZK(f) ((NJ_UINT16)((f)->fzk.info2 >> 7))

typedef struct {
    
    NJ_UINT16 operation_id;
#define NJ_OP_MASK          0x000f  
#define NJ_GET_RESULT_OP(id) ((id) & NJ_OP_MASK)
#define NJ_OP_SEARCH        0x0000  

#define NJ_FUNC_MASK        0x00f0  
#define NJ_GET_RESULT_FUNC(id) ((id) & NJ_FUNC_MASK)
#define NJ_FUNC_SEARCH              0x0000  

#define NJ_DIC_MASK                 0xf000  
#define NJ_GET_RESULT_DIC(id) ((id) & 0xF000)
#define NJ_DIC_STATIC               0x1000  
#define NJ_DIC_CUSTOMIZE            0x2000  
#define NJ_DIC_LEARN                0x3000  
#define NJ_DIC_USER                 0x4000  

    
    NJ_WORD word;
} NJ_RESULT;

typedef struct {
    NJ_UINT16  mode;                            
#define NJ_DEFAULT_MODE (NJ_NO_RENBUN|NJ_NO_TANBUN|NJ_RELATION_ON|NJ_YOMINASI_ON)
    NJ_UINT16  forecast_learn_limit;            
#define NJ_DEFAULT_FORECAST_LEARN_LIMIT 30      
    NJ_UINT16  forecast_limit;                  
#define NJ_DEFAULT_FORECAST_LIMIT 100           
    NJ_UINT8   char_min;                        
#define NJ_DEFAULT_CHAR_MIN 0                   
    NJ_UINT8   char_max;                        
#define NJ_DEFAULT_CHAR_MAX NJ_MAX_LEN          
} NJ_ANALYZE_OPTION;

#define NJ_STATE_MAX_FREQ  1000         
#define NJ_STATE_MIN_FREQ     0         

#include "njx_lib.h"

#define NJ_EXTERN extern

NJ_EXTERN NJ_INT16 njx_get_stroke(NJ_CLASS *iwnn, NJ_RESULT *result, NJ_CHAR  *buf, NJ_UINT16 buf_size);
NJ_EXTERN NJ_INT16 njx_get_candidate(NJ_CLASS *iwnn, NJ_RESULT *result, NJ_CHAR  *buf, NJ_UINT16 buf_size);
NJ_EXTERN NJ_INT16 njx_search_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor);
NJ_EXTERN NJ_INT16 njx_get_word(NJ_CLASS *iwnn, NJ_CURSOR *cursor, NJ_RESULT *result);
NJ_EXTERN NJ_INT16 njx_check_dic(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle, NJ_UINT8 restore, NJ_UINT32 size);
NJ_EXTERN NJ_INT16 njx_add_word(NJ_CLASS *iwnn, NJ_WORD_INFO *word, NJ_UINT8 type, NJ_UINT8 connect);
NJ_EXTERN NJ_INT16 njx_delete_word(NJ_CLASS *iwnn, NJ_RESULT *result);
NJ_EXTERN NJ_INT16 njx_create_dic(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle, NJ_INT8 type, NJ_UINT32 size);

NJ_EXTERN NJ_INT16 njx_init(NJ_CLASS *iwnn);
NJ_EXTERN NJ_INT16 njx_select(NJ_CLASS *iwnn, NJ_RESULT *r_result);

#endif 
