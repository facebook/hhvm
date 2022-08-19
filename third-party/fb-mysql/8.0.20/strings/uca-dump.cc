/* Copyright (c) 2004, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;
typedef unsigned short uint16;
typedef unsigned int uint;

#define MY_UCA_MAXWEIGHT_TO_PARSE 64
#define MY_UCA_MAXWEIGHT_TO_DUMP 8
#define MY_UCA_MAXLEVEL 4
#define MY_UCA_VERSION_SIZE 32
#define MY_UCA_MAX_CONTRACTION 6

#define MY_UCA_NCONTRACTIONS 1024
#define MY_UCA_MAXCHAR (0x10FFFF + 1)
#define MY_UCA_NCHARS 256
#define MY_UCA_CMASK 255
#define MY_UCA_PSHIFT 8
#define MY_UCA_NPAGES MY_UCA_MAXCHAR / MY_UCA_NCHARS
struct MY_UCA_ITEM {
  uchar num; /* Number of weights */
  uint16 weight[MY_UCA_MAXLEVEL + 1][MY_UCA_MAXWEIGHT_TO_DUMP];
  /* +1 for trailing zero */
};

struct MY_UCA_CONTRACTION {
  uint ch[MY_UCA_MAX_CONTRACTION];
  MY_UCA_ITEM item;
};

struct MY_UCA {
  char version[MY_UCA_VERSION_SIZE];
  MY_UCA_ITEM item[MY_UCA_MAXCHAR];
  size_t ncontractions;
  MY_UCA_CONTRACTION contraction[MY_UCA_NCONTRACTIONS];
  int optimize_contractions;
  int debug;
};

static int load_uca_file(MY_UCA *uca, size_t maxchar, int *pageloaded) {
  char str[512];
  size_t lineno, out_of_range_chars = 0;
  char *weights[MY_UCA_MAXWEIGHT_TO_PARSE];

  for (lineno = 0; fgets(str, sizeof(str), stdin); lineno++) {
    char *comment;
    char *weight;
    char *s, *ch[MY_UCA_MAX_CONTRACTION];
    size_t codenum, i, code;
    MY_UCA_ITEM *item = NULL;

    /* Skip comment lines */
    if (*str == '\r' || *str == '\n' || *str == '#') continue;

    /* Detect version */
    if (*str == '@' && !strncmp(str, "@version ", 9)) {
      const char *value;
      if (strtok(str, " \r\n\t") && (value = strtok(NULL, " \r\n\t")))
        snprintf(uca->version, MY_UCA_VERSION_SIZE, value);
      continue;
    }

    /* Skip big characters */
    if ((code = strtol(str, NULL, 16)) > maxchar) {
      out_of_range_chars++;
      continue;
    }

    if ((comment = strchr(str, '#'))) {
      *comment++ = '\0';
      for (; *comment == ' '; comment++)
        ;
    } else {
      fprintf(stderr, "Warning: could not parse line #%d:\n'%s'\n", lineno,
              str);
      continue;
    }

    if ((weight = strchr(str, ';'))) {
      *weight++ = '\0';
      for (; *weight == ' '; weight++)
        ;
    } else {
      fprintf(stderr, "Warning: could not parse line #%d:\n%s\n", lineno, str);
      continue;
    }

    for (codenum = 0, s = strtok(str, " \t"); s;
         codenum++, s = strtok(NULL, " \t")) {
      if (codenum == MY_UCA_MAX_CONTRACTION) {
        fprintf(stderr, "Contraction length is too long (%d) line #%d", codenum,
                lineno);
        exit(1);
      }
      ch[codenum] = s;
      ch[codenum + 1] = 0;
    }

    if (codenum > 1) {
      MY_UCA_CONTRACTION *c = &uca->contraction[uca->ncontractions++];
      size_t i;
      /* Multi-character weight (contraction) - not supported yet. */

      if (uca->ncontractions >= MY_UCA_NCONTRACTIONS) {
        fprintf(stderr,
                "Too many contractions (%d) at line #%d\n"
                "Rebuild with a bigger MY_UCA_MAXCONTRACTIONS value\n",
                uca->ncontractions, lineno);
        exit(1);
      }
      /* Copy codepoints of the contraction parts */
      for (i = 0; i < MY_UCA_MAX_CONTRACTION; i++) {
        c->ch[i] = (i < codenum) ? (uint)strtol(ch[i], NULL, 16) : 0;
      }

      if (uca->debug)
        fprintf(stderr, "Contraction: %04X-%04X-%04X\n", c->ch[0], c->ch[1],
                c->ch[2]);
      item = &c->item;
    } else {
      item = &uca->item[code];
    }

    /*
      Split weight string into separate weights

      "[p1.s1.t1.q1][p2.s2.t2.q2][p3.s3.t3.q3]" ->

      "p1.s1.t1.q1" "p2.s2.t2.q2" "p3.s3.t3.q3"
    */
    item->num = 0;
    s = strtok(weight, " []");
    while (s) {
      if (item->num >= MY_UCA_MAXWEIGHT_TO_PARSE) {
        fprintf(stderr, "Line #%d has more than %d weights\n", lineno,
                MY_UCA_MAXWEIGHT_TO_PARSE);
        fprintf(stderr, "Can't continue.\n");
        exit(1);
      }
      weights[item->num] = s;
      s = strtok(NULL, " []");
      item->num++;
    }

    for (i = 0; i < item->num; i++) {
      size_t level = 0;

      if (i >= MY_UCA_MAXWEIGHT_TO_DUMP) {
        fprintf(stderr,
                "Warning: at line %d: character %04X has"
                " more than %d many weights (%d). "
                "Skipping the extra weights.\n",
                lineno, code, MY_UCA_MAXWEIGHT_TO_DUMP, item->num);
        item->num = MY_UCA_MAXWEIGHT_TO_DUMP;
        break;
      }

      for (s = weights[i]; *s;) {
        char *endptr;
        size_t part = strtol(s + 1, &endptr, 16);
        if (i < MY_UCA_MAXWEIGHT_TO_DUMP) {
          item->weight[level][i] = part;
        } else {
          fprintf(stderr, "Too many weights (%d) at line %d\n", i, lineno);
          exit(1);
        }
        s = endptr;
        level++;
      }
    }
    /* Mark that a character from this page was loaded */
    pageloaded[code >> MY_UCA_PSHIFT]++;
  }

  if (out_of_range_chars)
    fprintf(stderr, "%d out-of-range characters skipped\n", out_of_range_chars);

  return 0;
}

/*
  We need to initialize implicit weights because
  some pages have both implicit and explicit weights:
  0x4D??, 0x9F??
*/
static void set_implicit_weights(MY_UCA *uca, size_t maxchar) {
  size_t code;
  /* Now set implicit weights */
  for (code = 0; code < maxchar; code++) {
    size_t base, aaaa, bbbb;
    MY_UCA_ITEM *item = &uca->item[code];

    if (item->num) continue;

    /*
    3400;<CJK Ideograph Extension A, First>
    4DB5;<CJK Ideograph Extension A, Last>
    4E00;<CJK Ideograph, First>
    9FA5;<CJK Ideograph, Last>
    */

    if (code >= 0x3400 && code <= 0x4DB5)
      base = 0xFB80;
    else if (code >= 0x4E00 && code <= 0x9FA5)
      base = 0xFB40;
    else
      base = 0xFBC0;

    aaaa = base + (code >> 15);
    bbbb = (code & 0x7FFF) | 0x8000;
    item->weight[0][0] = aaaa;
    item->weight[0][1] = bbbb;

    item->weight[1][0] = 0x0020;
    item->weight[1][1] = 0x0000;

    item->weight[2][0] = 0x0002;
    item->weight[2][1] = 0x0000;

    item->weight[3][0] = 0x0001;
    item->weight[3][2] = 0x0000;

    item->num = 2;
  }
}

static void get_page_statistics(MY_UCA *uca, size_t page, size_t level,
                                size_t *maxnum, size_t *ndefs) {
  size_t offs;

  for (offs = 0; offs < MY_UCA_NCHARS; offs++) {
    size_t i, num;
    MY_UCA_ITEM *item = &uca->item[page * MY_UCA_NCHARS + offs];

    /* Calculate only non-zero weights */
    for (num = 0, i = 0; i < item->num; i++) {
      if (item->weight[level][i]) num++;
    }
    *maxnum = *maxnum < num ? num : *maxnum;

    /* Check if default weight */
    if (level == 1 && num == 1) {
      /* 0020 0000 ... */
      if (item->weight[level][0] == 0x0020) (*ndefs)++;
    } else if (level == 2 && num == 1) {
      /* 0002 0000 ... */
      if (item->weight[level][0] == 0x0002) (*ndefs)++;
    }
  }
}

static const char *pname[] = {"", "_s", "_t", "_q"};
static const char *lname[] = {"primary", "secondary", "tertiary", "quaternary"};

static char *prefix_name(MY_UCA *uca) {
  static char prefix[MY_UCA_VERSION_SIZE];
  char *s, *d;
  strcpy(prefix, "uca");
  for (s = uca->version, d = prefix + strlen(prefix); *s; s++) {
    if ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'z')) *d++ = *s;
  }
  *d = '\0';
  return prefix;
}

static char *page_name(MY_UCA *uca, size_t page, size_t level) {
  static char page_name_buf[120];

  snprintf(page_name_buf, sizeof(page_name_buf), "%s_p%03X%s", prefix_name(uca),
           page, pname[level]);
  return page_name_buf;
}

/*
  "weight" must be [MY_UCA_MAXWEIGHT_TO_DUMP+1] elements long
*/
static size_t normalize_weight(MY_UCA_ITEM *item, size_t level, uint16 *weight,
                               size_t weight_elements) {
  size_t num, i;

  memset(weight, 0, weight_elements * sizeof(*weight));

  /*
    Copy non-zero weights only. For example:

    [.17A6.0020.0004.00DF][.0000.015F.0004.00DF][.17A6.0020.001F.00DF]

    makes [17A6][0000][17A6] on the primary level

    pack it to [17A6][17A7]
  */

  for (num = 0, i = 0; i < item->num && i < MY_UCA_MAXWEIGHT_TO_DUMP; i++) {
    if (item->weight[level][i]) {
      weight[num] = item->weight[level][i];
      num++;
    }
  }
  return num;
}

static void print_one_page(MY_UCA *uca, size_t level, size_t page,
                           size_t maxnum) {
  size_t offs, mchars, nchars = 0, chars_per_line;

  printf("uint16 %s[]= { /* %04X (%d weights per char) */\n",
         page_name(uca, page, level), page * MY_UCA_NCHARS, maxnum);

  /* Calculate how many wights to print per line */
  switch (maxnum) {
    case 0:
      mchars = 8;
      chars_per_line = 8;
      break;
    case 1:
      mchars = 8;
      chars_per_line = 8;
      break;
    case 2:
      mchars = 8;
      chars_per_line = 4;
      break;
    case 3:
      mchars = 9;
      chars_per_line = 3;
      break;
    case 4:
      mchars = 8;
      chars_per_line = 2;
      break;
    default:
      mchars = uca->item[page * MY_UCA_NCHARS + offs].num;
      chars_per_line = 1;
  }

  /* Print the page */
  for (offs = 0; offs < MY_UCA_NCHARS; offs++) {
    uint16 weight[MY_UCA_MAXWEIGHT_TO_DUMP + 1];
    size_t num, i, code = page * MY_UCA_NCHARS + offs;
    MY_UCA_ITEM *item = &uca->item[code];

    normalize_weight(item, level, weight, sizeof(weight) / sizeof(weight[0]));

    /* Print weights */
    for (i = 0; i < maxnum; i++) {
      int tmp = weight[i];

      printf("0x%04X", tmp);

      if (tmp > 0xFFFF || tmp < 0) {
        fprintf(stderr,
                "Error: Too big weight for code point %04X level %d: %08X\n",
                code, level, tmp);
        exit(1);
      }

      if ((offs + 1 != MY_UCA_NCHARS) || (i + 1 != maxnum))
        printf(",");
      else
        printf(" ");
      nchars++;
    }

    if (nchars >= mchars) {
      /* Print "\n" with a comment telling the first code on this line. */
      printf(" /* %04X */\n", (code + 1) - chars_per_line);
      nchars = 0;
    } else {
      printf(" ");
    }
  }
  printf("};\n\n");
}

/*
  Compare two weight strings.
  Return 1 if weight string differ
  Return 0 if weigh string are equal
*/
static int weight_cmp(uint16 *w1, uint16 *w2, size_t len) {
  size_t i;
  for (i = 0; i < len; i++) {
    if (w1[i] != w2[i]) return 1;
  }
  return 0;
}

static void print_contraction(MY_UCA *uca, MY_UCA_CONTRACTION *c,
                              size_t level) {
  size_t ch;
  uint16 weight[MY_UCA_MAXWEIGHT_TO_DUMP + 1];
  int optimize = 0;

  if (c) {
    normalize_weight(&c->item, level, weight,
                     sizeof(weight) / sizeof(weight[0]));

    if (uca->optimize_contractions) {
      /*
        Some contraction can be optimized away on certain levels.
        For example, in Unicode-6.0:

        0E40 0E01 ; [.2395.0020.0002.0E01][.23CF.0020.001F.0E40] # <THAI
        CHARACTER SARA E, THAI CHARACTER KO KAI>

        Its part weights are:

        0E40  ; [.23CF.0020.0002.0E40] # THAI CHARACTER SARA E
        0E01  ; [.2395.0020.0002.0E01] # THAI CHARACTER KO KAI

        On the secondary level weights for the contraction
        and for the two characters in a sequence are: 0020-0020.

        So "0E40 0E01" can be optimized away of the secondary level.

        This optimization is OFF by default, as it's better to optimize
        this at collation initialization time rather than at dump time,
        to preserve all available DUCET data.

        Also, this does not seem to ever happen on the primary level,
        so this optimization will not bring any serious performance
        improvement.
      */
      size_t i;
      uint16 sweight[MY_UCA_MAXWEIGHT_TO_DUMP * MY_UCA_MAX_CONTRACTION + 1],
          *sw;

      /* Concatenate weight arrays for the contraction parts */
      for (sw = sweight, i = 0; c->ch[i]; i++) {
        MY_UCA_ITEM *item = &uca->item[c->ch[i]];
        sw += normalize_weight(item, level, sw, MY_UCA_MAXWEIGHT_TO_DUMP);
      }

      if (sw - sweight < MY_UCA_MAXWEIGHT_TO_DUMP &&
          !weight_cmp(sweight, weight, MY_UCA_MAXWEIGHT_TO_DUMP)) {
        if (uca->debug)
          fprintf(stderr,
                  "Equal[%d]: %04X [%04X-%04X-%04X] == {%04X,%04X,%04X} "
                  "[%04X-%04X-%04X]\n",
                  level, c->ch[0], sweight[0], sweight[1], sweight[2], c->ch[0],
                  c->ch[1], c->ch[2], weight[0], weight[1], weight[2]);
        optimize = 1;
      }
    }
  }

  printf("%s{", optimize ? "/* " : "");
  for (ch = 0; ch < MY_UCA_MAX_CONTRACTION; ch++) {
    uint codepoint = c ? c->ch[ch] : 0; /* Real character or terminator line */
    printf("%s", ch > 0 ? "," : "");
    if (codepoint)
      printf("0x%04X", codepoint);
    else
      printf("0");
  }
  printf("},");
  printf("{");
  for (ch = 0; ch < MY_UCA_MAXWEIGHT_TO_DUMP; ch++) {
    uint w = c ? weight[ch] : 0; /* Real chr or terminator */
    printf("%s", ch > 0 ? "," : "");
    if (w)
      printf("0x%04X", w);
    else
      printf("0");
  }
  printf("}");
  printf(",0");
  printf("},%s\n", optimize ? " */" : "");
}

static void print_contractions(MY_UCA *uca, size_t level) {
  size_t i;

  printf("\n\n");
  printf("/* Contractions, %s level */\n", lname[level]);
  printf("MY_CONTRACTION %s_default_contraction%s[]= {\n", prefix_name(uca),
         pname[level]);
  for (i = 0; i < uca->ncontractions; i++) {
    MY_UCA_CONTRACTION *c = &uca->contraction[i];
    print_contraction(uca, c, level);
  }
  print_contraction(uca, NULL, level);
  printf("};\n");
}

static int contractions = 0;
static int nlevels = 1;

static void usage(FILE *file, int rc) {
  fprintf(file, "Usage:\n");
  fprintf(file, "uca-dump [options...] < /path/to/allkeys.txt\n");
  fprintf(file, "\n");
  fprintf(file, "Options:\n");
  fprintf(file,
          "--levels=NUM                 How many levels to dump, 1-4, default "
          "1.\n");
  fprintf(file,
          "--contractions=NUM           Whether to dump comtractions, 0-1, "
          "default 0.\n");
  fprintf(file,
          "--optimize-contractions=NUM  Whether to optimize contractions, 0-1, "
          "default 0.\n");
  fprintf(file,
          "--debug=NUM                  Print debug information, 0-1, default "
          "0.\n");
  fprintf(file, "\n\n");
  exit(rc);
}

static int get_int_option(const char *str, const char *name, int *num) {
  size_t namelen = strlen(name);
  if (!strncmp(str, name, namelen)) {
    *num = atoi(str + namelen);
    if (*num == 0 && str[namelen] != '0') {
      fprintf(stderr, "\nBad numeric option value: %s\n\n", str);
      usage(stderr, 1);
    }
    return 1;
  }
  return 0;
}

static void process_options(int ac, char **av, MY_UCA *uca) {
  size_t i;
  for (i = 1; i < ac; i++) {
    /*printf("[%d]=%s\n", i, av[i]);*/
    if (!get_int_option(av[i], "--levels=", &nlevels) &&
        !get_int_option(av[i], "--contractions=", &contractions) &&
        !get_int_option(av[i], "--debug=", &uca->debug) &&
        !get_int_option(
            av[i], "--optimize-contractions=", &uca->optimize_contractions)) {
      fprintf(stderr, "\nUnknown option: %s\n\n", av[i]);
      usage(stderr, 1);
    }
  }
}

int main(int ac, char **av) {
  static MY_UCA uca;
  size_t level, maxchar = MY_UCA_MAXCHAR;
  static int pageloaded[MY_UCA_NPAGES];

  memset(&uca, 0, sizeof(uca));

  process_options(ac, av, &uca);

  memset(pageloaded, 0, sizeof(pageloaded));

  load_uca_file(&uca, maxchar, pageloaded);

  /* Now set implicit weights */
  set_implicit_weights(&uca, maxchar);

  printf("#include \"my_uca.h\"\n");
  printf("\n\n");
  printf("#define MY_UCA_NPAGES %d\n", MY_UCA_NPAGES);
  printf("#define MY_UCA_NCHARS %d\n", MY_UCA_NCHARS);
  printf("#define MY_UCA_CMASK  %d\n", MY_UCA_CMASK);
  printf("#define MY_UCA_PSHIFT %d\n", MY_UCA_PSHIFT);
  printf("\n\n");
  printf("/* Created from allkeys.txt. Unicode version '%s'. */\n\n",
         uca.version);

  for (level = 0; level < nlevels; level++) {
    size_t page;
    int pagemaxlen[MY_UCA_NPAGES];

    for (page = 0; page < MY_UCA_NPAGES; page++) {
      size_t maxnum = 0;
      size_t ndefs = 0;

      pagemaxlen[page] = 0;

      /* Skip this page if no weights were loaded */
      if (!pageloaded[page]) continue;

      /*
        Calculate number of weights per character
        and number of default weights.
      */
      get_page_statistics(&uca, page, level, &maxnum, &ndefs);

      maxnum++; /* For zero terminator */

      /*
        If the page have only default weights
        then no needs to dump it, skip.
      */
      if (ndefs == MY_UCA_NCHARS) continue;

      pagemaxlen[page] = maxnum;

      /* Now print this page */
      print_one_page(&uca, level, page, maxnum);
    }

    /* Print page lengths */
    printf("uchar %s_length%s[%d]={\n", prefix_name(&uca), pname[level],
           MY_UCA_NPAGES);
    for (page = 0; page < MY_UCA_NPAGES; page++) {
      printf("%d%s%s", pagemaxlen[page], page < MY_UCA_NPAGES - 1 ? "," : "",
             (page + 1) % 16 ? "" : "\n");
    }
    printf("};\n");

    /* Print page index */
    printf("uint16 *%s_weight%s[%d]={\n", prefix_name(&uca), pname[level],
           MY_UCA_NPAGES);
    for (page = 0; page < MY_UCA_NPAGES; page++) {
      const char *comma = page < MY_UCA_NPAGES - 1 ? "," : "";
      const char *nline = (page + 1) % 4 ? "" : "\n";
      if (!pagemaxlen[page])
        printf("NULL       %s%s%s", level ? " " : "", comma, nline);
      else
        printf("%s%s%s", page_name(&uca, page, level), comma, nline);
    }
    printf("};\n");

    /* Print contractions */
    if (contractions) print_contractions(&uca, level);
  }

  printf("int main(void){ return 0;};\n");
  return 0;
}
