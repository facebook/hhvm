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

#include <thrift/compiler/ast/t_struct.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_program;

/**
 * A struct is a container for a set of member fields that has a name. Structs
 * are also used to implement exception and union types.
 *
 */
class t_result_struct : public t_struct {
 public:
  std::string result_return_type;

  t_result_struct(
      t_program* program, std::string name, std::string result_return_type)
      : t_struct(program, name), result_return_type{result_return_type} {}

  std::string getResultReturnType() const { return result_return_type; }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
