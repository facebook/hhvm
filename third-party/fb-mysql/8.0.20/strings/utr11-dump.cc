/* Copyright (c) 2004, 2015, Oracle and/or its affiliates. All rights reserved.

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

/*
  Dump an EastAsianWidth.txt file.
  See http://www.unicode.org/reports/tr11/ for details.
  Character types:
  F  - Full width  = 1
  H  - Half width  = 0
  W  - Wide        = 1
  Na - Narrow      = 0
  A  - Ambiguous   = 0
  N  - Neutral     = 0
*/

int main(int ac, char **av) {
  char str[128];
  int errors = 0;
  int plane[0x10000];
  int page[256];
  int i;

  memset(plane, 0, sizeof(plane));
  memset(page, 0, sizeof(page));

  while (fgets(str, sizeof(str), stdin)) {
    int code1, code2, width;
    char *end;

    if (str[0] == '#') continue;
    code1 = strtol(str, &end, 16);
    if (code1 < 0 || code1 > 0xFFFF) continue;
    if (end[0] == ';') /* One character */
    {
      code2 = code1;
    } else if (end[0] == '.' && end[1] == '.') /* Range */
    {
      end += 2;
      code2 = strtol(end, &end, 16);
      if (code2 < 0 || code2 > 0xFFFF) continue;
      if (end[0] != ';') {
        errors++;
        fprintf(stderr, "error: %s", str);
        continue;
      }
    } else {
      errors++;
      fprintf(stderr, "error: %s", str);
      continue;
    }

    end++;
    width = (end[0] == 'F' || end[0] == 'W') ? 1 : 0;

    for (; code1 <= code2; code1++) {
      plane[code1] = width;
    }
  }

  if (errors) return 1;

  for (i = 0; i < 256; i++) {
    int j;
    int *p = plane + 256 * i;
    page[i] = 0;
    for (j = 0; j < 256; j++) {
      page[i] += p[j];
    }
    if (page[i] != 0 && page[i] != 256) {
      printf("static char pg%02X[256]=\n{\n", i);
      for (j = 0; j < 256; j++) {
        printf("%d%s%s", p[j], j < 255 ? "," : "", (j + 1) % 32 ? "" : "\n");
      }
      printf("};\n\n");
    }
  }

  printf("static struct {int page; char *p;} utr11_data[256]=\n{\n");
  for (i = 0; i < 256; i++) {
    if (page[i] == 0 || page[i] == 256) {
      int width = (page[i] == 256) ? 1 : 0;
      printf("{%d,NULL}", width);
    } else {
      printf("{0,pg%02X}", i);
    }
    printf("%s%s", i < 255 ? "," : "", (i + 1) % 8 ? "" : "\n");
  }
  printf("};\n");
  return 0;
}
