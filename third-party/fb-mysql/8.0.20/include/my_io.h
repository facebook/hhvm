/*
   Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_IO_INCLUDED
#define MY_IO_INCLUDED 1

/**
  @file include/my_io.h
  Common \#defines and includes for file and socket I/O.
*/

#include <mysql/components/services/my_io_bits.h>

#ifdef _WIN32

/* Define missing access() modes. */
#define F_OK 0
#define W_OK 2
#define R_OK 4 /* Test for read permission.  */

/* Define missing file locking constants. */
#define F_RDLCK 1
#define F_WRLCK 2
#define F_UNLCK 3

#define O_NONBLOCK 1 /* For emulation of fcntl() */

/*
  SHUT_RDWR is called SD_BOTH in windows and
  is defined to 2 in winsock2.h
  #define SD_BOTH 0x02
*/
#define SHUT_RDWR 0x02

#endif  // _WIN32

/* file create flags */

#ifdef _WIN32
/* Only for my_fopen() - _O_BINARY is set by default for my_open() */
#define MY_FOPEN_BINARY _O_BINARY
#else
#define MY_FOPEN_BINARY 0 /* Ignore on non-Windows */
#endif

#ifdef _WIN32
#define O_NOFOLLOW 0 /* Ignore on Windows */
#endif

/* additional file share flags for win32 */
#ifdef _WIN32
#define _SH_DENYRWD 0x110 /* deny read/write mode & delete */
#define _SH_DENYWRD 0x120 /* deny write mode & delete      */
#define _SH_DENYRDD 0x130 /* deny read mode & delete       */
#define _SH_DENYDEL 0x140 /* deny delete only              */
#endif                    /* _WIN32 */

/* General constants */
#define FN_LEN 256        /* Max file name len */
#define FN_HEADLEN 253    /* Max length of filepart of file name */
#define FN_EXTLEN 20      /* Max length of extension (part of FN_LEN) */
#define FN_REFLEN 512     /* Max length of full path-name */
#define FN_REFLEN_SE 4000 /* Max length of full path-name in SE */
/*
  Max length of full path-name followed by length of previous_gtid_set in
  a binlog file separated by the delimiter ' '.

  Gtid set length is stored as long int which takes 10 characters in decimal.
  An extra character is used for '\0'.
*/
#define FILE_AND_GTID_SET_LENGTH (FN_REFLEN + 12)
#define FN_EXTCHAR '.'
#define FN_HOMELIB '~'    /* ~/ is used as abbrev for home dir */
#define FN_CURLIB '.'     /* ./ is used as abbrev for current dir */
#define FN_PARENTDIR ".." /* Parent directory; Must be a string */

#ifdef _WIN32
#define FN_LIBCHAR '\\'
#define FN_LIBCHAR2 '/'
#define FN_DIRSEP "/\\" /* Valid directory separators */
#define FN_EXEEXT ".exe"
#define FN_SOEXT ".dll"
#define FN_ROOTDIR "\\"
#define FN_DEVCHAR ':'
#define FN_NETWORK_DRIVES /* Uses \\ to indicate network drives */
#else
#define FN_LIBCHAR '/'
/*
  FN_LIBCHAR2 is not defined on !Windows. Use is_directory_separator().
*/
#define FN_DIRSEP "/" /* Valid directory separators */
#define FN_EXEEXT ""
#define FN_SOEXT ".so"
#define FN_ROOTDIR "/"
#endif

static inline int is_directory_separator(char c) {
#ifdef _WIN32
  return c == FN_LIBCHAR || c == FN_LIBCHAR2;
#else
  return c == FN_LIBCHAR;
#endif
}

/*
  MY_FILE_MIN is  Windows speciality and is used to quickly detect
  the mismatch of CRT and mysys file IO usage on Windows at runtime.
  CRT file descriptors can be in the range 0-2047, whereas descriptors
  returned by my_open() will start with 2048. If a file descriptor with value
  less then MY_FILE_MIN is passed to mysys IO function, chances are it stemms
  from open()/fileno() and not my_open()/my_fileno.

  For Posix,  mysys functions are light wrappers around libc, and MY_FILE_MIN
  is logically 0.
*/

#ifdef _WIN32
#define MY_FILE_MIN 2048
#else
#define MY_FILE_MIN 0
#endif

/*
  MY_NFILE is the default size of my_file_info array.

  It is larger on Windows, because it all file handles are stored in
  my_file_info Default size is 16384 and this should be enough for most cases.If
  it is not enough, --max-open-files with larger value can be used.

  For Posix , my_file_info array is only used to store filenames for
  error reporting and its size is not a limitation for number of open files.
*/
#ifdef _WIN32
#define MY_NFILE (16384 + MY_FILE_MIN)
#else
#define MY_NFILE 64
#endif

#define OS_FILE_LIMIT UINT_MAX

/*
  Io buffer size; Must be a power of 2 and a multiple of 512. May be
  smaller what the disk page size. This influences the speed of the
  isam btree library. eg to big to slow.
*/
#define IO_SIZE 4096

#if defined(_WIN32)
#define socket_errno WSAGetLastError()
#define SOCKET_EINTR WSAEINTR
#define SOCKET_EAGAIN WSAEINPROGRESS
#define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#define SOCKET_EADDRINUSE WSAEADDRINUSE
#define SOCKET_ETIMEDOUT WSAETIMEDOUT
#define SOCKET_ECONNRESET WSAECONNRESET
#define SOCKET_ENFILE ENFILE
#define SOCKET_EMFILE EMFILE
#else /* Unix */
#define socket_errno errno
#define closesocket(A) close(A)
#define SOCKET_EINTR EINTR
#define SOCKET_EAGAIN EAGAIN
#define SOCKET_EWOULDBLOCK EWOULDBLOCK
#define SOCKET_EADDRINUSE EADDRINUSE
#define SOCKET_ETIMEDOUT ETIMEDOUT
#define SOCKET_ECONNRESET ECONNRESET
#define SOCKET_ENFILE ENFILE
#define SOCKET_EMFILE EMFILE
#endif

#ifndef _WIN32
#define INVALID_SOCKET -1
#endif /* _WIN32 */

/* File permissions */
#define USER_READ (1L << 0)
#define USER_WRITE (1L << 1)
#define USER_EXECUTE (1L << 2)
#define GROUP_READ (1L << 3)
#define GROUP_WRITE (1L << 4)
#define GROUP_EXECUTE (1L << 5)
#define OTHERS_READ (1L << 6)
#define OTHERS_WRITE (1L << 7)
#define OTHERS_EXECUTE (1L << 8)
#define USER_RWX USER_READ | USER_WRITE | USER_EXECUTE
#define GROUP_RWX GROUP_READ | GROUP_WRITE | GROUP_EXECUTE
#define OTHERS_RWX OTHERS_READ | OTHERS_WRITE | OTHERS_EXECUTE

#endif  // MY_IO_INCLUDED
