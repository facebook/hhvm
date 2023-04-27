/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_winfile.cc
  The purpose of this file is to provide implementation of file IO routines on
  Windows that can be thought as drop-in replacement for corresponding C runtime
  functionality.

  Compared to Windows CRT, this one
  - does not have the same file descriptor
  limitation (default is 16384 and can  be increased further, whereas CRT poses
  a hard limit of 2048 file descriptors)
  - the file operations are not serialized
  - positional IO pread/pwrite is ported here.
  - no text mode for files, all IO is "binary"

  Naming convention:
  All routines are prefixed with my_win_, e.g Posix open() is implemented with
  my_win_open()

  Implemented are
  - POSIX routines(e.g open, read, lseek ...)
  - Some ANSI C stream routines (fopen, fdopen, fileno, fclose)
  - Windows CRT equivalents (my_get_osfhandle, open_osfhandle)

  Worth to note:
  - File descriptors used here are located in a range that is not compatible
  with CRT on purpose. Attempt to use a file descriptor from Windows CRT library
  range in my_win_* function will be punished with DBUG_ASSERT()

  - File streams (FILE *) are actually from the C runtime. The routines provided
  here are useful only in scenarios that use low-level IO with my_win_fileno()
*/

#include <algorithm>
#include <iostream>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>

#include "mutex_lock.h"  // MUTEX_LOCK
#include "my_dbug.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysys_priv.h"
#include "scope_guard.h"
#include "sql/malloc_allocator.h"

namespace {

/**
  RAII guard which ensures that:
  - GetLastError() is mapped to its errno equivalent if set
    (unless errno is also set).
  - errno is copied to my_errno if set.
 */
class WindowsErrorGuard {
 public:
  WindowsErrorGuard() {
    errno = 0;
    SetLastError(ERROR_SUCCESS);
  }
  ~WindowsErrorGuard() {
    auto le = GetLastError();

    // If neither errno or LastError has been set, there is nothing
    // for us to do
    if (le == ERROR_SUCCESS && errno == 0) return;

    // The posix api compatibility functions, e.g. _stati64, appears
    // to set both errno and LastError. Here we use our own mapping
    // function and check that errno does not change.
    if (errno != 0 && le != ERROR_SUCCESS) {
      auto orig_errno = errno;
      my_osmaperr(le);

      if (orig_errno != errno) {
        dbug("handleinfo", [&]() {
          char orig_message[512];
          strerror_s(orig_message, orig_errno);

          char curr_message[512];
          strerror_s(curr_message, errno);
          std::cerr << "orig_errno: " << orig_message << " (" << orig_errno
                    << ") errno: " << curr_message << " (" << errno
                    << ") le: " << le << std::endl;
        });

        errno = orig_errno;  // Report the original errno
      }
    }  // if (errno != 0 && le != ERROR_SUCCESS

    // We only set errno from GetLastError() if errno was not
    // already set. Rationale is that errno may have been set
    // explicitly, and then we do not want to overwrite errno with
    // success.
    if (errno == 0) {
      DBUG_ASSERT(le != ERROR_SUCCESS);
      my_osmaperr(le);
    }

    DBUG_ASSERT(errno != 0);
    set_my_errno(errno);
  }  // ~WindowsErrorGuard()
};

struct HandleInfo {
  HANDLE handle = INVALID_HANDLE_VALUE; /* win32 file handle */
  int oflag = 0;                        /* open flags, e.g O_APPEND */
};

using HandleInfoAllocator = Malloc_allocator<HandleInfo>;
using HandleInfoVector = std::vector<HandleInfo, HandleInfoAllocator>;
HandleInfoVector *hivp = nullptr;

size_t ToIndex(File fd) {
  DBUG_ASSERT(fd >= MY_FILE_MIN);
  return fd - MY_FILE_MIN;
}
int ToDescr(size_t hi) { return hi + MY_FILE_MIN; }

bool IsValidIndex(size_t hi) {
  const HandleInfoVector &hiv = *hivp;
  mysql_mutex_assert_owner(&THR_LOCK_open);
  return (hi >= 0 && hi < hiv.size());
}

bool IsValidDescr(File fd) { return IsValidIndex(ToIndex(fd)); }

HandleInfo GetHandleInfo(File fd) {
  HandleInfoVector &hiv = *hivp;
  size_t hi = ToIndex(fd);
  MUTEX_LOCK(g, &THR_LOCK_open);
  if (!IsValidIndex(hi) || hiv[hi].handle == INVALID_HANDLE_VALUE) {
    SetLastError(ERROR_INVALID_HANDLE);
    return {};
  }
  return hiv[hi];
}

File RegisterHandle(HANDLE handle, int oflag) {
  DBUG_ASSERT(handle != 0);
  HandleInfoVector &hiv = *hivp;

  MUTEX_LOCK(g, &THR_LOCK_open);
  HandleInfo hi{handle, oflag};
  auto it = std::find_if(hiv.begin(), hiv.end(), [](const HandleInfo &hi) {
    return (hi.handle == INVALID_HANDLE_VALUE);
  });

  if (it != hiv.end()) {
    *it = hi;
    return ToDescr(it - hiv.begin());
  }
  hiv.push_back(hi);
  return ToDescr(hiv.size() - 1);
}

HandleInfo UnregisterHandle(File fd) {
  HandleInfoVector &hiv = *hivp;
  size_t hi = ToIndex(fd);
  MUTEX_LOCK(g, &THR_LOCK_open);
  DBUG_ASSERT(IsValidIndex(hi));
  HandleInfo unreg = hiv[hi];
  hiv[hi] = {};
  return unreg;
}

File FileIndex(HANDLE handle) {
  const HandleInfoVector &hiv = *hivp;
  MUTEX_LOCK(g, &THR_LOCK_open);
  auto it = std::find_if(hiv.begin(), hiv.end(), [&](const HandleInfo &hi) {
    return (hi.handle == handle);
  });
  return (it == hiv.end() ? -1 : ToDescr(it - hiv.begin()));
}

LARGE_INTEGER MakeLargeInteger(int64_t src) {
  LARGE_INTEGER li;
  li.QuadPart = src;
  return li;
}

OVERLAPPED MakeOverlapped(DWORD l, DWORD h) { return {0, 0, {l, h}, 0}; }

OVERLAPPED MakeOverlapped(int64_t src) {
  LARGE_INTEGER li = MakeLargeInteger(src);
  return MakeOverlapped(li.LowPart, li.HighPart);
}

/**
  Open a file with sharing. Similar to _sopen() from libc, but allows managing
  share delete on win32.

  @param path    file name
  @param oflag   operation flags
  @param shflag  share flag
  @param pmode   permission flags

  @retval File descriptor of opened file if success
  @retval -1 and sets errno and/or LastError if fails.
*/

File my_win_sopen(const char *path, int oflag, int shflag, int pmode) {
  DBUG_TRACE;
  WindowsErrorGuard weg;
  if (check_if_legal_filename(path)) {
    DBUG_ASSERT(GetLastError() == ERROR_SUCCESS);
    errno = EACCES;
    return -1;
  }

  SECURITY_ATTRIBUTES SecurityAttributes;
  SecurityAttributes.nLength = sizeof(SecurityAttributes);
  SecurityAttributes.lpSecurityDescriptor = nullptr;
  SecurityAttributes.bInheritHandle = !(oflag & _O_NOINHERIT);

  // Decode the (requested) OS file access flags
  DWORD fileaccess;
  switch (oflag & (_O_RDONLY | _O_WRONLY | _O_RDWR)) {
    case _O_RDONLY: /* read access */
      fileaccess = GENERIC_READ;
      break;
    case _O_WRONLY: /* write access */
      fileaccess = GENERIC_WRITE;
      break;
    case _O_RDWR: /* read and write access */
      fileaccess = GENERIC_READ | GENERIC_WRITE;
      break;
    default: /* error, bad oflag */
      errno = EINVAL;
      return -1;
  }

  // Decode OS file sharing flags
  DWORD fileshare;
  switch (shflag) {
    case _SH_DENYRW: /* exclusive access except delete */
      fileshare = FILE_SHARE_DELETE;
      break;
    case _SH_DENYWR: /* share read and delete access */
      fileshare = FILE_SHARE_READ | FILE_SHARE_DELETE;
      break;
    case _SH_DENYRD: /* share write and delete access */
      fileshare = FILE_SHARE_WRITE | FILE_SHARE_DELETE;
      break;
    case _SH_DENYNO: /* share read, write and delete access */
      fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
      break;
    case _SH_DENYRWD: /* exclusive access */
      fileshare = 0L;
      break;
    case _SH_DENYWRD: /* share read access */
      fileshare = FILE_SHARE_READ;
      break;
    case _SH_DENYRDD: /* share write access */
      fileshare = FILE_SHARE_WRITE;
      break;
    case _SH_DENYDEL: /* share read and write access */
      fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE;
      break;
    default: /* error, bad shflag */
      errno = EINVAL;
      return -1;
  }

  // Decode OS method of opening/creating
  DWORD filecreate;
  switch (oflag & (_O_CREAT | _O_EXCL | _O_TRUNC)) {
    case 0:
    case _O_EXCL: /* ignore EXCL w/o CREAT */
      filecreate = OPEN_EXISTING;
      break;

    case _O_CREAT:
      filecreate = OPEN_ALWAYS;
      break;

    case _O_CREAT | _O_EXCL:
    case _O_CREAT | _O_TRUNC | _O_EXCL:
      filecreate = CREATE_NEW;
      break;

    case _O_TRUNC:
    case _O_TRUNC | _O_EXCL: /* ignore EXCL w/o CREAT */
      filecreate = TRUNCATE_EXISTING;
      break;

    case _O_CREAT | _O_TRUNC:
      filecreate = CREATE_ALWAYS;
      break;

    default:
      /* this can't happen ... all cases are covered */
      errno = EINVAL;
      return -1;
  }

  // Decode OS file attribute flags if _O_CREAT was specified
  DWORD fileattrib;
  fileattrib = FILE_ATTRIBUTE_NORMAL; /* default */

  int mask;
  if (oflag & _O_CREAT) {
    _umask((mask = _umask(0)));
    DBUG_ASSERT(errno == 0);
    if (!((pmode & ~mask) & _S_IWRITE)) fileattrib = FILE_ATTRIBUTE_READONLY;
  }

  /* Set temporary file (delete-on-close) attribute if requested. */
  if (oflag & _O_TEMPORARY) {
    fileattrib |= FILE_FLAG_DELETE_ON_CLOSE;
    fileaccess |= DELETE;
  }

  /* Set temporary file (delay-flush-to-disk) attribute if requested.*/
  if (oflag & _O_SHORT_LIVED) fileattrib |= FILE_ATTRIBUTE_TEMPORARY;

  /* Set sequential or random access attribute if requested. */
  if (oflag & _O_SEQUENTIAL)
    fileattrib |= FILE_FLAG_SEQUENTIAL_SCAN;
  else if (oflag & _O_RANDOM)
    fileattrib |= FILE_FLAG_RANDOM_ACCESS;

  /* try to open/create the file  */
  HANDLE osfh = CreateFile(path, fileaccess, fileshare, &SecurityAttributes,
                           filecreate, fileattrib, nullptr);

  if (osfh == INVALID_HANDLE_VALUE) {
    /*
       Note that it's not necessary to
       call _free_osfhnd (it hasn't been used yet).
    */
    return -1;
  }

  int fh = RegisterHandle(osfh, oflag & (_O_APPEND | _O_RDONLY | _O_TEXT));
  if (fh == -1) {
    // No scope_guard for this since we only close in one place.
    CloseHandle(osfh);
    return -1;
  }
  return fh;
}

/* Get the file descriptor for stdin,stdout or stderr */
File my_get_stdfile_descriptor(FILE *stream) {
  DBUG_TRACE;
  // no Windows Error Guard in static function. Error conversion is
  // handled at api level

  DWORD nStdHandle;
  if (stream == stdin)
    nStdHandle = STD_INPUT_HANDLE;
  else if (stream == stdout)
    nStdHandle = STD_OUTPUT_HANDLE;
  else if (stream == stderr)
    nStdHandle = STD_ERROR_HANDLE;
  else {
    errno = EINVAL;
    return -1;
  }

  HANDLE hFile = GetStdHandle(nStdHandle);
  if (hFile == INVALID_HANDLE_VALUE) return -1;

  return RegisterHandle(hFile, 0);
}
}  // namespace

/**
   Return the Windows HANDLE for a file descriptor obtained from
   RegisterHandle().

   @param fd "file descriptor" index into HandleInfoVector.

   @retval handle corresponding to fd, if found
   @retval INVALID_HANDLE_VALUE, if fd is not valid (illegal index
           into HandleInfoVector). In this case
   SetLastError(ERROR_INVALID_HANDLE) will have been called.
 */
HANDLE my_get_osfhandle(File fd) {
  DBUG_TRACE;

  return GetHandleInfo(fd).handle;
}

/**
   Homegrown posix emulation for Windows.

   @param path  File name.
   @param mode  File access mode string.

   @retval valid "file descriptor" (actually index into mapping
   vector) if successful.

   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
File my_win_open(const char *path, int mode) {
  DBUG_TRACE;
  return my_win_sopen(path, mode | _O_BINARY, _SH_DENYNO, _S_IREAD | S_IWRITE);
}

/**
   Homegrown posix emulation for Windows.

   @param fd     "File descriptor" (index into mapping vector).

   @retval 0 if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int my_win_close(File fd) {
  DBUG_TRACE;

  WindowsErrorGuard weg;
  HandleInfo unreg = UnregisterHandle(fd);
  if (unreg.handle == INVALID_HANDLE_VALUE) return -1;

  if (!CloseHandle(unreg.handle)) return -1;

  return 0;
}

/**
   Homegrown posix emulation for Windows. Implements BOTH posix read()
   and posix pread(). To read from the current file position (like
   posix read() supply an offset argument that is -1.

   @param fd     "File descriptor" (index into mapping vector).
   @param buffer Destination buffer.
   @param count  Number of bytes to read.
   @param offset Offset where read starts. -1 to read from the current file
                 position.

   @note ERROR_HANDLE_EOF and ERROR_BROKEN_PIPE are treated as success
         (returns 0, but with LastError set).

   @retval Number of bytes read, if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int64_t my_win_pread(File fd, uchar *buffer, size_t count, int64_t offset) {
  DBUG_TRACE;

  if (!count) return 0;
  if (count > UINT_MAX) count = UINT_MAX;

  WindowsErrorGuard weg;
  HANDLE hFile = my_get_osfhandle(fd);
  if (hFile == INVALID_HANDLE_VALUE) return -1;

  OVERLAPPED ov = MakeOverlapped(offset);

  DWORD nBytesRead;
  if (!ReadFile(hFile, buffer, count, &nBytesRead,
                (offset == -1 ? nullptr : &ov))) {
    DWORD lastError = GetLastError();
    /*
      ERROR_BROKEN_PIPE is returned when no more data coming
      through e.g. a command pipe in windows : see MSDN on ReadFile.
    */
    if (lastError == ERROR_HANDLE_EOF || lastError == ERROR_BROKEN_PIPE)
      return 0; /*return 0 at EOF*/
    return -1;
  }
  return nBytesRead;
}

/**
   Homegrown posix emulation for Windows.

   @param fd     "File descriptor" (index into mapping vector).
   @param buffer Source buffer.
   @param count  Number of bytes to write.
   @param offset Where to start the write.

   @retval Number of bytes written, if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int64_t my_win_pwrite(File fd, const uchar *buffer, size_t count,
                      int64_t offset) {
  DBUG_TRACE;

  if (!count) return 0;
  if (count > UINT_MAX) count = UINT_MAX;

  WindowsErrorGuard weg;
  HandleInfo hi = GetHandleInfo(fd);
  if (hi.handle == INVALID_HANDLE_VALUE) return -1;

  OVERLAPPED ov = MakeOverlapped(offset);

  DWORD nBytesWritten;
  if (!WriteFile(hi.handle, buffer, count, &nBytesWritten, &ov)) return -1;

  return nBytesWritten;
}

/**
   Homegrown posix emulation for Windows.

   @param fd     "File descriptor" (index into mapping vector).
   @param pos    Offset to seek.
   @param whence Where to seek from.

   @retval New offset into file, if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int64_t my_win_lseek(File fd, int64_t pos, int whence) {
  DBUG_TRACE;
  WindowsErrorGuard weg;

  static_assert(FILE_BEGIN == SEEK_SET && FILE_CURRENT == SEEK_CUR &&
                    FILE_END == SEEK_END,
                "Windows and POSIX seek constants must be compatible.");

  HandleInfo hi = GetHandleInfo(fd);
  if (hi.handle == INVALID_HANDLE_VALUE) return -1;

  LARGE_INTEGER newpos;
  if (!SetFilePointerEx(hi.handle, MakeLargeInteger(pos), &newpos, whence))
    return -1;

  return newpos.QuadPart;
}

/**
   Homegrown posix emulation for Windows.

   @param fd     "File descriptor" (index into mapping vector).
   @param Buffer Source Bytes.
   @param Count  Number of bytes to write.

   @retval Number of bytes written, if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int64_t my_win_write(File fd, const uchar *Buffer, size_t Count) {
  DBUG_TRACE;

  if (!Count) return 0;
  if (Count > UINT_MAX) Count = UINT_MAX;

  WindowsErrorGuard weg;

  HandleInfo hi = GetHandleInfo(fd);
  if (hi.handle == INVALID_HANDLE_VALUE) return -1;

  // The msdn docs state: "To write to the end of file, specify
  // both the Offset and OffsetHigh members of the OVERLAPPED structure
  // as 0xFFFFFFFF."
  OVERLAPPED ov_append = MakeOverlapped(0xFFFFFFFF, 0xFFFFFFFF);

  DWORD nWritten;
  if (!WriteFile(hi.handle, Buffer, Count, &nWritten,
                 (hi.oflag & _O_APPEND) ? &ov_append : nullptr))
    return -1;

  return nWritten;
}

/**
   Homegrown posix emulation for Windows.

   @param fd "File descriptor" (index into mapping vector).
   @param newlength new desired length.

   @retval 0 if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int my_win_chsize(File fd, int64_t newlength) {
  DBUG_TRACE;
  WindowsErrorGuard weg;

  HandleInfo hi = GetHandleInfo(fd);
  if (hi.handle == INVALID_HANDLE_VALUE) return -1;

  if (!SetFilePointerEx(hi.handle, MakeLargeInteger(newlength), nullptr,
                        FILE_BEGIN))
    return -1;
  if (!SetEndOfFile(hi.handle)) return -1;

  return 0;
}

// Emulation of posix functions for streams that are not part of the C standard.

/**
   Homegrown posix emulation for Windows.

   @param stream FILE stream to get fd for.

   @note Returned fd can either be a CRT standard fd (stdin stdout or
         stderr), or a pseudo fd which is an index into the mapping
         vector).

   @retval Valid "file descriptor" if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/

File my_win_fileno(FILE *stream) {
  DBUG_TRACE;
  WindowsErrorGuard weg;

  File stream_fd = _fileno(stream);
  if (stream_fd < 0)
    return -1;  // Only EINVAL if stream == nullptr, according to msdn

  HANDLE hFile = reinterpret_cast<HANDLE>(_get_osfhandle(stream_fd));
  if (hFile == INVALID_HANDLE_VALUE) return -1;

  File retval = FileIndex(hFile);
  if (retval == -1) /* try std stream */
    return my_get_stdfile_descriptor(stream);
  return retval;
}

/**
   Homegrown posix emulation for Windows.

   @param filename File name.
   @param mode     File access mode string.

   @retval FILE stream associated with given file name, if successful.
   @retval nullptr in case of errors. errno and/or LastError set to
           indicate the error.
*/
FILE *my_win_fopen(const char *filename, const char *mode) {
  DBUG_TRACE;
  WindowsErrorGuard weg;

  /*
    If we are not creating, then we need to use my_access to make sure
    the file exists since Windows doesn't handle files like "com1.sym"
    very  well
  */
  if (check_if_legal_filename(filename)) {
    errno = EACCES;
    return nullptr;
  }

  FILE *stream = fopen(filename, mode);
  if (!stream) return nullptr;
  auto sg = create_scope_guard([stream]() { fclose(stream); });

  int flags = (strchr(mode, 'a') != nullptr) ? O_APPEND : 0;

  /*
     Register file handle in my_table_info.
     Necessary for my_fileno()
   */
  if (RegisterHandle(reinterpret_cast<HANDLE>(_get_osfhandle(fileno(stream))),
                     flags) < 0)
    return nullptr;

  sg.commit();  // Do not close the stream we are about to return
  return stream;
}

/**
   Homegrown posix emulation for Windows.

   @param fd  "File descriptor" (index into mapping vector).
   @param mode File access mode string.

   @note Currently only used in ibd2sdi to bind the fd for a temporary
   file to a file stream. Could possibly be replaced with tmpfile_s.

   @retval FILE stream associated with given file descriptor, if successful.
   @retval nullptr in case of errors. errno and/or LastError set to
           indicate the error.
*/
FILE *my_win_fdopen(File fd, const char *mode) {
  DBUG_TRACE;
  WindowsErrorGuard weg;

  int flags = (strchr(mode, 'a') != nullptr) ? O_APPEND : 0;

  HANDLE hFile = my_get_osfhandle(fd);
  if (hFile == INVALID_HANDLE_VALUE) return nullptr;

  /* Convert OS file handle to CRT file descriptor and then call fdopen*/
  int crt_fd = _open_osfhandle((intptr_t)hFile, flags);
  if (crt_fd < 0) return nullptr;

  return fdopen(crt_fd, mode);
}

/**
   Homegrown posix emulation for Windows.

   @param stream FILE stream to close.

   @note Calls to my_fileno() and UnregsterHandle() would not be
   necessary if pseudo fds were not allocated for streams.

   @retval 0 if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int my_win_fclose(FILE *stream) {
  DBUG_TRACE;
  WindowsErrorGuard weg;
  File fd = my_fileno(stream);
  if (fd < 0) return -1;
  if (fd >= MY_FILE_MIN) UnregisterHandle(fd);
  if (fclose(stream) < 0) return -1;
  return 0;
}

/**
   Homegrown posix emulation for Windows.

   @param path    File name.
   @param mode    File access mode string
   @param stream  Existing stream to reopen

   @note C11 includes freopen_s. If pseudo fds did not have to be
   created for streams it would probably be sufficient to just forward
   to freopen_s.

   @retval valid FILE stream if successful.
   @retval nullptr in case of errors. errno and/or LastError set to
           indicate the error.
*/
FILE *my_win_freopen(const char *path, const char *mode, FILE *stream) {
  DBUG_TRACE;
  DBUG_ASSERT(path && stream);
  WindowsErrorGuard weg;

  /* Services don't have stdout/stderr on Windows, so _fileno returns -1. */
  File fd = _fileno(stream);
  if (fd < 0) {
    if (!freopen(path, mode, stream)) return nullptr;

    fd = _fileno(stream);
    if (fd < 0) return nullptr;
  }
  auto cfdg = create_scope_guard([fd]() { _close(fd); });

  HANDLE osfh =
      CreateFile(path, GENERIC_READ | GENERIC_WRITE,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (osfh == INVALID_HANDLE_VALUE) return nullptr;

  auto chg = create_scope_guard([osfh]() { CloseHandle(osfh); });
  int handle_fd = _open_osfhandle((intptr_t)osfh, _O_APPEND | _O_TEXT);

  if (handle_fd == -1) return nullptr;
  auto chfdg = create_scope_guard([handle_fd]() { _close(handle_fd); });

  if (_dup2(handle_fd, fd) < 0) return nullptr;

  // Leave the handle and fd open, but close handle_fd
  chg.commit();
  cfdg.commit();

  return stream;
}

/**
   Homegrown posix emulation for Windows.

   Quick and dirty implementation for Windows. Use CRT fstat on
   temporarily allocated file descriptor.  Patch file size, because
   size that fstat returns is not reliable (may be outdated).

   @param      fd   "File descriptor" (index into mapping vector).
   @param[out] buf  Area to store stat information in.

   @retval 0 if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int my_win_fstat(File fd, struct _stati64 *buf) {
  DBUG_TRACE;
  WindowsErrorGuard weg;

  HandleInfo hi = GetHandleInfo(fd);
  if (hi.handle == INVALID_HANDLE_VALUE) return -1;

  HANDLE hDup;
  if (!DuplicateHandle(GetCurrentProcess(), hi.handle, GetCurrentProcess(),
                       &hDup, 0, false, DUPLICATE_SAME_ACCESS))
    return -1;

  int crt_fd = _open_osfhandle((intptr_t)hDup, 0);
  if (crt_fd < 0) return -1;
  auto cg = create_scope_guard([crt_fd]() { _close(crt_fd); });

  if (_fstati64(crt_fd, buf) < 0) return -1;

  /* File size returned by stat is not accurate (may be outdated), fix it*/
  if (!GetFileSizeEx(hDup, (PLARGE_INTEGER)(&(buf->st_size)))) return -1;

  // crt_fd is only used to call _fstati64() so we do not commit the
  // scope guard here
  return 0;
}

/**
   Homegrown posix emulation for Windows.

   @param      path File name to obtain information about.
   @param[out] buf  Area to store stat information in.

   @retval Valid "file descriptor" (actually index into mapping
           vector) if successful.

   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int my_win_stat(const char *path, struct _stati64 *buf) {
  DBUG_TRACE;
  WindowsErrorGuard weg;
  if (_stati64(path, buf) < 0) {
    return -1;
  }

  /* File size returned by stat is not accurate (may be outdated), fix
     it */
  WIN32_FILE_ATTRIBUTE_DATA data;
  if (!GetFileAttributesEx(path, GetFileExInfoStandard, &data)) return -1;

  buf->st_size =
      LARGE_INTEGER{data.nFileSizeLow, (LONG)data.nFileSizeHigh}.QuadPart;

  return 0;
}

/**
   Homegrown posix emulation for Windows.

   @param fd "File descriptor" (index into mapping vector).

   @retval 0 if successful.
   @retval -1 in case of errors. errno and/or LastError set to
           indicate the error.
*/
int my_win_fsync(File fd) {
  DBUG_TRACE;
  WindowsErrorGuard weg;
  HandleInfo hi = GetHandleInfo(fd);

  if (hi.handle == INVALID_HANDLE_VALUE) return -1;
  if (!FlushFileBuffers(hi.handle)) return -1;

  return 0;
}

/**
   Constructs static objects.
 */
void MyWinfileInit() {
  hivp = new HandleInfoVector(HandleInfoAllocator(key_memory_win_handle_info));
}

/**
   Destroys static objects.
*/
void MyWinfileEnd() { delete hivp; }
