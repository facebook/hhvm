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
  @file mysys/my_read.cc
*/

#include "my_config.h"

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"
#ifdef _WIN32
#include "mysys/mysys_priv.h"
#endif

#ifndef _WIN32
// Mock away read() for unit testing.
ssize_t (*mock_read)(int fd, void *buf, size_t count) = nullptr;
#endif

/**
   Read a chunk of bytes from a file with retry's if needed
   If flag MY_FULL_IO is set then keep reading until EOF is found.

   @param       fd       File descriptor to read from
   @param[out]  Buffer   Buffer to hold at least Count bytes
   @param       Count    Bytes to read
   @param       MyFlags  Flags on what to do on error

   @return Operation status
     @retval  -1  on error
     @retval   0  if flag has bits MY_NABP or MY_FNABP set
     @retval   N  number of bytes read
*/

size_t my_read(File fd, uchar *Buffer, size_t Count, myf MyFlags) {
  int64_t savedbytes = 0;
  DBUG_TRACE;

  for (;;) {
    errno = 0; /* Linux, Windows don't reset this on EOF/success */
    int64_t readbytes =
#ifdef _WIN32
        // Using my_win_pread() with offset -1 which will cause
        // ReadFile() to be called with nullptr for the OVERLAPPED
        // argument. This way we avoid having both my_win_read()
        // my_win_pread() which were identical except for the OVERLAPPED
        // arg passed to ReadFile().
        my_win_pread(fd, Buffer, Count, -1);
#else
        (mock_read ? mock_read(fd, Buffer, Count) : read(fd, Buffer, Count));
#endif
    DBUG_EXECUTE_IF("simulate_file_read_error", {
      errno = ENOSPC;
      readbytes = -1;
      DBUG_SET("-d,simulate_file_read_error");
      DBUG_SET("-d,simulate_my_b_fill_error");
    });

    if (readbytes != static_cast<int64_t>(Count)) {
      set_my_errno(errno);
      if (errno == 0 || (readbytes != -1 && (MyFlags & (MY_NABP | MY_FNABP))))
        set_my_errno(HA_ERR_FILE_TOO_SHORT);

      if ((readbytes == 0 || readbytes == -1) && errno == EINTR) {
        continue; /* Interrupted */
      }

      if (MyFlags & (MY_WME | MY_FAE | MY_FNABP)) {
        if (readbytes == -1)
          MyOsError(my_errno(), EE_READ, MYF(0), my_filename(fd));
        else if (MyFlags & (MY_NABP | MY_FNABP))
          MyOsError(my_errno(), EE_EOFERR, MYF(0), my_filename(fd));
      }
      if (readbytes == -1 ||
          ((MyFlags & (MY_FNABP | MY_NABP)) && !(MyFlags & MY_FULL_IO)))
        return MY_FILE_ERROR; /* Return with error */
      /* readbytes == 0 when EOF. No need to continue in case of EOF */
      if (readbytes != 0 && (MyFlags & MY_FULL_IO)) {
        Buffer += readbytes;
        Count -= readbytes;
        savedbytes += readbytes;
        continue;
      }
    }

    if (MyFlags & (MY_NABP | MY_FNABP))
      readbytes = 0; /* Ok on read */
    else if (MyFlags & MY_FULL_IO)
      readbytes += savedbytes;

    return readbytes;
  }  // for (;;)
}
