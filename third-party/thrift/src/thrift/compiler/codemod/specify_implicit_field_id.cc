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
class specify_implicit_field_id {
 public:
  specify_implicit_field_id(source_manager& sm, t_program& program)
      : fm_(sm, program), sm_(sm), prog_(program) {}

  void run() {
    const_ast_visitor visitor;

    visitor.add_field_visitor([=](const auto& f) { visit_field(f); });

    visitor(prog_);

    fm_.apply_replacements();
  }

  void visit_field(const t_field& f) {
    if (f.explicit_id()) {
      return;
    }
    auto loc = f.type().src_range().begin.offset();
    if (f.qualifier() == t_field_qualifier::optional ||
        f.qualifier() == t_field_qualifier::required) {
      loc -= std::string_view("optional ").size();
    }
    fm_.add({loc, loc, fmt::format("{}\n    {}: ", kComment, f.id())});
  }

 private:
  codemod::file_manager fm_;
  source_manager sm_;
  t_program& prog_;

  constexpr static std::string_view kComment =
      "// @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.";
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
  specify_implicit_field_id(source_mgr, *program).run();

  return 0;
}
