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

#ifndef _print_version_h_
#define _print_version_h_

#ifdef __cplusplus
#include <string>

extern "C" {
#endif /* __cplusplus */
/**
  @file include/print_version.h
*/

/**
  This function prints a standard version string. Should be used by
  all utilities.
*/

void print_version();

/**
  This function prints a standard version string, with '-debug' added
  to the name of the executable. Used by utilties that have an
  explicit need to state that they have been compiled in debug mode.
*/

void print_version_debug();

/**
  This function prints a version string with the released version
  supplied by the caller. Used by the server process which needs to
  print if it is compiled with debug, ASAN, UBSAN or running with
  Valgrind.

  @param[in] version  Null-terminated release version string
*/

void print_explicit_version(const char *version);

#ifdef __cplusplus
/**
  This function builds a version string, with the program name
  supplied by the caller. Used by MEB and other utilities that want to
  present themselves under their own name.

  @param[in] progname  Program name C++ string.

  @param[out] destination Output buffer.

*/

void build_version(const std::string &progname, std::string *destination);
} /* extern "C" */
#endif /* __cplusplus */
#endif /* _print_version_h_  */
