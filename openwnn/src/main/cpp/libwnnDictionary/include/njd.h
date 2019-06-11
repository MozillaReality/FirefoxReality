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

#ifndef _NJD_H_
#define _NJD_H_

#define NJ_ST_SEARCH_NO_INIT    1       
#define NJ_ST_SEARCH_READY      2       
#define NJ_ST_SEARCH_END        3       
#define NJ_ST_SEARCH_END_EXT    4       

#define NJ_DIC_FMT_KANAKAN              0x0     

#define NJ_DIC_ID_LEN                   (4)             
#define NJ_DIC_IDENTIFIER               0x4e4a4443      
#define NJ_DIC_COMMON_HEADER_SIZE       0x001C          
#define NJ_DIC_POS_DATA_SIZE            0x0c            
#define NJ_DIC_POS_EXT_SIZE             0x10            
#define NJ_DIC_VERSION1                 0x00010000      
#define NJ_DIC_VERSION2                 0x00020000      
#define NJ_DIC_VERSION3                 0x00030000      
#define NJ_DIC_VERSION2_1               0x00020001      

#define ADD_WORD_DIC_TYPE_USER  0       
#define ADD_WORD_DIC_TYPE_LEARN 1       

#define DIC_FREQ_BASE 0     
#define DIC_FREQ_HIGH 1000  

#define LEARN_DIC_QUE_SIZE        32
#define LEARN_DIC_EXT_QUE_SIZE    6


#define NJ_GET_DIC_VER(h) NJ_INT32_READ((h)+4)


#define GET_LOCATION_STATUS(x) ((NJ_UINT8)((x)&0x0f))

#define GET_LOCATION_OPERATION(x) ((NJ_UINT8)(((x) >> 4)&0x0f))

#define SET_LOCATION_OPERATION(ope) ((NJ_UINT16)((ope) << 4))


#define NJ_GET_DIC_FMT(h) ((NJ_UINT8)((*((h)+0x1C)) & 0x03))


#define CALCULATE_HINDO(freq, base, high, div) \
    ((NJ_HINDO)((((freq) * ((high) - (base))) / (div)) + (base)))

#define NORMALIZE_HINDO(freq, max, min) \
    (((freq) < (min)) ? (min) : (((freq) > (max)) ? (max) : (freq)))


#endif 
