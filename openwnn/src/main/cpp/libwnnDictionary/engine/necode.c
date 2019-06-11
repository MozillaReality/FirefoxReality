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
#include "nj_ext.h"


#define HIRA_KATA_OFFSET (0x0060)    

#define ZEN_CHAR_LEN  1  

#define CHAR_TO_WCHAR(s)                                        \
    ((NJ_UINT16)( (((NJ_UINT8*)(s))[0] << 8) | ((NJ_UINT8*)(s))[1] ))

#define SET_WCHAR_TO_CHAR(s, c)                                \
    {                                                          \
        ((NJ_UINT8*)(s))[0] = (NJ_UINT8)(((c) >> 8) & 0x00ff); \
        ((NJ_UINT8*)(s))[1] = (NJ_UINT8)(((c))      & 0x00ff); \
    }

#define IS_HIRAGANA_WCHAR(c)  ( ((c) >= 0x3041) && ((c) <= 0x3093) )




NJ_INT16 nje_convert_hira_to_kata(NJ_CHAR *hira, NJ_CHAR *kata, NJ_UINT16 len)
{
    NJ_UINT16 pnt;
    NJ_UINT16 wchar;


    pnt = 0;
    while (pnt < len) {
        if (*hira == NJ_CHAR_NUL) {
            
            return pnt;
        }

        
        wchar = CHAR_TO_WCHAR(hira);
        hira++;
        pnt++;

        if (IS_HIRAGANA_WCHAR(wchar)) {
            
            SET_WCHAR_TO_CHAR(kata, wchar + HIRA_KATA_OFFSET);
            kata += ZEN_CHAR_LEN;
        } else {
            SET_WCHAR_TO_CHAR(kata, wchar);
            kata += ZEN_CHAR_LEN;
        }
    }

    
    *kata = NJ_CHAR_NUL;
    return pnt;
}
