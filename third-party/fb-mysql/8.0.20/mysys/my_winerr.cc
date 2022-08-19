/* Copyright (c) 2008, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_winerr.cc
  Convert Windows API error (GetLastError() to Posix equivalent (errno).
  The exported function  my_osmaperr() is modelled after and borrows
  heavily from undocumented _dosmaperr()(found of the static Microsoft C
  runtime).
*/

#include <errno.h>
#include <my_sys.h>
#include "my_thread_local.h"

struct errentry {
  unsigned long oscode; /* OS return value */
  int sysv_errno;       /* System V error code */
};

static struct errentry errtable[] = {
    {ERROR_INVALID_FUNCTION, EINVAL},      /* 1 */
    {ERROR_FILE_NOT_FOUND, ENOENT},        /* 2 */
    {ERROR_PATH_NOT_FOUND, ENOENT},        /* 3 */
    {ERROR_TOO_MANY_OPEN_FILES, EMFILE},   /* 4 */
    {ERROR_ACCESS_DENIED, EACCES},         /* 5 */
    {ERROR_INVALID_HANDLE, EBADF},         /* 6 */
    {ERROR_ARENA_TRASHED, ENOMEM},         /* 7 */
    {ERROR_NOT_ENOUGH_MEMORY, ENOMEM},     /* 8 */
    {ERROR_INVALID_BLOCK, ENOMEM},         /* 9 */
    {ERROR_BAD_ENVIRONMENT, E2BIG},        /* 10 */
    {ERROR_BAD_FORMAT, ENOEXEC},           /* 11 */
    {ERROR_INVALID_ACCESS, EINVAL},        /* 12 */
    {ERROR_INVALID_DATA, EINVAL},          /* 13 */
    {ERROR_INVALID_DRIVE, ENOENT},         /* 15 */
    {ERROR_CURRENT_DIRECTORY, EACCES},     /* 16 */
    {ERROR_NOT_SAME_DEVICE, EXDEV},        /* 17 */
    {ERROR_NO_MORE_FILES, ENOENT},         /* 18 */
    {ERROR_LOCK_VIOLATION, EACCES},        /* 33 */
    {ERROR_BAD_NETPATH, ENOENT},           /* 53 */
    {ERROR_NETWORK_ACCESS_DENIED, EACCES}, /* 65 */
    {ERROR_BAD_NET_NAME, ENOENT},          /* 67 */
    {ERROR_FILE_EXISTS, EEXIST},           /* 80 */
    {ERROR_CANNOT_MAKE, EACCES},           /* 82 */
    {ERROR_FAIL_I24, EACCES},              /* 83 */
    {ERROR_INVALID_PARAMETER, EINVAL},     /* 87 */
    {ERROR_NO_PROC_SLOTS, EAGAIN},         /* 89 */
    {ERROR_DRIVE_LOCKED, EACCES},          /* 108 */
    {ERROR_BROKEN_PIPE, EPIPE},            /* 109 */
    {ERROR_DISK_FULL, ENOSPC},             /* 112 */
    {ERROR_INVALID_TARGET_HANDLE, EBADF},  /* 114 */
    {ERROR_INVALID_NAME, ENOENT},          /* 123 */
    {ERROR_INVALID_HANDLE, EINVAL},        /* 124 */
    {ERROR_WAIT_NO_CHILDREN, ECHILD},      /* 128 */
    {ERROR_CHILD_NOT_COMPLETE, ECHILD},    /* 129 */
    {ERROR_DIRECT_ACCESS_HANDLE, EBADF},   /* 130 */
    {ERROR_NEGATIVE_SEEK, EINVAL},         /* 131 */
    {ERROR_SEEK_ON_DEVICE, EACCES},        /* 132 */
    {ERROR_DIR_NOT_EMPTY, ENOTEMPTY},      /* 145 */
    {ERROR_NOT_LOCKED, EACCES},            /* 158 */
    {ERROR_BAD_PATHNAME, ENOENT},          /* 161 */
    {ERROR_MAX_THRDS_REACHED, EAGAIN},     /* 164 */
    {ERROR_LOCK_FAILED, EACCES},           /* 167 */
    {ERROR_ALREADY_EXISTS, EEXIST},        /* 183 */
    {ERROR_FILENAME_EXCED_RANGE, ENOENT},  /* 206 */
    {ERROR_NESTING_NOT_ALLOWED, EAGAIN},   /* 215 */
    {ERROR_NOT_ENOUGH_QUOTA, ENOMEM}       /* 1816 */
};

/* size of the table */
#define ERRTABLESIZE (sizeof(errtable) / sizeof(errtable[0]))

/* The following two constants must be the minimum and maximum
values in the (contiguous) range of Exec Failure errors. */
#define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
#define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

/* These are the low and high value in the range of errors that are
access violations */
#define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
#define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED

static int get_errno_from_oserr(unsigned long oserrno) {
  int i;

  /* check the table for the OS error code */
  for (i = 0; i < ERRTABLESIZE; ++i) {
    if (oserrno == errtable[i].oscode) {
      return errtable[i].sysv_errno;
    }
  }

  /* The error code wasn't in the table.  We check for a range of */
  /* EACCES errors or exec failure errors (ENOEXEC).  Otherwise   */
  /* EINVAL is returned.                                          */

  if (oserrno >= MIN_EACCES_RANGE && oserrno <= MAX_EACCES_RANGE)
    return EACCES;
  else if (oserrno >= MIN_EXEC_ERROR && oserrno <= MAX_EXEC_ERROR)
    return ENOEXEC;
  else
    return EINVAL;
}

/* Set errno corresponsing to GetLastError() value */
void my_osmaperr(unsigned long oserrno) {
  /*
    set thr_winerr so that we could return the Windows Error Code
    when it is EINVAL.
  */
  set_thr_winerr(oserrno);
  errno = get_errno_from_oserr(oserrno);
}
