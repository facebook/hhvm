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

#ifndef UNIT_TEST_COMMON_H
#define UNIT_TEST_COMMON_H

#include <memory>
#include <string>

#ifdef _WIN32
#define FN_LIBCHAR '\\'
#define FN_LIBCHAR2 '/'
#define FN_ROOTDIR "\\"
#else
#define FN_LIBCHAR '/'
#define FN_ROOTDIR "/"
#endif

#define FN_DEV_CHAR '\0' /* For easier code */

struct Free_deleter {
  void operator()(void *ptr) const { free(ptr); }
};

/** std::unique_ptr, but with free as deleter. */
template <class T>
using unique_ptr_free = std::unique_ptr<T, Free_deleter>;

static inline int directory_separator(char c) {
#ifdef _WIN32
  return c == FN_LIBCHAR || c == FN_LIBCHAR2;
#else
  return c == FN_LIBCHAR;
#endif
}

bool make_absolute_urn(const char *input_urn, std::string *out_path);
int make_realpath(char *to, const char *filename);
size_t make_dirname_part(char *to, const char *name, size_t *to_res_length);
int make_setwd(const char *dir);

extern const char *progname;
#define INIT(name) \
  { progname = name; }

#endif
