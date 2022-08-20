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

#include <string>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_node.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace gen {
namespace cpp {

enum class reference_type {
  none = 0, // Not a reference.
  unique,
  shared_const,
  shared_mutable,
  boxed,
};

reference_type find_ref_type(const t_field& node);

} // namespace cpp
} // namespace gen
} // namespace compiler
} // namespace thrift
} // namespace apache
