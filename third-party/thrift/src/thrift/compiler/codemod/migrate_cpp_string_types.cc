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
#include <string>
#include <string_view>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::const_ast_visitor;
using apache::thrift::compiler::kCppAdapterUri;
using apache::thrift::compiler::kCppTypeUri;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::source_range;
using apache::thrift::compiler::t_const;
using apache::thrift::compiler::t_field;
using apache::thrift::compiler::t_named;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::t_type;
using apache::thrift::compiler::t_typedef;
namespace codemod = apache::thrift::compiler::codemod;

namespace {

bool is_iobuf_type(std::string_view name) {
  std::string normalized;
  normalized.reserve(name.size());
  for (char character : name) {
    if (!std::isspace(static_cast<unsigned char>(character))) {
      normalized.push_back(character);
    }
  }
  return normalized == "folly::IOBuf" || normalized == "::folly::IOBuf" ||
      normalized == "std::unique_ptr<folly::IOBuf>" ||
      normalized == "::std::unique_ptr<folly::IOBuf>" ||
      normalized == "std::unique_ptr<::folly::IOBuf>" ||
      normalized == "::std::unique_ptr<::folly::IOBuf>";
}

void migrate_cpp_type(
    codemod::file_manager& file_manager,
    const t_named& node,
    const t_type& type) {
  const t_type* true_type = type.get_true_type();
  if (true_type == nullptr || !true_type->is_string_or_binary()) {
    return;
  }

  const t_const* annotation =
      node.find_structured_annotation_or_null(kCppTypeUri);
  if (annotation == nullptr || annotation->value()->get_map().size() != 1 ||
      node.find_structured_annotation_or_null(kCppAdapterUri) != nullptr) {
    return;
  }
  const auto* name =
      annotation->get_value_from_structured_annotation_or_null("name");
  if (name == nullptr || is_iobuf_type(name->get_string()) ||
      name->get_string().find_first_of("\"\\") != std::string::npos) {
    return;
  }

  const source_range range = annotation->src_range();
  const std::string adapter =
      "@cpp.Adapter{name = \"::apache::thrift::StringTypeAdapter<" +
      name->get_string() + ">\"}";
  file_manager.add(
      {file_manager.to_offset(range.begin),
       file_manager.to_offset(range.end),
       adapter});
}

} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& source_manager, t_program_bundle& bundle) {
        t_program& program = *bundle.root_program();
        codemod::file_manager file_manager(source_manager, program);

        const_ast_visitor visitor;
        visitor.add_field_visitor([&](const t_field& field) {
          migrate_cpp_type(file_manager, field, *field.type());
        });
        visitor.add_typedef_visitor([&](const t_typedef& type) {
          migrate_cpp_type(file_manager, type, *type.type());
        });
        visitor(program);

        file_manager.apply_replacements();
      });
}
