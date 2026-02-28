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
  @file mysys/my_create.cc
*/

#include <fcntl.h>

#include "my_sys.h"
#if defined(_WIN32)
#include <share.h>
#endif

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_thread_local.h"
#include "mysys_err.h"
#include "mysys_priv.h"  // FILE_BY_CREATE
#if defined(_WIN32)
#include "mysys/mysys_priv.h"
#endif

/**
   Create a new file.

   @param FileName     Path-name of file
   @param CreateFlags  Read | write on file (umask value)
   @param AccessFlags  Read & Write on open file
   @param MyFlags      Special flags

   @retval File descriptor on Posix
   @retval FileInfo index on Windows.
   @retval -1 in case of errors.
*/

File my_create(const char *FileName, int CreateFlags, int AccessFlags,
               myf MyFlags) {
  DBUG_TRACE;

  File fd = -1;
#if defined(_WIN32)
  fd = my_win_open(FileName, AccessFlags | O_CREAT);
#else
  fd = mysys_priv::RetryOnEintr(
      [&]() {
        return open(FileName, AccessFlags | O_CREAT,
                    CreateFlags ? CreateFlags : my_umask);
      },
      -1);
#endif
  if (fd < 0) {
    set_my_errno(errno);
    if (MyFlags & (MY_FAE | MY_WME)) {
      MyOsError(my_errno(), EE_CANTCREATEFILE, MYF(0), FileName);
    }
    return -1;
  }

  file_info::RegisterFilename(fd, FileName,
                              file_info::OpenType::FILE_BY_CREATE);
  return fd;
}
