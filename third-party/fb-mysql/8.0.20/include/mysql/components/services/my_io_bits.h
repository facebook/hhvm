/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_MY_IO_BITS_H
#define COMPONENTS_SERVICES_MY_IO_BITS_H

/**
  @file mysql/components/services/my_io_bits.h
  Types to make file and socket I/O compatible.
*/

#ifdef _WIN32
/* Include common headers.*/
#include <io.h> /* access(), chmod() */
#ifdef WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h> /* SOCKET */
#endif
#endif

#ifndef MYSQL_ABI_CHECK
#if !defined(_WIN32)
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <errno.h>
#include <limits.h>
#include <sys/types.h>  // Needed for mode_t, so IWYU pragma: keep.
#endif

typedef int File; /* File descriptor */
#ifdef _WIN32
typedef int MY_MODE;
typedef int mode_t;
typedef int socket_len_t;
typedef SOCKET my_socket;
#else
typedef mode_t MY_MODE;
typedef socklen_t socket_len_t;
typedef int my_socket; /* File descriptor for sockets */
#endif /* _WIN32 */

#endif /* COMPONENTS_SERVICES_MY_IO_BITS_H */
