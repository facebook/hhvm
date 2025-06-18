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

#include <thrift/compiler/ast/t_struct.h>

namespace apache::thrift::compiler {

/**
 * Represents a union definition.
 */
class t_union final : public t_structured {
 public:
  using t_structured::t_structured;

  t_union(const t_program* program, std::string name)
      : t_structured(program, std::move(name)) {}

  ~t_union() override;
  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  bool is_union() const override { return true; }

  // TODO(T219861020): remove this is_struct override
  bool is_struct_or_union() const override { return true; }
};

} // namespace apache::thrift::compiler
