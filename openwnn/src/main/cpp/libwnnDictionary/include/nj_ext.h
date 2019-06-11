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

#ifndef _NJ_EXTERN_H_
#define _NJ_EXTERN_H_


#define NJ_MAX_CHAR_LEN  2

#define NJ_CHAR_IS_EQUAL(a, b) \
    ( (((NJ_UINT8*)(a))[0] == ((NJ_UINT8*)(b))[0]) && (((NJ_UINT8*)(a))[1] == ((NJ_UINT8*)(b))[1]) )

#define NJ_CHAR_IS_LESSEQ(a, b)                                         \
    ( (((NJ_UINT8*)(a))[0] < ((NJ_UINT8*)(b))[0]) ||                    \
      ((((NJ_UINT8*)(a))[0] == ((NJ_UINT8*)(b))[0]) && (((NJ_UINT8*)(a))[1] <= ((NJ_UINT8*)(b))[1])) )

#define NJ_CHAR_IS_MOREEQ(a, b)                                         \
    (  (((NJ_UINT8*)(a))[0] >  ((NJ_UINT8*)(b))[0]) ||                  \
      ((((NJ_UINT8*)(a))[0] == ((NJ_UINT8*)(b))[0]) && (((NJ_UINT8*)(a))[1] >= ((NJ_UINT8*)(b))[1])) )

#define NJ_CHAR_DIFF(a, b)                                              \
    ((NJ_INT16)                                                         \
     ( (((NJ_UINT8*)(a))[0] == ((NJ_UINT8*)(b))[0])                     \
       ? (((NJ_UINT8*)(a))[1] - ((NJ_UINT8*)(b))[1])                    \
       : (((NJ_UINT8*)(a))[0] - ((NJ_UINT8*)(b))[0]) )                  \
     )

#define NJ_CHAR_COPY(dst, src)                                          \
    {                                                                   \
        ((NJ_UINT8*)(dst))[0] = ((NJ_UINT8*)(src))[0];                  \
        ((NJ_UINT8*)(dst))[1] = ((NJ_UINT8*)(src))[1];                  \
    }

#define NJ_CHAR_STRLEN_IS_0(c)   (*(c) == NJ_CHAR_NUL)

#define NJ_CHAR_ILLEGAL_DIC_YINDEX(size)   ((size) != 2)


#define NJ_CHAR_LEN(s)                                                  \
    ( (NJ_CHAR_IS_MOREEQ((s), "\xD8\x00") && NJ_CHAR_IS_LESSEQ((s), "\xDB\xFF")) \
      ? ( (*((s)+1) == NJ_CHAR_NUL) ? 1 : 2)                            \
      : 1) 

#define UTL_CHAR(s)  1


#define NJ_GET_DIC_INFO(dicinfo) ((NJ_UINT8)((dicinfo)->type))

#define NJ_GET_DIC_TYPE_EX(type, handle) \
                 NJ_GET_DIC_TYPE((handle))                                    


#define GET_BITFIELD_16(data, pos, width)                        \
    ((NJ_UINT16)(((NJ_UINT16)(data) >> (16 - (pos) - (width))) & \
                 ((NJ_UINT16)0xffff >> (16 - (width)       ))))

#define GET_BITFIELD_32(data, pos, width)       \
    ((NJ_UINT32)(((NJ_UINT32)(data) >> (32 - (pos) - (width))) & ((NJ_UINT32)0xffffffff >> (32 - (width)))))

#define GET_BIT_TO_BYTE(bit) ((NJ_UINT8)(((bit) + 7) >> 3))


#define INIT_KEYWORD_IN_NJ_DIC_SET(x) \
    { (x)->keyword[0] = NJ_CHAR_NUL; (x)->keyword[1] = NJ_CHAR_NUL; }

#define GET_ERR_FUNCVAL(errval) \
    ((NJ_UINT16)(((NJ_UINT16)(errval) & 0x007F) << 8))


extern NJ_INT16 njd_get_word_data(NJ_CLASS *iwnn, NJ_DIC_SET *dicset, NJ_SEARCH_LOCATION_SET *loctset, NJ_UINT16 dic_idx, NJ_WORD *word);
extern NJ_INT16 njd_get_stroke(NJ_CLASS *iwnn, NJ_RESULT *result,
                               NJ_CHAR *stroke, NJ_UINT16 size);
extern NJ_INT16 njd_get_candidate(NJ_CLASS *iwnn, NJ_RESULT *result,
                               NJ_CHAR *candidate, NJ_UINT16 size);
extern NJ_INT16 njd_init_search_location_set(NJ_SEARCH_LOCATION_SET* loctset);
extern NJ_INT16 njd_init_word(NJ_WORD* word);

extern NJ_INT16 njd_b_search_word(NJ_SEARCH_CONDITION *con,
                                  NJ_SEARCH_LOCATION_SET *loctset);
extern NJ_INT16 njd_b_get_word(NJ_SEARCH_LOCATION_SET *loctset, NJ_WORD *word);
extern NJ_INT16 njd_b_get_candidate(NJ_WORD *word, NJ_CHAR *candidate,
                                    NJ_UINT16 size);
extern NJ_INT16 njd_b_get_stroke(NJ_WORD *word, NJ_CHAR *stroke, NJ_UINT16 size);

extern NJ_INT16 njd_f_search_word(NJ_SEARCH_CONDITION *con,
                                  NJ_SEARCH_LOCATION_SET *loctset);
extern NJ_INT16 njd_f_get_word(NJ_SEARCH_LOCATION_SET *loctset, NJ_WORD *word);
extern NJ_INT16 njd_f_get_stroke(NJ_WORD *word, NJ_CHAR *stroke,
                                 NJ_UINT16 size);
extern NJ_INT16 njd_f_get_candidate(NJ_WORD *word, NJ_CHAR *candidate,
                                    NJ_UINT16 size);

extern NJ_INT16 njd_l_search_word(NJ_CLASS *iwnn, NJ_SEARCH_CONDITION *con,
                                  NJ_SEARCH_LOCATION_SET *loctset, NJ_UINT8 comp_flg);
extern NJ_INT16 njd_l_add_word(NJ_CLASS *iwnn, NJ_LEARN_WORD_INFO *word,
                                NJ_UINT8 connect, NJ_UINT8 type,
                                NJ_UINT8 undo, NJ_UINT8 dictype);

extern NJ_INT16 njd_l_delete_word(NJ_CLASS *iwnn, NJ_SEARCH_LOCATION *loc);
extern NJ_INT16 njd_l_get_word(NJ_CLASS *iwnn, NJ_SEARCH_LOCATION_SET *loctset, NJ_WORD *word);
extern NJ_INT16 njd_l_get_stroke(NJ_CLASS *iwnn, NJ_WORD *word,
                                 NJ_CHAR *stroke, NJ_UINT16 size);
extern NJ_INT16 njd_l_get_candidate(NJ_CLASS *iwnn, NJ_WORD *word,
                                 NJ_CHAR *candidate, NJ_UINT16 size);
extern NJ_INT16 njd_l_undo_learn(NJ_CLASS *iwnn, NJ_UINT16 undo_count);
extern NJ_INT16 njd_l_check_dic(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle);
extern NJ_INT16 njd_l_init_area(NJ_DIC_HANDLE handle);
extern NJ_INT16 njd_l_make_space(NJ_CLASS *iwnn, NJ_UINT16 count, NJ_UINT8 mode);
extern NJ_INT16 njd_l_get_relational_word(NJ_CLASS *iwnn, NJ_SEARCH_LOCATION *loc,
                                 NJ_WORD *word, NJ_DIC_FREQ *mdic_freq);
extern NJ_INT16 njd_l_check_word_connect(NJ_CLASS *iwnn, NJ_WORD *word);
extern NJ_INT16 njd_l_get_ext_word_data(NJ_CLASS *iwnn, NJ_WORD *word, NJ_UINT16 *hinsi, NJ_UINT8 *len);
extern NJ_INT16 njd_l_mld_op_commit(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle);
extern NJ_INT16 njd_l_mld_op_commit_to_top(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle);
extern NJ_INT16 njd_l_mld_op_commit_cancel(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle);
extern NJ_INT16 njd_l_mld_op_get_space(NJ_CLASS *iwnn, NJ_DIC_HANDLE handle);

extern NJ_INT16 njd_r_get_hinsi(NJ_DIC_HANDLE rule, NJ_UINT8 type);
extern NJ_INT16 njd_r_get_connect(NJ_DIC_HANDLE rule,
                                  NJ_UINT16 hinsi, NJ_UINT8 type,
                                  NJ_UINT8 **connect);
extern NJ_INT16 njd_r_get_count(NJ_DIC_HANDLE rule,
                                NJ_UINT16 *fcount, NJ_UINT16 *rcount);

extern NJ_UINT16 nje_check_string(NJ_CHAR *s, NJ_UINT16 max_len);
extern NJ_UINT8 nje_get_top_char_type(NJ_CHAR *s);
extern NJ_INT16 nje_convert_kata_to_hira(NJ_CHAR *kata, NJ_CHAR *hira, NJ_UINT16 len, NJ_UINT16 max_len, NJ_UINT8 type);
extern NJ_INT16 nje_convert_hira_to_kata(NJ_CHAR *hira, NJ_CHAR *kata, NJ_UINT16 len);

extern NJ_INT16 njd_connect_test(NJ_SEARCH_CONDITION *con, NJ_UINT16 hinsiF, NJ_UINT16 hinsiR);

extern NJ_CHAR  *nj_strcpy(NJ_CHAR *dst, NJ_CHAR *src);
extern NJ_CHAR  *nj_strncpy(NJ_CHAR *dst, NJ_CHAR *src, NJ_UINT16 n);
extern NJ_UINT16 nj_strlen(NJ_CHAR *c);
extern NJ_INT16  nj_strcmp(NJ_CHAR *s1, NJ_CHAR *s2);
extern NJ_INT16  nj_strncmp(NJ_CHAR *s1, NJ_CHAR *s2, NJ_UINT16 n);
extern NJ_UINT16 nj_charlen(NJ_CHAR *c);
extern NJ_INT16  nj_charncmp(NJ_CHAR *s1, NJ_CHAR *s2, NJ_UINT16 n);
extern NJ_CHAR  *nj_charncpy(NJ_CHAR *dst, NJ_CHAR *src, NJ_UINT16 n);
extern NJ_UINT8 *nj_memcpy(NJ_UINT8 *dst, NJ_UINT8 *src, NJ_UINT16 n);


#endif 
