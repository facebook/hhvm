/*
  Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "client/dump/pattern_matcher.h"

using namespace Mysql::Tools::Dump::Detail;

Pattern_matcher::Pattern_matcher() {}

bool Pattern_matcher::is_pattern_matched(const std::string &to_match,
                                         const std::string &pattern,
                                         size_t i /*= 0*/, size_t j /*= 0*/) {
  while (i < to_match.size() && j < pattern.size()) {
    if (pattern[j] == '%') {
      /*
      Check two possibilities: either we stop consuming to_match with
      this instance or we consume one more character.
      */
      if (is_pattern_matched(to_match, pattern, i + 1, j)) return true;
      j++;
    } else if (pattern[j] == '_' || pattern[j] == to_match[i]) {
      i++;
      j++;
    } else
      return false;
  }
  /*
  There might be % pattern matching characters on the end of pattern, we
  can omit them.
  */
  while (j < pattern.size() && pattern[j] == '%') j++;
  return i == to_match.size() && j == pattern.size();
}
