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

#include <algorithm>
#include <map>
#include <string>

#include <fmt/core.h>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/compiler.h>
#include <thrift/compiler/lib/cpp2/util.h>

using namespace apache::thrift::compiler;

namespace {
class structure_annotations {
 public:
  structure_annotations(source_manager& sm, t_program& program)
      : fm_(sm, program), sm_(sm), prog_(program) {}

  void visit_field_or_typedef(
      const t_named& node, t_type_ref type, bool is_field) {
    std::set<std::string> to_add;
    // Annotations on type
    {
      std::vector<t_annotation> to_remove;
      for (const auto& [name, data] : type->annotations()) {
        bool has_cpp_type = false;
        if (name == "cpp.template" || name == "cpp2.template") {
          to_remove.emplace_back(name, data);
          if (!std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{template = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        }
        if (name == "cpp.type" || name == "cpp2.type") {
          to_remove.emplace_back(name, data);
          if (!std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{name = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        }
      }

      if (!to_remove.empty() &&
          to_remove.size() == type->annotations().size()) {
        fm_.remove_all_annotations(*type);
      } else {
        for (const auto& annot : to_remove) {
          fm_.remove(annot);
        }
      }
    }

    // Annotations on node
    {
      std::vector<t_annotation> to_remove;
      for (const auto& [name, data] : node.annotations()) {
        bool has_cpp_type = false;
        if (name == "cpp.template" || name == "cpp2.template") {
          to_remove.emplace_back(name, data);
          if (!is_field && !std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{template = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        }
        if (name == "cpp.type" || name == "cpp2.type") {
          to_remove.emplace_back(name, data);
          if (!is_field && !std::exchange(has_cpp_type, true)) {
            to_add.insert(
                fmt::format("@cpp.Type{{name = \"{}\"}}", data.value));
            fm_.add_include("thrift/annotation/cpp.thrift");
          }
        }
      }

      if (!to_remove.empty() && to_remove.size() == node.annotations().size()) {
        fm_.remove_all_annotations(node);
      } else {
        for (const auto& annot : to_remove) {
          fm_.remove(annot);
        }
      }
    }

    if (!to_add.empty()) {
      fm_.add(
          {node.src_range().begin.offset(),
           node.src_range().begin.offset(),
           fmt::format("{}\n", fmt::join(to_add, "\n"))});
    }
  }

  void run() {
    const_ast_visitor visitor;
    visitor.add_typedef_visitor([=](const auto& node) {
      visit_field_or_typedef(node, node.type(), false);
    });
    visitor.add_field_visitor([=](const auto& node) {
      visit_field_or_typedef(node, node.type(), true);
    });

    visitor(prog_);

    fm_.apply_replacements();
  }

 private:
  codemod::file_manager fm_;
  source_manager sm_;
  t_program& prog_;
};
} // namespace

int main(int argc, char** argv) {
  auto source_mgr = source_manager();
  auto program_bundle = parse_and_get_program(
      source_mgr, std::vector<std::string>(argv, argv + argc));
  if (!program_bundle) {
    return 1;
  }
  auto program = program_bundle->root_program();
  structure_annotations(source_mgr, *program).run();

  return 0;
}
