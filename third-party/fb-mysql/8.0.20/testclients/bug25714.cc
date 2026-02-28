/* Copyright (c) 2007, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <assert.h>
#include <mysql.h>
#include <sys/types.h>

#include "m_string.h"
#include "my_compiler.h"
#include "my_sys.h"

int main(int argc, char **argv) {
  MYSQL conn;
  int OK MY_ATTRIBUTE((unused));

  const char *query4 = "INSERT INTO federated.t1 SET Value=54";
  const char *query5 = "INSERT INTO federated.t1 SET Value=55";

  MY_INIT(argv[0]);

  if (argc != 2 || !strcmp(argv[1], "--help")) {
    fprintf(stderr,
            "This program is a part of the MySQL test suite. "
            "It is not intended to be executed directly by a user.\n");
    return -1;
  }

  mysql_init(&conn);
  if (!mysql_real_connect(&conn, "127.0.0.1", "root", "", "test", atoi(argv[1]),
                          nullptr, CLIENT_FOUND_ROWS)) {
    fprintf(stderr, "Failed to connect to database: Error: %s\n",
            mysql_error(&conn));
    return 1;
  } else {
    printf("%s\n", mysql_error(&conn));
  }

  OK = mysql_real_query(&conn, query4, (ulong)strlen(query4));

  assert(0 == OK);

  printf("%ld inserted\n", (long)mysql_insert_id(&conn));

  OK = mysql_real_query(&conn, query5, (ulong)strlen(query5));

  assert(0 == OK);

  printf("%ld inserted\n", (long)mysql_insert_id(&conn));

  mysql_close(&conn);
  my_end(0);

  return 0;
}
