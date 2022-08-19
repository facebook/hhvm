/*
   Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <m_ctype.h>
#include <string>
#include <unordered_set>

/**
  Enables comparison of strings against particular set of patterns. Patterns
  may contain wildcards (WILD_ONE/WILD_MANY/WILD_ESCAPE). Pattern strings may
  be added to the class through a special method. Matching method traverses all
  of the patterns within pattern matcher in search for a match.
*/
class Pattern_matcher {
 public:
  size_t add_patterns(const std::string &patterns, char delimiter = ':');
  bool is_matching(const std::string &text, const CHARSET_INFO *info) const;
  void clear();

 private:
  /** any (single) character wild card */
  const static char WILD_ONE = '?';

  /** zero or many characters wild card */
  const static char WILD_MANY = '*';

  /** escape sequence character */
  const static char WILD_ESCAPE = '\\';

  /** used for storing matcher patterns */
  std::unordered_set<std::string> m_patterns;
};
