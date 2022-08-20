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
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/compiler.h>
#include <thrift/compiler/lib/cpp2/util.h>

using namespace apache::thrift::compiler;

static bool only_annotations(
    const t_node& node, std::vector<std::string> annotations) {
  if (node.annotations().size() != annotations.size()) {
    return false;
  }

  std::vector<std::string> node_annotations;

  for (const auto& annotation : node.annotations()) {
    node_annotations.push_back(annotation.first);
  }

  std::sort(annotations.begin(), annotations.end());

  return std::equal(
      annotations.begin(), annotations.end(), node_annotations.begin());
}

// Migrates cpp.ref and cpp2.ref unstructured annotations to its
// newly implemented structured versions.
// NOTE: Rely on automated formatting to fix formatting issues.
static void cppref_to_structured(
    codemod::file_manager& fm, const t_field& field) {
  if (!field.annotations().count("cpp.ref") &&
      !field.annotations().count("cpp2.ref")) {
    return;
  }

  fm.add_include("thrift/annotation/cpp.thrift");

  const auto field_begin_offset = fm.to_offset(field.src_range().begin);

  fm.add(
      {field_begin_offset,
       field_begin_offset,
       "@cpp.Ref{type = cpp.RefType.Unique}\n"});

  if (only_annotations(field, {"cpp.ref", "cpp2.ref"})) {
    fm.remove_all_annotations(field);
    return;
  }

  for (const auto& annotation : field.annotations()) {
    if (annotation.first == "cpp.ref" || annotation.first == "cpp2.ref") {
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

  /* Currently other languages except C++ don't support `@cpp.Ref` */
  for (const auto& n : program->namespaces()) {
    if (n.first != "cpp" && n.first != "cpp2") {
      return 0;
    }
  }

  codemod::file_manager fm(source_mgr, *program);

  const_ast_visitor visitor;
  visitor.add_field_visitor(folly::partial(cppref_to_structured, std::ref(fm)));
  visitor(*program);

  fm.apply_replacements();

  return 0;
}
