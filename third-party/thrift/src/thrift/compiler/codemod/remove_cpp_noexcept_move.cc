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

#include <folly/functional/Partial.h>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/compiler.h>

using namespace apache::thrift::compiler;

// Removes cpp.noexcept_move annotation.
static void remove_cpp_noexcept_move(
    codemod::file_manager& fm, const t_structured& node) {
  for (const auto& annotation : node.annotations()) {
    if (annotation.first == "cpp.noexcept_move") {
      fm.remove(annotation);
    }
  }
}

int main(int argc, char** argv) {
  auto source_mgr = source_manager();
  auto program_bundle = parse_and_get_program(
      source_mgr, std::vector<std::string>(argv, argv + argc));

  if (!program_bundle) {
    return 0;
  }

  auto program = program_bundle->root_program();

  codemod::file_manager fm(source_mgr, *program);

  const_ast_visitor visitor;
  visitor.add_struct_visitor(
      folly::partial(remove_cpp_noexcept_move, std::ref(fm)));
  visitor.add_union_visitor(
      folly::partial(remove_cpp_noexcept_move, std::ref(fm)));
  visitor(*program);

  fm.apply_replacements();

  return 0;
}
