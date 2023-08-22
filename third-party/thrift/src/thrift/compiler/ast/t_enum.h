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

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * Represents an enum definition.
 */
class t_enum : public t_type {
 public:
  // An ~arbitrary, unlikely yet small number.
  static constexpr int32_t default_unused = 113;

  t_enum(t_program* program, std::string name)
      : t_type(program, std::move(name)) {}

  void set_values(t_enum_value_list values);
  void append_value(std::unique_ptr<t_enum_value> enum_value);
  node_list_view<t_enum_value> values() { return values_; }
  node_list_view<const t_enum_value> values() const { return values_; }

  // A value that does not currently have an associated name.
  int32_t unused() const { return unused_; }

  // Returns the enum_value with the given value, or nullptr.
  const t_enum_value* find_value(int32_t enum_value) const;

  // The t_consts associated with each value.
  node_list_view<const t_const> consts() const { return constants_; }

 private:
  t_enum_value_list values_;
  node_list<t_const> constants_;
  std::map<int32_t, const t_enum_value*> value_map_;
  int32_t unused_ = default_unused;

  // TODO(afuller): These methods are only provided for backwards
  // compatibility. Update all references and remove everything below.
  std::vector<t_enum_value*> values_raw_;

  void update_unused(int32_t val);

 public:
  void append(
      std::unique_ptr<t_enum_value> enum_value,
      std::unique_ptr<t_const> constant) {
    update_unused(enum_value->get_value());
    values_raw_.push_back(enum_value.get());
    value_map_.emplace(enum_value->get_value(), enum_value.get());
    values_.push_back(std::move(enum_value));
    constants_.push_back(std::move(constant));
  }
  void append(std::unique_ptr<t_enum_value> enum_value) {
    append_value(std::move(enum_value));
  }

  const std::vector<t_enum_value*>& get_enum_values() const {
    return values_raw_;
  }

  bool is_enum() const override { return true; }
  type get_type_value() const override { return type::t_enum; }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
