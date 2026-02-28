/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_open.cc
*/

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>
#include <iostream>
#include <thread>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"

/**
  Open a file.

  @param filename   Fully qualified file name
  @param Flags      Read | write
  @param MyFlags    Special flags

  @retval File descriptor if successful
  @retval -1 in case of errors
*/

File my_open(const char *filename, int Flags, myf MyFlags) {
  DBUG_TRACE;

  File fd = -1;
#if defined(_WIN32)
  fd = my_win_open(filename, Flags);
#else
  fd = mysys_priv::RetryOnEintr(
      [&]() { return open(filename, Flags, my_umask); }, -1);
#endif

  if (fd < 0) {
    set_my_errno(errno);
    DBUG_PRINT("error", ("Got error %d on open", my_errno()));
    if (MyFlags & (MY_FAE | MY_WME)) {
      MyOsError(my_errno(), EE_FILENOTFOUND, MYF(0), filename);
    }
    return fd;
  }
  RegisterFilename(fd, filename, file_info::OpenType::FILE_BY_OPEN);
  return fd;
}

/**
  Close a file.

  @param fd   File sescriptor
  @param MyFlags  Special Flags

  @retval 0 if successful
  @retval -1 in case of errors
*/

int my_close(File fd, myf MyFlags) {
  DBUG_TRACE;

  // Save the filename before unregistering, in case we need to report
  // error from close()
  std::string fname = my_filename(fd);

  // Need to remove file_info entry first to avoid race with another
  // thread reusing this fd after it has been closed.
  file_info::UnregisterFilename(fd);

  int err = -1;
#ifndef _WIN32
  err = mysys_priv::RetryOnEintr([&fd]() { return close(fd); }, -1);
#else
  err = my_win_close(fd);
#endif
  if (err == -1) {
    DBUG_PRINT("error", ("Got error %d on close", err));
    set_my_errno(errno);
    if (MyFlags & (MY_FAE | MY_WME)) {
      MyOsError(my_errno(), EE_BADCLOSE, MYF(0), fname.c_str());
    }
  }
  return err;
}
