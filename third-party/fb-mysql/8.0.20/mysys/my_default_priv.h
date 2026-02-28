/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_DEFAULT_PRIV_INCLUDED
#define MY_DEFAULT_PRIV_INCLUDED

/**
  @file mysys/my_default_priv.h
*/

#include "my_macros.h"

/*
  Number of byte used to store the length of
  cipher that follows.
*/
#define MAX_CIPHER_STORE_LEN 4U
#define LOGIN_KEY_LEN 20U

/**
  Place the login file name in the specified buffer.

  @param [out] file_name       Buffer to hold login file name
  @param [in] file_name_size   Length of the buffer

  @return 1 - Success
          0 - Failure
*/
int my_default_get_login_file(char *file_name, size_t file_name_size);

#endif /* MY_DEFAULT_PRIV_INCLUDED */
