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
  @file mysys/my_fstream.cc
*/

#include "my_config.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_thread_local.h"  // set_my_errno
#include "mysys_err.h"
#include "template_utils.h"  // pointer_cast
#if defined(_WIN32)
#include "mysys/mysys_priv.h"  // my_win_x
#endif

namespace {
/**
   Portable fseek() wrapper (without the modified semantics of my_fseek()).
*/
int64_t fseek_(FILE *stream, int64_t offset, int whence) {
#ifdef _WIN32
  return _fseeki64(stream, offset, whence);
#else
  return fseeko(stream, offset, whence);
#endif /* _WIN32 */
}
}  // namespace

/**
   Read a chunk of bytes from a FILE stream.

   @param stream  Source
   @param Buffer  Destination
   @param Count	  Number of bytes to read
   @param MyFlags Flags for error handling

   @retval Number of bytes read
   @retval MY_FILE_ERROR in case of errors
 */
size_t my_fread(FILE *stream, uchar *Buffer, size_t Count, myf MyFlags) {
  size_t readbytes;
  DBUG_TRACE;
  if ((readbytes = fread(Buffer, sizeof(char), Count, stream)) != Count) {
    if (MyFlags & (MY_WME | MY_FAE | MY_FNABP)) {
      if (ferror(stream)) {
        MyOsError(my_errno(), EE_READ, MYF(0), my_filename(my_fileno(stream)));
      } else if (MyFlags & (MY_NABP | MY_FNABP)) {
        MyOsError(errno, EE_EOFERR, MYF(0), my_filename(my_fileno(stream)));
      }
    }
    set_my_errno(errno ? errno : -1);
    if (ferror(stream) || MyFlags & (MY_NABP | MY_FNABP))
      return MY_FILE_ERROR; /* Return with error */
  }
  if (MyFlags & (MY_NABP | MY_FNABP)) return 0; /* Read ok */
  return readbytes;
}

/**
   Write a chunk of bytes to a FILE stream.

   @param stream	  Destination
   @param Buffer   Source
   @param Count	  Number of bytes to write
   @param MyFlags  Flags for error handling

   @retval Number of bytes written
   @retval MY_FILE_ERROR in case of errors
*/
size_t my_fwrite(FILE *stream, const uchar *Buffer, size_t Count, myf MyFlags) {
  size_t writtenbytes = 0;
  int64_t seekptr;

  DBUG_TRACE;
  DBUG_EXECUTE_IF("simulate_fwrite_error", return -1;);

  seekptr = my_ftell(stream);
  for (;;) {
    errno = 0;
    size_t written =
        fwrite(pointer_cast<const char *>(Buffer), sizeof(char), Count, stream);
    if (written != Count) {
      set_my_errno(errno);

      DBUG_ASSERT(written != MY_FILE_ERROR);
      seekptr += written;
      Buffer += written;
      writtenbytes += written;
      Count -= written;

      if (errno == EINTR) {
        fseek_(stream, seekptr, MY_SEEK_SET);
        continue;
      }
      if (ferror(stream) || (MyFlags & (MY_NABP | MY_FNABP))) {
        if (MyFlags & (MY_WME | MY_FAE | MY_FNABP)) {
          MyOsError(errno, EE_WRITE, MYF(0), my_filename(my_fileno(stream)));
        }
        writtenbytes = MY_FILE_ERROR; /* Return that we got error */
        break;
      }
    }
    if (MyFlags & (MY_NABP | MY_FNABP))
      writtenbytes = 0; /* Everything OK */
    else
      writtenbytes += written;
    break;
  }
  return writtenbytes;
}

/**
   Seek to position in FILE stream. Note that the semantics differ from
   normal fseek() in that it returns the new position, and not just
   0/-1 to indicate success/failure.

   @param stream to seek in
   @param pos offset into stream
   @param whence where to seek from

   @retval new offset in stream
   @retval MY_FILEPOS_ERROR in case of errors
*/
my_off_t my_fseek(FILE *stream, my_off_t pos, int whence) {
  DBUG_TRACE;

  return fseek_(stream, pos, whence) ? MY_FILEPOS_ERROR : my_ftell(stream);
}

/**
   Portable ftell() wrapper.
*/
my_off_t my_ftell(FILE *stream) {
  int64_t pos;
  DBUG_TRACE;

#ifdef _WIN32
  pos = _ftelli64(stream);
#else
  pos = ftello(stream);
#endif /* _WIN32 */
  return pos;
}

/**
   Portable fileno() wrapper.
*/
File my_fileno(FILE *f) {
#ifdef _WIN32
  return my_win_fileno(f);
#else
  return fileno(f);
#endif
}
