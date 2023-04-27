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

#include <functional>
#include <memory>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_typedef.h>

namespace apache {
namespace thrift {
namespace compiler {
class schematizer {
 public:
  using InternFunc = std::function<t_program::value_id(
      std::unique_ptr<t_const_value> val, t_type_ref type, t_program* program)>;

  explicit schematizer(InternFunc intern_value = &default_intern_value)
      : intern_value_(std::move(intern_value)) {}

  // Creates a constant of type schema.Struct describing the argument.
  // https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift
  std::unique_ptr<t_const_value> gen_schema(const t_structured& node);
  std::unique_ptr<t_const_value> gen_schema(const t_service& node);
  std::unique_ptr<t_const_value> gen_schema(const t_const& node);
  std::unique_ptr<t_const_value> gen_schema(const t_enum& node);
  std::unique_ptr<t_const_value> gen_schema(const t_program& node);
  std::unique_ptr<t_const_value> gen_schema(const t_typedef& node);

 private:
  std::function<t_program::value_id(
      std::unique_ptr<t_const_value> val, t_type_ref type, t_program* program)>
      intern_value_;

  static t_program::value_id default_intern_value(
      std::unique_ptr<t_const_value> val, t_type_ref type, t_program* program);
};

// Turns a t_const_value holding some value V into a t_const_value holding a
// protocol.Value representing V.
// Currently only uses bool/i64/double/string/list/map.
// TODO: allow increasing type fidelity.
std::unique_ptr<t_const_value> wrap_with_protocol_value(
    const t_const_value& value, t_type_ref ttype);
} // namespace compiler
} // namespace thrift
} // namespace apache
