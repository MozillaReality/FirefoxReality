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

#ifndef _NJ_DIC_H_
#define _NJ_DIC_H_

#define NJ_DIC_TYPE_JIRITSU                     0x00000000      
#define NJ_DIC_TYPE_FZK                         0x00000001      
#define NJ_DIC_TYPE_TANKANJI                    0x00000002      
#define NJ_DIC_TYPE_CUSTOM_COMPRESS             0x00000003      
#define NJ_DIC_TYPE_STDFORE                     0x00000004      
#define NJ_DIC_TYPE_FORECONV                    0x00000005      
#define NJ_DIC_TYPE_YOMINASHI                   0x00010000      
#define NJ_DIC_TYPE_CUSTOM_INCOMPRESS           0x00020002      
#define NJ_DIC_TYPE_USER                        0x80030000      
#define NJ_DIC_TYPE_RULE                        0x000F0000      

#define NJ_HINSI_V2_F            0      
#define NJ_HINSI_SUUJI_B        14      
#define NJ_HINSI_BUNTOU_B        3      
#define NJ_HINSI_TANKANJI_F      4      
#define NJ_HINSI_TANKANJI_B      5      
#define NJ_HINSI_MEISI_F         6      
#define NJ_HINSI_MEISI_B         7      
#define NJ_HINSI_JINMEI_F        8      
#define NJ_HINSI_JINMEI_B        9      
#define NJ_HINSI_CHIMEI_F       10      
#define NJ_HINSI_CHIMEI_B       11      
#define NJ_HINSI_KIGOU_F        12      
#define NJ_HINSI_KIGOU_B        13      
#define NJ_HINSI_V1_F           15      
#define NJ_HINSI_V3_F           16      
#define NJ_RULE_TYPE_BTOF       0
#define NJ_RULE_TYPE_FTOB       1

#define NJD_SAME_INDEX_LIMIT    50

#define NJ_INT16_READ(in)                                               \
    (((((NJ_INT16)((in)[0])) << 8) & 0xff00U) + ((in)[1] & 0xffU))

#define NJ_INT32_READ(in)                                               \
    (((((NJ_INT32)((in)[0])) << 24) & 0xff000000) |                     \
     ((((NJ_INT32)((in)[1])) << 16) &   0xff0000) |                     \
     ((((NJ_INT32)((in)[2])) <<  8) &     0xff00) |                     \
     ((((NJ_INT32)((in)[3]))      ) &       0xff))

#define NJ_INT32_WRITE(to, from)\
        {(to)[0]=(NJ_UINT8)(((from)>>24) & 0x000000ff);\
         (to)[1]=(NJ_UINT8)(((from)>>16) & 0x000000ff);\
         (to)[2]=(NJ_UINT8)(((from)>>8) & 0x000000ff);\
         (to)[3]=(NJ_UINT8)((from) & 0x000000ff);}

#define NJ_INT16_WRITE(to, from)\
        {(to)[0]=(NJ_UINT8)(((from)>>8) & 0x00ff);\
         (to)[1]=(NJ_UINT8)((from) & 0x00ff);}

#define NJ_GET_MAX_YLEN(h) ((NJ_INT16)(NJ_INT16_READ((h)+0x16)/sizeof(NJ_CHAR)))

#define NJ_GET_MAX_KLEN(h) ((NJ_INT16)(NJ_INT16_READ((h)+0x1A)/sizeof(NJ_CHAR)))

#define NJ_GET_DIC_TYPE(h) ((NJ_UINT32)(NJ_INT32_READ((h)+8)))

#define F_HINSI_SET_CNT(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x1C)))
#define B_HINSI_SET_CNT(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x1E)))


#endif 
