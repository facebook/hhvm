/* Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef UNIT_TEST_COMMON
#define UNIT_TEST_COMMON

#include "unit_test_common.h"
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#include <direct.h>  // getcwd
#else
#include <unistd.h>  // getcwd
#endif

#define FN_REFLEN 2048
bool make_absolute_urn(const char *input_urn, std::string *out_path) {
  char component_dir[FN_REFLEN];
  if (getcwd(component_dir, FN_REFLEN) == NULL) {
    return true;
  }
  /* Omit scheme prefix to get filename. */
  const char *file = strstr(input_urn, "://");
  if (file == NULL) {
    return true;
  }
  /* Offset by "://" */
  file += 3;

  /* Compile library path */
  std::string path = component_dir;
  path += "/";
  path += file;

  char path_buff[FN_REFLEN + 1];
  strcpy(path_buff, path.c_str());

  *out_path = "file://";
  *out_path += path_buff;

  return false;
}

char *make_str(char *dst, const char *src, size_t length) {
  while (length--)
    if (!(*dst++ = *src++)) return dst - 1;
  *dst = 0;
  return dst;
}

/*
  Resolve all symbolic links in path
  'to' may be equal to 'filename'
*/
int make_realpath(char *to, const char *filename) {
#ifndef _WIN32
  int result = 0;

  unique_ptr_free<char> ptr(realpath(filename, nullptr));
  if (ptr) {
    make_str(to, ptr.get(), FN_REFLEN - 1);
  } else {
    result = -1;
  }
  return result;
#else
  int ret = GetFullPathName(filename, FN_REFLEN, to, NULL);
  if (ret == 0) {
    return -1;
  }
  return 0;
#endif
}

size_t make_dirname_length(const char *name) {
  const char *pos = name - 1;
  const char *gpos = pos++;
  for (; *pos; pos++) /* Find last FN_LIBCHAR */
  {
    if (directory_separator(*pos)) gpos = pos;
  }
  return gpos + 1 - name;
}

char *make_convert_dirname(char *to, const char *from, const char *from_end) {
  char *to_org = to;

  /* We use -2 here, becasue we need place for the last FN_LIBCHAR */
  if (!from_end || (from_end - from) > FN_REFLEN - 2)
    from_end = from + FN_REFLEN - 2;

#if FN_LIBCHAR != '/'
  {
    for (; from < from_end && *from; from++) {
      if (*from == '/')
        *to++ = FN_LIBCHAR;
      else {
        *to++ = *from;
      }
    }
    *to = 0;
  }
#else
  /* This is ok even if to == from, becasue we need to cut the string */
  to = make_str(to, from, (size_t)(from_end - from));
#endif

  /* Add FN_LIBCHAR to the end of directory path */
  if (to != to_org && (to[-1] != FN_LIBCHAR && to[-1] != FN_DEV_CHAR)) {
    *to++ = FN_LIBCHAR;
    *to = 0;
  }
  return to; /* Pointer to end of dir */
}

size_t make_dirname_part(char *to, const char *name, size_t *to_res_length) {
  size_t length;
  length = make_dirname_length(name);
  *to_res_length = (size_t)(make_convert_dirname(to, name, name + length) - to);
  return length;
}

/* Set new working directory */

int make_setwd(const char *dir) {
  int res;

  if (!dir[0] || (dir[0] == FN_LIBCHAR && dir[1] == 0)) dir = FN_ROOTDIR;
  if ((res = chdir(dir)) != 0) {
    return res;
  }
  return res;
} /* my_setwd */

#endif
