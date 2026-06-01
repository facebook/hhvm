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

#include <thrift/compiler/generate/t_whisker_generator.h>
#include <thrift/compiler/generate/templates.h>

namespace apache::thrift::compiler {
namespace {

class t_typescript_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  std::string template_prefix() const override { return "typescript"; }

  whisker::source_manager template_source_manager() const final {
    return whisker::source_manager(
        std::make_unique<in_memory_source_manager_backend>(
            create_templates_by_path()));
  }

  void generate_program() override {
    auto filename = std::string(get_program()->name()) + ".ts";
    render_to_file(
        /*output_file=*/filename,
        /*template_file=*/"module",
        /*context=*/whisker::make::map());
  }
};

} // namespace

THRIFT_REGISTER_GENERATOR(
    typescript,
    "TypeScript",
    "Experimental TypeScript support for schematization primitives only - just structs and the like, **no RPC**");

} // namespace apache::thrift::compiler
