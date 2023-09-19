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
#include <memory>
#include <set>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/lib/py3/util.h>

namespace apache {
namespace thrift {
namespace compiler {

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
    mstch_array.push_back(mstch::map{
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

std::string to_flat_type_name(const t_type* type) {
  assert(type != nullptr);

  const auto* true_type = type->get_true_type();
  assert(true_type != nullptr);

  if (true_type->is_list()) {
    const auto* listType = dynamic_cast<const t_list*>(true_type);
    assert(listType != nullptr);

    return fmt::format(
        "List__{}", to_flat_type_name(listType->get_elem_type()));
  }

  if (true_type->is_set()) {
    const auto* setType = dynamic_cast<const t_set*>(true_type);
    assert(setType != nullptr);

    return fmt::format("Set__{}", to_flat_type_name(setType->get_elem_type()));
  }

  if (true_type->is_map()) {
    const auto* mapType = dynamic_cast<const t_map*>(true_type);
    assert(mapType != nullptr);

    return fmt::format(
        "Map__{}_{}",
        to_flat_type_name(mapType->get_key_type()),
        to_flat_type_name(mapType->get_val_type()));
  }

  if (true_type->is_binary()) {
    return "binary";
  }

  return true_type->get_name();
}

const std::string* get_py_adapter(const t_type* type) {
  assert(type != nullptr);

  if (!type->get_true_type()->is_struct()) {
    return nullptr;
  }

  return t_typedef::get_first_annotation_or_null(type, {"py.adapter"});
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
      import_modules.push_back(boost::algorithm::join(
          get_py_namespaces_raw(prog, is_asyncio, "ttypes"), "."));
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
            {"program:returnTypes", &pyi_mstch_program::get_return_types},
            {"program:pyNamespaces", &pyi_mstch_program::get_py_namespaces},
            {"program:pythonNamespaces",
             &pyi_mstch_program::get_python_namespaces},
            {"program:py3Namespaces", &pyi_mstch_program::get_py3_namespaces},
            {"program:importModules", &pyi_mstch_program::get_import_modules},
            {"program:containerTypes", &pyi_mstch_program::get_containers},
            {"program:moveContainerTypes",
             &pyi_mstch_program::get_move_containers},
        });
    register_has_option("program:asyncio?", "asyncio");
    register_has_option("program:cpp_transport?", "cpp_transport");

    this->visit_import_modules();
    this->visit_return_types();
    this->visit_containers();
  }

  mstch::node get_return_types() {
    return make_mstch_types(this->return_types_);
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
      mstch_array.push_back(module);
    }
    return mstch_array;
  }

  mstch::node get_containers() { return make_mstch_types(this->containers_); }

  mstch::node get_move_containers() {
    return make_mstch_types(this->move_containers_);
  }

 private:
  std::vector<const t_type*> return_types_;
  std::vector<std::string> import_modules_;
  std::vector<const t_type*> containers_;
  std::vector<const t_type*> move_containers_;

  void visit_import_modules() {
    this->import_modules_ = gather_import_modules(
        mstch_program::program_, mstch_base::has_option("asyncio"));
  }

  void visit_return_types() {
    std::set<std::string> visited;
    for (const auto* service : mstch_program::program_->services()) {
      for (const auto* function : service->get_functions()) {
        const auto* returnType = function->return_type();
        std::string name = to_flat_type_name(returnType);

        if (visited.find(name) == visited.end()) {
          visited.insert(name);
          this->return_types_.push_back(returnType);
        }
      }
    }
  }

  void visit_containers() {
    std::set<std::string> visited;

    for (const auto* service : mstch_program::program_->services()) {
      for (const auto& function : service->functions()) {
        for (const auto& param : function.get_paramlist()->fields()) {
          this->add_containers(visited, param.get_type());
        }

        auto return_type = function.return_type();
        this->add_containers(visited, return_type);
      }
    }

    for (const t_struct* object :
         mstch_program::program_->structured_definitions()) {
      for (const auto& field : object->fields()) {
        this->add_containers(visited, field.get_type());
      }
    }

    for (const auto* constant : mstch_program::program_->consts()) {
      const auto* const_type = constant->type();
      this->add_containers(visited, const_type);
    }

    // Collecting move containers within found containers
    visited.clear();
    for (const auto* container : this->containers_) {
      auto name = to_flat_type_name(container);
      boost::algorithm::replace_all(name, "binary", "string");

      if (visited.find(name) == visited.end()) {
        visited.insert(name);
        this->move_containers_.push_back(container);
      }
    }
  }

  void add_containers(std::set<std::string>& visited, const t_type* type) {
    assert(type != nullptr);

    if (!type->is_container()) {
      return;
    }

    std::string name = to_flat_type_name(type);
    if (visited.find(name) != visited.end()) {
      return;
    }

    if (type->is_list()) {
      const auto* listType = dynamic_cast<const t_list*>(type)->get_elem_type();
      add_containers(visited, listType);
    } else if (type->is_set()) {
      const auto* setType = dynamic_cast<const t_set*>(type)->get_elem_type();
      add_containers(visited, setType);
    } else if (type->is_map()) {
      const auto* mapType = dynamic_cast<const t_map*>(type);
      add_containers(visited, mapType->get_key_type());
      add_containers(visited, mapType->get_val_type());
    }

    visited.insert(name);
    this->containers_.push_back(type);
  }
};

// Field

class pyi_mstch_field : public mstch_field {
 public:
  pyi_mstch_field(
      const t_field* field,
      mstch_context& context,
      mstch_element_position position,
      const field_generator_context* field_context)
      : mstch_field(field, context, position, field_context) {
    register_methods(
        this,
        {
            {"field:requireValue?", &pyi_mstch_field::get_require_value},
            {"field:PEP484Optional?", &pyi_mstch_field::get_PEP484_optional},
            {"field:origName", &pyi_mstch_field::get_original_name},
            {"field:capitalizedName", &pyi_mstch_field::get_capitalized_name},
            {"field:py_name", &pyi_mstch_field::get_filtered_name},
        });

    auto field_type = mstch_field::field_->get_req();
    bool is_required = (field_type == t_field::e_req::required);
    bool is_optional = (field_type == t_field::e_req::optional);
    bool is_unqualified = !is_required && !is_optional;
    bool has_value = (mstch_field::field_->get_value() != nullptr);
    bool has_default_value = has_value || is_unqualified;

    this->require_value_ = is_required && !has_default_value;
    this->pep484_optional_ =
        (is_optional || (!has_default_value && !is_required));

    auto filteredName = mstch_field::field_->get_name();
    if (kKeywords.find(filteredName) != kKeywords.end()) {
      filteredName += "_PY_RESERVED_KEYWORD";
    }
    this->filtered_name_ = filteredName;
  }

  mstch::node get_require_value() { return this->require_value_; }

  mstch::node get_PEP484_optional() { return this->pep484_optional_; }

  mstch::node get_filtered_name() { return this->filtered_name_; }

  mstch::node get_original_name() { return mstch_field::field_->get_name(); }

  mstch::node get_capitalized_name() {
    std::string name(this->filtered_name_);
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    return name;
  }

 private:
  bool require_value_;
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
            {"type:flat_name", &pyi_mstch_type::get_flat_name},
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

  mstch::node get_flat_name() { return to_flat_type_name(mstch_type::type_); }

  mstch::node get_adapter() {
    return std::string(*get_py_adapter(mstch_type::type_));
  }

  mstch::node has_adapter() {
    return (get_py_adapter(mstch_type::type_) != nullptr);
  }

 private:
  const t_program* program_;

  const t_program* get_type_program() const {
    const auto* typeProgram = mstch_type::type_->program();
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
            {"service:externalProgram?",
             &pyi_mstch_service::is_external_program},
            {"service:pyNamespaces", &pyi_mstch_service::get_py_namespaces},
            {"service:programName", &pyi_mstch_service::getProgramName},
            {"program:pyNamespaces",
             &pyi_mstch_service::get_program_py_namespaces},
            {"program:importModules",
             &pyi_mstch_service::get_program_import_modules},
        });
    register_has_option("program:asyncio?", "asyncio");

    this->visit_program_import_modules();
  }

  mstch::node is_external_program() {
    const auto& programPath = mstch_service::service_->program()->path();
    return (programPath != this->program_->path());
  }

  mstch::node get_py_namespaces() {
    return create_string_array(get_py_namespaces_raw(
        mstch_service::service_->program(), mstch_base::has_option("asyncio")));
  }

  mstch::node getProgramName() {
    return mstch_service::service_->program()->name();
  }

  mstch::node get_program_py_namespaces() {
    return create_string_array(get_py_namespaces_raw(
        this->program_, mstch_base::has_option("asyncio")));
  }

  mstch::node get_program_import_modules() {
    mstch::array mstch_array;
    for (const auto& module : this->program_import_modules_) {
      mstch_array.push_back(module);
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
        !function_->return_type()->is_service();
  }
};

// Generator

class t_mstch_pyi_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "pyi"; }

  void generate_program() override;

 private:
  boost::filesystem::path root_path_;

  bool should_resolve_typedefs() const override { return true; }

  void create_factories();
  void generate_init_files();
  void generate_constants();
  void generate_ttypes();
  void generate_services();
  void render_file(
      const std::string& template_name,
      const boost::filesystem::path& path,
      const t_service* service = nullptr);
  boost::filesystem::path get_root_path() const;
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
  boost::filesystem::path directory;
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
    const std::string module = service->get_name() + ".pyi";

    this->render_file(template_name, this->root_path_ / module, service);
  }
}

void t_mstch_pyi_generator::render_file(
    const std::string& template_name,
    const boost::filesystem::path& path,
    const t_service* service) {
  auto mstchObject = (service == nullptr)
      ? make_mstch_program_cached(
            this->get_program(), t_mstch_generator::mstch_context_)
      : make_mstch_service_cached(
            service->get_program(), service, t_mstch_generator::mstch_context_);

  t_mstch_generator::render_to_file(mstchObject, template_name, path);
}

boost::filesystem::path t_mstch_pyi_generator::get_root_path() const {
  boost::filesystem::path path;

  auto namespaces = get_py_namespaces_raw(
      this->get_program(), t_mstch_generator::has_option("asyncio"));
  for (const auto& ns : namespaces) {
    path /= ns;
  }
  path += boost::filesystem::path::preferred_separator;

  return path;
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_pyi, "Legacy Python type information", "    no arguments\n");

} // namespace compiler
} // namespace thrift
} // namespace apache
