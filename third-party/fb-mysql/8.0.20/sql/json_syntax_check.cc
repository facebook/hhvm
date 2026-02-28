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

#include "sql/json_syntax_check.h"

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/reader.h>
#include <string>
#include <utility>

#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"

bool Syntax_check_handler::StartObject() {
  m_too_deep_error_raised = check_json_depth(++m_depth);
  return !m_too_deep_error_raised;
}

bool Syntax_check_handler::EndObject(rapidjson::SizeType) {
  --m_depth;
  return true;
}

bool Syntax_check_handler::StartArray() {
  m_too_deep_error_raised = check_json_depth(++m_depth);
  return !m_too_deep_error_raised;
}

bool Syntax_check_handler::EndArray(rapidjson::SizeType) {
  --m_depth;
  return true;
}

bool is_valid_json_syntax(const char *text, size_t length, size_t *error_offset,
                          std::string *error_message) {
  Syntax_check_handler handler;
  rapidjson::Reader reader;
  rapidjson::MemoryStream ms(text, length);
  const bool valid = reader.Parse<rapidjson::kParseDefaultFlags>(ms, handler);

  if (!valid && (error_offset != nullptr || error_message != nullptr)) {
    const std::pair<std::string, size_t> error = get_error_from_reader(reader);

    if (error_offset != nullptr) {
      *error_offset = error.second;
    }
    if (error_message != nullptr) {
      error_message->assign(error.first);
    }
  }

  return valid;
}

/// The maximum number of nesting levels allowed in a JSON document.
static constexpr int JSON_DOCUMENT_MAX_DEPTH = 100;

bool check_json_depth(size_t depth) {
  if (depth > JSON_DOCUMENT_MAX_DEPTH) {
    my_error(ER_JSON_DOCUMENT_TOO_DEEP, MYF(0));
    return true;
  }
  return false;
}

std::pair<std::string, size_t> get_error_from_reader(
    const rapidjson::Reader &reader) {
  DBUG_ASSERT(reader.GetParseErrorCode() != rapidjson::kParseErrorNone);
  return std::make_pair(
      std::string(rapidjson::GetParseError_En(reader.GetParseErrorCode())),
      reader.GetErrorOffset());
}
