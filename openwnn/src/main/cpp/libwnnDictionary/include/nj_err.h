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

#ifndef _NJ_ERR_H_
#define _NJ_ERR_H_

#define NJ_ERR_CODE_MASK        (0x7F00)        
#define NJ_ERR_FUNC_MASK        (0x00FF)        

#define NJ_GET_ERR_CODE(x)      ((x) & NJ_ERR_CODE_MASK)
#define NJ_GET_ERR_FUNC(x)      ((x) & NJ_ERR_FUNC_MASK)

#define NJ_SET_ERR_VAL(x, y)    ((NJ_INT16)((x) | (y) | 0x8000))

#define NJ_ERR_PARAM_DIC_NULL                       (0x0000) 
#define NJ_ERR_PARAM_YOMI_NULL                      (0x0100) 
#define NJ_ERR_PARAM_YOMI_SIZE                      (0x0200) 
#define NJ_ERR_PARAM_RESULT_NULL                    (0x0500) 
#define NJ_ERR_YOMI_TOO_LONG                        (0x0600) 
#define NJ_ERR_NO_RULEDIC                           (0x0800) 
#define NJ_ERR_PARAM_OPERATION                      (0x0900) 
#define NJ_ERR_PARAM_MODE                           (0x0A00) 
#define NJ_ERR_PARAM_KANJI_NULL                     (0x0B00) 
#define NJ_ERR_CANDIDATE_TOO_LONG                   (0x0C00) 
#define NJ_ERR_PARAM_CURSOR_NULL                    (0x0D00) 
#define NJ_ERR_DIC_TYPE_INVALID                     (0x0E00) 
#define NJ_ERR_DIC_HANDLE_NULL                      (0x0F00) 
#define NJ_ERR_FORMAT_INVALID                       (0x1000) 
#define NJ_ERR_NO_CANDIDATE_LIST                    (0x1100) 
#define NJ_ERR_AREASIZE_INVALID                     (0x1300) 
#define NJ_ERR_BUFFER_NOT_ENOUGH                    (0x1400) 
#define NJ_ERR_HINSI_GROUP_INVALID                  (0x1500) 
#define NJ_ERR_CREATE_TYPE_INVALID                  (0x1600) 
#define NJ_ERR_WORD_INFO_NULL                       (0x1700) 
#define NJ_ERR_DIC_NOT_FOUND                        (0x1800) 
#define NJ_ERR_CANNOT_GET_QUE                       (0x1900) 
#define NJ_ERR_INVALID_FLAG                         (0x1A00) 
#define NJ_ERR_INVALID_RESULT                       (0x1B00) 
#define NJ_ERR_INTERNAL                             (0x1D00) 
#define NJ_ERR_USER_YOMI_INVALID                    (0x1E00) 
#define NJ_ERR_USER_KOUHO_INVALID                   (0x1F00) 
#define NJ_ERR_USER_DIC_FULL                        (0x2000) 
#define NJ_ERR_SAME_WORD                            (0x2100) 
#define NJ_ERR_DIC_BROKEN                           (0x2200) 
#define NJ_ERR_WORD_NOT_FOUND                       (0x2400) 
#define NJ_ERR_DIC_VERSION_INVALID                  (0x2A00) 
#define NJ_ERR_DIC_FREQ_INVALID                     (0x2B00) 
#define NJ_ERR_CACHE_NOT_ENOUGH                     (0x2C00) 
#define NJ_ERR_CACHE_BROKEN                         (0x2D00) 
#define NJ_ERR_PARAM_ENV_NULL                       (0x2E00) 
#define NJ_ERR_PARAM_ILLEGAL_CHAR_LEN               (0x3200) 


#define NJ_FUNC_NJD_B_GET_CANDIDATE                 (0x0010)
#define NJ_FUNC_NJD_B_GET_STROKE                    (0x0061)
#define NJ_FUNC_NJD_B_SEARCH_WORD                   (0x0062)

#define NJ_FUNC_NJD_F_GET_WORD                      (0x0011)
#define NJ_FUNC_NJD_F_GET_STROKE                    (0x0012)
#define NJ_FUNC_NJD_F_GET_CANDIDATE                 (0x0013)
#define NJ_FUNC_NJD_L_DELETE_WORD                   (0x0014)
#define NJ_FUNC_NJD_L_ADD_WORD                      (0x0015)
#define NJ_FUNC_NJD_L_UNDO_LEARN                    (0x0016)
#define NJ_FUNC_DELETE_INDEX                        (0x0017)
#define NJ_FUNC_INSERT_INDEX                        (0x0018)
#define NJ_FUNC_QUE_STRCMP_COMPLETE_WITH_HYOUKI     (0x0019)
#define NJ_FUNC_NJD_L_GET_WORD                      (0x001B)
#define NJ_FUNC_NJD_L_GET_CANDIDATE                 (0x001C)
#define NJ_FUNC_NJD_L_GET_STROKE                    (0x001D)
#define NJ_FUNC_QUE_STRCMP_FORWARD                  (0x001E)
#define NJ_FUNC_NJD_L_CHECK_DIC                     (0x001F)
#define NJ_FUNC_SEARCH_RANGE_BY_YOMI                (0x0020)
#define NJ_FUNC_STR_QUE_CMP                         (0x0021)
#define NJ_FUNC_WRITE_LEARN_DATA                    (0x0022)
#define NJ_FUNC_NJD_R_CHECK_GROUP                   (0x0064)

#define NJ_FUNC_CHECK_SEARCH_CURSOR                 (0x0023)
#define NJ_FUNC_GET_WORD_AND_SEARCH_NEXT_WORD       (0x0024)

#define NJ_FUNC_NJD_GET_WORD_DATA                   (0x0025)
#define NJ_FUNC_NJD_GET_WORD                        (0x0027)
#define NJ_FUNC_NJD_CHECK_DIC                       (0x0028)

#define NJ_FUNC_NJ_CREATE_DIC                       (0x0029)
#define NJ_FUNC_NJD_GET_STROKE                      (0x002A)
#define NJ_FUNC_NJD_GET_CANDIDATE                   (0x002B)
#define NJ_FUNC_NJ_SEARCH_WORD                      (0x002C)
#define NJ_FUNC_NJ_GET_WORD                         (0x002D)
#define NJ_FUNC_NJ_ADD_WORD                         (0x002E)
#define NJ_FUNC_NJ_DELETE_WORD                      (0x002F)
#define NJ_FUNC_NJ_CHECK_DIC                        (0x0030)
#define NJ_FUNC_NJD_L_MAKE_SPACE                    (0x0053)
#define NJ_FUNC_SEARCH_RANGE_BY_YOMI_MULTI          (0x0054)
#define NJ_FUNC_NJD_L_GET_RELATIONAL_WORD           (0x0055)
#define NJ_FUNC_QUE_STRCMP_INCLUDE                  (0x0056)
#define NJ_FUNC_IS_CONTINUED                        (0x0057)
#define NJ_FUNC_CONTINUE_CNT                        (0x0058)

#define NJ_FUNC_SEARCH_WORD                         (0x003C)

#define NJ_FUNC_NJ_SELECT                           (0x0040)
#define NJ_FUNC_NJ_INIT                             (0x0041)
#define NJ_FUNC_NJ_GET_CANDIDATE                    (0x0042)
#define NJ_FUNC_NJ_GET_STROKE                       (0x0043)

#define NJ_FUNC_NJ_MANAGE_LEARNDIC                  (0x0093)

#endif 
