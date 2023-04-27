/* Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

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
  echo is a replacement for the "echo" command builtin to cmd.exe
  on Windows, to get a Unix eqvivalent behaviour when running commands
  like:
    $> echo "hello" | mysql

  The windows "echo" would have sent "hello" to mysql while
  Unix echo will send hello without the enclosing hyphens

  This is a very advanced high tech program so take care when
  you change it and remember to valgrind it before production
  use.

*/

#include <stdio.h>

int main(int argc, char **argv) {
  int i;
  for (i = 1; i < argc; i++) {
    fprintf(stdout, "%s", argv[i]);
    if (i < argc - 1) fprintf(stdout, " ");
  }
  fprintf(stdout, "\n");
  return 0;
}
