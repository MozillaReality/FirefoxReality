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
#include "jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni.h"

#include "nj_lib.h"
#include "nj_err.h"
#include "nj_ext.h"
#include "nj_dic.h"


#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "OpenWnnJni.h"


#include "predef_table.h"

/**
 * functions for internal use
 */
static void clearDictionaryStructure( NJ_DIC_INFO* dicInfo ) {
	dicInfo->type		= 0;
	dicInfo->handle		= NULL;
/*	dicInfo->srhCache	= NULL; */

	dicInfo->dic_freq[ NJ_MODE_TYPE_HENKAN ].base = 0;
	dicInfo->dic_freq[ NJ_MODE_TYPE_HENKAN ].high = 0;
}

static NJ_CHAR convertUTFCharToNjChar( NJ_UINT8* src )
{
    NJ_CHAR     ret;
    NJ_UINT8*   dst;

    /* convert UTF-16BE character to NJ_CHAR format */
    dst = ( NJ_UINT8* )&ret;
    dst[ 0 ] = src[ 0 ];
    dst[ 1 ] = src[ 1 ];

    return ret;
}

static int convertStringToNjChar( JNIEnv *env, NJ_CHAR* dst, jstring srcJ, int maxChars )
{
	const unsigned char*	src;

	src = ( const unsigned char* )( ( *env )->GetStringUTFChars( env, srcJ, NULL ) );
	if( src != NULL ) {
		int     i, o;

        /* convert UTF-8 to UTF-16BE */
        for( i = o = 0 ; src[ i ] != 0x00 && o < maxChars ; ) {
			NJ_UINT8* dst_tmp;
			dst_tmp = ( NJ_UINT8* )&( dst[ o ] );

            if( ( src[ i ] & 0x80 ) == 0x00 ) {
                /* U+0000 ... U+007f */
                /* 8[0xxxxxxx] -> 16BE[00000000 0xxxxxxx] */
                dst_tmp[ 0 ] = 0x00;
                dst_tmp[ 1 ] = src[ i + 0 ] & 0x7f;
                i++;
                o++;
            } else if( ( src[ i ] & 0xe0 ) == 0xc0 ) {
                /* U+0080 ... U+07ff */
                /* 8[110xxxxx 10yyyyyy] -> 16BE[00000xxx xxyyyyyy] */
                if( src[ i + 1 ] == 0x00 ) {
                    break;
                }
                dst_tmp[ 0 ] = ( ( src[ i + 0 ] & 0x1f ) >> 2 );
                dst_tmp[ 1 ] = ( ( src[ i + 0 ] & 0x1f ) << 6 ) |   ( src[ i + 1 ] & 0x3f );
                i += 2;
                o++;
            } else if( ( src[ i ] & 0xf0 ) == 0xe0 ) {
                /* U+0800 ... U+ffff */
                /* 8[1110xxxx 10yyyyyy 10zzzzzz] -> 16BE[xxxxyyyy yyzzzzzz] */
                if( src[ i + 1 ] == 0x00 || src[ i + 2 ] == 0x00 ) {
                    break;
                }
                dst_tmp[ 0 ] = ( ( src[ i + 0 ] & 0x0f ) << 4 ) | ( ( src[ i + 1 ] & 0x3f ) >> 2 );
                dst_tmp[ 1 ] = ( ( src[ i + 1 ] & 0x3f ) << 6 ) |   ( src[ i + 2 ] & 0x3f );
                i += 3;
                o++;
            } else if( ( src[ i ] & 0xf8 ) == 0xf0 ) {
                NJ_UINT8    dst1, dst2, dst3;
                /* U+10000 ... U+10ffff */
                /* 8[11110www 10xxxxxx 10yyyyyy 10zzzzzz] -> 32BE[00000000 000wwwxx xxxxyyyy yyzzzzzz] */
                /*                                        -> 16BE[110110WW XXxxxxyy 110111yy yyzzzzzz] */
                /*                                                      -- --======       == --------  */
                /*                                                      dst1   dst2          dst3      */
                /*                                        "wwwxx"(00001-10000) - 1 = "WWXX"(0000-1111) */
                if( !( o < maxChars - 1 ) ) {
                    /* output buffer is full */
                    break;
                }
                if( src[ i + 1 ] == 0x00 || src[ i + 2 ] == 0x00 || src[ i + 3 ] == 0x00 ) {
                    break;
                }
                dst1 = ( ( ( src[ i + 0 ] & 0x07 ) << 2 ) | ( ( src[ i + 1 ] & 0x3f ) >> 4 ) ) - 1;
                dst2 =   ( ( src[ i + 1 ] & 0x3f ) << 4 ) | ( ( src[ i + 2 ] & 0x3f ) >> 2 );
                dst3 =   ( ( src[ i + 2 ] & 0x3f ) << 6 ) |   ( src[ i + 3 ] & 0x3f );

                dst_tmp[ 0 ] = 0xd8 | ( ( dst1 & 0x0c ) >> 2 );
                dst_tmp[ 1 ] =        ( ( dst1 & 0x03 ) << 6 ) | ( ( dst2 & 0xfc ) >> 2 );
                dst_tmp[ 2 ] = 0xdc |                            ( ( dst2 & 0x03 ) );
                dst_tmp[ 3 ] =                                                              dst3;
                i += 4;
                o += 2;
            } else {    /* Broken code */
                break;
            }
        }
        dst[ o ] = NJ_CHAR_NUL;

		( *env )->ReleaseStringUTFChars( env, srcJ, ( const char* )src );
		return 0;
	}
	/* If retrieveing the string failed, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_CONVERT_STR_TO_NJC, NJ_ERR_JNI_FUNC_FAILED);
}

static int convertNjCharToString( JNIEnv* env, jstring* dstJ, NJ_CHAR* src, int maxChars )
{
    char        dst[ (NJ_MAX_LEN + NJ_MAX_RESULT_LEN + NJ_TERM_LEN ) * 3 + 1 ];

	int		i, o;

	/* convert UTF-16BE to a UTF-8 */
	for( i = o = 0 ; src[ i ] != 0x0000 && i < maxChars ; ) {
		NJ_UINT8* src_tmp;
		src_tmp = ( NJ_UINT8* )&( src[ i ] );

        if( src_tmp[ 0 ] == 0x00 && src_tmp[ 1 ] <= 0x7f ) {
            /* U+0000 ... U+007f */
            /* 16BE[00000000 0xxxxxxx] -> 8[0xxxxxxx] */
            dst[ o + 0 ] = src_tmp[ 1 ] & 0x007f;
            i++;
            o++;
        } else if ( src_tmp[ 0 ] <= 0x07 ) {
            /* U+0080 ... U+07ff */
            /* 16BE[00000xxx xxyyyyyy] -> 8[110xxxxx 10yyyyyy] */
            dst[ o + 0 ] = 0xc0 | ( ( src_tmp[ 0 ] & 0x07 ) << 2 ) | ( ( src_tmp[ 1 ] & 0xc0 ) >> 6 );
            dst[ o + 1 ] = 0x80 |                                    ( ( src_tmp[ 1 ] & 0x3f ) );
            i++;
            o += 2;
        } else if ( src_tmp[ 0 ] >= 0xd8 && src_tmp[ 0 ] <= 0xdb ) {
            NJ_UINT8    src1, src2, src3;
            /* U+10000 ... U+10ffff (surrogate pair) */
            /* 32BE[00000000 000wwwxx xxxxyyyy yyzzzzzz] -> 8[11110www 10xxxxxx 10yyyyyy 10zzzzzz] */
            /* 16BE[110110WW XXxxxxyy 110111yy yyzzzzzz]                                           */
            /*            -- --======       == --------                                            */
            /*            src1 src2            src3                                                */
            /* "WWXX"(0000-1111) + 1 = "wwwxx"(0001-10000)                                         */
            if( !( i < maxChars - 1 ) || src_tmp[ 2 ] < 0xdc || src_tmp[ 2 ] > 0xdf ) {
                /* That is broken code */
                break;
            }
            src1 = ( ( ( src_tmp[ 0 ] & 0x03 ) << 2 ) | ( ( src_tmp[ 1 ] & 0xc0 ) >> 6 )                               ) + 1;
            src2 =                                      ( ( src_tmp[ 1 ] & 0x3f ) << 2 ) | ( ( src_tmp[ 2 ] & 0x03 ) );
            src3 =                                                                             src_tmp[ 3 ];

            dst[ o + 0 ] = 0xf0 | ( ( src1 & 0x1c ) >> 2 );
            dst[ o + 1 ] = 0x80 | ( ( src1 & 0x03 ) << 4 ) | ( ( src2 & 0xf0 ) >> 4 );
            dst[ o + 2 ] = 0x80 |                            ( ( src2 & 0x0f ) << 2 ) | ( ( src3 & 0xc0 ) >> 6 );
            dst[ o + 3 ] = 0x80 |                                                         ( src3 & 0x3f );
            i += 2;
            o += 4;
        } else {
            /* U+0800 ... U+ffff (except range of surrogate pair) */
            /* 16BE[xxxxyyyy yyzzzzzz] -> 8[1110xxxx 10yyyyyy 10zzzzzz] */
            dst[ o + 0 ] = 0xe0 | ( ( src_tmp[ 0 ] & 0xf0 ) >> 4 );
            dst[ o + 1 ] = 0x80 | ( ( src_tmp[ 0 ] & 0x0f ) << 2 ) | ( ( src_tmp[ 1 ] & 0xc0 ) >> 6  );
            dst[ o + 2 ] = 0x80 |                                    ( ( src_tmp[ 1 ] & 0x3f ) );
            i++;
            o += 3;
        }
	}
	dst[ o ] = 0x00;

	*dstJ = ( *env )->NewStringUTF( env, dst );

	/* If NewString() failed, return an error code */
	return ( *dstJ == NULL ) ? NJ_SET_ERR_VAL(NJ_FUNC_JNI_CONVERT_NJC_TO_STR, NJ_ERR_JNI_FUNC_FAILED) : 0;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    createWnnWork
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_createWnnWork
  (JNIEnv *env, jobject obj, jstring dicLibPathJ)
{
	NJ_JNIWORK*		work;

	/* Allocating the internal work area */
	work = ( NJ_JNIWORK* )malloc( sizeof( NJ_JNIWORK ) );
	if( work != NULL ) {
		NJ_UINT32*		dic_size;
		NJ_UINT8*		dic_type;
        NJ_UINT8**      dic_data;
        NJ_UINT8**      con_data;
		const char*		dicLibPath;
		NJ_INT16		result;
		int				i;

		/* Initialize the work area */
		memset( work, 0x00, sizeof( NJ_JNIWORK ) );

		/* Load the dictionary library which is specified by dicLibPathJ */
		if( dicLibPathJ == NULL ||
			( dicLibPath = ( *env )->GetStringUTFChars( env, dicLibPathJ, 0 ) ) == NULL ) {
			free( work );
			return 0;
		}

		work->dicLibHandle = ( void* )dlopen( dicLibPath, RTLD_LAZY );
		( *env )->ReleaseStringUTFChars( env, dicLibPathJ, dicLibPath );

		if( work->dicLibHandle == NULL ) {
			free( work );
			return 0;
		}

		/* Retrieve data pointers of dictionary from the dictionary library, and put to internal work area */
		dic_size = ( NJ_UINT32* )dlsym( work->dicLibHandle, "dic_size" );
		dic_type = ( NJ_UINT8* )dlsym( work->dicLibHandle, "dic_type" );
        dic_data = ( NJ_UINT8** )dlsym( work->dicLibHandle, "dic_data" );
		if( dic_size == NULL || dic_type == NULL || dic_data == NULL ) {
			dlclose( work->dicLibHandle );
			free( work );
			return 0;
		}

		for( i = 0 ; i < NJ_MAX_DIC ; i++ ) {
            work->dicHandle[ i ]    = dic_data[ i ];
			work->dicSize[ i ]      = dic_size[ i ];
			work->dicType[ i ]      = dic_type[ i ];
		}

        /* Set the rule dictionary if the rule data exist */
        con_data = ( NJ_UINT8** )dlsym( work->dicLibHandle, "con_data" );
        if( con_data != NULL ) {
            work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ] = con_data[ 0 ];
        }

		/* Execute the initialize method to initialize the internal work area */
		result = njx_init( &( work->wnnClass ) );

		if( result >= 0 ) {
            jlong   jresult;

            *( NJ_JNIWORK** )&jresult = work;
			return jresult;
		}

		/* If allocating a byte array failed, free all resource, and return NULL */
		dlclose( work->dicLibHandle );
		free( work );
	}
	/* If allocating the internal work area failed, return NULL */
	return 0;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    freeWnnWork
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_freeWnnWork
  (JNIEnv *env, jobject obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
		/* If the internal work area was not yet released, remove that */ 
        if( work->dicLibHandle != NULL ) {
        	dlclose( work->dicLibHandle );
            work->dicLibHandle = NULL;
        }
		free( work );

		return 0;
	}

	/* freeWnnWork() is always successful even if the internal work area was already released */
	return 0;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    clearDictionaryParameters
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_clearDictionaryParameters
  (JNIEnv *env, jobject obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
		int				index;

		/* Clear all dictionary set information structure and reset search state */
		for( index = 0 ; index < NJ_MAX_DIC ; index++ ) {
    		clearDictionaryStructure( &( work->dicSet.dic[ index ] ) );
		}
        work->flag = NJ_JNI_FLAG_NONE;

        /* Clear the cache information */
        memset( work->dicSet.keyword, 0x00, sizeof( work->dicSet.keyword ) );

		return 0;
	}

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_CLEAR_DICTIONARY_PARAMETERS, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    setDictionaryParameter
 * Signature: (JIII)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_setDictionaryParameter
  (JNIEnv *env, jobject obj, jlong wnnWork, jint index, jint base, jint high)
{
	NJ_JNIWORK*	work;

	if( ( index < 0  || index > NJ_MAX_DIC-1 ) ||
		( base <  -1 || base > 1000 ) ||
		( high <  -1 || high > 1000 ) ) {
		/* If a invalid parameter was specified, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_DICTIONARY_PARAMETERS, NJ_ERR_INVALID_PARAM);
	}

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
		/* Create the dictionary set information structure */
		if( base < 0 || high < 0 || base > high ) {
			/* If -1 was specified to base or high, clear that dictionary information structure */
            /* If base is larger than high, clear that dictionary information structure */
    		clearDictionaryStructure( &( work->dicSet.dic[ index ] ) );
		} else {
			/* Set the dictionary informatin structure */
    		work->dicSet.dic[ index ].type		= work->dicType[ index ];
    		work->dicSet.dic[ index ].handle	= work->dicHandle[ index ];
    		work->dicSet.dic[ index ].srhCache	= &( work->srhCache[ index ] );

    		work->dicSet.dic[ index ].dic_freq[ NJ_MODE_TYPE_HENKAN ].base = base;
    		work->dicSet.dic[ index ].dic_freq[ NJ_MODE_TYPE_HENKAN ].high = high;
		}

        /* Reset search state because the dicionary information was changed */
        work->flag = NJ_JNI_FLAG_NONE;

		return 0;
	}

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_DICTIONARY_PARAMETERS, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    searchWord
 * Signature: (JIILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_searchWord
  (JNIEnv *env, jobject obj, jlong wnnWork, jint operation, jint order, jstring keyString)
{
	NJ_JNIWORK*	work;

	if( !( operation == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_SEARCH_EXACT ||
           operation == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_SEARCH_PREFIX ||
           operation == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_SEARCH_LINK ) ||
		!( order == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_ORDER_BY_FREQUENCY ||
           order == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_ORDER_BY_KEY ) ||
		   keyString == NULL ) {
		/* If a invalid parameter was specified, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SEARCH_WORD, NJ_ERR_INVALID_PARAM);
	}

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        if( ( *env )->GetStringLength( env, keyString ) > NJ_MAX_LEN ) {
            /* If too long key string was specified, return "No result is found" */
            work->flag &= ~NJ_JNI_FLAG_ENABLE_CURSOR;
            work->flag &= ~NJ_JNI_FLAG_ENABLE_RESULT;
            return 0;
        }

		if( convertStringToNjChar( env, work->keyString, keyString, NJ_MAX_LEN ) >= 0 ) {
            jint    result;

			/* Set the structure for search */
			memset( &( work->cursor ), 0x00, sizeof( NJ_CURSOR ) );
			work->cursor.cond.operation	= operation;
			work->cursor.cond.mode		= order;
			work->cursor.cond.ds		= &( work->dicSet );
			work->cursor.cond.yomi		= work->keyString;
   			work->cursor.cond.charset	= &( work->approxSet );

            /* If the link search feature is specified, set the predict search information to structure */
            if( operation == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_SEARCH_LINK ) {
                work->cursor.cond.yomi  = work->previousStroke;
                work->cursor.cond.kanji = work->previousCandidate;
            }

			/* Search a specified word */
            memcpy( &( work->wnnClass.dic_set ), &( work->dicSet ), sizeof( NJ_DIC_SET ) );
			result = ( jint )njx_search_word( &( work->wnnClass ), &( work->cursor ) );

            /* If a result is found, enable getNextWord method */
            if( result == 1 ) {
                work->flag |= NJ_JNI_FLAG_ENABLE_CURSOR;
            } else {
                work->flag &= ~NJ_JNI_FLAG_ENABLE_CURSOR;
            }
            work->flag &= ~NJ_JNI_FLAG_ENABLE_RESULT;

            return result;
		}
		/* If converting the string failed, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SEARCH_WORD, NJ_ERR_INTERNAL);
	}

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SEARCH_WORD, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getNextWord
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getNextWord
  (JNIEnv *env, jclass obj, jlong wnnWork, jint length)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        if( work->flag & NJ_JNI_FLAG_ENABLE_CURSOR ) {
            jint    result;

       		/* Get a specified word and search a next word */
            if( length <= 0 ) {
        		result = ( jint )njx_get_word( &( work->wnnClass ), &( work->cursor ), &( work->result ) );
            } else {
                do {
            		result = ( jint )njx_get_word( &( work->wnnClass ), &( work->cursor ), &( work->result ) );
                    if( length == ( NJ_GET_YLEN_FROM_STEM( &( work->result.word ) ) + NJ_GET_YLEN_FROM_FZK( &( work->result.word ) ) ) ) {
                        break;
                    }
                } while( result > 0 );
            }

            /* If a result is found, enable getStroke, getCandidate, getFrequency methods */
            if( result > 0 ) {
                work->flag |= NJ_JNI_FLAG_ENABLE_RESULT;
            } else {
                work->flag &= ~NJ_JNI_FLAG_ENABLE_RESULT;
            }
            return result;
        } else {
            /* When njx_search_word() was not yet called, return "No result is found" */
            return 0;
        }
	}

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_WORD, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getStroke
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getStroke
  (JNIEnv *env, jobject obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
   		jstring		str;

        if( work->flag & NJ_JNI_FLAG_ENABLE_RESULT ) {
    		NJ_CHAR		stroke[ NJ_MAX_LEN + NJ_TERM_LEN ];

    		if( njx_get_stroke( &( work->wnnClass ), &( work->result ), stroke, sizeof( NJ_CHAR ) * ( NJ_MAX_LEN + NJ_TERM_LEN ) ) >= 0 &&
    			convertNjCharToString( env, &str, stroke, NJ_MAX_LEN ) >= 0 ) {
    			return str;
    		}
        } else {
            /* When njx_get_word() was not yet called, return "No result is found" */
            if( convertNjCharToString( env, &str, ( NJ_CHAR* )"\x00\x00", NJ_MAX_LEN ) >= 0 ) {
                return str;
            }
        }
	}

	/* If the internal work area was already released, return an error status */
	return NULL;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getCandidate
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getCandidate
  (JNIEnv *env, jobject obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
   		jstring		str;

        if( work->flag & NJ_JNI_FLAG_ENABLE_RESULT ) {
    		NJ_CHAR		candidate[ NJ_MAX_LEN + NJ_TERM_LEN ];

    		if( njx_get_candidate( &( work->wnnClass ), &( work->result ), candidate, sizeof( NJ_CHAR ) * ( NJ_MAX_RESULT_LEN + NJ_TERM_LEN ) ) >= 0 &&
    			convertNjCharToString( env, &str, candidate, NJ_MAX_RESULT_LEN ) >= 0 ) {
    			return str;
            }
        } else {
            /* When njx_get_word() was not yet called, return "No result is found" */
            if( convertNjCharToString( env, &str, ( NJ_CHAR* )"\x00\x00", NJ_MAX_RESULT_LEN ) >= 0 ) {
                return str;
            }
        }
	}

	/* If the internal work area was already released, return an error status */
	return NULL;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getFrequency
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getFrequency
  (JNIEnv *env, jobject obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        if( work->flag & NJ_JNI_FLAG_ENABLE_RESULT ) {
    		return ( jint )( work->result.word.stem.hindo );
        } else {
            /* When njx_get_word() was not yet called, return "No result is found" */
            return 0;
        }
	}

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_FREQUENCY, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    clearApproxPatterns
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_clearApproxPatterns
  (JNIEnv *env, jobject obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
		int			i;

        /* Clear state */
        work->flag = NJ_JNI_FLAG_NONE;

        /* Clear approximate patterns */
		work->approxSet.charset_count = 0;
		for( i = 0 ; i < NJ_MAX_CHARSET ; i++ ) {
			work->approxSet.from[ i ] = NULL;
			work->approxSet.to[ i ]   = NULL;
		}

        /* Clear the cache information */
        memset( work->dicSet.keyword, 0x00, sizeof( work->dicSet.keyword ) );
	}
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    setApproxPattern
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_setApproxPattern__JLjava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jlong wnnWork, jstring srcJ, jstring dstJ)
{
	NJ_JNIWORK*	work;

	if( srcJ == NULL || ( *env )->GetStringLength( env, srcJ ) == 0 || ( *env )->GetStringLength( env, srcJ ) > 1 ||
		dstJ == NULL || ( *env )->GetStringLength( env, dstJ ) == 0 || ( *env )->GetStringLength( env, dstJ ) > 3 ) {
		/* If a invalid parameter was specified, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_APPROX_PATTERN, NJ_ERR_INVALID_PARAM);
	}

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
		if( work->approxSet.charset_count < NJ_MAX_CHARSET ) {
			NJ_CHAR*		from;
			NJ_CHAR*		to;

			/* Set pointers of string to store approximate informations */
			from = work->approxStr + NJ_APPROXSTORE_SIZE * work->approxSet.charset_count;
			to   = work->approxStr + NJ_APPROXSTORE_SIZE * work->approxSet.charset_count + NJ_MAX_CHARSET_FROM_LEN + NJ_TERM_LEN;
			work->approxSet.from[ work->approxSet.charset_count ] = from;
			work->approxSet.to[ work->approxSet.charset_count ]   = to;

			/* Convert approximate informations to internal format */
			if( convertStringToNjChar( env, from, srcJ, NJ_MAX_CHARSET_FROM_LEN ) >= 0 &&
				convertStringToNjChar( env, to, dstJ, NJ_MAX_CHARSET_TO_LEN )   >= 0 ) {
				work->approxSet.charset_count++;

                /* Reset search state because the seach condition was changed */
                work->flag = NJ_JNI_FLAG_NONE;

				return 0;
			}

			/* If converting informations failed, reset pointers, and return an error code */
			work->approxSet.from[ work->approxSet.charset_count ] = NULL;
			work->approxSet.to[ work->approxSet.charset_count ]   = NULL;
			return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_APPROX_PATTERN, NJ_ERR_INTERNAL);
		}
		/* If the approx pattern registration area was full, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_APPROX_PATTERN, NJ_ERR_APPROX_PATTERN_IS_FULL);
	}

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_APPROX_PATTERN, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    setApproxPattern
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_setApproxPattern__JI
  (JNIEnv *env, jclass obj, jlong wnnWork, jint approxPattern)
{
	NJ_JNIWORK	*work;

	if( !( approxPattern == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_APPROX_PATTERN_EN_TOUPPER ||
		   approxPattern == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_APPROX_PATTERN_EN_TOLOWER ||
		   approxPattern == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_APPROX_PATTERN_EN_QWERTY_NEAR ||
		   approxPattern == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_APPROX_PATTERN_EN_QWERTY_NEAR_UPPER ||
		   approxPattern == jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_APPROX_PATTERN_JAJP_12KEY_NORMAL ) ) {
		/* If a invalid parameter was specified, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_APPROX_PATTERN, NJ_ERR_INVALID_PARAM);
	}

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
		const PREDEF_APPROX_PATTERN*	pattern;

        pattern = predefinedApproxPatterns[ approxPattern ];
		if( work->approxSet.charset_count + pattern->size <= NJ_MAX_CHARSET ) {
			int     i;

			for( i = 0 ; i < pattern->size ; i++ ) {
				NJ_CHAR*    from;
				NJ_CHAR*    to;

				/* Set pointers of string to store approximate informations */
				from = work->approxStr + NJ_APPROXSTORE_SIZE * ( work->approxSet.charset_count + i );
				to   = work->approxStr + NJ_APPROXSTORE_SIZE * ( work->approxSet.charset_count + i ) + NJ_MAX_CHARSET_FROM_LEN + NJ_TERM_LEN;
				work->approxSet.from[ work->approxSet.charset_count + i ] = from;
				work->approxSet.to[ work->approxSet.charset_count + i ]   = to;

				/* Set approximate pattern */
				from[ 0 ] = convertUTFCharToNjChar( pattern->from + i * 2 );    /* "2" means the size of UTF-16BE */
				from[ 1 ] = 0x0000;

				to[ 0 ] = convertUTFCharToNjChar( pattern->to + i * 2 );        /* "2" means the size of UTF-16BE */
				to[ 1 ] = 0x0000;
			}
			work->approxSet.charset_count += pattern->size;

            /* Reset search state because the seach condition was changed */
            work->flag = NJ_JNI_FLAG_NONE;

			return 0;
		}
		/* If the approx pattern registration area was full, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_APPROX_PATTERN, NJ_ERR_APPROX_PATTERN_IS_FULL);
	}

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_APPROX_PATTERN, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getLeftPartOfSpeech
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getLeftPartOfSpeech
  (JNIEnv *env, jclass obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        return NJ_GET_FPOS_FROM_STEM( &( work->result.word ) );
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_LEFT_PART_OF_SPEECH, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getRightPartOfSpeech
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getRightPartOfSpeech
  (JNIEnv *env, jclass obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        return NJ_GET_BPOS_FROM_STEM( &( work->result.word ) );
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_RIGHT_PART_OF_SPEECH, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    clearResult
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_clearResult
  (JNIEnv *env, jclass obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        /* Clear the current word information */
        memset( &( work->result ), 0x00, sizeof( NJ_RESULT ) );
        memset( &( work->previousStroke ), 0x00, sizeof( work->previousStroke ) );
        memset( &( work->previousCandidate ), 0x00, sizeof( work->previousCandidate ) );
    }

    /* In this method, No error reports. */
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    setLeftPartOfSpeech
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_setLeftPartOfSpeech
  (JNIEnv *env, jclass obj, jlong wnnWork, jint leftPartOfSpeech)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        NJ_UINT16   lcount = 0, rcount = 0;

        if( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ] == NULL ) {
            /* No rule dictionary was set */
        	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_LEFT_PART_OF_SPEECH, NJ_ERR_NO_RULEDIC);
        }

        njd_r_get_count( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], &lcount, &rcount );

        if( leftPartOfSpeech < 1 || leftPartOfSpeech > lcount ) {
    		/* If a invalid parameter was specified, return an error code */
    		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_LEFT_PART_OF_SPEECH, NJ_ERR_INVALID_PARAM);
        }

        NJ_SET_FPOS_TO_STEM( &( work->result.word ), leftPartOfSpeech );
        return 0;
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_LEFT_PART_OF_SPEECH, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    setRightPartOfSpeech
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_setRightPartOfSpeech
  (JNIEnv *env, jclass obj, jlong wnnWork, jint rightPartOfSpeech)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        NJ_UINT16   lcount = 0, rcount = 0;

        if( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ] == NULL ) {
            /* No rule dictionary was set */
        	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_RIGHT_PART_OF_SPEECH, NJ_ERR_NO_RULEDIC);
        }

        njd_r_get_count( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], &lcount, &rcount );

        if( rightPartOfSpeech < 1 || rightPartOfSpeech > rcount ) {
    		/* If a invalid parameter was specified, return an error code */
    		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_RIGHT_PART_OF_SPEECH, NJ_ERR_INVALID_PARAM);
        }

        NJ_SET_BPOS_TO_STEM( &( work->result.word ), rightPartOfSpeech );
        return 0;
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_RIGHT_PART_OF_SPEECH, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    setStroke
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_setStroke
  (JNIEnv *env, jclass obj, jlong wnnWork, jstring stroke)
{
	NJ_JNIWORK*	work;

    if( stroke == NULL ) {
		/* If a invalid parameter was specified, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_STROKE, NJ_ERR_INVALID_PARAM);
    }

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        if( ( *env )->GetStringLength( env, stroke ) > NJ_MAX_LEN ) {
    		/* If a invalid parameter was specified, return an error code */
        	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_STROKE, NJ_ERR_YOMI_TOO_LONG);
        }

        /* Store stroke string */
		if( convertStringToNjChar( env, work->previousStroke, stroke, NJ_MAX_LEN ) >= 0 ) {
            return 0;
        }

		/* If converting the string failed, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_STROKE, NJ_ERR_INTERNAL);
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_STROKE, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    setCandidate
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_setCandidate
  (JNIEnv *env, jclass obj, jlong wnnWork, jstring candidate)
{
	NJ_JNIWORK*	work;

    if( candidate == NULL ) {
		/* If a invalid parameter was specified, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_CANDIDATE, NJ_ERR_INVALID_PARAM);
    }

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        if( ( *env )->GetStringLength( env, candidate ) > NJ_MAX_RESULT_LEN ) {
    		/* If a invalid parameter was specified, return an error code */
        	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_CANDIDATE, NJ_ERR_CANDIDATE_TOO_LONG);
        }

        /* Store candidate string */
		if( convertStringToNjChar( env, work->previousCandidate, candidate, NJ_MAX_RESULT_LEN ) >= 0 ) {
            return 0;
        }

		/* If converting the string failed, return an error code */
		return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_CANDIDATE, NJ_ERR_INTERNAL);
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SET_CANDIDATE, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    selectWord
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_selectWord
  (JNIEnv *env, jclass obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        /* Put the previous word information to engine */
        memcpy( &( work->wnnClass.dic_set ), &( work->dicSet ), sizeof( NJ_DIC_SET ) );
        return ( jint )njx_select( &( work->wnnClass ), &( work->result ) );
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_SELECT_WORD, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getLeftPartOfSpeechSpecifiedType
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getLeftPartOfSpeechSpecifiedType
  (JNIEnv *env, jclass obj, jlong wnnWork, jint type)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        switch( type ) {
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_V1:
            type = NJ_HINSI_V1_F;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_V2:
            type = NJ_HINSI_V2_F;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_V3:
            type = NJ_HINSI_V3_F;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_BUNTOU:
    		/* No part of speech is defined at this type */
            return 0;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_TANKANJI:
            type = NJ_HINSI_TANKANJI_F;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_SUUJI:
    		/* No part of speech is defined at this type */
            return 0;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_MEISI:
            type = NJ_HINSI_MEISI_F;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_JINMEI:
            type = NJ_HINSI_JINMEI_F;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_CHIMEI:
            type = NJ_HINSI_CHIMEI_F;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_KIGOU:
            type = NJ_HINSI_KIGOU_F;
            break;
        default:
    		/* If a invalid parameter was specified, return an error code */
        	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_LEFT_PART_OF_SPEECH_SPECIFIED_TYPE, NJ_ERR_INVALID_PARAM);
        }
        return ( jint )njd_r_get_hinsi( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], type );
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_LEFT_PART_OF_SPEECH_SPECIFIED_TYPE, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getRightPartOfSpeechSpecifiedType
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getRightPartOfSpeechSpecifiedType
  (JNIEnv *env, jclass obj, jlong wnnWork, jint type)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        switch( type ) {
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_V1:
    		/* No part of speech is defined at this type */
            return 0;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_V2:
    		/* No part of speech is defined at this type */
            return 0;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_V3:
    		/* No part of speech is defined at this type */
            return 0;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_BUNTOU:
            type = NJ_HINSI_BUNTOU_B;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_TANKANJI:
            type = NJ_HINSI_TANKANJI_B;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_SUUJI:
            type = NJ_HINSI_SUUJI_B;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_MEISI:
            type = NJ_HINSI_MEISI_B;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_JINMEI:
            type = NJ_HINSI_JINMEI_B;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_CHIMEI:
            type = NJ_HINSI_CHIMEI_B;
            break;
        case jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_POS_TYPE_KIGOU:
            type = NJ_HINSI_KIGOU_B;
            break;
        default:
    		/* If a invalid parameter was specified, return an error code */
        	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_LEFT_PART_OF_SPEECH_SPECIFIED_TYPE, NJ_ERR_INVALID_PARAM);
        }
        return ( jint )njd_r_get_hinsi( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], type );
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_RIGHT_PART_OF_SPEECH_SPECIFIED_TYPE, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getConnectArray
 * Signature: (JI)[B
 */
JNIEXPORT jbyteArray JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getConnectArray
  (JNIEnv *env, jclass obj, jlong wnnWork, jint leftPartOfSpeech)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        NJ_UINT16   lcount = 0, rcount = 0;
        jbyteArray  resultJ;

        if( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ] == NULL ) {
            /* No rule dictionary was set */
        	return NULL;
        }

        njd_r_get_count( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], &lcount, &rcount );

        if( leftPartOfSpeech < 0 || leftPartOfSpeech > lcount ) {
            /* Invalid POS is specified */
            return NULL;
        }

        /* 1-origin */
        resultJ = ( *env )->NewByteArray( env, rcount + 1 );

        if( resultJ != NULL ) {
            jbyte   *result;
            result = ( *env )->GetByteArrayElements( env, resultJ, NULL );

            if( result != NULL ) {
                int         i;
                NJ_UINT8*   connect;

                if( leftPartOfSpeech == 0 ) {
                    for( i = 0 ; i < rcount + 1 ; i++ ) {
                        result[ i ] = 0;
                    }
                } else {
                    /* Get the packed connect array */
                    njd_r_get_connect( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], leftPartOfSpeech, NJ_RULE_TYPE_FTOB, &connect );

                    /* Extract connect array from bit field */
                    result[ 0 ] = 0;

                    for( i = 0 ; i < rcount ; i++ ) {
                        if( connect[ i / 8 ] & (0x80 >> (i % 8))) {
                            result[ i + 1 ] = 1;
                        } else {
                            result[ i + 1 ] = 0;
                        }
                    }
                }

                ( *env )->ReleaseByteArrayElements( env, resultJ, result, 0 );
                return resultJ;
            }
        }
		/* If allocating the return area failed, return an error code */
       	return NULL;
    }
	/* If the internal work area was already released, return an error code */
    return NULL;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getNumberOfLeftPOS
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getNumberOfLeftPOS
  (JNIEnv *env, jclass obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        if( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ] == NULL ) {
            /* No rule dictionary was set */
            return 0;
        } else {
            NJ_UINT16   lcount = 0, rcount = 0;

            njd_r_get_count( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], &lcount, &rcount );
            return lcount;
        }
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_NUMBER_OF_LEFT_POS, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getNumberOfRightPOS
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getNumberOfRightPOS
  (JNIEnv *env, jclass obj, jlong wnnWork)
{
	NJ_JNIWORK*	work;

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        if( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ] == NULL ) {
            /* No rule dictionary was set */
            return 0;
        } else {
            NJ_UINT16   lcount = 0, rcount = 0;

            njd_r_get_count( work->dicSet.rHandle[ NJ_MODE_TYPE_HENKAN ], &lcount, &rcount );
            return rcount;
        }
    }

	/* If the internal work area was already released, return an error code */
	return NJ_SET_ERR_VAL(NJ_FUNC_JNI_GET_NUMBER_OF_RIGHT_POS, NJ_ERR_NOT_ALLOCATED);
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    getApproxPattern
 * Signature: (JLjava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_getApproxPattern
  (JNIEnv *env, jclass obj, jlong wnnWork, jstring srcJ)
{
	NJ_JNIWORK*	work;

	if( srcJ == NULL || ( *env )->GetStringLength( env, srcJ ) == 0 || ( *env )->GetStringLength( env, srcJ ) > 1 ) {
		/* If a invalid parameter was specified, return an error code */
		return NULL;
	}

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        int         i, outIndex, outCount;
        NJ_CHAR     from[ NJ_MAX_CHARSET_FROM_LEN + NJ_TERM_LEN ];

        if( convertStringToNjChar( env, from, srcJ, NJ_MAX_CHARSET_FROM_LEN ) >= 0 ) {
            outCount = 0;
            for( i = 0 ; i < work->approxSet.charset_count ; i++ ) {
                if( nj_strcmp( from, work->approxSet.from[ i ] ) == 0 ) {
                    outCount++;
                }
            }

            jclass strC = ( *env )->FindClass( env, "java/lang/String" );

            if( strC != NULL ) {
                jobjectArray retJ = ( *env )->NewObjectArray( env, outCount, strC, NULL );

                if( retJ != NULL ) {
                    for( i = outIndex = 0 ; i < work->approxSet.charset_count ; i++ ) {
                        if( nj_strcmp( from, work->approxSet.from[ i ] ) == 0 ) {
                            jstring dstJ;

                            if( convertNjCharToString( env, &dstJ, work->approxSet.to[ i ], NJ_MAX_CHARSET_TO_LEN ) < 0 ) {
                                return NULL;
                            }

                            ( *env )->SetObjectArrayElement( env, retJ, outIndex++, dstJ );
                        }

                    }
                    return retJ;
                }
            }
        }
        /* If the internal error occured, return an error code */
        return NULL;
    }

	/* If the internal work area was already released, return an error code */
	return NULL;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    createBindArray
 * Signature: (JLjava/lang/String;II)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_createBindArray
  (JNIEnv *env, jclass obj, jlong wnnWork, jstring keyStringJ, jint maxBindsOfQuery, jint maxPatternOfApprox)
{
	NJ_JNIWORK*	work;

	if( keyStringJ == NULL ) {
		/* If a invalid parameter was specified, return an error code */
		return NULL;
	}

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
        /* create the string array for result */
        jclass strC = ( *env )->FindClass( env, "java/lang/String" );

        if( strC != NULL ) {
            jobjectArray retJ = ( *env )->NewObjectArray( env, maxBindsOfQuery * (maxPatternOfApprox+1), strC, NULL );

            if( retJ != NULL ) {
                NJ_CHAR     keyString[ NJ_MAX_LEN + NJ_TERM_LEN ];

                if( convertStringToNjChar( env, keyString, keyStringJ, NJ_MAX_LEN ) >= 0 ) {
                    int         queryLen, outIndex, approxPattern;
                    NJ_CHAR     baseStr[ NJ_MAX_LEN + NJ_MAX_CHARSET_TO_LEN + NJ_TERM_LEN ];

                    outIndex = 0;
                    baseStr[ 0 ] = NJ_CHAR_NUL;

                    for( queryLen = 0 ; queryLen < maxBindsOfQuery && keyString[ queryLen ] != NJ_CHAR_NUL ; queryLen++ ) {
                        int i;

                        for( i = -1, approxPattern = -1 ; i < work->approxSet.charset_count ; i++ ) {
                            if( i == -1 || keyString[ queryLen ] == work->approxSet.from[ i ][ 0 ] ) {
                                int tailOffset = 0;

                                if( i == -1 ) {
                                    if(   *( ( NJ_UINT8* )( &keyString[ queryLen ] ) + 0 ) == 0x00 &&
                                        ( *( ( NJ_UINT8* )( &keyString[ queryLen ] ) + 1 ) == 0x25 ||       /* '%' */
                                          *( ( NJ_UINT8* )( &keyString[ queryLen ] ) + 1 ) == 0x5c ||       /* '\' */
                                          *( ( NJ_UINT8* )( &keyString[ queryLen ] ) + 1 ) == 0x5f ) ) {    /* '_' */
                                        *( ( NJ_UINT8* )( &baseStr[ queryLen + 0 ] ) + 0 ) = 0x00;
                                        *( ( NJ_UINT8* )( &baseStr[ queryLen + 0 ] ) + 1 ) = 0x5c;  /* '\' */
                                                           baseStr[ queryLen + 1 ] = keyString[ queryLen ];
                                        tailOffset = 2;
                                    } else {
                                        baseStr[ queryLen + 0 ] = keyString[ queryLen ];
                                        tailOffset = 1;
                                    }
                                } else {
                                    nj_strcpy( &baseStr[ queryLen ], work->approxSet.to[ i ] );
                                    tailOffset = nj_strlen( work->approxSet.to[ i ] );
                                }

                                *( ( NJ_UINT8* )( &baseStr[ queryLen + tailOffset     ] ) + 0 ) = 0x00;
                                *( ( NJ_UINT8* )( &baseStr[ queryLen + tailOffset     ] ) + 1 ) = 0x25;  /* '%' */
                                                   baseStr[ queryLen + tailOffset + 1 ]         = NJ_CHAR_NUL;

                                jstring dstJ;
                                if( convertNjCharToString( env, &dstJ, baseStr, NJ_MAX_LEN ) < 0 ) {
                                    return NULL;
                                }

                                ( *env )->SetObjectArrayElement( env, retJ, outIndex++, dstJ );
                                approxPattern++;
                            }
                        }
                        for( ; approxPattern < maxPatternOfApprox ; approxPattern++ ) {
                            jstring dstJ = ( *env )->NewStringUTF( env, "" );
                            if( dstJ == NULL ) {
                                return NULL;
                            }
                            ( *env )->SetObjectArrayElement( env, retJ, outIndex++, dstJ );
                        }

                        *( ( NJ_UINT8* )( &baseStr[ queryLen     ] ) + 0 ) = 0x00;
                        *( ( NJ_UINT8* )( &baseStr[ queryLen     ] ) + 1 ) = 0x5f;  /* '_' */
                                           baseStr[ queryLen + 1 ]         = NJ_CHAR_NUL;
                    }

                    for( ; queryLen < maxBindsOfQuery ; queryLen++ ) {
                        for( approxPattern = -1 ; approxPattern < maxPatternOfApprox ; approxPattern++ ) {
                            jstring dstJ = ( *env )->NewStringUTF( env, "%" );
                            if( dstJ == NULL ) {
                                return NULL;
                            }
                            ( *env )->SetObjectArrayElement( env, retJ, outIndex++, dstJ );
                        }
                    }

                    return retJ;
                }
            }
        }
        /* If the internal error occured, return an error code */
        return NULL;
    }

	/* If the internal work area was already released, return an error code */
	return NULL;
}

/*
 * Class:     jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni
 * Method:    createQueryStringBase
 * Signature: (JIILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_jp_co_omronsoft_openwnn_OpenWnnDictionaryImplJni_createQueryStringBase
  (JNIEnv *env, jclass obj, jlong wnnWork, jint maxBindsOfQuery, jint maxPatternOfApprox, jstring keyColumnNameJ)
{
    NJ_JNIWORK*	work;
    jstring retJ = NULL;

	if( keyColumnNameJ == NULL ) {
		/* If a invalid parameter was specified, return an error code */
		return NULL;
	}

	work = *( NJ_JNIWORK** )&wnnWork;
	if( work != NULL ) {
    	const unsigned char* keyName = ( const unsigned char* )( ( *env )->GetStringUTFChars( env, keyColumnNameJ, NULL ) );

        if( keyName != NULL ) {
            int keyLength = strlen( ( char* )keyName );

            char *dst = ( char* )malloc( maxBindsOfQuery * ( ( 1 + keyLength + 18 + 1 + 5 ) +
                                                             ( ( 4 + keyLength + 18 ) * maxPatternOfApprox ) +
                                                             1 ) );
            if( dst != NULL ) {
                int queryLen, dstPtr;

            	for( queryLen = dstPtr = 0 ; queryLen < maxBindsOfQuery ; queryLen++ ) {
                    int approxPattern;

                    strcpy( &dst[ dstPtr                 ], "(" );
                    strcpy( &dst[ dstPtr + 1             ], ( char* )keyName );
                    strcpy( &dst[ dstPtr + 1 + keyLength ], " like ? escape '\x5c'" );
                    dstPtr += 1 + keyLength + 18;

            		for( approxPattern = 0 ; approxPattern < maxPatternOfApprox ; approxPattern++ ) {
                        strcpy( &dst[ dstPtr                 ], " or " );
                        strcpy( &dst[ dstPtr + 4             ], ( char* )keyName );
                        strcpy( &dst[ dstPtr + 4 + keyLength ], " like ? escape '\x5c'" );
                        dstPtr += 4 + keyLength + 18;
            		}
                    strcpy( &dst[ dstPtr ], ")" );
                    dstPtr++;

            		if( queryLen != maxBindsOfQuery-1 ) {
                        strcpy( &dst[ dstPtr ], " and " );
                        dstPtr += 5;
            		}
            	}

                dst[ dstPtr ] = '\0';
                retJ = ( *env )->NewStringUTF( env, dst );

                free( dst );
            }

       		( *env )->ReleaseStringUTFChars( env, keyColumnNameJ, ( const char* )keyName );
        }
    }
	return retJ;
}

