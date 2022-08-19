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

/* Functions to map mysqld errno to sql_state */

#include <mysqld_error.h>
#include <sys/types.h>

#include "m_string.h"
#include "my_inttypes.h"  // IWYU pragma: keep
#include "mysql_com.h"    // IWYU pragma: keep
#include "mysys_err.h"    // EE*
#include "sql/derror.h"
#include "storage/perfschema/pfs_error.h"

extern server_error error_names_array[];
static server_error *sqlstate_map = &error_names_array[1];

static const char *builtin_get_sqlstate(int i) {
  return (i < 0) ? "HY000" : sqlstate_map[i].odbc_state;
}

const char *mysql_errno_to_sqlstate(uint mysql_errno) {
  return builtin_get_sqlstate(mysql_errno_to_builtin(mysql_errno));
}

static const char *builtin_get_symbol(int i) {
  return (i < 0) ? nullptr : sqlstate_map[i].name;
}

const char *mysql_errno_to_symbol(int mysql_errno) {
  return builtin_get_symbol(mysql_errno_to_builtin(mysql_errno));
}

int mysql_symbol_to_errno(const char *error_symbol) {
  int offset = 0;  // Position where the current section starts in the array.
  int i, j;

  // accept "MY-123456"-style pseudo-symbols ...
  if (0 == native_strncasecmp("MY-", error_symbol, 3)) {
    // ... but only if there is a message registered for that number!
    char *last;
    int errcode = (int)strtol(&error_symbol[3], &last, 10);
    if ((*last == '\0') &&
        (((errcode >= EE_ERROR_FIRST) && (errcode <= EE_ERROR_LAST)) ||
         (mysql_errno_to_builtin(errcode) >= 0)))
      return errcode;
    return -1;
  }

  for (i = 0; i < NUM_SECTIONS; i++) {
    for (j = 0; j < errmsg_section_size[i]; j++) {
      if (!native_strcasecmp(error_symbol, sqlstate_map[j + offset].name))
        return sqlstate_map[j + offset].mysql_errno;
    }
    offset += errmsg_section_size[i];
  }
  return -1; /* General error */
}
