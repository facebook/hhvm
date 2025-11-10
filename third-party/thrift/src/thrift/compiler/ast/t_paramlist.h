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

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_structured.h>

namespace apache::thrift::compiler {

class t_program;

class t_paramlist final : public t_structured {
 public:
  // Param lists are unnamed.
  // TODO(afuller): The program should always be null, because this is an
  // 'unnamed' t_type (or more accurately, not a type at all).
  explicit t_paramlist(const t_program* program) : t_structured(program, "") {}
};

} // namespace apache::thrift::compiler
