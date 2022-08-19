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

/* File strings/ctype-czech.c for MySQL.

        This file implements the Czech sorting for the MySQL database
        server (www.mysql.com). Due to some complicated rules the
        Czech language has for sorting strings, a more complex
        solution was needed than the one-to-one conversion table. To
        note a few, here is an example of a Czech sorting sequence:

                co < hlaska < hláska < hlava < chlapec < krtek

        It because some of the rules are: double char 'ch' is sorted
        between 'h' and 'i'. Accented character 'á' (a with acute) is
        sorted after 'a' and before 'b', but only if the word is
        otherwise the same. However, because 's' is sorted before 'v'
        in hlava, the accentness of 'á' is overridden. There are many
        more rules.

        This file defines functions my_strxfrm and my_strcoll for
        C-like zero terminated strings and my_strnxfrm and my_strnncoll
        for strings where the length comes as an parameter. Also
        defined here you will find function my_like_range that returns
        index range strings for LIKE expression and the
        MY_STRXFRM_MULTIPLY set to value 4 -- this is the ratio the
        strings grows during my_strxfrm. The algorithm has four
        passes, that's why we need four times more space for expanded
        string.

        This file also contains the ISO-Latin-2 definitions of
        characters.

        Author: (c) 1997--1998 Jan Pazdziora, adelton@fi.muni.cz
        Jan Pazdziora has a shared copyright for this code

        The original of this file can also be found at
        http://www.fi.muni.cz/~adelton/l10n/

        Bug reports and suggestions are always welcome.
*/

/*
 * This comment is parsed by configure to create ctype.c,
 * so don't change it unless you know what you are doing.
 *
 * .configure. strxfrm_multiply_czech=4
 */

#include <string.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "template_utils.h"

/*
        These are four tables for four passes of the algorithm. Please see
        below for what are the "special values"
*/

static const unsigned char literal0[] =
    "\000\000\000\000\000\000\000\000\000\002\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\002\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\043\044\045\046\047\050"
    "\051\052\053\054\000\000\000\000\000\000\000\003\004\377\007\010\011\012"
    "\013\015\016\017\020\022\023\024\025\026\027\031\033\034\035\036\037\040"
    "\041\000\000\000\000\000\000\003\004\377\007\010\011\012\013\015\016\017"
    "\020\022\023\024\025\026\027\031\033\034\035\036\037\040\041\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\003"
    "\000\021\000\020\032\000\000\032\032\033\042\000\042\042\000\003\000\021"
    "\000\020\032\000\000\032\032\033\042\000\042\042\027\003\003\003\003\020"
    "\006\006\006\010\010\010\010\015\015\007\007\023\023\024\024\024\024\000"
    "\030\034\034\034\034\040\033\000\027\003\003\003\003\020\006\006\006\010"
    "\010\010\010\015\015\007\007\023\023\024\024\024\024\000\030\034\034\034"
    "\034\040\033\000";
static const unsigned char literal1[] =
    "\000\000\000\000\000\000\000\000\000\002\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\002\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\106\107\110\111\112\113"
    "\114\115\116\117\000\000\000\000\000\000\000\003\011\377\016\021\026\027"
    "\030\032\035\036\037\043\044\047\054\055\056\061\065\070\075\076\077\100"
    "\102\000\000\000\000\000\000\003\011\377\016\021\026\027\030\032\035\036"
    "\037\043\044\047\054\055\056\061\065\070\075\076\077\100\102\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\010"
    "\000\042\000\041\063\000\000\062\064\066\104\000\103\105\000\010\000\042"
    "\000\041\063\000\000\062\064\066\104\000\103\105\057\004\005\007\006\040"
    "\014\015\013\022\025\024\023\033\034\017\020\046\045\050\051\053\052\000"
    "\060\072\071\074\073\101\067\000\057\004\005\007\006\040\014\015\013\022"
    "\025\024\023\033\034\017\020\046\045\050\051\053\052\000\060\072\071\074"
    "\073\101\067\000";
static const unsigned char literal2[] =
    "\000\000\000\000\000\000\000\000\000\002\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\002\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\212\213\214\215\216\217"
    "\220\221\222\223\000\000\000\000\000\000\000\004\020\377\032\040\052\054"
    "\056\063\071\073\075\105\107\115\127\131\133\141\151\157\171\173\175\177"
    "\203\000\000\000\000\000\000\003\017\377\031\037\051\053\055\062\070\072"
    "\074\104\106\114\126\130\132\140\150\156\170\172\174\176\202\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\016"
    "\000\103\000\101\145\000\000\143\147\153\207\000\205\211\000\015\000\102"
    "\000\100\144\000\000\142\146\152\206\000\204\210\135\006\010\014\012\077"
    "\026\030\024\042\050\046\044\065\067\034\036\113\111\117\121\125\123\000"
    "\137\163\161\167\165\201\155\000\134\005\007\013\011\076\025\027\023\041"
    "\047\045\043\064\066\033\035\112\110\116\120\124\122\000\136\162\160\166"
    "\164\200\154\000";
static const unsigned char literal3[] =
    "\264\265\266\267\270\271\272\273\274\002\276\277\300\301\302\303\304\305"
    "\306\307\310\311\312\313\314\315\316\317\320\321\322\323\002\230\232\253"
    "\324\252\251\234\240\241\261\260\225\262\224\235\212\213\214\215\216\217"
    "\220\221\222\223\231\226\244\257\245\227\250\004\020\377\032\040\052\054"
    "\056\063\071\073\075\105\107\115\127\131\133\141\151\157\171\173\175\177"
    "\203\242\237\243\254\255\233\003\017\377\031\037\051\053\055\062\070\072"
    "\074\104\106\114\126\130\132\140\150\156\170\172\174\176\202\246\236\247"
    "\256\325\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303"
    "\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\326\016"
    "\327\103\330\101\145\331\332\143\147\153\207\333\205\211\334\015\335\102"
    "\336\100\144\337\340\142\146\152\206\341\204\210\135\006\010\014\012\077"
    "\026\030\024\042\050\046\044\065\067\034\036\113\111\117\121\125\123\263"
    "\137\163\161\167\165\201\155\342\134\005\007\013\011\076\025\027\023\041"
    "\047\045\043\064\066\033\035\112\110\116\120\124\122\343\136\162\160\166"
    "\164\200\154\344";

static const unsigned char *CZ_SORT_TABLE[] = {literal0, literal1, literal2,
                                               literal3};

/*
        These define the valuse for the double chars that need to be
        sorted as they were single characters -- in Czech these are
        'ch', 'Ch' and 'CH'.
*/

namespace {

struct wordvalue {
  const char *word;
  const uchar *outvalue;
};

}  // namespace

static struct wordvalue doubles[] = {
    {"ch", pointer_cast<const uchar *>("\014\031\057\057")},
    {"Ch", pointer_cast<const uchar *>("\014\031\060\060")},
    {"CH", pointer_cast<const uchar *>("\014\031\061\061")},
    {"c", pointer_cast<const uchar *>("\005\012\021\021")},
    {"C", pointer_cast<const uchar *>("\005\012\022\022")},
};

/*
        Unformal description of the algorithm:

        We walk the string left to right.

        The end of the string is either passed as parameter, or is
        *p == 0. This is hidden in the IS_END macro.

        In the first two passes, we compare word by word. So we make
        first and second pass on the first word, first and second pass
        on the second word, etc. If we come to the end of the string
        during the first pass, we need to jump to the last word of the
        second pass.

        End of pass is marked with value 1 on the output.

        For each character, we read it's value from the table.

        If the value is ignore (0), we go straight to the next character.

        If the value is space/end of word (2) and we are in the first
        or second pass, we skip all characters having value 0 -- 2 and
        switch the passwd.

        If it's the compose character (255), we check if the double
        exists behind it, find its value.

        We append 0 to the end.
---
        Neformální popis algoritmu:

        Procházíme øetìzec zleva doprava.

        Konec øetìzce je pøedán buï jako parametr, nebo je to *p == 0.
        Toto je o¹etøeno makrem IS_END.

        Pokud jsme do¹li na konec øetìzce pøi prùchodu 0, nejdeme na
        zaèátek, ale na uloŸenou pozici, protoŸe první a druhý prùchod
        bìŸí souèasnì.

        Konec vstupu (prùchodu) oznaèíme na výstupu hodnotou 1.

        Pro kaŸdý znak øetìzce naèteme hodnotu z tøídící tabulky.

        Jde-li o hodnotu ignorovat (0), skoèíme ihned na dal¹í znak..

        Jde-li o hodnotu konec slova (2) a je to prùchod 0 nebo 1,
        pøeskoèíme v¹echny dal¹í 0 -- 2 a prohodíme prùchody.

        Jde-li o kompozitní znak (255), otestujeme, zda následuje
        správný do dvojice, dohledáme správnou hodnotu.

        Na konci pøipojíme znak 0
 */

#define ADD_TO_RESULT(dest, len, totlen, value) \
  {                                             \
    if ((totlen) < (len)) {                     \
      dest[totlen++] = value;                   \
    }                                           \
  }
#define IS_END(p, src, len) \
  ((pointer_cast<const char *>(p) - pointer_cast<const char *>(src)) >= (len))

#define NEXT_CMP_VALUE(src, p, store, pass, value, len)           \
  while (1) {                                                     \
    if (IS_END(p, src, len)) {                                    \
      /* when we are at the end of string */                      \
      /* return either 0 for end of string */                     \
      /* or 1 for end of pass */                                  \
      value = 0;                                                  \
      if (pass != 3) {                                            \
        p = (pass++ == 0) ? store : src;                          \
        value = 1;                                                \
      }                                                           \
      break;                                                      \
    }                                                             \
    /* not at end of string */                                    \
    value = CZ_SORT_TABLE[pass][*p];                              \
    if (value == 0) {                                             \
      p++;                                                        \
      continue;                                                   \
    }               /* ignore value */                            \
    if (value == 2) /* space */                                   \
    {                                                             \
      const uchar *tmp;                                           \
      const uchar *runner = ++p;                                  \
      while (!(IS_END(runner, src, len)) &&                       \
             (CZ_SORT_TABLE[pass][*runner] == 2))                 \
        runner++; /* skip all spaces */                           \
      if (IS_END(runner, src, len)) p = runner;                   \
      if ((pass <= 2) && !(IS_END(runner, src, len))) p = runner; \
      if (IS_END(p, src, len)) continue;                          \
      /* we switch passes */                                      \
      if (pass > 1) break;                                        \
      tmp = p;                                                    \
      pass = 1 - pass;                                            \
      p = store;                                                  \
      store = tmp;                                                \
      break;                                                      \
    }                                                             \
    if (value == 255) {                                           \
      int i;                                                      \
      for (i = 0; i < (int)sizeof(doubles); i++) {                \
        const char *pattern = doubles[i].word;                    \
        const char *q = (const char *)p;                          \
        int j = 0;                                                \
        while (pattern[j]) {                                      \
          if (IS_END(q, src, len) || (*q != pattern[j])) break;   \
          j++;                                                    \
          q++;                                                    \
        }                                                         \
        if (!(pattern[j])) {                                      \
          value = (int)(doubles[i].outvalue[pass]);               \
          p = (const uchar *)q - 1;                               \
          break;                                                  \
        }                                                         \
      }                                                           \
    }                                                             \
    p++;                                                          \
    break;                                                        \
  }

/*
  Function strnncoll, actually strcoll, with Czech sorting, which expect
  the length of the strings being specified
*/

extern "C" {
static int my_strnncoll_czech(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                              const uchar *s1, size_t len1, const uchar *s2,
                              size_t len2, bool s2_is_prefix) {
  int v1, v2;
  const uchar *p1, *p2, *store1, *store2;
  int pass1 = 0, pass2 = 0;

  if (s2_is_prefix && len1 > len2) len1 = len2;

  p1 = s1;
  p2 = s2;
  store1 = s1;
  store2 = s2;

  do {
    int diff;
    NEXT_CMP_VALUE(s1, p1, store1, pass1, v1, (int)len1);
    NEXT_CMP_VALUE(s2, p2, store2, pass2, v2, (int)len2);
    if ((diff = v1 - v2)) return diff;
  } while (v1);
  return 0;
}

/*
  TODO: Fix this one to compare strings as they are done in ctype-simple1
*/

static int my_strnncollsp_czech(const CHARSET_INFO *cs, const uchar *s,
                                size_t slen, const uchar *t, size_t tlen) {
  for (; slen && s[slen - 1] == ' '; slen--)
    ;
  for (; tlen && t[tlen - 1] == ' '; tlen--)
    ;
  return my_strnncoll_czech(cs, s, slen, t, tlen, false);
}

/*
  Returns the number of bytes required for strnxfrm().
*/
static size_t my_strnxfrmlen_czech(
    const CHARSET_INFO *cs MY_ATTRIBUTE((unused)), size_t len) {
  return len * 4 + 4;
}

/*
  Function strnxfrm, actually strxfrm, with Czech sorting, which expect
  the length of the strings being specified
*/

static size_t my_strnxfrm_czech(const CHARSET_INFO *cs MY_ATTRIBUTE((unused)),
                                uchar *dest, size_t len,
                                uint nweights_arg MY_ATTRIBUTE((unused)),
                                const uchar *src, size_t srclen, uint flags) {
  int value;
  const uchar *p, *store;
  int pass = 0;
  size_t totlen = 0;
  p = src;
  store = src;

  if (!(flags & 0x0F)) /* All levels by default */
    flags |= 0x0F;

  do {
    int add = (1 << pass) & flags; /* If this level is needed */
    NEXT_CMP_VALUE(src, p, store, pass, value, (int)srclen);
    if (add) ADD_TO_RESULT(dest, len, totlen, value);
  } while (value);
  if ((flags & MY_STRXFRM_PAD_TO_MAXLEN) && len > totlen) {
    memset(dest + totlen, ' ', len - totlen);
    totlen = len;
  }
  return totlen;
}
}  // extern "C"

#undef IS_END

/*
        Neformální popis algoritmu:

        procházíme øetìzec zleva doprava
        konec øetìzce poznáme podle *p == 0
        pokud jsme do¹li na konec øetìzce pøi prùchodu 0, nejdeme na
                zaèátek, ale na uloŸenou pozici, protoŸe první a druhý
                prùchod bìŸí souèasnì
        konec vstupu (prùchodu) oznaèíme na výstupu hodnotou 1

        naèteme hodnotu z tøídící tabulky
        jde-li o hodnotu ignorovat (0), skoèíme na dal¹í prùchod
        jde-li o hodnotu konec slova (2) a je to prùchod 0 nebo 1,
                pøeskoèíme v¹echny dal¹í 0 -- 2 a prohodíme
                prùchody
        jde-li o kompozitní znak (255), otestujeme, zda následuje
                správný do dvojice, dohledáme správnou hodnotu

        na konci pøipojíme znak 0
 */

/*
** Calculate min_str and max_str that ranges a LIKE string.
** Arguments:
** ptr		Pointer to LIKE string.
** ptr_length	Length of LIKE string.
** escape	Escape character in LIKE.  (Normally '\').
**		All escape characters should be removed from min_str and max_str
** res_length   Length of min_str and max_str.
** min_str      Smallest case sensitive string that ranges LIKE.
**		Should be space padded to res_length.
** max_str	Largest case sensitive string that ranges LIKE.
**		Normally padded with the biggest character sort value.
**
** The function should return 0 if ok and 1 if the LIKE string can't be
** optimized !
*/

#define min_sort_char ' '
#define max_sort_char '9'

extern "C" {
static bool my_like_range_czech(const CHARSET_INFO *cs, const char *ptr,
                                size_t ptr_length, char escape, char w_one,
                                char w_many, size_t res_length, char *min_str,
                                char *max_str, size_t *min_length,
                                size_t *max_length) {
  uchar value;
  const char *end = ptr + ptr_length;
  char *min_org = min_str;
  char *min_end = min_str + res_length;

  for (; ptr != end && min_str != min_end; ptr++) {
    if (*ptr == w_one) /* '_' in SQL */
    {
      break;
    }
    if (*ptr == w_many) /* '%' in SQL */
    {
      break;
    }

    if (*ptr == escape && ptr + 1 != end) {
      ptr++;
    } /* Skip escape */

    value = CZ_SORT_TABLE[0][(int)(uchar)*ptr];

    if (value == 0) /* Ignore in the first pass */
    {
      continue;
    }
    if (value <= 2) /* End of pass or end of string */
    {
      break;
    }
    if (value == 255) /* Double char too compicated */
    {
      break;
    }

    *min_str++ = *max_str++ = *ptr;
  }

  if (cs->state & MY_CS_BINSORT)
    *min_length = (size_t)(min_str - min_org);
  else {
    /* 'a\0\0... is the smallest possible string */
    *min_length = res_length;
  }
  /* a\ff\ff... is the biggest possible string */
  *max_length = res_length;

  while (min_str != min_end) {
    *min_str++ = min_sort_char; /* Because of key compression */
    *max_str++ = max_sort_char;
  }
  return false;
}
}  // extern "C"

/*
 * File generated by cset
 * (C) Abandoned 1997 Zarko Mocnik <zarko.mocnik@dem.si>
 *
 * definition table reworked by Jaromir Dolecek <dolecek@ics.muni.cz>
 */

static const uchar ctype_czech[257] = {
    0,  32,  32,  32,  32,  32,  32,  32,  32,  32,  40,  40, 40, 40, 40, 32,
    32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32, 32, 32,
    32, 72,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 16, 16, 16, 16,
    16, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 16, 16, 16, 16, 16,
    16, 16,  129, 129, 129, 129, 129, 129, 1,   1,   1,   1,  1,  1,  1,  1,
    1,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  16, 16, 16, 16,
    16, 16,  130, 130, 130, 130, 130, 130, 2,   2,   2,   2,  2,  2,  2,  2,
    2,  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  16, 16, 16, 16,
    32, 32,  32,  32,  32,  32,  32,  32,  32,  40,  40,  40, 40, 40, 32, 32,
    32, 32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32, 32, 32,
    72, 1,   16,  1,   16,  1,   1,   16,  0,   0,   1,   1,  1,  1,  16, 1,
    1,  16,  2,   16,  2,   16,  2,   2,   16,  16,  2,   2,  2,  2,  16, 2,
    2,  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,  1,  1,  1,  1,
    1,  16,  1,   1,   1,   1,   1,   1,   16,  1,   1,   1,  1,  1,  1,  1,
    16, 2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  2,  2,  2,  2,
    2,  2,   2,   2,   2,   2,   2,   2,   16,  2,   2,   2,  2,  2,  2,  2,
    16,
};

static const uchar to_lower_czech[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106,
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 177, 161, 179, 163, 181,
    182, 166, 167, 168, 185, 186, 187, 188, 173, 190, 191, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 224, 225, 226,
    227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 208, 241,
    242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255,
};

static const uchar to_upper_czech[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  65,  66,  67,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 160, 178, 162,
    180, 164, 165, 183, 184, 169, 170, 171, 172, 189, 174, 175, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192,
    193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    240, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222,
    255,
};

static const uchar sort_order_czech[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  71,  72,  76,  78,  83,  84,  85,  86,  90,
    91,  92,  96,  97,  100, 105, 106, 107, 110, 114, 117, 122, 123, 124, 125,
    127, 131, 132, 133, 134, 135, 136, 65,  71,  72,  76,  78,  83,  84,  85,
    86,  90,  91,  92,  96,  97,  100, 105, 106, 107, 110, 114, 117, 122, 123,
    124, 125, 127, 137, 138, 139, 140, 0,   1,   2,   3,   4,   5,   6,   7,
    8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  255, 66,  255, 93,  255, 94,
    111, 255, 255, 255, 112, 113, 115, 128, 255, 129, 130, 255, 66,  255, 93,
    255, 94,  111, 255, 255, 112, 113, 115, 128, 255, 129, 130, 108, 67,  68,
    69,  70,  95,  73,  75,  74,  79,  81,  82,  80,  89,  87,  77,  255, 98,
    99,  101, 102, 103, 104, 255, 109, 119, 118, 120, 121, 126, 116, 255, 108,
    67,  68,  69,  70,  95,  73,  75,  74,  79,  81,  82,  80,  89,  88,  77,
    255, 98,  99,  101, 102, 103, 104, 255, 109, 119, 118, 120, 121, 126, 116,
    255,
};

static uint16 tab_8859_2_uni[256] = {
    0,      0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,
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
    0x007E, 0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0x00A0, 0x0104,
    0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7, 0x00A8, 0x0160, 0x015E,
    0x0164, 0x0179, 0x00AD, 0x017D, 0x017B, 0x00B0, 0x0105, 0x02DB, 0x0142,
    0x00B4, 0x013E, 0x015B, 0x02C7, 0x00B8, 0x0161, 0x015F, 0x0165, 0x017A,
    0x02DD, 0x017E, 0x017C, 0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139,
    0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE,
    0x010E, 0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF, 0x0155,
    0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9,
    0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F, 0x0111, 0x0144, 0x0148,
    0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171,
    0x00FC, 0x00FD, 0x0163, 0x02D9};

/* 0000-00FD , 254 chars */
static const uchar tab_uni_8859_2_plane00[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
    0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0xA4, 0x00, 0x00, 0xA7,
    0xA8, 0x00, 0x00, 0x00, 0x00, 0xAD, 0x00, 0x00, 0xB0, 0x00, 0x00, 0x00,
    0xB4, 0x00, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xC1, 0xC2, 0x00, 0xC4, 0x00, 0x00, 0xC7, 0x00, 0xC9, 0x00, 0xCB,
    0x00, 0xCD, 0xCE, 0x00, 0x00, 0x00, 0x00, 0xD3, 0xD4, 0x00, 0xD6, 0xD7,
    0x00, 0x00, 0xDA, 0x00, 0xDC, 0xDD, 0x00, 0xDF, 0x00, 0xE1, 0xE2, 0x00,
    0xE4, 0x00, 0x00, 0xE7, 0x00, 0xE9, 0x00, 0xEB, 0x00, 0xED, 0xEE, 0x00,
    0x00, 0x00, 0x00, 0xF3, 0xF4, 0x00, 0xF6, 0xF7, 0x00, 0x00, 0xFA, 0x00,
    0xFC, 0xFD};

/* 0102-017E , 125 chars */
static const uchar tab_uni_8859_2_plane01[] = {
    0xC3, 0xE3, 0xA1, 0xB1, 0xC6, 0xE6, 0x00, 0x00, 0x00, 0x00, 0xC8, 0xE8,
    0xCF, 0xEF, 0xD0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCA, 0xEA,
    0xCC, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0xE5, 0x00, 0x00, 0xA5,
    0xB5, 0x00, 0x00, 0xA3, 0xB3, 0xD1, 0xF1, 0x00, 0x00, 0xD2, 0xF2, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD5, 0xF5, 0x00, 0x00, 0xC0, 0xE0,
    0x00, 0x00, 0xD8, 0xF8, 0xA6, 0xB6, 0x00, 0x00, 0xAA, 0xBA, 0xA9, 0xB9,
    0xDE, 0xFE, 0xAB, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xD9, 0xF9, 0xDB, 0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC,
    0xBC, 0xAF, 0xBF, 0xAE, 0xBE};

/* 02C7-02DD ,  23 chars */
static const uchar tab_uni_8859_2_plane02[] = {
    0xB7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xFF, 0x00, 0xB2, 0x00, 0xBD};

static MY_UNI_IDX idx_uni_8859_2[] = {{0x0000, 0x00FD, tab_uni_8859_2_plane00},
                                      {0x0102, 0x017E, tab_uni_8859_2_plane01},
                                      {0x02C7, 0x02DD, tab_uni_8859_2_plane02},
                                      {0, 0, nullptr}};

static MY_COLLATION_HANDLER my_collation_latin2_czech_ci_handler = {
    nullptr, /* init */
    nullptr,
    my_strnncoll_czech,
    my_strnncollsp_czech,
    my_strnxfrm_czech,
    my_strnxfrmlen_czech,
    my_like_range_czech,
    my_wildcmp_bin,
    my_strcasecmp_8bit,
    my_instr_simple,
    my_hash_sort_simple,
    my_propagate_simple};

CHARSET_INFO my_charset_latin2_czech_ci = {
    2,
    0,
    0,                                              /* number    */
    MY_CS_COMPILED | MY_CS_STRNXFRM | MY_CS_CSSORT, /* state     */
    "latin2",                                       /* cs name   */
    "latin2_czech_cs",                              /* name      */
    "ISO 8859-2 Central European",                  /* comment   */
    nullptr,                                        /* tailoring */
    nullptr,                                        /* coll_param */
    ctype_czech,
    to_lower_czech,
    to_upper_czech,
    sort_order_czech,
    nullptr,             /* uca          */
    tab_8859_2_uni,      /* tab_to_uni   */
    idx_uni_8859_2,      /* tab_from_uni */
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
    0,                   /* max_sort_char */
    ' ',                 /* pad char      */
    false,               /* escape_with_backslash_is_dangerous */
    4,                   /* levels_for_compare */
    &my_charset_8bit_handler,
    &my_collation_latin2_czech_ci_handler,
    PAD_SPACE};
