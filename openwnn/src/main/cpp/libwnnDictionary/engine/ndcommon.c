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





NJ_INT16 njd_connect_test(NJ_SEARCH_CONDITION *con, NJ_UINT16 hinsiF, NJ_UINT16 hinsiR)
{

    
    if (con->hinsi.fore != NULL) {
        if (hinsiF == 0) {
            return 0; 
        }

        hinsiF--;
        if (hinsiF >= con->hinsi.foreSize) {
            return 0; 
        }
        if (*(con->hinsi.fore + (hinsiF / 8)) & (0x80 >> (hinsiF % 8))) {
            
            if (con->hinsi.foreFlag != 0) {
                
                return 0; 
            }
        } else {
            
            if (con->hinsi.foreFlag == 0) {
                
                return 0;
            }
        }
    }

    
    if (con->hinsi.rear != NULL) {
        if (hinsiR == 0) {
            return 0; 
        }

        hinsiR--;
        if (hinsiR >= con->hinsi.rearSize) {
            return 0; 
        }
        if (*(con->hinsi.rear + (hinsiR / 8)) & (0x80 >> (hinsiR % 8))) {
            
            if (con->hinsi.rearFlag != 0) {
                
                return 0; 
            }
        } else {
            
            if (con->hinsi.rearFlag == 0) {
                
                return 0;
            }
        }
    }
    
    return 1;
}
