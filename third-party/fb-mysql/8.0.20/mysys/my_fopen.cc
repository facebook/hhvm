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
  @file mysys/my_fopen.cc
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"

namespace {
constexpr FILE *nullstream = nullptr;

/**
   Make a fopen() typestring from a open() type bitmap.
   This routine attempts to find the best possible match
   between  a numeric option and a string option that could be
   fed to fopen. There is not a 1 to 1 mapping between the two.

   MAPPING
   r  == O_RDONLY
   w  == O_WRONLY|O_TRUNC|O_CREAT
   a  == O_WRONLY|O_APPEND|O_CREAT
   r+ == O_RDWR
   w+ == O_RDWR|O_TRUNC|O_CREAT
   a+ == O_RDWR|O_APPEND|O_CREAT

   @param to	  String for fopen() is stored here
   @param flag  Flag used by open()

   @note On Unix, O_RDONLY is usually 0
*/

void make_ftype(char *to, int flag) {
  /* check some possible invalid combinations */
  DBUG_ASSERT((flag & (O_TRUNC | O_APPEND)) != (O_TRUNC | O_APPEND));
  DBUG_ASSERT((flag & (O_WRONLY | O_RDWR)) != (O_WRONLY | O_RDWR));

  if ((flag & (O_RDONLY | O_WRONLY)) == O_WRONLY)
    *to++ = (flag & O_APPEND) ? 'a' : 'w';
  else if (flag & O_RDWR) {
    /* Add '+' after these */
    if (flag & (O_TRUNC | O_CREAT))
      *to++ = 'w';
    else if (flag & O_APPEND)
      *to++ = 'a';
    else
      *to++ = 'r';
    *to++ = '+';
  } else
    *to++ = 'r';

  if (flag & MY_FOPEN_BINARY) *to++ = 'b';

  *to = '\0';
}
}  // namespace

/**
  Open a file as stream.

  @param filename   Path-name of file
  @param flags	    Read | write | append | trunc (like for open())
  @param MyFlags    Flags for handling errors

  @retval nullptr in case of errors
  @retval FILE pointer otherwise
*/

FILE *my_fopen(const char *filename, int flags, myf MyFlags) {
  DBUG_TRACE;

  char type[5];
  make_ftype(type, flags);

  FILE *stream = nullptr;
#ifdef _WIN32
  stream = my_win_fopen(filename, type);
#else
  stream = mysys_priv::RetryOnEintr([&]() { return fopen(filename, type); },
                                    nullstream);
#endif

  if (stream == nullptr) {
    set_my_errno(errno);
    DBUG_PRINT("error", ("Got error %d on open", my_errno()));
    if (MyFlags & (MY_FAE | MY_WME)) {
      MyOsError(my_errno(),
                ((flags & O_RDONLY) || (flags == O_RDONLY) ? EE_FILENOTFOUND
                                                           : EE_CANTCREATEFILE),
                MYF(0), filename);
    }
    return nullptr;
  }

  File fd = my_fileno(stream);
  file_info::RegisterFilename(fd, filename,
                              file_info::OpenType::STREAM_BY_FOPEN);

  return stream;
}

/**
  Change the file associated with a file stream.

  @param filename   Path to file.
  @param mode   Mode of the stream.
  @param stream File stream.

  @note
    This function is used to redirect stdout and stderr to a file and
    subsequently to close and reopen that file for log rotation.

  @retval A FILE pointer on success. Otherwise, NULL.
*/

FILE *my_freopen(const char *filename, const char *mode, FILE *stream) {
#if defined(_WIN32)
  return my_win_freopen(filename, mode, stream);
#else
  return mysys_priv::RetryOnEintr(
      [&]() { return freopen(filename, mode, stream); }, nullstream);
#endif
}

/**
   Close a stream.

   @param stream   FILE stream to close.
   @param MyFlags  Flags controlling error reporting.

   @retval 0 on success
   @retval -1 on error
*/

int my_fclose(FILE *stream, myf MyFlags) {
  DBUG_TRACE;

  File fd = my_fileno(stream);

  // Store the filename before unregistering, so that it can be
  // reported if close() fails.
  std::string fname = my_filename(fd);

  // Need to remove file_info entry first to avoid race with another
  // thread reusing this fd after it has been closed.
  file_info::UnregisterFilename(fd);

  int err = -1;
#ifndef _WIN32
  err = mysys_priv::RetryOnEintr([&]() { return fclose(stream); }, -1);
#else
  err = my_win_fclose(stream);
#endif
  if (err < 0) {
    set_my_errno(errno);
    if (MyFlags & (MY_FAE | MY_WME)) {
      MyOsError(my_errno(), EE_BADCLOSE, MYF(0), fname.c_str());
    }
  }

  return err;
}

/**
   Make a stream out of a file handle.

   @param fd       File descriptor to open a stream to.
   @param filename Name of file to which the fd refers. May be nullptr.
   @param flags    Numeric open mode flags (will be converted to string and
                   passed to fdopen)
   @param MyFlags  Flags for error handling

   @retval nullptr in case of errors
   @retval FILE stream if successful
*/

FILE *my_fdopen(File fd, const char *filename, int flags, myf MyFlags) {
  DBUG_TRACE;

  char type[5];
  make_ftype(type, flags);

  FILE *stream = nullptr;
#ifdef _WIN32
  stream = my_win_fdopen(fd, type);
#else
  stream =
      mysys_priv::RetryOnEintr([&]() { return fdopen(fd, type); }, nullstream);
#endif

  if (stream == nullptr) {
    set_my_errno(errno);
    if (MyFlags & (MY_FAE | MY_WME)) {
      MyOsError(my_errno(), EE_CANT_OPEN_STREAM, MYF(0));
    }
    return nullptr;
  }

  file_info::RegisterFilename(fd, filename,
                              file_info::OpenType::STREAM_BY_FDOPEN);
  return stream;
}
