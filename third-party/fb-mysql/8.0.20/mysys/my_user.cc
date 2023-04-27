/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2.0,
 as published by the Free Software Foundation.

 This program is also distributed with certain software (including
 but not limited to OpenSSL) that is licensed under separate terms,
 as designated in a particular file or component or in included license
 documentation.  The authors of MySQL hereby grant you an additional
 permission to link the program and your derivative works with the
 separately licensed software that they have included with MySQL.

 Without limiting anything contained in the foregoing, this file,
 which is part of C Driver for MySQL (Connector/C), is also subject to the
 Universal FOSS Exception, version 1.0, a copy of which can be found at
 http://oss.oracle.com/licenses/universal-foss-exception.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License, version 2.0, for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "my_user.h"
#include "m_string.h"
#include "my_hostname.h"  // HOSTNAME_LENGTH
#include "mysql_com.h"    // USERNAME_LENGTH

/*
  Parse user value to user name and host name parts.

  SYNOPSIS
    user_id_str     [IN]  User value string (the source).
    user_id_len     [IN]  Length of the user value.
    user_name_str   [OUT] Buffer to store user name part.
                          Must be not less than USERNAME_LENGTH + 1.
    user_name_len   [OUT] A place to store length of the user name part.
    host_name_str   [OUT] Buffer to store host name part.
                          Must be not less than HOSTNAME_LENGTH + 1.
    host_name_len   [OUT] A place to store length of the host name part.
*/

void parse_user(const char *user_id_str, size_t user_id_len,
                char *user_name_str, size_t *user_name_len, char *host_name_str,
                size_t *host_name_len) {
  const char *p = strrchr(user_id_str, '@');

  if (!p) {
    *user_name_len = 0;
    *host_name_len = 0;
  } else {
    *user_name_len = (uint)(p - user_id_str);
    *host_name_len = (uint)(user_id_len - *user_name_len - 1);

    if (*user_name_len > USERNAME_LENGTH) *user_name_len = USERNAME_LENGTH;

    if (*host_name_len > HOSTNAME_LENGTH) *host_name_len = HOSTNAME_LENGTH;

    memcpy(user_name_str, user_id_str, *user_name_len);
    memcpy(host_name_str, p + 1, *host_name_len);
  }

  user_name_str[*user_name_len] = 0;
  host_name_str[*host_name_len] = 0;
}
