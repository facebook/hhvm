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

#include <string>

#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/generate/csharp/util.h>
#include <thrift/compiler/generate/t_whisker_generator.h>
#include <thrift/compiler/generate/templates.h>

namespace apache::thrift::compiler {

namespace {

namespace w = whisker::make;

// Oncall: content_dre
class t_csharp_EXPERIMENTAL_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  std::string template_prefix() const override { return "csharp"; }

  whisker::source_manager template_source_manager() const final {
    return whisker::source_manager{
        std::make_unique<in_memory_source_manager_backend>(
            create_templates_by_path())};
  }

  void generate_program() override {
    out_dir_base_ = "gen-csharp";
    auto filename = std::string(get_program()->name()) + ".cs";
    render_to_file(
        /*output_file=*/filename,
        /*template_file=*/"module",
        /*context=*/w::map());
  }

 private:
  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);

    def.property("csharp_namespace", [this](const t_program& self) {
      return csharp::get_csharp_namespace(self, diags_);
    });

    return std::move(def).make();
  }

  prototype<t_include>::ptr make_prototype_for_include(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_include(proto);
    auto def = whisker::dsl::prototype_builder<h_include>::extends(base);

    def.property("csharp_namespace", [this](const t_include& self) {
      return csharp::get_csharp_namespace(*self.get_program(), diags_);
    });

    def.property("alias", [](const t_include& self) {
      return std::string(self.get_program()->name());
    });

    def.property("is_useful_alias?", [this](const t_include& self) {
      std::string ns =
          csharp::get_csharp_namespace(*self.get_program(), diags_);
      std::string alias = std::string(self.get_program()->name());
      return ns != alias;
    });

    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);

    def.property("csharp_name", [](const t_named& self) {
      return "@" + std::string(self.name());
    });

    // Namespace-qualified name for cross-program type references.
    // Returns "Namespace.@TypeName" for external types, "@TypeName" for local.
    def.property("csharp_qualified_name", [this](const t_named& self) {
      if (self.program() != nullptr && self.program() != get_program()) {
        return csharp::get_csharp_namespace(*self.program(), diags_) + ".@" +
            std::string(self.name());
      }
      return "@" + std::string(self.name());
    });

    return std::move(def).make();
  }
};

} // namespace

THRIFT_REGISTER_GENERATOR(
    csharp_EXPERIMENTAL,
    "C#",
    "EXPERIMENTAL support for C# - use at your own risk. APIs can change without prior notice. All behavior should be considered undefined, unless explicitly specified otherwise.");

} // namespace apache::thrift::compiler
