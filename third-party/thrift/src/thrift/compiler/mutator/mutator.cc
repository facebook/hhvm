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

#include <thrift/compiler/mutator/mutator.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

class mutator_list {
 public:
  mutator_list() = default;

  std::vector<visitor*> get_pointers() const {
    auto pointers = std::vector<visitor*>{};
    for (const auto& v : mutators_) {
      pointers.push_back(v.get());
    }
    return pointers;
  }

  template <typename T, typename... Args>
  void add(Args&&... args) {
    auto ptr = make_mutator<T>(std::forward<Args>(args)...);
    mutators_.push_back(std::move(ptr));
  }

 private:
  std::vector<std::unique_ptr<mutator>> mutators_;
};

} // namespace

static void fill_mutators(mutator_list& ms);

void mutator::mutate(t_program* const program) {
  auto mutators = mutator_list();
  fill_mutators(mutators);
  interleaved_visitor(mutators.get_pointers()).traverse(program);
}

/**
 * fill_mutators - the validator registry
 *
 * This is where all concrete validator types must be registered.
 */

static void fill_mutators(mutator_list& ms) {
  ms.add<field_type_to_const_value>();
  ms.add<const_type_to_const_value>();
  ms.add<structured_annotation_type_to_const_value>();

  // add more mutators here ...
}

static const t_type* resolve_type(const t_type* type) {
  while (type->is_typedef()) {
    type = dynamic_cast<const t_typedef*>(type)->get_type();
  }
  return type;
}

static void match_type_with_const_value(
    t_node* node,
    t_program* program,
    const t_type* long_type,
    t_const_value* value) {
  const t_type* type = resolve_type(long_type);
  value->set_ttype(t_type_ref::from_req_ptr(type));
  if (type->is_list()) {
    auto* elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
    for (auto list_val : value->get_list()) {
      match_type_with_const_value(node, program, elem_type, list_val);
    }
  }
  if (type->is_set()) {
    auto* elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
    for (auto set_val : value->get_list()) {
      match_type_with_const_value(node, program, elem_type, set_val);
    }
  }
  if (type->is_map()) {
    auto* key_type = dynamic_cast<const t_map*>(type)->get_key_type();
    auto* val_type = dynamic_cast<const t_map*>(type)->get_val_type();
    for (auto map_val : value->get_map()) {
      match_type_with_const_value(node, program, key_type, map_val.first);
      match_type_with_const_value(node, program, val_type, map_val.second);
    }
  }
  if (type->is_struct()) {
    auto* struct_type = dynamic_cast<const t_struct*>(type);
    for (auto map_val : value->get_map()) {
      auto name = map_val.first->get_string();
      auto tfield = struct_type->get_field_by_name(name);
      if (!tfield) {
        throw mutator_exception(
            node->src_range().begin,
            diagnostic_level::error,
            "field `" + name + "` does not exist.");
      }
      match_type_with_const_value(
          node, program, tfield->get_type(), map_val.second);
    }
  }
  // Set constant value types as enums when they are declared with integers
  if (type->is_enum() && !value->is_enum()) {
    value->set_is_enum();
    auto enm = dynamic_cast<const t_enum*>(type);
    value->set_enum(enm);
    if (value->get_type() == t_const_value::CV_STRING) {
      // The enum was defined after the struct field with that type was declared
      // so the field default value, if present, was treated as a string rather
      // than resolving to the enum constant in the parser.
      // So we have to resolve the string to the enum constant here instead.
      auto str = value->get_string();
      auto constant = program->scope()->find_constant(str);
      if (!constant) {
        auto full_str = program->name() + "." + str;
        constant = program->scope()->find_constant(full_str);
      }
      if (!constant) {
        throw std::runtime_error(
            std::string("type error: no matching constant: ") + str);
      }
      auto value_copy = constant->get_value()->clone();
      value->assign(std::move(*value_copy));
    }
    if (enm->find_value(value->get_integer())) {
      value->set_enum_value(enm->find_value(value->get_integer()));
    }
  }
  // Remove enum_value if type is a base_type to use the integer instead
  if (type->is_base_type() && value->is_enum()) {
    value->set_enum_value(nullptr);
  }
}

static void match_annotation_types_with_const_values(
    t_named* const tnamed, t_program* const program) {
  for (t_const& tconst : tnamed->structured_annotations()) {
    if (tconst.get_type() && tconst.get_value()) {
      match_type_with_const_value(
          &tconst, program, tconst.get_type(), tconst.get_value());
    }
  }
}

/**
 * field_type_to_const_value
 */
bool field_type_to_const_value::visit(t_program* const program) {
  program_ = program;
  return true;
}
bool field_type_to_const_value::visit(t_field* const tfield) {
  if (tfield->get_type() && tfield->get_value()) {
    match_type_with_const_value(
        tfield, program_, tfield->get_type(), tfield->get_value());
  }
  return true;
}

/**
 * const_type_to_const_value
 */
bool const_type_to_const_value::visit(t_program* const program) {
  program_ = program;
  return true;
}
bool const_type_to_const_value::visit(t_const* const tconst) {
  if (tconst->get_type() && tconst->get_value()) {
    match_type_with_const_value(
        tconst, program_, tconst->get_type(), tconst->get_value());
  }
  return true;
}

bool structured_annotation_type_to_const_value::visit(t_program* program) {
  program_ = program;
  return true;
}

bool structured_annotation_type_to_const_value::visit(t_struct* tstruct) {
  match_annotation_types_with_const_values(tstruct, program_);
  return true;
}

bool structured_annotation_type_to_const_value::visit(t_field* t_field) {
  match_annotation_types_with_const_values(t_field, program_);
  return true;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
