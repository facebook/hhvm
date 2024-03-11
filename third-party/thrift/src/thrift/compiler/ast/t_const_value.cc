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

#include <type_traits>

namespace apache::thrift::compiler {

const std::vector<t_const_value*>& t_const_value::get_list_or_empty_map()
    const {
  if (kind_ == CV_LIST) {
    return std::get<list_value>(value_).raw;
  }
  static const std::vector<t_const_value*> empty;
  return empty;
}

} // namespace apache::thrift::compiler
