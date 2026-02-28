/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file mysys/mf_cache.cc
  Open a temporary file and cache it with io_cache. Delete it on close.
*/

#include <fcntl.h>
#include <stddef.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"

/*
** Open tempfile cached by IO_CACHE
** Should be used when no seeks are done (only reinit_io_buff)
** Return 0 if cache is inited ok
** The actual file is created when the IO_CACHE buffer gets filled
** If dir is not given, use TMPDIR.
*/

bool open_cached_file(IO_CACHE *cache, const char *dir, const char *prefix,
                      size_t cache_size, myf cache_myflags) {
  DBUG_TRACE;
  cache->dir =
      dir ? my_strdup(key_memory_IO_CACHE, dir, MYF(cache_myflags & MY_WME))
          : (char *)nullptr;
  cache->prefix = (prefix ? my_strdup(key_memory_IO_CACHE, prefix,
                                      MYF(cache_myflags & MY_WME))
                          : (char *)nullptr);
  cache->file_name = nullptr;
  cache->buffer = nullptr; /* Mark that not open */
  if (!init_io_cache(cache, -1, cache_size, WRITE_CACHE, 0L, false,
                     MYF(cache_myflags | MY_NABP))) {
    return false;
  }
  my_free(cache->dir);
  my_free(cache->prefix);
  return true;
}

/* Create the temporary file */

bool real_open_cached_file(IO_CACHE *cache) {
  char name_buff[FN_REFLEN];
  int error = 1;
  DBUG_TRACE;
  if ((cache->file = mysql_file_create_temp(
           cache->file_key, name_buff, cache->dir, cache->prefix,
           (O_RDWR | O_TRUNC), UNLINK_FILE, MYF(MY_WME))) >= 0) {
    error = 0;
  }
  return error;
}

void close_cached_file(IO_CACHE *cache) {
  DBUG_TRACE;
  if (my_b_inited(cache)) {
    File file = cache->file;
    cache->file = -1; /* Don't flush data */
    (void)end_io_cache(cache);
    if (file >= 0) {
      (void)mysql_file_close(file, MYF(0));
    }
    my_free(cache->dir);
    my_free(cache->prefix);
    DBUG_ASSERT(cache->reported_disk_usage == 0);
  }
}
