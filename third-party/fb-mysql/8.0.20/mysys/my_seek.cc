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
  @file mysys/my_seek.cc
*/

#include "my_config.h"

#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"
#if defined(_WIN32)
#include "mysys/mysys_priv.h"
#endif

/**
  Seek to a position in a file. The my_seek  function  is a wrapper around
  the system call lseek and repositions  the  offset of the file descriptor
  fd to the argument offset according to the directive whence as follows:
      SEEK_SET    The offset is set to offset bytes.
      SEEK_CUR    The offset is set to its current location plus offset bytes
      SEEK_END    The offset is set to the size of the file plus offset bytes

  @param fd      The file descriptor
  @param pos     The expected position (absolute or relative)
  @param whence  A direction parameter and one of {SEEK_SET, SEEK_CUR, SEEK_END}
  @param MyFlags flags to control error handling.

  @retval newpos            The new position in the file.
  @retval MY_FILEPOS_ERROR  An error was encountered while performing
                            the seek. my_errno is set to indicate the
                            actual error.
*/

my_off_t my_seek(File fd, my_off_t pos, int whence, myf MyFlags) {
  DBUG_TRACE;

  /*
      Make sure we are using a valid file descriptor!
  */
  DBUG_ASSERT(fd != -1);
  const int64_t newpos =
#if defined(_WIN32)
      my_win_lseek(fd, pos, whence);
#else
      lseek(fd, pos, whence);
#endif
  if (newpos == -1) {
    set_my_errno(errno);
    if (MyFlags & MY_WME) {
      MyOsError(my_errno(), EE_CANT_SEEK, MYF(0), my_filename(fd));
    }
    return MY_FILEPOS_ERROR;
  }
  DBUG_ASSERT(newpos >= 0);
  return newpos;
}

/* Tell current position of file */
my_off_t my_tell(File fd, myf MyFlags) {
  DBUG_TRACE;
  DBUG_ASSERT(fd >= 0);

  const int64_t pos =
#if defined(HAVE_TELL) && !defined(_WIN32)
      tell(fd);
#else
      my_seek(fd, 0L, MY_SEEK_CUR, 0);
#endif
  if (pos == -1) {
    set_my_errno(errno);
    if (MyFlags & MY_WME) {
      MyOsError(my_errno(), EE_CANT_SEEK, MYF(0), my_filename(fd));
    }

    return MY_FILEPOS_ERROR;
  }
  DBUG_ASSERT(pos >= 0);
  return pos;
}
