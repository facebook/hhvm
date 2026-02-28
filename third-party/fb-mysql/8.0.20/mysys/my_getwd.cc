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
  @file mysys/my_getwd.cc
*/

/* my_setwd() and my_getwd() works with intern_filenames !! */

#include "my_config.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys/my_static.h"
#include "mysys_err.h"
#if defined(_WIN32)
#include <direct.h>
#include <dos.h>

#include "m_ctype.h"
#endif

/* Gets current working directory in buff.

  SYNPOSIS
    my_getwd()
    buf		Buffer to store result. Can be curr_dir[].
    size	Size of buffer
    MyFlags	Flags

  NOTES
    Directory is allways ended with FN_LIBCHAR

  RESULT
    0  ok
    #  error
*/

int my_getwd(char *buf, size_t size, myf MyFlags) {
  char *pos;
  DBUG_TRACE;
  DBUG_PRINT("my", ("buf: %p  size: %u  MyFlags %d", buf, (uint)size, MyFlags));

  if (size < 1) return -1;

  if (curr_dir[0]) /* Current pos is saved here */
    (void)strmake(buf, &curr_dir[0], size - 1);
  else {
    if (size < 2) return -1;
    if (!getcwd(buf, (uint)(size - 2)) && MyFlags & MY_WME) {
      set_my_errno(errno);
      MyOsError(my_errno(), EE_GETWD, MYF(0));
      return -1;
    }
    if (*((pos = strend(buf)) - 1) != FN_LIBCHAR) /* End with FN_LIBCHAR */
    {
      pos[0] = FN_LIBCHAR;
      pos[1] = 0;
    }
    (void)strmake(&curr_dir[0], buf, (size_t)(FN_REFLEN - 1));
  }
  return 0;
} /* my_getwd */

/* Set new working directory */

int my_setwd(const char *dir, myf MyFlags) {
  int res;
  size_t length;
  char *pos;
  DBUG_TRACE;
  DBUG_PRINT("my", ("dir: '%s'  MyFlags %d", dir, MyFlags));

  const char *start = dir;
  if (!dir[0] || (dir[0] == FN_LIBCHAR && dir[1] == 0)) dir = FN_ROOTDIR;
  if ((res = chdir(dir)) != 0) {
    set_my_errno(errno);
    if (MyFlags & MY_WME) {
      MyOsError(my_errno(), EE_SETWD, MYF(0), start);
    }
  } else {
    if (test_if_hard_path(start)) { /* Hard pathname */
      pos = strmake(&curr_dir[0], start, (size_t)FN_REFLEN - 1);
      if (pos[-1] != FN_LIBCHAR) {
        length = (uint)(pos - (char *)curr_dir);
        curr_dir[length] = FN_LIBCHAR; /* must end with '/' */
        curr_dir[length + 1] = '\0';
      }
    } else
      curr_dir[0] = '\0'; /* Don't save name */
  }
  return res;
}

/* Test if hard pathname */
/* Returns 1 if dirname is a hard path */

int test_if_hard_path(const char *dir_name) {
  if (dir_name[0] == FN_HOMELIB && dir_name[1] == FN_LIBCHAR)
    return (home_dir != NullS && test_if_hard_path(home_dir));
  if (dir_name[0] == FN_LIBCHAR) return (true);
#ifdef FN_DEVCHAR
  return (strchr(dir_name, FN_DEVCHAR) != 0);
#else
  return false;
#endif
} /* test_if_hard_path */

/*
  Test if a name contains an (absolute or relative) path.

  SYNOPSIS
    has_path()
    name                The name to test.

  RETURN
    true        name contains a path.
    false       name does not contain a path.
*/

bool has_path(const char *name) {
  return (strchr(name, FN_LIBCHAR) != nullptr)
#if FN_LIBCHAR != '/'
         || (strchr(name, '/') != nullptr)
#endif
#ifdef FN_DEVCHAR
         || (strchr(name, FN_DEVCHAR) != nullptr)
#endif
      ;
}
