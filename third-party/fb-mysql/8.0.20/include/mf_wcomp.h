/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MF_WCOMP_INCLUDED
#define MF_WCOMP_INCLUDED

/**
  Character constant for the escape character in a wildcard pattern
  (SQL style).
*/
const char wild_prefix = '\\';

/**
  Character constant for wildcard representing any one character
  (SQL style).
*/
const char wild_one = '_';

/**
  Character constant for wildcard representing zero or more
  characters (SQL style).
*/
const char wild_many = '%';

/**
  Performs wildcard matching, aka globbing, on the input string with
  the given wildcard pattern, and the specified wildcard characters.

  Note that when str_is_pattern is true, an escaped wildcard in the
  pattern will only match an escaped wildcard in the string, e.g the
  string "my_1" will *not* be matched by the pattern "my\\_1".

  @deprecated This function is not charset-aware, and should not be
  used in new code.

  @param str input which should be matched against pattern
  @param strlen length of str in bytes
  @param wildstr pattern with wildcards
  @param wildlen length of wildstr in bytes

  @param str_is_pattern if true the input string is considered to be a
  pattern, meaning that escape sequences in the input are processed as
  if they appeared in the pattern
  @param w_one wildcard character matching any single character
  @param w_many wildcard character matching 0 or more characters
  @param w_prefix escape character for the pattern

  @return 0 if match, 1 otherwise
*/
int wild_compare_full(const char *str, int strlen, const char *wildstr,
                      int wildlen, bool str_is_pattern, char w_prefix,
                      char w_one, char w_many);

/**
  Performs wildcard matching, aka globbing, on the input string with
  the given wildcard pattern, using the standard SQL wildcard ('_',
  '%' and '\\') notation.

  @deprecated This function is not charset-aware, and should not be
  used in new code.

  @param str input which should be matched against pattern
  @param strlen length of str in bytes
  @param wildstr pattern with wildcards
  @param wildlen length of wildstr in bytes

  @param str_is_pattern if true the input string is considered to be a
  pattern, meaning that escape sequences in the input are processed as
  if they appeared in the pattern.

  @return 0 if match, 1 otherwise
 */
int wild_compare(const char *str, int strlen, const char *wildstr, int wildlen,
                 bool str_is_pattern);

bool contains_wildcard(const char *str);

#endif /* !MF_WCOMP_INCLUDED */
