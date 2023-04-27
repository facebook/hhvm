/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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
  This file is used to dump DUCET 9.0.0 to table we use in MySQL collations.
  It is created on the basis of uca-dump.cc file for 5.2.0. It is changed to
  dump all 3 levels into one table.
  How to use:
    1. g++ uca9-dump.cc -o uca9dump
    2. uca9dump ducet --in_file=/path/to/allkeys.txt --out_file=/path/to/youfile

  This can also be used to dump weight table of Japanese Han characters.
  How to use:
    1. Copy the line of Han characters in CLDR file ja.xml to a seperate file,
       e.g. ja_han.txt.
    2. Make sure the file is saved in UTF-8 (use 'file' command to check), or
       use iconv to convert.
    3. uca9dump ja --in_file=/path/to/ja_han.txt --out_file=/path/to/yourfile

  This can also be used to dump the weight tables of Chinese Han characters.
  How to use:
    1. Make sure you have uca900_weights and all the weight tables in strings/
       uca900_data.h. If no, please refer to above comments about how to
       generate those tables.
    2. Copy the lines of Han characters in CLDR file zh.xml to a seperate
       file, e.g. zh_han.txt.
    3. Make sure the file is saved in UTF-8 (use 'file' command to check), or
       use iconv to convert.
    4. Remove all the comments ("# XX") at the end of each line. And remove
       all the "<*" at the beginning of each line. "<*" means for all the
       characters in this line, each character should be greater than its
       previous character. We'll do this with uca9dump. And also remove the
       lines like '\uFDD0A #index A'. These lines mark the beginning of each
       group of characters which have similar pronunciation. They don't
       affect how we arrange the weight of the characters.
    5. Join all the lines into one.
    6. uca9dump zh --in_file=/path/to/zh_han.txt --out_file=/path/to/yourfile
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <set>

#include "m_string.h"
#include "my_compiler.h"
#include "strings/mb_wc.h"
#include "strings/uca900_data.h"  // uca900_weights[]

typedef unsigned char uchar;
typedef unsigned short uint16;
typedef unsigned int uint;
typedef unsigned long my_wc_t;

#define MY_UCA_MAXWEIGHT_TO_PARSE 64
#define MY_UCA_MAXCE_TO_PARSE 18
#define MY_UCA_MAXWEIGHT_TO_DUMP 24
#define MY_UCA_MAXCE_TO_DUMP 8
#define MY_UCA_VERSION_SIZE 32
#define MY_UCA_CE_SIZE 3
#define MY_UCA_MAX_CONTRACTION 6

#define MY_UCA_MAXCHAR (0x10FFFF + 1)
#define MY_UCA_CHARS_PER_PAGE 256
#define MY_UCA_PSHIFT 8
#define MY_UCA_NPAGES MY_UCA_MAXCHAR / MY_UCA_CHARS_PER_PAGE

struct MY_UCA_ITEM {
  int num_of_ce; /* Number of collation elements */
  uint16 weight[MY_UCA_MAXWEIGHT_TO_DUMP + 1];
  /* +1 for trailing num_of_ce */
};

struct MY_UCA {
  char version[MY_UCA_VERSION_SIZE];
  MY_UCA_ITEM item[MY_UCA_MAXCHAR];  // Weight info of all characters
};

static int load_uca_file(MY_UCA *uca, int maxchar, int *pageloaded,
                         FILE *infile) {
  char str[512];
  int out_of_range_chars = 0;

  for (int lineno = 0; fgets(str, sizeof(str), infile); lineno++) {
    /* Skip comment lines */
    if (*str == '\r' || *str == '\n' || *str == '#') continue;

    /* Detect version */
    if (*str == '@') {
      if (!strncmp(str, "@version ", 9)) {
        const char *value;
        if (strtok(str, " \r\n\t") && (value = strtok(nullptr, " \r\n\t")))
          snprintf(uca->version, MY_UCA_VERSION_SIZE, "%s", value);
      }
      continue;
    }

    int code;
    /* Skip big characters */
    if ((code = strtol(str, nullptr, 16)) > maxchar) {
      out_of_range_chars++;
      continue;
    }

    char *comment;
    if (!(comment = strchr(str, '#'))) {
      fprintf(stderr, "Warning: could not parse line #%d:\n'%s'\n", lineno,
              str);
      continue;
    }
    *comment = '\0';

    char *weight;
    if ((weight = strchr(str, ';'))) {
      *weight++ = '\0';
      weight += strspn(weight, " ");
    } else {
      fprintf(stderr, "Warning: could not parse line #%d:\n%s\n", lineno, str);
      continue;
    }

    char *s;
    int codenum;
    for (codenum = 0, s = strtok(str, " \t"); s;
         codenum++, s = strtok(nullptr, " \t")) {
      /* Meet a contraction. To handle in the future. */
      if (codenum >= 1) {
        codenum++;
        break;
      }
    }

    MY_UCA_ITEM *item = nullptr;
    if (codenum > 1) {
      /* Contractions we don't support. */
      continue;
    } else {
      item = &uca->item[code];
    }

    /*
      Split weight string into separate weights

      "[p1.s1.t1.q1][p2.s2.t2.q2][p3.s3.t3.q3]" ->

      "p1.s1.t1.q1" "p2.s2.t2.q2" "p3.s3.t3.q3"
    */
    item->num_of_ce = 0;
    s = strtok(weight, " []");
    char *weights[MY_UCA_MAXWEIGHT_TO_PARSE];
    while (s) {
      if (item->num_of_ce >= MY_UCA_MAXCE_TO_PARSE) {
        fprintf(stderr, "Line #%d has more than %d collation elements\n",
                lineno, MY_UCA_MAXCE_TO_PARSE);
        fprintf(stderr, "Can't continue.\n");
        exit(1);
      }
      weights[item->num_of_ce] = s;
      s = strtok(nullptr, " []");
      item->num_of_ce++;
    }

    for (int i = 0; i < item->num_of_ce; i++) {
      /*
        The longest collation element in DUCET is assigned to 0xFDFA. It
        has 18 collation elements. The second longest is 8. Because 8
        collation elements is enough to distict 0xFDFA from other
        characters, we skip the extra weights and only use 8 here.
      */
      if (i >= MY_UCA_MAXCE_TO_DUMP) {
        fprintf(stderr,
                "Warning: at line %d: character %04X has"
                " more than %d collation elements (%d). "
                "Skipping the extra weights.\n",
                lineno, code, MY_UCA_MAXCE_TO_DUMP, item->num_of_ce);
        item->num_of_ce = MY_UCA_MAXCE_TO_DUMP;
        break;
      }

      int weight_of_ce = 0;
      for (s = weights[i]; *s;) {
        char *endptr;
        int part = strtol(s + 1, &endptr, 16);
        if (i < MY_UCA_MAXCE_TO_DUMP) {
          item->weight[i * MY_UCA_CE_SIZE + weight_of_ce] = part;
        } else {
          fprintf(stderr, "Too many weights (%d) at line %d\n", i, lineno);
          exit(1);
        }
        s = endptr;
        weight_of_ce++;
      }
    }
    /* Mark that a character from this page was loaded */
    pageloaded[code >> MY_UCA_PSHIFT]++;
  }

  if (out_of_range_chars)
    fprintf(stderr, "%d out-of-range characters skipped\n", out_of_range_chars);

  return 0;
}

#define HANGUL_JAMO_MAX_LENGTH 3

static int my_decompose_hangul_syllable(my_wc_t syllable, my_wc_t *jamo) {
  if (syllable < 0xAC00 || syllable > 0xD7AF) return 0;
  constexpr int syllable_base = 0xAC00;
  constexpr int leadingjamo_base = 0x1100;
  constexpr int voweljamo_base = 0x1161;
  constexpr int trailingjamo_base = 0x11A7;
  constexpr int voweljamo_cnt = 21;
  constexpr int trailingjamo_cnt = 28;
  int syllable_index = syllable - syllable_base;
  int v_t_combination = voweljamo_cnt * trailingjamo_cnt;
  int leadingjamo_index = syllable_index / v_t_combination;
  int voweljamo_index = (syllable_index % v_t_combination) / trailingjamo_cnt;
  int trailingjamo_index = syllable_index % trailingjamo_cnt;
  jamo[0] = leadingjamo_base + leadingjamo_index;
  jamo[1] = voweljamo_base + voweljamo_index;
  jamo[2] = trailingjamo_index ? (trailingjamo_base + trailingjamo_index) : 0;
  return trailingjamo_index ? 3 : 2;
}

void my_put_jamo_weights(const my_wc_t *hangul_jamo, int jamo_cnt,
                         MY_UCA_ITEM *item, const MY_UCA *uca) {
  for (int jamoind = 0; jamoind < jamo_cnt; jamoind++) {
    uint16 *implicit_weight = item->weight + jamoind * MY_UCA_CE_SIZE;
    const uint16 *jamo_weight = uca->item[hangul_jamo[jamoind]].weight;
    *implicit_weight = *jamo_weight;
    *(implicit_weight + 1) = *(jamo_weight + 1);
    *(implicit_weight + 2) = *(jamo_weight + 2) + 1;
  }
  item->num_of_ce = jamo_cnt;
}

static void set_implicit_weights(MY_UCA_ITEM *item, int code) {
  int base, aaaa, bbbb;
  if (code >= 0x17000 && code <= 0x18AFF)  // Tangut character
  {
    aaaa = 0xFB00;
    bbbb = (code - 0x17000) | 0x8000;
  } else {
    /* non-Core Han Unified Ideographs */
    if ((code >= 0x3400 && code <= 0x4DB5) ||
        (code >= 0x20000 && code <= 0x2A6D6) ||
        (code >= 0x2A700 && code <= 0x2B734) ||
        (code >= 0x2B740 && code <= 0x2B81D) ||
        (code >= 0x2B820 && code <= 0x2CEA1))
      base = 0xFB80;
    /* Core Han Unified Ideographs */
    else if ((code >= 0x4E00 && code <= 0x9FD5) ||
             (code >= 0xFA0E && code <= 0xFA29))
      base = 0xFB40;
    /* All other characters whose weight is unassigned */
    else
      base = 0xFBC0;
    aaaa = base + (code >> 15);
    bbbb = (code & 0x7FFF) | 0x8000;
  }

  item->weight[0] = aaaa;
  item->weight[1] = 0x0020;
  item->weight[2] = 0x0002;
  item->weight[3] = bbbb;
  item->weight[4] = 0x0000;
  item->weight[5] = 0x0000;

  item->num_of_ce = 2;
}
/*
  We need to initialize implicit weights because
  some pages have both implicit and explicit weights:
  0x4D??, 0x9F??
*/
static void set_implicit_weights(MY_UCA *uca, const int *pageloaded) {
  for (int page = 0; page < MY_UCA_NPAGES; page++) {
    if (pageloaded[page] == MY_UCA_CHARS_PER_PAGE) continue;
    /* Now set implicit weights */
    for (int code = page * MY_UCA_CHARS_PER_PAGE;
         code < (page + 1) * MY_UCA_CHARS_PER_PAGE; code++) {
      MY_UCA_ITEM *item = &uca->item[code];

      if (item->num_of_ce) continue;

      int jamo_cnt = 0;
      my_wc_t hangul_jamo[HANGUL_JAMO_MAX_LENGTH];
      if ((jamo_cnt = my_decompose_hangul_syllable(code, hangul_jamo))) {
        my_put_jamo_weights(hangul_jamo, jamo_cnt, item, uca);
        continue;
      }

      set_implicit_weights(item, code);
    }
  }
}

static void get_page_statistics(const MY_UCA *uca, int page, int *maxnum) {
  for (int offs = 0; offs < MY_UCA_CHARS_PER_PAGE; offs++) {
    const MY_UCA_ITEM *item = &uca->item[page * MY_UCA_CHARS_PER_PAGE + offs];

    *maxnum = *maxnum < item->num_of_ce ? item->num_of_ce : *maxnum;
  }
}

/*
  Compose the prefix name of weight tables from the version number.
*/
static char *prefix_name(const MY_UCA *uca) {
  static char prefix[MY_UCA_VERSION_SIZE];
  const char *s;
  char *d;
  strcpy(prefix, "uca");
  for (s = uca->version, d = prefix + strlen(prefix); *s; s++) {
    if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z')) *d++ = *s;
  }
  *d = '\0';
  return prefix;
}

static char *page_name(const MY_UCA *uca, int page, bool pageloaded) {
  static char page_name_buf[120];
  static char page_name_null[] = "NULL";

  if (pageloaded) {
    snprintf(page_name_buf, sizeof(page_name_buf), "%s_p%03X", prefix_name(uca),
             page);
    return page_name_buf;
  } else
    return page_name_null;
}

static void print_one_page(const MY_UCA *uca, int page,
                           const char *page_name_prefix, int maxnum,
                           FILE *outfile) {
  if (page_name_prefix == nullptr)
    fprintf(outfile, "uint16 %s[] = {\n", page_name(uca, page, true));
  else
    fprintf(outfile, "uint16 %s%03X[] = {\n", page_name_prefix, page);

  fprintf(outfile, "  /* Number of CEs for each character. */\n");
  for (int offs = 0; offs < MY_UCA_CHARS_PER_PAGE; ++offs) {
    const int code = page * MY_UCA_CHARS_PER_PAGE + offs;
    const MY_UCA_ITEM *item = &uca->item[code];
    if ((offs % 16) == 0) fprintf(outfile, "  ");
    fprintf(outfile, "%d, ", item->num_of_ce);
    if ((offs % 16) == 15) fprintf(outfile, "\n");
  }

  for (int i = 0; i < maxnum - 1; i++) {
    fprintf(outfile, "\n");
    if ((i % 3) == 0) {
      fprintf(outfile, "  /* Primary weight %d for each character. */\n",
              i / 3 + 1);
    } else if ((i % 3) == 1) {
      fprintf(outfile, "  /* Secondary weight %d for each character. */\n",
              i / 3 + 1);
    } else {
      fprintf(outfile, "  /* Tertiary weight %d for each character. */\n",
              i / 3 + 1);
    }
    for (int offs = 0; offs < MY_UCA_CHARS_PER_PAGE; offs++) {
      const int code = page * MY_UCA_CHARS_PER_PAGE + offs;
      const MY_UCA_ITEM *item = &uca->item[code];
      const uint16 *weight = item->weight;
      fprintf(outfile, "  0x%04X,   /* U+%04X */\n", weight[i], code);
    }
  }
  fprintf(outfile, "};\n\n");
}

/*
  This function is called to read in language specific data.
*/
int read_in_lang_data(char *inbytes, int maxbytes, FILE *infile) {
  do {
    if (!fgets((char *)inbytes, maxbytes, infile)) {
      fprintf(stderr, "Could not read more characters.\n");
      return -1;
    }
  } while (!strncmp((char *)inbytes, "#", 1));  // Jump over copyright info.
  return 0;
}

int dump_ja_hans(MY_UCA *uca, FILE *infile, FILE *outfile) {
  // There are 6355 Japanese Han characters.
  unsigned char ja_u8_bytes[8000 * 3] = {0};

  if (read_in_lang_data((char *)ja_u8_bytes, sizeof(ja_u8_bytes), infile))
    return 1;
  int ja_length = strlen((char *)ja_u8_bytes);
  while (ja_length > 0 && (ja_u8_bytes[ja_length - 1] == '\n' ||
                           ja_u8_bytes[ja_length - 1] == '\r')) {
    ja_u8_bytes[ja_length - 1] = '\0';
    ja_length--;
  }
  // All these Japanese Han characters should be 3 bytes.
  if ((ja_length % 3)) {
    fprintf(stderr, "Wrong UTF8 Han character bytes.\n");
    return 1;
  }
  int han_cnt = ja_length / 3;
  const int JA_CORE_HAN_BASE_WT = 0x54A4;
  const int ja_han_page_cnt = 0x9F - 0x4E + 1;
  // Set weight for Japanese Han characters.
  unsigned char *ja_han = ja_u8_bytes;
  int min_page = 0x1100;  // the max code point utf8mb4 supports is 0x10FFFF.
  int max_page = 0;
  for (int i = 0; i < han_cnt; i++) {
    my_wc_t ja_ch_u16 = 0;
    int bytes = my_mb_wc_utf8mb4(&ja_ch_u16, ja_han, ja_han + ja_length);
    if (bytes <= 0) break;
    ja_han += bytes;
    int page MY_ATTRIBUTE((unused)) = ja_ch_u16 >> 8;
    assert(page >= 0x4E && page <= 0x9F);
    MY_UCA_ITEM *item = &uca->item[ja_ch_u16 - 0x4E00];
    item->num_of_ce = 1;
    item->weight[0] = JA_CORE_HAN_BASE_WT + i;
    item->weight[1] = 0x20;
    item->weight[2] = 0x02;
    min_page = std::min(min_page, page);
    max_page = std::max(max_page, page);
  }

  // Set implicit weight for non-Japanese characters.
  for (int page = min_page; page <= max_page; page++) {
    for (int offs = 0; offs < MY_UCA_CHARS_PER_PAGE; ++offs) {
      int code = (page << 8) + offs;
      int ind = code - 0x4E00;
      MY_UCA_ITEM *item = &uca->item[ind];
      if (item->num_of_ce == 0) set_implicit_weights(item, code);
    }
  }

  fprintf(outfile, "#include \"my_inttypes.h\"\n\n");
  fprintf(outfile, "extern const int MIN_JA_HAN_PAGE = 0x%X;\n", min_page);
  fprintf(outfile, "extern const int MAX_JA_HAN_PAGE = 0x%X;\n\n", max_page);
  // Print weights.
  for (int page = 0; page < ja_han_page_cnt; page++) {
    fprintf(outfile, "uint16 ja_han_page%2X[]= {\n", min_page + page);
    fprintf(outfile, "  /* Number of CEs for each character. */\n");
    for (int offs = 0; offs < MY_UCA_CHARS_PER_PAGE; ++offs) {
      int ind = (page << 8) + offs;
      MY_UCA_ITEM *item = &uca->item[ind];
      if ((offs % 16) == 0) fprintf(outfile, "  ");
      fprintf(outfile, "%d, ", item->num_of_ce);
      if ((offs % 16) == 15) fprintf(outfile, "\n");
    }
    for (int i = 0; i < 6; i++) {
      fprintf(outfile, "\n");
      if ((i % 3) == 0) {
        fprintf(outfile, "  /* Primary weight %d for each character. */\n",
                i / 3 + 1);
      } else if ((i % 3) == 1) {
        fprintf(outfile, "  /* Secondary weight %d for each character. */\n",
                i / 3 + 1);
      } else {
        fprintf(outfile, "  /* Tertiary weight %d for each character. */\n",
                i / 3 + 1);
      }
      for (int offs = 0; offs < MY_UCA_CHARS_PER_PAGE; offs++) {
        const int ind = page * MY_UCA_CHARS_PER_PAGE + offs;
        const int code = (page + min_page) * MY_UCA_CHARS_PER_PAGE + offs;
        const MY_UCA_ITEM *item = &uca->item[ind];
        const uint16 *weight = item->weight;
        fprintf(outfile, "  0x%04X,   /* U+%04X */\n", weight[i], code);
      }
    }
    fprintf(outfile, "};\n\n");
  }
  /* Print page index */
  fprintf(outfile, "uint16* ja_han_pages[%d]= {\n", ja_han_page_cnt);
  for (int page = 0; page < ja_han_page_cnt; page++) {
    if (!(page % 5))
      fprintf(outfile, "%13s%2X", "ja_han_page", page + min_page);
    else
      fprintf(outfile, "%12s%2X", "ja_han_page", page + min_page);
    if ((page + 1) != ja_han_page_cnt) fprintf(outfile, ",");
    if (!((page + 1) % 5) || (page + 1) == ja_han_page_cnt)
      fprintf(outfile, "\n");
  }
  fprintf(outfile, "};\n\n");

  return 0;
}

/*
  Chinese Han characters are assigned an implicit weight according to the
  Unicode Collation Algorithm. But when creating our Chinese collation for
  utf8mb4, to implement this language's reorder rule, we decide to give the Han
  characters in CLDR zh.xml file the weight value from 0x1C47 to 0xBDBE, and let
  the other Han characters still have their implicit weight. Per UCA, the
  smallest leading primary weight of the implicit weight is 0xFB00, and the
  largest primary weight we ocuppy for the Han characters in zh.xml is 0xBDBE.
  There is a huge gap between these two weight values. To use this weight value
  gap and let the character groups like Latin, Cyrillic, have single primary
  weight as before reordering, we decide to change the leading primary weight of
  the implicit weight as below.
 */
uint16 change_zh_implicit(uint16 weight) {
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

/*
  UCA defines an algorithm to calculate character's implicit weight if this
  character's weight is not defined in the DUCET. This function is to help
  convert Chinese character's implicit weight calculated by UCA back to its code
  points.
  The implicit weight and the code point is not 1 : 1 map. But for the Han
  characters in zh.xml file, each one has unique implicit weight from others.
 */
my_wc_t convert_implicit_to_ch(uint16 first, uint16 second) {
  assert(first >= 0xFB40 && first <= 0xFBC1);
  if (first < 0xFB80)
    return (((first - 0xFB40) << 15) | (second & 0x7FFF));
  else if (first < 0xFBC0)
    return (((first - 0xFB80) << 15) | (second & 0x7FFF));
  else
    return (((first - 0xFBC0) << 15) | (second & 0x7FFF));
}

int dump_zh_hans(MY_UCA *uca, int *pageloaded, FILE *infile, FILE *outfile) {
  /*
    zh.xml of cldr v33 defines 41336 Chinese Han characters. This xml file is
    encoded in utf8. Most of the Han characters are encoded in 3 bytes, and some
    are encoded in 4 bytes.
   */
  constexpr int ZH_HAN_CNT = 41336;
  unsigned char zh_bytes[ZH_HAN_CNT * 4]{0};

  if (read_in_lang_data((char *)zh_bytes, sizeof(zh_bytes), infile)) return 1;
  /*
    Since the rule [reorder Hani], Chinese Han character's weight should be
    smaller than any other non-ignorable characters (except of the core
    characters like spaces, symbols).

    To make the reordering, we decide to change the weight of all characters
    as:
    Char Group   | Origin Weight Range         | Reordered Weight Range
    -------------|-----------------------------|----------------------------
    core chars   | 0200 - 1C46                 | 0200 - 1C46
    Han in zh.xml| [FB40, AAAA] - [FB85, BBBB] | 1C47 - BDBE
    Other Han    | [FB40, CCCC] - [FB85, DDDD] | [BDBF, CCCC] - [BDC3, DDDD]
    Latin, etc   | 1C47 - 54A3                 | BDC4 - F620
    Others       | [FBC0, XXXX] - [FBE1, YYYY] | [F621, XXXX] - [F642, YYYY]

    This function changes only the weight of the Han characters defined in
    zh.xml and other characters in the same pages these Han characters reside.
   */
  constexpr int ZH_CORE_HAN_BASE_WT = 0x1C47;

  std::map<int, int> zh_han_to_single_weight_map;
  unsigned char *zh_ch = zh_bytes;
  int zh_len = strlen((char *)zh_bytes);
  int min_page = 0x1100;  // the max code point utf8mb4 supports is 0x10FFFF.
  int max_page = 0;
  for (int i = 0; i < ZH_HAN_CNT; i++) {
    my_wc_t ch = 0;
    int bytes = my_mb_wc_utf8mb4(&ch, zh_ch, zh_ch + zh_len);
    if (bytes <= 0) break;
    zh_ch += bytes;
    int page = ch >> 8;
    uca->item[ch].num_of_ce = 1;
    uca->item[ch].weight[0] = ZH_CORE_HAN_BASE_WT + i;
    uca->item[ch].weight[1] = 0x20;
    uca->item[ch].weight[2] = 0x02;
    pageloaded[page]++;
    min_page = std::min(min_page, page);
    max_page = std::max(max_page, page);
    MY_UCA_ITEM tmp_item;
    set_implicit_weights(&tmp_item, ch);
    zh_han_to_single_weight_map[ch] = ZH_CORE_HAN_BASE_WT + i;
  }

  // Chinese Han characters defined in zh.xml are all in pages 0x2E ~ 0x9F and
  // pages 0x200 ~ 0x2B8.
  for (int page = min_page; page <= max_page; page++) {
    if (pageloaded[page]) {
      // There is same page in DUCET.
      if (uca900_weight[page]) {
        for (int off = 0; off < MY_UCA_CHARS_PER_PAGE; off++) {
          int ch_off = (page << 8) + off;
          // Copy other characters' weight from DUCET.
          if (uca->item[ch_off].num_of_ce == 0) {
            uca->item[ch_off].num_of_ce =
                UCA900_NUM_OF_CE(uca900_weight[page], off);
            for (int level = 0; level < 3; level++) {
              uint16 *weight =
                  UCA900_WEIGHT_ADDR(uca900_weight[page], level, off);
              uint16 *dst = uca->item[ch_off].weight + level;
              for (int ce = 0; ce < uca->item[ch_off].num_of_ce; ce++) {
                if (*weight >= 0x1C47 && *weight <= 0x54A3) {
                  *dst = *weight + 0xBDC4 - 0x1C47;
                } else if (*weight >= 0xFB00) {  // implicit weight
                  uint16 next_implicit =
                      *(weight + UCA900_DISTANCE_BETWEEN_WEIGHTS);
                  my_wc_t ch = convert_implicit_to_ch(*weight, next_implicit);
                  if (zh_han_to_single_weight_map.find(ch) !=
                      zh_han_to_single_weight_map.end()) {
                    *dst = zh_han_to_single_weight_map[ch];
                    dst += 3;
                    weight += UCA900_DISTANCE_BETWEEN_WEIGHTS;
                    ce++;
                  } else {
                    *dst = change_zh_implicit(*weight);
                    dst += 3;
                    weight += UCA900_DISTANCE_BETWEEN_WEIGHTS;
                    ce++;
                    *dst = *weight;
                    dst += 3;
                    weight += UCA900_DISTANCE_BETWEEN_WEIGHTS;
                  }
                } else {
                  *dst = *weight;
                }
                dst += 3;
                weight += UCA900_DISTANCE_BETWEEN_WEIGHTS;
              }
            }
          }
        }
      } else {
        for (int off = 0; off < MY_UCA_CHARS_PER_PAGE; off++) {
          int ch = (page << 8) + off;
          if (uca->item[ch].num_of_ce == 0) {
            // calculate its implicit weight.
            set_implicit_weights(&uca->item[ch], ch);
            // Only the first primary weight needs to be changed in place.
            uca->item[ch].weight[0] =
                change_zh_implicit(uca->item[ch].weight[0]);
          }
        }
      }
    }
  }

  fprintf(outfile, "#include \"my_inttypes.h\"\n\n");
  fprintf(outfile, "extern const int MIN_ZH_HAN_PAGE = 0x%X;\n", min_page);
  fprintf(outfile, "extern const int MAX_ZH_HAN_PAGE = 0x%X;\n\n", max_page);
  for (int page = min_page; page <= max_page; page++) {
    if (pageloaded[page]) {
      int maxnum = 0;
      get_page_statistics(uca, page, &maxnum);

      maxnum = maxnum * MY_UCA_CE_SIZE + 1;
      print_one_page(uca, page, "zh_han_p", maxnum, outfile);
    }
  }

  fprintf(outfile, "uint16* zh_han_pages[%d] = {\n", max_page - min_page + 1);
  for (int page = min_page; page <= max_page; page++) {
    if (!((page - min_page) % 5)) {
      if (pageloaded[page]) {
        fprintf(outfile, "%10s%03X", "zh_han_p", page);
      } else {
        fprintf(outfile, "%13s", "NULL");
      }
    } else {
      if (pageloaded[page]) {
        fprintf(outfile, "%9s%03X", "zh_han_p", page);
      } else {
        fprintf(outfile, "%12s", "NULL");
      }
    }
    if ((page - min_page + 1) != MY_UCA_NPAGES) fprintf(outfile, ",");
    if (!((page - min_page + 1) % 5) || (page - min_page + 1) == MY_UCA_NPAGES)
      fprintf(outfile, "\n");
  }
  fprintf(outfile, "\n};\n\n");

  fprintf(outfile, "int zh_han_to_single_weight[] = {\n");
  for (auto map_it = zh_han_to_single_weight_map.begin();
       map_it != zh_han_to_single_weight_map.end(); map_it++) {
    fprintf(outfile, "  0x%05X, 0x%04X,\n", map_it->first, map_it->second);
  }
  fprintf(outfile, "\n};\n\n");
  fprintf(outfile, "extern const int ZH_HAN_WEIGHT_PAIRS = %lu;\n",
          static_cast<unsigned long>(zh_han_to_single_weight_map.size()));

  return 0;
}

enum OPT_DUMP { DUCET_DUMP, JA_DUMP, ZH_DUMP, DUMP_ERROR };

OPT_DUMP handle_options(int ac, char **av, char **infilename,
                        char **outfilename) {
  if (ac != 4) return DUMP_ERROR;
  if (!native_strcasecmp(av[1], "ducet") || !native_strcasecmp(av[1], "ja") ||
      !native_strcasecmp(av[1], "zh")) {
    if (!native_strncasecmp(av[2], "--in_file=", 10)) *infilename = av[2] + 10;
    if (!native_strncasecmp(av[3], "--out_file=", 11))
      *outfilename = av[3] + 11;
    if (*infilename == nullptr || *outfilename == nullptr) return DUMP_ERROR;
    if (!native_strcasecmp(av[1], "ducet")) return DUCET_DUMP;
    if (!native_strcasecmp(av[1], "ja")) return JA_DUMP;
    if (!native_strcasecmp(av[1], "zh")) return ZH_DUMP;
  }
  return DUMP_ERROR;
}

int dump_ducet(MY_UCA *uca, int *pageloaded, FILE *infile, FILE *outfile) {
  int maxchar = MY_UCA_MAXCHAR;
  load_uca_file(uca, maxchar, pageloaded, infile);

  set_implicit_weights(uca, pageloaded);

  int pagemaxlen[MY_UCA_NPAGES];

  for (int page = 0; page < MY_UCA_NPAGES; page++) {
    int maxnum = 0;

    pagemaxlen[page] = 0;

    /* Skip this page if no weights were loaded */
    if (!pageloaded[page]) continue;

    /*
      Calculate number of weights per character
      and number of default weights.
    */
    get_page_statistics(uca, page, &maxnum);

    maxnum = maxnum * MY_UCA_CE_SIZE + 1;

    pagemaxlen[page] = maxnum;

    print_one_page(uca, page, nullptr, maxnum, outfile);
  }

  /* Print page index */
  fprintf(outfile, "uint16* %s_weight[%d]= {\n", prefix_name(uca),
          MY_UCA_NPAGES);
  for (int page = 0; page < MY_UCA_NPAGES; page++) {
    if (!(page % 6))
      fprintf(outfile, "%13s", page_name(uca, page, pagemaxlen[page]));
    else
      fprintf(outfile, "%12s", page_name(uca, page, pagemaxlen[page]));
    if ((page + 1) != MY_UCA_NPAGES) fprintf(outfile, ",");
    if (!((page + 1) % 6) || (page + 1) == MY_UCA_NPAGES)
      fprintf(outfile, "\n");
  }
  fprintf(outfile, "};\n\n");
  return 0;
}

int main(int ac, char **av) {
  char *infilename = nullptr;
  char *outfilename = nullptr;
  OPT_DUMP od = handle_options(ac, av, &infilename, &outfilename);
  if (od == DUMP_ERROR) {
    printf(
        "Usage: uca9dump [ducet|ja|zh] --in_file=[inputfile] "
        "--out_file=[outputfile]\n");
    return 0;
  }
  FILE *infile = fopen(infilename, "rb");
  if (!infile) {
    printf("Can not open the file: %s\n", infilename);
    return 0;
  }
  FILE *outfile = fopen(outfilename, "wb");
  if (!outfile) {
    printf("Can not open the file: %s\n", outfilename);
    fclose(infile);
    return 0;
  }

  MY_UCA *uca = new MY_UCA();
  int pageloaded[MY_UCA_NPAGES];

  memset(uca, 0, sizeof(MY_UCA));
  memset(pageloaded, 0, sizeof(pageloaded));

  switch (od) {
    case DUCET_DUMP:
      dump_ducet(uca, pageloaded, infile, outfile);
      break;
    case JA_DUMP:
      dump_ja_hans(uca, infile, outfile);
      break;
    case ZH_DUMP:
      dump_zh_hans(uca, pageloaded, infile, outfile);
      break;
    default:
      printf(
          "Usage: uca9dump [ducet|ja|zh] --in_file=[inputfile] "
          "--out_file=[outputfile]\n");
      break;
  }
  fclose(infile);
  fclose(outfile);

  delete uca;

  return 0;
}
