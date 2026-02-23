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

#include <algorithm>
#include <filesystem>
#include <memory>
#include <set>
#include <string>

#include <boost/algorithm/string/replace.hpp>
#include <fmt/format.h>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/python/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache::thrift::compiler {

namespace {

// Reserved Python keywords that are not blocked by thrift grammar - note that
// this is actually a longer list than what t_py_generator checks, but may
// as well fix up more of them here.
const std::unordered_set<std::string> kKeywords = {
    "async",
    "await",
    "from",
    "nonlocal",
    "DEF",
    "ELIF",
    "ELSE",
    "False",
    "IF",
    "None",
    "True",
};

// TO-DO: remove duplicate in py3
mstch::array create_string_array(const std::vector<std::string>& values) {
  mstch::array mstch_array;
  for (auto it = values.begin(); it != values.end(); ++it) {
    mstch_array.emplace_back(
        mstch::map{
            {"value", *it},
            {"first?", it == values.begin()},
            {"last?", std::next(it) == values.end()},
        });
  }

  return mstch_array;
}

// TO-DO: remove duplicate in py3
bool has_types(const t_program* program) {
  assert(program != nullptr);

  return !(
      program->structured_definitions().empty() && program->enums().empty() &&
      program->typedefs().empty() && program->consts().empty());
}

std::vector<std::string> get_py_namespaces_raw(
    const t_program* program, bool is_asyncio, const std::string& tail = {}) {
  assert(program != nullptr);

  auto& py_namespace = program->get_namespace("py");
  auto& py_asyncio_namespace = program->get_namespace("py.asyncio");
  auto namespace_name = is_asyncio && !py_asyncio_namespace.empty()
      ? py_asyncio_namespace
      : py_namespace;

  auto ns = split_namespace(namespace_name);
  if (ns.empty()) {
    ns.push_back(program->name());
  }
  if (!tail.empty()) {
    ns.push_back(tail);
  }

  return ns;
}

const std::string* get_py_adapter(const t_type* type) {
  assert(type != nullptr);

  if (!type->get_true_type()->is<t_struct>() &&
      !type->get_true_type()->is<t_union>()) {
    return nullptr;
  }

  return t_typedef::get_first_unstructured_annotation_or_null(
      type, {"py.adapter"});
}

std::set<std::string> get_distinct_adapters(const t_program* program) {
  assert(program != nullptr);

  std::set<std::string> adapters;

  auto add_adapter = [&adapters](const t_type* type) {
    const auto* adapter = get_py_adapter(type);
    if (adapter != nullptr) {
      adapters.insert(adapter->substr(0, adapter->find_last_of('.')));
    }
  };

  for (const auto* strct : program->structs_and_unions()) {
    for (const auto* type : collect_types(strct)) {
      add_adapter(type);
    }
  }

  for (const auto* type : program->typedefs()) {
    add_adapter(type);
  }

  return adapters;
}

std::vector<std::string> gather_import_modules(
    const t_program* program, bool is_asyncio) {
  assert(program != nullptr);

  std::vector<std::string> import_modules = {};

  for (const auto* prog : program->get_includes_for_codegen()) {
    if (prog->path() == program->path()) {
      continue;
    }

    if (has_types(prog)) {
      import_modules.push_back(
          fmt::format(
              "{}",
              fmt::join(
                  get_py_namespaces_raw(prog, is_asyncio, "ttypes"), ".")));
    }
  }

  auto adapters = get_distinct_adapters(program);
  for (const auto& adapter : adapters) {
    import_modules.push_back(adapter);
  }

  return import_modules;
}

// Program

class pyi_mstch_program : public mstch_program {
 public:
  pyi_mstch_program(
      const t_program* program,
      mstch_context& context,
      mstch_element_position position)
      : mstch_program(program, context, position) {
    register_methods(
        this,
        {
            {"program:pyNamespaces", &pyi_mstch_program::get_py_namespaces},
            {"program:pythonNamespaces",
             &pyi_mstch_program::get_python_namespaces},
            {"program:py3Namespaces", &pyi_mstch_program::get_py3_namespaces},
            {"program:importModules", &pyi_mstch_program::get_import_modules},
            {"program:enablePosArgs?", &pyi_mstch_program::get_enable_pos_args},
        });

    this->visit_import_modules();
  }

  mstch::node get_py_namespaces() {
    return create_string_array(get_py_namespaces_raw(
        mstch_program::program_, mstch_base::has_option("asyncio")));
  }

  mstch::node get_python_namespaces() {
    // TODO: take root_module_prefix into account
    return create_string_array(
        ::apache::thrift::compiler::get_py3_namespace(mstch_program::program_));
  }

  mstch::node get_py3_namespaces() {
    return create_string_array(
        ::apache::thrift::compiler::get_py3_namespace(mstch_program::program_));
  }

  mstch::node get_import_modules() {
    mstch::array mstch_array;
    for (const auto& module : this->import_modules_) {
      mstch_array.emplace_back(module);
    }
    return mstch_array;
  }

  mstch::node get_enable_pos_args() { return has_option("enable_pos_args"); }

 private:
  std::vector<std::string> import_modules_;

  void visit_import_modules() {
    this->import_modules_ = gather_import_modules(
        mstch_program::program_, mstch_base::has_option("asyncio"));
  }
};

// Field

class pyi_mstch_field : public mstch_field {
 public:
  pyi_mstch_field(
      const t_field* field,
      mstch_context& context,
      mstch_element_position position)
      : mstch_field(field, context, position) {
    register_methods(
        this,
        {
            {"field:PEP484Optional?", &pyi_mstch_field::get_PEP484_optional},
            {"field:capitalizedName", &pyi_mstch_field::get_capitalized_name},
            {"field:py_name", &pyi_mstch_field::get_filtered_name},
        });

    auto field_type = mstch_field::field_->qualifier();
    bool is_required = (field_type == t_field_qualifier::required);
    bool is_optional = (field_type == t_field_qualifier::optional);
    bool is_unqualified = !is_required && !is_optional;
    bool has_value = (mstch_field::field_->default_value() != nullptr);
    bool has_default_value = has_value || is_unqualified;

    this->pep484_optional_ =
        (is_optional || (!has_default_value && !is_required));

    auto filteredName = mstch_field::field_->name();
    if (kKeywords.find(filteredName) != kKeywords.end()) {
      filteredName += "_PY_RESERVED_KEYWORD";
    }
    this->filtered_name_ = filteredName;
  }

  mstch::node get_PEP484_optional() { return this->pep484_optional_; }

  mstch::node get_filtered_name() { return this->filtered_name_; }

  mstch::node get_capitalized_name() {
    std::string name(this->filtered_name_);
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    return name;
  }

 private:
  bool pep484_optional_;
  std::string filtered_name_;
};

// Type

class pyi_mstch_type : public mstch_type {
 public:
  pyi_mstch_type(
      const t_type* type,
      mstch_context& context,
      mstch_element_position position,
      const t_program* program)
      : mstch_type(type, context, position), program_(program) {
    register_methods(
        this,
        {
            {"type:modulePath", &pyi_mstch_type::get_module_path},
            {"type:externalProgram?", &pyi_mstch_type::is_external_program},
            {"type:adapter", &pyi_mstch_type::get_adapter},
            {"type:has_adapter?", &pyi_mstch_type::has_adapter},
        });
  }

  mstch::node get_module_path() {
    return create_string_array(get_py_namespaces_raw(
        this->get_type_program(), mstch_base::has_option("asyncio"), "ttypes"));
  }

  mstch::node is_external_program() {
    return (this->get_type_program()->path() != this->program_->path());
  }

  mstch::node get_adapter() {
    return std::string(*get_py_adapter(mstch_type::type_));
  }

  mstch::node has_adapter() {
    return (get_py_adapter(mstch_type::type_) != nullptr);
  }

 private:
  const t_program* program_;

  const t_program* get_type_program() const {
    const auto* typeProgram = mstch_type::resolved_type_->program();
    return (typeProgram != nullptr) ? typeProgram : this->program_;
  }
};

// Service

class pyi_mstch_service : public mstch_service {
 public:
  pyi_mstch_service(
      const t_service* service,
      mstch_context& context,
      mstch_element_position position,
      const t_program* program)
      : mstch_service(service, context, position), program_(program) {
    register_methods(
        this,
        {
            {"service:pyNamespaces", &pyi_mstch_service::get_py_namespaces},
            {"program:pyNamespaces",
             &pyi_mstch_service::get_program_py_namespaces},
            {"program:importModules",
             &pyi_mstch_service::get_program_import_modules},
        });

    this->visit_program_import_modules();
  }

  mstch::node get_py_namespaces() {
    return create_string_array(get_py_namespaces_raw(
        mstch_service::service_->program(), mstch_base::has_option("asyncio")));
  }

  mstch::node get_program_py_namespaces() {
    return create_string_array(get_py_namespaces_raw(
        this->program_, mstch_base::has_option("asyncio")));
  }

  mstch::node get_program_import_modules() {
    mstch::array mstch_array;
    for (const auto& module : this->program_import_modules_) {
      mstch_array.emplace_back(module);
    }
    return mstch_array;
  }

 private:
  const t_program* program_;
  std::vector<std::string> program_import_modules_;

  void visit_program_import_modules() {
    this->program_import_modules_ = gather_import_modules(
        this->program_, mstch_base::has_option("asyncio"));
  }
};

// Function

class pyi_mstch_function : public mstch_function {
 public:
  pyi_mstch_function(
      const t_function* function,
      mstch_context& context,
      mstch_element_position position)
      : mstch_function(function, context, position) {
    register_methods(
        this,
        {
            {"function:isSupported?", &pyi_mstch_function::is_supported},
        });
  }

  mstch::node is_supported() {
    // Stream and sink functions are not supported, see
    // t_py_generator::get_functions.
    return !function_->sink_or_stream() &&
        !function_->is_interaction_constructor();
  }
};

// Generator

class t_mstch_pyi_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "service:autogen_path",
        "program:autogen_path",
    };
    return opts;
  }

  std::string template_prefix() const override { return "pyi"; }

  void generate_program() override;

 private:
  std::filesystem::path root_path_;

  void create_factories();
  void generate_init_files();
  void generate_constants();
  void generate_ttypes();
  void generate_services();
  void render_file(
      const std::string& template_name,
      const std::filesystem::path& path,
      const t_service* service = nullptr);
  std::filesystem::path get_root_path() const;
};

void t_mstch_pyi_generator::generate_program() {
  this->root_path_ = this->get_root_path();
  this->out_dir_base_ = "gen-py";

  this->create_factories();

  this->generate_init_files();
  this->generate_constants();
  this->generate_ttypes();
  this->generate_services();
}

void t_mstch_pyi_generator::create_factories() {
  t_mstch_generator::mstch_context_.add<pyi_mstch_program>();
  t_mstch_generator::mstch_context_.add<pyi_mstch_field>();
  t_mstch_generator::mstch_context_.add<pyi_mstch_type>(this->get_program());
  t_mstch_generator::mstch_context_.add<pyi_mstch_service>(this->get_program());
  t_mstch_generator::mstch_context_.add<pyi_mstch_function>();
}

void t_mstch_pyi_generator::generate_init_files() {
  std::filesystem::path directory;
  for (const auto& part : this->root_path_) {
    directory /= part;

    this->render_file("common/AutoGeneratedPy", directory / "__init__.pyi");
  }
}

void t_mstch_pyi_generator::generate_constants() {
  const std::string template_name = "constants.pyi";
  this->render_file(template_name, this->root_path_ / template_name);
}

void t_mstch_pyi_generator::generate_ttypes() {
  const std::string template_name = "ttypes.pyi";
  this->render_file(template_name, this->root_path_ / template_name);
}

void t_mstch_pyi_generator::generate_services() {
  const std::string template_name = "service.pyi";
  for (const auto* service : this->get_program()->services()) {
    const std::string module = service->name() + ".pyi";

    this->render_file(template_name, this->root_path_ / module, service);
  }
}

void t_mstch_pyi_generator::render_file(
    const std::string& template_name,
    const std::filesystem::path& path,
    const t_service* service) {
  auto mstchObject = (service == nullptr)
      ? make_mstch_program_cached(
            this->get_program(), t_mstch_generator::mstch_context_)
      : make_mstch_service_cached(
            service->program(), service, t_mstch_generator::mstch_context_);

  t_mstch_generator::render_to_file(mstchObject, template_name, path);
}

std::filesystem::path t_mstch_pyi_generator::get_root_path() const {
  std::filesystem::path path;

  auto namespaces = get_py_namespaces_raw(
      this->get_program(), t_mstch_generator::has_option("asyncio"));
  for (const auto& ns : namespaces) {
    path /= ns;
  }
  path += std::filesystem::path::preferred_separator;

  return path;
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_pyi, "Legacy Python type information", "    no arguments\n");

} // namespace apache::thrift::compiler
