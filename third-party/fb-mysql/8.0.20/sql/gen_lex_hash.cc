/*
   Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  @details
@verbatim
Algorithm implemented from a description in
"The Art of Computer Programming" by Donald E. Knuth
Volume 3 "Sorting and searching",
chapter 6.3 "Digital searching"

as illustration of data structures, imagine next table:

static SYMBOL symbols[] = {
  { "ADD",              SYM(ADD),0,0},
  { "AND",              SYM(AND),0,0},
  { "DAY",              SYM(DAY_SYM),0,0},
};

for this structure, presented program generate next searching-structure:

+-----------+-+-+-+
|       len |1|2|3|
+-----------+-+-+-+
|first_char |0|0|a|
|last_char  |0|0|d|
|link       |0|0|+|
                 |
                 V
       +----------+-+-+-+--+
       |    1 char|a|b|c|d |
       +----------+-+-+-+--+
       |first_char|d|0|0|0 |
       |last_char |n|0|0|-1|
       |link      |+|0|0|+ |
                   |     |
                   |     V
                   |  symbols[2] ( "DAY" )
                   V
+----------+--+-+-+-+-+-+-+-+-+-+--+
|    2 char|d |e|f|j|h|i|j|k|l|m|n |
+----------+--+-+-+-+-+-+-+-+-+-+--+
|first_char|0 |0|0|0|0|0|0|0|0|0|0 |
|last_char |-1|0|0|0|0|0|0|0|0|0|-1|
|link      |+ |0|0|0|0|0|0|0|0|0|+ |
            |                    |
            V                    V
         symbols[0] ( "ADD" )  symbols[1] ( "AND" )

for optimization, link is the 16-bit index in 'symbols'
or search-array..

So, we can read full search-structure as 32-bit word
@endverbatim

@todo
    use instead to_upper_lex, special array
    (substitute chars) without skip codes..
@todo
    try use reverse order of comparing..

*/

#define NO_YACC_SYMBOLS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <set>
#include <utility>

#include "my_inttypes.h"
#include "sql/lex.h"
#include "sql/lex_symbol.h"
#include "template_utils.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

static bool check_duplicates(uint group_mask);

struct hash_lex_struct {
  int first_char;
  char last_char;
  // union value is undefined if first_char == 0
  union {
    hash_lex_struct *char_tails;  // if first_char > 0
    int iresult;                  // if first_char == -1
  };
  int ithis;

  void destroy() {
    if (first_char <= 0) return;
    for (int i = 0, size = static_cast<uchar>(last_char) - first_char + 1;
         i < size; i++)
      char_tails[i].destroy();
    free(char_tails);
  }
};

class hash_map_info {
 public:
  hash_lex_struct *root_by_len;
  int max_len;

  char *hash_map;
  int size_hash_map;

  hash_map_info()
      : root_by_len(nullptr), max_len(0), hash_map(nullptr), size_hash_map(0) {}

  ~hash_map_info() {
    for (int i = 0; i < max_len; i++) root_by_len[i].destroy();
    free(root_by_len);
    free(hash_map);
  }

  hash_lex_struct *get_hash_struct_by_len(int len);
  void insert_symbols(int group_mask);
  void add_structs_to_map(hash_lex_struct *st, int len);
  void add_struct_to_map(hash_lex_struct *st);
  void set_links(hash_lex_struct *st, int len);
  bool print_hash_map(const char *name, uint group_mask);
};

hash_lex_struct *hash_map_info::get_hash_struct_by_len(int len) {
  if (max_len < len) {
    root_by_len = (hash_lex_struct *)realloc((char *)root_by_len,
                                             sizeof(hash_lex_struct) * len);
    hash_lex_struct *cur, *end = root_by_len + len;
    for (cur = root_by_len + max_len; cur < end; cur++) cur->first_char = 0;
    max_len = len;
  }
  return root_by_len + len - 1;
}

static void insert_into_hash(hash_lex_struct *root, const char *name,
                             int len_from_begin, int index) {
  hash_lex_struct *end, *cur, *tails;

  if (!root->first_char) {
    root->first_char = -1;
    root->iresult = index;
    return;
  }

  if (root->first_char == -1) {
    int index2 = root->iresult;
    const char *name2 = symbols[index2].name + len_from_begin;
    root->first_char = (int)(uchar)name2[0];
    root->last_char = (char)root->first_char;
    tails = (hash_lex_struct *)malloc(sizeof(hash_lex_struct));
    root->char_tails = tails;
    tails->first_char = -1;
    tails->iresult = index2;
  }

  size_t real_size = (root->last_char - root->first_char + 1);

  if (root->first_char > (*name)) {
    size_t new_size = root->last_char - (*name) + 1;
    if (new_size < real_size) printf("error!!!!\n");
    tails = root->char_tails;
    tails = (hash_lex_struct *)realloc((char *)tails,
                                       sizeof(hash_lex_struct) * new_size);
    root->char_tails = tails;
    memmove(tails + (new_size - real_size), tails,
            real_size * sizeof(hash_lex_struct));
    end = tails + new_size - real_size;
    for (cur = tails; cur < end; cur++) cur->first_char = 0;
    root->first_char = (int)(uchar)*name;
  }

  if (root->last_char < (*name)) {
    size_t new_size = (*name) - root->first_char + 1;
    if (new_size < real_size) printf("error!!!!\n");
    tails = root->char_tails;
    tails = (hash_lex_struct *)realloc((char *)tails,
                                       sizeof(hash_lex_struct) * new_size);
    root->char_tails = tails;
    end = tails + new_size;
    for (cur = tails + real_size; cur < end; cur++) cur->first_char = 0;
    root->last_char = (*name);
  }

  insert_into_hash(root->char_tails + (*name) - root->first_char, name + 1,
                   len_from_begin + 1, index);
}

void hash_map_info::insert_symbols(int group_mask) {
  size_t i = 0;
  const SYMBOL *cur;
  for (cur = symbols; i < array_elements(symbols); cur++, i++) {
    if (!(cur->group & group_mask)) continue;
    hash_lex_struct *root = get_hash_struct_by_len(cur->length);
    insert_into_hash(root, cur->name, 0, i);
  }
}

void hash_map_info::add_struct_to_map(hash_lex_struct *st) {
  st->ithis = size_hash_map / 4;
  size_hash_map += 4;
  hash_map = (char *)realloc(hash_map, size_hash_map);
  hash_map[size_hash_map - 4] =
      (char)(st->first_char == -1 ? 0 : st->first_char);
  hash_map[size_hash_map - 3] =
      (char)(st->first_char == -1 || st->first_char == 0 ? 0 : st->last_char);
  if (st->first_char == -1) {
    hash_map[size_hash_map - 2] = ((unsigned int)(int16)st->iresult) & 255;
    hash_map[size_hash_map - 1] = ((unsigned int)(int16)st->iresult) >> 8;
  } else if (st->first_char == 0) {
    hash_map[size_hash_map - 2] =
        ((unsigned int)(int16)array_elements(symbols)) & 255;
    hash_map[size_hash_map - 1] =
        ((unsigned int)(int16)array_elements(symbols)) >> 8;
  }
}

void hash_map_info::add_structs_to_map(hash_lex_struct *st, int len) {
  hash_lex_struct *cur, *end = st + len;
  for (cur = st; cur < end; cur++) add_struct_to_map(cur);
  for (cur = st; cur < end; cur++) {
    if (cur->first_char && cur->first_char != -1)
      add_structs_to_map(cur->char_tails, cur->last_char - cur->first_char + 1);
  }
}

void hash_map_info::set_links(hash_lex_struct *st, int len) {
  hash_lex_struct *cur, *end = st + len;
  for (cur = st; cur < end; cur++) {
    if (cur->first_char != 0 && cur->first_char != -1) {
      int ilink = cur->char_tails->ithis;
      hash_map[cur->ithis * 4 + 2] = ilink % 256;
      hash_map[cur->ithis * 4 + 3] = ilink / 256;
      set_links(cur->char_tails, cur->last_char - cur->first_char + 1);
    }
  }
}

bool hash_map_info::print_hash_map(const char *name, uint group_mask) {
  if (check_duplicates(group_mask)) return true;
  insert_symbols(group_mask);
  add_structs_to_map(root_by_len, max_len);
  set_links(root_by_len, max_len);

  printf("static const unsigned char %s_map[%d]= {\n", name, size_hash_map);

  char *cur;
  int i;
  for (i = 0, cur = hash_map; i < size_hash_map; i++, cur++) {
    switch (i % 4) {
      case 0:
      case 1:
        if (!*cur)
          printf("0,   ");
        else
          printf("\'%c\', ", *cur);
        break;
      case 2:
        printf("%u, ", (uint)(uchar)*cur);
        break;
      case 3:
        printf("%u,\n", (uint)(uchar)*cur);
        break;
    }
  }
  printf("};\n\n");
  printf("const unsigned int %s_max_len=%d;\n", name, max_len);
  return false;
}

bool check_duplicates(uint group_mask) {
  std::set<const char *> names;

  size_t i = 0;
  const SYMBOL *cur;
  for (cur = symbols; i < array_elements(symbols); cur++, i++) {
    if (!(cur->group & group_mask)) continue;
    if (!names.insert(cur->name).second) {
      const char *err_tmpl =
          "\ngen_lex_hash fatal error : "
          "Unfortunately gen_lex_hash can not generate a hash,\n since "
          "your lex.h has duplicate definition for a symbol \"%s\"\n\n";
      printf(err_tmpl, cur->name);
      fprintf(stderr, err_tmpl, cur->name);
      return true;
    }
  }
  return false;
}

int main(int, char **) {
  /* Broken up to indicate that it's not advice to you, gentle reader. */
  printf(
      "/*\n\n  Do "
      "not "
      "edit "
      "this "
      "file "
      "directly!\n\n*/\n");

  puts(ORACLE_GPL_COPYRIGHT_NOTICE("2000"));

  /* Broken up to indicate that it's not advice to you, gentle reader. */
  printf(
      "/* Do "
      "not "
      "edit "
      "this "
      "file!  This is generated by "
      "gen_lex_hash.cc\nthat seeks for a perfect hash function */\n\n");

  /* Print sql_keywords_and_funcs_map[] and sql_keyword_and_funcs_max_len: */
  if (hash_map_info().print_hash_map("sql_keywords_and_funcs", SG_MAIN_PARSER))
    exit(1);

  printf("\n");

  /* Print sql_keywords_map[] and sql_keywords_max_len values: */
  if (hash_map_info().print_hash_map("sql_keywords",
                                     SG_KEYWORDS | SG_HINTABLE_KEYWORDS))
    exit(1);

  printf("\n");

  /* Print hint_keywords_map[] and hint_keywords_max_len values: */
  if (hash_map_info().print_hash_map("hint_keywords", SG_HINTS)) exit(1);

  exit(0);
}
