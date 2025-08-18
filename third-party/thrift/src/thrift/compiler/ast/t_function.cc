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

#include <thrift/compiler/ast/t_function.h>

#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_primitive_type.h>

namespace apache::thrift::compiler {

t_function::t_function(
    const t_program* program,
    t_type_ref return_type,
    std::string name,
    std::unique_ptr<t_paramlist> params,
    std::unique_ptr<t_sink> sink,
    std::unique_ptr<t_stream> stream,
    t_type_ref interaction)
    : t_named(program, std::move(name)),
      params_(std::move(params)),
      sink_(std::move(sink)),
      stream_(std::move(stream)),
      interaction_(interaction) {
  if (return_type) {
    return_type_ = return_type;
  } else {
    return_type_ = t_type_ref::from_ptr(&t_primitive_type::t_void());
  }
  if (!params_) {
    params_ = std::make_unique<t_paramlist>(program);
  }
}

t_function::~t_function() = default;

} // namespace apache::thrift::compiler
