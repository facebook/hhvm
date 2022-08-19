#ifndef OPTIONS_PARSER_H_INCLUDED
#define OPTIONS_PARSER_H_INCLUDED

/*
  Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <map>
#include <string>

class String;

namespace options_parser {

/**
  Parses options string into a map of keys and values, raises
  an error if parsing is unsuccessful.

  @param str unparsed options string
  @param map map to fill with keys and values
  @param func_name function name used in error reporting

  @retval false parsing was successful
  @retval true parsing was successful
*/
bool parse_string(String *str, std::map<std::string, std::string> *map,
                  const char *func_name);

}  // namespace options_parser

#endif  // OPTIONS_PARSER_H_INCLUDED
