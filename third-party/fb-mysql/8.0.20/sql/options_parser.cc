/*
  Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/options_parser.h"

#include <stddef.h>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql_string.h"

namespace options_parser {

enum class parser_status {
  OK,
  MISSING_SEPARATOR,
  MORE_THAN_ONE_SEPARATOR,
  SEQUENTIAL_SEPARATORS,
  SEQUENTIAL_DELIMITERS,
  STARTS_WITH_INVALID_CHARACTER,
  ENDS_WITH_INVALID_CHARACTER,
  DUPLICATE_KEY,
};

/**
  Struct for storing the parsing result. Gives the user
  specific feedback if an invalid string is entered.
*/
struct result {
  /**
    Whether parsing was successful or not, and what type of error
    occurred, if any.
  */
  parser_status status;
  /**
    Information about the specific error that occurred.
  */
  std::string msg;
  /**
    Character that caused the parsing to fail.
  */
  char c;
};

/**
  Parses the options string into keys and values

  Parses an options string into a  set of keys and values, and stores the
  result in an an std::map pointer. The string is split based on a delimiter
  and a key-value separator, and might look something like:
  "key1=value1, key2=value2, key3=value3...".

  @param str unparsed options string
  @param delimiter character to split the options string into several substrings
  @param key_value_separator value which seperates string into key and value
  @param options pointer to an std::map

  @return result
*/
result parse_options_string(String *str, char delimiter,
                            char key_value_separator,
                            std::map<std::string, std::string> *options) {
  std::string str_copy(str->ptr(), str->length());

  size_t str_start_pos = 0;
  size_t str_end_pos = str_copy.length() - 1;
  result parser_result;

  if (str_copy.size() == 0) {
    parser_result.status = parser_status::OK;
    parser_result.msg = "";
    return parser_result;
  }

  // Remove whitespaces at beginning of string.
  for (size_t i = 0; i <= str_copy.size() - 1; i++) {
    if (!std::isspace(str_copy.at(i))) {
      str_start_pos = i;
      break;
    }

    // In case of string containing only whitespaces.
    if (i == str_copy.size() - 1) {
      parser_result.status = parser_status::OK;
      parser_result.msg = "";
      return parser_result;
    }
  }

  // Remove whitespaces at end of string.
  for (int i = static_cast<int>(str_copy.size()) - 1; i >= 0; i--) {
    if (!std::isspace(str_copy.at(i))) {
      if (i < static_cast<int>(str_copy.size() - 1)) {
        str_end_pos = i;
      }

      break;
    }
  }

  std::transform(str_copy.begin(), str_copy.end(), str_copy.begin(), ::tolower);

  // String cannot start or end with the delimiter value.
  if (str_copy.at(str_start_pos) == delimiter) {
    parser_result.status = parser_status::STARTS_WITH_INVALID_CHARACTER;
    parser_result.c = delimiter;
    return parser_result;
  }

  if (str_copy.at(str_start_pos) == key_value_separator) {
    parser_result.status = parser_status::STARTS_WITH_INVALID_CHARACTER;
    parser_result.c = key_value_separator;
    return parser_result;
  }

  if (str_copy.at(str_end_pos) == delimiter) {
    parser_result.status = parser_status::ENDS_WITH_INVALID_CHARACTER;
    parser_result.c = delimiter;
    return parser_result;
  }

  if (str_copy.at(str_end_pos) == key_value_separator) {
    parser_result.status = parser_status::ENDS_WITH_INVALID_CHARACTER;
    parser_result.c = key_value_separator;
    return parser_result;
  }

  size_t substring_startpos = str_start_pos;
  size_t delimiter_count =
      std::count(str_copy.begin(), str_copy.end(), delimiter);
  size_t substring_endpos = 0;
  size_t key_value_separator_pos = 0;

  for (size_t i = 0; i <= delimiter_count; i++) {
    int prep_whitespace_counter = 0;
    int trailing_whitespace_counter = 0;

    if (i == delimiter_count) {
      substring_endpos = str_end_pos;
    } else {
      DBUG_ASSERT(substring_endpos != std::string::npos);
      substring_endpos = str_copy.find(delimiter, substring_startpos);

      for (size_t j = substring_endpos - 1; j >= substring_startpos; j--) {
        if (std::isspace(str_copy.at(j))) {
          trailing_whitespace_counter++;
        } else {
          break;
        }
      }

      substring_endpos -= trailing_whitespace_counter;

      prep_whitespace_counter = 0;
      for (size_t j = substring_endpos + trailing_whitespace_counter + 1;
           j < str_copy.length(); j++) {
        if (std::isspace(str_copy.at(j))) {
          prep_whitespace_counter++;
        } else {
          break;
        }
      }

      if (str_copy.at(substring_endpos + trailing_whitespace_counter +
                      prep_whitespace_counter + 1) == delimiter) {
        parser_result.status = parser_status::SEQUENTIAL_DELIMITERS;
        parser_result.c = delimiter;
        return parser_result;
      }
      substring_endpos -= 1;  // Decrement to set end position at position
                              // before delimiter.
    }

    size_t key_value_separator_count =
        std::count(str_copy.begin() + substring_startpos,
                   str_copy.begin() + substring_endpos, key_value_separator);

    if (key_value_separator_count == 0) {
      parser_result.status = parser_status::MISSING_SEPARATOR;
      parser_result.msg = str_copy.substr(
          substring_startpos, substring_endpos - substring_startpos + 1);
      parser_result.c = key_value_separator;
      return parser_result;
    }

    key_value_separator_pos =
        str_copy.find(key_value_separator, substring_startpos);
    size_t key_startpos = substring_startpos;
    size_t key_endpos = key_value_separator_pos - 1;
    size_t value_startpos = key_value_separator_pos + 1;
    size_t value_endpos = substring_endpos;

    for (size_t j = key_value_separator_pos - 1; j >= substring_startpos; j--) {
      if (std::isspace(str_copy.at(j))) {
        key_endpos--;
      } else {
        break;
      }
    }

    // Remove whitespaces after key-value separator.
    for (size_t j = key_value_separator_pos + 1; j < str_copy.length(); j++) {
      if (std::isspace(str_copy.at(j))) {
        value_startpos++;
      } else {
        break;
      }
    }

    if (str_copy.at(value_startpos) == key_value_separator) {
      parser_result.status = parser_status::SEQUENTIAL_SEPARATORS;
      parser_result.c = key_value_separator;
      return parser_result;
    }

    std::string key =
        str_copy.substr(key_startpos, key_endpos - key_startpos + 1);
    std::string value =
        str_copy.substr(value_startpos, value_endpos - value_startpos + 1);
    if (key_value_separator_count > 1) {
      parser_result.status = parser_status::MORE_THAN_ONE_SEPARATOR;
      parser_result.msg = key + key_value_separator + value;
      parser_result.c = key_value_separator;
      return parser_result;
    }

    if (str_copy.at(substring_startpos) == key_value_separator) {
      parser_result.status = parser_status::STARTS_WITH_INVALID_CHARACTER;
      parser_result.c = key_value_separator;
      return parser_result;
    }

    if (str_copy.at(substring_endpos) == key_value_separator) {
      parser_result.status = parser_status::ENDS_WITH_INVALID_CHARACTER;
      parser_result.c = key_value_separator;
      return parser_result;
    }

    // Duplicate entry check.
    if (!options->emplace(key, value).second) {
      parser_result.status = parser_status::DUPLICATE_KEY;
      parser_result.msg = key;
      parser_result.c = key_value_separator;
      return parser_result;
    }

    // Skip delimiter.
    substring_startpos = substring_endpos + prep_whitespace_counter +
                         trailing_whitespace_counter + 2;
  }

  parser_result.status = parser_status::OK;

  return parser_result;
}

/**
  Determines result of parsing operation, raises error if parsing was
  unsuccessful.

  Determines the result of parsing operation by looking at the status
  field of the result struct, and raises an appropriate error if the
  parsing operation is unsuccessful.

  @param result struct returned from parsing function
  @param function_name name of function that uses parsing function

  @retval false parsing was successful
  @retval true parsing was ununsuccessful
*/
bool resolve_parser_result(result result, const char *function_name) {
  switch (result.status) {
    case parser_status::STARTS_WITH_INVALID_CHARACTER: {
      my_error(ER_INVALID_OPTION_START_CHARACTER, MYF(0), function_name,
               result.c);
      return true;
    }
    case parser_status::ENDS_WITH_INVALID_CHARACTER: {
      my_error(ER_INVALID_OPTION_END_CHARACTER, MYF(0), function_name,
               result.c);
      return true;
    }
    case parser_status::MISSING_SEPARATOR:
    case parser_status::MORE_THAN_ONE_SEPARATOR: {
      my_error(ER_INVALID_OPTION_KEY_VALUE_PAIR, MYF(0), result.msg.c_str(),
               result.c, function_name);
      return true;
    }
    case parser_status::SEQUENTIAL_SEPARATORS:
    case parser_status::SEQUENTIAL_DELIMITERS: {
      std::string invalid_character_sequence;
      invalid_character_sequence += result.c;
      invalid_character_sequence += result.c;
      my_error(ER_INVALID_OPTION_CHARACTERS, MYF(0), function_name,
               invalid_character_sequence.c_str());
      return true;
    }
    case parser_status::DUPLICATE_KEY: {
      my_error(ER_DUPLICATE_OPTION_KEY, MYF(0), result.msg.c_str(),
               function_name);
      return true;
    }
    case parser_status::OK: {
      return false;
    }
  }
  return true;
}

bool parse_string(String *str, std::map<std::string, std::string> *map,
                  const char *func_name) {
  /*
    Since we're using certain std::string functions (such as std::string.at()),
    the code might give an out_of_range exception. Therefore, a try-catch with
    a generic error message is necessary.
  */
  try {
    return resolve_parser_result(parse_options_string(str, ',', '=', map),
                                 func_name);
  } catch (const std::out_of_range &) {
    DBUG_ASSERT(false);
    my_error(ER_UNKNOWN_ERROR, MYF(0));
    return true;
  }
}

}  // namespace options_parser
