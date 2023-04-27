/* Copyright (c) 2006, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <m_ctype.h>
#include <m_string.h>

struct MY_CTYPE_NAME_ST {
  const char *name;
  int val;
};

static MY_CTYPE_NAME_ST my_ctype_name[] = {
    {"Lu", _MY_U}, /* Letter, Uppercase          */
    {"Ll", _MY_L}, /* Letter, Lowercase          */
    {"Lt", _MY_U}, /* Letter, Titlecase          */
    {"Lm", _MY_L}, /* Letter, Modifier           */
    {"Lo", _MY_L}, /* Letter, other              */

    {"Nd", _MY_NMR},                 /* Number, Decimal Digit      */
    {"Nl", _MY_NMR | _MY_U | _MY_L}, /* Number, Letter             */
    {"No", _MY_NMR | _MY_PNT},       /* Number, Other              */

    {"Mn", _MY_L | _MY_PNT}, /* Mark, Nonspacing           */
    {"Mc", _MY_L | _MY_PNT}, /* Mark, Spacing Combining    */
    {"Me", _MY_L | _MY_PNT}, /* Mark, Enclosing            */

    {"Pc", _MY_PNT}, /* Punctuation, Connector     */
    {"Pd", _MY_PNT}, /* Punctuation, Dash          */
    {"Ps", _MY_PNT}, /* Punctuation, Open          */
    {"Pe", _MY_PNT}, /* Punctuation, Close         */
    {"Pi", _MY_PNT}, /* Punctuation, Initial quote */
    {"Pf", _MY_PNT}, /* Punctuation, Final quote   */
    {"Po", _MY_PNT}, /* Punctuation, Other         */

    {"Sm", _MY_PNT}, /* Symbol, Math               */
    {"Sc", _MY_PNT}, /* Symbol, Currency           */
    {"Sk", _MY_PNT}, /* Symbol, Modifier           */
    {"So", _MY_PNT}, /* Symbol, Other              */

    {"Zs", _MY_SPC}, /* Separator, Space           */
    {"Zl", _MY_SPC}, /* Separator, Line            */
    {"Zp", _MY_SPC}, /* Separator, Paragraph       */

    {"Cc", _MY_CTR}, /* Other, Control             */
    {"Cf", _MY_CTR}, /* Other, Format              */
    {"Cs", _MY_CTR}, /* Other, Surrogate           */
    {"Co", _MY_CTR}, /* Other, Private Use         */
    {"Cn", _MY_CTR}, /* Other, Not Assigned        */
    {NULL, 0}};

static int ctypestr2num(const char *tok) {
  MY_CTYPE_NAME_ST *p;
  for (p = my_ctype_name; p->name; p++) {
    if (!strncasecmp(p->name, tok, 2)) return p->val;
  }
  return 0;
}

#define MAX_CHAR 0x10FFFF
#define MAX_DECOMPOSITION_LENGTH 2

typedef struct {
  uint code;
  char *name;
  char general_category[3];
  int combining_class;
  int bidirectional_category;
  uint decomposition_mapping[MAX_DECOMPOSITION_LENGTH];
  uint decimal_digit_value; /* 0-9 */
  uint digit_value;         /* 0-9 */
  char *numeric_value;      /* Examples: 0, 1, 10, 100, 1000, 1/2, 5/2 */
  bool mirrored;            /* Y or N */
  char *unicode_1_0_name;
  char *iso10646_comment_field;
  uint uppercase_mapping;
  uint lowercase_mapping;
  uint titlecase_mapping;

  int mysql_ctype; /* ctype in MySQL format */

} MY_UNIDATA_CHAR;

typedef struct {
  int maxchar;
  int debug;
  int ctype;
  int decomp;
  const char *fname;
  const char *varname;
} MY_UNIDATA_PARAM;

static void unidata_param_init(MY_UNIDATA_PARAM *p) {
  p->maxchar = MAX_CHAR;
  p->debug = 0;
  p->ctype = 1;
  p->decomp = 1;
  p->fname = NULL;
  p->varname = "";
}

static void load_unidata(MY_UNIDATA_PARAM *prm, MY_UNIDATA_CHAR *chr) {
  char str[1024];
  FILE *f = prm->fname ? fopen(prm->fname, "r") : stdin;
  if (!f) {
    fprintf(stderr, "Can't open file %s\n", prm->fname);
    exit(1);
  }

  while (fgets(str, sizeof(str), f)) {
    size_t n;
    char *s, *e;
    MY_UNIDATA_CHAR ch;
    memset(&ch, 0, sizeof(ch));

    for (n = 0, s = str; s; n++) {
      char *end, tok[1024] = "";

      if ((e = strchr(s, ';'))) {
        strncpy(tok, s, (unsigned int)(e - s));
        tok[e - s] = 0;
      } else {
        strcpy(tok, s);
      }

      end = tok + strlen(tok);

      switch (n) {
        case 0:
          ch.code = strtol(tok, &end, 16);
          break;
        case 1:
          break; /* Character name */
        case 2:  /* General category */
          ch.general_category[0] = tok[0];
          ch.general_category[1] = tok[1];
          ch.general_category[2] = '\0';
          ch.mysql_ctype = ctypestr2num(tok);
          break;

        case 3: /* Canonical Combining Class */
          ch.combining_class = atoi(tok);
          /*
          if (ch.combining_class)
            printf("YYY[%04X]=%d\n", ch.code, ch.combining_class);
          */
          break;
        case 4:
          break; /* Bidirectional Category */
        case 5:  /* Character Decomposition Mapping */
          if (*tok != '<') {
            size_t i;
            char *dec, *endptr;
            for (dec = strtok_r(tok, " \t", &endptr), i = 0; dec;
                 dec = strtok_r(NULL, " \t", &endptr), i++) {
              if (i >= MAX_DECOMPOSITION_LENGTH) {
                fprintf(stderr,
                        "Decomposition length is too long for character %04X\n",
                        ch.code);
                exit(1);
              }
              ch.decomposition_mapping[i] = strtol(dec, NULL, 16);
            }
          }
          break;

        case 6: /* Decimal digit value */
          ch.decimal_digit_value = atoi(tok);
          break;

        case 7: /* Digit value */
          ch.digit_value = atoi(tok);
          break;

        case 8: /* Numeric value */
          break;

        case 9:
          break; /* Mirrored */
        case 10:
          break; /* Unicode 1.0 Name */
        case 11:
          break; /* 10646 comment field */
        case 12:
          break; /* Uppercase */
        case 13:
          break; /* Lowecase  */
        case 14:
          break; /* Titlecase */
      }
      s = e ? e + 1 : e;
    }
    if (ch.code <= prm->maxchar) chr[ch.code] = ch;
  }
}

static void unidata_char_set_cjk(MY_UNIDATA_CHAR *unidata, int max_char,
                                 int cur_char) {
  if (cur_char < max_char) {
    MY_UNIDATA_CHAR *ch = &unidata[cur_char];
    ch->mysql_ctype = _MY_L | _MY_U;
    strcpy(ch->general_category, "Lo");
  }
}

static void fill_implicit_ctype(MY_UNIDATA_PARAM *prm,
                                MY_UNIDATA_CHAR *unidata) {
  int i;
  /* Fill digits */
  for (i = '0'; i <= '9'; i++) unidata[i].mysql_ctype = _MY_NMR;
  /* Fill hex digits */
  for (i = 'a'; i <= 'z'; i++) unidata[i].mysql_ctype |= _MY_X;
  for (i = 'A'; i <= 'Z'; i++) unidata[i].mysql_ctype |= _MY_X;

  /* Fill ideographs  */
  /* CJK Ideographs Extension A (U+3400 - U+4DB5) */
  for (i = 0x3400; i <= 0x4DB5; i++)
    unidata_char_set_cjk(unidata, prm->maxchar, i);

  /* CJK Ideographs (U+4E00 - U+9FA5) */
  for (i = 0x4E00; i <= 0x9FA5; i++) /* 9FCB in 5.2.0 */
    unidata_char_set_cjk(unidata, prm->maxchar, i);

  /* Hangul Syllables (U+AC00 - U+D7A3)  */
  for (i = 0xAC00; i <= 0xD7A3; i++)
    unidata_char_set_cjk(unidata, prm->maxchar, i);

  /*
  20000;<CJK Ideograph Extension B, First>;Lo;0;L;;;;;N;;;;;
  2A6D6;<CJK Ideograph Extension B, Last>;Lo;0;L;;;;;N;;;;;
  */
  for (i = 0x20000; i <= 0x2A6D6; i++)
    unidata_char_set_cjk(unidata, prm->maxchar, i);

  /*
  2A700;<CJK Ideograph Extension C, First>;Lo;0;L;;;;;N;;;;;
  2B734;<CJK Ideograph Extension C, Last>;Lo;0;L;;;;;N;;;;;
  */
  for (i = 0x2A700; i <= 0x2B734; i++)
    unidata_char_set_cjk(unidata, prm->maxchar, i);

  /*
   TODO:
   D800;<Non Private Use High Surrogate, First>;Cs;0;L;;;;;N;;;;;
   DB7F;<Non Private Use High Surrogate, Last>;Cs;0;L;;;;;N;;;;;
   DB80;<Private Use High Surrogate, First>;Cs;0;L;;;;;N;;;;;
   DBFF;<Private Use High Surrogate, Last>;Cs;0;L;;;;;N;;;;;
   DC00;<Low Surrogate, First>;Cs;0;L;;;;;N;;;;;
   DFFF;<Low Surrogate, Last>;Cs;0;L;;;;;N;;;;;

   E000;<Private Use, First>;Co;0;L;;;;;N;;;;;
   F8FF;<Private Use, Last>;Co;0;L;;;;;N;;;;;
   F0000;<Plane 15 Private Use, First>;Co;0;L;;;;;N;;;;;
   FFFFD;<Plane 15 Private Use, Last>;Co;0;L;;;;;N;;;;;
   100000;<Plane 16 Private Use, First>;Co;0;L;;;;;N;;;;;
   10FFFD;<Plane 16 Private Use, Last>;Co;0;L;;;;;N;;;;;0
   */
}

/*
  Check if ctype for the entire page consisting of "nchars"
  characters is the same.
  Return -1 otherwise.
*/
static int page_ctype(MY_UNIDATA_CHAR *data, size_t nchars) {
  size_t i;
  for (i = 1; i < nchars; i++) {
    if (data[i].mysql_ctype != data->mysql_ctype) return -1;
  }
  return data->mysql_ctype;
}

static void dump_ctype(MY_UNIDATA_PARAM *prm, MY_UNIDATA_CHAR *unidata) {
  int page, max_page = (prm->maxchar + 255) / 256;

  printf("/*\n");
  printf("  Unicode ctype data\n");
  printf("  Generated from %s\n", prm->fname ? prm->fname : "stdin");
  printf("*/\n");

  /* Dump planes with mixed ctype */
  for (page = 0; page < max_page; page++) {
    if (page_ctype(unidata + page * 256, 256) < 0) {
      size_t charnum, num;
      printf("static unsigned char uctype%s_page%02X[256]=\n{\n", prm->varname,
             page);
      for (num = 0, charnum = 0; charnum < 256; charnum++) {
        printf(" %2d%s", unidata[page * 256 + charnum].mysql_ctype,
               charnum < 255 ? "," : "");
        if (++num == 16) {
          printf("\n");
          num = 0;
        }
      }
      printf("};\n\n");
    }
  }

  /* Dump ctype page index */
  printf("MY_UNI_CTYPE my_uni_ctype%s[%d]={\n", prm->varname, max_page);
  for (page = 0; page < max_page; page++) {
    char page_name[128] = "NULL";
    int ctype;
    if ((ctype = page_ctype(unidata + page * 256, 256)) < 0) {
      sprintf(page_name, "uctype%s_page%02X", prm->varname, page);
      ctype = 0;
    }
    printf("\t{%d,%s}%s\n", ctype, page_name, page < max_page - 1 ? "," : "");
  }
  printf("};\n\n\n");
}

/*
static int
decomposition_length(MY_UNIDATA_CHAR *ch)
{
  if (ch->decomposition_mapping[1])
    return 2;
  if (ch->decomposition_mapping[0])
    return 1;
  return 0;
}
*/

static void dump_decomposition_page(MY_UNIDATA_PARAM *prm,
                                    MY_UNIDATA_CHAR *unidata, uint pageno,
                                    uint nchars) {
  uint i, ofs = pageno * 256;
  printf("static MY_UNI_DECOMPOSITION decomp%s_p%02X[256]= {\n", prm->varname,
         pageno);
  for (i = 0; i < nchars; i++) {
    MY_UNIDATA_CHAR *ch = &unidata[ofs + i];

    printf("/* %04X */ {0x%04X,0x%04X},", ofs + i, ch->decomposition_mapping[0],
           ch->decomposition_mapping[1]);

    if (ch->decomposition_mapping[0])
      printf(" %s/* [%s-%s][%d-%d] */",
             ch->decomposition_mapping[0] < 0x10000 ? " " : "",
             unidata[ch->decomposition_mapping[0]].general_category,
             unidata[ch->decomposition_mapping[1]].general_category,
             unidata[ch->decomposition_mapping[0]].combining_class,
             unidata[ch->decomposition_mapping[1]].combining_class);
    printf("\n");
  }
  printf("};\n\n\n");
}

static size_t calc_decompositions(MY_UNIDATA_CHAR *unidata, size_t nchars) {
  size_t i, n;
  for (n = i = 0; i < nchars; i++) {
    if (unidata[i].decomposition_mapping[0]) n++;
  }
  return n;
}

static void dump_decomposition(MY_UNIDATA_PARAM *prm,
                               MY_UNIDATA_CHAR *unidata) {
  int i, npages = (prm->maxchar + 255) / 256;

  printf("/*\n");
  printf("  Unicode canonical decomposition data\n");
  printf("  Generated from %s\n", prm->fname ? prm->fname : "stdin");
  printf("*/\n");

  /* Dump pages */
  for (i = 0; i < npages; i++) {
    MY_UNIDATA_CHAR *page = unidata + i * 256;
    if (calc_decompositions(page, 256))
      dump_decomposition_page(prm, unidata, i, 256);
  }

  /* Dump decompositions */
  printf("static MY_UNI_DECOMPOSITION *my_uni_decomp%s[%d]=\n{\n", prm->varname,
         npages);
  for (i = 0; i < npages; i++) {
    MY_UNIDATA_CHAR *page = unidata + i * 256;
    if (calc_decompositions(page, 256))
      printf("decom%s_p%02X,", prm->varname, i);
    else
      printf("NULL,");
    if ((i % 8) == 7) printf("\n");
  }
  printf("};\n");
}

static void usage(FILE *f, int rc) { exit(rc); }

static int get_int_option(const char *str, const char *name, int *num) {
  size_t namelen = strlen(name);
  if (!strncmp(str, name, namelen)) {
    const char *val = str + namelen;
    if (val[0] == '0' && val[1] == 'x') {
      *num = strtol(val, NULL, 16);
    } else {
      *num = atoi(val);
      if (*num == 0 && *val != '0') {
        fprintf(stderr, "\nBad numeric option value: %s\n\n", str);
        usage(stderr, 1);
      }
    }
    return 1;
  }
  return 0;
}

static int get_const_str_option(const char *str, const char *name,
                                const char **val) {
  size_t namelen = strlen(name);
  if (!strncmp(str, name, namelen)) {
    *val = str + namelen;
    return 1;
  }
  return 0;
}

static void process_options(MY_UNIDATA_PARAM *prm, int ac, char **av) {
  int i;
  unidata_param_init(prm);
  for (i = 1; i < ac; i++) {
    /* printf("[%d]=%s\n", i, av[i]); */
    if (av[i][0] != '-' || av[i][1] != '-') break;
    if (!get_const_str_option(av[i], "--name=", &prm->varname) &&
        !get_int_option(av[i], "--maxchar=", &prm->maxchar) &&
        !get_int_option(av[i], "--ctype=", &prm->ctype) &&
        !get_int_option(av[i], "--decomp=", &prm->decomp) &&
        !get_int_option(av[i], "--debug=", &prm->debug)) {
      fprintf(stderr, "\nUnknown option: %s\n\n", av[i]);
      usage(stderr, 1);
    }
  }
  prm->fname = av[i];
}

int main(int ac, char **av) {
  MY_UNIDATA_PARAM prm;
  static MY_UNIDATA_CHAR unidata[MAX_CHAR + 1];

  process_options(&prm, ac, av);
  memset(unidata, 0, sizeof(unidata));
  fill_implicit_ctype(&prm, unidata);
  load_unidata(&prm, unidata);

  if (prm.ctype) dump_ctype(&prm, unidata);

  if (prm.decomp) dump_decomposition(&prm, unidata);

  return 0;
}
