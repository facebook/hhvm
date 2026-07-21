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

#include <cctype>
#include <optional>
#include <string_view>
#include <utility>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::basic_ast_visitor;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_field;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::codemod::file_manager;

namespace {

bool is_identifier_start(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool is_identifier_char(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::optional<std::pair<size_t, size_t>> find_qualifier(
    std::string_view source, size_t begin, size_t end) {
  std::optional<std::pair<size_t, size_t>> last_identifier;

  for (size_t pos = begin; pos < end;) {
    if (std::isspace(static_cast<unsigned char>(source[pos]))) {
      ++pos;
      continue;
    }
    if (source.substr(pos, 2) == "//") {
      pos = source.find('\n', pos + 2);
      if (pos == std::string_view::npos || pos >= end) {
        break;
      }
      continue;
    }
    if (source.substr(pos, 2) == "/*") {
      pos = source.find("*/", pos + 2);
      if (pos == std::string_view::npos || pos + 2 >= end) {
        break;
      }
      pos += 2;
      continue;
    }
    if (source[pos] == '\'' || source[pos] == '"') {
      const char quote = source[pos++];
      while (pos < end && source[pos] != quote) {
        pos += source[pos] == '\\' && pos + 1 < end ? 2 : 1;
      }
      pos += pos < end ? 1 : 0;
      last_identifier.reset();
      continue;
    }
    if (is_identifier_start(source[pos])) {
      const size_t identifier_begin = pos++;
      while (pos < end && is_identifier_char(source[pos])) {
        ++pos;
      }
      last_identifier = std::pair(identifier_begin, pos);
      continue;
    }
    last_identifier.reset();
    ++pos;
  }

  if (!last_identifier) {
    return std::nullopt;
  }
  const auto [qualifier_begin, qualifier_end] = *last_identifier;
  const std::string_view qualifier =
      source.substr(qualifier_begin, qualifier_end - qualifier_begin);
  if (qualifier != "required" && qualifier != "optional") {
    return std::nullopt;
  }
  if (qualifier_begin != begin &&
      !std::isspace(static_cast<unsigned char>(source[qualifier_begin - 1])) &&
      source[qualifier_begin - 1] != ':') {
    return std::nullopt;
  }
  return last_identifier;
}

void remove_qualifier(file_manager& fm, const t_field& field) {
  const size_t field_begin = field.src_range().begin.offset();
  const size_t type_begin = field.type().src_range().begin.offset();
  auto qualifier = find_qualifier(fm.old_content(), field_begin, type_begin);
  if (!qualifier) {
    return;
  }

  auto [qualifier_begin, qualifier_end] = *qualifier;
  while (qualifier_end < type_begin &&
         std::isspace(
             static_cast<unsigned char>(fm.old_content()[qualifier_end]))) {
    ++qualifier_end;
  }
  fm.add({qualifier_begin, qualifier_end, ""});
}

} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        t_program& program = *pb.root_program();
        basic_ast_visitor<true, file_manager&> visitor;
        visitor.add_function_param_visitor(remove_qualifier);
        file_manager fm(sm, program);
        visitor(fm, program);
        fm.apply_replacements();
      });
}
