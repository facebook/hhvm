/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_BASENAME_INCLUDED
#define MY_BASENAME_INCLUDED

/**
  @file include/my_basename.h
  A macro that gives __FILE__ without the directory name (e.g. foo.cc instead of
  ../sql/foo.cc), calculated compile-time. Note that the entire __FILE__
  string is still present in the binary; only the pointer is adjusted.
*/

static constexpr int basename_index(const char *const path, const int index) {
  return (index == -1 || path[index] == '/' || path[index] == '\\')
             ? index + 1
             : basename_index(path, index - 1);
}

#define MY_BASENAME (&__FILE__[basename_index(__FILE__, sizeof(__FILE__) - 1)])

#ifndef LOG_SUBSYSTEM_TAG
constexpr const char *basename_prefix_eval(const char *const path) {
  return (path[0] == 'r' && path[1] == 'p' && path[2] == 'l' && path[3] == '_')
             ? "Repl"
             : nullptr;
}

constexpr int basename_prefix_find(const char *const path, const int index) {
  return (path[index] == '/' || path[index] == '\\')
             ? index + 1
             : basename_prefix_find(path, index - 1);
}

#define LOG_SUBSYSTEM_TAG \
  basename_prefix_eval(   \
      &__FILE__[basename_prefix_find(__FILE__, sizeof(__FILE__) - 1)])

#endif

#endif  // MY_BASENAME_INCLUDED
