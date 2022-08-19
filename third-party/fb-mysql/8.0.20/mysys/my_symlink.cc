/* Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_symlink.cc
*/

#include "my_config.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "m_string.h"
#include "map_helpers.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"
#ifndef _WIN32
#include <sys/stat.h>
#endif

/*
  Reads the content of a symbolic link
  If the file is not a symbolic link, return the original file name in to.

  RETURN
    0  If filename was a symlink,    (to will be set to value of symlink)
    1  If filename was a normal file (to will be set to filename)
   -1  on error.
*/

int my_readlink(char *to, const char *filename, myf MyFlags) {
#ifdef _WIN32
  my_stpcpy(to, filename);
  return 1;
#else
  int result = 0;
  int length;
  DBUG_TRACE;

  if ((length = readlink(filename, to, FN_REFLEN - 1)) < 0) {
    /* Don't give an error if this wasn't a symlink */
    set_my_errno(errno);
    if (my_errno() == EINVAL) {
      result = 1;
      my_stpcpy(to, filename);
    } else {
      if (MyFlags & MY_WME) {
        MyOsError(errno, EE_CANT_READLINK, MYF(0), filename);
      }
      result = -1;
    }
  } else
    to[length] = 0;
  DBUG_PRINT("exit", ("result: %d", result));
  return result;
#endif /* !_WIN32 */
}

/* Create a symbolic link */

#ifndef _WIN32
int my_symlink(const char *content, const char *linkname, myf MyFlags) {
  int result;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("content: %s  linkname: %s", content, linkname));

  result = 0;
  if (symlink(content, linkname)) {
    result = -1;
    set_my_errno(errno);
    if (MyFlags & MY_WME) {
      MyOsError(errno, EE_CANT_SYMLINK, MYF(0), linkname, content);
    }
  }
  return result;
}
#endif /* !_WIN32 */

int my_is_symlink(const char *filename, ST_FILE_ID *file_id) {
#ifndef _WIN32
  struct stat stat_buff;
  int result = !lstat(filename, &stat_buff) && S_ISLNK(stat_buff.st_mode);
  if (file_id && !result) {
    file_id->st_dev = stat_buff.st_dev;
    file_id->st_ino = stat_buff.st_ino;
  }
  return result;

#else
  DWORD dwAttr = GetFileAttributes(filename);
  return (dwAttr != INVALID_FILE_ATTRIBUTES) &&
         (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT);
#endif
}

/*
  Resolve all symbolic links in path
  'to' may be equal to 'filename'
*/

int my_realpath(char *to, const char *filename, myf MyFlags) {
#ifndef _WIN32
  int result = 0;
  DBUG_TRACE;

  DBUG_PRINT("info", ("executing realpath"));
  unique_ptr_free<char> ptr(realpath(filename, nullptr));
  if (ptr) {
    strmake(to, ptr.get(), FN_REFLEN - 1);
  } else {
    /*
      Realpath didn't work;  Use my_load_path() which is a poor substitute
      original name but will at least be able to resolve paths that starts
      with '.'.
    */
    DBUG_PRINT("error", ("realpath failed with errno: %d", errno));
    set_my_errno(errno);
    if (MyFlags & MY_WME) {
      MyOsError(my_errno(), EE_REALPATH, MYF(0), filename);
    }
    my_load_path(to, filename, NullS);
    result = -1;
  }
  return result;
#else
  int ret = GetFullPathName(filename, FN_REFLEN, to, NULL);
  if (ret == 0 || ret > FN_REFLEN) {
    set_my_errno((ret > FN_REFLEN) ? ENAMETOOLONG : GetLastError());
    if (MyFlags & MY_WME) {
      MyOsError(my_errno(), EE_REALPATH, MYF(0), filename);
    }
    /*
      GetFullPathName didn't work : use my_load_path() which is a poor
      substitute original name but will at least be able to resolve
      paths that starts with '.'.
    */
    my_load_path(to, filename, NullS);
    return -1;
  }
  return 0;
#endif
}

/**
  Return non-zero if the file descriptor and a previously lstat-ed file
  identified by file_id point to the same file.
*/
int my_is_same_file(File file, const ST_FILE_ID *file_id) {
  MY_STAT stat_buf;
  if (my_fstat(file, &stat_buf) == -1) {
    set_my_errno(errno);
    return 0;
  }
  return (stat_buf.st_dev == file_id->st_dev) &&
         (stat_buf.st_ino == file_id->st_ino);
}
