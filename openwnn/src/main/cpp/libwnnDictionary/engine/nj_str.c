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


NJ_CHAR *nj_strcpy(NJ_CHAR *dst, NJ_CHAR *src) {

    NJ_CHAR *ret = dst;


    while (*src != NJ_CHAR_NUL) {
        *dst++ = *src++;
    }
    *dst = *src;
    return ret;
}

NJ_CHAR *nj_strncpy(NJ_CHAR *dst, NJ_CHAR *src, NJ_UINT16 n) {

    NJ_CHAR *d = dst;


    while (n != 0) {
        if (*src == NJ_CHAR_NUL) {
            while (n != 0) {
                *d++ = NJ_CHAR_NUL;
                n--;
            }
            break;
        } else {
            *d++ = *src++;
        }
        n--;
    }
    return dst;
}

NJ_UINT16 nj_strlen(NJ_CHAR *c) {

    NJ_UINT16 count = 0;
    

    while (*c++ != NJ_CHAR_NUL) {
        count++;
    }
    return count;
}

NJ_INT16 nj_strcmp(NJ_CHAR *s1, NJ_CHAR *s2) {

    while (*s1 == *s2) {
        if (*s1 == NJ_CHAR_NUL) {
            return (0);
        }
        s1++;
        s2++;
    }
    return NJ_CHAR_DIFF(s1, s2);
}

NJ_INT16 nj_strncmp(NJ_CHAR *s1, NJ_CHAR *s2, NJ_UINT16 n) {

    while (n != 0) {
        if (*s1 != *s2++) {
            return NJ_CHAR_DIFF(s1, (s2 - 1));
        }
        if (*s1++ == NJ_CHAR_NUL) {
            break;
        }
        n--;
    }
    return (0);
}

NJ_UINT16 nj_charlen(NJ_CHAR *c) {

    NJ_UINT16 count = 0;
    

    while (*c != NJ_CHAR_NUL) {
        count++;
        c += NJ_CHAR_LEN(c);
    }
    return count;
}

NJ_INT16 nj_charncmp(NJ_CHAR *s1, NJ_CHAR *s2, NJ_UINT16 n) {
    NJ_UINT16 i;


    while (n != 0) {
        for (i = NJ_CHAR_LEN(s1); i != 0; i--) {
            if (*s1 != *s2) {
                return NJ_CHAR_DIFF(s1, s2);
            }
            if (*s1 == NJ_CHAR_NUL) { 
                break;
            }
            s1++;
            s2++;
        }
        n--;
    }
    return (0);
}

NJ_CHAR *nj_charncpy(NJ_CHAR *dst, NJ_CHAR *src, NJ_UINT16 n) {

    NJ_CHAR *d = dst;
    NJ_UINT16 i;


    while (n != 0) {
        for (i = NJ_CHAR_LEN(src); i != 0; i--) {
            *d = *src;
            if (*src == NJ_CHAR_NUL) {
                return dst; 
            }
            d++;
            src++;
        }
        n--;
    }
    *d = NJ_CHAR_NUL;
    return dst;
}

NJ_UINT8 *nj_memcpy(NJ_UINT8 *dst, NJ_UINT8 *src, NJ_UINT16 n) {

    NJ_UINT8 *d = dst;


    while (n != 0) {
        *d++ = *src++;
        n--;
    }
    return dst;
}
