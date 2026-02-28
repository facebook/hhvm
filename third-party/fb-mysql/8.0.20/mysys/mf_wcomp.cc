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
  @file mysys/mf_wcomp.cc
  Functions for comparing with wild-cards.
*/
#include "mf_wcomp.h"

#include "my_dbug.h"

/*
  This function is different from the wildcmp function of collations.
  1. This function doesn't guard it from stack overrun.
  2. This function is not aware of charset.
  3. This function's expression string can be considered as pattern. Please
  refer to the declaration.
*/
int wild_compare_full(const char *str, int strlen, const char *wildstr,
                      int wildlen, bool str_is_pattern, char w_prefix,
                      char w_one, char w_many) {
  const char *strend = str + strlen;
  const char *wildend = wildstr + wildlen;
  char cmp;
  DBUG_TRACE;

  while (wildstr < wildend) {
    /*
      Loop through expression string (str) and pattern string (wildstr) byte by
      byte until they are different, or we find a wildcard char (w_many or
      w_one) in pattern string.
    */
    while (wildstr < wildend && *wildstr != w_many && *wildstr != w_one) {
      if (*wildstr == w_prefix && wildstr + 1 < wildend) {
        wildstr++;
        /*
          If there is a escape char in pattern string, and expression string can
          be considered as pattern, there should be a escape char in input
          string too.
        */
        if (str_is_pattern && str < strend && *str++ != w_prefix) return 1;
      }
      if (str == strend || *wildstr++ != *str++) return 1;
    }
    if (wildstr == wildend) return str < strend;
    /*
      Skip one char if wildcard is w_one. If expression string can be
      considered as pattern, any char in expression string except of w_many can
      be skipped.
    */
    if (*wildstr++ == w_one) {
      if (str == strend || (str_is_pattern && *str == w_many))
        return 1; /* One char; skip */
      if (*str++ == w_prefix && str_is_pattern) str++;
    } else { /* Found '*' */
      /*
        If wildcard char is w_many, then we skip any wildcard char following
        it.
      */
      while (str_is_pattern && str < strend && *str == w_many) str++;
      for (; wildstr < wildend && (*wildstr == w_many || *wildstr == w_one);
           wildstr++) {
        if (*wildstr == w_many) {
          while (str_is_pattern && str < strend && *str == w_many) str++;
        } else {
          if (str_is_pattern && str + 1 < strend && *str == w_prefix)
            str += 2;
          else if (str == strend)
            return 1;
        }
      }
      if (wildstr == wildend) return 0; /* '*' as last char: OK */
      if ((cmp = *wildstr) == w_prefix && wildstr + 1 < wildend &&
          !str_is_pattern)
        cmp = wildstr[1];
      // cmp is the character following w_many.
      for (;; str++) {
        /*
          Skip until we find a character in the expression string that is
          equal to cmp. For the character not equal to cmp, we consider they are
          all matched by w_many.
        */
        while (str < strend && *str != cmp) str++;
        if (str == strend) return 1;
        // Recursively call ourselves until we find a match.
        if (wild_compare_full(str, strend - str, wildstr, wildend - wildstr,
                              str_is_pattern, w_prefix, w_one, w_many) == 0)
          return 0;
      }
      /* We will never come here */
    }
  }
  return str < strend;
} /* wild_compare */

int wild_compare(const char *str, int strlen, const char *wildstr, int wildlen,
                 bool str_is_pattern) {
  return wild_compare_full(str, strlen, wildstr, wildlen, str_is_pattern,
                           wild_prefix, wild_one, wild_many);
}

bool contains_wildcard(const char *str) {
  while (*str) {
    if (*str == wild_one || *str == wild_many) return true;
    /* Skip \_ or \% */
    if (*str == wild_prefix && str[1])
      str += 2;
    else
      str++;
  }

  return false;
}
