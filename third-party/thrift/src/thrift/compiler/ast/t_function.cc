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

#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_interaction.h>

namespace apache {
namespace thrift {
namespace compiler {

t_function::t_function(
    t_program* program,
    t_type_ref return_type,
    std::string name,
    std::unique_ptr<t_paramlist> params,
    std::unique_ptr<t_node> sink_or_stream,
    t_type_ref interaction)
    : t_named(program, std::move(name)),
      sink_or_stream_(std::move(sink_or_stream)),
      interaction_(interaction),
      params_(std::move(params)) {
  if (return_type) {
    return_types_.push_back(return_type);
  }
  if (!params_) {
    params_ = std::make_unique<t_paramlist>(program);
  }
  assert(!sink_or_stream_ || sink() || stream());
}

const t_type* t_function::return_type() const {
  if (is_interaction_constructor_) {
    // The old syntax (performs) treats an interaction as a response.
    return interaction_.get_type();
  }
  return !return_types_.empty() && !sink_or_stream()
      ? return_types_.back().get_type()
      : &t_base_type::t_void();
}
} // namespace compiler
} // namespace thrift
} // namespace apache
