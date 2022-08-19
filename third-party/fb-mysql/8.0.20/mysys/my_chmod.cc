/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_chmod.cc
*/

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"

/*
  Generate MY_MODE representation from perm_flags.

  @param perm_flags Permission information

  @return Permission in MY_STAT format
*/

MY_MODE get_file_perm(ulong perm_flags) {
  MY_MODE file_perm = 0;
  if (perm_flags <= 0) return file_perm;

#if defined _WIN32
  if (perm_flags & (USER_READ | GROUP_READ | OTHERS_READ))
    file_perm |= _S_IREAD;
  if (perm_flags & (USER_WRITE | GROUP_WRITE | OTHERS_WRITE))
    file_perm |= _S_IWRITE;
#else
  if (perm_flags & USER_READ) file_perm |= S_IRUSR;
  if (perm_flags & USER_WRITE) file_perm |= S_IWUSR;
  if (perm_flags & USER_EXECUTE) file_perm |= S_IXUSR;
  if (perm_flags & GROUP_READ) file_perm |= S_IRGRP;
  if (perm_flags & GROUP_WRITE) file_perm |= S_IWGRP;
  if (perm_flags & GROUP_EXECUTE) file_perm |= S_IXGRP;
  if (perm_flags & OTHERS_READ) file_perm |= S_IROTH;
  if (perm_flags & OTHERS_WRITE) file_perm |= S_IWOTH;
  if (perm_flags & OTHERS_EXECUTE) file_perm |= S_IXOTH;
#endif

  return file_perm;
}

/*
  my_chmod : Change permission on a file

  @param filename : Name of the file
  @param perm_flags : Permission information
  @param my_flags : Error handling

  @return
    @retval true : Error changing file permission
    @retval false : File permission changed successfully
*/

bool my_chmod(const char *filename, ulong perm_flags, myf MyFlags) {
  int ret_val;
  MY_MODE file_perm;
  DBUG_TRACE;
  DBUG_ASSERT(filename && filename[0]);

  file_perm = get_file_perm(perm_flags);
#ifdef _WIN32
  ret_val = _chmod(filename, file_perm);
#else
  ret_val = chmod(filename, file_perm);
#endif

  if (ret_val && (MyFlags & (MY_FAE | MY_WME))) {
    set_my_errno(errno);
    MyOsError(my_errno(), EE_CHANGE_PERMISSIONS, MYF(0), filename);
  }

  return ret_val ? true : false;
}
