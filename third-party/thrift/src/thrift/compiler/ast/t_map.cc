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

#include <thrift/compiler/ast/t_map.h>

namespace apache::thrift::compiler {

bool t_map::is_sealed() const {
  // NOTE: According to the Thrift Object Model, map keys must always be sealed,
  // but this is not enforced at the IDL level (yet), so it must be checked
  // here too.
  return key_type_->is_sealed() &&
      val_type_->is_sealed(); // Throws if unresolved
}

t_map::~t_map() = default;

} // namespace apache::thrift::compiler
