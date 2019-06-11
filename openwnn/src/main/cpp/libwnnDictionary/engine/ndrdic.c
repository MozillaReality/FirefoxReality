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


#define F_HINSI_TOP_ADDR(h) ((NJ_UINT8*)((h)+NJ_INT32_READ((h)+0x20)))
#define B_HINSI_TOP_ADDR(h) ((NJ_UINT8*)((h)+NJ_INT32_READ((h)+0x24)))
#define V2_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x28)))
#define BUN_B_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x2A)))
#define TAN_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x30)))
#define TAN_B_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x32)))
#define SUUJI_B_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x34)))
#define MEISI_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x36)))
#define MEISI_B_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x38)))
#define JINMEI_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x3A)))
#define JINMEI_B_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x3C)))
#define CHIMEI_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x3E)))
#define CHIMEI_B_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x40)))
#define KIGOU_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x42)))
#define KIGOU_B_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x44)))
#define V1_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x52)))
#define V3_F_HINSI(h) ((NJ_UINT16)(NJ_INT16_READ((h)+0x54)))

NJ_INT16 njd_r_get_hinsi(NJ_DIC_HANDLE rule, NJ_UINT8 type) {

    
    if (rule == NULL) {
        return 0; 
    }

    switch (type) {
    case NJ_HINSI_V2_F :        
        return V2_F_HINSI(rule);
    case NJ_HINSI_BUNTOU_B :    
        return BUN_B_HINSI(rule);
    case NJ_HINSI_TANKANJI_F :  
        return TAN_F_HINSI(rule);
    case NJ_HINSI_TANKANJI_B :  
        return TAN_B_HINSI(rule);
    case NJ_HINSI_SUUJI_B:      
        return SUUJI_B_HINSI(rule);
    case NJ_HINSI_MEISI_F :     
        return MEISI_F_HINSI(rule);
    case NJ_HINSI_MEISI_B :     
        return MEISI_B_HINSI(rule);
    case NJ_HINSI_JINMEI_F :    
        return JINMEI_F_HINSI(rule);
    case NJ_HINSI_JINMEI_B :    
        return JINMEI_B_HINSI(rule);
    case NJ_HINSI_CHIMEI_F :    
        return CHIMEI_F_HINSI(rule);
    case NJ_HINSI_CHIMEI_B :    
        return CHIMEI_B_HINSI(rule);
    case NJ_HINSI_KIGOU_F :     
        return KIGOU_F_HINSI(rule);
    case NJ_HINSI_KIGOU_B :     
        return KIGOU_B_HINSI(rule);
    case NJ_HINSI_V1_F :        
        return V1_F_HINSI(rule);
    case NJ_HINSI_V3_F :        
        return V3_F_HINSI(rule);    default:
    
        return 0; 
    }
}

NJ_INT16 njd_r_get_connect(NJ_DIC_HANDLE rule, NJ_UINT16 hinsi, NJ_UINT8 type, NJ_UINT8 **connect) {
    NJ_UINT16 i, rec_len;

    
    if (rule == NULL) {
        return 0; 
    }
    if (hinsi < 1) {
        return 0;
    }

    if (type == NJ_RULE_TYPE_BTOF) {    
        i = F_HINSI_SET_CNT(rule);      
        rec_len = (NJ_UINT16)((i + 7) / 8);
                                        
        *connect = (NJ_UINT8*)(F_HINSI_TOP_ADDR(rule) + ((hinsi - 1) * rec_len));
    } else {                            
        i = B_HINSI_SET_CNT(rule);      
        rec_len = (NJ_UINT16)((i + 7) / 8);
                                        
        *connect = (NJ_UINT8*)(B_HINSI_TOP_ADDR(rule) + ((hinsi - 1) * rec_len));
    }
    return 0;
}

NJ_INT16 njd_r_get_count(NJ_DIC_HANDLE rule, NJ_UINT16 *fcount, NJ_UINT16 *rcount) {

    
    if (rule == NULL) {
        return 0; 
    }

    *fcount = F_HINSI_SET_CNT(rule);
    *rcount = B_HINSI_SET_CNT(rule);

    return 0;
}
