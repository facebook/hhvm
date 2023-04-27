/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef REGULAR_EXPRESSIONS_INCLUDED
#define REGULAR_EXPRESSIONS_INCLUDED

#include <regex>

#include "mysql/psi/psi_base.h"
#include "prealloced_array.h"

extern std::regex explain_re;
extern std::regex opt_trace_re;
extern std::regex ps_re;
extern std::regex sp_re;
extern std::regex view_re;

struct st_regex {
  char *pattern; /* Pattern to be replaced */
  char *replace; /* String or expression to replace the pattern with */
  int icase;     /* true if the match is case insensitive */
};

struct st_replace_regex {
  st_replace_regex() : regex_arr(PSI_NOT_INSTRUMENTED) {}
  /* stores a list of st_regex subsitutions */
  Prealloced_array<st_regex, 128> regex_arr;

  /*
    Temporary storage areas for substitutions. To reduce unnessary copying
    and memory freeing/allocation, we pre-allocate two buffers, and alternate
    their use, one for input/one for output, the roles changing on the next
    st_regex substition. At the end of substitutions  buf points to the
    one containing the final result.
  */
  char *buf{nullptr};
  char *even_buf{nullptr};
  char *odd_buf{nullptr};
  int even_buf_len{0};
  int odd_buf_len{0};
};

int multi_reg_replace(struct st_replace_regex *r, char *val, size_t *len);
int search_protocol_re(std::regex *re, const char *str);

#endif  // REGULAR_EXPRESSIONS_INCLUDED
