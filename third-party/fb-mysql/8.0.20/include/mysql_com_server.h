/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file include/mysql_com_server.h
  Definitions private to the server,
  used in the networking layer to notify specific events.
*/

#ifndef _mysql_com_server_h
#define _mysql_com_server_h

#include <stddef.h>

#include "compression.h"
#include "my_inttypes.h"

typedef void (*before_header_callback_fn)(NET *net, void *user_data,
                                          size_t count);

typedef void (*after_header_callback_fn)(NET *net, void *user_data,
                                         size_t count, bool rc);

/**
  This structure holds the negotiated compression algorithm and level
  between client and server.
*/
struct compression_attributes {
  char compress_algorithm[COMPRESSION_ALGORITHM_NAME_LENGTH_MAX];
  unsigned int compress_level;
  bool compression_optional;
  compression_attributes() {
    compress_algorithm[0] = '\0';
    compress_level = 0;
    compression_optional = false;
  }
};

typedef struct NET_SERVER {
  before_header_callback_fn m_before_header{nullptr};
  after_header_callback_fn m_after_header{nullptr};
  void *m_user_data{nullptr};
  struct compression_attributes compression;
  mysql_compress_context compress_ctx;
} NET_SERVER;

#endif
