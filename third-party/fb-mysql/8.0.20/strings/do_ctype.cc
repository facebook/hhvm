/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* Prints case-convert and sort-convert tabell on stdout. This is used to
   make _ctype.c easyer */

#ifdef DBUG_OFF
#undef DBUG_OFF
#endif

#include <ctype.h>
#include <my_sys.h>

#include "m_string.h"

uchar to_upper[256];
uchar to_lower[256], sort_order[256];

static int ascii_output = 1;
static string tab_names[] = {"to_lower[]={", "to_upper[]={", "sort_order[]={"};
static uchar *tabell[] = {to_lower, to_upper, sort_order};

void get_options(), init_case_convert();

main(argc, argv) int argc;
char *argv[];
{
  int i, j, ch;
  DBUG_TRACE;
  DBUG_PROCESS(argv[0]);

  get_options(&argc, &argv);
  init_case_convert();
  puts("Tabells for caseconverts and sorttest of characters\n");
  for (i = 0; i < 3; i++) {
    printf("uchar %s\n", tab_names[i]);
    for (j = 0; j <= 255; j++) {
      ch = (int)tabell[i][j];
      if (ascii_output && isprint(ch) && !(ch & 128)) {
        if (strchr("\\'", (char)ch))
          printf("'\\%c',  ", ch);
        else
          printf("'%c',   ", ch);
      } else
        printf("'\\%03o',", ch);
      if ((j + 1 & 7) == 0) puts("");
    }
    puts("};\n");
  }
  return 0;
} /* main */

/* Read options */

void get_options(argc, argv) int *argc;
char **argv[];
{
  int help, version;
  char *pos, *progname;

  progname = (*argv)[0];
  help = 0;
  ascii_output = 1;
  while (--*argc > 0 && *(pos = *(++*argv)) == '-') {
    while (*++pos) {
      version = 0;
      switch (*pos) {
        case 'n': /* Numeric output */
          ascii_output = 0;
          break;
        case '#':
          DBUG_PUSH(++pos);
          *(pos--) = '\0'; /* Skippa argument */
          break;
        case 'V':
          version = 1;
        case 'I':
        case '?':
          printf("%s  Ver 1.0\n", progname);
          if (version) break;
          puts("Output tabells of to_lower[], to_upper[] and sortorder[]\n");
          printf("Usage: %s [-n?I]\n", progname);
          puts("Options: -? or -I \"Info\" -n \"numeric output\"");
          break;
        default:
          fprintf(stderr, "illegal option: -%c\n", *pos);
          break;
      }
    }
  }
  return;
} /* get_options */

/* set up max character for which isupper() and toupper() gives */
/* right answer. Is usually 127 or 255 */

#define MAX_CHAR_OK 127 /* 7 Bit ascii */

/* Initiate arrays for case-conversation */

void init_case_convert() {
  int16 i;
  uchar *higher_pos, *lower_pos;
  DBUG_TRACE;

  for (i = 0; i <= MAX_CHAR_OK; i++) {
    to_upper[i] = sort_order[i] = (islower(i) ? toupper(i) : (char)i);
    to_lower[i] = (isupper(i) ? tolower(i) : (char)i);
  }
#if MAX_CHAR_OK != 255
  for (i--; i++ < 255;) to_upper[i] = sort_order[i] = to_lower[i] = (char)i;
#endif

  higher_pos = (uchar *)"[]\\@^";
  lower_pos = (uchar *)"{}|`~";

  while (*higher_pos) {
    to_upper[*lower_pos] = sort_order[*lower_pos] = (char)*higher_pos;
    to_lower[*higher_pos++] = (char)*lower_pos++;
  }

  /* sets upp sortorder; higer_pos character (upper and lower) is */
  /* changed to lower_pos character */

  higher_pos = (uchar *)"][\\~`"; /* R{tt ordning p} tecknen */
  lower_pos = (uchar *)"[\\]YE";  /* Ordning enligt ascii */

  while (*higher_pos) {
    sort_order[*higher_pos] = sort_order[(uchar)to_lower[*higher_pos]] =
        *lower_pos;
    higher_pos++;
    lower_pos++;
  }
} /* init_case_convert */
