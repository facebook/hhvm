/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
  This file defines various function for checking if a string is a valid JSON.
  The checking is done by the rapidjson library, but we extend the checks a bit
  further by rejecting JSON objects/array that are nested deeper than
  JSON_DOCUMENT_MAX_DEPTH levels.
*/

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/reader.h>
#include <string>
#include <utility>

/**
  Check if the depth of a JSON document exceeds the maximum supported
  depth (JSON_DOCUMENT_MAX_DEPTH). Raise an error if the maximum depth
  has been exceeded.

  @param[in] depth  the current depth of the document
  @return true if the maximum depth is exceeded, false otherwise
*/
bool check_json_depth(size_t depth);

/**
  This class implements a handler for use with rapidjson::Reader when
  we want to check if a string is a valid JSON text. The handler does
  not build a DOM structure, so it is quicker than Json_dom::parse()
  in the cases where we don't care about the DOM, such as in the
  JSON_VALID() function.

  The handler keeps track of how deeply nested the document is, and it
  raises an error and stops parsing when the depth exceeds
  JSON_DOCUMENT_MAX_DEPTH.

  All the member functions follow the rapidjson convention of
  returning true on success and false on failure.
*/
class Syntax_check_handler : public rapidjson::BaseReaderHandler<> {
 public:
  bool StartObject();
  bool EndObject(rapidjson::SizeType);
  bool StartArray();
  bool EndArray(rapidjson::SizeType);

  bool too_deep_error_raised() const { return m_too_deep_error_raised; }

 private:
  size_t m_depth{0};  ///< The current depth of the document

  bool m_too_deep_error_raised{false};
};

/**
  Check if a string is a valid JSON.

  @param[in] text A pointer to the start of the text
  @param[in] length The length of the text
  @param[out] error_offset If the text is not a valid JSON, this variable will
                           be set to the position in the input string where
                           the error occured. Can be nullptr.
  @param[out] error_message If the text is not a valid JSON, this variable
                            will contain a readable error message. Can be
                            nullptr.
  @retval true if the input text is a valid JSON.
  @retval false if the input text is not a valid JSON.
*/
bool is_valid_json_syntax(const char *text, size_t length, size_t *error_offset,
                          std::string *error_message);

/**
  Extract a readable error from a rapidjson reader and return it to the
  caller.

  @param[in] reader The rapidjson reader to extract the error from.
  @return A pair where the first element is a readable error and the second
          element is the position in the input string where the error was
          found.
*/
std::pair<std::string, size_t> get_error_from_reader(
    const rapidjson::Reader &reader);
