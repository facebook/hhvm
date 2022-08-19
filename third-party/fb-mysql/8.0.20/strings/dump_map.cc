/* Copyright (c) 2003, 2015, Oracle and/or its affiliates. All rights reserved.

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
#include <string.h>

static void print_short_array(unsigned short *a, size_t width) {
  int i;
  printf("{\n");
  for (i = 0; i <= 0xFF; i++) {
    const char *fmt = (width == 4) ? "0x%04X" : "0x%02X";
    printf(fmt, (int)a[i]);
    printf("%s%s", i < 0xFF ? "," : "", (i + 1) % 8 ? "" : "\n");
  }
  printf("};\n");
}

int main(void) {
  char str[160];
  unsigned short touni[256];
  unsigned short fromuni[65536];
  unsigned short fromstat[256];
  int i;

  memset(touni, 0, sizeof(touni));
  memset(fromuni, 0, sizeof(fromuni));
  memset(fromstat, 0, sizeof(fromstat));

  while (fgets(str, sizeof(str), stdin)) {
    unsigned int c, u;

    if ((str[0] == '#') || (2 != sscanf(str, "%x%x", &c, &u))) continue;
    if (c > 0xFF || u > 0xFFFF) continue;

    touni[c] = u;
    fromuni[u] = c;
  }

  printf("unsigned short cs_to_uni[256]=");
  print_short_array(touni, 4);

  for (i = 0; i <= 0xFF; i++) {
    fromstat[touni[i] >> 8]++;
  }

  for (i = 0; i <= 256; i++) {
    if (fromstat[i]) {
      printf("unsigned char pl%02X[256]=", i);
      print_short_array(fromuni + i * 256, 2);
    }
  }

  printf("unsigned short *uni_to_cs[256]={\n");
  for (i = 0; i <= 255; i++) {
    if (fromstat[i])
      printf("pl%02X", i);
    else
      printf("NULL");
    printf("%s%s", i < 255 ? "," : "", ((i + 1) % 8) ? "" : "\n");
  }
  printf("};\n");

  return 0;
}
