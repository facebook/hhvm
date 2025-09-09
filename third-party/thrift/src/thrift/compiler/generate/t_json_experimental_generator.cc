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

struct json_codegen_data {
  // The current program being generated
  const t_program* current_program;
  std::string compiler_path;
  source_manager* sm;
};

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
  json_codegen_data data_;

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);
    def.property("docstring", [](const t_named& self) {
      return json_quote_ascii(self.doc());
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

    // To allow rendering a brace not surrounded by whitespace, without
    // interfering with the `{{` `}}` used by the mustache syntax.
    register_method("open_object", [] { return mstch::node("{"); });
    register_method("close_object", [] { return mstch::node("}"); });
  }

  mstch::node get_py_namespace() { return program_->get_namespace("py"); }
  mstch::node has_includes() { return !program_->includes().empty(); }
  mstch::node includes() {
    mstch::array includes;
    auto last = program_->includes().size();
    for (auto program : program_->get_included_programs()) {
      includes.push_back(mstch::map{
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
      result.push_back(mstch::map{
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
      result.push_back(mstch::map{
          {"key", std::string("domain_prefix")},
          {"value", fmt::format("{}", fmt::join(domain_prefix, "."))},
          {"last?", false}});
      result.push_back(mstch::map{
          {"key", std::string("domain_suffix")},
          {"value", package.domain()[domain.size() - 1]},
          {"last?", false}});
    }
    if (!package.path().empty()) {
      result.push_back(mstch::map{
          {"key", std::string("path")},
          {"value", fmt::format("{}", fmt::join(package.path(), "/"))},
          {"last?", false}});
    }
    result.push_back(mstch::map{
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

class json_experimental_service : public mstch_service {
 public:
  json_experimental_service(
      const t_service* s,
      mstch_context& ctx,
      mstch_element_position pos,
      json_codegen_data d)
      : mstch_service(s, ctx, pos), data_(d) {
    register_methods(
        this,
        {
            {"service:path", &json_experimental_service::path},
        });
  }
  mstch::node path() {
    return compiler::get_filepath(*service_, *data_.sm, data_.compiler_path);
  }

 private:
  json_codegen_data data_;
};

class json_experimental_function : public mstch_function {
 public:
  json_experimental_function(
      const t_function* f,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_function(f, ctx, pos), source_mgr_(*sm) {}

 private:
  source_manager& source_mgr_;
};

class json_experimental_struct : public mstch_struct {
 public:
  json_experimental_struct(
      const t_structured* s,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_struct(s, ctx, pos), source_mgr_(*sm) {}

 private:
  source_manager& source_mgr_;
};

class json_experimental_type : public mstch_type {
 public:
  json_experimental_type(
      const t_type* t,
      mstch_context& ctx,
      mstch_element_position pos,
      json_codegen_data d)
      : mstch_type(t, ctx, pos), data_(d) {
    register_methods(
        this,
        {
            {"type:external?", &json_experimental_type::is_external},
            {"type:path", &json_experimental_type::path},
        });
  }

  mstch::node path() {
    return compiler::get_filepath(*type_, *data_.sm, data_.compiler_path);
  }
  mstch::node is_external() {
    return data_.current_program != type_->program();
  }

 private:
  json_codegen_data data_;
};

class json_experimental_field : public mstch_field {
 public:
  json_experimental_field(
      const t_field* f,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_field(f, ctx, pos), source_mgr_(*sm) {}

 private:
  source_manager& source_mgr_;
};

class json_experimental_typedef : public mstch_typedef {
 public:
  json_experimental_typedef(
      const t_typedef* t,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_typedef(t, ctx, pos), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"typedef:exception?", &json_experimental_typedef::is_exception},
        });
  }
  mstch::node is_exception() { return typedef_->is<t_exception>(); }

 private:
  source_manager& source_mgr_;
};

class json_experimental_enum : public mstch_enum {
 public:
  json_experimental_enum(
      const t_enum* e,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_enum(e, ctx, pos), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"enum:empty?", &json_experimental_enum::is_empty},
        });
  }
  mstch::node is_empty() { return enum_->get_enum_values().empty(); }

 private:
  source_manager& source_mgr_;
};

class json_experimental_const_value : public mstch_const_value {
 public:
  json_experimental_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      source_manager* sm)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type),
        source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"value:integer_or_enum?",
             &json_experimental_const_value::is_integer_or_enum},
            {"value:bool_integer_value",
             &json_experimental_const_value::get_bool_integer_value},
            {"value:type_name", &json_experimental_const_value::get_type_name},
            {"value:qualified_name",
             &json_experimental_const_value::get_qualified_name},
            {"value:string_value_any",
             &json_experimental_const_value::string_value_any},
        });
  }
  mstch::node is_integer_or_enum() {
    // Enums are represented with CV_INTEGER with const_value_->is_enum().
    return type_ == cv::CV_INTEGER;
  }
  mstch::node get_bool_integer_value() {
    return type_ == cv::CV_BOOL ? (const_value_->get_bool() ? 1 : 0)
                                : mstch::node();
  }

  mstch::node get_type_name() {
    return current_const_->type()->get_true_type()->get_full_name();
  }

  mstch::node get_qualified_name() {
    return current_const_->type()->get_true_type()->get_scoped_name();
  }
  mstch::node string_value_any() { return to_json(const_value_); }

 private:
  source_manager& source_mgr_;
};

void t_json_experimental_generator::generate_program() {
  out_dir_base_ = "gen-json_experimental";
  const auto* program = get_program();
  data_.current_program = program;
  data_.compiler_path = std::filesystem::current_path().generic_string();
  data_.sm = &source_mgr_;
  set_mstch_factories();
  auto mstch_program = mstch_context_.program_factory->make_mstch_object(
      program, mstch_context_);
  render_to_file(mstch_program, "thrift_ast", program->name() + ".json");
}

void t_json_experimental_generator::set_mstch_factories() {
  mstch_context_.add<json_experimental_program>();
  mstch_context_.add<json_experimental_service>(data_);
  mstch_context_.add<json_experimental_function>(&source_mgr_);
  mstch_context_.add<json_experimental_struct>(&source_mgr_);
  mstch_context_.add<json_experimental_field>(&source_mgr_);
  mstch_context_.add<json_experimental_enum>(&source_mgr_);
  mstch_context_.add<json_experimental_const_value>(&source_mgr_);
  mstch_context_.add<json_experimental_typedef>(&source_mgr_);
  mstch_context_.add<json_experimental_type>(data_);
}

THRIFT_REGISTER_GENERATOR(json_experimental, "JSON_EXPERIMENTAL", "");

} // namespace
} // namespace apache::thrift::compiler
