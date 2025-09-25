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
#include <memory>

#include <thrift/compiler/generate/json.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/whisker/mstch_compat.h>

namespace apache::thrift::compiler {
namespace {

std::string get_filepath(
    const t_node& node, source_manager& sm, std::string compiler_path) {
  std::string path = sm.resolve_location(node.src_range().begin).file_name();
  if (auto full_path = sm.found_include_file(path)) {
    path = std::move(*full_path);
  }
  return std::filesystem::relative(
             std::filesystem::canonical(std::filesystem::path(path)),
             compiler_path)
      .generic_string();
}

class t_json_experimental_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "json"; }

  void generate_program() override;

 private:
  void set_mstch_factories();

  whisker::map::raw globals() const override {
    whisker::map::raw globals = t_mstch_generator::globals();
    // To allow rendering a brace not surrounded by whitespace, without
    // interfering with the `{{` `}}` used by the mustache syntax.
    globals["open_object"] = whisker::make::string("{");
    globals["close_object"] = whisker::make::string("}");
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

class json_experimental_program : public mstch_program {
 public:
  json_experimental_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:py_namespace",
             &json_experimental_program::get_py_namespace},
            {"program:includes?", &json_experimental_program::has_includes},
            {"program:includes", &json_experimental_program::includes},
            {"program:namespaces?", &json_experimental_program::has_namespaces},
            {"program:namespaces", &json_experimental_program::namespaces},
            {"program:package?", &json_experimental_program::has_package},
            {"program:package", &json_experimental_program::package},
            {"program:normalized_include_prefix",
             &json_experimental_program::include_prefix},
        });
  }

  mstch::node get_py_namespace() { return program_->get_namespace("py"); }
  mstch::node has_includes() { return !program_->includes().empty(); }
  mstch::node includes() {
    mstch::array includes;
    auto last = program_->includes().size();
    for (auto program : program_->get_included_programs()) {
      includes.emplace_back(mstch::map{
          {"name", program->name()},
          {"path", program->include_prefix() + program->name() + ".thrift"},
          {"last?", (--last) == 0}});
    }
    return includes;
  }
  mstch::node has_namespaces() { return !program_->namespaces().empty(); }
  mstch::node namespaces() {
    mstch::array result;
    auto last = program_->namespaces().size();
    for (auto it : program_->namespaces()) {
      result.emplace_back(mstch::map{
          {"key", it.first}, {"value", it.second}, {"last?", (--last) == 0}});
    }
    return result;
  }
  mstch::node has_package() { return !program_->package().empty(); }
  mstch::node package() {
    mstch::array result;
    auto& package = program_->package();
    auto domain = package.domain();
    if (!domain.empty() && domain.size() > 1) {
      auto domain_prefix = std::vector<std::string>();
      domain_prefix.insert(
          domain_prefix.end(), domain.begin(), std::prev(domain.end()));
      result.emplace_back(mstch::map{
          {"key", std::string("domain_prefix")},
          {"value", fmt::format("{}", fmt::join(domain_prefix, "."))},
          {"last?", false}});
      result.emplace_back(mstch::map{
          {"key", std::string("domain_suffix")},
          {"value", package.domain()[domain.size() - 1]},
          {"last?", false}});
    }
    if (!package.path().empty()) {
      result.emplace_back(mstch::map{
          {"key", std::string("path")},
          {"value", fmt::format("{}", fmt::join(package.path(), "/"))},
          {"last?", false}});
    }
    result.emplace_back(mstch::map{
        {"key", std::string("filename")},
        {"value", program_->name()},
        {"last?", true}});
    return result;
  }

  mstch::node include_prefix() {
    auto prefix = program_->include_prefix();
    if (prefix.empty()) {
      return std::string();
    }
    return std::filesystem::path(prefix).has_root_directory()
        ? context_.options["include_prefix"]
        : prefix;
  }
};

void t_json_experimental_generator::generate_program() {
  out_dir_base_ = "gen-json_experimental";
  const auto* program = get_program();
  set_mstch_factories();
  auto mstch_program = mstch_context_.program_factory->make_mstch_object(
      program, mstch_context_);
  render_to_file(mstch_program, "thrift_ast", program->name() + ".json");
}

void t_json_experimental_generator::set_mstch_factories() {
  mstch_context_.add<json_experimental_program>();
}

THRIFT_REGISTER_GENERATOR(json_experimental, "JSON_EXPERIMENTAL", "");

} // namespace
} // namespace apache::thrift::compiler
