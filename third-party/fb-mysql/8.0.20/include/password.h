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

#ifndef PASSWORD_INCLUDED
#define PASSWORD_INCLUDED

/**
  @file include/password.h
*/

#include <stddef.h>
#include <sys/types.h>

#include "my_macros.h"

struct rand_struct *get_sql_rand();

// extern "C" since it is an (undocumented) part of the libmysql ABI.
extern "C" void my_make_scrambled_password(char *to, const char *password,
                                           size_t pass_len);
void my_make_scrambled_password_sha1(char *to, const char *password,
                                     size_t pass_len);

void hash_password(ulong *result, const char *password, uint password_len);

#endif /* PASSWORD_INCLUDED */
