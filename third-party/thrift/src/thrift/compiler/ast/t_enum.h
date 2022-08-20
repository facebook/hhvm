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
 * An enumerated type. A list of constant objects with a name for the type.
 */
class t_enum : public t_type {
 public:
  t_enum(t_program* program, std::string name)
      : t_type(program, std::move(name)) {}

  void set_values(t_enum_value_list values);
  void append_value(std::unique_ptr<t_enum_value> enum_value);
  node_list_view<t_enum_value> values() { return values_; }
  node_list_view<const t_enum_value> values() const { return values_; }

  // Returns the enum_value with the given value, or nullptr.
  const t_enum_value* find_value(int32_t enum_value) const;

  // The t_consts associated with each value.
  node_list_view<const t_const> consts() const { return constants_; }

  std::string get_full_name() const override { return make_full_name("enum"); }

 private:
  t_enum_value_list values_;
  node_list<t_const> constants_;

  // TODO(afuller): These methods are only provided for backwards
  // compatibility. Update all references and remove everything below.
  std::vector<t_enum_value*> values_raw_;

 public:
  void append(
      std::unique_ptr<t_enum_value> enum_value,
      std::unique_ptr<t_const> constant) {
    values_raw_.push_back(enum_value.get());
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
