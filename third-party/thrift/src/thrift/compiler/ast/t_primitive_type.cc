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

#include "thrift/compiler/ast/t_primitive_type.h"

namespace apache::thrift::compiler {

const t_primitive_type& t_primitive_type::t_void() {
  static t_primitive_type type{"void", type::t_void};
  return type;
}

const t_primitive_type& t_primitive_type::t_string() {
  static t_primitive_type type{"string", type::t_string};
  return type;
}

const t_primitive_type& t_primitive_type::t_binary() {
  static t_primitive_type type{"binary", type::t_binary};
  return type;
}

const t_primitive_type& t_primitive_type::t_bool() {
  static t_primitive_type type{"bool", type::t_bool};
  return type;
}

const t_primitive_type& t_primitive_type::t_byte() {
  static t_primitive_type type{"byte", type::t_byte};
  return type;
}

const t_primitive_type& t_primitive_type::t_i16() {
  static t_primitive_type type{"i16", type::t_i16};
  return type;
}

const t_primitive_type& t_primitive_type::t_i32() {
  static t_primitive_type type{"i32", type::t_i32};
  return type;
}

const t_primitive_type& t_primitive_type::t_i64() {
  static t_primitive_type type{"i64", type::t_i64};
  return type;
}

const t_primitive_type& t_primitive_type::t_double() {
  static t_primitive_type type{"double", type::t_double};
  return type;
}

const t_primitive_type& t_primitive_type::t_float() {
  static t_primitive_type type{"float", type::t_float};
  return type;
}

bool t_primitive_type::is_sealed() const {
  return true;
}

t_primitive_type::~t_primitive_type() = default;

} // namespace apache::thrift::compiler
