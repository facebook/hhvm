/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

/* This header file contains type declarations used by UCA code. */

#ifndef STR_UCA_TYPE_H
#define STR_UCA_TYPE_H

#include <vector>

#include "my_inttypes.h"

/*
  So far we have only Croatian collation needs to reorder Latin and
  Cyrillic group of characters. May add more in future.
*/
#define UCA_MAX_CHAR_GRP 4
enum enum_uca_ver { UCA_V400, UCA_V520, UCA_V900 };

enum enum_char_grp {
  CHARGRP_NONE,
  CHARGRP_CORE,
  CHARGRP_LATIN,
  CHARGRP_CYRILLIC,
  CHARGRP_ARAB,
  CHARGRP_KANA,
  CHARGRP_OTHERS
};

struct Weight_boundary {
  uint16 begin;
  uint16 end;
};

struct Reorder_wt_rec {
  struct Weight_boundary old_wt_bdy;
  struct Weight_boundary new_wt_bdy;
};

struct Reorder_param {
  enum enum_char_grp reorder_grp[UCA_MAX_CHAR_GRP];
  struct Reorder_wt_rec wt_rec[2 * UCA_MAX_CHAR_GRP];
  int wt_rec_num;
  uint16 max_weight;
};

enum enum_case_first { CASE_FIRST_OFF, CASE_FIRST_UPPER, CASE_FIRST_LOWER };

struct Coll_param {
  struct Reorder_param *reorder_param;
  bool norm_enabled;  // false = normalization off, default;
                      // true = on
  enum enum_case_first case_first;
};

/*
  NOTE: If you change MY_UCA_MAX_CONTRACTION, be sure to update the comment on
  MY_UCA_CNT_MID1 in strings/uca_data.h, as it might cause us to run out of
  bits in a byte flag.
*/
#define MY_UCA_MAX_CONTRACTION 6
#define MY_UCA_MAX_WEIGHT_SIZE 25
#define MY_UCA_WEIGHT_LEVELS 1

/*
  We store all the contractions in a trie, indexed on the codepoints they
  consist of. The trie is organized as:
  1. Each node stores one code point (ch) of contraction, and a list of nodes
     (child_nodes) store all possible following code points.
  2. The vector in MY_UCA_INFO stores a list of nodes which store the first
     code points of all contractions.
  3. Each node has a boolean value (is_contraction_tail) which shows
     whether the code point stored in the node is the end of a contraction.
     This is necessary because even if one code point is the end of a
     contraction, there might be longer contraction contains all the
     code points in the path (e.g., for Hungarian, both 'DZ' and 'DZS' are
     contractions).
  4. A contraction is formed by all the code points in the path until the
     end of the contraction.
  5. If it is the end of a contraction (is_contraction_tail == true), the
     weight of this contraction is stored in array weight.
  6. If it is the end of a contraction (is_contraction_tail == true),
     with_context shows whether it is common contraction (with_context ==
     false), or previous context contraction (with_context == true).
  7. If it is the end of a contraction (is_contraction_tail == true),
     contraction_len shows how many code points this contraction consists of.
*/
struct MY_CONTRACTION {
  my_wc_t ch;
  // Lists of following nodes.
  std::vector<MY_CONTRACTION> child_nodes;
  std::vector<MY_CONTRACTION> child_nodes_context;

  // weight and with_context are only useful when is_contraction_tail is true.
  uint16 weight[MY_UCA_MAX_WEIGHT_SIZE]; /* Its weight string, 0-terminated */
  bool is_contraction_tail;
  size_t contraction_len;
};

struct MY_UCA_INFO {
  enum enum_uca_ver version;

  // Collation weights.
  my_wc_t maxchar;
  uchar *lengths;
  uint16 **weights;
  bool have_contractions;
  std::vector<MY_CONTRACTION> *contraction_nodes;
  /*
    contraction_flags is only used when a collation has contraction rule.
    UCA collation supports at least 65535 characters, but only a few of
    them can be part of contraction, it is huge waste of time to find out
    whether one character is in contraction list for every character.
    contraction_flags points to memory which is allocated when a collation
    has contraction rule. For a character in contraction, its corresponding
    byte (contraction_flags[ch & 0x1000]) will be set to a certain value
    according to the position (head, tail or middle) of this character in
    contraction. This byte will be used to quick check whether one character
    can be part of contraction.
  */
  char *contraction_flags;

  /* Logical positions */
  my_wc_t first_non_ignorable;
  my_wc_t last_non_ignorable;
  my_wc_t first_primary_ignorable;
  my_wc_t last_primary_ignorable;
  my_wc_t first_secondary_ignorable;
  my_wc_t last_secondary_ignorable;
  my_wc_t first_tertiary_ignorable;
  my_wc_t last_tertiary_ignorable;
  my_wc_t first_trailing;
  my_wc_t last_trailing;
  my_wc_t first_variable;
  my_wc_t last_variable;
  /*
    extra_ce_pri_base, extra_ce_sec_base and extra_ce_ter_base are only used for
    the UCA collations whose UCA version is not smaller than UCA_V900. For why
    we need this extra CE, please see the comment in my_char_weight_put_900()
    and apply_primary_shift_900().

    The value of these three variables is set by the definition of my_uca_v900.
    The value of extra_ce_pri_base is usually 0x54A4 (which is the maximum
    regular weight value pluses one, 0x54A3 + 1 = 0x54A4). But for the Chinese
    collation, the extra_ce_pri_base needs to change. This is because 0x54A4 has
    been occupied to do reordering. There might be weight conflict if we still
    use 0x54A4. Please also see the comment on modify_all_zh_pages().
   */
  uint16 extra_ce_pri_base;  // Primary weight of extra CE
  uint16 extra_ce_sec_base;  // Secondary weight of extra CE
  uint16 extra_ce_ter_base;  // Tertiary weight of extra CE
};

#define MY_UCA_CNT_FLAG_SIZE 4096
#define MY_UCA_CNT_FLAG_MASK 4095

/** Whether the given character can be the first in any contraction. */
#define MY_UCA_CNT_HEAD 1

/** Whether the given character can be the last in any contraction. */
#define MY_UCA_CNT_TAIL 2

/**
 Whether the given character can be the second in any contraction.

 Also defined implicitly through shifting MY_UCA_CNT_MID1:

 \#define MY_UCA_CNT_MID2  8
 \#define MY_UCA_CNT_MID3  16
 \#define MY_UCA_CNT_MID4  32

 There's no need for MY_UCA_CNT_MID5 (which would cause us to run out of
 bits) since MY_UCA_MAX_CONTRACTION is 6 (so head, four in the middle,
 and then tail).
*/
#define MY_UCA_CNT_MID1 4

/**
 Whether the given character is the first part of a context-sensitive
 contraction. Context-sensitive contractions are like normal contractions,
 except that for performance reasons, they trigger on the _last_ character
 instead of the first. The case given in Unicode TR35 is that in some
 scripts (such as katakana in Japanese), "a-" should sort as "aa"
 (except on the tertiary level), "e-" should sort as "ee" and so on.
 However, adding regular contractions on "a" and "e" would cause undue
 performance loss, so instead, we add a special "context-sensitive"
 contraction on "-" that then looks at the _previous_ character.

 We don't support context-sensitive contractions longer than two characters
 at the moment, since none exist in CLDR. Thus, there is no
 MY_UCA_PREVIOUS_CONTEXT_MID1 and so on.
*/
#define MY_UCA_PREVIOUS_CONTEXT_HEAD 64

/** Similar to MY_UCA_PREVIOUS_CONTEXT_HEAD, just for the tail. */
#define MY_UCA_PREVIOUS_CONTEXT_TAIL 128

#define MY_UCA_PSHIFT 8

/**
  Check if a code point can be contraction head

  @param flags    Pointer to UCA contraction flag data
  @param wc       Code point

  @retval   0 - cannot be contraction head
  @retval   1 - can be contraction head
*/

inline bool my_uca_can_be_contraction_head(const char *flags, my_wc_t wc) {
  return flags[wc & MY_UCA_CNT_FLAG_MASK] & MY_UCA_CNT_HEAD;
}

/**
  Check if a code point can be contraction tail

  @param flags    Pointer to UCA contraction flag data
  @param wc       Code point

  @retval   0 - cannot be contraction tail
  @retval   1 - can be contraction tail
*/

inline bool my_uca_can_be_contraction_tail(const char *flags, my_wc_t wc) {
  return flags[wc & MY_UCA_CNT_FLAG_MASK] & MY_UCA_CNT_TAIL;
}

const uint16 *my_uca_contraction2_weight(
    const std::vector<MY_CONTRACTION> *cont_nodes, my_wc_t wc1, my_wc_t wc2);
#endif
