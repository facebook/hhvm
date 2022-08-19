/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/*
   Copyright (C) 2003  by Sathit Jittanupat
          <jsat66@hotmail.com,jsat66@yahoo.com>
        * solving bug crash with long text field string
        * sorting with different number of space or sign char. within string

   Copyright (C) 2001  by Korakot Chaovavanich <korakot@iname.com> and
                          Apisilp Trunganont <apisilp@pantip.inet.co.th>
   Copyright (C) 1998, 1999 by Pruet Boonma <pruet@eng.cmu.ac.th>
   Copyright (C) 1998  by Theppitak Karoonboonyanan <thep@links.nectec.or.th>
   Copyright (C) 1989, 1991 by Samphan Raruenrom <samphan@thai.com>

   Permission to use, copy, modify, distribute and sell this software
   and its documentation for any purpose is hereby granted without fee,
   provided that the above copyright notice appear in all copies.
   Samphan Raruenrom , Theppitak Karoonboonyanan , Pruet Boonma ,
   Korakot Chaovavanich and Apisilp Trunganont makes no representations
   about the suitability of this software for any purpose.  It is provided
   "as is" without express or implied warranty.
*/

/*
   This file is basicly tis620 character sets with some extra functions
   for tis-620 handling
*/

/*
 * This comment is parsed by configure to create ctype.c,
 * so don't change it unless you know what you are doing.
 *
 * .configure. strxfrm_multiply_tis620=4
 */

#include <string.h>
#include <sys/types.h>

#include <algorithm>

#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "strings/t_ctype.h"

#define M L_MIDDLE
#define U L_UPPER
#define L L_LOWER
#define UU L_UPRUPR
#define X L_MIDDLE

static const int t_ctype[][TOT_LEVELS] = {
    /*0x00*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x01*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x02*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x03*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x04*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x05*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x06*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x07*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x08*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x09*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x0A*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x0B*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x0C*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x0D*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x0E*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x0F*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x10*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x11*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x12*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x13*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x14*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x15*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x16*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x17*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x18*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x19*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x1A*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x1B*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x1C*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x1D*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x1E*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x1F*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x20*/ {IGNORE, IGNORE, L3_SPACE, IGNORE, M},
    /*0x21*/ {IGNORE, IGNORE, L3_EXCLAMATION, IGNORE, M},
    /*0x22*/ {IGNORE, IGNORE, L3_QUOTATION, IGNORE, M},
    /*0x23*/ {IGNORE, IGNORE, L3_NUMBER, IGNORE, M},
    /*0x24*/ {IGNORE, IGNORE, L3_DOLLAR, IGNORE, M},
    /*0x25*/ {IGNORE, IGNORE, L3_PERCENT, IGNORE, M},
    /*0x26*/ {IGNORE, IGNORE, L3_AMPERSAND, IGNORE, M},
    /*0x27*/ {IGNORE, IGNORE, L3_APOSTROPHE, IGNORE, M},
    /*0x28*/ {IGNORE, IGNORE, L3_L_PARANTHESIS, IGNORE, M},
    /*0x29*/ {IGNORE, IGNORE, L3_R_PARENTHESIS, IGNORE, M},
    /*0x2A*/ {IGNORE, IGNORE, L3_ASTERISK, IGNORE, M},
    /*0x2B*/ {IGNORE, IGNORE, L3_PLUS, IGNORE, M},
    /*0x2C*/ {IGNORE, IGNORE, L3_COMMA, IGNORE, M},
    /*0x2D*/ {IGNORE, IGNORE, L3_HYPHEN, IGNORE, M},
    /*0x2E*/ {IGNORE, IGNORE, L3_FULL_STOP, IGNORE, M},
    /*0x2F*/ {IGNORE, IGNORE, L3_SOLIDUS, IGNORE, M},
    /*0x30*/ {L1_08, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x31*/ {L1_18, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x32*/ {L1_28, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x33*/ {L1_38, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x34*/ {L1_48, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x35*/ {L1_58, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x36*/ {L1_68, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x37*/ {L1_78, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x38*/ {L1_88, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x39*/ {L1_98, L2_BLANK, L3_BLANK, L4_BLANK, M},
    /*0x3A*/ {IGNORE, IGNORE, L3_COLON, IGNORE, M},
    /*0x3B*/ {IGNORE, IGNORE, L3_SEMICOLON, IGNORE, M},
    /*0x3C*/ {IGNORE, IGNORE, L3_LESS_THAN, IGNORE, M},
    /*0x3D*/ {IGNORE, IGNORE, L3_EQUAL, IGNORE, M},
    /*0x3E*/ {IGNORE, IGNORE, L3_GREATER_THAN, IGNORE, M},
    /*0x3F*/ {IGNORE, IGNORE, L3_QUESTION, IGNORE, M},
    /*0x40*/ {IGNORE, IGNORE, L3_AT, IGNORE, M},
    /*0x41*/ {L1_A8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x42*/ {L1_B8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x43*/ {L1_C8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x44*/ {L1_D8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x45*/ {L1_E8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x46*/ {L1_F8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x47*/ {L1_G8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x48*/ {L1_H8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x49*/ {L1_I8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x4A*/ {L1_J8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x4B*/ {L1_K8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x4C*/ {L1_L8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x4D*/ {L1_M8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x4E*/ {L1_N8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x4F*/ {L1_O8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x50*/ {L1_P8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x51*/ {L1_Q8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x52*/ {L1_R8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x53*/ {L1_S8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x54*/ {L1_T8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x55*/ {L1_U8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x56*/ {L1_V8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x57*/ {L1_W8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x58*/ {L1_X8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x59*/ {L1_Y8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x5A*/ {L1_Z8, L2_BLANK, L3_BLANK, L4_CAP, M},
    /*0x5B*/ {IGNORE, IGNORE, L3_L_BRACKET, IGNORE, M},
    /*0x5C*/ {IGNORE, IGNORE, L3_BK_SOLIDUS, IGNORE, M},
    /*0x5D*/ {IGNORE, IGNORE, L3_R_BRACKET, IGNORE, M},
    /*0x5E*/ {IGNORE, IGNORE, L3_CIRCUMFLEX, IGNORE, M},
    /*0x5F*/ {IGNORE, IGNORE, L3_LOW_LINE, IGNORE, M},
    /*0x60*/ {IGNORE, IGNORE, L3_GRAVE, IGNORE, M},
    /*0x61*/ {L1_A8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x62*/ {L1_B8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x63*/ {L1_C8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x64*/ {L1_D8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x65*/ {L1_E8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x66*/ {L1_F8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x67*/ {L1_G8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x68*/ {L1_H8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x69*/ {L1_I8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x6A*/ {L1_J8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x6B*/ {L1_K8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x6C*/ {L1_L8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x6D*/ {L1_M8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x6E*/ {L1_N8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x6F*/ {L1_O8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x70*/ {L1_P8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x71*/ {L1_Q8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x72*/ {L1_R8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x73*/ {L1_S8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x74*/ {L1_T8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x75*/ {L1_U8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x76*/ {L1_V8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x77*/ {L1_W8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x78*/ {L1_X8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x79*/ {L1_Y8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x7A*/ {L1_Z8, L2_BLANK, L3_BLANK, L4_MIN, M},
    /*0x7B*/ {IGNORE, IGNORE, L3_L_BRACE, IGNORE, M},
    /*0x7C*/ {IGNORE, IGNORE, L3_V_LINE, IGNORE, M},
    /*0x7D*/ {IGNORE, IGNORE, L3_R_BRACE, IGNORE, M},
    /*0x7E*/ {IGNORE, IGNORE, L3_TILDE, IGNORE, M},
    /*0x7F*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x80*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x81*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x82*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x83*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x84*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x85*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x86*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x87*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x88*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x89*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x8A*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x8B*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x8C*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x8D*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x8E*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x8F*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x90*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x91*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x92*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x93*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x94*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x95*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x96*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x97*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x98*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x99*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x9A*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x9B*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x9C*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x9D*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x9E*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0x9F*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0xA0*/ {IGNORE, IGNORE, L3_NB_SACE, IGNORE, X},
    /*0xA1*/ {L1_KO_KAI, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA2*/ {L1_KHO_KHAI, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA3*/ {L1_KHO_KHUAT, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA4*/ {L1_KHO_KHWAI, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA5*/ {L1_KHO_KHON, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA6*/ {L1_KHO_RAKHANG, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA7*/ {L1_NGO_NGU, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA8*/ {L1_CHO_CHAN, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xA9*/ {L1_CHO_CHING, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xAA*/ {L1_CHO_CHANG, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xAB*/ {L1_SO_SO, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xAC*/ {L1_CHO_CHOE, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xAD*/ {L1_YO_YING, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xAE*/ {L1_DO_CHADA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xAF*/ {L1_TO_PATAK, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB0*/ {L1_THO_THAN, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB1*/ {L1_THO_NANGMONTHO, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB2*/ {L1_THO_PHUTHAO, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB3*/ {L1_NO_NEN, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB4*/ {L1_DO_DEK, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB5*/ {L1_TO_TAO, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB6*/ {L1_THO_THUNG, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB7*/ {L1_THO_THAHAN, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB8*/ {L1_THO_THONG, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xB9*/ {L1_NO_NU, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xBA*/ {L1_BO_BAIMAI, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xBB*/ {L1_PO_PLA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xBC*/ {L1_PHO_PHUNG, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xBD*/ {L1_FO_FA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xBE*/ {L1_PHO_PHAN, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xBF*/ {L1_FO_FAN, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC0*/ {L1_PHO_SAMPHAO, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC1*/ {L1_MO_MA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC2*/ {L1_YO_YAK, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC3*/ {L1_RO_RUA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC4*/ {L1_RU, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC5*/ {L1_LO_LING, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC6*/ {L1_LU, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC7*/ {L1_WO_WAEN, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC8*/ {L1_SO_SALA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xC9*/ {L1_SO_RUSI, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xCA*/ {L1_SO_SUA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xCB*/ {L1_HO_HIP, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xCC*/ {L1_LO_CHULA, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xCD*/ {L1_O_ANG, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xCE*/ {L1_HO_NOKHUK, L2_BLANK, L3_BLANK, L4_BLANK, M | _consnt},
    /*0xCF*/ {IGNORE, IGNORE, L3_PAIYAN_NOI, IGNORE, M},
    /*0xD0*/ {L1_SARA_A, L2_BLANK, L3_BLANK, L4_BLANK, M | _fllwvowel},
    /*0xD1*/ {L1_MAI_HAN_AKAT, L2_BLANK, L3_BLANK, L4_BLANK, U | _uprvowel},
    /*0xD2*/ {L1_SARA_AA, L2_BLANK, L3_BLANK, L4_BLANK, M | _fllwvowel},
    /*0xD3*/ {L1_SARA_AM, L2_BLANK, L3_BLANK, L4_BLANK, M | _fllwvowel},
    /*0xD4*/ {L1_SARA_I, L2_BLANK, L3_BLANK, L4_BLANK, U | _uprvowel},
    /*0xD5*/ {L1_SARA_II, L2_BLANK, L3_BLANK, L4_BLANK, U | _uprvowel},
    /*0xD6*/ {L1_SARA_UE, L2_BLANK, L3_BLANK, L4_BLANK, U | _uprvowel},
    /*0xD7*/ {L1_SARA_UEE, L2_BLANK, L3_BLANK, L4_BLANK, U | _uprvowel},
    /*0xD8*/ {L1_SARA_U, L2_BLANK, L3_BLANK, L4_BLANK, L | _lwrvowel},
    /*0xD9*/ {L1_SARA_UU, L2_BLANK, L3_BLANK, L4_BLANK, L | _lwrvowel},
    /*0xDA*/ {IGNORE, L2_PINTHU, L3_BLANK, L4_BLANK, L},
    /*0xDB*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0xDC*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0xDD*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0xDE*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0xDF*/ {IGNORE, IGNORE, L3_BAHT, IGNORE, M},
    /*0xE0*/ {L1_SARA_E, L2_BLANK, L3_BLANK, L4_BLANK, M | _ldvowel},
    /*0xE1*/ {L1_SARA_AE, L2_BLANK, L3_BLANK, L4_BLANK, M | _ldvowel},
    /*0xE2*/ {L1_SARA_O, L2_BLANK, L3_BLANK, L4_BLANK, M | _ldvowel},
    /*0xE3*/ {L1_SARA_AI_MAIMUAN, L2_BLANK, L3_BLANK, L4_BLANK, M | _ldvowel},
    /*0xE4*/ {L1_SARA_AI_MAIMALAI, L2_BLANK, L3_BLANK, L4_BLANK, M | _ldvowel},
    /*0xE5*/ {L1_SARA_AA, L2_BLANK, L3_BLANK, L4_EXT, M | _fllwvowel},
    /*0xE6*/ {IGNORE, IGNORE, L3_MAI_YAMOK, IGNORE, M | _stone},
    /*0xE7*/ {IGNORE, L2_TYKHU, L3_BLANK, L4_BLANK, U | _diacrt1 | _stone},
    /*0xE8*/
    {IGNORE, L2_TONE1, L3_BLANK, L4_BLANK, UU | _tone | _combine | _stone},
    /*0xE9*/
    {IGNORE, L2_TONE2, L3_BLANK, L4_BLANK, UU | _tone | _combine | _stone},
    /*0xEA*/
    {IGNORE, L2_TONE3, L3_BLANK, L4_BLANK, UU | _tone | _combine | _stone},
    /*0xEB*/
    {IGNORE, L2_TONE4, L3_BLANK, L4_BLANK, UU | _tone | _combine | _stone},
    /*0xEC*/
    {IGNORE, L2_GARAN, L3_BLANK, L4_BLANK, UU | _diacrt2 | _combine | _stone},
    /*0xED*/ {L1_NKHIT, L2_BLANK, L3_BLANK, L4_BLANK, U | _diacrt1},
    /*0xEE*/ {IGNORE, L2_YAMAK, L3_BLANK, L4_BLANK, U | _diacrt1},
    /*0xEF*/ {IGNORE, IGNORE, L3_FONGMAN, IGNORE, M},
    /*0xF0*/ {L1_08, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF1*/ {L1_18, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF2*/ {L1_28, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF3*/ {L1_38, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF4*/ {L1_48, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF5*/ {L1_58, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF6*/ {L1_68, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF7*/ {L1_78, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF8*/ {L1_88, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xF9*/ {L1_98, L2_THAII, L3_BLANK, L4_BLANK, M | _tdig},
    /*0xFA*/ {IGNORE, IGNORE, L3_ANGKHANKHU, IGNORE, X},
    /*0xFB*/ {IGNORE, IGNORE, L3_KHOMUT, IGNORE, X},
    /*0xFC*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0xFD*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /*0xFE*/ {IGNORE, IGNORE, IGNORE, IGNORE, X},
    /* Utilize 0xFF for max_sort_chr in my_like_range_tis620 */
    /*0xFF*/ {255 /*IGNORE*/, IGNORE, IGNORE, IGNORE, X},
};

static const uchar ctype_tis620[257] = {
    0, /* For standard library */
    32,  32,  32,  32,  32,  32,  32,  32,  32,  40,  40, 40, 40, 40, 32, 32,
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32, 32, 32, 32,
    72,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 16, 16, 16, 16, 16,
    132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 16, 16, 16, 16, 16, 16,
    16,  129, 129, 129, 129, 129, 129, 1,   1,   1,   1,  1,  1,  1,  1,  1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  16, 16, 16, 16, 16,
    16,  130, 130, 130, 130, 130, 130, 2,   2,   2,   2,  2,  2,  2,  2,  2,
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  16, 16, 16, 16, 32,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,
};

static const uchar to_lower_tis620[] = {
    '\000',        '\001',        '\002',        '\003',        '\004',
    '\005',        '\006',        '\007',        '\010',        '\011',
    '\012',        '\013',        '\014',        '\015',        '\016',
    '\017',        '\020',        '\021',        '\022',        '\023',
    '\024',        '\025',        '\026',        '\027',        '\030',
    '\031',        '\032',        '\033',        '\034',        '\035',
    '\036',        '\037',        ' ',           '!',           '"',
    '#',           '$',           '%',           '&',           '\'',
    '(',           ')',           '*',           '+',           ',',
    '-',           '.',           '/',           '0',           '1',
    '2',           '3',           '4',           '5',           '6',
    '7',           '8',           '9',           ':',           ';',
    '<',           '=',           '>',           '?',           '@',
    'a',           'b',           'c',           'd',           'e',
    'f',           'g',           'h',           'i',           'j',
    'k',           'l',           'm',           'n',           'o',
    'p',           'q',           'r',           's',           't',
    'u',           'v',           'w',           'x',           'y',
    'z',           '[',           '\\',          ']',           '^',
    '_',           '`',           'a',           'b',           'c',
    'd',           'e',           'f',           'g',           'h',
    'i',           'j',           'k',           'l',           'm',
    'n',           'o',           'p',           'q',           'r',
    's',           't',           'u',           'v',           'w',
    'x',           'y',           'z',           '{',           '|',
    '}',           '~',           '\177',        (uchar)'\200', (uchar)'\201',
    (uchar)'\202', (uchar)'\203', (uchar)'\204', (uchar)'\205', (uchar)'\206',
    (uchar)'\207', (uchar)'\210', (uchar)'\211', (uchar)'\212', (uchar)'\213',
    (uchar)'\214', (uchar)'\215', (uchar)'\216', (uchar)'\217', (uchar)'\220',
    (uchar)'\221', (uchar)'\222', (uchar)'\223', (uchar)'\224', (uchar)'\225',
    (uchar)'\226', (uchar)'\227', (uchar)'\230', (uchar)'\231', (uchar)'\232',
    (uchar)'\233', (uchar)'\234', (uchar)'\235', (uchar)'\236', (uchar)'\237',
    (uchar)'\240', (uchar)'\241', (uchar)'\242', (uchar)'\243', (uchar)'\244',
    (uchar)'\245', (uchar)'\246', (uchar)'\247', (uchar)'\250', (uchar)'\251',
    (uchar)'\252', (uchar)'\253', (uchar)'\254', (uchar)'\255', (uchar)'\256',
    (uchar)'\257', (uchar)'\260', (uchar)'\261', (uchar)'\262', (uchar)'\263',
    (uchar)'\264', (uchar)'\265', (uchar)'\266', (uchar)'\267', (uchar)'\270',
    (uchar)'\271', (uchar)'\272', (uchar)'\273', (uchar)'\274', (uchar)'\275',
    (uchar)'\276', (uchar)'\277', (uchar)'\300', (uchar)'\301', (uchar)'\302',
    (uchar)'\303', (uchar)'\304', (uchar)'\305', (uchar)'\306', (uchar)'\307',
    (uchar)'\310', (uchar)'\311', (uchar)'\312', (uchar)'\313', (uchar)'\314',
    (uchar)'\315', (uchar)'\316', (uchar)'\317', (uchar)'\320', (uchar)'\321',
    (uchar)'\322', (uchar)'\323', (uchar)'\324', (uchar)'\325', (uchar)'\326',
    (uchar)'\327', (uchar)'\330', (uchar)'\331', (uchar)'\332', (uchar)'\333',
    (uchar)'\334', (uchar)'\335', (uchar)'\336', (uchar)'\337', (uchar)'\340',
    (uchar)'\341', (uchar)'\342', (uchar)'\343', (uchar)'\344', (uchar)'\345',
    (uchar)'\346', (uchar)'\347', (uchar)'\350', (uchar)'\351', (uchar)'\352',
    (uchar)'\353', (uchar)'\354', (uchar)'\355', (uchar)'\356', (uchar)'\357',
    (uchar)'\360', (uchar)'\361', (uchar)'\362', (uchar)'\363', (uchar)'\364',
    (uchar)'\365', (uchar)'\366', (uchar)'\367', (uchar)'\370', (uchar)'\371',
    (uchar)'\372', (uchar)'\373', (uchar)'\374', (uchar)'\375', (uchar)'\376',
    (uchar)'\377',
};

static const uchar to_upper_tis620[] = {
    '\000',        '\001',        '\002',        '\003',        '\004',
    '\005',        '\006',        '\007',        '\010',        '\011',
    '\012',        '\013',        '\014',        '\015',        '\016',
    '\017',        '\020',        '\021',        '\022',        '\023',
    '\024',        '\025',        '\026',        '\027',        '\030',
    '\031',        '\032',        '\033',        '\034',        '\035',
    '\036',        '\037',        ' ',           '!',           '"',
    '#',           '$',           '%',           '&',           '\'',
    '(',           ')',           '*',           '+',           ',',
    '-',           '.',           '/',           '0',           '1',
    '2',           '3',           '4',           '5',           '6',
    '7',           '8',           '9',           ':',           ';',
    '<',           '=',           '>',           '?',           '@',
    'A',           'B',           'C',           'D',           'E',
    'F',           'G',           'H',           'I',           'J',
    'K',           'L',           'M',           'N',           'O',
    'P',           'Q',           'R',           'S',           'T',
    'U',           'V',           'W',           'X',           'Y',
    'Z',           '[',           '\\',          ']',           '^',
    '_',           '`',           'A',           'B',           'C',
    'D',           'E',           'F',           'G',           'H',
    'I',           'J',           'K',           'L',           'M',
    'N',           'O',           'P',           'Q',           'R',
    'S',           'T',           'U',           'V',           'W',
    'X',           'Y',           'Z',           '{',           '|',
    '}',           '~',           '\177',        (uchar)'\200', (uchar)'\201',
    (uchar)'\202', (uchar)'\203', (uchar)'\204', (uchar)'\205', (uchar)'\206',
    (uchar)'\207', (uchar)'\210', (uchar)'\211', (uchar)'\212', (uchar)'\213',
    (uchar)'\214', (uchar)'\215', (uchar)'\216', (uchar)'\217', (uchar)'\220',
    (uchar)'\221', (uchar)'\222', (uchar)'\223', (uchar)'\224', (uchar)'\225',
    (uchar)'\226', (uchar)'\227', (uchar)'\230', (uchar)'\231', (uchar)'\232',
    (uchar)'\233', (uchar)'\234', (uchar)'\235', (uchar)'\236', (uchar)'\237',
    (uchar)'\240', (uchar)'\241', (uchar)'\242', (uchar)'\243', (uchar)'\244',
    (uchar)'\245', (uchar)'\246', (uchar)'\247', (uchar)'\250', (uchar)'\251',
    (uchar)'\252', (uchar)'\253', (uchar)'\254', (uchar)'\255', (uchar)'\256',
    (uchar)'\257', (uchar)'\260', (uchar)'\261', (uchar)'\262', (uchar)'\263',
    (uchar)'\264', (uchar)'\265', (uchar)'\266', (uchar)'\267', (uchar)'\270',
    (uchar)'\271', (uchar)'\272', (uchar)'\273', (uchar)'\274', (uchar)'\275',
    (uchar)'\276', (uchar)'\277', (uchar)'\300', (uchar)'\301', (uchar)'\302',
    (uchar)'\303', (uchar)'\304', (uchar)'\305', (uchar)'\306', (uchar)'\307',
    (uchar)'\310', (uchar)'\311', (uchar)'\312', (uchar)'\313', (uchar)'\314',
    (uchar)'\315', (uchar)'\316', (uchar)'\317', (uchar)'\320', (uchar)'\321',
    (uchar)'\322', (uchar)'\323', (uchar)'\324', (uchar)'\325', (uchar)'\326',
    (uchar)'\327', (uchar)'\330', (uchar)'\331', (uchar)'\332', (uchar)'\333',
    (uchar)'\334', (uchar)'\335', (uchar)'\336', (uchar)'\337', (uchar)'\340',
    (uchar)'\341', (uchar)'\342', (uchar)'\343', (uchar)'\344', (uchar)'\345',
    (uchar)'\346', (uchar)'\347', (uchar)'\350', (uchar)'\351', (uchar)'\352',
    (uchar)'\353', (uchar)'\354', (uchar)'\355', (uchar)'\356', (uchar)'\357',
    (uchar)'\360', (uchar)'\361', (uchar)'\362', (uchar)'\363', (uchar)'\364',
    (uchar)'\365', (uchar)'\366', (uchar)'\367', (uchar)'\370', (uchar)'\371',
    (uchar)'\372', (uchar)'\373', (uchar)'\374', (uchar)'\375', (uchar)'\376',
    (uchar)'\377',
};

static const uchar sort_order_tis620[] = {
    '\000',        '\001',        '\002',        '\003',        '\004',
    '\005',        '\006',        '\007',        '\010',        '\011',
    '\012',        '\013',        '\014',        '\015',        '\016',
    '\017',        '\020',        '\021',        '\022',        '\023',
    '\024',        '\025',        '\026',        '\027',        '\030',
    '\031',        '\032',        '\033',        '\034',        '\035',
    '\036',        '\037',        ' ',           '!',           '"',
    '#',           '$',           '%',           '&',           '\'',
    '(',           ')',           '*',           '+',           ',',
    '-',           '.',           '/',           '0',           '1',
    '2',           '3',           '4',           '5',           '6',
    '7',           '8',           '9',           ':',           ';',
    '<',           '=',           '>',           '?',           '@',
    'A',           'B',           'C',           'D',           'E',
    'F',           'G',           'H',           'I',           'J',
    'K',           'L',           'M',           'N',           'O',
    'P',           'Q',           'R',           'S',           'T',
    'U',           'V',           'W',           'X',           'Y',
    'Z',           '\\',          ']',           '[',           '^',
    '_',           'E',           'A',           'B',           'C',
    'D',           'E',           'F',           'G',           'H',
    'I',           'J',           'K',           'L',           'M',
    'N',           'O',           'P',           'Q',           'R',
    'S',           'T',           'U',           'V',           'W',
    'X',           'Y',           'Z',           '{',           '|',
    '}',           'Y',           '\177',        (uchar)'\200', (uchar)'\201',
    (uchar)'\202', (uchar)'\203', (uchar)'\204', (uchar)'\205', (uchar)'\206',
    (uchar)'\207', (uchar)'\210', (uchar)'\211', (uchar)'\212', (uchar)'\213',
    (uchar)'\214', (uchar)'\215', (uchar)'\216', (uchar)'\217', (uchar)'\220',
    (uchar)'\221', (uchar)'\222', (uchar)'\223', (uchar)'\224', (uchar)'\225',
    (uchar)'\226', (uchar)'\227', (uchar)'\230', (uchar)'\231', (uchar)'\232',
    (uchar)'\233', (uchar)'\234', (uchar)'\235', (uchar)'\236', (uchar)'\237',
    (uchar)'\240', (uchar)'\241', (uchar)'\242', (uchar)'\243', (uchar)'\244',
    (uchar)'\245', (uchar)'\246', (uchar)'\247', (uchar)'\250', (uchar)'\251',
    (uchar)'\252', (uchar)'\253', (uchar)'\254', (uchar)'\255', (uchar)'\256',
    (uchar)'\257', (uchar)'\260', (uchar)'\261', (uchar)'\262', (uchar)'\263',
    (uchar)'\264', (uchar)'\265', (uchar)'\266', (uchar)'\267', (uchar)'\270',
    (uchar)'\271', (uchar)'\272', (uchar)'\273', (uchar)'\274', (uchar)'\275',
    (uchar)'\276', (uchar)'\277', (uchar)'\300', (uchar)'\301', (uchar)'\302',
    (uchar)'\303', (uchar)'\304', (uchar)'\305', (uchar)'\306', (uchar)'\307',
    (uchar)'\310', (uchar)'\311', (uchar)'\312', (uchar)'\313', (uchar)'\314',
    (uchar)'\315', (uchar)'\316', (uchar)'\317', (uchar)'\320', (uchar)'\321',
    (uchar)'\322', (uchar)'\323', (uchar)'\324', (uchar)'\325', (uchar)'\326',
    (uchar)'\327', (uchar)'\330', (uchar)'\331', (uchar)'\332', (uchar)'\333',
    (uchar)'\334', (uchar)'\335', (uchar)'\336', (uchar)'\337', (uchar)'\340',
    (uchar)'\341', (uchar)'\342', (uchar)'\343', (uchar)'\344', (uchar)'\345',
    (uchar)'\346', (uchar)'\347', (uchar)'\350', (uchar)'\351', (uchar)'\352',
    (uchar)'\353', (uchar)'\354', (uchar)'\355', (uchar)'\356', (uchar)'\357',
    (uchar)'\360', (uchar)'\361', (uchar)'\362', (uchar)'\363', (uchar)'\364',
    (uchar)'\365', (uchar)'\366', (uchar)'\367', (uchar)'\370', (uchar)'\371',
    (uchar)'\372', (uchar)'\373', (uchar)'\374', (uchar)'\375', (uchar)'\376',
    (uchar)'\377',
};

/*
  Convert thai string to "Standard C String Function" sortable string

  SYNOPSIS
    thai2sortable()
    tstr		String to convert. Does not have to end with \0
    len			Length of tstr
*/

static size_t thai2sortable(uchar *tstr, size_t len) {
  uchar *p;
  size_t tlen;
  uchar l2bias;

  tlen = len;
  l2bias = 256 - 8;
  for (p = tstr; tlen > 0; p++, tlen--) {
    uchar c = *p;

    if (isthai(c)) {
      const int *t_ctype0 = t_ctype[c];

      if (isconsnt(c)) l2bias -= 8;
      if (isldvowel(c) && tlen != 1 && isconsnt(p[1])) {
        /* simply swap between leading-vowel and consonant */
        *p = p[1];
        p[1] = c;
        tlen--;
        p++;
        continue;
      }

      /* if found level 2 char (L2_GARAN,L2_TONE*,L2_TYKHU) move to last */
      if (t_ctype0[1] >= L2_GARAN) {
        /*
          l2bias use to control position weight of l2char
          example (*=l2char) XX*X must come before X*XX
        */
        memmove((char *)p, (char *)(p + 1), tlen - 1);
        tstr[len - 1] = l2bias + t_ctype0[1] - L2_GARAN + 1;
        p--;
        continue;
      }
    } else {
      l2bias -= 8;
      *p = to_lower_tis620[c];
    }
  }
  return len;
}

/*
  strncoll() replacement, compare 2 string, both are converted to sortable
  string

  NOTE:
    We can't cut strings at end \0 as this would break comparision with
    LIKE characters, where the min range is stored as end \0

  Arg: 2 Strings and it compare length
  Ret: strcmp result
*/

extern "C" {
static int my_strnncoll_tis620(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                               const uchar *s1, size_t len1, const uchar *s2,
                               size_t len2, bool s2_is_prefix) {
  uchar buf[80];
  uchar *tc1, *tc2;
  int i;

  if (s2_is_prefix && len1 > len2) len1 = len2;

  tc1 = buf;
  if ((len1 + len2 + 2) > (int)sizeof(buf))
    tc1 = static_cast<uchar *>(my_str_malloc(len1 + len2 + 2));
  tc2 = tc1 + len1 + 1;
  memcpy(tc1, s1, len1);
  tc1[len1] = 0; /* if length(s1)> len1, need to put 'end of string' */
  memcpy(tc2, s2, len2);
  tc2[len2] = 0; /* put end of string */
  thai2sortable(tc1, len1);
  thai2sortable(tc2, len2);
  i = strcmp((char *)tc1, (char *)tc2);
  if (tc1 != buf) my_str_free(tc1);
  return i;
}

static int my_strnncollsp_tis620(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                                 const uchar *a0, size_t a_length,
                                 const uchar *b0, size_t b_length) {
  uchar buf[80], *end, *a, *b, *alloced = nullptr;
  size_t length;
  int res = 0;

  a = buf;
  if ((a_length + b_length + 2) > (int)sizeof(buf))
    alloced = a = (uchar *)my_str_malloc(a_length + b_length + 2);

  b = a + a_length + 1;
  memcpy(a, a0, a_length);
  a[a_length] = 0; /* if length(a0)> len1, need to put 'end of string' */
  memcpy(b, b0, b_length);
  b[b_length] = 0; /* put end of string */
  a_length = thai2sortable(a, a_length);
  b_length = thai2sortable(b, b_length);

  end = a + (length = std::min(a_length, b_length));
  while (a < end) {
    if (*a++ != *b++) {
      res = ((int)a[-1] - (int)b[-1]);
      goto ret;
    }
  }
  if (a_length != b_length) {
    int swap = 1;
    /*
      Check the next not space character of the longer key. If it's < ' ',
      then it's smaller than the other key.
    */
    if (a_length < b_length) {
      /* put shorter key in s */
      a_length = b_length;
      a = b;
      swap = -1; /* swap sign of result */
      res = -res;
    }
    for (end = a + a_length - length; a < end; a++) {
      if (*a != ' ') {
        res = (*a < ' ') ? -swap : swap;
        goto ret;
      }
    }
  }

ret:

  if (alloced) my_str_free(alloced);
  return res;
}

/*
  strnxfrm replacment, convert Thai string to sortable string

  Arg: Destination buffer, source string, dest length and source length
  Ret: Conveted string size
*/

static size_t my_strnxfrm_tis620(const CHARSET_INFO *cs, uchar *dst,
                                 size_t dstlen, uint nweights, const uchar *src,
                                 size_t srclen, uint flags) {
  size_t dstlen0 = dstlen;
  size_t min_len = std::min(dstlen, srclen);
  size_t len = 0;

  /*
    We don't use strmake here, since it requires one more character for
    the terminating '\0', while this function itself and the following calling
    functions do not require it
  */
  while (len < min_len) {
    if (!(dst[len] = src[len])) break;
    len++;
  }

  len = thai2sortable(dst, len);
  dstlen = std::min(dstlen, size_t(nweights));
  len = std::min(len, size_t(dstlen));
  len = my_strxfrm_pad(cs, dst, dst + len, dst + dstlen, (uint)(dstlen - len),
                       flags);
  if ((flags & MY_STRXFRM_PAD_TO_MAXLEN) && len < dstlen0) {
    size_t fill_length = dstlen0 - len;
    cs->cset->fill(cs, (char *)dst + len, fill_length, cs->pad_char);
    len = dstlen0;
  }
  return len;
}
}  // extern "C"

static const unsigned short cs_to_uni[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,
    0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x0010, 0x0011,
    0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A,
    0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022, 0x0023,
    0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C,
    0x002D, 0x002E, 0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
    0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E,
    0x003F, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050,
    0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
    0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 0x0060, 0x0061, 0x0062,
    0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B,
    0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D,
    0x007E, 0x007F, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086,
    0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098,
    0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0xFFFD, 0x0E01,
    0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07, 0x0E08, 0x0E09, 0x0E0A,
    0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F, 0x0E10, 0x0E11, 0x0E12, 0x0E13,
    0x0E14, 0x0E15, 0x0E16, 0x0E17, 0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C,
    0x0E1D, 0x0E1E, 0x0E1F, 0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25,
    0x0E26, 0x0E27, 0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E,
    0x0E2F, 0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37,
    0x0E38, 0x0E39, 0x0E3A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x0E3F, 0x0E40,
    0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47, 0x0E48, 0x0E49,
    0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F, 0x0E50, 0x0E51, 0x0E52,
    0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57, 0x0E58, 0x0E59, 0x0E5A, 0x0E5B,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD};
static const uchar pl00[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,
    0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x0010, 0x0011,
    0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A,
    0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022, 0x0023,
    0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C,
    0x002D, 0x002E, 0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
    0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E,
    0x003F, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050,
    0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
    0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 0x0060, 0x0061, 0x0062,
    0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B,
    0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D,
    0x007E, 0x007F, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086,
    0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098,
    0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000};
static const uchar pl0E[256] = {
    0x0000, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8,
    0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1,
    0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA,
    0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF, 0x00C0, 0x00C1, 0x00C2, 0x00C3,
    0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC,
    0x00CD, 0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5,
    0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0,
    0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9,
    0x00FA, 0x00FB, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000};
static const uchar plFF[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x00FF, 0x0000, 0x0000};
static const uchar *uni_to_cs[256] = {
    pl00,    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, pl0E,    nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, plFF};

extern "C" {
static int my_mb_wc_tis620(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                           my_wc_t *wc, const uchar *str, const uchar *end) {
  if (str >= end) return MY_CS_TOOSMALL;

  *wc = cs_to_uni[*str];
  return (!wc[0] && str[0]) ? -1 : 1;
}

static int my_wc_mb_tis620(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                           my_wc_t wc, uchar *str, uchar *end) {
  const uchar *pl;

  if (str >= end) return MY_CS_TOOSMALL;

  pl = uni_to_cs[(wc >> 8) & 0xFF];
  str[0] = pl ? pl[wc & 0xFF] : '\0';
  return (!str[0] && wc) ? MY_CS_ILUNI : 1;
}
}  // extern "C"

static MY_COLLATION_HANDLER my_collation_ci_handler = {
    nullptr, /* init */
    nullptr,
    my_strnncoll_tis620,
    my_strnncollsp_tis620,
    my_strnxfrm_tis620,
    my_strnxfrmlen_simple,
    my_like_range_simple,
    my_wildcmp_8bit, /* wildcmp   */
    my_strcasecmp_8bit,
    my_instr_simple, /* QQ: To be fixed */
    my_hash_sort_simple,
    my_propagate_simple};

static MY_CHARSET_HANDLER my_charset_handler = {
    nullptr,           /* init */
    nullptr,           /* ismbchar  */
    my_mbcharlen_8bit, /* mbcharlen */
    my_numchars_8bit,
    my_charpos_8bit,
    my_well_formed_len_8bit,
    my_lengthsp_8bit,
    my_numcells_8bit,
    my_mb_wc_tis620, /* mb_wc     */
    my_wc_mb_tis620, /* wc_mb     */
    my_mb_ctype_8bit,
    my_caseup_str_8bit,
    my_casedn_str_8bit,
    my_caseup_8bit,
    my_casedn_8bit,
    my_snprintf_8bit,
    my_long10_to_str_8bit,
    my_longlong10_to_str_8bit,
    my_fill_8bit,
    my_strntol_8bit,
    my_strntoul_8bit,
    my_strntoll_8bit,
    my_strntoull_8bit,
    my_strntod_8bit,
    my_strtoll10_8bit,
    my_strntoull10rnd_8bit,
    my_scan_8bit};

CHARSET_INFO my_charset_tis620_thai_ci = {
    18,
    0,
    0,                                               /* number    */
    MY_CS_COMPILED | MY_CS_PRIMARY | MY_CS_STRNXFRM, /* state     */
    "tis620",                                        /* cs name    */
    "tis620_thai_ci",                                /* name      */
    "TIS620 Thai",                                   /* comment   */
    nullptr,                                         /* tailoring */
    nullptr,                                         /* coll_param */
    ctype_tis620,
    to_lower_tis620,
    to_upper_tis620,
    sort_order_tis620,
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    4,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    1,                   /* mbminlen   */
    1,                   /* mbmaxlen   */
    1,                   /* mbmaxlenlen */
    0,                   /* min_sort_char */
    255,                 /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_handler,
    &my_collation_ci_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_tis620_bin = {
    89,
    0,
    0,                              /* number    */
    MY_CS_COMPILED | MY_CS_BINSORT, /* state     */
    "tis620",                       /* cs name    */
    "tis620_bin",                   /* name      */
    "TIS620 Thai",                  /* comment   */
    nullptr,                        /* tailoring */
    nullptr,                        /* coll_param */
    ctype_tis620,
    to_lower_tis620,
    to_upper_tis620,
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    1,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    1,                   /* mbminlen   */
    1,                   /* mbmaxlen   */
    1,                   /* mbmaxlenlen */
    0,                   /* min_sort_char */
    255,                 /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_handler,
    &my_collation_8bit_bin_handler,
    PAD_SPACE};
