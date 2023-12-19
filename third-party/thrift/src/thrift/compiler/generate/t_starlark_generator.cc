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

#include <utility>

#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

class t_starlark_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "starlark"; }

  void generate_program() override;
};

void t_starlark_generator::generate_program() {
  out_dir_base_ = "gen-star";
  const auto* program = get_program();
  auto mstch_program = mstch_context_.program_factory->make_mstch_object(
      program, mstch_context_);
  render_to_file(
      std::move(mstch_program), "enums.star", program->name() + ".star");
}

THRIFT_REGISTER_GENERATOR(starlark, "Starlark", "Starlark generator");

} // namespace
} // namespace compiler
} // namespace thrift
} // namespace apache
