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
#include <string>
#include <vector>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_program;

/**
 * Represents an interface definition.
 *
 * An interface consists of a set of functions.
 */
class t_interface : public t_type {
 public:
  t_interface(t_program* program, std::string name)
      : t_type(program, std::move(name)) {}

  node_list_view<t_function> functions() { return functions_; }
  node_list_view<const t_function> functions() const { return functions_; }

  void set_functions(node_list<t_function> functions);
  void add_function(std::unique_ptr<t_function> func);

 private:
  node_list<t_function> functions_;

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
  std::vector<t_function*> old_functions_raw_;

 public:
  const std::vector<t_function*>& get_functions() const {
    return old_functions_raw_;
  }

  bool is_interaction() const;
  bool is_serial_interaction() const;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
