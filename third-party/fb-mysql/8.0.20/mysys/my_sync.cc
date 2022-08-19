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
  @file mysys/my_sync.cc
*/

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"
#if defined(_WIN32)
#include "mysys/mysys_priv.h"
#endif

static void (*before_sync_wait)(void) = nullptr;
static void (*after_sync_wait)(void) = nullptr;

void thr_set_sync_wait_callback(void (*before_wait)(void),
                                void (*after_wait)(void)) {
  before_sync_wait = before_wait;
  after_sync_wait = after_wait;
}

/*
  Sync data in file to disk

  SYNOPSIS
    my_sync()
    fd			File descritor to sync
    my_flags		Flags (now only MY_WME is supported)

  NOTE
    If file system supports its, only file data is synced, not inode data.

    MY_IGNORE_BADFD is useful when fd is "volatile" - not protected by a
    mutex. In this case by the time of fsync(), fd may be already closed by
    another thread, or even reassigned to a different file. With this flag -
    MY_IGNORE_BADFD - such a situation will not be considered an error.
    (which is correct behaviour, if we know that the other thread synced the
    file before closing)

  RETURN
    0 ok
    -1 error
*/

int my_sync(File fd, myf my_flags) {
  int res;
  DBUG_TRACE;
  DBUG_PRINT("my", ("Fd: %d  my_flags: %d", fd, my_flags));

  if (before_sync_wait) (*before_sync_wait)();
  do {
#if defined(HAVE_FDATASYNC) && defined(HAVE_DECL_FDATASYNC)
    res = fdatasync(fd);
#elif defined(HAVE_FSYNC)
    res = fsync(fd);
#elif defined(_WIN32)
    res = my_win_fsync(fd);
#else
#error Cannot find a way to sync a file, durability in danger
    res = 0; /* No sync (strange OS) */
#endif
  } while (res == -1 && errno == EINTR);

  if (res) {
    int er = errno;
    set_my_errno(er);
    if (!er) set_my_errno(-1); /* Unknown error */
    if (after_sync_wait) (*after_sync_wait)();
    if ((my_flags & MY_IGNORE_BADFD) &&
        (er == EBADF || er == EINVAL || er == EROFS
#ifdef __APPLE__
         || er == ENOTSUP
#endif
         )) {
      DBUG_PRINT("info", ("ignoring errno %d", er));
      res = 0;
    } else if (my_flags & MY_WME) {
      MyOsError(my_errno(), EE_SYNC, MYF(0), my_filename(fd));
    }
  } else {
    if (after_sync_wait) (*after_sync_wait)();
  }
  return res;
}
