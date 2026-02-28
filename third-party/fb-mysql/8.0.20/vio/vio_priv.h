/* Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef VIO_PRIV_INCLUDED
#define VIO_PRIV_INCLUDED

/**
  @file vio/vio_priv.h
  Structures and functions private to the vio package
*/

#define DONT_MAP_VIO
#include <mysql_com.h>

#include "m_string.h"
#include "my_sys.h"
#include "mysql/psi/psi_memory.h"
#include "violite.h"

extern PSI_memory_key key_memory_vio;
extern PSI_memory_key key_memory_vio_read_buffer;
extern PSI_memory_key key_memory_vio_ssl_fd;

#ifdef _WIN32
size_t vio_read_pipe(Vio *vio, uchar *buf, size_t size);
size_t vio_write_pipe(Vio *vio, const uchar *buf, size_t size);
bool vio_is_connected_pipe(Vio *vio);
int vio_shutdown_pipe(Vio *vio);

size_t vio_read_shared_memory(Vio *vio, uchar *buf, size_t size);
size_t vio_write_shared_memory(Vio *vio, const uchar *buf, size_t size);
bool vio_is_connected_shared_memory(Vio *vio);
int vio_shutdown_shared_memory(Vio *vio);
void vio_delete_shared_memory(Vio *vio);
#endif /* _WIN32 */

bool vio_buff_has_data(Vio *vio);
int vio_socket_io_wait(Vio *vio, enum enum_vio_io_event event);
int vio_socket_timeout(Vio *vio, uint which, bool old_mode);

size_t vio_ssl_read(Vio *vio, uchar *buf, size_t size);
size_t vio_ssl_write(Vio *vio, const uchar *buf, size_t size);

/* When the workday is over... */
int vio_ssl_shutdown(Vio *vio);
void vio_ssl_delete(Vio *vio);
bool vio_ssl_has_data(Vio *vio);

#endif /* VIO_PRIV_INCLUDED */
