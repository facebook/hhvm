/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/generate/json.h>

#include <fmt/core.h>

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <sstream>

namespace apache {
namespace thrift {
namespace compiler {

namespace {
// Trim all white spaces and commas from end (in place).
void rtrim(std::string& s) {
  s.erase(
      std::find_if(
          s.rbegin(),
          s.rend(),
          [](int ch) { return !(ch == ' ' || ch == ','); })
          .base(),
      s.end());
}
} // namespace

std::string to_json(const t_const_value* value) {
  auto stringify_list = [](const auto& value) {
    std::string result;
    for (const auto& v : value) {
      result += to_json(v) + ", ";
    }
    rtrim(result);
    return "[" + result + "]";
  };

  auto stringify_map = [](const auto& value) {
    std::string result;
    for (const auto& v : value) {
      auto key = to_json(v.first);
      if (v.first->kind() != t_const_value::CV_STRING) {
        // map keys must be strings
        key = json_quote_ascii(key);
      }
      result += key + ": " + to_json(v.second) + ", ";
    }
    rtrim(result);
    return "{" + result + "}";
  };

  switch (value->kind()) {
    case t_const_value::CV_BOOL:
      return value->get_bool() ? "true" : "false";
    case t_const_value::CV_INTEGER:
      return std::to_string(value->get_integer());
    case t_const_value::CV_DOUBLE:
      return fmt::format("{}", value->get_double());
    case t_const_value::CV_STRING:
      return json_quote_ascii(value->get_string());
    case t_const_value::CV_LIST:
      return stringify_list(value->get_list());
    case t_const_value::CV_MAP:
      return stringify_map(value->get_map());
    case t_const_value::CV_IDENTIFIER:
      break;
  }
  return "";
}

std::string json_quote_ascii(const std::string& s) {
  std::ostringstream o;
  json_quote_ascii(o, s);
  return o.str();
}

std::ostream& json_quote_ascii(std::ostream& o, const std::string& s) {
  o << "\"";
  for (char c : s) {
    switch (c) {
        // clang-format off
      case '"':  o << "\\\""; break;
      case '\\': o << "\\\\"; break;
      case '\b': o << "\\b";  break;
      case '\f': o << "\\f";  break;
      case '\n': o << "\\n";  break;
      case '\r': o << "\\r";  break;
      // clang-format on
      default: {
        uint8_t b = static_cast<uint8_t>(c);
        if (!(b >= 0x20 && b < 0x80)) {
          constexpr auto hex = "0123456789abcdef";
          auto c1 = static_cast<char>(hex[(b >> 4) & 0x0f]);
          auto c0 = static_cast<char>(hex[(b >> 0) & 0x0f]);
          o << "\\u00" << c1 << c0;
        } else {
          o << c;
        }
      }
    }
  }
  o << "\"";
  return o;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
