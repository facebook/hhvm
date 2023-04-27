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
  @file mysys/mf_format.cc
*/

#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <cstring>

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h"

/**
  Formats a filename with possible replace of directory of extension
  Function can handle the case where 'to' == 'name'
  For a description of the flag values, consult my_sys.h
  The arguments should be in unix format.

  Pre-condition: At least FN_REFLEN bytes can be stored in buffer
  pointed to by 'to'. 'name', 'dir' and 'extension' are
  '\0'-terminated byte buffers (or nullptr where permitted).

  Post-condition: At most FN_REFLEN bytes will have been written to
  'to'. If the combined length of 'from' and any expanded elements
  exceeds FN_REFLEN-1, the result is truncated and likely not what the
  caller expects.

  @param to destination buffer.
  @param name file name component.
  @param dir directory component.
  @param extension filename extension
  @param flag

  @return to (destination buffer), or nullptr if overflow and
  MY_SAFE_PATH is used.
*/

char *fn_format(char *to, const char *name, const char *dir,
                const char *extension, uint flag) {
  char dev[FN_REFLEN], buff[FN_REFLEN], *pos;
  const char *ext;
  size_t dev_length;
  DBUG_TRACE;
  DBUG_ASSERT(name != nullptr);
  DBUG_ASSERT(extension != nullptr);
  DBUG_PRINT("enter", ("name: %s  dir: %s  extension: %s  flag: %d", name, dir,
                       extension, flag));

  /* Copy and skip directory */
  const char *startpos = name;
  size_t length = dirname_part(dev, name, &dev_length);
  name += length;
  if (length == 0 || (flag & MY_REPLACE_DIR)) {
    DBUG_ASSERT(dir != nullptr);
    /* Use given directory */
    convert_dirname(dev, dir, NullS); /* Fix to this OS */
  } else if ((flag & MY_RELATIVE_PATH) && !test_if_hard_path(dev)) {
    DBUG_ASSERT(dir != nullptr);
    /* Put 'dir' before the given path */
    strmake(buff, dev, sizeof(buff) - 1);
    pos = convert_dirname(dev, dir, NullS);
    strmake(pos, buff, sizeof(buff) - 1 - (int)(pos - dev));
  }

  if (flag & MY_UNPACK_FILENAME)
    (void)unpack_dirname(dev, dev); /* Replace ~/.. with dir */

  if (!(flag & MY_APPEND_EXT) &&
      (pos = const_cast<char *>(std::strchr(name, FN_EXTCHAR))) != nullptr) {
    if ((flag & MY_REPLACE_EXT) == 0) /* If we should keep old ext */
    {
      length = strlength(name); /* Use old extension */
      ext = "";
    } else {
      length = pos - name; /* Change extension */
      ext = extension;
    }
  } else {
    length = strlength(name); /* No ext, use the now one */
    ext = extension;
  }

  if (strlen(dev) + length + strlen(ext) >= FN_REFLEN || length >= FN_LEN) {
    /* To long path, return original or NULL */
    size_t tmp_length;
    if (flag & MY_SAFE_PATH) return NullS;
    tmp_length = strlength(startpos);
    DBUG_PRINT("error",
               ("dev: '%s'  ext: '%s'  length: %u", dev, ext, (uint)length));
    (void)strmake(to, startpos, std::min(tmp_length, size_t{FN_REFLEN - 1}));
  } else {
    if (to == startpos) {
      memmove(buff, name, length); /* Save name for last copy */
      name = buff;
    }
    pos = strmake(my_stpcpy(to, dev), name, length);
    (void)my_stpcpy(pos, ext); /* Don't convert extension */
  }
  /*
    If MY_RETURN_REAL_PATH and MY_RESOLVE_SYMLINK is given, only do
    realpath if the file is a symbolic link
  */
  if (flag & MY_RETURN_REAL_PATH)
    (void)my_realpath(to, to,
                      MYF(flag & MY_RESOLVE_SYMLINKS ? MY_RESOLVE_LINK : 0));
  else if (flag & MY_RESOLVE_SYMLINKS) {
    my_stpcpy(buff, to);
    (void)my_readlink(to, buff, MYF(0));
  }
  return to;
} /* fn_format */

/**
  Calculate the length of str not including any trailing ' '-bytes.

  Pre-condition: 'str' is a '\0'-terminated byte buffer.
  @param str input to calculate the length of.
  @return length of string with end-space:s not counted.
*/

size_t strlength(const char *str) {
  const char *pos;
  const char *found;
  DBUG_TRACE;

  pos = found = str;

  while (*pos) {
    if (*pos != ' ') {
      while (*++pos && *pos != ' ') {
      };
      if (!*pos) {
        found = pos; /* String ends here */
        break;
      }
    }
    found = pos;
    while (*++pos == ' ') {
    };
  }
  return (size_t)(found - str);
} /* strlength */
