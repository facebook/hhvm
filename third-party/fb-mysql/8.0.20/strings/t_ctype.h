/* Copyright (c) 2000, 2001, 2003 MySQL AB
   Use is subject to license terms

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/*
  Copyright (C) 1998, 1999 by Pruet Boonma, all rights reserved.
  Copyright (C) 1998 by Theppitak Karoonboonyanan, all rights reserved.
  Permission to use, copy, modify, distribute and sell this software
   and its documentation for any purpose is hereby granted without fee,
   provided that the above copyright notice appear in all copies.
   Smaphan Raruenrom and Pruet Boonma makes no representations about
   the suitability of this software for any purpose.  It is provided
    "as is" without express or implied warranty.
*/

/*
  LC_COLLATE category + Level information
*/

#ifndef _t_ctype_h
#define _t_ctype_h

typedef unsigned char tchar;

#define TOT_LEVELS 5
#define LAST_LEVEL 4 /* TOT_LEVELS - 1 */

#define IGNORE 0

/* level 1 symbols & order */
enum l1_symbols {
  L1_08 = TOT_LEVELS,
  L1_18,
  L1_28,
  L1_38,
  L1_48,
  L1_58,
  L1_68,
  L1_78,
  L1_88,
  L1_98,
  L1_A8,
  L1_B8,
  L1_C8,
  L1_D8,
  L1_E8,
  L1_F8,
  L1_G8,
  L1_H8,
  L1_I8,
  L1_J8,
  L1_K8,
  L1_L8,
  L1_M8,
  L1_N8,
  L1_O8,
  L1_P8,
  L1_Q8,
  L1_R8,
  L1_S8,
  L1_T8,
  L1_U8,
  L1_V8,
  L1_W8,
  L1_X8,
  L1_Y8,
  L1_Z8,
  L1_KO_KAI,
  L1_KHO_KHAI,
  L1_KHO_KHUAT,
  L1_KHO_KHWAI,
  L1_KHO_KHON,
  L1_KHO_RAKHANG,
  L1_NGO_NGU,
  L1_CHO_CHAN,
  L1_CHO_CHING,
  L1_CHO_CHANG,
  L1_SO_SO,
  L1_CHO_CHOE,
  L1_YO_YING,
  L1_DO_CHADA,
  L1_TO_PATAK,
  L1_THO_THAN,
  L1_THO_NANGMONTHO,
  L1_THO_PHUTHAO,
  L1_NO_NEN,
  L1_DO_DEK,
  L1_TO_TAO,
  L1_THO_THUNG,
  L1_THO_THAHAN,
  L1_THO_THONG,
  L1_NO_NU,
  L1_BO_BAIMAI,
  L1_PO_PLA,
  L1_PHO_PHUNG,
  L1_FO_FA,
  L1_PHO_PHAN,
  L1_FO_FAN,
  L1_PHO_SAMPHAO,
  L1_MO_MA,
  L1_YO_YAK,
  L1_RO_RUA,
  L1_RU,
  L1_LO_LING,
  L1_LU,
  L1_WO_WAEN,
  L1_SO_SALA,
  L1_SO_RUSI,
  L1_SO_SUA,
  L1_HO_HIP,
  L1_LO_CHULA,
  L1_O_ANG,
  L1_HO_NOKHUK,
  L1_NKHIT,
  L1_SARA_A,
  L1_MAI_HAN_AKAT,
  L1_SARA_AA,
  L1_SARA_AM,
  L1_SARA_I,
  L1_SARA_II,
  L1_SARA_UE,
  L1_SARA_UEE,
  L1_SARA_U,
  L1_SARA_UU,
  L1_SARA_E,
  L1_SARA_AE,
  L1_SARA_O,
  L1_SARA_AI_MAIMUAN,
  L1_SARA_AI_MAIMALAI
};

/* level 2 symbols & order */
enum l2_symbols {
  L2_BLANK = TOT_LEVELS,
  L2_THAII,
  L2_YAMAK,
  L2_PINTHU,
  L2_GARAN,
  L2_TYKHU,
  L2_TONE1,
  L2_TONE2,
  L2_TONE3,
  L2_TONE4
};

/* level 3 symbols & order */
enum l3_symbols {
  L3_BLANK = TOT_LEVELS,
  L3_SPACE,
  L3_NB_SACE,
  L3_LOW_LINE,
  L3_HYPHEN,
  L3_COMMA,
  L3_SEMICOLON,
  L3_COLON,
  L3_EXCLAMATION,
  L3_QUESTION,
  L3_SOLIDUS,
  L3_FULL_STOP,
  L3_PAIYAN_NOI,
  L3_MAI_YAMOK,
  L3_GRAVE,
  L3_CIRCUMFLEX,
  L3_TILDE,
  L3_APOSTROPHE,
  L3_QUOTATION,
  L3_L_PARANTHESIS,
  L3_L_BRACKET,
  L3_L_BRACE,
  L3_R_BRACE,
  L3_R_BRACKET,
  L3_R_PARENTHESIS,
  L3_AT,
  L3_BAHT,
  L3_DOLLAR,
  L3_FONGMAN,
  L3_ANGKHANKHU,
  L3_KHOMUT,
  L3_ASTERISK,
  L3_BK_SOLIDUS,
  L3_AMPERSAND,
  L3_NUMBER,
  L3_PERCENT,
  L3_PLUS,
  L3_LESS_THAN,
  L3_EQUAL,
  L3_GREATER_THAN,
  L3_V_LINE
};

/* level 4 symbols & order */
enum l4_symbols { L4_BLANK = TOT_LEVELS, L4_MIN, L4_CAP, L4_EXT };

enum level_symbols { L_UPRUPR = TOT_LEVELS, L_UPPER, L_MIDDLE, L_LOWER };

#define _is(c) (t_ctype[(c)][LAST_LEVEL])
#define _level 8
#define _consnt 16
#define _ldvowel 32
#define _fllwvowel 64
#define _uprvowel 128
#define _lwrvowel 256
#define _tone 512
#define _diacrt1 1024
#define _diacrt2 2048
#define _combine 4096
#define _stone 8192
#define _tdig 16384
#define _rearvowel (_fllwvowel | _uprvowel | _lwrvowel)
#define _diacrt (_diacrt1 | _diacrt2)
#define levelof(c) (_is(c) & _level)
#define isthai(c) ((c) >= 128)
#define istalpha(c) \
  (_is(c) & (_consnt | _ldvowel | _rearvowel | _tone | _diacrt1 | _diacrt2))
#define isconsnt(c) (_is(c) & _consnt)
#define isldvowel(c) (_is(c) & _ldvowel)
#define isfllwvowel(c) (_is(c) & _fllwvowel)
#define ismidvowel(c) (_is(c) & (_ldvowel | _fllwvowel))
#define isuprvowel(c) (_is(c) & _uprvowel)
#define islwrvowel(c) (_is(c) & _lwrvowel)
#define isuprlwrvowel(c) (_is(c) & (_lwrvowel | _uprvowel))
#define isrearvowel(c) (_is(c) & _rearvowel)
#define isvowel(c) (_is(c) & (_ldvowel | _rearvowel))
#define istone(c) (_is(c) & _tone)
#define isunldable(c) (_is(c) & (_rearvowel | _tone | _diacrt1 | _diacrt2))
#define iscombinable(c) (_is(c) & _combine)
#define istdigit(c) (_is(c) & _tdig)
#define isstone(c) (_is(c) & _stone)
#define isdiacrt1(c) (_is(c) & _diacrt1)
#define isdiacrt2(c) (_is(c) & _diacrt2)
#define isdiacrt(c) (_is(c) & _diacrt)

/* Function prototype called by sql/field.cc */
void ThNormalize(uchar *ptr, uint field_length, const uchar *from, uint length);

#endif
