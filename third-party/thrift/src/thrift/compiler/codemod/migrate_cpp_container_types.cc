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
#include <string>
#include <string_view>
#include <vector>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_container.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>

using apache::thrift::compiler::const_ast_visitor;
using apache::thrift::compiler::cpp_name_resolver;
using apache::thrift::compiler::kCppTypeUri;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::source_range;
using apache::thrift::compiler::t_const;
using apache::thrift::compiler::t_container;
using apache::thrift::compiler::t_field;
using apache::thrift::compiler::t_list;
using apache::thrift::compiler::t_map;
using apache::thrift::compiler::t_named;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::t_set;
using apache::thrift::compiler::t_type;
using apache::thrift::compiler::t_typedef;
namespace codemod = apache::thrift::compiler::codemod;

namespace {

struct template_instantiation {
  std::string name;
  std::vector<std::string> arguments;
};

std::string_view trim(std::string_view value) {
  while (!value.empty() &&
         std::isspace(static_cast<unsigned char>(value.front()))) {
    value.remove_prefix(1);
  }
  while (!value.empty() &&
         std::isspace(static_cast<unsigned char>(value.back()))) {
    value.remove_suffix(1);
  }
  return value;
}

std::optional<template_instantiation> parse_template_instantiation(
    std::string_view value) {
  value = trim(value);
  const size_t open = value.find('<');
  if (open == std::string_view::npos || trim(value.substr(0, open)).empty()) {
    return std::nullopt;
  }

  int depth = 0;
  size_t argument_begin = open + 1;
  template_instantiation result{std::string(trim(value.substr(0, open))), {}};
  for (size_t i = open; i < value.size(); ++i) {
    switch (value[i]) {
      case '<':
        ++depth;
        break;
      case '>':
        if (--depth < 0) {
          return std::nullopt;
        }
        if (depth == 0) {
          auto argument =
              trim(value.substr(argument_begin, i - argument_begin));
          if (argument.empty() || !trim(value.substr(i + 1)).empty()) {
            return std::nullopt;
          }
          result.arguments.emplace_back(argument);
          return result;
        }
        break;
      case ',':
        if (depth == 1) {
          auto argument =
              trim(value.substr(argument_begin, i - argument_begin));
          if (argument.empty()) {
            return std::nullopt;
          }
          result.arguments.emplace_back(argument);
          argument_begin = i + 1;
        }
        break;
    }
  }
  return std::nullopt;
}

bool is_identifier_character(char value) {
  return std::isalnum(static_cast<unsigned char>(value)) || value == '_';
}

void replace_identifier(
    std::string& value, std::string_view from, std::string_view to) {
  size_t pos = 0;
  while ((pos = value.find(from, pos)) != std::string::npos) {
    if ((pos > 0 && is_identifier_character(value[pos - 1])) ||
        (pos + from.size() < value.size() &&
         is_identifier_character(value[pos + from.size()]))) {
      pos += from.size();
      continue;
    }
    value.replace(pos, from.size(), to);
    pos += to.size();
  }
}

std::string normalize_cpp_type(std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (size_t i = 0; i < value.size();) {
    if (std::isspace(static_cast<unsigned char>(value[i]))) {
      ++i;
      continue;
    }
    if (value.substr(i, 2) == "::" &&
        (result.empty() || result.back() == '<' || result.back() == ',' ||
         result.back() == '(')) {
      i += 2;
      continue;
    }
    result.push_back(value[i++]);
  }

  replace_identifier(result, "std::int8_t", "int8_t");
  replace_identifier(result, "std::int16_t", "int16_t");
  replace_identifier(result, "std::int32_t", "int32_t");
  replace_identifier(result, "std::int64_t", "int64_t");
  replace_identifier(result, "std::uint8_t", "uint8_t");
  replace_identifier(result, "std::uint16_t", "uint16_t");
  replace_identifier(result, "std::uint32_t", "uint32_t");
  replace_identifier(result, "std::uint64_t", "uint64_t");
  return result;
}

std::optional<size_t> find_closing_quote(
    std::string_view value, size_t opening_quote) {
  bool escaped = false;
  for (size_t i = opening_quote + 1; i < value.size(); ++i) {
    if (value[i] == value[opening_quote] && !escaped) {
      return i;
    }
    escaped = value[i] == '\\' && !escaped;
  }
  return std::nullopt;
}

std::vector<std::string> generated_arguments(
    const t_container& container, cpp_name_resolver& resolver) {
  if (const auto* list = container.try_as<t_list>()) {
    return {resolver.get_native_type(*list->elem_type())};
  }
  if (const auto* set = container.try_as<t_set>()) {
    return {resolver.get_native_type(*set->elem_type())};
  }
  const auto& map = container.as<t_map>();
  return {
      resolver.get_native_type(*map.key_type()),
      resolver.get_native_type(*map.val_type()),
  };
}

void migrate_cpp_type(
    codemod::file_manager& file_manager,
    cpp_name_resolver& resolver,
    const t_named& node,
    const t_type& type) {
  const t_type* true_type = type.get_true_type();
  if (true_type == nullptr) {
    return;
  }
  const auto* container = true_type->try_as<t_container>();
  const t_const* annotation =
      node.find_structured_annotation_or_null(kCppTypeUri);
  if (container == nullptr || annotation == nullptr ||
      annotation->value()->get_map().size() != 1) {
    return;
  }

  const auto* name =
      annotation->get_value_from_structured_annotation_or_null("name");
  if (name == nullptr) {
    return;
  }
  auto parsed = parse_template_instantiation(name->get_string());
  if (!parsed) {
    return;
  }

  const auto expected = generated_arguments(*container, resolver);
  if (parsed->arguments.size() != expected.size()) {
    return;
  }
  for (size_t i = 0; i < expected.size(); ++i) {
    if (normalize_cpp_type(parsed->arguments[i]) !=
        normalize_cpp_type(expected[i])) {
      return;
    }
  }

  const source_range range = annotation->src_range();
  const size_t begin = file_manager.to_offset(range.begin);
  const size_t end = file_manager.to_offset(range.end);
  std::string replacement(
      file_manager.old_content().substr(begin, end - begin));

  size_t key_pos = 0;
  while ((key_pos = replacement.find("name", key_pos)) != std::string::npos) {
    const bool starts_identifier = key_pos > 0 &&
        (std::isalnum(static_cast<unsigned char>(replacement[key_pos - 1])) ||
         replacement[key_pos - 1] == '_');
    const size_t key_end = key_pos + std::string_view("name").size();
    const bool ends_identifier = key_end < replacement.size() &&
        is_identifier_character(replacement[key_end]);
    size_t equals = key_end;
    while (equals < replacement.size() &&
           std::isspace(static_cast<unsigned char>(replacement[equals]))) {
      ++equals;
    }
    if (!starts_identifier && !ends_identifier && equals < replacement.size() &&
        replacement[equals] == '=') {
      const size_t quote = replacement.find_first_of("\"'", equals + 1);
      if (quote == std::string::npos) {
        return;
      }
      const auto quote_end = find_closing_quote(replacement, quote);
      if (!quote_end) {
        return;
      }
      replacement.replace(quote + 1, *quote_end - quote - 1, parsed->name);
      replacement.replace(key_pos, std::string_view("name").size(), "template");
      file_manager.add({begin, end, std::move(replacement)});
      return;
    }
    key_pos += std::string_view("name").size();
  }
}

} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& source_manager, t_program_bundle& bundle) {
        t_program& program = *bundle.root_program();
        codemod::file_manager file_manager(source_manager, program);
        cpp_name_resolver resolver;

        const_ast_visitor visitor;
        visitor.add_field_visitor([&](const t_field& field) {
          migrate_cpp_type(file_manager, resolver, field, *field.type());
        });
        visitor.add_typedef_visitor([&](const t_typedef& type) {
          migrate_cpp_type(file_manager, resolver, type, *type.type());
        });
        visitor.add_container_visitor([&](const t_container& container) {
          migrate_cpp_type(file_manager, resolver, container, container);
        });
        visitor(program);

        file_manager.apply_replacements();
      });
}
