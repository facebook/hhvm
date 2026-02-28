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
  @file mysys/mf_tempfile.cc
*/

#include "my_config.h"

#include <errno.h>
#ifdef HAVE_O_TMPFILE
#include <fcntl.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"

#ifdef WIN32
#include <fcntl.h>  // O_EXCL
#endif

/*
  @brief
  Create a temporary file with unique name in a given directory

  @details
  create_temp_file
    to             pointer to buffer where temporary filename will be stored
    dir            directory where to create the file
    prefix         prefix the filename with this
    mode           Flags to use for my_create/my_open
    MyFlags        Magic flags

  @return
    File descriptor of opened file if success
    -1 and sets errno if fails.

  @note
    The behaviour of this function differs a lot between
    implementation, it's main use is to generate a file with
    a name that does not already exist.

    The implementation using mkstemp should be considered the
    reference implementation when adding a new or modifying an
    existing one

*/

File create_temp_file(char *to, const char *dir, const char *prefix,
                      int mode MY_ATTRIBUTE((unused)),
                      UnlinkOrKeepFile unlink_or_keep,
                      myf MyFlags MY_ATTRIBUTE((unused))) {
  File file = -1;
#ifdef _WIN32
  TCHAR path_buf[MAX_PATH - 14];
#endif

  DBUG_TRACE;
  DBUG_PRINT("enter", ("dir: %s, prefix: %s", dir, prefix));
#if defined(_WIN32)

  /*
    Use GetTempPath to determine path for temporary files.
    This is because the documentation for GetTempFileName
    has the following to say about this parameter:
    "If this parameter is NULL, the function fails."
  */
  if (!dir) {
    if (GetTempPath(sizeof(path_buf), path_buf) > 0) dir = path_buf;
  }
  /*
    Use GetTempFileName to generate a unique filename, create
    the file and release it's handle
     - uses up to the first three letters from prefix
  */
  if (GetTempFileName(dir, prefix, 0, to) == 0) return -1;

  DBUG_PRINT("info", ("name: %s", to));

  /*
    Open the file without the "open only if file doesn't already exist"
    since the file has already been created by GetTempFileName
  */
  if ((file = my_open(to, (mode & ~O_EXCL), MyFlags)) < 0) {
    /* Open failed, remove the file created by GetTempFileName */
    int tmp = my_errno();
    (void)my_delete(to, MYF(0));
    set_my_errno(tmp);
    return file;
  }
  if (unlink_or_keep == UNLINK_FILE) {
    my_delete(to, MYF(0));
  }

#else /* mkstemp() is available on all non-Windows supported platforms. */
#ifdef HAVE_O_TMPFILE
  if (unlink_or_keep == UNLINK_FILE) {
    if (!dir && !(dir = getenv("TMPDIR"))) dir = DEFAULT_TMPDIR;

    char dirname_buf[FN_REFLEN];
    convert_dirname(dirname_buf, dir, nullptr);

    // Verify that the generated filename will fit in a FN_REFLEN size buffer.
    int max_filename_len = snprintf(to, FN_REFLEN, "%s%.20sfd=%d", dirname_buf,
                                    prefix ? prefix : "tmp.", 4 * 1024 * 1024);
    if (max_filename_len >= FN_REFLEN) {
      errno = ENAMETOOLONG;
      set_my_errno(ENAMETOOLONG);
      return file;
    }

    /* Explicitly don't use O_EXCL here as it has a different
       meaning with O_TMPFILE.
    */
    file = mysys_priv::RetryOnEintr(
        [&]() {
          return open(dirname_buf, O_RDWR | O_TMPFILE | O_CLOEXEC,
                      S_IRUSR | S_IWUSR);
        },
        -1);

    if (file >= 0) {
      sprintf(to, "%s%.20sfd=%d", dirname_buf, prefix ? prefix : "tmp.", file);
      file_info::RegisterFilename(file, to,
                                  file_info::OpenType::FILE_BY_O_TMPFILE);
    }
  }
  // Fall through, in case open() failed above (or we have KEEP_FILE).
#endif /* HAVE_O_TMPFILE */
  if (file == -1) {
    char prefix_buff[30];
    uint pfx_len;

    pfx_len = (uint)(my_stpcpy(my_stpnmov(prefix_buff, prefix ? prefix : "tmp.",
                                          sizeof(prefix_buff) - 7),
                               "XXXXXX") -
                     prefix_buff);
    if (!dir && !(dir = getenv("TMPDIR"))) dir = DEFAULT_TMPDIR;
    if (strlen(dir) + pfx_len > FN_REFLEN - 2) {
      errno = ENAMETOOLONG;
      set_my_errno(ENAMETOOLONG);
      return file;
    }
    my_stpcpy(convert_dirname(to, dir, NullS), prefix_buff);
    file = mkstemp(to);
    file_info::RegisterFilename(file, to, file_info::OpenType::FILE_BY_MKSTEMP);
    if (unlink_or_keep == UNLINK_FILE) {
      unlink(to);
    }
  }
#endif /* _WIN32 */
  if (file >= 0) {
    mysql_mutex_lock(&THR_LOCK_open);
    my_tmp_file_created++;
    mysql_mutex_unlock(&THR_LOCK_open);
  }
  return file;
}
