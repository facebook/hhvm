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

#include <thrift/compiler/ast/t_structured.h>

namespace apache::thrift::compiler {

class t_program;

/**
 * The exceptions thrown by a t_function, t_sink, etc.
 */
class t_throws : public t_structured {
 public:
  t_throws() : t_structured(nullptr, "") {}

  // A helper that returns true if value contains now exceptions, either because
  // it is not set, or because it is empty.
  static bool is_null_or_empty(const t_throws* value) {
    return value == nullptr || !value->has_fields();
  }
};

// Returns a view of all the elements in the throws clause if it is non-null
// or an empty view otherwise.
inline node_list_view<const t_field> get_elems(const t_throws* t) {
  return t ? t->fields() : node_list_view<const t_field>();
}

} // namespace apache::thrift::compiler
