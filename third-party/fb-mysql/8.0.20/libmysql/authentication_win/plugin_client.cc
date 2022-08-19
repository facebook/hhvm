/* Copyright (c) 2011, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <mysql.h>
#include <mysql/client_plugin.h>

#include "common.h"

/*
  The following MS C++ specific pragma embeds a comment in the resulting
  object file. A "lib" comment tells the linker to use the specified
  library, thus the dependency is handled automagically.
*/

#ifdef _MSC_VER
#pragma comment(lib, "Secur32")
#endif

static int win_auth_client_plugin_init(char *, size_t, int, va_list) {
  return 0;
}

static int win_auth_client_plugin_deinit() { return 0; }

int win_auth_handshake_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql);

/*
  Client plugin declaration. This is added to mysql_client_builtins[]
  in sql-common/client.c
*/

extern "C" auth_plugin_t win_auth_client_plugin = {
    MYSQL_CLIENT_AUTHENTICATION_PLUGIN,
    MYSQL_CLIENT_AUTHENTICATION_PLUGIN_INTERFACE_VERSION,
    "authentication_windows_client",
    "Rafal Somla",
    "Windows Authentication Plugin - client side",
    {0, 1, 0},
    "GPL",
    NULL,
    win_auth_client_plugin_init,
    win_auth_client_plugin_deinit,
    NULL,  // option handling
    win_auth_handshake_client};
