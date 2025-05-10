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

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/generate/cpp/util.h>

namespace apache::thrift::compiler {
namespace {

// whether type is/has set/map with custom types (cpp.Type or cpp.Adapter)
// Nested fields don't count.
bool has_custom_set_or_map(
    const t_type& type, std::unordered_set<const t_type*>& seen) {
  if (!seen.emplace(&type).second) {
    // We already saw the `type` previously. It can either be that
    //
    // 1. we checked this type and it has no custom set/map,
    // 2. or the structure has recurisve types and we haven't finished checking
    //    this type yet and meet it again.
    //
    // Either way we can assume `type` is not a custom set or map since it will
    // be checked elsewhere.
    return false;
  }
  if (cpp2::is_custom_type(type) &&
      (type.get_true_type()->is_set() || type.get_true_type()->is_map())) {
    // Example:
    // @cpp.Type{template = "std::unordered_map"}
    // 1: map<i32, i32> foo;
    return true;
  }
  if (auto next = dynamic_cast<t_typedef const*>(&type)) {
    // Examples:
    // @cpp.Type{template = "std::unordered_set"}
    // typedef set<i32> CustomSet1;
    //
    // @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
    // typedef set<i32> CustomSet3;
    return has_custom_set_or_map(*next->get_type(), seen);
  }
  if (auto list = dynamic_cast<t_list const*>(&type)) {
    // Examples:
    // 1: list<CustomSet1> foo;
    // In this case the structure is still not orderable.
    return has_custom_set_or_map(*list->get_elem_type(), seen);
  }
  if (auto set = dynamic_cast<t_set const*>(&type)) {
    // Examples:
    // 1: set<CustomSet1> foo;
    return has_custom_set_or_map(*set->get_elem_type(), seen);
  }
  if (auto map = dynamic_cast<t_map const*>(&type)) {
    // Examples:
    // 1: map<i32, CustomSet1> foo;
    return has_custom_set_or_map(*map->get_key_type(), seen) ||
        has_custom_set_or_map(*map->get_val_type(), seen);
  }
  return false;
}

bool structure_has_custom_set_or_map_field(const t_structured& s) {
  std::unordered_set<const t_type*> seen;
  for (const t_field& i : s.fields()) {
    if (has_custom_set_or_map(i.type().deref(), seen)) {
      return true;
    }
  }
  return false;
}

void codemod_main(source_manager& sm, t_program_bundle& bundle) {
  const t_program& program = *bundle.get_root_program();
  codemod::file_manager fm(sm, program);
  const_ast_visitor visitor;
  visitor.add_structured_definition_visitor([&](const t_structured& s) {
    if (!cpp2::is_orderable(
            s, true /* enableCustomTypeOrderingIfStructureHasUri */)) {
      // Even if custom type ordering was enabled, `s` would not be orderable.

      // NOTE: for bonus points, we could check if `s` is annotated with
      // @cpp.EnableCustomTypeOrdering annotation, which would not make sense
      // and could be removed.
      return;
    }

    // If this point is reached: `s` can be made orderable (potentially by
    // enabling custom type ordering)

    if (s.uri().empty() ||
        s.find_structured_annotation_or_null(kCppEnableCustomTypeOrdering)) {
      // `s` does not have an URI, or is already annotated => this codemod does
      // not apply
      return;
    }

    if (!structure_has_custom_set_or_map_field(s)) {
      // `s` does not have custom set or maps, so the logic to explicitly enable
      // custom type ordering is not needed. The structure is effectively
      // already orderable without any additional step => nothing to do in this
      // codemod.
      return;
    }

    // If this point is reached: `s` is orderable thanks to custom type ordering
    // support, which is implicitly provided because it has a URI => this
    // codemod applies, and should add the annotation to make this logic
    // explicit rather than implicit.

    fm.add_include("thrift/annotation/cpp.thrift");
    size_t offset = fm.to_offset(s.src_range().begin);
    fm.add({offset, offset, "@cpp.EnableCustomTypeOrdering\n"});
  });
  visitor(program);
  fm.apply_replacements();
}
} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return run_codemod(
      argc,
      argv,
      {.skip_lowering_annotations = false},
      apache::thrift::compiler::codemod_main);
}
