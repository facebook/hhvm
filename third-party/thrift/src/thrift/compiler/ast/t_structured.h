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

#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {

// Forward declare that puppy
class t_program;

/**
 * A structured node contains a set of fields that each have a unique name and
 * id.
 */
class t_structured : public t_type {
 public:
  // Appends the given field, throwing an std::runtime_error on a
  // conflict with an existing field.
  void append_field(std::unique_ptr<t_field> field);

  // Tries to append the given field, moving the argument on success or
  // leaving it as is on failure.
  bool try_append_field(std::unique_ptr<t_field>& field);

  // Creates a new field using the given arguments, appends it, and returns a
  // mutable reference to the newly create (and internally owned) field
  // instance, if successful.
  template <typename... Args>
  t_field& create_field(Args&&... args) {
    auto* ptr = new t_field(std::forward<Args>(args)...);
    append_field(std::unique_ptr<t_field>(ptr));
    return *ptr;
  }

  node_list_view<t_field> fields() { return fields_; }
  node_list_view<const t_field> fields() const { return fields_; }
  bool has_fields() const { return !fields_.empty(); }

  // Get the fields, ordered by id.
  const std::vector<const t_field*>& fields_id_order() const {
    return fields_id_order_;
  }

  // Access the field by id or name.
  const t_field* get_field_by_id(t_field_id id) const;
  const t_field* get_field_by_name(std::string_view name) const {
    auto it = fields_by_name_.find(name);
    return it != fields_by_name_.end() ? it->second : nullptr;
  }

  type get_type_value() const override;

  ~t_structured() override;

 protected:
  t_field_list fields_;
  std::vector<const t_field*> fields_id_order_;
  std::unordered_map<std::string_view, const t_field*> fields_by_name_;

  t_structured(const t_program* program, std::string name)
      : t_type(program, std::move(name)) {}
  explicit t_structured(const t_program* program = nullptr) : t_type(program) {}

  ////
  // TODO(T227540797)
  // Everyting below here is for backwards compatiblity, and will be removed.
  ////
 public:
  // Tries to append the gieven field, throwing an exception on failure.
  void append(std::unique_ptr<t_field> elem);

  const t_field* get_field_named(std::string_view name) const {
    const auto* result = get_field_by_name(name);
    assert(result != nullptr);
    return result;
  }

  t_field* get_field(size_t index) { return fields_.at(index).get(); }
  const t_field* get_field(size_t index) const {
    return fields_.at(index).get();
  }

  const std::vector<t_field*>& get_members() const { return fields_raw_; }

  const std::vector<t_field*>& get_sorted_members() const {
    return fields_raw_id_order_;
  }

  const t_field* get_member(std::string_view name) const {
    return get_field_by_name(name);
  }

  bool has_field_named(std::string_view name) const {
    return get_field_by_name(name) != nullptr;
  }

  bool validate_field(t_field* field) {
    return get_field_by_id(field->id()) == nullptr;
  }

 protected:
  std::vector<t_field*> fields_raw_;
  std::vector<t_field*> fields_raw_id_order_;
};

} // namespace apache::thrift::compiler
