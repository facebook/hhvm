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

#include <thrift/compiler/ast/t_interface.h>

#include <cassert>
#include <stdexcept>
#include <fmt/format.h>
#include <thrift/compiler/ast/t_interaction.h>

namespace apache::thrift::compiler {

void t_interface::set_functions(node_list<t_function> functions) {
  functions_ = std::move(functions);
#ifndef NDEBUG
  // Assert is no-op when NDEBUG is defined. Check pointers in debug mode, avoid
  // unused variable warning-as-error in release mode
  for (const auto& func : functions_) {
    assert(func != nullptr);
  }
#endif
}

void t_interface::add_function(std::unique_ptr<t_function> func) {
  assert(func != nullptr);
  functions_.push_back(std::move(func));
}

bool t_interface::is_sealed() const {
  throw std::logic_error(
      fmt::format(
          "is_sealed() called on \"interface\" `{}`. This is non-sensical: "
          "the concept of sealedness only applies to actual types, not "
          "services or interactions.",
          get_scoped_name()));
}

t_interface::~t_interface() = default;

} // namespace apache::thrift::compiler
