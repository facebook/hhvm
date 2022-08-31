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

#include <memory>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_structured.h>

namespace apache {
namespace thrift {
namespace compiler {
class schematizer {
 public:
  // Creates a constant of type schema.Struct describing the argument.
  // https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/schema.thrift
  static std::unique_ptr<t_const_value> gen_schema(const t_structured& node);
};
} // namespace compiler
} // namespace thrift
} // namespace apache
