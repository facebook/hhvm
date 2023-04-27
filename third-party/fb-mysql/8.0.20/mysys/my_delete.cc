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
  @file mysys/my_delete.cc
*/

#include "my_config.h"

#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys_err.h"

int my_delete(const char *name, myf MyFlags) {
  int err;
  DBUG_TRACE;
  DBUG_PRINT("my", ("name %s MyFlags %d", name, MyFlags));

  if ((err = unlink(name)) == -1) {
    set_my_errno(errno);
    if (MyFlags & (MY_FAE | MY_WME)) {
      MyOsError(my_errno(), EE_DELETE, MYF(0), name);
    }
  }
  return err;
}

#if defined(_WIN32)
/**
  Delete file which is possibly not closed.

  This function is intended to be used exclusively as a temporal solution
  for Win NT in case when it is needed to delete a not closed file (note
  that the file must be opened everywhere with FILE_SHARE_DELETE mode).
  Deleting not-closed files can not be supported on Win 98|ME (and because
  of that is considered harmful).

  The function deletes the file with its preliminary renaming. This is
  because when not-closed share-delete file is deleted it still lives on
  a disk until it will not be closed everwhere. This may conflict with an
  attempt to create a new file with the same name. The deleted file is
  renamed to <name>.<num>.deleted where <name> - the initial name of the
  file, <num> - a hexadecimal number chosen to make the temporal name to
  be unique.

  @param the name of the being deleted file
  @param the flags instructing how to react on an error internally in
         the function

  @note The per-thread @c my_errno holds additional info for a caller to
        decide how critical the error can be.

  @retval
    0	ok
  @retval
    1   error


*/
int nt_share_delete(const char *name, myf MyFlags) {
  char buf[MAX_PATH + 20];
  ulong cnt;
  DBUG_TRACE;
  DBUG_PRINT("my", ("name %s MyFlags %d", name, MyFlags));

  for (cnt = GetTickCount(); cnt; cnt--) {
    errno = 0;
    sprintf(buf, "%s.%08X.deleted", name, cnt);
    if (MoveFile(name, buf)) break;

    if ((errno = GetLastError()) == ERROR_ALREADY_EXISTS) continue;

    /* This happened during tests with MERGE tables. */
    if (errno == ERROR_ACCESS_DENIED) continue;

    DBUG_PRINT("warning",
               ("Failed to rename %s to %s, errno: %d", name, buf, errno));
    break;
  }

  if (errno == ERROR_FILE_NOT_FOUND) {
    set_my_errno(ENOENT);  // marking, that `name' doesn't exist
  } else if (errno == 0) {
    if (DeleteFile(buf)) return 0;
    /*
      The below is more complicated than necessary. For some reason, the
      assignment to my_errno clears the error number, which is retrieved
      by GetLastError() (VC2005EE). Assigning to errno first, allows to
      retrieve the correct value.
    */
    errno = GetLastError();
    if (errno == 0)
      set_my_errno(ENOENT);  // marking, that `buf' doesn't exist
    else
      set_my_errno(errno);
  } else
    set_my_errno(errno);

  if (MyFlags & (MY_FAE + MY_WME)) {
    MyOsError(my_errno(), EE_DELETE, MYF(0), name);
  }
  return -1;
}
#endif
