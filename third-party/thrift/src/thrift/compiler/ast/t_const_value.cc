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

#include "thrift/compiler/ast/t_const_value.h"

namespace apache::thrift::compiler {

const std::vector<t_const_value*>& t_const_value::get_list_or_empty_map()
    const {
  if (kind_ == CV_LIST) {
    return std::get<list_value>(value_).raw;
  }
  static const std::vector<t_const_value*> empty;
  return empty;
}

t_type::value_type from_const_value_type(
    t_const_value::t_const_value_kind kind) {
  switch (kind) {
    case t_const_value::t_const_value_kind::CV_BOOL:
      return t_type::value_type::BOOL;
    case t_const_value::t_const_value_kind::CV_INTEGER:
      return t_type::value_type::I64;
    case t_const_value::t_const_value_kind::CV_DOUBLE:
      return t_type::value_type::DOUBLE;
    case t_const_value::t_const_value_kind::CV_STRING:
      return t_type::value_type::STRING;
    case t_const_value::t_const_value_kind::CV_MAP:
      return t_type::value_type::MAP;
    case t_const_value::t_const_value_kind::CV_LIST:
      return t_type::value_type::LIST;
    case t_const_value::t_const_value_kind::CV_IDENTIFIER:
      return t_type::value_type::STRING;
  }
}

} // namespace apache::thrift::compiler
