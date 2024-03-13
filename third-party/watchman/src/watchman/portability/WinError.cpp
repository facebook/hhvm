/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/portability/WinError.h"

#ifdef _WIN32

#include <windows.h>

const char* win32_strerror(uint32_t err) {
  static thread_local char msgbuf[1024];

  FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      err,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      msgbuf,
      sizeof(msgbuf) - 1,
      nullptr);

  return msgbuf;
}

int map_win32_err(uint32_t err) {
  switch (err) {
    case ERROR_SUCCESS:
      return 0;
    case ERROR_ALREADY_EXISTS:
      return EEXIST;
    case ERROR_TIMEOUT:
      return ETIMEDOUT;
    case WAIT_TIMEOUT:
      return ETIMEDOUT;
    case WAIT_IO_COMPLETION:
      return EINTR;
    case ERROR_INVALID_FUNCTION:
      return ENOSYS;
    case ERROR_PATH_NOT_FOUND:
      return ENOTDIR;
    case ERROR_FILE_NOT_FOUND:
      return ENOENT;
    case ERROR_TOO_MANY_OPEN_FILES:
      return EMFILE;
    case ERROR_ACCESS_DENIED:
      return EACCES;
    case ERROR_INVALID_HANDLE:
      return EBADF;
    case ERROR_NOT_ENOUGH_MEMORY:
      return ENOMEM;
    case ERROR_INVALID_ACCESS:
      return EACCES;
    case ERROR_INVALID_DATA:
      return EINVAL;
    case ERROR_NO_MORE_FILES:
      return EMFILE;
    case ERROR_WRITE_PROTECT:
      return EPERM;
    case ERROR_NOT_SUPPORTED:
      return ENOSYS;
    case ERROR_DEV_NOT_EXIST:
      return ENOENT;
    case ERROR_FILE_EXISTS:
      return EEXIST;
    case ERROR_INVALID_PARAMETER:
      return EINVAL;
    case ERROR_NO_PROC_SLOTS:
      return EAGAIN;
    case ERROR_BROKEN_PIPE:
      return EPIPE;
    case ERROR_DISK_FULL:
      return ENOSPC;
    case ERROR_IO_INCOMPLETE:
      return EAGAIN;
    case ERROR_IO_PENDING:
      return EAGAIN;
    case WSAECONNREFUSED:
      return ECONNREFUSED;
    default:
      return EINVAL;
  }
}

#endif
