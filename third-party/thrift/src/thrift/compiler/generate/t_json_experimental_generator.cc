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

#include <filesystem>

#include <thrift/compiler/generate/json.h>
#include <thrift/compiler/generate/t_whisker_generator.h>

namespace apache::thrift::compiler {
namespace {

std::string get_filepath(
    const t_node& node, source_manager& sm, const std::string& compiler_path) {
  std::string path = sm.resolve_location(node.src_range().begin).file_name();
  if (auto full_path = sm.found_include_file(path)) {
    path = std::move(*full_path);
  }
  return std::filesystem::relative(
             std::filesystem::canonical(std::filesystem::path(path)),
             compiler_path)
      .generic_string();
}

class t_json_experimental_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  std::string template_prefix() const override { return "json"; }

  void generate_program() override;

 private:
  whisker::map::raw globals(whisker::prototype_database& proto) const override {
    whisker::map::raw globals = t_whisker_generator::globals(proto);
    // By default, Whisker considers f64s to be unprintable in strict mode, as
    // floats can have non-deterministic string results (e.g. fmt vs
    // std::ostream). For this reason, each generator should explicitly define
    // how its floats should be rendered.
    globals["float_to_string"] = whisker::dsl::make_function(
        "float_to_string",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(
              fmt::format("{}", ctx.argument<whisker::f64>(0)));
        });
    return globals;
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    def.property("bool_integer_value", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_BOOL
          ? whisker::make::i64(self.get_bool() ? 1 : 0)
          : whisker::make::null;
    });
    def.property("string_value_any", [](const t_const_value& self) {
      return to_json(&self);
    });

    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);
    def.property("docstring", [](const t_named& self) {
      return json_quote_ascii(self.doc());
    });
    return std::move(def).make();
  }

  prototype<t_node>::ptr make_prototype_for_node(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_node(proto);
    auto def = whisker::dsl::prototype_builder<h_node>::extends(base);
    def.property("path", [this](const t_node& node) {
      return get_filepath(
          node, source_mgr_, std::filesystem::current_path().generic_string());
    });
    return std::move(def).make();
  }

  prototype<t_package>::ptr make_prototype_for_package(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_package(proto);
    auto def = whisker::dsl::prototype_builder<h_package>::extends(base);

    def.property("domain?", [](const t_package& self) {
      return self.domain().size() > 1;
    });
    def.property("domain_prefix", [](const t_package& self) {
      return self.domain().size() > 1
          ? whisker::make::string(
                fmt::format(
                    "{}",
                    // Take everything but the last element (domain_suffix)
                    // E.g. "foo.bar.baz" -> "foo.bar"
                    fmt::join(
                        self.domain().begin(),
                        std::prev(self.domain().end()),
                        ".")))
          : whisker::make::null;
    });
    def.property("domain_suffix", [](const t_package& self) {
      // Last element of the domain. E.g. "foo.bar.baz" -> "baz"
      return self.domain().size() > 1
          ? whisker::make::string(self.domain().back())
          : whisker::make::null;
    });
    def.property("path", [](const t_package& self) {
      return self.path().empty()
          ? whisker::make::null
          : whisker::make::string(
                fmt::format("{}", fmt::join(self.path(), "/")));
    });

    return std::move(def).make();
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);

    def.property("normalized_include_prefix", [this](const t_program& self) {
      const std::string& prefix = self.include_prefix();
      return !prefix.empty() &&
              std::filesystem::path(prefix).has_root_directory()
          ? whisker::string(get_compiler_option("include_prefix").value_or(""))
          : prefix;
    });

    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);
    def.property("external?", [this](const t_type& self) {
      return self.program() != get_program();
    });
    return std::move(def).make();
  }
};

void t_json_experimental_generator::generate_program() {
  out_dir_base_ = "gen-json_experimental";
  whisker::object context = whisker::make::map({
      {"program",
       whisker::make::native_handle(
           render_state().prototypes->create<t_program>(*program_))},
  });
  render_to_file(/*output_file=*/fmt::format("{}.json", program_->name()),
                 /*template_file=*/"thrift_ast",
                 /*context=*/context);
}

THRIFT_REGISTER_GENERATOR(json_experimental, "JSON_EXPERIMENTAL", "");

} // namespace
} // namespace apache::thrift::compiler
