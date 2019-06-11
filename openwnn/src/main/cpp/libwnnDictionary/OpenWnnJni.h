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
#ifndef _OPENWNNJNI_H
#define _OPENWNNJNI_H

/**
 * Error codes
 */
#define NJ_FUNC_JNI_CONVERT_STR_TO_NJC				        (0x00FE)
#define NJ_FUNC_JNI_CONVERT_NJC_TO_STR				        (0x00FD)
#define NJ_FUNC_JNI_FREE_WNNWORK					        (0x00FC)
#define NJ_FUNC_JNI_CLEAR_DICTIONARY_PARAMETERS		        (0x00FB)
#define NJ_FUNC_JNI_SET_DICTIONARY_PARAMETERS		        (0x00FA)
#define NJ_FUNC_JNI_SEARCH_WORD						        (0x00F9)
#define NJ_FUNC_JNI_GET_WORD						        (0x00F8)
#define NJ_FUNC_JNI_GET_FREQUENCY					        (0x00F7)
#define NJ_FUNC_JNI_SET_APPROX_PATTERN				        (0x00F6)
#define NJ_FUNC_JNI_GET_LEFT_PART_OF_SPEECH	       	        (0x00F5)
#define NJ_FUNC_JNI_GET_RIGHT_PART_OF_SPEECH      	        (0x00F4)
#define NJ_FUNC_JNI_SET_LEFT_PART_OF_SPEECH	       	        (0x00F3)
#define NJ_FUNC_JNI_SET_RIGHT_PART_OF_SPEECH      	        (0x00F2)
#define NJ_FUNC_JNI_SET_STROKE                              (0x00F1)
#define NJ_FUNC_JNI_SET_CANDIDATE                           (0x00F0)
#define NJ_FUNC_JNI_SELECT_WORD             		        (0x00EF)
#define NJ_FUNC_JNI_GET_LEFT_PART_OF_SPEECH_SPECIFIED_TYPE  (0x00EE)
#define NJ_FUNC_JNI_GET_RIGHT_PART_OF_SPEECH_SPECIFIED_TYPE (0x00ED)
#define NJ_FUNC_JNI_GET_NUMBER_OF_LEFT_POS                  (0x00EC)
#define NJ_FUNC_JNI_GET_NUMBER_OF_RIGHT_POS                 (0x00EB)

#define NJ_ERR_JNI_FUNC_FAILED						        (0x7E00)
#define NJ_ERR_ALLOC_FAILED							        (0x7D00)
#define NJ_ERR_NOT_ALLOCATED						        (0x7C00)
#define NJ_ERR_INVALID_PARAM						        (0x7B00)
#define NJ_ERR_APPROX_PATTERN_IS_FULL				        (0x7A00)

/**
 * Structure of internal work area
 */
#define NJ_MAX_CHARSET_FROM_LEN                     1
#define NJ_MAX_CHARSET_TO_LEN                       3
#define NJ_APPROXSTORE_SIZE                         (NJ_MAX_CHARSET_FROM_LEN + NJ_TERM_LEN + NJ_MAX_CHARSET_TO_LEN + NJ_TERM_LEN)


#define NJ_JNI_FLAG_NONE                            (0x00)
#define NJ_JNI_FLAG_ENABLE_CURSOR                   (0x01)
#define NJ_JNI_FLAG_ENABLE_RESULT                   (0x02)

typedef struct {
	void*				dicLibHandle;
	NJ_DIC_HANDLE		dicHandle[ NJ_MAX_DIC ];
	NJ_UINT32			dicSize[ NJ_MAX_DIC ];
	NJ_UINT8			dicType[ NJ_MAX_DIC ];
	NJ_CHAR				keyString[ NJ_MAX_LEN + NJ_TERM_LEN ];
	NJ_RESULT			result;
	NJ_CURSOR			cursor;
	NJ_SEARCH_CACHE		srhCache[ NJ_MAX_DIC ];
	NJ_DIC_SET			dicSet;
	NJ_CLASS			wnnClass;
	NJ_CHARSET			approxSet;
	NJ_CHAR				approxStr[ NJ_MAX_CHARSET * NJ_APPROXSTORE_SIZE ];
    NJ_CHAR             previousStroke[ NJ_MAX_LEN + NJ_TERM_LEN ];
    NJ_CHAR             previousCandidate[ NJ_MAX_RESULT_LEN + NJ_TERM_LEN ];
    NJ_UINT8            flag;
} NJ_JNIWORK;

/**
 * Predefined approx patterns
 */
typedef struct {
    int         size;
    NJ_UINT8*   from;
    NJ_UINT8*   to;
} PREDEF_APPROX_PATTERN;

#endif /* _OPENWNNJNI_H */
