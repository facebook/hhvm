/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_fallocator.cc
*/

#include "my_config.h"

#include <errno.h>
#ifdef HAVE_POSIX_FALLOCATE
#include <fcntl.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <limits>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"
#ifdef _WIN32
#include "mysys/mysys_priv.h"
#endif

/** Change size of the specified file. Forces the OS to reserve disk space for
the file, even when called to fill with zeros. The function also changes the
file position. Usually it points to the end of the file after execution.

@note Implementation based on, and mostly copied from, my_chsize. But instead
of ftruncate uses posix_fallocate. This implementation was needed because of
allocation in temptable. Probably this implementation should replace my_chsize
implementation.

@param[in] fd File descriptor
@param[in] newlength New file size
@param[in] filler Fill up all bytes after new_length with this character
@param[in] MyFlags Flags
@return 0 if OK, 1 otherwise
*/
int my_fallocator(File fd, my_off_t newlength, int filler, myf MyFlags) {
  my_off_t oldsize;
  uchar buff[IO_SIZE];
  DBUG_TRACE;

  if ((oldsize = my_seek(fd, 0L, MY_SEEK_END, MYF(MY_WME + MY_FAE))) ==
      newlength)
    return 0;

  if (oldsize > newlength) {
#ifdef _WIN32
    if (my_win_chsize(fd, newlength)) {
      set_my_errno(errno);
      goto err;
    }
    return 0;
#elif defined(HAVE_POSIX_FALLOCATE)
    if (posix_fallocate(fd, 0, newlength) != 0) {
      set_my_errno(errno);
      goto err;
    }
    return 0;
#else
    /*
    Fill space between requested length and true length with 'filler'
    We should never come here on any modern machine
    */
    if (my_seek(fd, newlength, MY_SEEK_SET, MYF(MY_WME + MY_FAE)) ==
        MY_FILEPOS_ERROR) {
      goto err;
    }
    std::swap(newlength, oldsize);
#endif  // _WIN32
  }

  /* Full file with 'filler' until it's as big as requested */
  memset(buff, filler, IO_SIZE);
  while (newlength - oldsize > IO_SIZE) {
    if (my_write(fd, buff, IO_SIZE, MYF(MY_NABP))) goto err;
    oldsize += IO_SIZE;
  }

  if (my_write(fd, buff, static_cast<size_t>(newlength - oldsize),
               MYF(MY_NABP)))
    goto err;
  return 0;

err:
  if (MyFlags & MY_WME) {
    MyOsError(my_errno(), EE_CANT_CHSIZE, MYF(0));
  }
  return 1;
}
