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

#include "my_config.h"

#include <stdlib.h>
#include <sys/types.h>

#include "m_string.h"
#include "mutex_lock.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "mysys/mysys_priv.h"
#include "prealloced_array.h"

#if defined(_WIN32)
#define DELIM ';'
#else
#define DELIM ':'
#endif

bool init_tmpdir(MY_TMPDIR *tmpdir, const char *pathlist) {
  const char *end;
  char *copy;
  char buff[FN_REFLEN];
  DBUG_TRACE;
  DBUG_PRINT("enter", ("pathlist: %s", pathlist ? pathlist : "NULL"));

  Prealloced_array<char *, 10> full_list(key_memory_MY_TMPDIR_full_list);

  *tmpdir = MY_TMPDIR();
  if (!pathlist || !pathlist[0]) {
    /* Get default temporary directory */
    pathlist = getenv("TMPDIR"); /* Use this if possible */
#if defined(_WIN32)
    if (!pathlist) pathlist = getenv("TEMP");
    if (!pathlist) pathlist = getenv("TMP");
#endif
    if (!pathlist || !pathlist[0]) pathlist = DEFAULT_TMPDIR;
  }
  do {
    size_t length;
    end = strcend(pathlist, DELIM);
    strmake(buff, pathlist, (uint)(end - pathlist));
    length = cleanup_dirname(buff, buff);
    if (!(copy = my_strndup(key_memory_MY_TMPDIR_full_list, buff, length,
                            MYF(MY_WME))) ||
        full_list.push_back(copy))
      return true;
    pathlist = end + 1;
  } while (*end);

  tmpdir->list = static_cast<char **>(
      my_malloc(key_memory_MY_TMPDIR_full_list,
                sizeof(char *) * full_list.size(), MYF(MY_WME)));
  if (tmpdir->list == nullptr) return true;

  mysql_mutex_init(key_TMPDIR_mutex, &tmpdir->mutex, MY_MUTEX_INIT_FAST);
  memcpy(tmpdir->list, &full_list[0], sizeof(char *) * full_list.size());
  tmpdir->max = full_list.size() - 1;
  tmpdir->cur = 0;
  return false;
}

char *my_tmpdir(MY_TMPDIR *tmpdir) {
  if (0 == tmpdir->max) return tmpdir->list[0];

  MUTEX_LOCK(lock, &tmpdir->mutex);
  char *dir = tmpdir->list[tmpdir->cur];
  tmpdir->cur = (tmpdir->cur == tmpdir->max) ? 0 : tmpdir->cur + 1;

  return dir;
}

void free_tmpdir(MY_TMPDIR *tmpdir) {
  if (tmpdir->list == nullptr) return;
  for (uint i = 0; i <= tmpdir->max; i++) my_free(tmpdir->list[i]);
  my_free(tmpdir->list);
  mysql_mutex_destroy(&tmpdir->mutex);
}
