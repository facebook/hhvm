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
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/lib/py3/util.h>

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

const std::string* get_py_adapter(const t_type* type) {
  if (!type->get_true_type()->is_struct()) {
    return nullptr;
  }
  return t_typedef::get_first_annotation_or_null(type, {"py.adapter"});
}

} // namespace

class t_mstch_pyi_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "pyi"; }

  void generate_program() override;
  mstch::map extend_program(const t_program&) override;
  mstch::map extend_field(const t_field&) override;
  mstch::map extend_type(const t_type&) override;
  mstch::map extend_service(const t_service&) override;
  mstch::map extend_function(const t_function&) override;

 protected:
  void generate_init_files(const t_program&);
  void generate_constants(const t_program&);
  void generate_ttypes(const t_program&);
  void generate_services(const t_program&);
  boost::filesystem::path package_to_path(const t_program&);
  mstch::array get_return_types(const t_program&);
  void add_container_types(const t_program&, mstch::map&);
  std::vector<std::string> get_py_namespace_raw(
      const t_program&, const std::string& tail = "");
  mstch::array get_py_namespace(const t_program&, const std::string& tail = "");
  std::string flatten_type_name(const t_type&) const;
  mstch::array get_python_namespace(const t_program&);
  mstch::array get_py3_namespace(const t_program&);

 private:
  void load_container_type(
      std::vector<const t_type*>& container_types,
      std::set<std::string>& visited_names,
      const t_type* type) const;
};

mstch::map t_mstch_pyi_generator::extend_program(const t_program& program) {
  const auto pyNamespaces = get_py_namespace(program, "");
  const auto pythonNamespaces = get_python_namespace(program);
  const auto py3Namespaces = get_py3_namespace(program);

  mstch::array importModules;
  for (const auto* included_program : program.get_included_programs()) {
    if (included_program->path() == program.path()) {
      continue;
    }

    const auto hasStructs = !included_program->objects().empty();
    const auto hasEnums = !included_program->enums().empty();
    const auto hasTypedefs = !included_program->typedefs().empty();
    const auto hasConsts = !included_program->consts().empty();
    if (hasStructs || hasEnums || hasTypedefs || hasConsts) {
      importModules.push_back(boost::algorithm::join(
          get_py_namespace_raw(*included_program, "ttypes"), "."));
    }
  }

  std::set<std::string> adapterModules;
  for (const auto* strct : program.structs()) {
    for (const auto* t : collect_types(strct)) {
      if (const auto* adapter = get_py_adapter(t)) {
        adapterModules.emplace(adapter->substr(0, adapter->find_last_of('.')));
      }
    }
  }
  for (const auto* type : program.typedefs()) {
    if (const auto* adapter = get_py_adapter(type)) {
      adapterModules.emplace(adapter->substr(0, adapter->find_last_of('.')));
    }
  }

  for (const auto& module : adapterModules) {
    importModules.push_back(module);
  }

  mstch::map result{
      {"returnTypes", get_return_types(program)},
      {"pyNamespaces", pyNamespaces},
      {"pythonNamespaces", pythonNamespaces},
      {"py3Namespaces", py3Namespaces},
      {"importModules", importModules},
      {"asyncio?", has_option("asyncio")},
      {"cpp_transport?", has_option("cpp_transport")},
      {"json?", has_option("json")},
  };
  add_container_types(program, result);

  return result;
}

mstch::map t_mstch_pyi_generator::extend_field(const t_field& field) {
  auto req = field.get_req();
  const auto required = (req == t_field::e_req::required);
  const auto optional = (req == t_field::e_req::optional);
  const auto unqualified = !required && !optional;
  const auto hasValue = (field.get_value() != nullptr);
  const auto hasDefaultValue = hasValue || unqualified;
  const auto requireValue = required && !hasDefaultValue;

  // For typing, can a property getter return None, if so it needs to Optional[]
  const auto isPEP484Optional = (optional || (!hasDefaultValue && !required));

  auto cleanedName = field.get_name();
  if (kKeywords.find(cleanedName) != kKeywords.end()) {
    cleanedName += "_PY_RESERVED_KEYWORD";
  }

  std::string capitalizedName(cleanedName);
  std::transform(
      capitalizedName.begin(),
      capitalizedName.end(),
      capitalizedName.begin(),
      ::toupper);

  return mstch::map{
      {"requireValue?", requireValue},
      {"PEP484Optional?", isPEP484Optional},
      // Override the default name attr
      {"name", cleanedName},
      {"origName", field.get_name()},
      {"capitalizedName", capitalizedName},
  };
}

mstch::map t_mstch_pyi_generator::extend_type(const t_type& type) {
  const auto type_program = type.program();
  const auto program = type_program ? type_program : get_program();
  const auto modulePath = get_py_namespace(*program, "ttypes");
  bool externalProgram = false;
  const auto& prog_path = program->path();
  if (prog_path != get_program()->path()) {
    externalProgram = true;
  }

  mstch::map result{
      {"modulePath", modulePath},
      {"externalProgram?", externalProgram},
      {"flat_name", flatten_type_name(type)}};
  if (const auto* adapter = get_py_adapter(&type)) {
    result["adapter"] = *adapter;
  }

  return result;
}

mstch::map t_mstch_pyi_generator::extend_service(const t_service& service) {
  const auto program = service.program();
  const auto& pyNamespaces = get_py_namespace(*program);
  bool externalProgram = false;
  const auto& prog_path = program->path();
  if (prog_path != get_program()->path()) {
    externalProgram = true;
  }
  return mstch::map{
      {"externalProgram?", externalProgram},
      {"pyNamespaces", pyNamespaces},
      {"programName", program->name()},
  };
}

mstch::map t_mstch_pyi_generator::extend_function(const t_function& func) {
  // Stream and sink functions are not supported, see
  // t_py_generator::get_functions.
  const bool isSupported = !func.returns_stream() && !func.returns_sink() &&
      !func.get_returntype()->is_service();

  return mstch::map{
      {"isSupported?", isSupported},
  };
}

void t_mstch_pyi_generator::generate_init_files(const t_program& program) {
  auto path = package_to_path(program);
  auto directory = boost::filesystem::path{};
  for (const auto& part : path) {
    directory /= part;
    render_to_file(
        program, "common/AutoGeneratedPy", directory / "__init__.pyi");
  }
}

void t_mstch_pyi_generator::generate_constants(const t_program& program) {
  auto path = package_to_path(program);
  const std::string template_name = "constants.pyi";
  render_to_file(program, template_name, path / template_name);
}

void t_mstch_pyi_generator::generate_ttypes(const t_program& program) {
  auto path = package_to_path(program);
  const std::string template_name = "ttypes.pyi";
  render_to_file(program, template_name, path / template_name);
}

void t_mstch_pyi_generator::generate_services(const t_program& program) {
  auto path = package_to_path(program);
  const std::string template_name = "service.pyi";
  for (const auto* service : program.services()) {
    std::string module = service->get_name() + ".pyi";
    mstch::map extra{
        {"service", dump(*service)},
    };
    render_to_file(program, extra, template_name, path / module);
  }
}

boost::filesystem::path t_mstch_pyi_generator::package_to_path(
    const t_program& program) {
  boost::filesystem::path path;

  const auto ns = get_py_namespace_raw(program);
  for (auto it = ns.begin(); it != ns.end(); ++it) {
    path /= *it;
  }
  path += boost::filesystem::path::preferred_separator;

  return path;
}

mstch::array t_mstch_pyi_generator::get_return_types(const t_program& program) {
  mstch::array distinct_return_types;

  std::set<std::string> visited_names;
  for (const auto* service : program.services()) {
    for (const auto* function : service->get_functions()) {
      const auto* returntype = function->get_returntype();
      std::string flat_name = flatten_type_name(*returntype);
      if (!visited_names.count(flat_name)) {
        distinct_return_types.push_back(dump(*returntype));
        visited_names.insert(flat_name);
      }
    }
  }

  return distinct_return_types;
}

/*
 * Add two items to the results map, one "containerTypes" that lists all
 * container types, and one "moveContainerTypes" that treats binary and string
 * as one type. Required because in pxd's we can't have duplicate move(string)
 * definitions */
void t_mstch_pyi_generator::add_container_types(
    const t_program& program, mstch::map& results) {
  std::vector<const t_type*> container_types;
  std::vector<const t_type*> move_container_types;
  std::set<std::string> visited_names;

  for (const auto* service : program.services()) {
    for (const auto& function : service->functions()) {
      for (const auto& param : function.get_paramlist()->fields()) {
        load_container_type(container_types, visited_names, param.get_type());
      }
      auto return_type = function.get_returntype();
      load_container_type(container_types, visited_names, return_type);
    }
  }
  for (const auto* object : program.objects()) {
    for (const auto& field : object->fields()) {
      load_container_type(container_types, visited_names, field.get_type());
    }
  }
  for (const auto* constant : program.consts()) {
    const auto* const_type = constant->get_type();
    load_container_type(container_types, visited_names, const_type);
  }

  results.emplace("containerTypes", dump_elems(container_types));

  // create second set that treats strings and binaries the same
  visited_names.clear();

  for (const auto* type : container_types) {
    auto flat_name = flatten_type_name(*type);
    boost::algorithm::replace_all(flat_name, "binary", "string");

    if (!visited_names.count(flat_name)) {
      visited_names.insert(flat_name);
      move_container_types.push_back(type);
    }
  }

  results.emplace("moveContainerTypes", dump_elems(move_container_types));
}

void t_mstch_pyi_generator::load_container_type(
    std::vector<const t_type*>& container_types,
    std::set<std::string>& visited_names,
    const t_type* type) const {
  if (!type->is_container()) {
    return;
  }

  std::string flat_name = flatten_type_name(*type);
  if (visited_names.count(flat_name)) {
    return;
  }

  if (type->is_list()) {
    const auto elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
    load_container_type(container_types, visited_names, elem_type);
  } else if (type->is_set()) {
    const auto elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
    load_container_type(container_types, visited_names, elem_type);
  } else if (type->is_map()) {
    const auto map_type = dynamic_cast<const t_map*>(type);
    const auto key_type = map_type->get_key_type();
    const auto value_type = map_type->get_val_type();
    load_container_type(container_types, visited_names, key_type);
    load_container_type(container_types, visited_names, value_type);
  }

  visited_names.insert(flat_name);
  container_types.push_back(type);
}

std::string t_mstch_pyi_generator::flatten_type_name(const t_type& type) const {
  if (type.is_list()) {
    return "List__" +
        flatten_type_name(*dynamic_cast<const t_list&>(type).get_elem_type());
  }

  if (type.is_set()) {
    return "Set__" +
        flatten_type_name(*dynamic_cast<const t_set&>(type).get_elem_type());
  }

  if (type.is_map()) {
    return (
        "Map__" +
        flatten_type_name(*dynamic_cast<const t_map&>(type).get_key_type()) +
        "_" +
        flatten_type_name(*dynamic_cast<const t_map&>(type).get_val_type()));
  }

  if (type.is_binary()) {
    return "binary";
  }

  return type.get_name();
}

std::vector<std::string> t_mstch_pyi_generator::get_py_namespace_raw(
    const t_program& program, const std::string& tail) {
  bool isAsyncio = has_option("asyncio");
  auto& py_namespace = program.get_namespace("py");
  auto& py_asyncio_namespace = program.get_namespace("py.asyncio");
  auto namespace_name = isAsyncio && !py_asyncio_namespace.empty()
      ? py_asyncio_namespace
      : py_namespace;

  auto ns = split_namespace(namespace_name);
  if (ns.empty()) {
    ns.push_back(program.name());
  }
  if (!tail.empty()) {
    ns.push_back(tail);
  }

  return ns;
}

mstch::array t_mstch_pyi_generator::get_py_namespace(
    const t_program& program, const std::string& tail) {
  return dump_elems(get_py_namespace_raw(program, tail));
}

mstch::array t_mstch_pyi_generator::get_python_namespace(
    const t_program& program) {
  // TODO: take root_module_prefix into account
  return dump_elems(::apache::thrift::compiler::get_py3_namespace(&program));
}

mstch::array t_mstch_pyi_generator::get_py3_namespace(
    const t_program& program) {
  return dump_elems(::apache::thrift::compiler::get_py3_namespace(&program));
}

void t_mstch_pyi_generator::generate_program() {
  out_dir_base_ = "gen-py";
  generate_init_files(*get_program());
  generate_constants(*get_program());
  generate_ttypes(*get_program());
  generate_services(*get_program());
}

THRIFT_REGISTER_GENERATOR(
    mstch_pyi, "Legacy Python type information", "    no arguments\n");

} // namespace apache::thrift::compiler
