/* Copyright (c) 2004, 2020, Oracle and/or its affiliates. All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License, version 2.0, as published by the Free Software Foundation.

   This library is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the library and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License, version 2.0, for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
   MA 02110-1301  USA */

/*
   UCA (Unicode Collation Algorithm) support.

   Features that are not implemented yet:
   - No Normalization From D is done
     + No decomposition is done
     + No Thai/Lao orderding is done
   - No combining marks processing is done
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <bitset>
#include <iterator>
#include <map>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "mysys_err.h"
#include "strings/mb_wc.h"
#include "strings/str_uca_type.h"
#include "strings/uca900_data.h"
#include "strings/uca900_ja_data.h"
#include "strings/uca900_zh_data.h"
#include "strings/uca_data.h"
#include "template_utils.h"

MY_UCA_INFO my_uca_v400 = {
    UCA_V400,

    0xFFFF,                                 /* maxchar           */
    uca_length, uca_weight, false, nullptr, /* contractions      */
    nullptr,

    /* Logical positions */
    0x0009, /* first_non_ignorable       p != ignore                  */
    0xA48C, /* last_non_ignorable        Not a CJK and not UNASSIGNED */

    0x0332, /* first_primary_ignorable   p == 0                       */
    0x20EA, /* last_primary_ignorable                                 */

    0x0000, /* first_secondary_ignorable p,s == 0                     */
    0xFE73, /* last_secondary_ignorable  p,s == 0                     */

    0x0000, /* first_tertiary_ignorable  p,s,t == 0                   */
    0xFE73, /* last_tertiary_ignorable   p,s,t == 0                   */

    0x0000, /* first_trailing            */
    0x0000, /* last_trailing             */

    0x0009, /* first_variable            */
    0x2183, /* last_variable             */
    0,      /* extra_ce_pri_base, not used */
    0,      /* extra_ce_sec_base, not used */
    0       /* extra_ce_ter_base, not used */
};

/******************************************************/

MY_UCA_INFO my_uca_v520 = {
    UCA_V520,

    0x10FFFF, /* maxchar           */
    uca520_length,
    uca520_weight,
    false,
    nullptr, /* contractions      */
    nullptr,

    0x0009,  /* first_non_ignorable       p != ignore                       */
    0x1342E, /* last_non_ignorable        Not a CJK and not UASSIGNED       */

    0x0332,  /* first_primary_ignorable   p == ignore                       */
    0x101FD, /* last_primary_ignorable                                      */

    0x0000, /* first_secondary_ignorable p,s= ignore                       */
    0xFE73, /* last_secondary_ignorable                                    */

    0x0000, /* first_tertiary_ignorable  p,s,t == ignore                   */
    0xFE73, /* last_tertiary_ignorable                                     */

    0x0000, /* first_trailing                                              */
    0x0000, /* last_trailing                                               */

    0x0009,  /* first_variable            if alt=non-ignorable: p != ignore */
    0x1D371, /* last_variable             if alt=shifter: p,s,t == ignore   */
    0,       /* extra_ce_pri_base, not used */
    0,       /* extra_ce_sec_base, not used */
    0        /* extra_ce_ter_base, not used */
};

/******************************************************/

/*
  German Phonebook
*/
static const char german2[] =
    "&AE << \\u00E6 <<< \\u00C6 << \\u00E4 <<< \\u00C4 "
    "&OE << \\u0153 <<< \\u0152 << \\u00F6 <<< \\u00D6 "
    "&UE << \\u00FC <<< \\u00DC ";

/*
  Some sources treat LETTER A WITH DIARESIS (00E4,00C4)
  secondary greater than LETTER AE (00E6,00C6).
  http://www.evertype.com/alphabets/icelandic.pdf
  http://developer.mimer.com/collations/charts/icelandic.htm

  Other sources do not provide any special rules
  for LETTER A WITH DIARESIS:
  http://www.omniglot.com/writing/icelandic.htm
  http://en.wikipedia.org/wiki/Icelandic_alphabet
  http://oss.software.ibm.com/icu/charts/collation/is.html

  Let's go the first way.
*/

static const char icelandic[] =
    "& A < \\u00E1 <<< \\u00C1 "
    "& D < \\u00F0 <<< \\u00D0 "
    "& E < \\u00E9 <<< \\u00C9 "
    "& I < \\u00ED <<< \\u00CD "
    "& O < \\u00F3 <<< \\u00D3 "
    "& U < \\u00FA <<< \\u00DA "
    "& Y < \\u00FD <<< \\u00DD "
    "& Z < \\u00FE <<< \\u00DE "
    "< \\u00E6 <<< \\u00C6 << \\u00E4 <<< \\u00C4 "
    "< \\u00F6 <<< \\u00D6 << \\u00F8 <<< \\u00D8 "
    "< \\u00E5 <<< \\u00C5 ";

/*
  Some sources treat I and Y primary different.
  Other sources treat I and Y the same on primary level.
  We'll go the first way.
*/

static const char latvian[] =
    "& C < \\u010D <<< \\u010C "
    "& G < \\u0123 <<< \\u0122 "
    "& I < \\u0079 <<< \\u0059 "
    "& K < \\u0137 <<< \\u0136 "
    "& L < \\u013C <<< \\u013B "
    "& N < \\u0146 <<< \\u0145 "
    "& R < \\u0157 <<< \\u0156 "
    "& S < \\u0161 <<< \\u0160 "
    "& Z < \\u017E <<< \\u017D ";

static const char romanian[] =
    "& A < \\u0103 <<< \\u0102 < \\u00E2 <<< \\u00C2 "
    "& I < \\u00EE <<< \\u00CE "
    "& S < \\u0219 <<< \\u0218 << \\u015F <<< \\u015E "
    "& T < \\u021B <<< \\u021A << \\u0163 <<< \\u0162 ";

static const char slovenian[] =
    "& C < \\u010D <<< \\u010C "
    "& S < \\u0161 <<< \\u0160 "
    "& Z < \\u017E <<< \\u017D ";

static const char polish[] =
    "& A < \\u0105 <<< \\u0104 "
    "& C < \\u0107 <<< \\u0106 "
    "& E < \\u0119 <<< \\u0118 "
    "& L < \\u0142 <<< \\u0141 "
    "& N < \\u0144 <<< \\u0143 "
    "& O < \\u00F3 <<< \\u00D3 "
    "& S < \\u015B <<< \\u015A "
    "& Z < \\u017A <<< \\u0179 < \\u017C <<< \\u017B";

static const char estonian[] =
    "& S < \\u0161 <<< \\u0160 "
    " < \\u007A <<< \\u005A "
    " < \\u017E <<< \\u017D "
    "& W < \\u00F5 <<< \\u00D5 "
    "< \\u00E4 <<< \\u00C4 "
    "< \\u00F6 <<< \\u00D6 "
    "< \\u00FC <<< \\u00DC ";

static const char spanish[] = "& N < \\u00F1 <<< \\u00D1 ";

/*
  Some sources treat V and W as similar on primary level.
  We'll treat V and W as different on primary level.
*/

static const char swedish[] =
    "& Y <<\\u00FC <<< \\u00DC "
    "& Z < \\u00E5 <<< \\u00C5 "
    "< \\u00E4 <<< \\u00C4 << \\u00E6 <<< \\u00C6 "
    "< \\u00F6 <<< \\u00D6 << \\u00F8 <<< \\u00D8 ";

static const char turkish[] =
    "& C < \\u00E7 <<< \\u00C7 "
    "& G < \\u011F <<< \\u011E "
    "& H < \\u0131 <<< \\u0049 "
    "& O < \\u00F6 <<< \\u00D6 "
    "& S < \\u015F <<< \\u015E "
    "& U < \\u00FC <<< \\u00DC ";

static const char czech[] =
    "& C < \\u010D <<< \\u010C "
    "& H <      ch <<<      Ch <<< CH"
    "& R < \\u0159 <<< \\u0158"
    "& S < \\u0161 <<< \\u0160"
    "& Z < \\u017E <<< \\u017D";

static const char danish[] = /* Also good for Norwegian */
    "& Y << \\u00FC <<< \\u00DC << \\u0171 <<< \\u0170"
    "& Z  < \\u00E6 <<< \\u00C6 << \\u00E4 <<< \\u00C4"
    " < \\u00F8 <<< \\u00D8 << \\u00F6 <<< \\u00D6 << \\u0151 <<< \\u0150"
    " < \\u00E5 <<< \\u00C5 << aa <<<  Aa <<< AA";

static const char lithuanian[] =
    "& C << ch <<< Ch <<< CH< \\u010D <<< \\u010C"
    "& E << \\u0119 <<< \\u0118 << \\u0117 <<< \\u0116"
    "& I << y <<< Y"
    "& S  < \\u0161 <<< \\u0160"
    "& Z  < \\u017E <<< \\u017D";

static const char slovak[] =
    "& A < \\u00E4 <<< \\u00C4"
    "& C < \\u010D <<< \\u010C"
    "& H < ch <<< Ch <<< CH"
    "& O < \\u00F4 <<< \\u00D4"
    "& S < \\u0161 <<< \\u0160"
    "& Z < \\u017E <<< \\u017D";

static const char spanish2[] = /* Also good for Asturian and Galician */
    "&C <  ch <<< Ch <<< CH"
    "&L <  ll <<< Ll <<< LL"
    "&N < \\u00F1 <<< \\u00D1";

static const char roman[] = /* i.e. Classical Latin */
    "& I << j <<< J "
    "& V << u <<< U ";

/*
  Persian collation support was provided by
  Jody McIntyre <mysql@modernduck.com>

  To: internals@lists.mysql.com
  Subject: Persian UTF8 collation support
  Date: 17.08.2004

  Contraction is not implemented.  Some implementations do perform
  contraction but others do not, and it is able to sort all my test
  strings correctly.

  Jody.
*/
static const char persian[] =
    "& \\u066D < \\u064E < \\uFE76 < \\uFE77 < \\u0650 < \\uFE7A < \\uFE7B"
    " < \\u064F < \\uFE78 < \\uFE79 < \\u064B < \\uFE70 < \\uFE71"
    " < \\u064D < \\uFE74 < \\u064C < \\uFE72"
    "& \\uFE7F < \\u0653 < \\u0654 < \\u0655 < \\u0670"
    "& \\u0669 < \\u0622 < \\u0627 < \\u0671 < \\u0621 < \\u0623 < \\u0625"
    " < \\u0624 < \\u0626"
    "& \\u0642 < \\u06A9 < \\u0643"
    "& \\u0648 < \\u0647 < \\u0629 < \\u06C0 < \\u06CC < \\u0649 < \\u064A"
    "& \\uFE80 < \\uFE81 < \\uFE82 < \\uFE8D < \\uFE8E < \\uFB50 < \\uFB51"
    " < \\uFE80 "
    /*
      FE80 appears both in reset and shift.
      We need to break the rule here and reset to *new* FE80 again,
      so weight for FE83 is calculated as P[FE80]+1, not as P[FE80]+8.
    */
    " & \\uFE80 < \\uFE83 < \\uFE84 < \\uFE87 < \\uFE88 < \\uFE85"
    " < \\uFE86 < \\u0689 < \\u068A"
    "& \\uFEAE < \\uFDFC"
    "& \\uFED8 < \\uFB8E < \\uFB8F < \\uFB90 < \\uFB91 < \\uFED9 < \\uFEDA"
    " < \\uFEDB < \\uFEDC"
    "& \\uFEEE < \\uFEE9 < \\uFEEA < \\uFEEB < \\uFEEC < \\uFE93 < \\uFE94"
    " < \\uFBA4 < \\uFBA5 < \\uFBFC < \\uFBFD < \\uFBFE < \\uFBFF"
    " < \\uFEEF < \\uFEF0 < \\uFEF1 < \\uFEF2 < \\uFEF3 < \\uFEF4"
    " < \\uFEF5 < \\uFEF6 < \\uFEF7 < \\uFEF8 < \\uFEF9 < \\uFEFA"
    " < \\uFEFB < \\uFEFC";

/*
  Esperanto tailoring.
  Contributed by Bertilo Wennergren <bertilow at gmail dot com>
  September 1, 2005
*/
static const char esperanto[] =
    "& C < \\u0109 <<< \\u0108"
    "& G < \\u011D <<< \\u011C"
    "& H < \\u0125 <<< \\u0124"
    "& J < \\u0135 <<< \\u0134"
    "& S < \\u015d <<< \\u015c"
    "& U < \\u016d <<< \\u016c";

/*
  A simplified version of Hungarian, without consonant contractions.
*/
static const char hungarian[] =
    "&O < \\u00F6 <<< \\u00D6 << \\u0151 <<< \\u0150"
    "&U < \\u00FC <<< \\u00DC << \\u0171 <<< \\u0170";

static const char croatian[] =
    "&C < \\u010D <<< \\u010C < \\u0107 <<< \\u0106"
    "&D < d\\u017E = \\u01C6 <<< d\\u017D <<< D\\u017E = \\u01C5 <<< D\\u017D "
    "= \\u01C4"
    "   < \\u0111 <<< \\u0110"
    "&L < lj = \\u01C9  <<< lJ <<< Lj = \\u01C8 <<< LJ = \\u01C7"
    "&N < nj = \\u01CC  <<< nJ <<< Nj = \\u01CB <<< NJ = \\u01CA"
    "&S < \\u0161 <<< \\u0160"
    "&Z < \\u017E <<< \\u017D";

/*
  SCCII Part 1 : Collation Sequence (SLS1134)
  2006/11/24
  Harshula Jayasuriya <harshula at gmail dot com>
  Language Technology Research Lab, University of Colombo / ICTA
*/
#if 0
static const char sinhala[]=
    "& \\u0D96 < \\u0D82 < \\u0D83"
    "& \\u0DA5 < \\u0DA4"
    "& \\u0DD8 < \\u0DF2 < \\u0DDF < \\u0DF3"
    "& \\u0DDE < \\u0DCA";
#else
static const char sinhala[] =
    "& \\u0D96 < \\u0D82 < \\u0D83 < \\u0D9A < \\u0D9B < \\u0D9C < \\u0D9D"
    "< \\u0D9E < \\u0D9F < \\u0DA0 < \\u0DA1 < \\u0DA2 < \\u0DA3"
    "< \\u0DA5 < \\u0DA4 < \\u0DA6"
    "< \\u0DA7 < \\u0DA8 < \\u0DA9 < \\u0DAA < \\u0DAB < \\u0DAC"
    "< \\u0DAD < \\u0DAE < \\u0DAF < \\u0DB0 < \\u0DB1"
    "< \\u0DB3 < \\u0DB4 < \\u0DB5 < \\u0DB6 < \\u0DB7 < \\u0DB8"
    "< \\u0DB9 < \\u0DBA < \\u0DBB < \\u0DBD < \\u0DC0 < \\u0DC1"
    "< \\u0DC2 < \\u0DC3 < \\u0DC4 < \\u0DC5 < \\u0DC6"
    "< \\u0DCF"
    "< \\u0DD0 < \\u0DD1 < \\u0DD2 < \\u0DD3 < \\u0DD4 < \\u0DD6"
    "< \\u0DD8 < \\u0DF2 < \\u0DDF < \\u0DF3 < \\u0DD9 < \\u0DDA"
    "< \\u0DDB < \\u0DDC < \\u0DDD < \\u0DDE < \\u0DCA";
#endif

static const char vietnamese[] =
    " &A << \\u00E0 <<< \\u00C0" /* A */
    " << \\u1EA3 <<< \\u1EA2"
    " << \\u00E3 <<< \\u00C3"
    " << \\u00E1 <<< \\u00C1"
    " << \\u1EA1 <<< \\u1EA0"
    "  < \\u0103 <<< \\u0102" /* A WITH BREVE */
    " << \\u1EB1 <<< \\u1EB0"
    " << \\u1EB3 <<< \\u1EB2"
    " << \\u1EB5 <<< \\u1EB4"
    " << \\u1EAF <<< \\u1EAE"
    " << \\u1EB7 <<< \\u1EB6"
    "  < \\u00E2 <<< \\u00C2" /* A WITH CIRCUMFLEX */
    " << \\u1EA7 <<< \\u1EA6"
    " << \\u1EA9 <<< \\u1EA8"
    " << \\u1EAB <<< \\u1EAA"
    " << \\u1EA5 <<< \\u1EA4"
    " << \\u1EAD <<< \\u1EAC"
    " &D  < \\u0111 <<< \\u0110" /* D WITH STROKE */
    " &E << \\u00E8 <<< \\u00C8" /* E */
    " << \\u1EBB <<< \\u1EBA"
    " << \\u1EBD <<< \\u1EBC"
    " << \\u00E9 <<< \\u00C9"
    " << \\u1EB9 <<< \\u1EB8"
    "  < \\u00EA <<< \\u00CA" /* E WITH CIRCUMFLEX */
    " << \\u1EC1 <<< \\u1EC0"
    " << \\u1EC3 <<< \\u1EC2"
    " << \\u1EC5 <<< \\u1EC4"
    " << \\u1EBF <<< \\u1EBE"
    " << \\u1EC7 <<< \\u1EC6"
    " &I << \\u00EC <<< \\u00CC" /* I */
    " << \\u1EC9 <<< \\u1EC8"
    " << \\u0129 <<< \\u0128"
    " << \\u00ED <<< \\u00CD"
    " << \\u1ECB <<< \\u1ECA"
    " &O << \\u00F2 <<< \\u00D2" /* O */
    " << \\u1ECF <<< \\u1ECE"
    " << \\u00F5 <<< \\u00D5"
    " << \\u00F3 <<< \\u00D3"
    " << \\u1ECD <<< \\u1ECC"
    "  < \\u00F4 <<< \\u00D4" /* O WITH CIRCUMFLEX */
    " << \\u1ED3 <<< \\u1ED2"
    " << \\u1ED5 <<< \\u1ED4"
    " << \\u1ED7 <<< \\u1ED6"
    " << \\u1ED1 <<< \\u1ED0"
    " << \\u1ED9 <<< \\u1ED8"
    "  < \\u01A1 <<< \\u01A0" /* O WITH HORN */
    " << \\u1EDD <<< \\u1EDC"
    " << \\u1EDF <<< \\u1EDE"
    " << \\u1EE1 <<< \\u1EE0"
    " << \\u1EDB <<< \\u1EDA"
    " << \\u1EE3 <<< \\u1EE2"
    " &U << \\u00F9 <<< \\u00D9" /* U */
    " << \\u1EE7 <<< \\u1EE6"
    " << \\u0169 <<< \\u0168"
    " << \\u00FA <<< \\u00DA"
    " << \\u1EE5 <<< \\u1EE4"
    "  < \\u01B0 <<< \\u01AF" /* U WITH HORN */
    " << \\u1EEB <<< \\u1EEA"
    " << \\u1EED <<< \\u1EEC"
    " << \\u1EEF <<< \\u1EEE"
    " << \\u1EE9 <<< \\u1EE8"
    " << \\u1EF1 <<< \\u1EF0"
    " &Y << \\u1EF3 <<< \\u1EF2" /* Y */
    " << \\u1EF7 <<< \\u1EF6"
    " << \\u1EF9 <<< \\u1EF8"
    " << \\u00FD <<< \\u00DD"
    " << \\u1EF5 <<< \\u1EF4";

/* German Phonebook */
static const char de_pb_cldr_30[] =
    "&AE << \\u00E4 <<< \\u00C4 "
    "&OE << \\u00F6 <<< \\u00D6 "
    "&UE << \\u00FC <<< \\u00DC ";

/* Icelandic */
static const char is_cldr_30[] =
    "&[before 1]b       <  \\u00E1 <<< \\u00C1 "
    "&          d       << \\u0111 <<< \\u0110 < \\u00F0 <<< \\u00D0 "
    "&[before 1]f       <  \\u00E9 <<< \\u00C9 "
    "&[before 1]j       <  \\u00ED <<< \\u00CD "
    "&[before 1]p       <  \\u00F3 <<< \\u00D3 "
    "&[before 1]v       <  \\u00FA <<< \\u00DA "
    "&[before 1]z       <  \\u00FD <<< \\u00DD "
    "&[before 1]\\u01C0 <  \\u00E6 <<< \\u00C6 << \\u00E4 <<< \\u00C4 "
    "<  \\u00F6 <<< \\u00D6 << \\u00F8 <<< \\u00D8 "
    "<  \\u00E5 <<< \\u00C5";

/* Latvian */
static const char lv_cldr_30[] =
    "&[before 1]D       <  \\u010D <<< \\u010C "
    "&[before 1]H       <  \\u0123 <<< \\u0122 "
    "&          I       << y       <<< Y "
    "&[before 1]L       <  \\u0137 <<< \\u0136 "
    "&[before 1]M       <  \\u013C <<< \\u013B "
    "&[before 1]O       <  \\u0146 <<< \\u0145 "
    "&[before 1]S       <  \\u0157 <<< \\u0156 "
    "&[before 1]T       <  \\u0161 <<< \\u0160 "
    "&[before 1]\\u01B7 <  \\u017E <<< \\u017D";

/* Romanian */
static const char ro_cldr_30[] =
    "&A < \\u0103 <<< \\u0102 <   \\u00E2 <<< \\u00C2 "
    "&I < \\u00EE <<< \\u00CE "
    "&S < \\u015F =   \\u0219 <<< \\u015E =   \\u0218 "
    "&T < \\u0163 =   \\u021B <<< \\u0162 =   \\u021A";

/* Slovenian */
static const char sl_cldr_30[] =
    "&C < \\u010D <<< \\u010C < \\u0107 <<< \\u0106 "
    "&D < \\u0111 <<< \\u0110 "
    "&S < \\u0161 <<< \\u0160 "
    "&Z < \\u017E <<< \\u017D";

/* Polish */
static const char pl_cldr_30[] =
    "&A < \\u0105 <<< \\u0104 "
    "&C < \\u0107 <<< \\u0106 "
    "&E < \\u0119 <<< \\u0118 "
    "&L < \\u0142 <<< \\u0141 "
    "&N < \\u0144 <<< \\u0143 "
    "&O < \\u00F3 <<< \\u00D3 "
    "&S < \\u015B <<< \\u015A "
    "&Z < \\u017A <<< \\u0179 < \\u017C <<< \\u017B";

/* Estonian */
static const char et_cldr_30[] =
    "&[before 1]T <   \\u0161 <<< \\u0160 < z         <<< Z "
    "<   \\u017E <<< \\u017D "
    "&[before 1]X <   \\u00F5 <<< \\u00D5 <   \\u00E4 <<< \\u00C4 "
    "<   \\u00F6 <<< \\u00D6 <   \\u00FC <<< \\u00DC";

/* Swedish */
static const char sv_cldr_30[] =
    "&          D       <<  \\u0111   <<< \\u0110 <<  \\u00F0 <<< \\u00D0 "
    "&          t       <<< \\u00FE/h "
    "&          T       <<< \\u00DE/H "
    "&          Y       <<  \\u00FC   <<< \\u00DC <<  \\u0171 <<< \\u0170 "
    "&[before 1]\\u01C0 <   \\u00E5   <<< \\u00C5 <   \\u00E4 <<< \\u00C4 "
    "<< \\u00E6   <<< \\u00C6 <<  \\u0119 <<< \\u0118 "
    "<  \\u00F6   <<< \\u00D6 <<  \\u00F8 <<< \\u00D8 "
    "<< \\u0151   <<< \\u0150 <<  \\u0153 <<< \\u0152 "
    "<< \\u00F4   <<< \\u00D4";

/* Turkish */
static const char tr_cldr_30[] =
    "&          C <   \\u00E7 <<< \\u00C7 "
    "&          G <   \\u011F <<< \\u011E "
    "&[before 1]i <   \\u0131 <<< I "
    "&          i <<< \\u0130 "
    "&          O <   \\u00F6 <<< \\u00D6 "
    "&          S <   \\u015F <<< \\u015E "
    "&          U <   \\u00FC <<< \\u00DC ";

/* Czech */
static const char cs_cldr_30[] =
    "&C < \\u010D <<< \\u010C "
    "&H < ch      <<< cH       <<< Ch <<< CH "
    "&R < \\u0159 <<< \\u0158"
    "&S < \\u0161 <<< \\u0160"
    "&Z < \\u017E <<< \\u017D";

/* Danish */
static const char da_cldr_30[] =
    "&          D       <<  \\u0111   <<< \\u0110 <<  \\u00F0 <<< \\u00D0 "
    "&          t       <<< \\u00FE/h "
    "&          T       <<< \\u00DE/H "
    "&          Y       <<  \\u00FC   <<< \\u00DC <<  \\u0171 <<< \\u0170 "
    "&[before 1]\\u01C0 <   \\u00E6   <<< \\u00C6 <<  \\u00E4 <<< \\u00C4 "
    "<   \\u00F8   <<< \\u00D8 <<  \\u00F6 <<< \\u00D6 "
    "<<  \\u0151   <<< \\u0150 <<  \\u0153 <<< \\u0152 "
    "<   \\u00E5   <<< \\u00C5 <<< aa      <<< Aa "
    "<<< AA";

static Coll_param da_coll_param = {nullptr, false, CASE_FIRST_UPPER};

/* Lithuanian */
static const char lt_cldr_30[] =
    "&\\u0300 = \\u0307\\u0300 "
    "&\\u0301 = \\u0307\\u0301 "
    "&\\u0303 = \\u0307\\u0303 "
    "&A << \\u0105 <<< \\u0104 "
    "&C <  \\u010D <<< \\u010C "
    "&E << \\u0119 <<< \\u0118 << \\u0117 <<< \\u0116"
    "&I << \\u012F <<< \\u012E << y       <<< Y "
    "&S <  \\u0161 <<< \\u0160 "
    "&U << \\u0173 <<< \\u0172 << \\u016B <<< \\u016A "
    "&Z <  \\u017E <<< \\u017D";

/* Slovak */
static const char sk_cldr_30[] =
    "&A < \\u00E4 <<< \\u00C4 "
    "&C < \\u010D <<< \\u010C "
    "&H < ch      <<< cH      <<< Ch <<< CH "
    "&O < \\u00F4 <<< \\u00D4 "
    "&R < \\u0159 <<< \\u0158 "
    "&S < \\u0161 <<< \\u0160 "
    "&Z < \\u017E <<< \\u017D";

/* Spanish (Traditional) */
static const char es_trad_cldr_30[] =
    "&N <  \\u00F1 <<< \\u00D1 "
    "&C <  ch      <<< Ch      <<< CH "
    "&l <  ll      <<< Ll      <<< LL";

/* Persian */
#if 0
static const char fa_cldr_30[]=
  "&          \\u064E << \\u0650 << \\u064F <<  \\u064B << \\u064D "
                     "<< \\u064C "
  "&[before 1]\\u0627 <  \\u0622 "
  "&          \\u0627 << \\u0671 <  \\u0621 <<  \\u0623 << \\u0672 "
                     "<< \\u0625 << \\u0673 <<  \\u0624 << \\u06CC\\u0654 "
                     "<<< \\u0649\\u0654    <<< \\u0626 "
  "&          \\u06A9 << \\u06AA << \\u06AB <<  \\u0643 << \\u06AC "
                     "<< \\u06AD << \\u06AE "
  "&          \\u06CF <  \\u0647 << \\u06D5 <<  \\u06C1 << \\u0629 "
                     "<< \\u06C3 << \\u06C0 <<  \\u06BE "
  "&          \\u06CC << \\u0649 << \\u06D2 <<  \\u064A << \\u06D0 "
                     "<< \\u06D1 << \\u06CD <<  \\u06CE";

static Reorder_param fa_reorder_param= {
  {CHARGRP_ARAB, CHARGRP_NONE}, {{{0, 0}, {0, 0}}}, 0
};

static Coll_param fa_coll_param= {
  &fa_reorder_param, true
};
#endif

/* Hungarian */
static const char hu_cldr_30[] =
    "&C  <   cs      <<< Cs      <<< CS "
    "&D  <   dz      <<< Dz      <<< DZ "
    "&DZ <   dzs     <<< Dzs     <<< DZS "
    "&G  <   gy      <<< Gy      <<< GY "
    "&L  <   ly      <<< Ly      <<< LY "
    "&N  <   ny      <<< Ny      <<< NY "
    "&S  <   sz      <<< Sz      <<< SZ "
    "&T  <   ty      <<< Ty      <<< TY "
    "&Z  <   zs      <<< Zs      <<< ZS "
    "&O  <   \\u00F6 <<< \\u00D6 <<  \\u0151 <<< \\u0150 "
    "&U  <   \\u00FC <<< \\u00DC <<  \\u0171 <<< \\u0170 "
    "&cs <<< ccs/cs "
    "&Cs <<< Ccs/cs "
    "&CS <<< CCS/CS "
    "&dz <<< ddz/dz "
    "&Dz <<< Ddz/dz "
    "&DZ <<< DDZ/DZ "
    "&dzs<<< ddzs/dzs "
    "&Dzs<<< Ddzs/dzs "
    "&DZS<<< DDZS/DZS "
    "&gy <<< ggy/gy "
    "&Gy <<< Ggy/gy "
    "&GY <<< GGY/GY "
    "&ly <<< lly/ly "
    "&Ly <<< Lly/ly "
    "&LY <<< LLY/LY "
    "&ny <<< nny/ny "
    "&Ny <<< Nny/ny "
    "&NY <<< NNY/NY "
    "&sz <<< ssz/sz "
    "&Sz <<< Ssz/sz "
    "&SZ <<< SSZ/SZ "
    "&ty <<< tty/ty "
    "&Ty <<< Tty/ty "
    "&TY <<< TTY/TY "
    "&zs <<< zzs/zs "
    "&Zs <<< Zzs/zs "
    "&ZS <<< ZZS/ZS";

/* Croatian */
static const char hr_cldr_30[] =
    "&C <   \\u010D  <<< \\u010C <   \\u0107  <<< \\u0106 "
    "&D <   d\\u017E <<< \\u01C6 <<< D\\u017E <<< \\u01C5 <<< D\\u017D "
    "<<< \\u01C4  <   \\u0111 <<< \\u0110 "
    "&L <   lj       <<< \\u01C9 <<< Lj       <<< \\u01C8 <<< LJ "
    "<<< \\u01C7 "
    "&N <   nj       <<< \\u01CC <<< Nj       <<< \\u01CB <<< NJ "
    "<<< \\u01CA "
    "&S <   \\u0161  <<< \\u0160 "
    "&Z <   \\u017E  <<< \\u017D ";

static Reorder_param hr_reorder_param = {
    {CHARGRP_LATIN, CHARGRP_CYRILLIC, CHARGRP_NONE}, {{{0, 0}, {0, 0}}}, 0, 0};

static Coll_param hr_coll_param = {&hr_reorder_param, false, CASE_FIRST_OFF};

/* Sinhala */
#if 0
static const char si_cldr_30[]=
  "&\\u0D96 < \\u0D82 < \\u0D83 "
  "&\\u0DA5 < \\u0DA4";
#endif

/* Vietnamese */
static const char vi_cldr_30[] =
    "&\\u0300 << \\u0309 <<  \\u0303 << \\u0301 <<  \\u0323 "
    "&a       < \\u0103 <<< \\u0102 <  \\u00E2 <<< \\u00C2 "
    "&d       < \\u0111 <<< \\u0110 "
    "&e       < \\u00EA <<< \\u00CA "
    "&o       < \\u00F4 <<< \\u00D4 <  \\u01A1 <<< \\u01A0 "
    "&u       < \\u01B0 <<< \\u01AF";

static Coll_param vi_coll_param = {nullptr, true, CASE_FIRST_OFF};

static Reorder_param ja_reorder_param = {
    /*
      Per CLDR 30, Japanese reorder rule is defined as [Latn Kana Hani],
      but for Hani characters, their weight is implicit according to UCA,
      which is different from other character groups. We don't add "Hani"
      below and will have special handling for them in
      adjust_japanese_weight() and apply_reorder_param(). Implicit weight
      has two collation elements. To make strnxfrm() run faster, we give
      Japanese Han characters tailored weight which has only one collation
      element. These characters' weight is defined in ja_han_pages.
    */
    {CHARGRP_LATIN, CHARGRP_KANA, CHARGRP_NONE},
    {{{0, 0}, {0, 0}}},
    0,
    0};

static Coll_param ja_coll_param = {&ja_reorder_param, false /*norm_enabled*/,
                                   CASE_FIRST_OFF};

/*
  The Chinese reorder rule is defined as [Hani]. This means all Han characters'
  weight should be greater than the core group and smaller than any other
  character groups.
  The Han characters are separated into two parts. The CLDR collation
  definition file, zh.xml, defines 41336 Han characters' order, and all other
  Han characters have implicit weight.
  Since the core group characters occupy the weight value from 0x0209 to 0x1C46
  in DUCET, so we decide to set the weight of all Han characters defined in
  zh.xml to be the value from 0x1C47 to 0xBDBE. The smallest weight value of
  these Han characters, 0x1C47, being the largest weight value of the core
  group plus one (0x1C46 + 1), ensures these Han characters sort greater than
  the core group characters.
  Also, we set the implicit weight to the Han characters like
  [BDBF - BDC3, 0020, 0002][XXXX, 0000, 0000].
  To tailor the weight of characters of Latin, Cyrillic and so on to be bigger
  than all Han characters, we give these characters weights from 0xBDC4 to
  0xF620. There are many character groups between the core group and the Han
  group, so it would be a long list if we put them in the following reorder_grp
  structure. But since it is a very simple weight shift, we put their calculated
  weight here and do not calculate it in my_prepare_reorder().

  NOTE: We use the zh.xml file from CLDR v33.1 to implement this Chinese
  collation, because we found that the file of CLDR v30 is missing some very
  common Han characters (the Han character 'small', etc).
 */
static Reorder_param zh_reorder_param = {
    {CHARGRP_NONE}, {{{0x1C47, 0x54A3}, {0xBDC4, 0xF620}}}, 1, 0x54A3};

static Coll_param zh_coll_param = {&zh_reorder_param, false, CASE_FIRST_OFF};

/* Russian */
static Reorder_param ru_reorder_param = {
    {CHARGRP_CYRILLIC, CHARGRP_NONE}, {{{0, 0}, {0, 0}}}, 0, 0};

static Coll_param ru_coll_param = {&ru_reorder_param, false /*norm_enabled*/,
                                   CASE_FIRST_OFF};

static constexpr uint16 nochar[] = {0, 0};

/**
  Unicode Collation Algorithm:
  Collation element (weight) scanner,
  for consequent scan of collations
  weights from a string.

  Only meant as a base class; instantiate uca_scanner_any or uca_scanner_900
  instead of this.
*/
class my_uca_scanner {
 protected:
  my_uca_scanner(const CHARSET_INFO *cs_arg, const uchar *str, size_t length)
      : wbeg(nochar),
        sbeg(str),
        send(str + length),
        uca(cs_arg->uca),
        cs(cs_arg),
        sbeg_dup(str) {}

 public:
  /**
    Get the level the scanner is currently working on. The string
    can be scanned multiple times (if the collation requires multi-level
    comparisons, e.g. for accent or case sensitivity); first to get
    primary weights, then from the start again for secondary, etc.
  */
  uint get_weight_level() const { return weight_lv; }

 protected:
  uint weight_lv{0};   /* 0 = Primary, 1 = Secondary, 2 = Tertiary */
  const uint16 *wbeg;  /* Beginning of the current weight string */
  uint wbeg_stride{0}; /* Number of bytes between weights in string */
  const uchar *sbeg;   /* Beginning of the input string          */
  const uchar *send;   /* End of the input string                */
  const MY_UCA_INFO *uca;
  uint16 implicit[10];
  my_wc_t prev_char{0};  // Previous code point we scanned, if any.
  const CHARSET_INFO *cs;
  uint num_of_ce_left{0};
  const uchar *sbeg_dup; /* Backup of beginning of input string */

 protected:
  const uint16 *contraction_find(my_wc_t wc0, size_t *chars_skipped);
  inline const uint16 *previous_context_find(my_wc_t wc0, my_wc_t wc1);
};

/*
  Charset dependent scanner part, to optimize
  some character sets.
*/

template <class Mb_wc>
struct uca_scanner_any : public my_uca_scanner {
  uca_scanner_any(const Mb_wc mb_wc, const CHARSET_INFO *cs_arg,
                  const uchar *str, size_t length)
      : my_uca_scanner(cs_arg, str, length), mb_wc(mb_wc) {
    // UCA 9.0.0 uses a different table format from what this scanner expects.
    DBUG_ASSERT(cs_arg->uca == nullptr || cs_arg->uca->version != UCA_V900);
  }

  uint get_char_index() const { return char_index; }

  inline int next();

 private:
  /**
    How many code points (possibly multibyte) we have scanned so far.
    This includes code points with zero weight. Note that this is reset
    once we get to the end of the string and restart the scanning for
    the next weight level, but it is _not_ reset when we reach the
    end of the last level.
  */
  uint char_index{0};

  const Mb_wc mb_wc;

  inline int next_implicit(my_wc_t ch);
};

template <class Mb_wc, int LEVELS_FOR_COMPARE>
class uca_scanner_900 : public my_uca_scanner {
 public:
  uca_scanner_900(const Mb_wc mb_wc, const CHARSET_INFO *cs_arg,
                  const uchar *str, size_t length)
      : my_uca_scanner(cs_arg, str, length), mb_wc(mb_wc) {}

  inline int next();

  /**
    For each weight in sequence, call "func", which should have
    a function signature of "bool func(int weight, bool is_level_separator)".
    Stops the iteration early if "func" returns false.

    This is morally equivalent to

      int weight;
      while ((weight= next()) >= 0)
      {
        if (!func(weight, weight == 0)) break;
      }

    except that it might employ optimizations internally to speed up
    the process. These optimizations will not modify the number of calls
    to func() (or their order), but might affect the internal scanner
    state during the calls, so func() should not try to read from
    the scanner except by calling public member functions.

    As a special optimization, if "bool preaccept_data(int num_weights)"
    returns true, the next "num_weights" calls to func() _must_ return
    true. This is so that bounds checking costs can be amortized
    over fewer calls.
  */
  template <class T, class U>
  inline void for_each_weight(T func, U preaccept_data);

 private:
  const Mb_wc mb_wc;

  inline int next_raw();
  inline int more_weight();
  uint16 apply_case_first(uint16 weight);
  uint16 apply_reorder_param(uint16 weight);
  inline int next_implicit(my_wc_t ch);
  void my_put_jamo_weights(my_wc_t *hangul_jamo, int jamo_cnt);
  /*
    apply_reorder_param() needs to return two weights for each origin
    weight. This boolean signals whether we have already returned the
    FB86 weight, and are ready to return the origin weight.
  */
  bool return_origin_weight{true};
  /*
    For Japanese kana-sensitive collation, we only add quaternary
    weight for katakana and hiragana, but not for others like latin
    and kanji, because characters like latin and kanji can be already
    distinguished from kana by three levels of weight.
    has_quaternary_weight is to indicate whether quaternary weight is
    needed for characters in string.
  */
  bool has_quaternary_weight{false};
  int handle_ja_contraction_quat_wt();
  int handle_ja_common_quat_wt(my_wc_t wc);
};

/********** Helper functions to handle contraction ************/

/**
  Mark a code point as a contraction part

  @param flags    Pointer to UCA contraction flag data
  @param wc       Unicode code point
  @param flag     flag: "is contraction head", "is contraction tail"
*/

static inline void my_uca_add_contraction_flag(char *flags, my_wc_t wc,
                                               int flag) {
  flags[wc & MY_UCA_CNT_FLAG_MASK] |= flag;
}

/**
  Check if UCA level data has contractions.

  @param uca      Pointer to UCA data

  @return   Flags indicating if UCA with contractions
  @retval   0 - no contractions
  @retval   1 - there are some contractions
*/

static inline bool my_uca_have_contractions(const MY_UCA_INFO *uca) {
  return uca->have_contractions;
}

struct trie_node_cmp {
  bool operator()(const MY_CONTRACTION &a, const my_wc_t b) { return a.ch < b; }
  bool operator()(const MY_CONTRACTION &a, const MY_CONTRACTION &b) {
    return a.ch < b.ch;
  }
};

static std::vector<MY_CONTRACTION>::const_iterator
find_contraction_part_in_trie(const std::vector<MY_CONTRACTION> &cont_nodes,
                              my_wc_t ch) {
  if (cont_nodes.empty()) return cont_nodes.end();
  return std::lower_bound(cont_nodes.begin(), cont_nodes.end(), ch,
                          trie_node_cmp());
}

static std::vector<MY_CONTRACTION>::iterator find_contraction_part_in_trie(
    std::vector<MY_CONTRACTION> &cont_nodes, my_wc_t ch) {
  if (cont_nodes.empty()) return cont_nodes.end();
  return std::lower_bound(cont_nodes.begin(), cont_nodes.end(), ch,
                          trie_node_cmp());
}
/**
  Find a contraction consisting of two code points and return its weight array

  @param cont_nodes Vector that contains contraction nodes
  @param wc1        First code point
  @param wc2        Second code point

  @return   Weight array
  @retval   NULL - no contraction found
  @retval   ptr  - contraction weight array
*/

const uint16 *my_uca_contraction2_weight(
    const std::vector<MY_CONTRACTION> *cont_nodes, my_wc_t wc1, my_wc_t wc2) {
  if (!cont_nodes) return nullptr;

  if (!cont_nodes->empty()) {
    std::vector<MY_CONTRACTION>::const_iterator node_it1 =
        find_contraction_part_in_trie(*cont_nodes, wc1);
    if (node_it1 == cont_nodes->end() || node_it1->ch != wc1) return nullptr;
    std::vector<MY_CONTRACTION>::const_iterator node_it2 =
        find_contraction_part_in_trie(node_it1->child_nodes, wc2);
    if (node_it2 != node_it1->child_nodes.end() && node_it2->ch == wc2 &&
        node_it2->is_contraction_tail)
      return node_it2->weight;
  }
  return nullptr;
}

/**
  Check if a code point can be previous context head

  @param flags    Pointer to UCA contraction flag data
  @param wc       Code point

  @retval false - cannot be previous context head
  @retval true  - can be previous context head
*/

static inline bool my_uca_can_be_previous_context_head(const char *flags,
                                                       my_wc_t wc) {
  return flags[wc & MY_UCA_CNT_FLAG_MASK] & MY_UCA_PREVIOUS_CONTEXT_HEAD;
}

/**
  Check if a code point can be previous context tail

  @param flags    Pointer to UCA contraction flag data
  @param wc       Code point

  @retval false - cannot be contraction tail
  @retval true - can be contraction tail
*/

static inline bool my_uca_can_be_previous_context_tail(const char *flags,
                                                       my_wc_t wc) {
  return flags[wc & MY_UCA_CNT_FLAG_MASK] & MY_UCA_PREVIOUS_CONTEXT_TAIL;
}

/**
  Check if a string is a contraction of exactly the given length,
  and return its weight array on success.

  @param cont_nodes Vector that contains contraction nodes
  @param wc         Pointer to wide string
  @param len        String length

  @return       Weight array
  @retval       NULL - Input string is not a known contraction
  @retval       ptr  - contraction weight array
*/

static inline const uint16 *my_uca_contraction_weight(
    const std::vector<MY_CONTRACTION> *cont_nodes, const my_wc_t *wc,
    size_t len) {
  if (!cont_nodes) return nullptr;

  std::vector<MY_CONTRACTION>::const_iterator node_it;
  for (size_t ch_ind = 0; ch_ind < len; ++ch_ind) {
    node_it = find_contraction_part_in_trie(*cont_nodes, wc[ch_ind]);
    if (node_it == cont_nodes->end() || node_it->ch != wc[ch_ind])
      return nullptr;
    cont_nodes = &node_it->child_nodes;
  }
  if (node_it->is_contraction_tail) return node_it->weight;
  return nullptr;
}

/**
  Return length of a 0-terminated wide string, analogous to strnlen().

  @param  s       Pointer to wide string
  @param  maxlen  Mamixum string length

  @return         string length, or maxlen if no '\0' is met.
*/
static size_t my_wstrnlen(my_wc_t *s, size_t maxlen) {
  for (size_t i = 0; i < maxlen; i++) {
    if (s[i] == 0) return i;
  }
  return maxlen;
}

/**
  Find a contraction in the input stream and return its weight array

  Scan input code points to find a longest path in contraction trie
  which contains all these code points. If the ending node of this
  path is end of contraction, return the weight array.

  @param wc0 The first code point of the contraction (which should have
    the MY_UCA_CNT_HEAD flag).
  @param[out] chars_skipped How many code points where skipped in the
    contraction we found. Only makes sense if we actually found one.

  @return         Weight array
  @retval         NULL no contraction found
  @retval         ptr  contraction weight array
*/

const uint16 *my_uca_scanner::contraction_find(my_wc_t wc0,
                                               size_t *chars_skipped) {
  const uchar *beg = nullptr;
  auto mb_wc = cs->cset->mb_wc;

  const uchar *s = sbeg;
  const std::vector<MY_CONTRACTION> *cont_nodes = uca->contraction_nodes;
  const MY_CONTRACTION *longest_contraction = nullptr;
  std::vector<MY_CONTRACTION>::const_iterator node_it;
  for (;;) {
    node_it = find_contraction_part_in_trie(*cont_nodes, wc0);
    if (node_it == cont_nodes->end() || node_it->ch != wc0) break;
    if (node_it->is_contraction_tail) {
      longest_contraction = &(*node_it);
      beg = s;
      *chars_skipped = node_it->contraction_len - 1;
    }
    int mblen;
    if ((mblen = mb_wc(cs, &wc0, s, send)) <= 0) break;
    s += mblen;
    cont_nodes = &node_it->child_nodes;
  }

  if (longest_contraction != nullptr) {
    const uint16 *cweight = longest_contraction->weight;
    if (uca->version == UCA_V900) {
      cweight += weight_lv;
      wbeg = cweight + MY_UCA_900_CE_SIZE;
      wbeg_stride = MY_UCA_900_CE_SIZE;
      num_of_ce_left = 7;
    } else {
      wbeg = cweight + 1;
      wbeg_stride = MY_UCA_900_CE_SIZE;
    }
    sbeg = beg;
    return cweight;
  }
  return nullptr; /* No contractions were found */
}

/**
  Find weight for contraction with previous context
  and return its weight array.

  @param wc0      Previous code point
  @param wc1      Current code point

  @return   Weight array
  @retval   NULL - no contraction with context found
  @retval   ptr  - contraction weight array
*/
ALWAYS_INLINE
const uint16 *my_uca_scanner::previous_context_find(my_wc_t wc0, my_wc_t wc1) {
  std::vector<MY_CONTRACTION>::const_iterator node_it1 =
      find_contraction_part_in_trie(*uca->contraction_nodes, wc1);
  if (node_it1 == uca->contraction_nodes->end() || node_it1->ch != wc1)
    return nullptr;
  std::vector<MY_CONTRACTION>::const_iterator node_it2 =
      find_contraction_part_in_trie(node_it1->child_nodes_context, wc0);
  if (node_it2 != node_it1->child_nodes_context.end() && node_it2->ch == wc0) {
    if (uca->version == UCA_V900) {
      wbeg = node_it2->weight + MY_UCA_900_CE_SIZE + weight_lv;
      wbeg_stride = MY_UCA_900_CE_SIZE;
      num_of_ce_left = 7;
    } else {
      wbeg = node_it2->weight + 1;
      wbeg_stride = MY_UCA_900_CE_SIZE;
    }
    return node_it2->weight + weight_lv;
  }
  return nullptr;
}

/****************************************************************/
#define HANGUL_JAMO_MAX_LENGTH 3
/**
  Check if a code point is Hangul syllable. Decompose it to jamos
  if it is, and return tailored weights.

  @param       syllable    Hangul syllable to be decomposed
  @param[out]  jamo        Corresponding jamos

  @return      0           The code point is not Hangul syllable
                           or cannot be decomposed
               others      The number of jamos returned
*/
static int my_decompose_hangul_syllable(my_wc_t syllable, my_wc_t *jamo) {
  if (syllable < 0xAC00 || syllable > 0xD7AF) return 0;
  constexpr uint syllable_base = 0xAC00;
  constexpr uint leadingjamo_base = 0x1100;
  constexpr uint voweljamo_base = 0x1161;
  constexpr uint trailingjamo_base = 0x11A7;
  constexpr uint voweljamo_cnt = 21;
  constexpr uint trailingjamo_cnt = 28;
  const uint syllable_index = syllable - syllable_base;
  const uint v_t_combination = voweljamo_cnt * trailingjamo_cnt;
  const uint leadingjamo_index = syllable_index / v_t_combination;
  const uint voweljamo_index =
      (syllable_index % v_t_combination) / trailingjamo_cnt;
  const uint trailingjamo_index = syllable_index % trailingjamo_cnt;
  jamo[0] = leadingjamo_base + leadingjamo_index;
  jamo[1] = voweljamo_base + voweljamo_index;
  jamo[2] = trailingjamo_index ? (trailingjamo_base + trailingjamo_index) : 0;
  return trailingjamo_index ? 3 : 2;
}

template <class Mb_wc, int LEVELS_FOR_COMPARE>
void uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::my_put_jamo_weights(
    my_wc_t *hangul_jamo, int jamo_cnt) {
  for (int jamoind = 0; jamoind < jamo_cnt; jamoind++) {
    uint16 *implicit_weight = implicit + jamoind * MY_UCA_900_CE_SIZE;
    uint page = hangul_jamo[jamoind] >> 8;
    uint code = hangul_jamo[jamoind] & 0xFF;
    const uint16 *jamo_weight_page = uca->weights[page];
    implicit_weight[0] = UCA900_WEIGHT(jamo_weight_page, 0, code);
    implicit_weight[1] = UCA900_WEIGHT(jamo_weight_page, 1, code);
    implicit_weight[2] = UCA900_WEIGHT(jamo_weight_page, 2, code);
  }
  implicit[9] = jamo_cnt;
}

/*
  Chinese Han characters are assigned an implicit weight according to the
  Unicode Collation Algorithm. But when creating our Chinese collation for
  utf8mb4, to implement this language's reorder rule, we give the Han
  characters in CLDR zh.xml file weight values from 0x1C47 to 0xBDBE, and let
  the other Han characters keep their implicit weight. Per UCA, the smallest
  leading primary weight of the implicit weight is 0xFB00, and the largest
  primary weight we ocuppy for the Han characters in zh.xml is 0xBDBE. There is
  a huge gap between these two weight values. To use this weight value gap and
  let the character groups like Latin, Cyrillic, have a single primary weight as
  before reordering, we change the leading primary weight of the implicit weight
  as below.
 */
static uint16 change_zh_implicit(uint16 weight) {
  DBUG_ASSERT(weight >= 0xFB00);
  switch (weight) {
    case 0xFB00:
      return 0xF621;
    case 0xFB40:
      return 0xBDBF;
    case 0xFB41:
      return 0xBDC0;
    case 0xFB80:
      return 0xBDC1;
    case 0xFB84:
      return 0xBDC2;
    case 0xFB85:
      return 0xBDC3;
    default:
      return weight + 0xF622 - 0xFBC0;
  }
}

template <class Mb_wc, int LEVELS_FOR_COMPARE>
ALWAYS_INLINE int uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::next_implicit(
    my_wc_t ch) {
  my_wc_t hangul_jamo[HANGUL_JAMO_MAX_LENGTH];
  int jamo_cnt;
  if ((jamo_cnt = my_decompose_hangul_syllable(ch, hangul_jamo))) {
    my_put_jamo_weights(hangul_jamo, jamo_cnt);
    num_of_ce_left = jamo_cnt - 1;
    wbeg = implicit + MY_UCA_900_CE_SIZE + weight_lv;
    wbeg_stride = MY_UCA_900_CE_SIZE;
    return *(implicit + weight_lv);
  }

  /*
    We give the Chinese collation different leading primary weight to make
    sure there are enough single weight values to be assigned to character
    groups like Latin, Cyrillic, etc.
  */
  uint page;
  if (ch >= 0x17000 && ch <= 0x18AFF)  // Tangut character
  {
    page = 0xFB00;
    implicit[3] = (ch - 0x17000) | 0x8000;
  } else {
    page = ch >> 15;
    implicit[3] = (ch & 0x7FFF) | 0x8000;
    if ((ch >= 0x3400 && ch <= 0x4DB5) || (ch >= 0x20000 && ch <= 0x2A6D6) ||
        (ch >= 0x2A700 && ch <= 0x2B734) || (ch >= 0x2B740 && ch <= 0x2B81D) ||
        (ch >= 0x2B820 && ch <= 0x2CEA1)) {
      page += 0xFB80;
    } else if ((ch >= 0x4E00 && ch <= 0x9FD5) || (ch >= 0xFA0E && ch <= 0xFA29))
      page += 0xFB40;
    else
      page += 0xFBC0;
  }
  if (cs->coll_param == &zh_coll_param) {
    page = change_zh_implicit(page);
  }
  implicit[0] = page;
  implicit[1] = 0x0020;
  implicit[2] = 0x0002;
  // implicit[3] is set above.
  implicit[4] = 0;
  implicit[5] = 0;
  num_of_ce_left = 1;
  wbeg = implicit + MY_UCA_900_CE_SIZE + weight_lv;
  wbeg_stride = MY_UCA_900_CE_SIZE;

  return *(implicit + weight_lv);
}

/**
  Return implicit UCA weight
  Used for code points that do not have assigned UCA weights.

  @return   The leading implicit weight.
*/

template <class Mb_wc>
ALWAYS_INLINE int uca_scanner_any<Mb_wc>::next_implicit(my_wc_t ch) {
  implicit[0] = (ch & 0x7FFF) | 0x8000;
  implicit[1] = 0;
  wbeg = implicit;
  wbeg_stride = MY_UCA_900_CE_SIZE;

  uint page = ch >> 15;

  if (ch >= 0x3400 && ch <= 0x4DB5)
    page += 0xFB80;
  else if (ch >= 0x4E00 && ch <= 0x9FA5)
    page += 0xFB40;
  else
    page += 0xFBC0;

  return page;
}

template <class Mb_wc>
ALWAYS_INLINE int uca_scanner_any<Mb_wc>::next() {
  /*
    Check if the weights for the previous code point have been
    already fully scanned. If yes, then get the next code point and
    initialize wbeg and wlength to its weight string.
  */

  if (wbeg[0])      /* More weights left from the previous step: */
    return *wbeg++; /* return the next weight from expansion     */

  do {
    my_wc_t wc = 0;

    /* Get next code point */
    int mblen = mb_wc(&wc, sbeg, send);
    if (mblen <= 0) {
      ++weight_lv;
      return -1;
    }

    sbeg += mblen;
    char_index++;
    if (wc > uca->maxchar) {
      /* Return 0xFFFD as weight for all characters outside BMP */
      wbeg = nochar;
      wbeg_stride = 0;
      return 0xFFFD;
    }

    if (my_uca_have_contractions(uca)) {
      const uint16 *cweight;
      /*
        If we have scanned a code point which can have previous context,
        and there were some more code point already before,
        then verify that {prev_char, wc} together form
        a real previous context pair.
        Note, we support only 2-character long sequences with previous
        context at the moment. CLDR does not have longer sequences.
      */
      if (my_uca_can_be_previous_context_tail(uca->contraction_flags, wc) &&
          wbeg != nochar && /* if not the very first character */
          my_uca_can_be_previous_context_head(uca->contraction_flags,
                                              prev_char) &&
          (cweight = previous_context_find(prev_char, wc))) {
        prev_char = 0; /* Clear for the next character */
        return *cweight;
      } else if (my_uca_can_be_contraction_head(uca->contraction_flags, wc)) {
        /* Check if wc starts a contraction */
        size_t chars_skipped;
        if ((cweight = contraction_find(wc, &chars_skipped))) {
          char_index += chars_skipped;
          return *cweight;
        }
      }
      prev_char = wc;
    }

    /* Process single code point */
    uint page = wc >> 8;
    uint code = wc & 0xFF;

    /* If weight page for wc does not exist, then calculate algoritmically */
    const uint16 *wpage = uca->weights[page];
    if (!wpage) return next_implicit(wc);

    /* Calculate pointer to wc's weight, using page and offset */
    wbeg = wpage + code * uca->lengths[page];
    wbeg_stride = UCA900_DISTANCE_BETWEEN_WEIGHTS;
  } while (!wbeg[0]); /* Skip ignorable code points */

  return *wbeg++;
}

template <class Mb_wc, int LEVELS_FOR_COMPARE>
inline int uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::more_weight() {
  /*
    Check if the weights for the previous code point have been
    already fully scanned. If no, return the first non-zero
    weight.
  */

  while (num_of_ce_left != 0 && *wbeg == 0) {
    wbeg += wbeg_stride;
    --num_of_ce_left;
  }
  if (num_of_ce_left != 0) {
    uint16 rtn = *wbeg;
    wbeg += wbeg_stride;
    --num_of_ce_left;
    return rtn; /* return the next weight from expansion     */
  }
  return -1;
}

static inline bool is_hiragana_char(my_wc_t wc) {
  return wc >= 0x3041 && wc <= 0x3096;
}

static inline bool is_katakana_char(my_wc_t wc) {
  return (wc >= 0x30A1 && wc <= 0x30FA) ||  // Full width katakana
         (wc >= 0xFF66 && wc <= 0xFF9D);    // Half width katakana
}

static inline bool is_katakana_iteration(my_wc_t wc) {
  return wc == 0x30FD || wc == 0x30FE;
}

static inline bool is_hiragana_iteration(my_wc_t wc) {
  return wc == 0x309D || wc == 0x309E;
}

static inline bool is_ja_length_mark(my_wc_t wc) { return wc == 0x30FC; }

/**
  Return quaternary weight when running for that level.

  @retval  0 - Do not return quaternary weight.
  @retval  others - Quaternary weight for this character.
*/
template <class Mb_wc, int LEVELS_FOR_COMPARE>
ALWAYS_INLINE int
uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::handle_ja_contraction_quat_wt() {
  /*
    For Japanese, only weight shift rule and previous context rule is
    defined. And in previous context rules, the first character is always
    katakana / hiragana, and the second character is always iteration or
    length mark. The quaternary weight of iteration / length mark is
    same as the first character. So has_quaternary_weight is always true.
    For how we return quaternary weight, please refer to the comment in
    handle_ja_common_quat_wt().
  */
  if (weight_lv == 3) {
    wbeg = nochar;
    num_of_ce_left = 0;
    if (is_katakana_char(prev_char)) {
      return JA_KATA_QUAT_WEIGHT;
    } else if (is_hiragana_char(prev_char)) {
      return JA_HIRA_QUAT_WEIGHT;
    }
  }
  return 0;
}

/**
  Check whether quaternary weight is needed for character with Japanese
  kana-sensitive collation. If it is, return quaternary weight when running
  for that level.

  @retval  0 - Quaternary weight check is done.
  @retval -1 - There is no quaternary weight for this character.
  @retval others - Quaternary weight for this character.
*/
template <class Mb_wc, int LEVELS_FOR_COMPARE>
ALWAYS_INLINE int
uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::handle_ja_common_quat_wt(
    my_wc_t wc) {
  /*
    For Japanese kana-sensitive collation, we detect whether quaternary
    weight is necessary when scanning for the first level of weight.
    If it is, the quaternary weight will be returned for katakana /
    hiragana later.
  */
  if (weight_lv == 0 && !has_quaternary_weight) {
    if (is_katakana_char(wc) || is_katakana_iteration(wc) ||
        is_hiragana_char(wc) || is_hiragana_iteration(wc) ||
        is_ja_length_mark(wc))
      has_quaternary_weight = true;
  } else if (weight_lv == 3) {
    wbeg = nochar;
    num_of_ce_left = 0;
    if (is_katakana_char(wc) || is_katakana_iteration(wc) ||
        is_ja_length_mark(wc)) {
      return JA_KATA_QUAT_WEIGHT;
    } else if (is_hiragana_char(wc) || is_hiragana_iteration(wc)) {
      return JA_HIRA_QUAT_WEIGHT;
    }
    return -1;
  }
  return 0;
}

// Generic version that can handle any number of levels.
template <class Mb_wc, int LEVELS_FOR_COMPARE>
ALWAYS_INLINE int uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::next_raw() {
  int remain_weight = more_weight();
  if (remain_weight >= 0) return remain_weight;

  do {
    my_wc_t wc = 0;

    /* Get next code point */
    int mblen = mb_wc(&wc, sbeg, send);
    if (mblen <= 0) {
      if (LEVELS_FOR_COMPARE == 1) {
        ++weight_lv;
        return -1;
      }

      if (++weight_lv < LEVELS_FOR_COMPARE) {
        if (LEVELS_FOR_COMPARE == 4 && cs->coll_param == &ja_coll_param) {
          // Return directly if we don't have quaternary weight.
          if (weight_lv == 3 && !has_quaternary_weight) return -1;
        }
        /*
          Restart scanning from the beginning of the string, and add
          a level separator.
        */
        sbeg = sbeg_dup;
        return 0;
      }

      // If we don't have any more levels left, we're done.
      return -1;
    }

    sbeg += mblen;
    DBUG_ASSERT(wc <= uca->maxchar);  // mb_wc() has already checked this.

    if (my_uca_have_contractions(uca)) {
      const uint16 *cweight;
      /*
        If we have scanned a code point which can have previous context,
        and there were some more code points already before,
        then verify that {prev_char, wc} together form
        a real previous context pair.
        Note, we support only 2-character long sequences with previous
        context at the moment. CLDR does not have longer sequences.
        CLDR doesn't have previous context rule whose first character is
        0x0000, so the initial value (0) of prev_char won't break the logic.
      */
      if (my_uca_can_be_previous_context_tail(uca->contraction_flags, wc) &&
          my_uca_can_be_previous_context_head(uca->contraction_flags,
                                              prev_char) &&
          (cweight = previous_context_find(prev_char, wc))) {
        // For Japanese kana-sensitive collation.
        if (LEVELS_FOR_COMPARE == 4 && cs->coll_param == &ja_coll_param) {
          int quat_wt = handle_ja_contraction_quat_wt();
          prev_char = 0;
          if (quat_wt > 0) return quat_wt;
        }
        prev_char = 0; /* Clear for the next code point */
        return *cweight;
      } else if (my_uca_can_be_contraction_head(uca->contraction_flags, wc)) {
        /* Check if wc starts a contraction */
        size_t chars_skipped;  // Ignored.
        if ((cweight = contraction_find(wc, &chars_skipped))) return *cweight;
      }
      prev_char = wc;
    }

    // For Japanese kana-sensitive collation.
    if (LEVELS_FOR_COMPARE == 4 && cs->coll_param == &ja_coll_param) {
      int quat_wt = handle_ja_common_quat_wt(wc);
      if (quat_wt == -1)
        continue;
      else if (quat_wt)
        return quat_wt;
    }
    /* Process single code point */
    uint page = wc >> 8;
    uint code = wc & 0xFF;

    /* If weight page for wc does not exist, then calculate algoritmically */
    const uint16 *wpage = uca->weights[page];
    if (!wpage) return next_implicit(wc);

    /* Calculate pointer to wc's weight, using page and offset */
    wbeg = UCA900_WEIGHT_ADDR(wpage, weight_lv, code);
    wbeg_stride = UCA900_DISTANCE_BETWEEN_WEIGHTS;
    num_of_ce_left = UCA900_NUM_OF_CE(wpage, code);
  } while (!wbeg[0]); /* Skip ignorable code points */

  uint16 rtn = *wbeg;
  wbeg += wbeg_stride;
  --num_of_ce_left;
  return rtn;
}

template <class Mb_wc, int LEVELS_FOR_COMPARE>
template <class T, class U>
ALWAYS_INLINE void uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::for_each_weight(
    T func, U preaccept_data) {
  if (cs->tailoring || cs->mbminlen != 1 || cs->coll_param) {
    // Slower, generic path.
    int s_res;
    while ((s_res = next()) >= 0) {
      if (!func(s_res, s_res == 0)) return;
    }
    return;
  }

  /*
    Fast path. TODO: See if we can accept some character sets
    with tailorings.
  */
  const uint16 *ascii_wpage =
      UCA900_WEIGHT_ADDR(uca->weights[0], /*level=*/weight_lv, /*subcode=*/0);

  /*
    Precalculate the limit for the fast path below, taking care not to form
    pointers that are before sbeg, as those cannot be legally compared.
    (In particular, this catches the case of sbeg == send == nullptr.)
  */
  const uchar *send_local = (send - sbeg > 3) ? (send - 3) : sbeg;

  for (;;) {
    /*
      We could have more weights left from the previous call to next()
      (if any) that we need to deal with.
    */
    int s_res;
    while ((s_res = more_weight()) >= 0) {
      if (!func(s_res, s_res == 0)) return;
    }

    /*
      Loop in a simple fast path as long as we only have non-ignorable
      ASCII characters. These characters always have exactly a single weight
      and consist of only a single byte, so we can skip a lot of the checks
      we'd otherwise have to do.
    */
    const uchar *sbeg_local = sbeg;
    while (sbeg_local < send_local && preaccept_data(sizeof(uint32))) {
      /*
        Check if all four bytes are in the range 0x20..0x7e, inclusive.
        These have exactly one weight. Note that this unfortunately does not
        include tab and newline, which would otherwise be legal candidates.

        See the FastOutOfRange unit test for verification that the bitfiddling
        trick used here is correct.
      */
      uint32 four_bytes;
      memcpy(&four_bytes, sbeg_local, sizeof(four_bytes));
      if (((four_bytes + 0x01010101u) & 0x80808080) ||
          ((four_bytes - 0x20202020u) & 0x80808080))
        break;
      const int s_res0 = ascii_wpage[sbeg_local[0]];
      const int s_res1 = ascii_wpage[sbeg_local[1]];
      const int s_res2 = ascii_wpage[sbeg_local[2]];
      const int s_res3 = ascii_wpage[sbeg_local[3]];
      DBUG_ASSERT(s_res0 != 0);
      DBUG_ASSERT(s_res1 != 0);
      DBUG_ASSERT(s_res2 != 0);
      DBUG_ASSERT(s_res3 != 0);
      func(s_res0, /*is_level_separator=*/false);
      func(s_res1, /*is_level_separator=*/false);
      func(s_res2, /*is_level_separator=*/false);
      func(s_res3, /*is_level_separator=*/false);
      sbeg_local += sizeof(uint32);
    }
    sbeg = sbeg_local;

    // Do a single code point in the generic path.
    s_res = next_raw();
    if (s_res == 0) {
      // Level separator, so we have to update our page pointer.
      ascii_wpage += UCA900_DISTANCE_BETWEEN_LEVELS;
    }
    if (s_res < 0 || !func(s_res, s_res == 0)) return;
  }
}

/**
  Change a weight according to the reorder parameters.
  @param   weight     The weight to change
  @retval  reordered weight
*/
template <class Mb_wc, int LEVELS_FOR_COMPARE>
uint16 uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::apply_reorder_param(
    uint16 weight) {
  /*
    Chinese collation's reordering is done in next_implicit() and
    modify_all_zh_pages(). See the comment on zh_reorder_param and
    change_zh_implicit().
   */
  if (cs->coll_param == &zh_coll_param) return weight;
  const Reorder_param *param = cs->coll_param->reorder_param;
  if (weight >= START_WEIGHT_TO_REORDER && weight <= param->max_weight) {
    for (int rec_ind = 0; rec_ind < param->wt_rec_num; ++rec_ind) {
      const Reorder_wt_rec *wt_rec = param->wt_rec + rec_ind;
      if (weight >= wt_rec->old_wt_bdy.begin &&
          weight <= wt_rec->old_wt_bdy.end) {
        /*
          As commented in adjust_japanese_weight(), if this is a Japanese
          collation, for characters whose weight is between Latin and Kana
          group, and for the characters whose weight is between Kana and
          Han, we need to change their weight to be after all Han
          characters. We decide to give them the weights [FB86 0000 0000]
          [origin weight] to make sure the new weights are greater than
          the maximum implicit weight of Han characters. If this character's
          origin weight has more than one non-ignorable primary weight, for
          example, [AAAA 0020 0002][BBBB 0020 0002], both AAAA and BBBB need
          to be changed. The new weight should be:
          [FB86 0000 0000][AAAA 0020 0002][FB86 0000 0000][BBBB 0020 0002].
        */
        if (param == &ja_reorder_param && wt_rec->new_wt_bdy.begin == 0) {
          return_origin_weight = !return_origin_weight;
          if (return_origin_weight) break;

          /*
            We didn't consume the weight; rewind the iterator, so we will
            get another call where we can output it.
          */
          wbeg -= wbeg_stride;
          ++num_of_ce_left;
          return 0xFB86;
        }

        // Regular (non-Japanese-specific) reordering.
        return weight - wt_rec->old_wt_bdy.begin + wt_rec->new_wt_bdy.begin;
      }
    }
  }
  return weight;
}

// See Unicode TR35 section 3.14.1.
static bool is_tertiary_weight_upper_case(uint16 weight) {
  if ((weight >= 0x08 && weight <= 0x0C) || weight == 0x0E || weight == 0x11 ||
      weight == 0x12 || weight == 0x1D)
    return true;
  return false;
}

template <class Mb_wc, int LEVELS_FOR_COMPARE>
uint16 uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::apply_case_first(
    uint16 weight) {
  /*
    We only apply case weight change here when the character is not tailored.
    Tailored character's case weight has been changed in
    my_char_weight_put_900().
    We have only 1 collation (Danish) needs to implement [caseFirst upper].
  */
  if (cs->coll_param->case_first == CASE_FIRST_UPPER && weight_lv == 2 &&
      weight < 0x20) {
    if (is_tertiary_weight_upper_case(weight))
      weight |= CASE_FIRST_UPPER_MASK;
    else
      weight |= CASE_FIRST_LOWER_MASK;
  }
  return weight;
}

template <class Mb_wc, int LEVELS_FOR_COMPARE>
ALWAYS_INLINE int uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::next() {
  int res = next_raw();
  Coll_param *param = cs->coll_param;
  if (res > 0 && param) {
    /* Reorder weight change only on primary level. */
    if (param->reorder_param && weight_lv == 0) res = apply_reorder_param(res);
    if (param->case_first != CASE_FIRST_OFF) res = apply_case_first(res);
  }
  return res;
}

/*
  Compares two strings according to the collation

  SYNOPSIS:
    my_strnncoll_uca()
    cs		Character set information
    s		First string
    slen	First string length
    t		Second string
    tlen	Second string length

  NOTES:
    Initializes two weight scanners and gets weights
    corresponding to two strings in a loop. If weights are not
    the same at some step then returns their difference.

    In the while() comparison these situations are possible:
    1. (s_res>0) and (t_res>0) and (s_res == t_res)
       Weights are the same so far, continue comparison
    2. (s_res>0) and (t_res>0) and (s_res!=t_res)
       A difference has been found, return.
    3. (s_res>0) and (t_res<0)
       We have reached the end of the second string, or found
       an illegal multibyte sequence in the second string.
       Return a positive number, i.e. the first string is bigger.
    4. (s_res<0) and (t_res>0)
       We have reached the end of the first string, or found
       an illegal multibyte sequence in the first string.
       Return a negative number, i.e. the second string is bigger.
    5. (s_res<0) and (t_res<0)
       Both scanners returned -1. It means we have riched
       the end-of-string of illegal-sequence in both strings
       at the same time. Return 0, strings are equal.

  RETURN
    Difference between two strings, according to the collation:
    0               - means strings are equal
    negative number - means the first string is smaller
    positive number - means the first string is bigger
*/

template <class Scanner, int LEVELS_FOR_COMPARE, class Mb_wc>
static int my_strnncoll_uca(const CHARSET_INFO *cs, const Mb_wc mb_wc,
                            const uchar *s, size_t slen, const uchar *t,
                            size_t tlen, bool t_is_prefix) {
  Scanner sscanner(mb_wc, cs, s, slen);
  Scanner tscanner(mb_wc, cs, t, tlen);
  int s_res = 0;
  int t_res = 0;

  /*
    We compare 2 strings in same level first. If only string A's scanner
    has gone to next level, which means another string, B's weight of
    current level is longer than A's. We'll compare B's remaining weights
    with space.
  */
  for (uint current_lv = 0; current_lv < LEVELS_FOR_COMPARE; ++current_lv) {
    /* Run the scanners until one of them runs out of current lv */
    do {
      s_res = sscanner.next();
      t_res = tscanner.next();
    } while (s_res == t_res && s_res >= 0 &&
             sscanner.get_weight_level() == current_lv &&
             tscanner.get_weight_level() == current_lv);

    /*
      Two scanners run to next level at same time, or we found a difference,
      or we found an error.
    */
    if (sscanner.get_weight_level() == tscanner.get_weight_level()) {
      if (s_res == t_res && s_res >= 0) continue;
      break;  // Error or inequality found, end.
    }

    if (tscanner.get_weight_level() > current_lv) {
      // t ran out of weights on this level, and s didn't.
      if (t_is_prefix) {
        // Consume the rest of the weights from s.
        do {
          s_res = sscanner.next();
        } while (s_res >= 0 && sscanner.get_weight_level() == current_lv);

        if (s_res < 0) break;  // Error found, end.

        // s is now also on the next level. Continue comparison.
        continue;
      } else {
        // s is longer than t (and t_prefix isn't set).
        return 1;
      }
    }

    if (sscanner.get_weight_level() > current_lv) {
      // s ran out of weights on this level, and t didn't.
      return -1;
    }

    break;
  }

  return (s_res - t_res);
}

static inline int my_space_weight(const CHARSET_INFO *cs) /* W3-TODO */
{
  if (cs->uca && cs->uca->version == UCA_V900)
    return UCA900_WEIGHT(cs->uca->weights[0], /*weight_lv=*/0, 0x20);
  else
    return cs->uca->weights[0][0x20 * cs->uca->lengths[0]];
}

/**
  Helper function:
  Find address of weights of the given code point.

  @param uca      Pointer to UCA data
  @param wc       character Unicode code point

  @return Weight array
    @retval  pointer to weight array for the given code point,
             or nullptr if this page does not have implicit weights.
*/

static inline uint16 *my_char_weight_addr(MY_UCA_INFO *uca, my_wc_t wc) {
  uint page, ofst;
  return wc > uca->maxchar ? nullptr
                           : (uca->weights[page = (wc >> 8)]
                                  ? uca->weights[page] + (ofst = (wc & 0xFF)) *
                                                             uca->lengths[page]
                                  : nullptr);
}

/**
  Helper function:
  Find address of weights of the given code point, for UCA 9.0.0 format.

  @param uca      Pointer to UCA data
  @param wc       character Unicode code point

  @return Weight array
    @retval  pointer to weight array for the given code point,
             or nullptr if this page does not have implicit weights.
*/

static inline uint16 *my_char_weight_addr_900(MY_UCA_INFO *uca, my_wc_t wc) {
  if (wc > uca->maxchar) return nullptr;

  uint page = wc >> 8;
  uint ofst = wc & 0xFF;
  uint16 *weights = uca->weights[page];
  if (weights)
    return UCA900_WEIGHT_ADDR(weights, /*level=*/0, ofst);
  else
    return nullptr;
}

/*
  Compares two strings according to the collation,
  ignoring trailing spaces.

  SYNOPSIS:
    my_strnncollsp_uca()
    cs		Character set information
    s		First string
    slen	First string length
    t		Second string
    tlen	Second string length

  NOTES:
    Works exactly the same with my_strnncoll_uca(),
    but ignores trailing spaces.

    In the while() comparison these situations are possible:
    1. (s_res>0) and (t_res>0) and (s_res == t_res)
       Weights are the same so far, continue comparison
    2. (s_res>0) and (t_res>0) and (s_res!=t_res)
       A difference has been found, return.
    3. (s_res>0) and (t_res<0)
       We have reached the end of the second string, or found
       an illegal multibyte sequence in the second string.
       Compare the first string to an infinite array of
       space characters until difference is found, or until
       the end of the first string.
    4. (s_res<0) and (t_res>0)
       We have reached the end of the first string, or found
       an illegal multibyte sequence in the first string.
       Compare the second string to an infinite array of
       space characters until difference is found or until
       the end of the second steing.
    5. (s_res<0) and (t_res<0)
       Both scanners returned -1. It means we have riched
       the end-of-string of illegal-sequence in both strings
       at the same time. Return 0, strings are equal.

  RETURN
    Difference between two strings, according to the collation:
    0               - means strings are equal
    negative number - means the first string is smaller
    positive number - means the first string is bigger
*/

template <class Mb_wc>
static int my_strnncollsp_uca(const CHARSET_INFO *cs, Mb_wc mb_wc,
                              const uchar *s, size_t slen, const uchar *t,
                              size_t tlen) {
  int s_res, t_res;

  uca_scanner_any<Mb_wc> sscanner(mb_wc, cs, s, slen);
  uca_scanner_any<Mb_wc> tscanner(mb_wc, cs, t, tlen);

  do {
    s_res = sscanner.next();
    t_res = tscanner.next();
  } while (s_res == t_res && s_res > 0);

  if (s_res > 0 && t_res < 0) {
    /* Calculate weight for SPACE character */
    t_res = my_space_weight(cs);

    /* compare the first string to spaces */
    do {
      if (s_res != t_res) return (s_res - t_res);
      s_res = sscanner.next();
    } while (s_res > 0);
    return 0;
  }

  if (s_res < 0 && t_res > 0) {
    /* Calculate weight for SPACE character */
    s_res = my_space_weight(cs);

    /* compare the second string to spaces */
    do {
      if (s_res != t_res) return (s_res - t_res);
      t_res = tscanner.next();
    } while (t_res > 0);
    return 0;
  }

  return (s_res - t_res);
}

/*
  Calculates hash value for the given string,
  according to the collation, and ignoring trailing spaces.

  SYNOPSIS:
    my_hash_sort_uca()
    cs		Character set information
    s		String
    slen	String's length
    n1		First hash parameter
    n2		Second hash parameter

  NOTES:
    Scans consequently weights and updates
    hash parameters n1 and n2. In a case insensitive collation,
    upper and lower case of the same letter will return the same
    weight sequence, and thus will produce the same hash values
    in n1 and n2.

  RETURN
    N/A
*/

template <class Mb_wc>
static void my_hash_sort_uca(const CHARSET_INFO *cs, Mb_wc mb_wc,
                             const uchar *s, size_t slen, uint64 *n1,
                             uint64 *n2) {
  int s_res;
  uint64 tmp1;
  uint64 tmp2;

  slen = cs->cset->lengthsp(cs, pointer_cast<const char *>(s), slen);
  uca_scanner_any<Mb_wc> scanner(mb_wc, cs, s, slen);

  tmp1 = *n1;
  tmp2 = *n2;

  while ((s_res = scanner.next()) > 0) {
    tmp1 ^= (((tmp1 & 63) + tmp2) * (s_res >> 8)) + (tmp1 << 8);
    tmp2 += 3;
    tmp1 ^= (((tmp1 & 63) + tmp2) * (s_res & 0xFF)) + (tmp1 << 8);
    tmp2 += 3;
  }

  *n1 = tmp1;
  *n2 = tmp2;
}

/*
  For the given string creates its "binary image", suitable
  to be used in binary comparison, i.e. in memcmp().

  SYNOPSIS:
    my_strnxfrm_uca()
    cs		Character set information
    dst		Where to write the image
    dstlen	Space available for the image, in bytes
    src		The source string
    srclen	Length of the source string, in bytes

  NOTES:
    In a loop, scans weights from the source string and writes
    them into the binary image. In a case insensitive collation,
    upper and lower cases of the same letter will produce the
    same image subsequences. When we have reached the end-of-string
    or found an illegal multibyte sequence, the loop stops.

    It is impossible to restore the original string using its
    binary image.

    Binary images are used for bulk comparison purposes,
    e.g. in ORDER BY, when it is more efficient to create
    a binary image and use it instead of weight scanner
    for the original strings for every comparison.

  RETURN
    Number of bytes that have been written into the binary image.
*/

template <class Mb_wc>
static size_t my_strnxfrm_uca(const CHARSET_INFO *cs, Mb_wc mb_wc, uchar *dst,
                              size_t dstlen, uint num_codepoints,
                              const uchar *src, size_t srclen, uint flags) {
  uchar *d0 = dst;
  uchar *de = dst + dstlen;
  int s_res;
  uca_scanner_any<Mb_wc> scanner(mb_wc, cs, src, srclen);

  while (dst < de && (s_res = scanner.next()) > 0) {
    *dst++ = s_res >> 8;
    if (dst < de) *dst++ = s_res & 0xFF;
  }

  if (dst < de) {
    /*
      PAD SPACE behavior.

      We still have space left in the output buffer, which must mean
      that the scanner is at the end of the last level. Find out
      how many weights we wrote per level, and add any remaining
      spaces we need to get us up to the requested total.
    */
    DBUG_ASSERT(num_codepoints >= scanner.get_char_index());
    num_codepoints -= scanner.get_char_index();

    if (num_codepoints) {
      uint space_count = std::min<uint>((de - dst) / 2, num_codepoints);
      s_res = my_space_weight(cs);
      for (; space_count; space_count--) {
        dst = store16be(dst, s_res);
      }
    }
  }
  if ((flags & MY_STRXFRM_PAD_TO_MAXLEN) && dst < de) {
    s_res = my_space_weight(cs);
    for (; dst < de;) {
      *dst++ = s_res >> 8;
      if (dst < de) *dst++ = s_res & 0xFF;
    }
  }
  return dst - d0;
}

static int my_uca_charcmp_900(const CHARSET_INFO *cs, my_wc_t wc1,
                              my_wc_t wc2) {
  uint16 *weight1_ptr = my_char_weight_addr_900(cs->uca, wc1); /* W3-TODO */
  uint16 *weight2_ptr = my_char_weight_addr_900(cs->uca, wc2);

  /* Check if some of the characters does not have implicit weights */
  if (!weight1_ptr || !weight2_ptr) return wc1 != wc2;

  if (weight1_ptr[0] && weight2_ptr[0] && weight1_ptr[0] != weight2_ptr[0])
    return 1;

  /* Thoroughly compare all weights */
  size_t length1 = weight1_ptr[-UCA900_DISTANCE_BETWEEN_LEVELS];
  size_t length2 = weight2_ptr[-UCA900_DISTANCE_BETWEEN_LEVELS];

  for (int level = 0; level < cs->levels_for_compare; ++level) {
    size_t wt_ind1 = 0;
    size_t wt_ind2 = 0;
    uint16 *weight1 = weight1_ptr + level * UCA900_DISTANCE_BETWEEN_LEVELS;
    uint16 *weight2 = weight2_ptr + level * UCA900_DISTANCE_BETWEEN_LEVELS;
    while (wt_ind1 < length1 && wt_ind2 < length2) {
      // Zero weight is ignorable.
      for (; wt_ind1 < length1 && !*weight1; wt_ind1++)
        weight1 += UCA900_DISTANCE_BETWEEN_WEIGHTS;
      if (wt_ind1 == length1) break;
      for (; wt_ind2 < length2 && !*weight2; wt_ind2++)
        weight2 += UCA900_DISTANCE_BETWEEN_WEIGHTS;
      if (wt_ind2 == length2) break;

      // Check if these two non-ignorable weights are equal.
      if (*weight1 != *weight2) return 1;
      wt_ind1++;
      wt_ind2++;
      weight1 += UCA900_DISTANCE_BETWEEN_WEIGHTS;
      weight2 += UCA900_DISTANCE_BETWEEN_WEIGHTS;
    }
    /*
      If either character is out of weights but we have equality so far,
      check if the other character has any non-ignorable weights left.
    */
    for (; wt_ind1 < length1; wt_ind1++) {
      if (*weight1) return 1;
      weight1 += UCA900_DISTANCE_BETWEEN_WEIGHTS;
    }
    for (; wt_ind2 < length2; wt_ind2++) {
      if (*weight2) return 1;
      weight2 += UCA900_DISTANCE_BETWEEN_WEIGHTS;
    }
  }
  return 0;
}

/*
  This function compares if two code points are the same.
  The sign +1 or -1 does not matter. The only
  important thing is that the result is 0 or not 0.
  This fact allows us to use memcmp() safely, on both
  little-endian and big-endian machines.
*/

static int my_uca_charcmp(const CHARSET_INFO *cs, my_wc_t wc1, my_wc_t wc2) {
  if (wc1 == wc2) return 0;

  if (cs->uca != nullptr && cs->uca->version == UCA_V900)
    return my_uca_charcmp_900(cs, wc1, wc2);

  size_t length1, length2;
  uint16 *weight1 = my_char_weight_addr(cs->uca, wc1); /* W3-TODO */
  uint16 *weight2 = my_char_weight_addr(cs->uca, wc2);

  /* Check if some of the code points does not have implicit weights */
  if (!weight1 || !weight2) return wc1 != wc2;

  /* Quickly compare first weights */
  if (weight1[0] != weight2[0]) return 1;

  /* Thoroughly compare all weights */
  length1 = cs->uca->lengths[wc1 >> MY_UCA_PSHIFT]; /* W3-TODO */
  length2 = cs->uca->lengths[wc2 >> MY_UCA_PSHIFT];

  if (length1 > length2)
    return memcmp((const void *)weight1, (const void *)weight2, length2 * 2)
               ? 1
               : weight1[length2];

  if (length1 < length2)
    return memcmp((const void *)weight1, (const void *)weight2, length1 * 2)
               ? 1
               : weight2[length1];

  return memcmp((const void *)weight1, (const void *)weight2, length1 * 2);
}

/*** Compare string against string with wildcard
**	0 if matched
**	-1 if not matched with wildcard
**	 1 if matched with wildcard
*/

static int my_wildcmp_uca_impl(const CHARSET_INFO *cs, const char *str,
                               const char *str_end, const char *wildstr,
                               const char *wildend, int escape, int w_one,
                               int w_many, int recurse_level) {
  if (my_string_stack_guard && my_string_stack_guard(recurse_level)) return 1;
  while (wildstr != wildend) {
    int result = -1; /* Not found, using wildcards */
    auto mb_wc = cs->cset->mb_wc;

    /*
      Compare the expression and pattern strings character-by-character until
      we find a '%' (w_many) in the pattern string. Once we do, we break out
      of the loop and try increasingly large widths for the '%' match,
      calling ourselves recursively until we find a match. (As an
      optimization, we test for the character immediately after '%' before we
      recurse.) This takes exponential time in the worst case.

      Example: Say we are trying to match the pattern 'ab%cd' against the
      string 'ab..c.cd'. We first match the initial 'ab' against each other,
      and then see the '%' in the pattern. Since the first character after
      '%' is 'c', we skip to the first 'c' in the expression string, and try
      to match 'c.cd' against 'cd' by a recursive call. Since this failed, we
      scan for the next 'c', and try to match 'cd' against 'cd', which works.
    */
    my_wc_t w_wc;
    while (true) {
      int mb_len;
      if ((mb_len = mb_wc(cs, &w_wc, (const uchar *)wildstr,
                          (const uchar *)wildend)) <= 0)
        return 1;

      wildstr += mb_len;
      // If we found '%' (w_many), break out this loop.
      if (w_wc == (my_wc_t)w_many) {
        result = 1;
        break;
      }

      /*
        If the character we just read was an escape character, skip it and
        read the next character instead. This character is used verbatim
        without checking if it is a wildcard (% or _). However, as a
        special exception, a lone escape character at the end of a string is
        treated as itself.
      */
      bool escaped = false;
      if (w_wc == (my_wc_t)escape && wildstr < wildend) {
        if ((mb_len = mb_wc(cs, &w_wc, (const uchar *)wildstr,
                            (const uchar *)wildend)) <= 0)
          return 1;
        wildstr += mb_len;
        escaped = true;
      }

      my_wc_t s_wc;
      if ((mb_len = mb_wc(cs, &s_wc, (const uchar *)str,
                          (const uchar *)str_end)) <= 0)
        return 1;
      str += mb_len;

      // If we found '_' (w_one), skip one character in expression string.
      if (!escaped && w_wc == (my_wc_t)w_one) {
        result = 1;
      } else {
        if (my_uca_charcmp(cs, s_wc, w_wc)) return 1;
      }
      if (wildstr == wildend)
        return (str != str_end); /* Match if both are at end */
    }

    if (w_wc == (my_wc_t)w_many) {
      // Remove any '%' and '_' following w_many in the pattern string.
      for (;;) {
        if (wildstr == wildend) {
          /*
            The previous w_many (%) was the last character in the pattern
            string, so we have a match no matter what the rest of the
            expression string looks like (even empty).
          */
          return 0;
        }
        int mb_len_wild =
            mb_wc(cs, &w_wc, (const uchar *)wildstr, (const uchar *)wildend);
        if (mb_len_wild <= 0) return 1;
        wildstr += mb_len_wild;
        if (w_wc == (my_wc_t)w_many) continue;

        if (w_wc == (my_wc_t)w_one) {
          /*
            Skip one character in expression string because '_' needs to
            match one.
          */
          my_wc_t s_wc;
          int mb_len =
              mb_wc(cs, &s_wc, (const uchar *)str, (const uchar *)str_end);
          if (mb_len <= 0) return 1;
          str += mb_len;
          continue;
        }
        break; /* Not a wild character */
      }

      // No character in the expression string to match w_wc.
      if (str == str_end) return -1;

      // Skip the escape character ('\') in the pattern if needed.
      if (w_wc == (my_wc_t)escape && wildstr < wildend) {
        int mb_len =
            mb_wc(cs, &w_wc, (const uchar *)wildstr, (const uchar *)wildend);
        if (mb_len <= 0) return 1;
        wildstr += mb_len;
      }

      /*
        w_wc is now the character following w_many (e.g., if the pattern is
        "a%c", w_wc is 'c').
      */
      while (true) {
        /*
          Skip until we find a character in the expression string that is
          equal to w_wc.
        */
        int mb_len = 0;
        while (str != str_end) {
          my_wc_t s_wc;
          if ((mb_len = mb_wc(cs, &s_wc, (const uchar *)str,
                              (const uchar *)str_end)) <= 0)
            return 1;

          if (!my_uca_charcmp(cs, s_wc, w_wc)) break;
          str += mb_len;
        }
        // No character in the expression string is equal to w_wc.
        if (str == str_end) return -1;
        str += mb_len;

        /*
          The strings match up until the first character after w_many in the
          pattern string. For the rest part of pattern string and expression
          string, we recursively call to get wild compare result.
          Example, wildcmp(..., "abcdefg", "a%de%g", ...), we'll run again on
          wildcmp(..., "efg", "e%g", ...).
        */
        result = my_wildcmp_uca_impl(cs, str, str_end, wildstr, wildend, escape,
                                     w_one, w_many, recurse_level + 1);

        if (result <= 0) return result;
      }
    }
  }
  return (str != str_end ? 1 : 0);
}

static int my_strcasecmp_uca(const CHARSET_INFO *cs, const char *s,
                             const char *t) {
  const MY_UNICASE_INFO *uni_plane = cs->caseinfo;
  const MY_UNICASE_CHARACTER *page;
  while (s[0] && t[0]) {
    my_wc_t s_wc, t_wc;

    if (static_cast<uchar>(s[0]) < 128) {
      s_wc = uni_plane->page[0][static_cast<uchar>(s[0])].tolower;
      s++;
    } else {
      int res;

      res = cs->cset->mb_wc(cs, &s_wc, pointer_cast<const uchar *>(s),
                            pointer_cast<const uchar *>(s + 4));

      if (res <= 0) return strcmp(s, t);
      s += res;
      if (s_wc <= uni_plane->maxchar && (page = uni_plane->page[s_wc >> 8]))
        s_wc = page[s_wc & 0xFF].tolower;
    }

    /* Do the same for the second string */

    if (static_cast<uchar>(t[0]) < 128) {
      /* Convert single byte character into weight */
      t_wc = uni_plane->page[0][static_cast<uchar>(t[0])].tolower;
      t++;
    } else {
      int res = cs->cset->mb_wc(cs, &t_wc, pointer_cast<const uchar *>(t),
                                pointer_cast<const uchar *>(t + 4));
      if (res <= 0) return strcmp(s, t);
      t += res;

      if (t_wc <= uni_plane->maxchar && (page = uni_plane->page[t_wc >> 8]))
        t_wc = page[t_wc & 0xFF].tolower;
    }

    /* Now we have two weights, let's compare them */
    if (s_wc != t_wc) return static_cast<int>(s_wc) - static_cast<int>(t_wc);
  }
  return static_cast<int>(static_cast<uchar>(s[0])) -
         static_cast<int>(static_cast<uchar>(t[0]));
}

extern "C" {
static int my_wildcmp_uca(const CHARSET_INFO *cs, const char *str,
                          const char *str_end, const char *wildstr,
                          const char *wildend, int escape, int w_one,
                          int w_many) {
  return my_wildcmp_uca_impl(cs, str, str_end, wildstr, wildend, escape, w_one,
                             w_many, 1);
}
}  // extern "C"

/*
  Collation language is implemented according to
  subset of ICU Collation Customization (tailorings):
  http://icu.sourceforge.net/userguide/Collate_Customization.html

  Collation language elements:
  Delimiters:
    space   - skipped

  <char> :=  A-Z | a-z | \uXXXX

  Shift command:
    <shift>  := &       - reset at this letter.

  Diff command:
    <d1> :=  <     - Identifies a primary difference.
    <d2> :=  <<    - Identifies a secondary difference.
    <d3> := <<<    - Idenfifies a tertiary difference.


  Collation rules:
    <ruleset> :=  <rule>  { <ruleset> }

    <rule> :=   <d1>    <string>
              | <d2>    <string>
              | <d3>    <string>
              | <shift> <char>

    <string> := <char> [ <string> ]

  An example, Polish collation:

    &A < \u0105 <<< \u0104
    &C < \u0107 <<< \u0106
    &E < \u0119 <<< \u0118
    &L < \u0142 <<< \u0141
    &N < \u0144 <<< \u0143
    &O < \u00F3 <<< \u00D3
    &S < \u015B <<< \u015A
    &Z < \u017A <<< \u017B
*/

typedef enum my_coll_lexem_num_en {
  MY_COLL_LEXEM_EOF = 0,
  MY_COLL_LEXEM_SHIFT = 1,
  MY_COLL_LEXEM_RESET = 4,
  MY_COLL_LEXEM_CHAR = 5,
  MY_COLL_LEXEM_ERROR = 6,
  MY_COLL_LEXEM_OPTION = 7,
  MY_COLL_LEXEM_EXTEND = 8,
  MY_COLL_LEXEM_CONTEXT = 9
} my_coll_lexem_num;

/**
  Convert collation customization lexem to string,
  for nice error reporting

  @param term   lexem code

  @return       lexem name
*/

static const char *my_coll_lexem_num_to_str(my_coll_lexem_num term) {
  switch (term) {
    case MY_COLL_LEXEM_EOF:
      return "EOF";
    case MY_COLL_LEXEM_SHIFT:
      return "Shift";
    case MY_COLL_LEXEM_RESET:
      return "&";
    case MY_COLL_LEXEM_CHAR:
      return "Character";
    case MY_COLL_LEXEM_OPTION:
      return "Bracket option";
    case MY_COLL_LEXEM_EXTEND:
      return "/";
    case MY_COLL_LEXEM_CONTEXT:
      return "|";
    case MY_COLL_LEXEM_ERROR:
      return "ERROR";
  }
  return nullptr;
}

struct MY_COLL_LEXEM {
  my_coll_lexem_num term;
  const char *beg;
  const char *end;
  const char *prev;
  int diff;
  int code;
};

/*
  Initialize collation rule lexical anilizer

  SYNOPSIS
    my_coll_lexem_init
    lexem                Lex analizer to init
    str                  Const string to parse
    str_end               End of the string
  USAGE

  RETURN VALUES
    N/A
*/

static void my_coll_lexem_init(MY_COLL_LEXEM *lexem, const char *str,
                               const char *str_end) {
  lexem->beg = str;
  lexem->prev = str;
  lexem->end = str_end;
  lexem->diff = 0;
  lexem->code = 0;
}

/**
  Compare lexem to string with length

  @param lexem       lexem
  @param pattern     string
  @param patternlen  string length

  @retval            0 if lexem is equal to string, non-0 otherwise.
*/

static int lex_cmp(MY_COLL_LEXEM *lexem, const char *pattern,
                   size_t patternlen) {
  size_t lexemlen = lexem->beg - lexem->prev;
  if (lexemlen < patternlen) return 1; /* Not a prefix */
  return native_strncasecmp(lexem->prev, pattern, patternlen);
}

/*
  Print collation customization expression parse error, with context.

  SYNOPSIS
    my_coll_lexem_print_error
    lexem                Lex analizer to take context from
    errstr               sting to write error to
    errsize              errstr size
    txt                  error message
    col_name             collation name
  USAGE

  RETURN VALUES
    N/A
*/

static void my_coll_lexem_print_error(MY_COLL_LEXEM *lexem, char *errstr,
                                      size_t errsize, const char *txt,
                                      const char *col_name) {
  char tail[30];
  size_t len = lexem->end - lexem->prev;
  strmake(tail, lexem->prev, std::min(len, sizeof(tail) - 1));
  errstr[errsize - 1] = '\0';
  snprintf(errstr, errsize - 1, "%s at '%s' for COLLATION : %s",
           txt[0] ? txt : "Syntax error", tail, col_name);
}

/*
  Convert a hex digit into its numeric value

  SYNOPSIS
    ch2x
    ch                   hex digit to convert
  USAGE

  RETURN VALUES
    an integer value in the range 0..15
    -1 on error
*/

static int ch2x(int ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';

  if (ch >= 'a' && ch <= 'f') return 10 + ch - 'a';

  if (ch >= 'A' && ch <= 'F') return 10 + ch - 'A';

  return -1;
}

/*
  Collation language lexical parser:
  Scans the next lexem.

  SYNOPSIS
    my_coll_lexem_next
    lexem                Lex analizer, previously initialized by
                         my_coll_lexem_init.
  USAGE
    Call this function in a loop

  RETURN VALUES
    Lexem number: eof, diff, shift, char or error.
*/

static my_coll_lexem_num my_coll_lexem_next(MY_COLL_LEXEM *lexem) {
  const char *beg;
  my_coll_lexem_num rc;

  for (beg = lexem->beg; beg < lexem->end; beg++) {
    switch (*beg) {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        continue;

      case '[': /* Bracket expression, e.g. "[optimize [a-z]]" */
      {
        size_t nbrackets; /* Indicates nested recursion level */
        for (beg++, nbrackets = 1; beg < lexem->end; beg++) {
          if (*beg == '[') /* Enter nested bracket expression */
            nbrackets++;
          else if (*beg == ']') {
            if (--nbrackets == 0) {
              rc = MY_COLL_LEXEM_OPTION;
              beg++;
              goto ex;
            }
          }
        }
        rc = MY_COLL_LEXEM_ERROR;
        goto ex;
      }

      case '&':
        beg++;
        rc = MY_COLL_LEXEM_RESET;
        goto ex;

      case '=':
        beg++;
        lexem->diff = 0;
        rc = MY_COLL_LEXEM_SHIFT;
        goto ex;

      case '/':
        beg++;
        rc = MY_COLL_LEXEM_EXTEND;
        goto ex;

      case '|':
        beg++;
        rc = MY_COLL_LEXEM_CONTEXT;
        goto ex;

      case '<': /* Shift: '<' or '<<' or '<<<' or '<<<<' */
      {
        /* Scan up to 3 additional '<' characters */
        for (beg++, lexem->diff = 1;
             (beg < lexem->end) && (*beg == '<') && (lexem->diff <= 3);
             beg++, lexem->diff++)
          ;
        rc = MY_COLL_LEXEM_SHIFT;
        goto ex;
      }
      default:
        break;
    }

    /* Escaped character, e.g. \u1234 */
    if ((*beg == '\\') && (beg + 2 < lexem->end) && (beg[1] == 'u') &&
        my_isxdigit(&my_charset_utf8_general_ci, beg[2])) {
      int ch;

      beg += 2;
      lexem->code = 0;
      while ((beg < lexem->end) && ((ch = ch2x(beg[0])) >= 0)) {
        lexem->code = (lexem->code << 4) + ch;
        beg++;
      }
      rc = MY_COLL_LEXEM_CHAR;
      goto ex;
    }

    /*
      Unescaped single byte character:
        allow printable ASCII range except SPACE and
        special characters parsed above []<&/|=
    */
    if (*beg >= 0x21 && *beg <= 0x7E) {
      lexem->code = *beg++;
      rc = MY_COLL_LEXEM_CHAR;
      goto ex;
    }

    if (((uchar)*beg) > 0x7F) /* Unescaped multibyte character */
    {
      CHARSET_INFO *cs = &my_charset_utf8_general_ci;
      my_wc_t wc;
      int nbytes = cs->cset->mb_wc(cs, &wc, pointer_cast<const uchar *>(beg),
                                   pointer_cast<const uchar *>(lexem->end));
      if (nbytes > 0) {
        rc = MY_COLL_LEXEM_CHAR;
        beg += nbytes;
        lexem->code = (int)wc;
        goto ex;
      }
    }

    rc = MY_COLL_LEXEM_ERROR;
    goto ex;
  }
  rc = MY_COLL_LEXEM_EOF;

ex:
  lexem->prev = lexem->beg;
  lexem->beg = beg;
  lexem->term = rc;
  return rc;
}

/*
  Collation rule item
*/

#define MY_UCA_MAX_EXPANSION 6 /* Maximum expansion length   */

struct MY_COLL_RULE {
  my_wc_t base[MY_UCA_MAX_EXPANSION];   /* Base character                  */
  my_wc_t curr[MY_UCA_MAX_CONTRACTION]; /* Current character               */
  int diff[4]; /* Primary, Secondary, Tertiary, Quaternary difference  */
  size_t before_level; /* "reset before" indicator        */
  bool with_context;
};

/**
  Return length of the "reset" string of a rule.

  @param  r  Collation customization rule

  @return    Length of r->base
*/

static inline size_t my_coll_rule_reset_length(MY_COLL_RULE *r) {
  return my_wstrnlen(r->base, MY_UCA_MAX_EXPANSION);
}

/**
  Return length of the "shift" string of a rule.

  @param  r  Collation customization rule

  @return    Length of r->base
*/

static inline size_t my_coll_rule_shift_length(MY_COLL_RULE *r) {
  return my_wstrnlen(r->curr, MY_UCA_MAX_CONTRACTION);
}

/**
  Append new character to the end of a 0-terminated wide string.

  @param  wc     Wide string
  @param  limit  Maximum possible result length
  @param  code   Character to add

  @return        1 if character was added, 0 if string was too long
*/

static int my_coll_rule_expand(my_wc_t *wc, size_t limit, my_wc_t code) {
  size_t i;
  for (i = 0; i < limit; i++) {
    if (wc[i] == 0) {
      wc[i] = code;
      return 1;
    }
  }
  return 0;
}

/**
  Initialize collation customization rule

  @param  r     Rule
*/

static void my_coll_rule_reset(MY_COLL_RULE *r) { memset(r, 0, sizeof(*r)); }

/*
  Shift methods:
  Simple: "&B < C" : weight('C') = weight('B') + 1
  Expand: weight('C') =  { weight('B'), weight(last_non_ignorable) + 1 }
*/
typedef enum {
  my_shift_method_simple = 0,
  my_shift_method_expand
} my_coll_shift_method;

struct MY_COLL_RULES {
  MY_UCA_INFO *uca;   /* Unicode weight data               */
  size_t nrules;      /* Number of rules in the rule array */
  size_t mrules;      /* Number of allocated rules         */
  MY_COLL_RULE *rule; /* Rule array                        */
  MY_CHARSET_LOADER *loader;
  my_coll_shift_method shift_after_method;
};

/**
  Realloc rule array to a new size.
  Reallocate memory for 128 additional rules at once,
  to reduce the number of reallocs, which is important
  for long tailorings (e.g. for East Asian collations).

  @param  rules   Rule container
  @param  n       new number of rules

  @return         0 on success, -1 on error.
*/

static int my_coll_rules_realloc(MY_COLL_RULES *rules, size_t n) {
  if (rules->nrules < rules->mrules ||
      (rules->rule = static_cast<MY_COLL_RULE *>(rules->loader->mem_realloc(
           rules->rule, sizeof(MY_COLL_RULE) * (rules->mrules = n + 128)))))
    return 0;
  return -1;
}

/**
  Append one new rule to a rule array

  @param  rules   Rule container
  @param  rule    New rule to add

  @return         0 on success, -1 on error.
*/

static int my_coll_rules_add(MY_COLL_RULES *rules, MY_COLL_RULE *rule) {
  if (my_coll_rules_realloc(rules, rules->nrules + 1)) return -1;
  rules->rule[rules->nrules++] = rule[0];
  return 0;
}

/**
  Apply difference at level

  @param  r      Rule
  @param  level  Level (0,1,2,3,4)
*/

static void my_coll_rule_shift_at_level(MY_COLL_RULE *r, int level) {
  switch (level) {
    case 4: /* Quaternary difference */
      r->diff[3]++;
      break;
    case 3: /* Tertiary difference */
      r->diff[2]++;
      r->diff[3] = 0;
      break;
    case 2: /* Secondary difference */
      r->diff[1]++;
      r->diff[2] = r->diff[3] = 0;
      break;
    case 1: /* Primary difference */
      r->diff[0]++;
      r->diff[1] = r->diff[2] = r->diff[3] = 0;
      break;
    case 0:
      /* Do nothing for '=': use the previous offsets for all levels */
      break;
    default:
      DBUG_ASSERT(0);
  }
}

struct MY_COLL_RULE_PARSER {
  MY_COLL_LEXEM tok[2]; /* Current token and next token for look-ahead */
  MY_COLL_RULE rule;    /* Currently parsed rule */
  MY_COLL_RULES *rules; /* Rule list pointer     */
  char errstr[128];     /* Error message         */
};

/**
  Current parser token

  @param  p   Collation customization parser

  @return     Pointer to the current token
*/

static MY_COLL_LEXEM *my_coll_parser_curr(MY_COLL_RULE_PARSER *p) {
  return &p->tok[0];
}

/**
  Next parser token, to look ahead.

  @param  p   Collation customization parser

  @return     Pointer to the next token
*/

static MY_COLL_LEXEM *my_coll_parser_next(MY_COLL_RULE_PARSER *p) {
  return &p->tok[1];
}

/**
  Scan one token from the input stream

  @param  p   Collation customization parser

  @return     1, for convenience, to use in logical expressions easier.
*/
static int my_coll_parser_scan(MY_COLL_RULE_PARSER *p) {
  my_coll_parser_curr(p)[0] = my_coll_parser_next(p)[0];
  my_coll_lexem_next(my_coll_parser_next(p));
  return 1;
}

/**
  Initialize collation customization parser

  @param  p        Collation customization parser
  @param  rules    Where to store rules
  @param  str      Beginning of a collation customization sting
  @param  str_end  End of the collation customizations string
*/

static void my_coll_parser_init(MY_COLL_RULE_PARSER *p, MY_COLL_RULES *rules,
                                const char *str, const char *str_end) {
  /*
    Initialize parser to the input buffer and scan two tokens,
    to make the current token and the next token known.
  */
  memset(p, 0, sizeof(*p));
  p->rules = rules;
  p->errstr[0] = '\0';
  my_coll_lexem_init(my_coll_parser_curr(p), str, str_end);
  my_coll_lexem_next(my_coll_parser_curr(p));
  my_coll_parser_next(p)[0] = my_coll_parser_curr(p)[0];
  my_coll_lexem_next(my_coll_parser_next(p));
}

/**
  Display error when an unexpected token found

  @param  p        Collation customization parser
  @param  term     Which lexem was expected

  @return          0, to use in "return" and boolean expressions.
*/

static int my_coll_parser_expected_error(MY_COLL_RULE_PARSER *p,
                                         my_coll_lexem_num term) {
  snprintf(p->errstr, sizeof(p->errstr), "%s expected",
           my_coll_lexem_num_to_str(term));
  return 0;
}

/**
  Display error when a too long character sequence is met

  @param  p        Collation customization parser
  @param  name     Which kind of sequence: contraction, expansion, etc.

  @return          0, to use in "return" and boolean expressions.
*/

static int my_coll_parser_too_long_error(MY_COLL_RULE_PARSER *p,
                                         const char *name) {
  snprintf(p->errstr, sizeof(p->errstr), "%s is too long", name);
  return 0;
}

/**
  Scan the given lexem from input stream, or display "expected" error.

  @param  p        Collation customization parser
  @param  term     Which lexem is expected.

  @retval          0 if the required term was not found.
  @retval          1 if the required term was found.
*/
static int my_coll_parser_scan_term(MY_COLL_RULE_PARSER *p,
                                    my_coll_lexem_num term) {
  if (my_coll_parser_curr(p)->term != term)
    return my_coll_parser_expected_error(p, term);
  return my_coll_parser_scan(p);
}

/*
  In the following code we have a few functions to parse
  various collation customization non-terminal symbols.
  Unlike our usual coding convension, they return
  - 0 on "error" (when the rule was not scanned) and
  - 1 on "success"(when the rule was scanned).
  This is done intentionally to make body of the functions look easier
  and repeat the grammar of the rules in straightforward manner.
  For example:

  // <x> ::= <y> | <z>
  int parse_x() { return parse_y() || parser_z(); }

  // <x> ::= <y> <z>
  int parse_x() { return parse_y() && parser_z(); }

  Using 1 on "not found" and 0 on "found" in the parser code would
  make the code more error prone and harder to read because
  of having to use inverse boolean logic.
*/

/**
  Scan a collation setting in brakets, for example UCA version.

  @param  p        Collation customization parser

  @retval          0 if setting was scanned.
  @retval          1 if setting was not scanned.
*/

static int my_coll_parser_scan_setting(MY_COLL_RULE_PARSER *p) {
  MY_COLL_RULES *rules = p->rules;
  MY_COLL_LEXEM *lexem = my_coll_parser_curr(p);

  if (!lex_cmp(lexem, STRING_WITH_LEN("[version 4.0.0]"))) {
    rules->uca = &my_uca_v400;
  } else if (!lex_cmp(lexem, STRING_WITH_LEN("[version 5.2.0]"))) {
    rules->uca = &my_uca_v520;
  } else if (!lex_cmp(lexem, STRING_WITH_LEN("[shift-after-method expand]"))) {
    rules->shift_after_method = my_shift_method_expand;
  } else if (!lex_cmp(lexem, STRING_WITH_LEN("[shift-after-method simple]"))) {
    rules->shift_after_method = my_shift_method_simple;
  } else {
    return 0;
  }
  return my_coll_parser_scan(p);
}

/**
  Scan multiple collation settings

  @param  p        Collation customization parser

  @retval          0 if no settings were scanned.
  @retval          1 if one or more settings were scanned.
*/

static int my_coll_parser_scan_settings(MY_COLL_RULE_PARSER *p) {
  /* Scan collation setting or special purpose command */
  while (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_OPTION) {
    if (!my_coll_parser_scan_setting(p)) return 0;
  }
  return 1;
}

/**
  Scan [before xxx] reset option

  @param  p        Collation customization parser

  @retval          0 if reset option was not scanned.
  @retval          1 if reset option was scanned.
*/

static int my_coll_parser_scan_reset_before(MY_COLL_RULE_PARSER *p) {
  MY_COLL_LEXEM *lexem = my_coll_parser_curr(p);
  if (!lex_cmp(lexem, STRING_WITH_LEN("[before primary]")) ||
      !lex_cmp(lexem, STRING_WITH_LEN("[before 1]"))) {
    p->rule.before_level = 1;
  } else if (!lex_cmp(lexem, STRING_WITH_LEN("[before secondary]")) ||
             !lex_cmp(lexem, STRING_WITH_LEN("[before 2]"))) {
    p->rule.before_level = 2;
  } else if (!lex_cmp(lexem, STRING_WITH_LEN("[before tertiary]")) ||
             !lex_cmp(lexem, STRING_WITH_LEN("[before 3]"))) {
    p->rule.before_level = 3;
  } else if (!lex_cmp(lexem, STRING_WITH_LEN("[before quaternary]")) ||
             !lex_cmp(lexem, STRING_WITH_LEN("[before 4]"))) {
    p->rule.before_level = 4;
  } else {
    p->rule.before_level = 0;
    return 0; /* Don't scan the next character */
  }
  return my_coll_parser_scan(p);
}

/**
  Scan logical position and add to the wide string.

  @param  p        Collation customization parser
  @param  pwc      Wide string to add code to
  @param  limit    The result string cannot be longer than 'limit' characters

  @retval          0 if logical position was not scanned.
  @retval          1 if logical position was scanned.
*/

static int my_coll_parser_scan_logical_position(MY_COLL_RULE_PARSER *p,
                                                my_wc_t *pwc, size_t limit) {
  MY_COLL_RULES *rules = p->rules;
  MY_COLL_LEXEM *lexem = my_coll_parser_curr(p);

  if (!lex_cmp(lexem, STRING_WITH_LEN("[first non-ignorable]")))
    lexem->code = rules->uca->first_non_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[last non-ignorable]")))
    lexem->code = rules->uca->last_non_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[first primary ignorable]")))
    lexem->code = rules->uca->first_primary_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[last primary ignorable]")))
    lexem->code = rules->uca->last_primary_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[first secondary ignorable]")))
    lexem->code = rules->uca->first_secondary_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[last secondary ignorable]")))
    lexem->code = rules->uca->last_secondary_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[first tertiary ignorable]")))
    lexem->code = rules->uca->first_tertiary_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[last tertiary ignorable]")))
    lexem->code = rules->uca->last_tertiary_ignorable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[first trailing]")))
    lexem->code = rules->uca->first_trailing;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[last trailing]")))
    lexem->code = rules->uca->last_trailing;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[first variable]")))
    lexem->code = rules->uca->first_variable;
  else if (!lex_cmp(lexem, STRING_WITH_LEN("[last variable]")))
    lexem->code = rules->uca->last_variable;
  else
    return 0; /* Don't scan the next token */

  if (!my_coll_rule_expand(pwc, limit, lexem->code)) {
    /*
      Logical position can not be in a contraction,
      so the above call should never fail.
      Let's assert in debug version and print
      a nice error message in production version.
    */
    DBUG_ASSERT(0);
    return my_coll_parser_too_long_error(p, "Logical position");
  }
  return my_coll_parser_scan(p);
}

/**
  Scan character list

    @<character list@> ::= CHAR [ CHAR... ]

  @param  p        Collation customization parser
  @param  pwc      Character string to add code to
  @param  limit    The result string cannot be longer than 'limit' characters
  @param  name     E.g. "contraction", "expansion"

  @retval          0 if character sequence was not scanned.
  @retval          1 if character sequence was scanned.
*/

static int my_coll_parser_scan_character_list(MY_COLL_RULE_PARSER *p,
                                              my_wc_t *pwc, size_t limit,
                                              const char *name) {
  if (my_coll_parser_curr(p)->term != MY_COLL_LEXEM_CHAR)
    return my_coll_parser_expected_error(p, MY_COLL_LEXEM_CHAR);

  if (!my_coll_rule_expand(pwc, limit, my_coll_parser_curr(p)->code))
    return my_coll_parser_too_long_error(p, name);

  if (!my_coll_parser_scan_term(p, MY_COLL_LEXEM_CHAR)) return 0;

  while (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_CHAR) {
    if (!my_coll_rule_expand(pwc, limit, my_coll_parser_curr(p)->code))
      return my_coll_parser_too_long_error(p, name);
    my_coll_parser_scan(p);
  }
  return 1;
}

/**
  Scan reset sequence

  @<reset sequence@> ::=
    [ @<reset before option@> ] @<character list@>
  | [ @<reset before option@> ] @<logical reset position@>

  @param  p        Collation customization parser

  @retval          0 if reset sequence was not scanned.
  @retval          1 if reset sequence was scanned.
*/

static int my_coll_parser_scan_reset_sequence(MY_COLL_RULE_PARSER *p) {
  my_coll_rule_reset(&p->rule);

  /* Scan "[before x]" option, if exists */
  if (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_OPTION)
    my_coll_parser_scan_reset_before(p);

  /* Try logical reset position */
  if (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_OPTION) {
    if (!my_coll_parser_scan_logical_position(p, p->rule.base, 1)) return 0;
  } else {
    /* Scan single reset character or expansion */
    if (!my_coll_parser_scan_character_list(p, p->rule.base,
                                            MY_UCA_MAX_EXPANSION, "Expansion"))
      return 0;
  }

  if ((p->rules->shift_after_method == my_shift_method_expand ||
       p->rule.before_level == 1) &&
      p->rules->uca->version < UCA_V900) /* Apply "before primary" option  */
  {
    /*
      Suppose we have this rule:  &B[before primary] < C
      i.e. we need to put C before B, but after A, so
      the result order is: A < C < B.

      Let primary weight of B be [BBBB].

      We cannot just use [BBBB-1] as weight for C:
      DUCET does not have enough unused weights between any two characters,
      so using [BBBB-1] will likely make C equal to the previous character,
      which is A, so we'll get this order instead of the desired: A = C < B.

      To guarantee that that C is sorted after A, we'll use expansion
      with a kind of "biggest possible character".
      As "biggest possible character" we'll use "last_non_ignorable":

      We'll compose weight for C as: [BBBB-1][MMMM+1]
      where [MMMM] is weight for "last_non_ignorable".

      We also do the same trick for "reset after" if the collation
      option says so. E.g. for the rules "&B < C", weight for
      C will be calculated as: [BBBB][MMMM+1]

      At this point we only need to store codepoints
      'B' and 'last_non_ignorable'. Actual weights for 'C'
      will be calculated according to the above formula later,
      in create_tailoring().
    */
    if (!my_coll_rule_expand(p->rule.base, MY_UCA_MAX_EXPANSION,
                             p->rules->uca->last_non_ignorable))
      return my_coll_parser_too_long_error(p, "Expansion");
  }
  return 1;
}

/**
  Scan shift sequence

  @<shift sequence@> ::=
    @<character list@>  [ / @<character list@> ]
  | @<character list@>  [ | @<character list@> ]

  @param  p        Collation customization parser

  @retval          0 if shift sequence was not scanned.
  @retval          1 if shift sequence was scanned.
*/

static int my_coll_parser_scan_shift_sequence(MY_COLL_RULE_PARSER *p) {
  MY_COLL_RULE before_extend;

  memset(&p->rule.curr, 0, sizeof(p->rule.curr));

  /* Scan single shift character or contraction */
  if (!my_coll_parser_scan_character_list(
          p, p->rule.curr, MY_UCA_MAX_CONTRACTION, "Contraction"))
    return 0;

  before_extend = p->rule; /* Remember the part before "/" */

  /* Append the part after "/" as expansion */
  if (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_EXTEND) {
    my_coll_parser_scan(p);
    if (!my_coll_parser_scan_character_list(p, p->rule.base,
                                            MY_UCA_MAX_EXPANSION, "Expansion"))
      return 0;
  } else if (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_CONTEXT) {
    /*
      We support 2-character long context sequences only:
      one character is the previous context, plus the current character.
      It's OK as Unicode's CLDR does not have longer examples.
    */
    my_coll_parser_scan(p);
    p->rule.with_context = true;
    if (!my_coll_parser_scan_character_list(
            p, p->rule.curr + 1, MY_UCA_MAX_EXPANSION - 1, "context"))
      return 0;
    /*
      It might be CONTEXT followed by EXPANSION. For example, Japanese
      collation has one rule defined as:
      "&[before 3]へ<<<へ|ゝ=べ|ゝ=へ|ゞ/\u3099"
      The part of "へ|ゞ/\u3099" is CONTEXT ('|') followed by EXPANSION ('/').
    */
    if (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_EXTEND) {
      my_coll_parser_scan(p);
      size_t len = my_wstrnlen(p->rule.base, MY_UCA_MAX_EXPANSION);
      if (!my_coll_parser_scan_character_list(
              p, p->rule.base + len, MY_UCA_MAX_EXPANSION - len, "Expansion"))
        return 0;
    }
  }

  /* Add rule to the rule list */
  if (my_coll_rules_add(p->rules, &p->rule)) return 0;

  p->rule = before_extend; /* Restore to the state before "/" */

  return 1;
}

/**
  Scan shift operator

  @<shift@> ::=  <  | <<  | <<<  | <<<<  | =

  @param  p        Collation customization parser

  @retval          0 if shift operator was not scanned.
  @retval          1 if shift operator was scanned.
*/
static int my_coll_parser_scan_shift(MY_COLL_RULE_PARSER *p) {
  if (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_SHIFT) {
    my_coll_rule_shift_at_level(&p->rule, my_coll_parser_curr(p)->diff);
    return my_coll_parser_scan(p);
  }
  return 0;
}

/**
  Scan one rule: reset followed by a number of shifts

  @<rule@> ::=
    & @<reset sequence@>
    @<shift@> @<shift sequence@>
    [ { @<shift@> @<shift sequence@> }... ]

  @param  p        Collation customization parser

  @retval          0 if rule was not scanned.
  @retval          1 if rule was scanned.
*/
static int my_coll_parser_scan_rule(MY_COLL_RULE_PARSER *p) {
  if (!my_coll_parser_scan_term(p, MY_COLL_LEXEM_RESET) ||
      !my_coll_parser_scan_reset_sequence(p))
    return 0;

  /* Scan the first required shift command */
  if (!my_coll_parser_scan_shift(p))
    return my_coll_parser_expected_error(p, MY_COLL_LEXEM_SHIFT);

  /* Scan the first shift sequence */
  if (!my_coll_parser_scan_shift_sequence(p)) return 0;

  /* Scan subsequent shift rules */
  while (my_coll_parser_scan_shift(p)) {
    if (!my_coll_parser_scan_shift_sequence(p)) return 0;
  }
  return 1;
}

/**
  Scan collation customization: settings followed by rules

  @<collation customization@> ::=
    [ @<setting@> ... ]
    [ @<rule@>... ]

  @param  p        Collation customization parser

  @retval          0 if collation customization expression was not scanned.
  @retval          1 if collation customization expression was scanned.
*/

static int my_coll_parser_exec(MY_COLL_RULE_PARSER *p) {
  if (!my_coll_parser_scan_settings(p)) return 0;

  while (my_coll_parser_curr(p)->term == MY_COLL_LEXEM_RESET) {
    if (!my_coll_parser_scan_rule(p)) return 0;
  }
  /* Make sure no unparsed input data left */
  return my_coll_parser_scan_term(p, MY_COLL_LEXEM_EOF);
}

/*
  Collation language syntax parser.
  Uses lexical parser.

  @param rules           Collation rule list to load to.
  @param str             A string with collation customization.
  @param str_end         End of the string.
  @param col_name        Collation name

  @retval                0 on success
  @retval                1 on error
*/

static int my_coll_rule_parse(MY_COLL_RULES *rules, const char *str,
                              const char *str_end, const char *col_name) {
  MY_COLL_RULE_PARSER p;

  my_coll_parser_init(&p, rules, str, str_end);

  if (!my_coll_parser_exec(&p)) {
    rules->loader->errcode = EE_COLLATION_PARSER_ERROR;
    my_coll_lexem_print_error(my_coll_parser_curr(&p), rules->loader->errarg,
                              sizeof(rules->loader->errarg) - 1, p.errstr,
                              col_name);
    return 1;
  }
  return 0;
}

static void spread_case_mask(uint16 *to, size_t to_stride,
                             size_t tailored_ce_cnt, uint16 case_mask) {
  for (size_t i = 0; i < tailored_ce_cnt; ++i) {
    uint16 *case_weight = &to[(i * MY_UCA_900_CE_SIZE + 2) * to_stride];
    if (*case_weight > CASE_FIRST_UPPER_MASK)
      case_mask = *case_weight & 0xFF00;
    else if (*case_weight)
      *case_weight |= case_mask;
  }
}

/*
 If the collation is marked as [caseFirst upper], move all of the weights
 around to accomodate that. Only tailored weights are changed; for non-tailored
 weights, we do it on-the-fly in uca_scanner_900::apply_case_first().

 [caseFirst upper] is a directive that says that case should override all
 other tertiary case concerns (in a sense, a “level 2.5”), and furthermore,
 that uppercase should come before lowercase. (Normally lowercase sorts
 before uppercase.) It is currently only used in the Danish collation.

 This is done by looking at the tertiary weight, inferring the case from it,
 and then using the upper bits (which are normally unused) to signal the case.
 The algorithm is detailed in Unicode TR35, section 3.14, although we don't
 seem to follow it exactly.
*/
static void change_weight_if_case_first(CHARSET_INFO *cs,
                                        const MY_UCA_INFO *dst, MY_COLL_RULE *r,
                                        uint16 *to, size_t to_stride,
                                        size_t curr_len,
                                        size_t tailored_ce_cnt) {
  /* We only need to implement [caseFirst upper] right now. */
  if (!(cs->coll_param && cs->coll_param->case_first == CASE_FIRST_UPPER &&
        cs->levels_for_compare == 3))
    return;

  DBUG_ASSERT(cs->uca->version == UCA_V900);

  // How many CEs this character has with non-ignorable primary weight.
  int tailored_pri_cnt = 0;
  int origin_pri_cnt = 0;
  for (size_t i = 0; i < tailored_ce_cnt; ++i) {
    /*
      If rule A has already applied a case weight change, and we have rule B
      which is inherited from A, apply the same case weight change on the rest
      of rule B and return.
    */
    if (to[(i * MY_UCA_900_CE_SIZE + 2) * to_stride] > CASE_FIRST_UPPER_MASK) {
      spread_case_mask(to, to_stride, tailored_ce_cnt, /*case_mask=*/0);
      return;
    }
    if (to[i * MY_UCA_900_CE_SIZE * to_stride]) tailored_pri_cnt++;
  }
  if (r->before_level == 1 || r->diff[0]) tailored_pri_cnt--;

  // Use the DUCET weight to detect the character's case.
  MY_UCA_INFO *src = &my_uca_v900;
  int changed_ce = 0;

  my_wc_t *curr = r->curr;
  for (size_t i = 0; i < curr_len; ++i) {
    const uint16 *from = my_char_weight_addr_900(src, *curr);
    uint page = *curr >> 8;
    uint code = *curr & 0xFF;
    curr++;
    int ce_cnt =
        src->weights[page] ? UCA900_NUM_OF_CE(src->weights[page], code) : 0;
    for (int i_ce = 0; i_ce < ce_cnt; ++i_ce) {
      if (from[i_ce * UCA900_DISTANCE_BETWEEN_WEIGHTS]) origin_pri_cnt++;
    }
  }
  int case_to_copy = 0;
  if (origin_pri_cnt <= tailored_pri_cnt)
    case_to_copy = origin_pri_cnt;
  else
    case_to_copy = tailored_pri_cnt - 1;
  int upper_cnt = 0;
  int lower_cnt = 0;
  curr = r->curr;
  uint16 case_mask = 0;
  for (size_t curr_ind = 0; curr_ind < curr_len; ++curr_ind) {
    const uint16 *from = my_char_weight_addr_900(src, *curr);
    uint page = *curr >> 8;
    uint code = *curr & 0xFF;
    curr++;
    int ce_cnt =
        src->weights[page] ? UCA900_NUM_OF_CE(src->weights[page], code) : 0;
    changed_ce = 0;
    for (int i_ce = 0; i_ce < ce_cnt; ++i_ce) {
      uint16 primary_weight = from[i_ce * UCA900_DISTANCE_BETWEEN_WEIGHTS];
      if (primary_weight) {
        uint16 case_weight = from[i_ce * UCA900_DISTANCE_BETWEEN_WEIGHTS +
                                  2 * UCA900_DISTANCE_BETWEEN_LEVELS];
        uint16 *ce_to = nullptr;
        if (is_tertiary_weight_upper_case(case_weight)) {
          if (!case_to_copy)
            upper_cnt++;
          else
            case_mask = CASE_FIRST_UPPER_MASK;
        } else {
          if (!case_to_copy)
            lower_cnt++;
          else
            case_mask = CASE_FIRST_LOWER_MASK;
        }
        if (case_to_copy) {
          do {
            ce_to = to + changed_ce * MY_UCA_900_CE_SIZE * to_stride;
            changed_ce++;
          } while (*ce_to == 0);
          ce_to[2 * to_stride] |= case_mask;
          case_to_copy--;
        }
      }
    }
  }
  if (origin_pri_cnt <= tailored_pri_cnt) {
    for (int i = origin_pri_cnt; i < tailored_pri_cnt; ++i) {
      const int offset = changed_ce * MY_UCA_900_CE_SIZE * to_stride;
      if (to[offset] && to[offset] < dst->extra_ce_pri_base)
        to[offset + 2 * to_stride] = 0;
    }
  } else {
    if (upper_cnt && lower_cnt)
      case_mask = CASE_FIRST_MIXED_MASK;
    else if (upper_cnt && !lower_cnt)
      case_mask = CASE_FIRST_UPPER_MASK;
    else
      case_mask = CASE_FIRST_LOWER_MASK;
    bool skipped_extra_ce = false;
    for (int i = tailored_ce_cnt - 1; i >= 0; --i) {
      int offset = i * MY_UCA_900_CE_SIZE * to_stride;
      if (to[offset] && to[offset] < dst->extra_ce_pri_base) {
        if ((r->before_level == 1 || r->diff[0]) && !skipped_extra_ce) {
          skipped_extra_ce = true;
          continue;
        }
        to[(i * MY_UCA_900_CE_SIZE + 2) * to_stride] |= case_mask;
        break;
      }
    }
  }
  spread_case_mask(to, to_stride, tailored_ce_cnt, case_mask);
}

static size_t my_char_weight_put_900(MY_UCA_INFO *dst, uint16 *to,
                                     size_t to_stride, size_t to_length,
                                     uint16 *to_num_ce,
                                     const MY_COLL_RULE *rule,
                                     size_t base_len) {
  size_t count;
  int total_ce_cnt = 0;

  const my_wc_t *base = rule->base;
  for (count = 0; base_len;) {
    const uint16 *from = nullptr;
    size_t from_stride = 0;
    int ce_cnt = 0;

    for (size_t chlen = base_len; chlen > 1; chlen--) {
      if ((from = my_uca_contraction_weight(dst->contraction_nodes, base,
                                            chlen))) {
        from_stride = 1;
        base += chlen;
        base_len -= chlen;
        ce_cnt = *(from + MY_UCA_MAX_WEIGHT_SIZE - 1);
        break;
      }
    }

    if (!from) {
      uint page = *base >> 8;
      uint code = *base & 0xFF;
      base++;
      base_len--;
      if (dst->weights[page]) {
        from = UCA900_WEIGHT_ADDR(dst->weights[page], /*level=*/0, code);
        from_stride = UCA900_DISTANCE_BETWEEN_LEVELS;
        ce_cnt = UCA900_NUM_OF_CE(dst->weights[page], code);
      }
    }

    for (int weight_ind = 0;
         weight_ind < ce_cnt * MY_UCA_900_CE_SIZE && count < to_length;
         weight_ind++) {
      *to = *from;
      to += to_stride;
      from += from_stride;
      count++;
    }
    total_ce_cnt += ce_cnt;
  }

  /*
    For shift on primary weight, there might be no enough room in the tables.
    For example, Sihala has the rule "&\\u0DA5 < \\u0DA4", which means
    that we should move U+0DA4 after U+0DA5 (on the primary level).
    However, there is no room after U+0DA5 in DUCET unless we wanted to
    conflict with U+0DA6:

      0DA4  ; [.28EC.0020.0002] # SINHALA LETTER TAALUJA NAASIKYAYA
      0DA5  ; [.28ED.0020.0002] # SINHALA LETTER TAALUJA SANYOOGA NAAKSIKYAYA
      0DA6  ; [.28EE.0020.0002] # SINHALA LETTER SANYAKA JAYANNA

    Before our implementation of UCA 9.0.0, the shift on primary weight was
    done by making it a fake expansion when parsing the rule, where we'd expand
    U+0DA4 to U+0DA5 U+MMMM, MMMM being 'last_non_ignorable'. (This happens
    in my_coll_parser_scan_reset_sequence()). But from UCA 9.0.0, we also
    support accent- and case-sensitive collations, and then, having the extra
    weights of 'last_non_ignorable' (which is just a random character) on the
    second and third level may cause unexpected results for algorithms that
    use the meaning of the tertiary weight to infer case. Thus, we'll abandon
    the fake expansion way; instead, instead add an extra CE (after the one
    from U+0DA5, the character we are moving after) to represent all the
    weights we might want to shift. The actual shifting happens in
    apply_shift_900().

    For the rule "&\\u0DA5 < \\u0DA4", U+0DA4's weights become
    [.28ED.0020.0002][.54A4.0000.0000], where 0x54A4 is the value of
    extra_ce_pri_base. We then apply the differences from the rule
    (which are never negative) to the last CE, so that it becomes
    e.g. [.54A5.0000.0000].
  */
  if ((rule->diff[0] || rule->diff[1] || rule->diff[2]) && count < to_length) {
    *to = rule->diff[0] ? dst->extra_ce_pri_base : 0;
    to += to_stride;
    *to = rule->diff[1] ? dst->extra_ce_sec_base : 0;
    to += to_stride;
    *to = rule->diff[2] ? dst->extra_ce_ter_base : 0;
    to += to_stride;
    total_ce_cnt++;
    count += 3;
  }
  total_ce_cnt =
      std::min(total_ce_cnt, (MY_UCA_MAX_WEIGHT_SIZE - 1) / MY_UCA_900_CE_SIZE);
  *to_num_ce = total_ce_cnt;

  return total_ce_cnt;
}

/**
  Helper function:
  Copies UCA weights for a given "uint" string
  to the given location.

  @param dst        destination UCA weight data
  @param to         destination address
  @param to_stride  number of bytes between each successive weight in "to"
  @param to_length  size of destination
  @param to_num_ce  where to put the number of CEs generated
  @param rule       The rule that contains the characters whose weight
                    are to copied
  @param base_len   The length of base character list
  @param uca_ver    UCA version

  @return    number of weights put
*/

static size_t my_char_weight_put(MY_UCA_INFO *dst, uint16 *to, size_t to_stride,
                                 size_t to_length, uint16 *to_num_ce,
                                 const MY_COLL_RULE *rule, size_t base_len,
                                 enum_uca_ver uca_ver) {
  if (uca_ver == UCA_V900)
    return my_char_weight_put_900(dst, to, to_stride, to_length, to_num_ce,
                                  rule, base_len);

  const my_wc_t *base = rule->base;
  size_t count = 0;
  while (base_len != 0) {
    const uint16 *from = nullptr;

    for (size_t chlen = base_len; chlen > 1; chlen--) {
      if ((from = my_uca_contraction_weight(dst->contraction_nodes, base,
                                            chlen))) {
        base += chlen;
        base_len -= chlen;
        break;
      }
    }

    if (!from) {
      from = my_char_weight_addr(dst, *base);
      base++;
      base_len--;
    }

    for (; from && *from && count < to_length;) {
      *to = *from++;
      to += to_stride;
      count++;
    }
  }

  *to = 0;
  return count;
}

/**
  Alloc new page and copy the default UCA weights
  @param cs       Character set
  @param loader   Character set loader
  @param src      Default UCA data to copy from
  @param dst      UCA data to copy weights to
  @param page     page number

  @retval false on success
  @retval true  on error
*/
static bool my_uca_copy_page(CHARSET_INFO *cs, MY_CHARSET_LOADER *loader,
                             const MY_UCA_INFO *src, MY_UCA_INFO *dst,
                             size_t page) {
  const uint dst_size = 256 * dst->lengths[page] * sizeof(uint16);
  if (!(dst->weights[page] = (uint16 *)(loader->once_alloc)(dst_size)))
    return true;

  DBUG_ASSERT(src->lengths[page] <= dst->lengths[page]);
  memset(dst->weights[page], 0, dst_size);
  if (cs->uca && cs->uca->version == UCA_V900) {
    const uint src_size = 256 * src->lengths[page] * sizeof(uint16);
    memcpy(dst->weights[page], src->weights[page], src_size);
  } else if (src->lengths[page] > 0) {
    for (uint chc = 0; chc < 256; chc++) {
      memcpy(dst->weights[page] + chc * dst->lengths[page],
             src->weights[page] + chc * src->lengths[page],
             src->lengths[page] * sizeof(uint16));
    }
  }
  return false;
}

/*
  This is used to apply the weight shift if there is a [before 1] rule.
  If we have a rule "&[before 1] A < B < C", and A's collation element is [P, S,
  T], then in my_char_weight_put_900(), we append one extra collation element to
  A's CE to be B and C's CE. So B and C's CE becomes [P, S, T][p, 0, 0]. What we
  do with this function is to change B's CE to [P - 1, S, T][p + n, 0, 0].
  1. The rule "&[before 1] A < B < C" means "B < C < A" on primary level. Since
     "B < A", so we give B the first primary weight as (P - 1).
  2. p is a weight value which is the maximum regular primary weight in DUCET
     plus one (0x54A3 + 1 = 0x54A4). This is to make sure B's primary weight
     less than A and greater than any character which sorts before A.
  3. n is the number of characters in this rule's character list. For the B in
     this rule, n = 1. For the C in this rule, n = 2. This can make sure "B <
     C".

  It is the same thing that apply_secondary_shift_900() and
  apply_tertiary_shift_900() do, but on different weight levels.
 */
static bool apply_primary_shift_900(MY_CHARSET_LOADER *loader,
                                    MY_COLL_RULES *rules, MY_COLL_RULE *r,
                                    uint16 *to, size_t to_stride,
                                    size_t nweights,
                                    uint16 *const last_weight_ptr) {
  /*
    Find the second-to-last non-ignorable primary weight to apply shift,
    because the last one is the extra CE we added in my_char_weight_put_900().
  */
  int last_sec_pri = 0;
  for (last_sec_pri = nweights - 2; last_sec_pri >= 0; --last_sec_pri) {
    if (to[last_sec_pri * to_stride * MY_UCA_900_CE_SIZE]) break;
  }
  if (last_sec_pri >= 0) {
    to[last_sec_pri * to_stride * MY_UCA_900_CE_SIZE]--; /* Reset before */
    if (rules->shift_after_method == my_shift_method_expand) {
      /*
        Special case. Don't let characters shifted after X
        and before next(X) intermix to each other.

        For example:
        "[shift-after-method expand] &0 < a &[before primary]1 < A".
        I.e. we reorder 'a' after '0', and then 'A' before '1'.
        'a' must be sorted before 'A'.

        Note, there are no real collations in CLDR which shift
        after and before two neighbouring characters. We need this
        just in case. Reserving 4096 (0x1000) weights for such
        cases is perfectly enough.
      */
      /* W3-TODO: const may vary on levels 2,3*/
      last_weight_ptr[0] += 0x1000;
    }
  } else {
    loader->errcode = EE_FAILED_TO_RESET_BEFORE_PRIMARY_IGNORABLE_CHAR;
    snprintf(loader->errarg, sizeof(loader->errarg), "U+%04lX", r->base[0]);
    return true;
  }
  return false;
}

/*
  This is used to apply the weight shift if there is a [before 2] rule. Please
  see the comment on apply_primary_shift_900().
 */
static bool apply_secondary_shift_900(MY_CHARSET_LOADER *loader,
                                      MY_COLL_RULES *rules, MY_COLL_RULE *r,
                                      uint16 *to, size_t to_stride,
                                      size_t nweights,
                                      uint16 *const last_weight_ptr) {
  /*
    Find the second-to-last non-ignorable secondary weight to apply shift,
    because the last one is the extra CE we added in my_char_weight_put_900().
  */
  int last_sec_sec;
  for (last_sec_sec = nweights - 2; last_sec_sec >= 0; --last_sec_sec) {
    if (to[last_sec_sec * MY_UCA_900_CE_SIZE * to_stride + to_stride]) break;
  }
  if (last_sec_sec >= 0) {
    // Reset before.
    to[last_sec_sec * MY_UCA_900_CE_SIZE * to_stride + to_stride]--;
    if (rules->shift_after_method == my_shift_method_expand) {
      /*
        Same reason as in apply_primary_shift_900(), reserve 256 (0x100)
        weights for secondary level.
      */
      last_weight_ptr[to_stride] += 0x100;
    }
  } else {
    loader->errcode = EE_FAILED_TO_RESET_BEFORE_SECONDARY_IGNORABLE_CHAR;
    snprintf(loader->errarg, sizeof(loader->errarg), "U+%04lX", r->base[0]);
    return true;
  }
  return false;
}

/*
  This is used to apply the weight shift if there is a [before 3] rule. Please
  see the comment on apply_primary_shift_900().
 */
static bool apply_tertiary_shift_900(MY_CHARSET_LOADER *loader,
                                     MY_COLL_RULES *rules, MY_COLL_RULE *r,
                                     uint16 *to, size_t to_stride,
                                     size_t nweights,
                                     uint16 *const last_weight_ptr) {
  /*
    Find the second-to-last non-ignorable tertiary weight to apply shift,
    because the last one is the extra CE we added in my_char_weight_put_900().
  */
  int last_sec_ter;
  for (last_sec_ter = nweights - 2; last_sec_ter >= 0; --last_sec_ter) {
    if (to[last_sec_ter * MY_UCA_900_CE_SIZE * to_stride + 2 * to_stride])
      break;
  }
  if (last_sec_ter >= 0) {
    // Reset before.
    to[last_sec_ter * MY_UCA_900_CE_SIZE * to_stride + 2 * to_stride]--;
    if (rules->shift_after_method == my_shift_method_expand) {
      /*
        Same reason as in apply_primary_shift_900(), reserve 16 (0x10)
        weights for tertiary level.
      */
      last_weight_ptr[to_stride * 2] += 0x10;
    }
  } else {
    loader->errcode = EE_FAILED_TO_RESET_BEFORE_TERTIARY_IGNORABLE_CHAR;
    snprintf(loader->errarg, sizeof(loader->errarg), "U+%04lX", r->base[0]);
    return true;
  }
  return false;
}

static bool apply_shift_900(MY_CHARSET_LOADER *loader, MY_COLL_RULES *rules,
                            MY_COLL_RULE *r, uint16 *to, size_t to_stride,
                            size_t nweights) {
  // nweights should not less than 1 because of the extra CE.
  DBUG_ASSERT(nweights);
  // Apply level difference.
  uint16 *const last_weight_ptr =
      to + (nweights - 1) * to_stride * MY_UCA_900_CE_SIZE;
  last_weight_ptr[0] += r->diff[0];
  last_weight_ptr[to_stride] += r->diff[1];
  last_weight_ptr[to_stride * 2] += r->diff[2];
  if (r->before_level == 1)  // Apply "&[before primary]".
    return apply_primary_shift_900(loader, rules, r, to, to_stride, nweights,
                                   last_weight_ptr);
  else if (r->before_level == 2)  // Apply "[before 2]".
    return apply_secondary_shift_900(loader, rules, r, to, to_stride, nweights,
                                     last_weight_ptr);
  else if (r->before_level == 3)  // Apply "[before 3]".
    return apply_tertiary_shift_900(loader, rules, r, to, to_stride, nweights,
                                    last_weight_ptr);
  return false;
}

static bool apply_shift(MY_CHARSET_LOADER *loader, MY_COLL_RULES *rules,
                        MY_COLL_RULE *r, int level, uint16 *to,
                        size_t to_stride, size_t nweights) {
  if (rules->uca->version == UCA_V900)
    return apply_shift_900(loader, rules, r, to, to_stride, nweights);

  DBUG_ASSERT(to_stride == 1);

  /* Apply level difference. */
  if (nweights) {
    to[nweights - 1] += r->diff[0];
    if (r->before_level == 1) /* Apply "&[before primary]" */
    {
      if (nweights >= 2) {
        to[nweights - 2]--; /* Reset before */
        if (rules->shift_after_method == my_shift_method_expand) {
          /*
            Special case. Don't let characters shifted after X
            and before next(X) intermix to each other.

            For example:
            "[shift-after-method expand] &0 < a &[before primary]1 < A".
            I.e. we reorder 'a' after '0', and then 'A' before '1'.
            'a' must be sorted before 'A'.

            Note, there are no real collations in CLDR which shift
            after and before two neighbourgh characters. We need this
            just in case. Reserving 4096 (0x1000) weights for such
            cases is perfectly enough.
          */
          /* W3-TODO: const may vary on levels 2,3*/
          to[nweights - 1] += 0x1000;
        }
      } else {
        loader->errcode = EE_FAILED_TO_RESET_BEFORE_PRIMARY_IGNORABLE_CHAR;
        snprintf(loader->errarg, sizeof(loader->errarg), "U+%04lX", r->base[0]);
        return true;
      }
    }
  } else {
    /* Shift to an ignorable character, e.g.: & \u0000 < \u0001 */
    DBUG_ASSERT(to[0] == 0);
    to[0] = r->diff[level];
  }
  return false;
}

static MY_CONTRACTION *add_contraction_to_trie(
    std::vector<MY_CONTRACTION> *cont_nodes, MY_COLL_RULE *r) {
  MY_CONTRACTION new_node{0, {}, {}, {}, false, 0};
  if (r->with_context)  // previous-context contraction
  {
    DBUG_ASSERT(my_wstrnlen(r->curr, MY_UCA_MAX_CONTRACTION) == 2);
    std::vector<MY_CONTRACTION>::iterator node_it =
        find_contraction_part_in_trie(*cont_nodes, r->curr[1]);
    if (node_it == cont_nodes->end() || node_it->ch != r->curr[1]) {
      new_node.ch = r->curr[1];
      node_it = cont_nodes->insert(node_it, new_node);
    }
    cont_nodes = &node_it->child_nodes_context;

    node_it = find_contraction_part_in_trie(*cont_nodes, r->curr[0]);
    if (node_it == cont_nodes->end() || node_it->ch != r->curr[0]) {
      new_node.ch = r->curr[0];
      node_it = cont_nodes->insert(node_it, new_node);
    }
    node_it->is_contraction_tail = true;
    node_it->contraction_len = 2;
    return &(*node_it);
  } else  // normal contraction
  {
    size_t contraction_len = my_wstrnlen(r->curr, MY_UCA_MAX_CONTRACTION);
    std::vector<MY_CONTRACTION>::iterator node_it;
    for (size_t ch_ind = 0; ch_ind < contraction_len; ++ch_ind) {
      node_it = find_contraction_part_in_trie(*cont_nodes, r->curr[ch_ind]);
      if (node_it == cont_nodes->end() || node_it->ch != r->curr[ch_ind]) {
        new_node.ch = r->curr[ch_ind];
        node_it = cont_nodes->insert(node_it, new_node);
      }
      cont_nodes = &node_it->child_nodes;
    }
    node_it->is_contraction_tail = true;
    node_it->contraction_len = contraction_len;
    return &(*node_it);
  }
}

static bool apply_one_rule(CHARSET_INFO *cs, MY_CHARSET_LOADER *loader,
                           MY_COLL_RULES *rules, MY_COLL_RULE *r, int level,
                           MY_UCA_INFO *dst) {
  size_t nweights;
  size_t nreset = my_coll_rule_reset_length(r); /* Length of reset sequence */
  size_t nshift = my_coll_rule_shift_length(r); /* Length of shift sequence */
  uint16 *to, *to_num_ce;
  size_t to_stride;

  if (nshift >= 2) /* Contraction */
  {
    size_t i;
    int flag;
    /* Add HEAD, MID and TAIL flags for the contraction parts */
    my_uca_add_contraction_flag(
        dst->contraction_flags, r->curr[0],
        r->with_context ? MY_UCA_PREVIOUS_CONTEXT_HEAD : MY_UCA_CNT_HEAD);
    for (i = 1, flag = MY_UCA_CNT_MID1; i < nshift - 1; i++, flag <<= 1)
      my_uca_add_contraction_flag(dst->contraction_flags, r->curr[i], flag);
    my_uca_add_contraction_flag(
        dst->contraction_flags, r->curr[i],
        r->with_context ? MY_UCA_PREVIOUS_CONTEXT_TAIL : MY_UCA_CNT_TAIL);
    /* Add new contraction to the contraction list */
    MY_CONTRACTION *trie_node =
        add_contraction_to_trie(dst->contraction_nodes, r);
    to = trie_node->weight;
    to_stride = 1;
    to_num_ce = &to[MY_UCA_MAX_WEIGHT_SIZE - 1];
    /* Store weights of the "reset to" character */
    nweights =
        my_char_weight_put(dst, to, to_stride, MY_UCA_MAX_WEIGHT_SIZE - 1,
                           to_num_ce, r, nreset, rules->uca->version);
  } else {
    my_wc_t pagec = (r->curr[0] >> 8);
    DBUG_ASSERT(dst->weights[pagec]);
    if (cs->uca && cs->uca->version == UCA_V900) {
      to = my_char_weight_addr_900(dst, r->curr[0]);
      to_stride = UCA900_DISTANCE_BETWEEN_LEVELS;
      to_num_ce = to - UCA900_DISTANCE_BETWEEN_LEVELS;
    } else {
      to = my_char_weight_addr(dst, r->curr[0]);
      to_stride = 1;
      to_num_ce = to + (dst->lengths[pagec] - 1);
    }
    /* Store weights of the "reset to" character */
    if (dst->lengths[pagec] == 0)
      nweights = 0;
    else
      nweights = my_char_weight_put(dst, to, to_stride, dst->lengths[pagec] - 1,
                                    to_num_ce, r, nreset, rules->uca->version);
  }

  change_weight_if_case_first(cs, dst, r, to, to_stride, nshift, nweights);
  /* Apply level difference. */
  return apply_shift(loader, rules, r, level, to, to_stride, nweights);
}

/**
  Check if collation rules are valid,
  i.e. characters are not outside of the collation suported range.
*/
static int check_rules(MY_CHARSET_LOADER *loader, const MY_COLL_RULES *rules,
                       const MY_UCA_INFO *dst, const MY_UCA_INFO *src) {
  const MY_COLL_RULE *r, *rlast;
  for (r = rules->rule, rlast = rules->rule + rules->nrules; r < rlast; r++) {
    if (r->curr[0] > dst->maxchar) {
      loader->errcode = EE_SHIFT_CHAR_OUT_OF_RANGE;
      snprintf(loader->errarg, sizeof(loader->errarg), "u%04X",
               (uint)r->curr[0]);
      return true;
    } else if (r->base[0] > src->maxchar) {
      loader->errcode = EE_RESET_CHAR_OUT_OF_RANGE;
      snprintf(loader->errarg, sizeof(loader->errarg), "u%04X",
               (uint)r->base[0]);
      return true;
    }
  }
  return false;
}

static void synthesize_lengths_900(uchar *lengths, const uint16 *const *weights,
                                   uint npages) {
  for (uint page = 0; page < npages; ++page) {
    int max_len = 0;
    if (weights[page]) {
      for (uint code = 0; code < 256; ++code) {
        max_len = std::max<int>(max_len, weights[page][code]);
      }
    }
    if (max_len == 0)
      lengths[page] = 0;
    else
      lengths[page] = max_len * MY_UCA_900_CE_SIZE + 1;
  }
}

static void copy_ja_han_pages(const CHARSET_INFO *cs, MY_UCA_INFO *dst) {
  if (!cs->uca || cs->uca->version != UCA_V900 ||
      cs->coll_param != &ja_coll_param)
    return;
  for (int page = MIN_JA_HAN_PAGE; page <= MAX_JA_HAN_PAGE; page++) {
    // In DUCET, weight is not assigned to code points in [U+4E00, U+9FFF].
    DBUG_ASSERT(dst->weights[page] == nullptr);
    dst->weights[page] = ja_han_pages[page - MIN_JA_HAN_PAGE];
  }
}

/*
  We have reordered all the characters in the pages which contains Chinese Han
  characters with uca9dump (see dump_zh_pages() in uca9-dump.cc). Replace the
  DUCET pages with these pages.
 */
static void copy_zh_han_pages(MY_UCA_INFO *dst) {
  for (int page = MIN_ZH_HAN_PAGE; page <= MAX_ZH_HAN_PAGE; page++) {
    if (zh_han_pages[page - MIN_ZH_HAN_PAGE]) {
      dst->weights[page] = zh_han_pages[page - MIN_ZH_HAN_PAGE];
    }
  }
}

/*
  UCA defines an algorithm to calculate character's implicit weight if this
  character's weight is not defined in the DUCET. This function is to help
  convert Chinese character's implicit weight calculated by UCA back to its code
  points.
  The implicit weight and the code point is not 1 : 1 map because DUCET lets
  some characters share implicit primary weight. For example, the DUCET defines
  "2F00  ; [.FB40.0020.0004][.CE00.0000.0000] # KANGXI RADICAL ONE", and 4E00's
  implicit weight is [.FB40.0020.0002][.CE00.0000.0000]. We can see the primary
  weights of U+2F00 and U+4E00 are same (FB40 CE00).

  But for the Han characters in zh.xml file, each one has unique implicit
  weight.
 */
static inline my_wc_t convert_implicit_to_ch(uint16 first, uint16 second) {
  /*
    For reference, here is how UCA calculates one character's implicit weight.
    AAAA = 0xFB40 + (CP >> 15)  # The 0xFB40 changes for different character
                                # groups
    BBBB = (CP & 0x7FFF) | 0x8000
   */
  if (first < 0xFB80)
    return (((first - 0xFB40) << 15) | (second & 0x7FFF));
  else if (first < 0xFBC0)
    return (((first - 0xFB80) << 15) | (second & 0x7FFF));
  else
    return (((first - 0xFBC0) << 15) | (second & 0x7FFF));
}

/*
  Usually we do reordering in apply_reorder_param(). But for the Chinese
  collation, since we want to remove the weight gap between the character groups
  (see the comment on change_zh_implicit()), and we have done the reordering for
  some characters in the pages which contains Chinese Han characters, if we
  still use apply_reorder_param() to do the reordering for other characters, we
  might meet weight conflict. For example, in the DUCET page, 'A' has primary
  weight 0x1C47, but this value has been assigned to the first Chinese Han
  character in CLDR zh.xml file.
  So we do the reordering for all the DUCET pages when initializing the
  collation.
 */
static void modify_all_zh_pages(Reorder_param *reorder_param, MY_UCA_INFO *dst,
                                int npages) {
  std::map<int, int> zh_han_to_single_weight_map;
  for (int i = 0; i < ZH_HAN_WEIGHT_PAIRS; i++) {
    zh_han_to_single_weight_map[zh_han_to_single_weight[i * 2]] =
        zh_han_to_single_weight[i * 2 + 1];
  }

  for (int page = 0; page < npages; page++) {
    /*
      If there is no page in the DUCET, then all the characters in this page
      must have implicit weight. The reordering for it will be done by
      change_zh_implicit(). Do not need to change here.
      If there is page in zh_han_pages[], then all the characters in this page
      have been reordered by uca9dump. Do not need to change here.
     */
    if (!dst->weights[page] ||
        (page >= MIN_ZH_HAN_PAGE && page <= MAX_ZH_HAN_PAGE &&
         zh_han_pages[page - MIN_ZH_HAN_PAGE]))
      continue;
    for (int off = 0; off < 256; off++) {
      uint16 *wbeg = UCA900_WEIGHT_ADDR(dst->weights[page], 0, off);
      int num_of_ce = UCA900_NUM_OF_CE(dst->weights[page], off);
      for (int ce = 0; ce < num_of_ce; ce++) {
        DBUG_ASSERT(reorder_param->wt_rec_num == 1);
        if (*wbeg >= reorder_param->wt_rec[0].old_wt_bdy.begin &&
            *wbeg <= reorder_param->wt_rec[0].old_wt_bdy.end) {
          *wbeg = *wbeg + reorder_param->wt_rec[0].new_wt_bdy.begin -
                  reorder_param->wt_rec[0].old_wt_bdy.begin;
        } else if (*wbeg >= 0xFB00) {
          uint16 next_wt = *(wbeg + UCA900_DISTANCE_BETWEEN_WEIGHTS);
          if (*wbeg >= 0xFB40 && *wbeg <= 0xFBC1) {  // Han's implicit weight
            /*
              If some characters in DUCET share the same implicit weight, their
              reordered weight should be same too.
             */
            my_wc_t ch = convert_implicit_to_ch(*wbeg, next_wt);
            if (zh_han_to_single_weight_map.find(ch) !=
                zh_han_to_single_weight_map.end()) {
              *wbeg = zh_han_to_single_weight_map[ch];
              *(wbeg + UCA900_DISTANCE_BETWEEN_WEIGHTS) = 0;
              wbeg += UCA900_DISTANCE_BETWEEN_WEIGHTS;
              ce++;
              continue;
            }
          }
          *wbeg = change_zh_implicit(*wbeg);
          wbeg += UCA900_DISTANCE_BETWEEN_WEIGHTS;
          ce++;
        }
        wbeg += UCA900_DISTANCE_BETWEEN_WEIGHTS;
      }
    }
  }
}

static bool init_weight_level(CHARSET_INFO *cs, MY_CHARSET_LOADER *loader,
                              MY_COLL_RULES *rules, int level, MY_UCA_INFO *dst,
                              const MY_UCA_INFO *src,
                              bool lengths_are_temporary) {
  MY_COLL_RULE *r, *rlast;
  size_t i, npages = (src->maxchar + 1) / 256;
  bool has_contractions = false;

  dst->maxchar = src->maxchar;

  if (check_rules(loader, rules, dst, src)) return true;

  /* Allocate memory for pages and their lengths */
  if (lengths_are_temporary) {
    if (!(dst->lengths = (uchar *)(loader->mem_malloc)(npages))) return true;
    if (!(dst->weights =
              (uint16 **)(loader->once_alloc)(npages * sizeof(uint16 *)))) {
      (loader->mem_free)(dst->lengths);
      return true;
    }
  } else {
    if (!(dst->lengths = (uchar *)(loader->once_alloc)(npages)) ||
        !(dst->weights =
              (uint16 **)(loader->once_alloc)(npages * sizeof(uint16 *))))
      return true;
  }

  /*
    Copy pages lengths and page pointers from the default UCA weights.
  */
  memcpy(dst->lengths, src->lengths, npages);
  memcpy(dst->weights, src->weights, npages * sizeof(uint16 *));

  /*
    Calculate maximum lenghts for the pages which will be overwritten.
    Mark pages that will be overwritten as NULL.
    We'll allocate their own memory.
  */
  for (r = rules->rule, rlast = rules->rule + rules->nrules; r < rlast; r++) {
    if (!r->curr[1]) /* If not a contraction */
    {
      uint pagec = (r->curr[0] >> 8);
      if (r->base[1]) /* Expansion */
      {
        /* Reserve space for maximum possible length */
        dst->lengths[pagec] = MY_UCA_MAX_WEIGHT_SIZE;
      } else {
        uint pageb = (r->base[0] >> 8);
        if ((r->diff[0] || r->diff[1] || r->diff[2]) &&
            dst->lengths[pagec] < (src->lengths[pageb] + 3)) {
          if ((src->lengths[pageb] + 3) > MY_UCA_MAX_WEIGHT_SIZE)
            dst->lengths[pagec] = MY_UCA_MAX_WEIGHT_SIZE;
          else
            dst->lengths[pagec] = src->lengths[pageb] + 3;
        } else if (dst->lengths[pagec] < src->lengths[pageb])
          dst->lengths[pagec] = src->lengths[pageb];
      }
      dst->weights[pagec] = nullptr; /* Mark that we'll overwrite this page */
    } else
      has_contractions = true;
  }

  if (has_contractions) {
    dst->have_contractions = true;
    dst->contraction_nodes = new std::vector<MY_CONTRACTION>(0);
    if (!(dst->contraction_flags =
              (char *)(loader->once_alloc)(MY_UCA_CNT_FLAG_SIZE)))
      return true;
    memset(dst->contraction_flags, 0, MY_UCA_CNT_FLAG_SIZE);
  }
  if (cs->coll_param == &zh_coll_param) {
    /*
      We are going to reorder the weight of characters in uca pages when
      initializing this collation. And because of the reorder rule [reorder
      Hani], we need to change almost every character's weight. So copy all
      the pages.
      Please also see the comment on modify_all_zh_pages().
     */
    bool rc;
    for (i = 0; i < npages; i++) {
      if (dst->lengths[i] && (rc = my_uca_copy_page(cs, loader, src, dst, i)))
        return rc;
    }
    modify_all_zh_pages(cs->coll_param->reorder_param, dst, npages);
    copy_zh_han_pages(dst);
  } else {
    /* Allocate pages that we'll overwrite and copy default weights */
    for (i = 0; i < npages; i++) {
      bool rc;
      /*
        Don't touch pages with lengths[i]==0, they have implicit weights
        calculated algorithmically.
      */
      if (!dst->weights[i] && dst->lengths[i] &&
          (rc = my_uca_copy_page(cs, loader, src, dst, i)))
        return rc;
    }

    copy_ja_han_pages(cs, dst);
  }

  /*
    Preparatory step is done at this point.
    Now we have memory allocated for the pages that we'll overwrite,
    and for contractions, including previous context contractions.
    Also, for the pages that we'll overwrite, we have copied default weights.
    Now iterate through the rules, overwrite weights for the characters
    that appear in the rules, and put all contractions into contraction list.
  */
  for (r = rules->rule; r < rlast; r++) {
    if (apply_one_rule(cs, loader, rules, r, level, dst)) return true;
  }
  return false;
}

/**
  Check whether the composition character is already in rule list
  @param  rules   The rule list
  @param  wc      The composition character
  @return true    The composition character is already in list
          false   The composition character is not in list
*/
static bool my_comp_in_rulelist(const MY_COLL_RULES *rules, my_wc_t wc) {
  MY_COLL_RULE *r, *rlast;
  for (r = rules->rule, rlast = rules->rule + rules->nrules; r < rlast; r++) {
    if (r->curr[0] == wc && r->curr[1] == 0) return true;
  }
  return false;
}

/**
  Check whether a composition character in the decomposition list is a
  normal character.
  @param  dec_ind   The index of composition character in list
  @return           Whether it is a normal character
*/
static inline bool my_compchar_is_normal_char(uint dec_ind) {
  return uni_dec[dec_ind].decomp_tag == DECOMP_TAG_NONE;
}

static inline bool my_compchar_is_normal_char(const Unidata_decomp *decomp) {
  return my_compchar_is_normal_char(decomp - std::begin(uni_dec));
}

static Unidata_decomp *get_decomposition(my_wc_t ch) {
  auto comp_func = [](Unidata_decomp x, Unidata_decomp y) {
    return x.charcode < y.charcode;
  };
  Unidata_decomp to_find = {ch, CHAR_CATEGORY_LU, DECOMP_TAG_NONE, {0}};
  Unidata_decomp *decomp = std::lower_bound(
      std::begin(uni_dec), std::end(uni_dec), to_find, comp_func);
  if (decomp == std::end(uni_dec) || decomp->charcode != ch) return nullptr;
  return decomp;
}

static Combining_mark *my_find_combining_mark(my_wc_t code) {
  auto comp_func = [](Combining_mark x, Combining_mark y) {
    return x.charcode < y.charcode;
  };
  Combining_mark to_find = {code, 0};
  return std::lower_bound(std::begin(combining_marks),
                          std::end(combining_marks), to_find, comp_func);
}

/**
  Check if a list of combining marks contains the whole list of origin
  decomposed combining marks.
  @param    origin_dec    The origin list of combining marks decomposed from
                          character in tailoring rule.
  @param    dec_codes     The list of combining marks decomposed from
                          character in decomposition list.
  @param    dec_diff      The combining marks exist in dec_codes but not in
                          origin_dec.
  @return                 Whether the list of combining marks contains the
                          whole list of origin combining marks.
*/
static bool my_is_inheritance_of_origin(const my_wc_t *origin_dec,
                                        const my_wc_t *dec_codes,
                                        my_wc_t *dec_diff) {
  int ind0, ind1, ind2;
  if (origin_dec[0] != dec_codes[0]) return false;
  for (ind0 = ind1 = ind2 = 1; ind0 < MY_UCA_MAX_CONTRACTION &&
                               ind1 < MY_UCA_MAX_CONTRACTION &&
                               origin_dec[ind0] && dec_codes[ind1];) {
    if (origin_dec[ind0] == dec_codes[ind1]) {
      ind0++;
      ind1++;
    } else {
      Combining_mark *mark0 = my_find_combining_mark(origin_dec[ind0]);
      Combining_mark *mark1 = my_find_combining_mark(dec_codes[ind1]);
      if (mark0->ccc == mark1->ccc) return false;
      dec_diff[ind2++] = dec_codes[ind1++];
    }
  }
  if (ind0 >= MY_UCA_MAX_CONTRACTION || !origin_dec[ind0]) {
    while (ind1 < MY_UCA_MAX_CONTRACTION) {
      dec_diff[ind2++] = dec_codes[ind1++];
    }
    return true;
  }
  return false;
}

/**
  Add new rules recersively if one rule's characters are in decomposition
  list.
  @param          rules       The rule list
  @param          r           The rule to check
  @param          decomp_rec  The decomposition of the character in rule.
  @param          comp_added  Bitset which marks whether the comp
                              character has been added to rule list.
  @return 1       Error adding new rules
          0       Add rules successfully
*/
static int my_coll_add_inherit_rules(
    MY_COLL_RULES *rules, MY_COLL_RULE *r, const Unidata_decomp *decomp_rec,
    std::bitset<array_elements(uni_dec)> *comp_added) {
  for (uint dec_ind = 0; dec_ind < array_elements(uni_dec); dec_ind++) {
    /*
      For normal character which can be decomposed, it is always decomposed to
      be another character and one combining mark.

      Currently we only support the weight inheritance of character that can be
      canonical-decomposed to another character and a list of combining marks.
      So skip the compatibility decomposition.

      Sample from UnicodeData.txt:
      Canonical decomposition: U+00DC : U+0055 U+0308
      Compatibility decompsition: U+FF59 : <wide> U+0079
    */
    if (!my_compchar_is_normal_char(dec_ind) || comp_added->test(dec_ind) ||
        (decomp_rec != nullptr &&
         uni_dec[dec_ind].decomp_tag != decomp_rec->decomp_tag))
      continue;
    /*
      In DUCET, all accented character's weight is defined as base
      character's weight followed by accent mark's weight. For example:
      00DC = 0055 + 0308
      0055  ; [.1E30.0020.0008]                  # LATIN CAPITAL LETTER U
      0308  ; [.0000.002B.0002]                  # COMBINING DIAERESIS
      00DC  ; [.1E30.0020.0008][.0000.002B.0002] # LATIN CAPITAL LETTER U
                                                   WITH DIAERESIS
      So the composition character's rule should be same as origin rule
      except of the change of curr value.
    */
    my_wc_t dec_diff[MY_UCA_MAX_CONTRACTION]{r->curr[0], 0};
    my_wc_t orig_dec[MY_UCA_MAX_CONTRACTION]{0};
    if (decomp_rec == nullptr) {
      /*
        If there is no decomposition record found in Unidata_decomp, it means
        its decomposition form is itself.
      */
      orig_dec[0] = r->curr[0];
    } else {
      memcpy(orig_dec, decomp_rec->dec_codes, sizeof(orig_dec));
    }
    if (my_is_inheritance_of_origin(orig_dec, uni_dec[dec_ind].dec_codes,
                                    dec_diff) &&
        !my_comp_in_rulelist(rules, uni_dec[dec_ind].charcode)) {
      MY_COLL_RULE newrule{{0}, {uni_dec[dec_ind].charcode, 0}, {0}, 0, false};
      memcpy(newrule.base, dec_diff, sizeof(newrule.base));
      if (my_coll_rules_add(rules, &newrule)) return 1;
      comp_added->set(dec_ind);
    }
  }
  return 0;
}

static bool combining_mark_in_rulelist(const my_wc_t *dec_codes,
                                       const MY_COLL_RULE *r_start,
                                       const MY_COLL_RULE *r_end) {
  for (int i = 1; i < MY_UCA_MAX_CONTRACTION; ++i) {
    if (!*(dec_codes + i)) return false;
    for (const MY_COLL_RULE *r = r_start; r < r_end; ++r) {
      if (r->curr[0] == *(dec_codes + i)) {
        return true;
      }
    }
  }
  return false;
}

static int add_normalization_rules(const CHARSET_INFO *cs,
                                   MY_COLL_RULES *rules) {
  if (!cs->coll_param || !cs->coll_param->norm_enabled) return 0;
  const int orig_rule_num = rules->nrules;
  for (Unidata_decomp *decomp = std::begin(uni_dec); decomp < std::end(uni_dec);
       ++decomp) {
    if (!my_compchar_is_normal_char(decomp) ||
        my_comp_in_rulelist(rules, decomp->charcode) ||
        !combining_mark_in_rulelist(decomp->dec_codes, rules->rule,
                                    rules->rule + orig_rule_num))
      continue;
    MY_COLL_RULE newrule{{0}, {decomp->charcode, 0}, {0}, 0, false};
    memcpy(newrule.base, decomp->dec_codes, sizeof(newrule.base));
    if (my_coll_rules_add(rules, &newrule)) return 1;
  }
  return 0;
}

/**
  For every rule in rule list, check and add new rules if it is in
  decomposition list.
  @param  cs    Character set info
  @param  rules The rule list
  @return 1     Error happens when adding new rule
          0     Add rules successfully
*/
static int my_coll_check_rule_and_inherit(const CHARSET_INFO *cs,
                                          MY_COLL_RULES *rules) {
  if (rules->uca->version != UCA_V900) return 0;

  /*
    Character can combine with marks to be a new character. For example,
    A + [mark b] = A1, A1 + [mark c] = A2. We think the weight of A1 and
    A2 should shift with A if A is in rule list and its weight shifts,
    unless A1 / A2 is already in rule list.
  */
  std::bitset<array_elements(uni_dec)> comp_added;
  int orig_rule_num = rules->nrules;
  for (int i = 0; i < orig_rule_num; ++i) {
    MY_COLL_RULE r = *(rules->rule + i);
    /*
      Do not add inheritance rule for contraction.
      But for the Chinese collation, the weight shift rule of Chinese collation
      is a bit different from all the languages we added so far. For example, it
      has a rule "&e << ... << e\\u0302\\u0300". So far, if a language's rule
      involves 'e\\u0302\\u0300', it will use the combining form character,
      U+1EC1, and it is not a contraction. If we don't handle this for Chinese
      collation, it will skip some further rule inheriting.
     */
    if (cs->coll_param != &zh_coll_param && r.curr[1]) continue;
    Unidata_decomp *decomp_rec = get_decomposition(r.curr[0]);
    if (my_coll_add_inherit_rules(rules, &r, decomp_rec, &comp_added)) return 1;
  }
  return 0;
}

/**
  Helper function to store weight boundary values.
  @param[out] wt_rec     Weight boundary for each character group and gap
                         between groups
  @param      rec_ind    The position from where to store weight boundary
  @param      old_begin  Beginning weight of character group before reorder
  @param      old_end    End weight of character group before reorder
  @param      new_begin  Beginning weight of character group after reorder
  @param      new_end    End weight of character group after reorder
*/
static inline void my_set_weight_rec(
    Reorder_wt_rec (&wt_rec)[2 * UCA_MAX_CHAR_GRP], int rec_ind,
    uint16 old_begin, uint16 old_end, uint16 new_begin, uint16 new_end) {
  wt_rec[rec_ind] = {{old_begin, old_end}, {new_begin, new_end}};
}

/**
  Calculate the reorder parameters for the character groups.
  @param      cs         Character set info
  @param[out] rec_ind    The position from where to store weight boundary
*/
static void my_calc_char_grp_param(const CHARSET_INFO *cs, int &rec_ind) {
  int weight_start = START_WEIGHT_TO_REORDER;
  int grp_ind = 0;
  Reorder_param *param = cs->coll_param->reorder_param;
  for (; grp_ind < UCA_MAX_CHAR_GRP; ++grp_ind) {
    if (param->reorder_grp[grp_ind] == CHARGRP_NONE) break;
    for (Char_grp_info *info = std::begin(char_grp_infos);
         info < std::end(char_grp_infos); ++info) {
      if (param->reorder_grp[grp_ind] != info->group) continue;
      my_set_weight_rec(
          param->wt_rec, grp_ind, info->grp_wt_bdy.begin, info->grp_wt_bdy.end,
          weight_start,
          weight_start + info->grp_wt_bdy.end - info->grp_wt_bdy.begin);
      weight_start = param->wt_rec[grp_ind].new_wt_bdy.end + 1;
      break;
    }
  }
  rec_ind = grp_ind;
}

/**
  Calculate the reorder parameters for the gap between character groups.
  @param      cs         Character set info
  @param      rec_ind    The position from where to store weight boundary
*/
static void my_calc_char_grp_gap_param(CHARSET_INFO *cs, int &rec_ind) {
  Reorder_param *param = cs->coll_param->reorder_param;
  uint16 weight_start = param->wt_rec[rec_ind - 1].new_wt_bdy.end + 1;
  Char_grp_info *last_grp = nullptr;
  for (Char_grp_info *info = std::begin(char_grp_infos);
       info < std::end(char_grp_infos); ++info) {
    for (int ind = 0; ind < UCA_MAX_CHAR_GRP; ++ind) {
      if (param->reorder_grp[ind] == CHARGRP_NONE) break;
      if (param->reorder_grp[ind] != info->group) continue;
      if (param->max_weight < info->grp_wt_bdy.end)
        param->max_weight = info->grp_wt_bdy.end;
      /*
        There might be some character groups before the first character
        group in our list.
      */
      if (!last_grp && info->grp_wt_bdy.begin > START_WEIGHT_TO_REORDER) {
        my_set_weight_rec(param->wt_rec, rec_ind, START_WEIGHT_TO_REORDER,
                          info->grp_wt_bdy.begin - 1, weight_start,
                          weight_start + (info->grp_wt_bdy.begin - 1) -
                              START_WEIGHT_TO_REORDER);
        weight_start = param->wt_rec[rec_ind].new_wt_bdy.end + 1;
        rec_ind++;
      }
      /* Gap between 2 character groups in out list. */
      if (last_grp && last_grp->grp_wt_bdy.end < (info->grp_wt_bdy.begin - 1)) {
        my_set_weight_rec(param->wt_rec, rec_ind, last_grp->grp_wt_bdy.end + 1,
                          info->grp_wt_bdy.begin - 1, weight_start,
                          weight_start + (info->grp_wt_bdy.begin - 1) -
                              (last_grp->grp_wt_bdy.end + 1));
        weight_start = param->wt_rec[rec_ind].new_wt_bdy.end + 1;
        rec_ind++;
      }
      last_grp = info;
      break;
    }
  }
  param->wt_rec_num = rec_ind;
}

/**
  Prepare reorder parameters.
  @param  cs     Character set info
*/
static int my_prepare_reorder(CHARSET_INFO *cs) {
  /*
    Chinese collation's reordering is done in next_implicit() and
    modify_all_zh_pages(). See the comment on zh_reorder_param and
    change_zh_implicit().
   */
  if (!cs->coll_param->reorder_param || cs->coll_param == &zh_coll_param)
    return 0;
  /*
    For each group of character, for example, latin characters,
    their weights are in a seperate range. The default sequence
    of these groups is: Latin, Greek, Coptic, Cyrillic, and so
    on. Some languages want to change the default sequence. For
    example, Croatian wants to put Cyrillic to just behind Latin.
    We need to reorder the character groups and change their
    weight accordingly. Here we calculate the parameters needed
    for weight change. And the change will happen when weight
    returns from strnxfrm.
  */
  int rec_ind = 0;
  my_calc_char_grp_param(cs, rec_ind);
  my_calc_char_grp_gap_param(cs, rec_ind);
  return rec_ind;
}

static void adjust_japanese_weight(CHARSET_INFO *cs, int rec_ind) {
  /*
    Per CLDR 30, Japanese collations need to reorder characters as
    [Latin, Kana, Han, others]. So for the original character group list:
    [Latin, CharA, Kana, CharB, Han, Others], it should be reordered as
    [Latin, Kana, Han, CharA, CharB, Others]. But my_prepare_reorder()
    reorders original group to be [Latin, Kana, CharA, CharB, Han, Others].
    This is because Han characters are different from others in that Han
    characters' weight is implicit and has two primary weights for each
    character. Other characters have only one primary weight for each (base)
    character. Han characters always sort bigger.

    CLDR defines the collating order for 6355 Japanese Han characters. All
    of them are in [U+4E00, U+9FFF]; we give them tailored primary weights
    in ja_han_pages. The tailored primary weights are just after Kana,
    because these characters are very common. These Han characters' weight
    pages will be added to collation's UCA data in copy_ja_han_pages().
    For the other Han characters, we don't change their implicit weights,
    which is [FB80 - FB85, 0020, 0002][XXXX, 0000, 0000].

    To make sure CharA and CharB's weight is greater than all Han characters,
    we give them weight as [FB86, 0000, 0000][origin weights]. This will be
    done in apply_reorder_param().

    Because the values stored in last wt_rec element is calculated for moving
    CharA to be after Kana, but we want them to be after all Han character,
    we reset the weight boundary here, and will change all these characters'
    weight in apply_reorder_param().
  */
  Reorder_param *param = cs->coll_param->reorder_param;
  param->wt_rec[rec_ind - 1].new_wt_bdy.begin = 0;
  param->wt_rec[rec_ind - 1].new_wt_bdy.end = 0;
  param->wt_rec[rec_ind].old_wt_bdy.begin = param->wt_rec[1].old_wt_bdy.end + 1;
  param->wt_rec[rec_ind].old_wt_bdy.end = 0x54A3;
  param->wt_rec[rec_ind].new_wt_bdy.begin = 0;
  param->wt_rec[rec_ind].new_wt_bdy.end = 0;
  param->wt_rec_num++;
  param->max_weight = 0x54A3;
}

/**
  Prepare parametric tailoring, like reorder, etc.
  @param  cs     Character set info
  @param  rules  Collation rule list to add to.
  @return false  Collation parameters applied sucessfully.
          true   Error happened.
*/
static bool my_prepare_coll_param(CHARSET_INFO *cs, MY_COLL_RULES *rules) {
  if (rules->uca->version != UCA_V900 || !cs->coll_param) return false;

  int rec_ind = my_prepare_reorder(cs);
  if (add_normalization_rules(cs, rules)) return true;

  if (cs->coll_param == &ja_coll_param) adjust_japanese_weight(cs, rec_ind);
  /* Might add other parametric tailoring rules later. */
  return false;
}

/*
  This function copies an UCS2 collation from
  the default Unicode Collation Algorithm (UCA)
  weights applying tailorings, i.e. a set of
  alternative weights for some characters.

  The default UCA weights are stored in uca_weight/uca_length.
  They consist of 256 pages, 256 character each.

  If a page is not overwritten by tailoring rules,
  it is copies as is from UCA as is.

  If a page contains some overwritten characters, it is
  allocated. Untouched characters are copied from the
  default weights.
*/

static bool create_tailoring(CHARSET_INFO *cs, MY_CHARSET_LOADER *loader) {
  if (!cs->tailoring)
    return false; /* Ok to add a collation without tailoring */

  MY_COLL_RULES rules;
  MY_UCA_INFO new_uca, *src_uca = nullptr;
  int rc = 0;
  MY_UCA_INFO *src, *dst;
  size_t npages;
  bool lengths_are_temporary;

  loader->errcode = 0;
  *loader->errarg = '\0';

  memset(&rules, 0, sizeof(rules));
  rules.loader = loader;
  rules.uca = cs->uca ? cs->uca : &my_uca_v400; /* For logical positions, etc */
  memset(&new_uca, 0, sizeof(new_uca));

  /* Parse ICU Collation Customization expression */
  if ((rc =
           my_coll_rule_parse(&rules, cs->tailoring,
                              cs->tailoring + strlen(cs->tailoring), cs->name)))
    goto ex;

  if ((rc = my_coll_check_rule_and_inherit(cs, &rules))) goto ex;

  if ((rc = my_prepare_coll_param(cs, &rules))) goto ex;

  if (rules.uca->version == UCA_V520) /* Unicode-5.2.0 requested */
  {
    src_uca = &my_uca_v520;
    cs->caseinfo = &my_unicase_unicode520;
  } else if (rules.uca->version == UCA_V400) /* Unicode-4.0.0 requested */
  {
    src_uca = &my_uca_v400;
    if (!cs->caseinfo) cs->caseinfo = &my_unicase_default;
  } else /* No Unicode version specified */
  {
    src_uca = cs->uca ? cs->uca : &my_uca_v400;
    if (!cs->caseinfo) cs->caseinfo = &my_unicase_default;
  }

  /*
    For UCA 9.0.0, we don't have a length page, but we still create one
    temporarily so that we can keep track of much memory we need to
    allocate for weights.
  */
  src = src_uca;
  dst = &new_uca;

  dst->extra_ce_pri_base = cs->uca->extra_ce_pri_base;
  dst->extra_ce_sec_base = cs->uca->extra_ce_sec_base;
  dst->extra_ce_ter_base = cs->uca->extra_ce_ter_base;
  if (cs->coll_param && cs->coll_param == &zh_coll_param) {
    dst->extra_ce_pri_base = ZH_EXTRA_CE_PRI;
  }

  npages = (src->maxchar + 1) / 256;
  if (rules.uca->version == UCA_V900) {
    if (!(src->lengths = (uchar *)(loader->mem_malloc)(npages))) goto ex;
    synthesize_lengths_900(src->lengths, src->weights, npages);
  }

  lengths_are_temporary = (rules.uca->version == UCA_V900);
  if ((rc = init_weight_level(cs, loader, &rules, 0, dst, src,
                              lengths_are_temporary)))
    goto ex;

  if (lengths_are_temporary) {
    (loader->mem_free)(src->lengths);
    (loader->mem_free)(dst->lengths);
    src->lengths = nullptr;
    dst->lengths = nullptr;
  }

  new_uca.version = src_uca->version;
  if (!(cs->uca = (MY_UCA_INFO *)(loader->once_alloc)(sizeof(MY_UCA_INFO)))) {
    rc = 1;
    goto ex;
  }
  memset(cs->uca, 0, sizeof(MY_UCA_INFO));
  cs->uca[0] = new_uca;

ex:
  (loader->mem_free)(rules.rule);
  if (rc != 0 && loader->errcode) {
    if (new_uca.contraction_nodes) delete new_uca.contraction_nodes;
    loader->reporter(ERROR_LEVEL, loader->errcode, loader->errarg);
  }
  return rc;
}

static void my_coll_uninit_uca(CHARSET_INFO *cs) {
  if (cs->uca && cs->uca->contraction_nodes) {
    delete cs->uca->contraction_nodes;
    cs->uca->contraction_nodes = nullptr;
    cs->state &= ~MY_CS_READY;
  }
}
/*
  Universal CHARSET_INFO compatible wrappers
  for the above internal functions.
  Should work for any character set.
*/

extern "C" {
static bool my_coll_init_uca(CHARSET_INFO *cs, MY_CHARSET_LOADER *loader) {
  cs->pad_char = ' ';
  cs->ctype = my_charset_utf8_unicode_ci.ctype;
  if (!cs->caseinfo) cs->caseinfo = &my_unicase_default;
  if (!cs->uca) cs->uca = &my_uca_v400;
  return create_tailoring(cs, loader);
}

static int my_strnncoll_any_uca(const CHARSET_INFO *cs, const uchar *s,
                                size_t slen, const uchar *t, size_t tlen,
                                bool t_is_prefix) {
  if (cs->cset->mb_wc == my_mb_wc_utf8mb4_thunk) {
    return my_strnncoll_uca<uca_scanner_any<Mb_wc_utf8mb4>, 1>(
        cs, Mb_wc_utf8mb4(), s, slen, t, tlen, t_is_prefix);
  }

  Mb_wc_through_function_pointer mb_wc(cs);
  return my_strnncoll_uca<uca_scanner_any<decltype(mb_wc)>, 1>(
      cs, mb_wc, s, slen, t, tlen, t_is_prefix);
}

static int my_strnncollsp_any_uca(const CHARSET_INFO *cs, const uchar *s,
                                  size_t slen, const uchar *t, size_t tlen) {
  if (cs->cset->mb_wc == my_mb_wc_utf8mb4_thunk) {
    return my_strnncollsp_uca(cs, Mb_wc_utf8mb4(), s, slen, t, tlen);
  }

  Mb_wc_through_function_pointer mb_wc(cs);
  return my_strnncollsp_uca(cs, mb_wc, s, slen, t, tlen);
}

static void my_hash_sort_any_uca(const CHARSET_INFO *cs, const uchar *s,
                                 size_t slen, uint64 *n1, uint64 *n2) {
  if (cs->cset->mb_wc == my_mb_wc_utf8mb4_thunk) {
    my_hash_sort_uca(cs, Mb_wc_utf8mb4(), s, slen, n1, n2);
  } else {
    Mb_wc_through_function_pointer mb_wc(cs);
    my_hash_sort_uca(cs, mb_wc, s, slen, n1, n2);
  }
}

static size_t my_strnxfrm_any_uca(const CHARSET_INFO *cs, uchar *dst,
                                  size_t dstlen, uint num_codepoints,
                                  const uchar *src, size_t srclen, uint flags) {
  if (cs->cset->mb_wc == my_mb_wc_utf8mb4_thunk) {
    return my_strnxfrm_uca(cs, Mb_wc_utf8mb4(), dst, dstlen, num_codepoints,
                           src, srclen, flags);
  }

  Mb_wc_through_function_pointer mb_wc(cs);
  return my_strnxfrm_uca(cs, mb_wc, dst, dstlen, num_codepoints, src, srclen,
                         flags);
}

static int my_strnncoll_uca_900(const CHARSET_INFO *cs, const uchar *s,
                                size_t slen, const uchar *t, size_t tlen,
                                bool t_is_prefix) {
  if (cs->cset->mb_wc == my_mb_wc_utf8mb4_thunk) {
    switch (cs->levels_for_compare) {
      case 1:
        return my_strnncoll_uca<uca_scanner_900<Mb_wc_utf8mb4, 1>, 1>(
            cs, Mb_wc_utf8mb4(), s, slen, t, tlen, t_is_prefix);
      case 2:
        return my_strnncoll_uca<uca_scanner_900<Mb_wc_utf8mb4, 2>, 2>(
            cs, Mb_wc_utf8mb4(), s, slen, t, tlen, t_is_prefix);
      default:
        DBUG_ASSERT(false);
      case 3:
        return my_strnncoll_uca<uca_scanner_900<Mb_wc_utf8mb4, 3>, 3>(
            cs, Mb_wc_utf8mb4(), s, slen, t, tlen, t_is_prefix);
      case 4:
        return my_strnncoll_uca<uca_scanner_900<Mb_wc_utf8mb4, 4>, 4>(
            cs, Mb_wc_utf8mb4(), s, slen, t, tlen, t_is_prefix);
    }
  }

  Mb_wc_through_function_pointer mb_wc(cs);
  switch (cs->levels_for_compare) {
    case 1:
      return my_strnncoll_uca<uca_scanner_900<decltype(mb_wc), 1>, 1>(
          cs, mb_wc, s, slen, t, tlen, t_is_prefix);
    case 2:
      return my_strnncoll_uca<uca_scanner_900<decltype(mb_wc), 2>, 2>(
          cs, mb_wc, s, slen, t, tlen, t_is_prefix);
    default:
      DBUG_ASSERT(false);
    case 3:
      return my_strnncoll_uca<uca_scanner_900<decltype(mb_wc), 3>, 3>(
          cs, mb_wc, s, slen, t, tlen, t_is_prefix);
    case 4:
      return my_strnncoll_uca<uca_scanner_900<decltype(mb_wc), 4>, 4>(
          cs, mb_wc, s, slen, t, tlen, t_is_prefix);
  }
}

static int my_strnncollsp_uca_900(const CHARSET_INFO *cs, const uchar *s,
                                  size_t slen, const uchar *t, size_t tlen) {
  // We are a NO PAD collation, so this is identical to strnncoll.
  return my_strnncoll_uca_900(cs, s, slen, t, tlen, false);
}

}  // extern "C"

template <class Mb_wc, int LEVELS_FOR_COMPARE>
static void my_hash_sort_uca_900_tmpl(const CHARSET_INFO *cs, const Mb_wc mb_wc,
                                      const uchar *s, size_t slen, uint64 *n1) {
  uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE> scanner(mb_wc, cs, s, slen);

  /*
    A variation of the FNV-1a hash. The differences between this and
    standard FNV-1a as described in literature are:

     - We work naturally on 16-bit weights, so we XOR in the entire weight
       instead of hashing byte-by-byte. (This is effectively a speed/quality
       tradeoff, as it will reduce avalanche.)
     - We use the n1 seed by XOR-ing it onto the offset basis; FNV-1a as
       typically described does not use a seed. This should be safe, since
       there's nothing magical about the offset basis; it's just the FNV-1a
       hash of some human-readable text.

    This is nowhere near a perfect hash function; it has suboptimal avalanche
    characteristics, and it not multicollision resistant. In particular,
    it fails many SMHasher tests, mostly for bias (collision tests are fine).
    However, it is of much better quality than the home-grown hash used
    for other collations (which fails _all_ SMHasher tests), while being
    much faster.

    We ignore the n2 seed entirely, since we don't need it. The caller is
    responsible for doing hash folding at the end; we can't do that.

    See http://isthe.com/chongo/tech/comp/fnv/#FNV-param for constants.
  */

  uint64 h = *n1;
  h ^= 14695981039346656037ULL;

  scanner.for_each_weight(
      [&](int s_res, bool) -> bool {
        h ^= s_res;
        h *= 1099511628211ULL;
        return true;
      },
      [](int) { return true; });

  *n1 = h;
}

extern "C" {

static void my_hash_sort_uca_900(const CHARSET_INFO *cs, const uchar *s,
                                 size_t slen, uint64 *n1, uint64 *) {
  if (cs->cset->mb_wc == my_mb_wc_utf8mb4_thunk) {
    switch (cs->levels_for_compare) {
      case 1:
        return my_hash_sort_uca_900_tmpl<Mb_wc_utf8mb4, 1>(cs, Mb_wc_utf8mb4(),
                                                           s, slen, n1);
      case 2:
        return my_hash_sort_uca_900_tmpl<Mb_wc_utf8mb4, 2>(cs, Mb_wc_utf8mb4(),
                                                           s, slen, n1);
      default:
        DBUG_ASSERT(false);
      case 3:
        return my_hash_sort_uca_900_tmpl<Mb_wc_utf8mb4, 3>(cs, Mb_wc_utf8mb4(),
                                                           s, slen, n1);
      case 4:
        return my_hash_sort_uca_900_tmpl<Mb_wc_utf8mb4, 4>(cs, Mb_wc_utf8mb4(),
                                                           s, slen, n1);
    }
  }

  Mb_wc_through_function_pointer mb_wc(cs);
  switch (cs->levels_for_compare) {
    case 1:
      return my_hash_sort_uca_900_tmpl<decltype(mb_wc), 1>(cs, mb_wc, s, slen,
                                                           n1);
    case 2:
      return my_hash_sort_uca_900_tmpl<decltype(mb_wc), 2>(cs, mb_wc, s, slen,
                                                           n1);
    default:
      DBUG_ASSERT(false);
    case 3:
      return my_hash_sort_uca_900_tmpl<decltype(mb_wc), 3>(cs, mb_wc, s, slen,
                                                           n1);
    case 4:
      return my_hash_sort_uca_900_tmpl<decltype(mb_wc), 4>(cs, mb_wc, s, slen,
                                                           n1);
  }
}

}  // extern "C"

/*
  Check if a constant can be propagated

  Currently we don't check the constant itself, and decide not to propagate
  a constant just if the collation itself allows expansions or contractions.
*/
bool my_propagate_uca_900(const CHARSET_INFO *cs,
                          const uchar *str MY_ATTRIBUTE((unused)),
                          size_t length MY_ATTRIBUTE((unused))) {
  return !my_uca_have_contractions(cs->uca);
}

template <class Mb_wc, int LEVELS_FOR_COMPARE>
static size_t my_strnxfrm_uca_900_tmpl(const CHARSET_INFO *cs,
                                       const Mb_wc mb_wc, uchar *dst,
                                       size_t dstlen, const uchar *src,
                                       size_t srclen, uint flags) {
  uchar *d0 = dst;
  uchar *dst_end = dst + dstlen;
  uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE> scanner(mb_wc, cs, src, srclen);

  DBUG_ASSERT((dstlen % 2) == 0);
  if ((dstlen % 2) == 1) {
    // Emergency workaround for optimized mode.
    --dst_end;
  }

  if (dst != dst_end) {
    scanner.for_each_weight(
        [&dst, dst_end](
            int s_res, bool is_level_separator MY_ATTRIBUTE((unused))) -> bool {
          DBUG_ASSERT(is_level_separator == (s_res == 0));
          if (LEVELS_FOR_COMPARE == 1) DBUG_ASSERT(!is_level_separator);

          dst = store16be(dst, s_res);
          return (dst < dst_end);
        },
        [&dst, dst_end](int num_weights) {
          return (dst < dst_end - num_weights * 2);
        });
  }

  if (flags & MY_STRXFRM_PAD_TO_MAXLEN) {
    memset(dst, 0, dst_end - dst);
    dst = dst_end;
  }

  return dst - d0;
}

extern "C" {

static size_t my_strnxfrm_uca_900(const CHARSET_INFO *cs, uchar *dst,
                                  size_t dstlen,
                                  uint num_codepoints MY_ATTRIBUTE((unused)),
                                  const uchar *src, size_t srclen, uint flags) {
  if (cs->cset->mb_wc == my_mb_wc_utf8mb4_thunk) {
    switch (cs->levels_for_compare) {
      case 1:
        return my_strnxfrm_uca_900_tmpl<Mb_wc_utf8mb4, 1>(
            cs, Mb_wc_utf8mb4(), dst, dstlen, src, srclen, flags);
      case 2:
        return my_strnxfrm_uca_900_tmpl<Mb_wc_utf8mb4, 2>(
            cs, Mb_wc_utf8mb4(), dst, dstlen, src, srclen, flags);
      default:
        DBUG_ASSERT(false);
      case 3:
        return my_strnxfrm_uca_900_tmpl<Mb_wc_utf8mb4, 3>(
            cs, Mb_wc_utf8mb4(), dst, dstlen, src, srclen, flags);
      case 4:
        return my_strnxfrm_uca_900_tmpl<Mb_wc_utf8mb4, 4>(
            cs, Mb_wc_utf8mb4(), dst, dstlen, src, srclen, flags);
    }
  } else {
    Mb_wc_through_function_pointer mb_wc(cs);
    switch (cs->levels_for_compare) {
      case 1:
        return my_strnxfrm_uca_900_tmpl<decltype(mb_wc), 1>(
            cs, mb_wc, dst, dstlen, src, srclen, flags);
      case 2:
        return my_strnxfrm_uca_900_tmpl<decltype(mb_wc), 2>(
            cs, mb_wc, dst, dstlen, src, srclen, flags);
      default:
        DBUG_ASSERT(false);
      case 3:
        return my_strnxfrm_uca_900_tmpl<decltype(mb_wc), 3>(
            cs, mb_wc, dst, dstlen, src, srclen, flags);
      case 4:
        return my_strnxfrm_uca_900_tmpl<decltype(mb_wc), 4>(
            cs, mb_wc, dst, dstlen, src, srclen, flags);
    }
  }
}

static size_t my_strnxfrmlen_uca_900(const CHARSET_INFO *cs, size_t len) {
  /*
    The character with the most weights is U+FDFA ARABIC LIGATURE SALLALLAHOU
    ALAYHE WASALLAM, which we truncate to eight weights. This is the most we
    can get in regular DUCET.

    In addition, collations with reorderings can add an extra weight per weight,
    which currently only happens on the primary level. We simulate this by
    simply adding an extra level.

    One could conceivably have tailorings yielding expansions having more than
    this, but we don't currently, and mostly, tailorings are about contractions
    and adding single weights anyway.

    We also need to add room for one level separator between each level.
  */
  // We really ought to have len % 4 == 0, but not all calling code conforms.
  const size_t num_codepoints = (len + 3) / 4;
  const size_t max_num_weights_per_level = num_codepoints * 8;
  size_t max_num_weights = max_num_weights_per_level * cs->levels_for_compare;
  if (cs->coll_param && cs->coll_param->reorder_param) {
    max_num_weights += max_num_weights_per_level;
  }
  return (max_num_weights + (cs->levels_for_compare - 1)) * sizeof(uint16_t);
}

}  // extern "C"

/*
  UCS2 optimized CHARSET_INFO compatible wrappers.
*/
extern "C" {
static int my_strnncoll_ucs2_uca(const CHARSET_INFO *cs, const uchar *s,
                                 size_t slen, const uchar *t, size_t tlen,
                                 bool t_is_prefix) {
  Mb_wc_through_function_pointer mb_wc(cs);
  return my_strnncoll_uca<uca_scanner_any<decltype(mb_wc)>, 1>(
      cs, mb_wc, s, slen, t, tlen, t_is_prefix);
}

static int my_strnncollsp_ucs2_uca(const CHARSET_INFO *cs, const uchar *s,
                                   size_t slen, const uchar *t, size_t tlen) {
  Mb_wc_through_function_pointer mb_wc(cs);
  return my_strnncollsp_uca(cs, mb_wc, s, slen, t, tlen);
}

static void my_hash_sort_ucs2_uca(const CHARSET_INFO *cs, const uchar *s,
                                  size_t slen, uint64 *n1, uint64 *n2) {
  Mb_wc_through_function_pointer mb_wc(cs);
  my_hash_sort_uca(cs, mb_wc, s, slen, n1, n2);
}

static size_t my_strnxfrm_ucs2_uca(const CHARSET_INFO *cs, uchar *dst,
                                   size_t dstlen, uint num_codepoints,
                                   const uchar *src, size_t srclen,
                                   uint flags) {
  Mb_wc_through_function_pointer mb_wc(cs);
  return my_strnxfrm_uca(cs, mb_wc, dst, dstlen, num_codepoints, src, srclen,
                         flags);
}
}  // extern "C"

MY_COLLATION_HANDLER my_collation_ucs2_uca_handler = {
    my_coll_init_uca, /* init */
    my_coll_uninit_uca,
    my_strnncoll_ucs2_uca,
    my_strnncollsp_ucs2_uca,
    my_strnxfrm_ucs2_uca,
    my_strnxfrmlen_simple,
    my_like_range_generic,
    my_wildcmp_uca,
    nullptr,
    my_instr_mb,
    my_hash_sort_ucs2_uca,
    my_propagate_complex};

CHARSET_INFO my_charset_ucs2_unicode_ci = {
    128,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_unicode_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    "",                  /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_icelandic_uca_ci = {
    129,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_icelandic_ci", /* name         */
    "UCS-2 Unicode",     /* comment      */
    icelandic,           /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_latvian_uca_ci = {
    130,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_latvian_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    latvian,             /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_romanian_uca_ci = {
    131,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_romanian_ci",  /* name         */
    "UCS-2 Unicode",     /* comment      */
    romanian,            /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_slovenian_uca_ci = {
    132,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_slovenian_ci", /* name         */
    "UCS-2 Unicode",     /* comment      */
    slovenian,           /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_polish_uca_ci = {
    133,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_polish_ci",    /* name         */
    "UCS-2 Unicode",     /* comment      */
    polish,              /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_estonian_uca_ci = {
    134,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_estonian_ci",  /* name         */
    "UCS-2 Unicode",     /* comment      */
    estonian,            /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_spanish_uca_ci = {
    135,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_spanish_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    spanish,             /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_swedish_uca_ci = {
    136,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_swedish_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    swedish,             /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_turkish_uca_ci = {
    137,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_turkish_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    turkish,             /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_turkish, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_czech_uca_ci = {
    138,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_czech_ci",     /* name         */
    "UCS-2 Unicode",     /* comment      */
    czech,               /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_danish_uca_ci = {
    139,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_danish_ci",    /* name         */
    "UCS-2 Unicode",     /* comment      */
    danish,              /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_lithuanian_uca_ci = {
    140,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",               /* cs name    */
    "ucs2_lithuanian_ci", /* name         */
    "UCS-2 Unicode",      /* comment      */
    lithuanian,           /* tailoring    */
    nullptr,              /* coll_param   */
    nullptr,              /* ctype        */
    nullptr,              /* to_lower     */
    nullptr,              /* to_upper     */
    nullptr,              /* sort_order   */
    nullptr,              /* uca          */
    nullptr,              /* tab_to_uni   */
    nullptr,              /* tab_from_uni */
    &my_unicase_default,  /* caseinfo     */
    nullptr,              /* state_map    */
    nullptr,              /* ident_map    */
    8,                    /* strxfrm_multiply */
    1,                    /* caseup_multiply  */
    1,                    /* casedn_multiply  */
    2,                    /* mbminlen     */
    2,                    /* mbmaxlen     */
    1,                    /* mbmaxlenlen  */
    9,                    /* min_sort_char */
    0xFFFF,               /* max_sort_char */
    ' ',                  /* pad char      */
    false,                /* escape_with_backslash_is_dangerous */
    1,                    /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_slovak_uca_ci = {
    141,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_slovak_ci",    /* name         */
    "UCS-2 Unicode",     /* comment      */
    slovak,              /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_spanish2_uca_ci = {
    142,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_spanish2_ci",  /* name         */
    "UCS-2 Unicode",     /* comment      */
    spanish2,            /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_roman_uca_ci = {
    143,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_roman_ci",     /* name         */
    "UCS-2 Unicode",     /* comment      */
    roman,               /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_persian_uca_ci = {
    144,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_persian_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    persian,             /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_esperanto_uca_ci = {
    145,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_esperanto_ci", /* name         */
    "UCS-2 Unicode",     /* comment      */
    esperanto,           /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_hungarian_uca_ci = {
    146,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* cs name    */
    "ucs2_hungarian_ci", /* name         */
    "UCS-2 Unicode",     /* comment      */
    hungarian,           /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_sinhala_uca_ci = {
    147,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* csname    */
    "ucs2_sinhala_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    sinhala,             /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_german2_uca_ci = {
    148,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* csname    */
    "ucs2_german2_ci",   /* name         */
    "UCS-2 Unicode",     /* comment      */
    german2,             /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_croatian_uca_ci = {
    149,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",              /* csname    */
    "ucs2_croatian_ci",  /* name         */
    "UCS-2 Unicode",     /* comment      */
    croatian,            /* tailoring    */
    nullptr,             /* coll_param   */
    nullptr,             /* ctype        */
    nullptr,             /* to_lower     */
    nullptr,             /* to_upper     */
    nullptr,             /* sort_order   */
    nullptr,             /* uca          */
    nullptr,             /* tab_to_uni   */
    nullptr,             /* tab_from_uni */
    &my_unicase_default, /* caseinfo     */
    nullptr,             /* state_map    */
    nullptr,             /* ident_map    */
    8,                   /* strxfrm_multiply */
    1,                   /* caseup_multiply  */
    1,                   /* casedn_multiply  */
    2,                   /* mbminlen     */
    2,                   /* mbmaxlen     */
    1,                   /* mbmaxlenlen  */
    9,                   /* min_sort_char */
    0xFFFF,              /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    1,                   /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_unicode_520_ci = {
    150,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",                 /* cs name      */
    "ucs2_unicode_520_ci",  /* name       */
    "UCS-2 Unicode",        /* comment      */
    "",                     /* tailoring    */
    nullptr,                /* coll_param   */
    nullptr,                /* ctype        */
    nullptr,                /* to_lower     */
    nullptr,                /* to_upper     */
    nullptr,                /* sort_order   */
    &my_uca_v520,           /* uca          */
    nullptr,                /* tab_to_uni   */
    nullptr,                /* tab_from_uni */
    &my_unicase_unicode520, /* caseinfo  */
    nullptr,                /* state_map    */
    nullptr,                /* ident_map    */
    8,                      /* strxfrm_multiply */
    1,                      /* caseup_multiply  */
    1,                      /* casedn_multiply  */
    2,                      /* mbminlen     */
    2,                      /* mbmaxlen     */
    1,                      /* mbmaxlenlen  */
    9,                      /* min_sort_char */
    0xFFFF,                 /* max_sort_char */
    ' ',                    /* pad char      */
    false,                  /* escape_with_backslash_is_dangerous */
    1,                      /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_ucs2_vietnamese_ci = {
    151,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "ucs2",               /* csname       */
    "ucs2_vietnamese_ci", /* name         */
    "UCS-2 Unicode",      /* comment      */
    vietnamese,           /* tailoring    */
    nullptr,              /* coll_param   */
    nullptr,              /* ctype        */
    nullptr,              /* to_lower     */
    nullptr,              /* to_upper     */
    nullptr,              /* sort_order   */
    nullptr,              /* uca          */
    nullptr,              /* tab_to_uni   */
    nullptr,              /* tab_from_uni */
    &my_unicase_default,  /* caseinfo     */
    nullptr,              /* state_map    */
    nullptr,              /* ident_map    */
    8,                    /* strxfrm_multiply */
    1,                    /* caseup_multiply  */
    1,                    /* casedn_multiply  */
    2,                    /* mbminlen     */
    2,                    /* mbmaxlen     */
    1,                    /* mbmaxlenlen  */
    9,                    /* min_sort_char */
    0xFFFF,               /* max_sort_char */
    ' ',                  /* pad char      */
    false,                /* escape_with_backslash_is_dangerous */
    1,                    /* levels_for_compare */
    &my_charset_ucs2_handler,
    &my_collation_ucs2_uca_handler,
    PAD_SPACE};

MY_COLLATION_HANDLER my_collation_any_uca_handler = {
    my_coll_init_uca, /* init */
    my_coll_uninit_uca,   my_strnncoll_any_uca,  my_strnncollsp_any_uca,
    my_strnxfrm_any_uca,  my_strnxfrmlen_simple, my_like_range_mb,
    my_wildcmp_uca,       my_strcasecmp_uca,     my_instr_mb,
    my_hash_sort_any_uca, my_propagate_complex};

MY_COLLATION_HANDLER my_collation_uca_900_handler = {
    my_coll_init_uca, /* init */
    my_coll_uninit_uca,   my_strnncoll_uca_900,   my_strnncollsp_uca_900,
    my_strnxfrm_uca_900,  my_strnxfrmlen_uca_900, my_like_range_mb,
    my_wildcmp_uca,       my_strcasecmp_uca,      my_instr_mb,
    my_hash_sort_uca_900, my_propagate_uca_900};

/*
  We consider bytes with code more than 127 as a letter.
  This garantees that word boundaries work fine with regular
  expressions. Note, there is no need to mark byte 255  as a
  letter, it is illegal byte in UTF8.
*/
static const uchar ctype_utf8[] = {
    0,  32,  32,  32,  32,  32,  32,  32,  32,  32,  40,  40, 40, 40, 40, 32,
    32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32, 32, 32,
    32, 72,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 16, 16, 16, 16,
    16, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 16, 16, 16, 16, 16,
    16, 16,  129, 129, 129, 129, 129, 129, 1,   1,   1,   1,  1,  1,  1,  1,
    1,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  16, 16, 16, 16,
    16, 16,  130, 130, 130, 130, 130, 130, 2,   2,   2,   2,  2,  2,  2,  2,
    2,  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  16, 16, 16, 16,
    32, 3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    3,  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    3,  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    3,  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    3,  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    3,  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    3,  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    3,  3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,
    0};

extern MY_CHARSET_HANDLER my_charset_utf8_handler;

#define MY_CS_UTF8MB3_UCA_FLAGS \
  (MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE)

CHARSET_INFO my_charset_utf8_unicode_ci = {
    192,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_unicode_ci",       /* name         */
    "UCS-2 Unicode",         /* comment      */
    "",                      /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_icelandic_uca_ci = {
    193,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_icelandic_ci",     /* name         */
    "UTF-8 Unicode",         /* comment      */
    icelandic,               /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_latvian_uca_ci = {
    194,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_latvian_ci",       /* name         */
    "UTF-8 Unicode",         /* comment      */
    latvian,                 /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_romanian_uca_ci = {
    195,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_romanian_ci",      /* name         */
    "UTF-8 Unicode",         /* comment      */
    romanian,                /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_slovenian_uca_ci = {
    196,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_slovenian_ci",     /* name         */
    "UTF-8 Unicode",         /* comment      */
    slovenian,               /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_polish_uca_ci = {
    197,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_polish_ci",        /* name         */
    "UTF-8 Unicode",         /* comment      */
    polish,                  /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_estonian_uca_ci = {
    198,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_estonian_ci",      /* name         */
    "UTF-8 Unicode",         /* comment      */
    estonian,                /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_spanish_uca_ci = {
    199,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_spanish_ci",       /* name         */
    "UTF-8 Unicode",         /* comment      */
    spanish,                 /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_swedish_uca_ci = {
    200,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_swedish_ci",       /* name         */
    "UTF-8 Unicode",         /* comment      */
    swedish,                 /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_turkish_uca_ci = {
    201,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_turkish_ci",       /* name         */
    "UTF-8 Unicode",         /* comment      */
    turkish,                 /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_turkish,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    2,                       /* caseup_multiply  */
    2,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_czech_uca_ci = {
    202,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_czech_ci",         /* name         */
    "UTF-8 Unicode",         /* comment      */
    czech,                   /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_danish_uca_ci = {
    203,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_danish_ci",        /* name         */
    "UTF-8 Unicode",         /* comment      */
    danish,                  /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_lithuanian_uca_ci = {
    204,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_lithuanian_ci",    /* name         */
    "UTF-8 Unicode",         /* comment      */
    lithuanian,              /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_slovak_uca_ci = {
    205,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_slovak_ci",        /* name         */
    "UTF-8 Unicode",         /* comment      */
    slovak,                  /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_spanish2_uca_ci = {
    206,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_spanish2_ci",      /* name         */
    "UTF-8 Unicode",         /* comment      */
    spanish2,                /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_roman_uca_ci = {
    207,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_roman_ci",         /* name         */
    "UTF-8 Unicode",         /* comment      */
    roman,                   /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_persian_uca_ci = {
    208,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_persian_ci",       /* name         */
    "UTF-8 Unicode",         /* comment      */
    persian,                 /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_esperanto_uca_ci = {
    209,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_esperanto_ci",     /* name         */
    "UTF-8 Unicode",         /* comment      */
    esperanto,               /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_hungarian_uca_ci = {
    210,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name    */
    "utf8_hungarian_ci",     /* name         */
    "UTF-8 Unicode",         /* comment      */
    hungarian,               /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_sinhala_uca_ci = {
    211,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS, /* flags    */
    "utf8",                  /* cs name      */
    "utf8_sinhala_ci",       /* name         */
    "UTF-8 Unicode",         /* comment      */
    sinhala,                 /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    3,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_german2_uca_ci = {
    212,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS,  /* flags    */
    MY_UTF8MB3,               /* cs name      */
    MY_UTF8MB3 "_german2_ci", /* name    */
    "UTF-8 Unicode",          /* comment      */
    german2,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen     */
    3,                        /* mbmaxlen     */
    1,                        /* mbmaxlenlen  */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_croatian_uca_ci = {
    213,
    0,
    0,                         /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS,   /* flags    */
    MY_UTF8MB3,                /* cs name      */
    MY_UTF8MB3 "_croatian_ci", /* name    */
    "UTF-8 Unicode",           /* comment      */
    croatian,                  /* tailoring    */
    nullptr,                   /* coll_param   */
    ctype_utf8,                /* ctype        */
    nullptr,                   /* to_lower     */
    nullptr,                   /* to_upper     */
    nullptr,                   /* sort_order   */
    nullptr,                   /* uca          */
    nullptr,                   /* tab_to_uni   */
    nullptr,                   /* tab_from_uni */
    &my_unicase_default,       /* caseinfo     */
    nullptr,                   /* state_map    */
    nullptr,                   /* ident_map    */
    8,                         /* strxfrm_multiply */
    1,                         /* caseup_multiply  */
    1,                         /* casedn_multiply  */
    1,                         /* mbminlen     */
    3,                         /* mbmaxlen     */
    1,                         /* mbmaxlenlen  */
    9,                         /* min_sort_char */
    0xFFFF,                    /* max_sort_char */
    ' ',                       /* pad char      */
    false,                     /* escape_with_backslash_is_dangerous */
    1,                         /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_unicode_520_ci = {
    214,
    0,
    0,                            /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS,      /* flags     */
    MY_UTF8MB3,                   /* csname       */
    MY_UTF8MB3 "_unicode_520_ci", /* name */
    "UTF-8 Unicode",              /* comment      */
    "",                           /* tailoring    */
    nullptr,                      /* coll_param   */
    ctype_utf8,                   /* ctype        */
    nullptr,                      /* to_lower     */
    nullptr,                      /* to_upper     */
    nullptr,                      /* sort_order   */
    &my_uca_v520,                 /* uca          */
    nullptr,                      /* tab_to_uni   */
    nullptr,                      /* tab_from_uni */
    &my_unicase_unicode520,       /* caseinfo   */
    nullptr,                      /* state_map    */
    nullptr,                      /* ident_map    */
    8,                            /* strxfrm_multiply */
    1,                            /* caseup_multiply  */
    1,                            /* casedn_multiply  */
    1,                            /* mbminlen     */
    3,                            /* mbmaxlen     */
    1,                            /* mbmaxlenlen  */
    9,                            /* min_sort_char */
    0xFFFF,                       /* max_sort_char */
    ' ',                          /* pad char      */
    false,                        /* escape_with_backslash_is_dangerous */
    1,                            /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8_vietnamese_ci = {
    215,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB3_UCA_FLAGS,     /* flags     */
    MY_UTF8MB3,                  /* cs name      */
    MY_UTF8MB3 "_vietnamese_ci", /* name  */
    "UTF-8 Unicode",             /* comment      */
    vietnamese,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    nullptr,                     /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_default,         /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    8,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen     */
    3,                           /* mbmaxlen     */
    1,                           /* mbmaxlenlen  */
    9,                           /* min_sort_char */
    0xFFFF,                      /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

extern MY_CHARSET_HANDLER my_charset_utf8mb4_handler;

#define MY_CS_UTF8MB4_UCA_FLAGS \
  (MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_UNICODE_SUPPLEMENT)

CHARSET_INFO my_charset_utf8mb4_unicode_ci = {
    224,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_unicode_ci", /* name    */
    "UTF-8 Unicode",          /* comment      */
    "",                       /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen     */
    4,                        /* mbmaxlen     */
    1,                        /* mbmaxlenlen  */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_icelandic_uca_ci = {
    225,
    0,
    0,                          /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,    /* state    */
    MY_UTF8MB4,                 /* csname     */
    MY_UTF8MB4 "_icelandic_ci", /* name */
    "UTF-8 Unicode",            /* comment      */
    icelandic,                  /* tailoring    */
    nullptr,                    /* coll_param   */
    ctype_utf8,                 /* ctype        */
    nullptr,                    /* to_lower     */
    nullptr,                    /* to_upper     */
    nullptr,                    /* sort_order   */
    nullptr,                    /* uca          */
    nullptr,                    /* tab_to_uni   */
    nullptr,                    /* tab_from_uni */
    &my_unicase_default,        /* caseinfo     */
    nullptr,                    /* state_map    */
    nullptr,                    /* ident_map    */
    8,                          /* strxfrm_multiply */
    1,                          /* caseup_multiply  */
    1,                          /* casedn_multiply  */
    1,                          /* mbminlen     */
    4,                          /* mbmaxlen     */
    1,                          /* mbmaxlenlen  */
    9,                          /* min_sort_char */
    0xFFFF,                     /* max_sort_char */
    ' ',                        /* pad char      */
    false,                      /* escape_with_backslash_is_dangerous */
    1,                          /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_latvian_uca_ci = {
    226,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_latvian_ci", /*   name */
    "UTF-8 Unicode",          /* comment      */
    latvian,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen     */
    4,                        /* mbmaxlen     */
    1,                        /* mbmaxlenlen  */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_romanian_uca_ci = {
    227,
    0,
    0,                         /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,   /* state    */
    MY_UTF8MB4,                /* csname      */
    MY_UTF8MB4 "_romanian_ci", /* name  */
    "UTF-8 Unicode",           /* comment      */
    romanian,                  /* tailoring    */
    nullptr,                   /* coll_param   */
    ctype_utf8,                /* ctype        */
    nullptr,                   /* to_lower     */
    nullptr,                   /* to_upper     */
    nullptr,                   /* sort_order   */
    nullptr,                   /* uca          */
    nullptr,                   /* tab_to_uni   */
    nullptr,                   /* tab_from_uni */
    &my_unicase_default,       /* caseinfo     */
    nullptr,                   /* state_map    */
    nullptr,                   /* ident_map    */
    8,                         /* strxfrm_multiply */
    1,                         /* caseup_multiply  */
    1,                         /* casedn_multiply  */
    1,                         /* mbminlen     */
    4,                         /* mbmaxlen     */
    1,                         /* mbmaxlenlen  */
    9,                         /* min_sort_char */
    0xFFFF,                    /* max_sort_char */
    ' ',                       /* pad char      */
    false,                     /* escape_with_backslash_is_dangerous */
    1,                         /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_slovenian_uca_ci = {
    228,
    0,
    0,                          /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,    /* state    */
    MY_UTF8MB4,                 /* csname      */
    MY_UTF8MB4 "_slovenian_ci", /* name  */
    "UTF-8 Unicode",            /* comment      */
    slovenian,                  /* tailoring    */
    nullptr,                    /* coll_param   */
    ctype_utf8,                 /* ctype        */
    nullptr,                    /* to_lower     */
    nullptr,                    /* to_upper     */
    nullptr,                    /* sort_order   */
    nullptr,                    /* uca          */
    nullptr,                    /* tab_to_uni   */
    nullptr,                    /* tab_from_uni */
    &my_unicase_default,        /* caseinfo     */
    nullptr,                    /* state_map    */
    nullptr,                    /* ident_map    */
    8,                          /* strxfrm_multiply */
    1,                          /* caseup_multiply  */
    1,                          /* casedn_multiply  */
    1,                          /* mbminlen     */
    4,                          /* mbmaxlen     */
    1,                          /* mbmaxlenlen  */
    9,                          /* min_sort_char */
    0xFFFF,                     /* max_sort_char */
    ' ',                        /* pad char      */
    false,                      /* escape_with_backslash_is_dangerous */
    1,                          /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_polish_uca_ci = {
    229,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS, /* state    */
    MY_UTF8MB4,              /* csname      */
    MY_UTF8MB4 "_polish_ci", /* name    */
    "UTF-8 Unicode",         /* comment      */
    polish,                  /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    4,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_estonian_uca_ci = {
    230,
    0,
    0,                         /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,   /* state    */
    MY_UTF8MB4,                /* csname      */
    MY_UTF8MB4 "_estonian_ci", /*  name */
    "UTF-8 Unicode",           /* comment      */
    estonian,                  /* tailoring    */
    nullptr,                   /* coll_param   */
    ctype_utf8,                /* ctype        */
    nullptr,                   /* to_lower     */
    nullptr,                   /* to_upper     */
    nullptr,                   /* sort_order   */
    nullptr,                   /* uca          */
    nullptr,                   /* tab_to_uni   */
    nullptr,                   /* tab_from_uni */
    &my_unicase_default,       /* caseinfo     */
    nullptr,                   /* state_map    */
    nullptr,                   /* ident_map    */
    8,                         /* strxfrm_multiply */
    1,                         /* caseup_multiply  */
    1,                         /* casedn_multiply  */
    1,                         /* mbminlen     */
    4,                         /* mbmaxlen     */
    1,                         /* mbmaxlenlen  */
    9,                         /* min_sort_char */
    0xFFFF,                    /* max_sort_char */
    ' ',                       /* pad char      */
    false,                     /* escape_with_backslash_is_dangerous */
    1,                         /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_spanish_uca_ci = {
    231,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_spanish_ci", /* name   */
    "UTF-8 Unicode",          /* comment      */
    spanish,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen     */
    4,                        /* mbmaxlen     */
    1,                        /* mbmaxlenlen  */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_swedish_uca_ci = {
    232,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_swedish_ci", /* name   */
    "UTF-8 Unicode",          /* comment      */
    swedish,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen     */
    4,                        /* mbmaxlen     */
    1,                        /* mbmaxlenlen  */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_turkish_uca_ci = {
    233,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_turkish_ci", /* name   */
    "UTF-8 Unicode",          /* comment      */
    turkish,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_turkish,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    2,                        /* caseup_multiply  */
    2,                        /* casedn_multiply  */
    1,                        /* mbminlen     */
    4,                        /* mbmaxlen     */
    1,                        /* mbmaxlenlen  */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_czech_uca_ci = {
    234,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS, /* state    */
    MY_UTF8MB4,              /* csname      */
    MY_UTF8MB4 "_czech_ci",  /* name     */
    "UTF-8 Unicode",         /* comment      */
    czech,                   /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    4,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_danish_uca_ci = {
    235,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS, /* state    */
    MY_UTF8MB4,              /* csname      */
    MY_UTF8MB4 "_danish_ci", /* name    */
    "UTF-8 Unicode",         /* comment      */
    danish,                  /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    4,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_lithuanian_uca_ci = {
    236,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname      */
    MY_UTF8MB4 "_lithuanian_ci", /* name */
    "UTF-8 Unicode",             /* comment      */
    lithuanian,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    nullptr,                     /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_default,         /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    8,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen     */
    4,                           /* mbmaxlen     */
    1,                           /* mbmaxlenlen  */
    9,                           /* min_sort_char */
    0xFFFF,                      /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_slovak_uca_ci = {
    237,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS, /* state    */
    MY_UTF8MB4,              /* csname      */
    MY_UTF8MB4 "_slovak_ci", /* name    */
    "UTF-8 Unicode",         /* comment      */
    slovak,                  /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    4,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_spanish2_uca_ci = {
    238,
    0,
    0,                         /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,   /* state    */
    MY_UTF8MB4,                /* csname      */
    MY_UTF8MB4 "_spanish2_ci", /* name */
    "UTF-8 Unicode",           /* comment      */
    spanish2,                  /* tailoring    */
    nullptr,                   /* coll_param   */
    ctype_utf8,                /* ctype        */
    nullptr,                   /* to_lower     */
    nullptr,                   /* to_upper     */
    nullptr,                   /* sort_order   */
    nullptr,                   /* uca          */
    nullptr,                   /* tab_to_uni   */
    nullptr,                   /* tab_from_uni */
    &my_unicase_default,       /* caseinfo     */
    nullptr,                   /* state_map    */
    nullptr,                   /* ident_map    */
    8,                         /* strxfrm_multiply */
    1,                         /* caseup_multiply  */
    1,                         /* casedn_multiply  */
    1,                         /* mbminlen     */
    4,                         /* mbmaxlen     */
    1,                         /* mbmaxlenlen  */
    9,                         /* min_sort_char */
    0xFFFF,                    /* max_sort_char */
    ' ',                       /* pad char      */
    false,                     /* escape_with_backslash_is_dangerous */
    1,                         /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_roman_uca_ci = {
    239,
    0,
    0,                       /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS, /* state    */
    MY_UTF8MB4,              /* csname      */
    MY_UTF8MB4 "_roman_ci",  /* name     */
    "UTF-8 Unicode",         /* comment      */
    roman,                   /* tailoring    */
    nullptr,                 /* coll_param   */
    ctype_utf8,              /* ctype        */
    nullptr,                 /* to_lower     */
    nullptr,                 /* to_upper     */
    nullptr,                 /* sort_order   */
    nullptr,                 /* uca          */
    nullptr,                 /* tab_to_uni   */
    nullptr,                 /* tab_from_uni */
    &my_unicase_default,     /* caseinfo     */
    nullptr,                 /* state_map    */
    nullptr,                 /* ident_map    */
    8,                       /* strxfrm_multiply */
    1,                       /* caseup_multiply  */
    1,                       /* casedn_multiply  */
    1,                       /* mbminlen     */
    4,                       /* mbmaxlen     */
    1,                       /* mbmaxlenlen  */
    9,                       /* min_sort_char */
    0xFFFF,                  /* max_sort_char */
    ' ',                     /* pad char      */
    false,                   /* escape_with_backslash_is_dangerous */
    1,                       /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_persian_uca_ci = {
    240,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_persian_ci", /* name   */
    "UTF-8 Unicode",          /* comment      */
    persian,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen     */
    4,                        /* mbmaxlen     */
    1,                        /* mbmaxlenlen  */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_esperanto_uca_ci = {
    241,
    0,
    0,                          /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,    /* state    */
    MY_UTF8MB4,                 /* csname      */
    MY_UTF8MB4 "_esperanto_ci", /* name  */
    "UTF-8 Unicode",            /* comment      */
    esperanto,                  /* tailoring    */
    nullptr,                    /* coll_param   */
    ctype_utf8,                 /* ctype        */
    nullptr,                    /* to_lower     */
    nullptr,                    /* to_upper     */
    nullptr,                    /* sort_order   */
    nullptr,                    /* uca          */
    nullptr,                    /* tab_to_uni   */
    nullptr,                    /* tab_from_uni */
    &my_unicase_default,        /* caseinfo     */
    nullptr,                    /* state_map    */
    nullptr,                    /* ident_map    */
    8,                          /* strxfrm_multiply */
    1,                          /* caseup_multiply  */
    1,                          /* casedn_multiply  */
    1,                          /* mbminlen     */
    4,                          /* mbmaxlen     */
    1,                          /* mbmaxlenlen  */
    9,                          /* min_sort_char */
    0xFFFF,                     /* max_sort_char */
    ' ',                        /* pad char      */
    false,                      /* escape_with_backslash_is_dangerous */
    1,                          /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_hungarian_uca_ci = {
    242,
    0,
    0,                          /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,    /* state    */
    MY_UTF8MB4,                 /* csname      */
    MY_UTF8MB4 "_hungarian_ci", /* name  */
    "UTF-8 Unicode",            /* comment      */
    hungarian,                  /* tailoring    */
    nullptr,                    /* coll_param   */
    ctype_utf8,                 /* ctype        */
    nullptr,                    /* to_lower     */
    nullptr,                    /* to_upper     */
    nullptr,                    /* sort_order   */
    nullptr,                    /* uca          */
    nullptr,                    /* tab_to_uni   */
    nullptr,                    /* tab_from_uni */
    &my_unicase_default,        /* caseinfo     */
    nullptr,                    /* state_map    */
    nullptr,                    /* ident_map    */
    8,                          /* strxfrm_multiply */
    1,                          /* caseup_multiply  */
    1,                          /* casedn_multiply  */
    1,                          /* mbminlen     */
    4,                          /* mbmaxlen     */
    1,                          /* mbmaxlenlen  */
    9,                          /* min_sort_char */
    0xFFFF,                     /* max_sort_char */
    ' ',                        /* pad char      */
    false,                      /* escape_with_backslash_is_dangerous */
    1,                          /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_sinhala_uca_ci = {
    243,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_sinhala_ci", /* name  */
    "UTF-8 Unicode",          /* comment      */
    sinhala,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen      */
    4,                        /* mbmaxlen      */
    1,                        /* mbmaxlenlen   */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_german2_uca_ci = {
    244,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname      */
    MY_UTF8MB4 "_german2_ci", /* name  */
    "UTF-8 Unicode",          /* comment      */
    german2,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    nullptr,                  /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_default,      /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    8,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen      */
    4,                        /* mbmaxlen      */
    1,                        /* mbmaxlenlen   */
    9,                        /* min_sort_char */
    0xFFFF,                   /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    1,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_croatian_uca_ci = {
    245,
    0,
    0,                         /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,   /* state    */
    MY_UTF8MB4,                /* csname      */
    MY_UTF8MB4 "_croatian_ci", /* name  */
    "UTF-8 Unicode",           /* comment      */
    croatian,                  /* tailoring    */
    nullptr,                   /* coll_param   */
    ctype_utf8,                /* ctype        */
    nullptr,                   /* to_lower     */
    nullptr,                   /* to_upper     */
    nullptr,                   /* sort_order   */
    nullptr,                   /* uca          */
    nullptr,                   /* tab_to_uni   */
    nullptr,                   /* tab_from_uni */
    &my_unicase_default,       /* caseinfo     */
    nullptr,                   /* state_map    */
    nullptr,                   /* ident_map    */
    8,                         /* strxfrm_multiply */
    1,                         /* caseup_multiply  */
    1,                         /* casedn_multiply  */
    1,                         /* mbminlen      */
    4,                         /* mbmaxlen      */
    1,                         /* mbmaxlenlen   */
    9,                         /* min_sort_char */
    0xFFFF,                    /* max_sort_char */
    ' ',                       /* pad char      */
    false,                     /* escape_with_backslash_is_dangerous */
    1,                         /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_unicode_520_ci = {
    246,
    0,
    0,                            /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,      /* flags     */
    MY_UTF8MB4,                   /* csname       */
    MY_UTF8MB4 "_unicode_520_ci", /* name */
    "UTF-8 Unicode",              /* comment      */
    "",                           /* tailoring    */
    nullptr,                      /* coll_param   */
    ctype_utf8,                   /* ctype        */
    nullptr,                      /* to_lower     */
    nullptr,                      /* to_upper     */
    nullptr,                      /* sort_order   */
    &my_uca_v520,                 /* uca          */
    nullptr,                      /* tab_to_uni   */
    nullptr,                      /* tab_from_uni */
    &my_unicase_unicode520,       /* caseinfo   */
    nullptr,                      /* state_map    */
    nullptr,                      /* ident_map    */
    8,                            /* strxfrm_multiply */
    1,                            /* caseup_multiply  */
    1,                            /* casedn_multiply  */
    1,                            /* mbminlen     */
    4,                            /* mbmaxlen     */
    1,                            /* mbmaxlenlen  */
    9,                            /* min_sort_char */
    0x10FFFF,                     /* max_sort_char */
    ' ',                          /* pad char      */
    false,                        /* escape_with_backslash_is_dangerous */
    1,                            /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_vietnamese_ci = {
    247,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_vietnamese_ci", /* name */
    "UTF-8 Unicode",             /* comment      */
    vietnamese,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    nullptr,                     /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_default,         /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    8,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0xFFFF,                      /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_any_uca_handler,
    PAD_SPACE};

MY_COLLATION_HANDLER my_collation_utf32_uca_handler = {
    my_coll_init_uca, /* init */
    my_coll_uninit_uca,
    my_strnncoll_any_uca,
    my_strnncollsp_any_uca,
    my_strnxfrm_any_uca,
    my_strnxfrmlen_simple,
    my_like_range_generic,
    my_wildcmp_uca,
    nullptr,
    my_instr_mb,
    my_hash_sort_any_uca,
    my_propagate_complex};

extern MY_CHARSET_HANDLER my_charset_utf32_handler;

#define MY_CS_UTF32_UCA_FLAGS                        \
  (MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | \
   MY_CS_UNICODE_SUPPLEMENT | MY_CS_NONASCII)

CHARSET_INFO my_charset_utf32_unicode_ci = {
    160,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_unicode_ci",    /* name         */
    "",                    /* comment      */
    "",                    /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_icelandic_uca_ci = {
    161,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_icelandic_ci",  /* name         */
    "",                    /* comment      */
    icelandic,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_latvian_uca_ci = {
    162,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_latvian_ci",    /* name         */
    "",                    /* comment      */
    latvian,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_romanian_uca_ci = {
    163,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_romanian_ci",   /* name         */
    "",                    /* comment      */
    romanian,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_slovenian_uca_ci = {
    164,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_slovenian_ci",  /* name         */
    "",                    /* comment      */
    slovenian,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_polish_uca_ci = {
    165,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_polish_ci",     /* name         */
    "",                    /* comment      */
    polish,                /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_estonian_uca_ci = {
    166,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_estonian_ci",   /* name         */
    "",                    /* comment      */
    estonian,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_spanish_uca_ci = {
    167,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_spanish_ci",    /* name         */
    "",                    /* comment      */
    spanish,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_swedish_uca_ci = {
    168,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_swedish_ci",    /* name         */
    "",                    /* comment      */
    swedish,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_turkish_uca_ci = {
    169,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_turkish_ci",    /* name         */
    "",                    /* comment      */
    turkish,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_turkish,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_czech_uca_ci = {
    170,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_czech_ci",      /* name         */
    "",                    /* comment      */
    czech,                 /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_danish_uca_ci = {
    171,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_danish_ci",     /* name         */
    "",                    /* comment      */
    danish,                /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_lithuanian_uca_ci = {
    172,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_lithuanian_ci", /* name        */
    "",                    /* comment      */
    lithuanian,            /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_slovak_uca_ci = {
    173,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_slovak_ci",     /* name         */
    "",                    /* comment      */
    slovak,                /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_spanish2_uca_ci = {
    174,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_spanish2_ci",   /* name         */
    "",                    /* comment      */
    spanish2,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_roman_uca_ci = {
    175,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_roman_ci",      /* name         */
    "",                    /* comment      */
    roman,                 /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_persian_uca_ci = {
    176,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_persian_ci",    /* name         */
    "",                    /* comment      */
    persian,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_esperanto_uca_ci = {
    177,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_esperanto_ci",  /* name         */
    "",                    /* comment      */
    esperanto,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_hungarian_uca_ci = {
    178,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state       */
    "utf32",               /* csname       */
    "utf32_hungarian_ci",  /* name         */
    "",                    /* comment      */
    hungarian,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_sinhala_uca_ci = {
    179,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state      */
    "utf32",               /* csname       */
    "utf32_sinhala_ci",    /* name         */
    "",                    /* comment      */
    sinhala,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_german2_uca_ci = {
    180,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state      */
    "utf32",               /* csname      */
    "utf32_german2_ci",    /* name         */
    "",                    /* comment      */
    german2,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_croatian_uca_ci = {
    181,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state      */
    "utf32",               /* csname      */
    "utf32_croatian_ci",   /* name        */
    "",                    /* comment      */
    croatian,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_unicode_520_ci = {
    182,
    0,
    0,                      /* number       */
    MY_CS_UTF32_UCA_FLAGS,  /* stat e      */
    "utf32",                /* csname       */
    "utf32_unicode_520_ci", /* name       */
    "",                     /* comment      */
    "",                     /* tailoring    */
    nullptr,                /* coll_param   */
    nullptr,                /* ctype        */
    nullptr,                /* to_lower     */
    nullptr,                /* to_upper     */
    nullptr,                /* sort_order   */
    &my_uca_v520,           /* uca          */
    nullptr,                /* tab_to_uni   */
    nullptr,                /* tab_from_uni */
    &my_unicase_unicode520, /* caseinfo   */
    nullptr,                /* state_map    */
    nullptr,                /* ident_map    */
    8,                      /* strxfrm_multiply */
    1,                      /* caseup_multiply  */
    1,                      /* casedn_multiply  */
    4,                      /* mbminlen     */
    4,                      /* mbmaxlen     */
    1,                      /* mbmaxlenlen  */
    9,                      /* min_sort_char */
    0x10FFFF,               /* max_sort_char */
    ' ',                    /* pad char      */
    false,                  /* escape_with_backslash_is_dangerous */
    1,                      /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf32_vietnamese_ci = {
    183,
    0,
    0,                     /* number       */
    MY_CS_UTF32_UCA_FLAGS, /* state      */
    "utf32",               /* csname       */
    "utf32_vietnamese_ci", /* name       */
    "",                    /* comment      */
    vietnamese,            /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    4,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf32_handler,
    &my_collation_utf32_uca_handler,
    PAD_SPACE};

MY_COLLATION_HANDLER my_collation_utf16_uca_handler = {
    my_coll_init_uca, /* init */
    my_coll_uninit_uca,
    my_strnncoll_any_uca,
    my_strnncollsp_any_uca,
    my_strnxfrm_any_uca,
    my_strnxfrmlen_simple,
    my_like_range_generic,
    my_wildcmp_uca,
    nullptr,
    my_instr_mb,
    my_hash_sort_any_uca,
    my_propagate_complex};

extern MY_CHARSET_HANDLER my_charset_utf16_handler;

#define MY_CS_UTF16_UCA_FLAGS \
  (MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII)

CHARSET_INFO my_charset_utf16_unicode_ci = {
    101,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* csname       */
    "utf16_unicode_ci",    /* name         */
    "",                    /* comment      */
    "",                    /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_icelandic_uca_ci = {
    102,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* csname       */
    "utf16_icelandic_ci",  /* name         */
    "",                    /* comment      */
    icelandic,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_latvian_uca_ci = {
    103,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_latvian_ci",    /* name         */
    "",                    /* comment      */
    latvian,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_romanian_uca_ci = {
    104,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_romanian_ci",   /* name         */
    "",                    /* comment      */
    romanian,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_slovenian_uca_ci = {
    105,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_slovenian_ci",  /* name         */
    "",                    /* comment      */
    slovenian,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_polish_uca_ci = {
    106,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_polish_ci",     /* name         */
    "",                    /* comment      */
    polish,                /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_estonian_uca_ci = {
    107,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_estonian_ci",   /* name         */
    "",                    /* comment      */
    estonian,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_spanish_uca_ci = {
    108,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_spanish_ci",    /* name         */
    "",                    /* comment      */
    spanish,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_swedish_uca_ci = {
    109,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_swedish_ci",    /* name         */
    "",                    /* comment      */
    swedish,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_turkish_uca_ci = {
    110,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_turkish_ci",    /* name         */
    "",                    /* comment      */
    turkish,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_turkish,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_czech_uca_ci = {
    111,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_czech_ci",      /* name         */
    "",                    /* comment      */
    czech,                 /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_danish_uca_ci = {
    112,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_danish_ci",     /* name         */
    "",                    /* comment      */
    danish,                /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_lithuanian_uca_ci = {
    113,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_lithuanian_ci", /* name        */
    "",                    /* comment      */
    lithuanian,            /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_slovak_uca_ci = {
    114,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state       */
    "utf16",               /* cs name      */
    "utf16_slovak_ci",     /* name         */
    "",                    /* comment      */
    slovak,                /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_spanish2_uca_ci = {
    115,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state      */
    "utf16",               /* cs name      */
    "utf16_spanish2_ci",   /* name         */
    "",                    /* comment      */
    spanish2,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen      */
    4,                     /* mbmaxlen      */
    1,                     /* mbmaxlenlen   */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_roman_uca_ci = {
    116,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state      */
    "utf16",               /* cs name      */
    "utf16_roman_ci",      /* name         */
    "",                    /* comment      */
    roman,                 /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen      */
    4,                     /* mbmaxlen      */
    1,                     /* mbmaxlenlen   */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_persian_uca_ci = {
    117,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state      */
    "utf16",               /* cs name      */
    "utf16_persian_ci",    /* name         */
    "",                    /* comment      */
    persian,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen      */
    4,                     /* mbmaxlen      */
    1,                     /* mbmaxlenlen   */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_esperanto_uca_ci = {
    118,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state      */
    "utf16",               /* cs name      */
    "utf16_esperanto_ci",  /* name        */
    "",                    /* comment      */
    esperanto,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo     */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen      */
    4,                     /* mbmaxlen      */
    1,                     /* mbmaxlenlen   */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_hungarian_uca_ci = {
    119,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state     */
    "utf16",               /* cs name      */
    "utf16_hungarian_ci",  /* name       */
    "",                    /* comment      */
    hungarian,             /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo    */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen      */
    4,                     /* mbmaxlen      */
    1,                     /* mbmaxlenlen   */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_sinhala_uca_ci = {
    120,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state     */
    "utf16",               /* cs name      */
    "utf16_sinhala_ci",    /* name         */
    "",                    /* comment      */
    sinhala,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo    */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_german2_uca_ci = {
    121,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state     */
    "utf16",               /* cs name    */
    "utf16_german2_ci",    /* name         */
    "",                    /* comment      */
    german2,               /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo    */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_croatian_uca_ci = {
    122,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state     */
    "utf16",               /* cs name    */
    "utf16_croatian_ci",   /* name         */
    "",                    /* comment      */
    croatian,              /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo    */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_unicode_520_ci = {
    123,
    0,
    0, /* number       */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_UNICODE | MY_CS_NONASCII,
    "utf16",                /* csname       */
    "utf16_unicode_520_ci", /* name       */
    "",                     /* comment      */
    "",                     /* tailoring    */
    nullptr,                /* coll_param   */
    nullptr,                /* ctype        */
    nullptr,                /* to_lower     */
    nullptr,                /* to_upper     */
    nullptr,                /* sort_order   */
    &my_uca_v520,           /* uca          */
    nullptr,                /* tab_to_uni   */
    nullptr,                /* tab_from_uni */
    &my_unicase_unicode520, /* caseinfo   */
    nullptr,                /* state_map    */
    nullptr,                /* ident_map    */
    8,                      /* strxfrm_multiply */
    1,                      /* caseup_multiply  */
    1,                      /* casedn_multiply  */
    2,                      /* mbminlen     */
    4,                      /* mbmaxlen     */
    1,                      /* mbmaxlenlen  */
    9,                      /* min_sort_char */
    0x10FFFF,               /* max_sort_char */
    0x20,                   /* pad char      */
    false,                  /* escape_with_backslash_is_dangerous */
    1,                      /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf16_vietnamese_ci = {
    124,
    0,
    0,                     /* number       */
    MY_CS_UTF16_UCA_FLAGS, /* state     */
    "utf16",               /* cs name      */
    "utf16_vietnamese_ci", /* name      */
    "",                    /* comment      */
    vietnamese,            /* tailoring    */
    nullptr,               /* coll_param   */
    nullptr,               /* ctype        */
    nullptr,               /* to_lower     */
    nullptr,               /* to_upper     */
    nullptr,               /* sort_order   */
    nullptr,               /* uca          */
    nullptr,               /* tab_to_uni   */
    nullptr,               /* tab_from_uni */
    &my_unicase_default,   /* caseinfo    */
    nullptr,               /* state_map    */
    nullptr,               /* ident_map    */
    8,                     /* strxfrm_multiply */
    1,                     /* caseup_multiply  */
    1,                     /* casedn_multiply  */
    2,                     /* mbminlen     */
    4,                     /* mbmaxlen     */
    1,                     /* mbmaxlenlen  */
    9,                     /* min_sort_char */
    0xFFFF,                /* max_sort_char */
    ' ',                   /* pad char      */
    false,                 /* escape_with_backslash_is_dangerous */
    1,                     /* levels_for_compare */
    &my_charset_utf16_handler,
    &my_collation_utf16_uca_handler,
    PAD_SPACE};

MY_COLLATION_HANDLER my_collation_gb18030_uca_handler = {
    my_coll_init_uca, /* init */
    my_coll_uninit_uca,
    my_strnncoll_any_uca,
    my_strnncollsp_any_uca,
    my_strnxfrm_any_uca,
    my_strnxfrmlen_simple,
    my_like_range_mb,
    my_wildcmp_uca,
    nullptr,
    my_instr_mb,
    my_hash_sort_any_uca,
    my_propagate_complex};

/**
  The array used for "type of characters" bit mask for each
  character. The ctype[0] is reserved for EOF(-1), so we use
  ctype[(char)+1]. Also refer to strings/CHARSET_INFO.txt
*/
static const uchar ctype_gb18030[257] = {
    0, /* For standard library */
    32,  32,  32,  32,  32,  32,  32,  32,  32,  40,  40, 40, 40, 40, 32, 32,
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32, 32, 32, 32,
    72,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 16, 16, 16, 16, 16,
    132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 16, 16, 16, 16, 16, 16,
    16,  129, 129, 129, 129, 129, 129, 1,   1,   1,   1,  1,  1,  1,  1,  1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  16, 16, 16, 16, 16,
    16,  130, 130, 130, 130, 130, 130, 2,   2,   2,   2,  2,  2,  2,  2,  2,
    2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  16, 16, 16, 16, 32,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  3,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,  3,  3,  3,  3,  0};

extern MY_CHARSET_HANDLER my_charset_gb18030_uca_handler;

CHARSET_INFO my_charset_gb18030_unicode_520_ci = {
    250,
    0,
    0,                                                /* number        */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_NONASCII, /* state         */
    "gb18030",                                        /* cs name       */
    "gb18030_unicode_520_ci",                         /* name        */
    "China National Standard GB18030",                /* comment       */
    "",                                               /* tailoring     */
    nullptr,                                          /* coll_param   */
    ctype_gb18030,                                    /* ctype         */
    nullptr,                                          /* lower         */
    nullptr,                                          /* UPPER         */
    nullptr,                                          /* sort order    */
    &my_uca_v520,                                     /* uca           */
    nullptr,                                          /* tab_to_uni    */
    nullptr,                                          /* tab_from_uni  */
    &my_unicase_unicode520,                           /* caseinfo     */
    nullptr,                                          /* state_map     */
    nullptr,                                          /* ident_map     */
    8,                                                /* strxfrm_multiply */
    2,                                                /* caseup_multiply  */
    2,                                                /* casedn_multiply  */
    1,                                                /* mbminlen      */
    4,                                                /* mbmaxlen      */
    2,                                                /* mbmaxlenlen   */
    0,                                                /* min_sort_char */
    0xE3329A35,                                       /* max_sort_char */
    ' ',                                              /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    1,     /* levels_for_compare */
    &my_charset_gb18030_uca_handler,
    &my_collation_gb18030_uca_handler,
    PAD_SPACE};

CHARSET_INFO my_charset_utf8mb4_0900_ai_ci = {
    255,
    0,
    0,                                       /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_PRIMARY, /* state    */
    MY_UTF8MB4,                              /* csname       */
    MY_UTF8MB4 "_0900_ai_ci",                /* name */
    "UTF-8 Unicode",                         /* comment      */
    nullptr,                                 /* tailoring    */
    nullptr,                                 /* coll_param   */
    ctype_utf8,                              /* ctype        */
    nullptr,                                 /* to_lower     */
    nullptr,                                 /* to_upper     */
    nullptr,                                 /* sort_order   */
    &my_uca_v900,                            /* uca_900      */
    nullptr,                                 /* tab_to_uni   */
    nullptr,                                 /* tab_from_uni */
    &my_unicase_unicode900,                  /* caseinfo     */
    nullptr,                                 /* state_map    */
    nullptr,                                 /* ident_map    */
    0,                                       /* strxfrm_multiply */
    1,                                       /* caseup_multiply  */
    1,                                       /* casedn_multiply  */
    1,                                       /* mbminlen      */
    4,                                       /* mbmaxlen      */
    1,                                       /* mbmaxlenlen   */
    9,                                       /* min_sort_char */
    0x10FFFF,                                /* max_sort_char */
    ' ',                                     /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    1,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_de_pb_0900_ai_ci = {
    256,
    0,
    0,                              /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,        /* state    */
    MY_UTF8MB4,                     /* csname       */
    MY_UTF8MB4 "_de_pb_0900_ai_ci", /* name */
    "",                             /* comment      */
    de_pb_cldr_30,                  /* tailoring    */
    nullptr,                        /* coll_param   */
    ctype_utf8,                     /* ctype        */
    nullptr,                        /* to_lower     */
    nullptr,                        /* to_upper     */
    nullptr,                        /* sort_order   */
    &my_uca_v900,                   /* uca_900          */
    nullptr,                        /* tab_to_uni   */
    nullptr,                        /* tab_from_uni */
    &my_unicase_unicode900,         /* caseinfo     */
    nullptr,                        /* state_map    */
    nullptr,                        /* ident_map    */
    0,                              /* strxfrm_multiply */
    1,                              /* caseup_multiply  */
    1,                              /* casedn_multiply  */
    1,                              /* mbminlen      */
    4,                              /* mbmaxlen      */
    1,                              /* mbmaxlenlen   */
    9,                              /* min_sort_char */
    0x10FFFF,                       /* max_sort_char */
    ' ',                            /* pad char      */
    false,                          /* escape_with_backslash_is_dangerous */
    1,                              /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_is_0900_ai_ci = {
    257,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_is_0900_ai_ci", /* name */
    "",                          /* comment      */
    is_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_lv_0900_ai_ci = {
    258,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_lv_0900_ai_ci", /* name */
    "",                          /* comment      */
    lv_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_ro_0900_ai_ci = {
    259,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_ro_0900_ai_ci", /* name */
    "",                          /* comment      */
    ro_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_sl_0900_ai_ci = {
    260,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_sl_0900_ai_ci", /* name */
    "",                          /* comment      */
    sl_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_pl_0900_ai_ci = {
    261,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_pl_0900_ai_ci", /* name */
    "",                          /* comment      */
    pl_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_et_0900_ai_ci = {
    262,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_et_0900_ai_ci", /* name */
    "",                          /* comment      */
    et_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_es_0900_ai_ci = {
    263,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_es_0900_ai_ci", /* name */
    "",                          /* comment      */
    spanish,                     /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_sv_0900_ai_ci = {
    264,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_sv_0900_ai_ci", /* name */
    "",                          /* comment      */
    sv_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_tr_0900_ai_ci = {
    265,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_tr_0900_ai_ci", /* name */
    "",                          /* comment      */
    tr_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_cs_0900_ai_ci = {
    266,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_cs_0900_ai_ci", /* name */
    "",                          /* comment      */
    cs_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_da_0900_ai_ci = {
    267,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_da_0900_ai_ci", /* name */
    "",                          /* comment      */
    da_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_lt_0900_ai_ci = {
    268,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_lt_0900_ai_ci", /* name */
    "",                          /* comment      */
    lt_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_sk_0900_ai_ci = {
    269,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_sk_0900_ai_ci", /* name */
    "",                          /* comment      */
    sk_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_es_trad_0900_ai_ci = {
    270,
    0,
    0,                                /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,          /* state    */
    MY_UTF8MB4,                       /* csname       */
    MY_UTF8MB4 "_es_trad_0900_ai_ci", /* name */
    "",                               /* comment      */
    es_trad_cldr_30,                  /* tailoring    */
    nullptr,                          /* coll_param   */
    ctype_utf8,                       /* ctype        */
    nullptr,                          /* to_lower     */
    nullptr,                          /* to_upper     */
    nullptr,                          /* sort_order   */
    &my_uca_v900,                     /* uca          */
    nullptr,                          /* tab_to_uni   */
    nullptr,                          /* tab_from_uni */
    &my_unicase_unicode900,           /* caseinfo     */
    nullptr,                          /* state_map    */
    nullptr,                          /* ident_map    */
    0,                                /* strxfrm_multiply */
    1,                                /* caseup_multiply  */
    1,                                /* casedn_multiply  */
    1,                                /* mbminlen      */
    4,                                /* mbmaxlen      */
    1,                                /* mbmaxlenlen   */
    9,                                /* min_sort_char */
    0x10FFFF,                         /* max_sort_char */
    ' ',                              /* pad char      */
    false,                            /* escape_with_backslash_is_dangerous */
    1,                                /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_la_0900_ai_ci = {
    271,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_la_0900_ai_ci", /* name */
    "",                          /* comment      */
    roman,                       /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

#if 0
CHARSET_INFO my_charset_utf8mb4_fa_0900_ai_ci=
{
  272, 0, 0,            /* number       */
  MY_CS_UTF8MB4_UCA_FLAGS,/* state    */
  MY_UTF8MB4,         /* csname       */
  MY_UTF8MB4 "_fa_0900_ai_ci",/* name */
  "",                 /* comment      */
  fa_cldr_30,         /* tailoring    */
  &fa_coll_param,     /* coll_param   */
  ctype_utf8,         /* ctype        */
  NULL,               /* to_lower     */
  NULL,               /* to_upper     */
  NULL,               /* sort_order   */
  &my_uca_v900,       /* uca          */
  NULL,               /* tab_to_uni   */
  NULL,               /* tab_from_uni */
  &my_unicase_unicode900,/* caseinfo     */
  NULL,               /* state_map    */
  NULL,               /* ident_map    */
  0,                  /* strxfrm_multiply */
  1,                  /* caseup_multiply  */
  1,                  /* casedn_multiply  */
  1,                  /* mbminlen      */
  4,                  /* mbmaxlen      */
  1,                  /* mbmaxlenlen   */
  9,                  /* min_sort_char */
  0x10FFFF,           /* max_sort_char */
  ' ',                /* pad char      */
  0,                  /* escape_with_backslash_is_dangerous */
  1,                  /* levels_for_compare */
  &my_charset_utf8mb4_handler,
  &my_collation_uca_900_handler,
  NO_PAD
};
#endif

CHARSET_INFO my_charset_utf8mb4_eo_0900_ai_ci = {
    273,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_eo_0900_ai_ci", /* name */
    "",                          /* comment      */
    esperanto,                   /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_hu_0900_ai_ci = {
    274,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_hu_0900_ai_ci", /* name */
    "",                          /* comment      */
    hu_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_hr_0900_ai_ci = {
    275,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_hr_0900_ai_ci", /* name */
    "",                          /* comment      */
    hr_cldr_30,                  /* tailoring    */
    &hr_coll_param,              /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

#if 0
CHARSET_INFO my_charset_utf8mb4_si_0900_ai_ci=
{
  276, 0, 0,            /* number       */
  MY_CS_UTF8MB4_UCA_FLAGS,/* state    */
  MY_UTF8MB4,         /* csname       */
  MY_UTF8MB4 "_si_0900_ai_ci",/* name */
  "",                 /* comment      */
  si_cldr_30,         /* tailoring    */
  NULL,               /* coll_param   */
  ctype_utf8,         /* ctype        */
  NULL,               /* to_lower     */
  NULL,               /* to_upper     */
  NULL,               /* sort_order   */
  &my_uca_v900,       /* uca          */
  NULL,               /* tab_to_uni   */
  NULL,               /* tab_from_uni */
  &my_unicase_unicode900,/* caseinfo     */
  NULL,               /* state_map    */
  NULL,               /* ident_map    */
  0,                  /* strxfrm_multiply */
  1,                  /* caseup_multiply  */
  1,                  /* casedn_multiply  */
  1,                  /* mbminlen      */
  4,                  /* mbmaxlen      */
  1,                  /* mbmaxlenlen   */
  9,                  /* min_sort_char */
  0x10FFFF,           /* max_sort_char */
  ' ',                /* pad char      */
  0,                  /* escape_with_backslash_is_dangerous */
  1,                  /* levels_for_compare */
  &my_charset_utf8mb4_handler,
  &my_collation_uca_900_handler,
  NO_PAD
};
#endif

CHARSET_INFO my_charset_utf8mb4_vi_0900_ai_ci = {
    277,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_vi_0900_ai_ci", /* name */
    "",                          /* comment      */
    vi_cldr_30,                  /* tailoring    */
    nullptr,                     /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    9,                           /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_0900_as_cs = {
    278,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_0900_as_cs",               /* name */
    "",                                     /* comment      */
    nullptr,                                /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_de_pb_0900_as_cs = {
    279,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_de_pb_0900_as_cs",         /* name */
    "",                                     /* comment      */
    de_pb_cldr_30,                          /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_is_0900_as_cs = {
    280,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_is_0900_as_cs",            /* name */
    "",                                     /* comment      */
    is_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_lv_0900_as_cs = {
    281,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_lv_0900_as_cs",            /* name */
    "",                                     /* comment      */
    lv_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_ro_0900_as_cs = {
    282,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_ro_0900_as_cs",            /* name */
    "",                                     /* comment      */
    ro_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_sl_0900_as_cs = {
    283,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_sl_0900_as_cs",            /* name */
    "",                                     /* comment      */
    sl_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_pl_0900_as_cs = {
    284,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_pl_0900_as_cs",            /* name */
    "",                                     /* comment      */
    pl_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_et_0900_as_cs = {
    285,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_et_0900_as_cs",            /* name */
    "",                                     /* comment      */
    et_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_es_0900_as_cs = {
    286,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_es_0900_as_cs",            /* name */
    "",                                     /* comment      */
    spanish,                                /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_sv_0900_as_cs = {
    287,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_sv_0900_as_cs",            /* name */
    "",                                     /* comment      */
    sv_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_tr_0900_as_cs = {
    288,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_tr_0900_as_cs",            /* name */
    "",                                     /* comment      */
    tr_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_cs_0900_as_cs = {
    289,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_cs_0900_as_cs",            /* name */
    "",                                     /* comment      */
    cs_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_da_0900_as_cs = {
    290,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_da_0900_as_cs",            /* name */
    "",                                     /* comment      */
    da_cldr_30,                             /* tailoring    */
    &da_coll_param,                         /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_lt_0900_as_cs = {
    291,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_lt_0900_as_cs",            /* name */
    "",                                     /* comment      */
    lt_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_sk_0900_as_cs = {
    292,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_sk_0900_as_cs",            /* name */
    "",                                     /* comment      */
    sk_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_es_trad_0900_as_cs = {
    293,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_es_trad_0900_as_cs",       /* name */
    "",                                     /* comment      */
    es_trad_cldr_30,                        /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_la_0900_as_cs = {
    294,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_la_0900_as_cs",            /* name */
    "",                                     /* comment      */
    roman,                                  /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

#if 0
CHARSET_INFO my_charset_utf8mb4_fa_0900_as_cs=
{
  295, 0, 0,            /* number       */
  MY_CS_UTF8MB4_UCA_FLAGS|MY_CS_CSSORT,/* state    */
  MY_UTF8MB4,         /* csname       */
  MY_UTF8MB4 "_fa_0900_as_cs",/* name */
  "",                 /* comment      */
  fa_cldr_30,         /* tailoring    */
  &fa_coll_param,     /* coll_param   */
  ctype_utf8,         /* ctype        */
  NULL,               /* to_lower     */
  NULL,               /* to_upper     */
  NULL,               /* sort_order   */
  &my_uca_v900,       /* uca          */
  NULL,               /* tab_to_uni   */
  NULL,               /* tab_from_uni */
  &my_unicase_unicode900,/* caseinfo     */
  NULL,               /* state_map    */
  NULL,               /* ident_map    */
  0,                  /* strxfrm_multiply */
  1,                  /* caseup_multiply  */
  1,                  /* casedn_multiply  */
  1,                  /* mbminlen      */
  4,                  /* mbmaxlen      */
  1,                  /* mbmaxlenlen   */
  9,                  /* min_sort_char */
  0x10FFFF,           /* max_sort_char */
  ' ',                /* pad char      */
  0,                  /* escape_with_backslash_is_dangerous */
  3,                  /* levels_for_compare */
  &my_charset_utf8mb4_handler,
  &my_collation_uca_900_handler,
  NO_PAD
};
#endif

CHARSET_INFO my_charset_utf8mb4_eo_0900_as_cs = {
    296,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_eo_0900_as_cs",            /* name */
    "",                                     /* comment      */
    esperanto,                              /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_hu_0900_as_cs = {
    297,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_hu_0900_as_cs",            /* name */
    "",                                     /* comment      */
    hu_cldr_30,                             /* tailoring    */
    nullptr,                                /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_hr_0900_as_cs = {
    298,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_hr_0900_as_cs",            /* name */
    "",                                     /* comment      */
    hr_cldr_30,                             /* tailoring    */
    &hr_coll_param,                         /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

#if 0
CHARSET_INFO my_charset_utf8mb4_si_0900_as_cs=
{
  299, 0, 0,            /* number       */
  MY_CS_UTF8MB4_UCA_FLAGS|MY_CS_CSSORT,/* state    */
  MY_UTF8MB4,         /* csname       */
  MY_UTF8MB4 "_si_0900_as_cs",/* name */
  "",                 /* comment      */
  si_cldr_30,         /* tailoring    */
  NULL,               /* coll_param   */
  ctype_utf8,         /* ctype        */
  NULL,               /* to_lower     */
  NULL,               /* to_upper     */
  NULL,               /* sort_order   */
  &my_uca_v900,       /* uca          */
  NULL,               /* tab_to_uni   */
  NULL,               /* tab_from_uni */
  &my_unicase_unicode900,/* caseinfo     */
  NULL,               /* state_map    */
  NULL,               /* ident_map    */
  0,                  /* strxfrm_multiply */
  1,                  /* caseup_multiply  */
  1,                  /* casedn_multiply  */
  1,                  /* mbminlen      */
  4,                  /* mbmaxlen      */
  1,                  /* mbmaxlenlen   */
  9,                  /* min_sort_char */
  0x10FFFF,           /* max_sort_char */
  ' ',                /* pad char      */
  0,                  /* escape_with_backslash_is_dangerous */
  3,                  /* levels_for_compare */
  &my_charset_utf8mb4_handler,
  &my_collation_uca_900_handler,
  NO_PAD
};
#endif

CHARSET_INFO my_charset_utf8mb4_vi_0900_as_cs = {
    300,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_vi_0900_as_cs",            /* name */
    "",                                     /* comment      */
    vi_cldr_30,                             /* tailoring    */
    &vi_coll_param,                         /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    9,                                      /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_ja_0900_as_cs = {
    303,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_ja_0900_as_cs",            /* name */
    "",                                     /* comment      */
    ja_cldr_30,                             /* tailoring    */
    &ja_coll_param,                         /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    32,                                     /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_ja_0900_as_cs_ks = {
    304,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_ja_0900_as_cs_ks",         /* name */
    "",                                     /* comment      */
    ja_cldr_30,                             /* tailoring    */
    &ja_coll_param,                         /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    24,                                     /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    32,                                     /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    4,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_0900_as_ci = {
    305,
    0,
    0,                        /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,  /* state    */
    MY_UTF8MB4,               /* csname       */
    MY_UTF8MB4 "_0900_as_ci", /* name */
    "",                       /* comment      */
    nullptr,                  /* tailoring    */
    nullptr,                  /* coll_param   */
    ctype_utf8,               /* ctype        */
    nullptr,                  /* to_lower     */
    nullptr,                  /* to_upper     */
    nullptr,                  /* sort_order   */
    &my_uca_v900,             /* uca          */
    nullptr,                  /* tab_to_uni   */
    nullptr,                  /* tab_from_uni */
    &my_unicase_unicode900,   /* caseinfo     */
    nullptr,                  /* state_map    */
    nullptr,                  /* ident_map    */
    0,                        /* strxfrm_multiply */
    1,                        /* caseup_multiply  */
    1,                        /* casedn_multiply  */
    1,                        /* mbminlen      */
    4,                        /* mbmaxlen      */
    1,                        /* mbmaxlenlen   */
    32,                       /* min_sort_char */
    0x10FFFF,                 /* max_sort_char */
    ' ',                      /* pad char      */
    false,                    /* escape_with_backslash_is_dangerous */
    2,                        /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_ru_0900_ai_ci = {
    306,
    0,
    0,                           /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS,     /* state    */
    MY_UTF8MB4,                  /* csname       */
    MY_UTF8MB4 "_ru_0900_ai_ci", /* name */
    "",                          /* comment      */
    "",                          /* tailoring    */
    &ru_coll_param,              /* coll_param   */
    ctype_utf8,                  /* ctype        */
    nullptr,                     /* to_lower     */
    nullptr,                     /* to_upper     */
    nullptr,                     /* sort_order   */
    &my_uca_v900,                /* uca          */
    nullptr,                     /* tab_to_uni   */
    nullptr,                     /* tab_from_uni */
    &my_unicase_unicode900,      /* caseinfo     */
    nullptr,                     /* state_map    */
    nullptr,                     /* ident_map    */
    0,                           /* strxfrm_multiply */
    1,                           /* caseup_multiply  */
    1,                           /* casedn_multiply  */
    1,                           /* mbminlen      */
    4,                           /* mbmaxlen      */
    1,                           /* mbmaxlenlen   */
    32,                          /* min_sort_char */
    0x10FFFF,                    /* max_sort_char */
    ' ',                         /* pad char      */
    false,                       /* escape_with_backslash_is_dangerous */
    1,                           /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_ru_0900_as_cs = {
    307,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_ru_0900_as_cs",            /* name */
    "",                                     /* comment      */
    "",                                     /* tailoring    */
    &ru_coll_param,                         /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    32,                                     /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

CHARSET_INFO my_charset_utf8mb4_zh_0900_as_cs = {
    308,
    0,
    0,                                      /* number       */
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_CSSORT, /* state    */
    MY_UTF8MB4,                             /* csname       */
    MY_UTF8MB4 "_zh_0900_as_cs",            /* name */
    "",                                     /* comment      */
    zh_cldr_30,                             /* tailoring    */
    &zh_coll_param,                         /* coll_param   */
    ctype_utf8,                             /* ctype        */
    nullptr,                                /* to_lower     */
    nullptr,                                /* to_upper     */
    nullptr,                                /* sort_order   */
    &my_uca_v900,                           /* uca          */
    nullptr,                                /* tab_to_uni   */
    nullptr,                                /* tab_from_uni */
    &my_unicase_unicode900,                 /* caseinfo     */
    nullptr,                                /* state_map    */
    nullptr,                                /* ident_map    */
    0,                                      /* strxfrm_multiply */
    1,                                      /* caseup_multiply  */
    1,                                      /* casedn_multiply  */
    1,                                      /* mbminlen      */
    4,                                      /* mbmaxlen      */
    1,                                      /* mbmaxlenlen   */
    32,                                     /* min_sort_char */
    0x10FFFF,                               /* max_sort_char */
    ' ',                                    /* pad char      */
    false, /* escape_with_backslash_is_dangerous */
    3,     /* levels_for_compare */
    &my_charset_utf8mb4_handler,
    &my_collation_uca_900_handler,
    NO_PAD};

/*
  Comparing the UTF-8 representation automatically yields codepoint order,
  so we can just do a binary comparison. Note that
  my_strnxfrm_unicode_full_bin() chooses to transform to UCS before collation;
  this is purely for legacy reasons and is not needed here.
 */
static size_t my_strnxfrm_utf8mb4_0900_bin(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), uchar *dst, size_t dstlen,
    uint nweights MY_ATTRIBUTE((unused)), const uchar *src, size_t srclen,
    uint flags) {
  DBUG_ASSERT(src);

  size_t weight_len = std::min<size_t>(srclen, dstlen);
  memcpy(dst, src, weight_len);
  if (flags & MY_STRXFRM_PAD_TO_MAXLEN) {
    memset(dst + weight_len, 0, dstlen - weight_len);
    return dstlen;
  } else {
    return weight_len;
  }
}

static int my_strnncollsp_utf8mb4_0900_bin(const CHARSET_INFO *cs,
                                           const uchar *s, size_t slen,
                                           const uchar *t, size_t tlen) {
  return my_strnncoll_mb_bin(cs, s, slen, t, tlen, false);
}

static MY_COLLATION_HANDLER my_collation_utf8mb4_0900_bin_handler = {
    nullptr, /* init */
    nullptr,
    my_strnncoll_mb_bin,
    my_strnncollsp_utf8mb4_0900_bin,
    my_strnxfrm_utf8mb4_0900_bin,
    my_strnxfrmlen_simple,
    my_like_range_mb,
    my_wildcmp_mb_bin,
    my_strcasecmp_mb_bin,
    my_instr_mb,
    my_hash_sort_mb_bin,
    my_propagate_simple};

CHARSET_INFO my_charset_utf8mb4_0900_bin = {
    309,
    0,
    0,                                        // number
    MY_CS_UTF8MB4_UCA_FLAGS | MY_CS_BINSORT,  // state
    MY_UTF8MB4,                               // cs name
    MY_UTF8MB4 "_0900_bin",                   // name
    "",                                       // comment
    nullptr,                                  // tailoring
    nullptr,                                  // coll_param
    ctype_utf8,                               // ctype
    nullptr,                                  // to_lower
    nullptr,                                  // to_upper
    nullptr,                                  // sort_order
    nullptr,                                  // uca
    nullptr,                                  // tab_to_uni
    nullptr,                                  // tab_from_uni
    &my_unicase_unicode900,                   // caseinfo
    nullptr,                                  // state_map
    nullptr,                                  // ident_map
    1,                                        // strxfrm_multiply
    1,                                        // caseup_multiply
    1,                                        // casedn_multiply
    1,                                        // mbminlen
    4,                                        // mbmaxlen
    1,                                        // mbmaxlenlen
    0,                                        // min_sort_char
    0x10FFFF,                                 // max_sort_char
    ' ',                                      // pad char
    false,  // escape_with_backslash_is_dangerous
    1,      // levels_for_compare
    &my_charset_utf8mb4_handler,
    &my_collation_utf8mb4_0900_bin_handler,
    NO_PAD};
