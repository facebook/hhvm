/*
   Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef C_STRING_LESS_INCLUDED
#define C_STRING_LESS_INCLUDED

#include <string.h>

/**
  @file include/c_string_less.h
*/

/**
  Compares two strings for order, assures ascending lexicographical order.
  Is used as a const char * std::less specialization.
*/
struct c_string_less {
  /**
    Compares two strings for order.

    @param s1 String to compare.
    @param s2 String to compare.
    @return Result of comparison, assures ascending lexicographical order.
  */
  bool operator()(const char *s1, const char *s2) const {
    return strcmp(s1, s2) < 0;
  }
};

#endif
