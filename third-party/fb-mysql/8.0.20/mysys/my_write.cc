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
  @file mysys/my_write.cc
*/

#include "my_config.h"

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
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

#include <algorithm>

extern PSI_stage_info stage_waiting_for_disk_space;

#ifndef _WIN32
// Mock away write() for unit testing.
ssize_t (*mock_write)(int fd, const void *buf, size_t count) = nullptr;
#endif

/**
  Write a chunk of bytes to a file

  if (MyFlags & (MY_NABP | MY_FNABP))
  returns
    0  if Count == 0
    On succes, 0
    On failure, (size_t)-1 == MY_FILE_ERROR

  otherwise
  returns
    0  if Count == 0
    On success, the number of bytes written.
    On partial success (if less than Count bytes could be written),
       the actual number of bytes written.
    On failure, (size_t)-1 == MY_FILE_ERROR
*/
size_t my_write(File Filedes, const uchar *Buffer, size_t Count, myf MyFlags) {
  int64_t sum_written = 0;
  uint errors = 0;
  const size_t initial_count = Count;
  size_t ToWriteCount;

  DBUG_TRACE;

  /* The behavior of write(fd, buf, 0) is not portable */
  if (unlikely(!Count)) return 0;

  DBUG_EXECUTE_IF("simulate_no_free_space_error",
                  { DBUG_SET("+d,simulate_file_write_error"); });
  for (;;) {
    errno = 0;

    ToWriteCount = Count;
    DBUG_EXECUTE_IF("force_wait_for_disk_space", { ToWriteCount = 1; });
    DBUG_EXECUTE_IF("simulate_io_thd_wait_for_disk_space", {
      if (strstr(my_filename(Filedes), "slave-relay-bin.0")) ToWriteCount = 1;
    });
    DBUG_EXECUTE_IF("simulate_random_io_thd_wait_for_disk_space", {
      if (strstr(my_filename(Filedes), "slave-relay-bin.0")) {
        if (rand() % 3 == 0)
          ToWriteCount = Count;
        else
          ToWriteCount = std::min(Count, 1 + (Count * (rand() % 100) / 100));
      }
    });

    int64_t writtenbytes =
#ifdef _WIN32
        my_win_write(Filedes, Buffer, ToWriteCount);
#else
        (mock_write ? mock_write(Filedes, Buffer, ToWriteCount)
                    : write(Filedes, Buffer, ToWriteCount));
#endif
    DBUG_EXECUTE_IF("simulate_file_write_error", {
      errno = ENOSPC;
      writtenbytes = -1;
    });
    DBUG_EXECUTE_IF("force_wait_for_disk_space", { errno = ENOSPC; });
    DBUG_EXECUTE_IF("simulate_io_thd_wait_for_disk_space", {
      if (strstr(my_filename(Filedes), "slave-relay-bin.0")) errno = ENOSPC;
    });
    DBUG_EXECUTE_IF("simulate_random_io_thd_wait_for_disk_space", {
      if (strstr(my_filename(Filedes), "slave-relay-bin.0") &&
          ToWriteCount != Count)
        errno = ENOSPC;
    });
    if (writtenbytes == static_cast<int64_t>(Count)) {
      sum_written += writtenbytes;
      break;
    }
    if (writtenbytes != -1) { /* Safeguard */
      sum_written += writtenbytes;
      Buffer += writtenbytes;
      Count -= writtenbytes;
    }
    set_my_errno(errno);
    if (is_killed_hook(nullptr))
      MyFlags &= ~MY_WAIT_IF_FULL; /* End if aborted by user */

    if ((my_errno() == ENOSPC || my_errno() == EDQUOT) &&
        (MyFlags & MY_WAIT_IF_FULL)) {
      PSI_stage_info old_stage;
      if (MyFlags & MY_REPORT_WAITING_IF_FULL) {
        set_waiting_for_disk_space_hook(nullptr, true);
        enter_stage_hook(nullptr, &stage_waiting_for_disk_space, &old_stage,
                         __func__, __FILE__, __LINE__);
      }
      wait_for_free_space(my_filename(Filedes), errors);
      if (MyFlags & MY_REPORT_WAITING_IF_FULL) {
        enter_stage_hook(nullptr, &old_stage, nullptr, __func__, __FILE__,
                         __LINE__);
        set_waiting_for_disk_space_hook(nullptr, false);
      }

      errors++;
      DBUG_EXECUTE_IF("simulate_no_free_space_error",
                      { DBUG_SET("-d,simulate_file_write_error"); });
      if (is_killed_hook(nullptr)) break;
      continue;
    }

    if (writtenbytes != 0 && writtenbytes != -1 && !is_killed_hook(nullptr))
      continue; /* Retry if something written */

    if (my_errno() == EINTR) {
      continue;                                /* Interrupted, retry */
    } else if (writtenbytes == 0 && !errors++) /* Retry once */
    {
      /* We may come here if the file quota is exeeded */
      continue;
    }
    break;
  }
  if (MyFlags & (MY_NABP | MY_FNABP)) {
    if (sum_written == static_cast<int64_t>(initial_count))
      return 0; /* Want only errors, not bytes written */
    if (MyFlags & (MY_WME | MY_FAE | MY_FNABP)) {
      MyOsError(my_errno(), EE_WRITE, MYF(0), my_filename(Filedes));
    }
    return MY_FILE_ERROR;
  }

  if (sum_written == 0) return MY_FILE_ERROR;

  return sum_written;
}
