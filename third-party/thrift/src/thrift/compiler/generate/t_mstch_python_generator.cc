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
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <fmt/format.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/python/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/sema/ast_validator.h>
#include <thrift/compiler/whisker/mstch_compat.h>

namespace apache::thrift::compiler {

namespace {

const t_const* find_structured_adapter_annotation(
    const t_named& node, const char* uri = kPythonAdapterUri) {
  return node.find_structured_annotation_or_null(uri);
}

const t_const* find_structured_adapter_annotation(
    const t_type& type, const char* uri = kPythonAdapterUri) {
  // Traverse typedefs and find first adapter if any.
  return t_typedef::get_first_structured_annotation_or_null(&type, uri);
}

std::string_view get_annotation_property(
    const t_const* annotation, const std::string& key) {
  if (annotation) {
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == key) {
        return item.second->get_string();
      }
    }
  }
  return "";
}

const t_const* get_transitive_annotation_of_adapter_or_null(
    const t_named& node) {
  for (const auto& annotation : node.structured_annotations()) {
    const t_type& annotation_type = *annotation.type();
    if (is_transitive_annotation(annotation_type)) {
      if (annotation_type.has_structured_annotation(kPythonAdapterUri)) {
        return &annotation;
      }
    }
  }
  return nullptr;
}

std::string mangle_program_path(
    const t_program* program, const std::string& root_module_prefix) {
  std::string prefix = root_module_prefix.empty() ? std::string("_fbthrift")
                                                  : root_module_prefix;
  boost::algorithm::replace_all(prefix, ".", "__");
  return get_py3_namespace_with_name_and_prefix(program, prefix, "__");
}

class python_mstch_const_value;

mstch::node adapter_node(
    const t_const* adapter_annotation,
    const t_const* transitive_adapter_annotation,
    mstch_context& context,
    mstch_element_position pos) {
  if (!adapter_annotation) {
    return false;
  }
  auto type_hint = get_annotation_property(adapter_annotation, "typeHint");
  bool is_generic = boost::algorithm::ends_with(type_hint, "[]");
  if (is_generic) {
    type_hint = type_hint.substr(0, type_hint.size() - 2);
  }
  bool is_transitive = (transitive_adapter_annotation != nullptr);
  mstch::map node{
      {"adapter:name", get_annotation_property(adapter_annotation, "name")},
      {"adapter:type_hint", std::string(type_hint)},
      {"adapter:is_generic?", is_generic},
      {"adapter:is_transitive?", is_transitive},
  };
  if (is_transitive) {
    node["adapter:transitive_annotation"] =
        mstch::make_shared_node<python_mstch_const_value>(
            transitive_adapter_annotation->value(),
            context,
            pos,
            transitive_adapter_annotation,
            &*transitive_adapter_annotation->type());
  }
  return node;
}

bool is_invariant_container_type(const t_type* type) {
  // Mapping is invariant in its key type
  // For example, if `Derived` extends `Base`,
  // then Mapping[Derived, Any] is not compatible with Mapping[Base, Any].
  // We first check if the map has an `Base` key type (for abstract types).
  // Then, we recursively verify whether the map's value type, or the element
  // type of a list or set, contains any such mapping incompatibility.
  const t_type* true_type = type->get_true_type();
  if (true_type->is<t_map>()) {
    const t_map* map_type = dynamic_cast<const t_map*>(true_type);
    const t_type* key_type = map_type->get_key_type()->get_true_type();
    return key_type->is<t_structured>() || key_type->is<t_container>() ||
        is_invariant_container_type(map_type->get_val_type());
  } else if (true_type->is<t_list>()) {
    return is_invariant_container_type(
        dynamic_cast<const t_list*>(true_type)->get_elem_type());
  } else if (true_type->is<t_set>()) {
    return is_invariant_container_type(
        dynamic_cast<const t_set*>(true_type)->get_elem_type());
  }

  return false;
}

bool is_invariant_adapter(
    const t_const* adapter_annotation, const t_type* true_type) {
  if (true_type->is<t_primitive_type>() || !adapter_annotation) {
    return false;
  }

  auto type_hint = get_annotation_property(adapter_annotation, "typeHint");
  return boost::algorithm::ends_with(type_hint, "[]");
}

bool field_has_invariant_type(const t_field* field) {
  if (is_invariant_adapter(
          find_structured_adapter_annotation(*field),
          field->get_type()->get_true_type())) {
    return true;
  }

  if (is_invariant_adapter(
          find_structured_adapter_annotation(*field->type()->get_true_type()),
          field->get_type()->get_true_type())) {
    return true;
  }

  return ::apache::thrift::compiler::is_invariant_container_type(
      field->get_type());
}

class python_mstch_program : public mstch_program {
 public:
  python_mstch_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:module_path", &python_mstch_program::module_path},
            {"program:safe_patch?", &python_mstch_program::safe_patch},
            {"program:safe_patch_module_path",
             &python_mstch_program::safe_patch_module_path},
            {"program:module_mangle", &python_mstch_program::module_mangle},
            {"program:py_deprecated_module_path",
             &python_mstch_program::py_deprecated_module_path},
            {"program:py_asyncio_module_path",
             &python_mstch_program::py_asyncio_module_path},
            {"program:include_namespaces",
             &python_mstch_program::include_namespaces},
            {"program:base_library_package",
             &python_mstch_program::base_library_package},
            {"program:root_module_prefix",
             &python_mstch_program::root_module_prefix},
            {"program:adapter_modules", &python_mstch_program::adapter_modules},
            {"program:adapter_type_hint_modules",
             &python_mstch_program::adapter_type_hint_modules},
            {"program:py3_auto_migrate?",
             &python_mstch_program::py3_auto_migrate},

            {"program:is_types_file?",
             {with_no_caching, &python_mstch_program::is_types_file}},
            {"program:is_source_file?",
             {with_no_caching, &python_mstch_program::is_source_file}},
            {"program:is_type_stub?",
             {with_no_caching, &python_mstch_program::is_type_stub}},
            {"program:generate_abstract_types",
             {with_no_caching, &python_mstch_program::generate_abstract_types}},
            {"program:generate_mutable_types",
             {with_no_caching, &python_mstch_program::generate_mutable_types}},
            {"program:generate_immutable_types",
             {with_no_caching,
              &python_mstch_program::generate_immutable_types}},
            {"program:enable_abstract_types?",
             {with_no_caching, &python_mstch_program::enable_abstract_types}},
        });
    register_has_option("program:import_static?", "import_static");
    gather_included_program_namespaces();
    visit_types_for_services_and_interactions();
    visit_types_for_objects();
    visit_types_for_constants();
    visit_types_for_typedefs();
    visit_types_for_adapters();
  }

  mstch::node py3_auto_migrate() { return has_option("auto_migrate"); }

  mstch::node is_types_file() { return has_option("is_types_file"); }
  mstch::node is_source_file() { return has_option("is_source_file"); }
  mstch::node is_type_stub() { return has_option("is_type_stub"); }

  mstch::node include_namespaces() {
    std::vector<const Namespace*> namespaces;
    namespaces.reserve(include_namespaces_.size());
    for (const auto& it : include_namespaces_) {
      namespaces.push_back(&it.second);
    }
    std::sort(
        namespaces.begin(), namespaces.end(), [](const auto* m, const auto* n) {
          return m->ns < n->ns;
        });
    mstch::array a;
    for (const auto& it : namespaces) {
      a.emplace_back(mstch::map{
          {"included_module_path", it->ns},
          {"included_module_mangle", it->ns_mangle},
          {"has_services?", it->has_services},
          {"has_types?", it->has_types},
          {"is_patch?", it->is_patch},
          {"needed_by_patch?", it->needed_by_patch}});
    }
    return a;
  }

  mstch::node module_path() {
    return get_py3_namespace_with_name_and_prefix(
        program_, get_option("root_module_prefix"));
  }

  mstch::node safe_patch() {
    constexpr std::string_view prefix = "gen_safe_patch_";
    return program_->name().substr(0, prefix.size()) == prefix;
  }

  mstch::node safe_patch_module_path() {
    auto ns = std::get<std::string>(module_path());

    // Change the namespace from "path.to.file" to "path.to.gen_safe_patch_file"
    auto pos = ns.rfind('.');
    return ns.substr(0, pos + 1) + "gen_safe_patch_" + ns.substr(pos + 1);
  }

  mstch::node module_mangle() {
    return mangle_program_path(program_, get_option("root_module_prefix"));
  }

  mstch::node py_deprecated_module_path() {
    std::string module_path = program_->get_namespace("py");
    if (module_path.empty()) {
      return program_->name();
    }
    return module_path;
  }

  mstch::node py_asyncio_module_path() {
    std::string module_path = program_->get_namespace("py.asyncio");
    if (module_path.empty()) {
      return program_->name();
    }
    return module_path;
  }

  mstch::node base_library_package() {
    std::string option = get_option("base_library_package");
    return option.empty() ? "thrift.python" : option;
  }

  mstch::node root_module_prefix() {
    std::string prefix = get_option("root_module_prefix");
    return prefix.empty() ? "" : prefix + ".";
  }

  mstch::node adapter_modules() { return module_path_array(adapter_modules_); }

  mstch::node adapter_type_hint_modules() {
    return module_path_array(adapter_type_hint_modules_);
  }

  mstch::node generate_abstract_types() {
    return !get_option("generate_abstract_types").empty();
  }

  mstch::node generate_mutable_types() {
    return !get_option("generate_mutable_types").empty();
  }

  mstch::node generate_immutable_types() {
    return !get_option("generate_immutable_types").empty();
  }

  mstch::node enable_abstract_types() {
    return !get_option("enable_abstract_types").empty();
  }

 protected:
  struct Namespace {
    std::string ns;
    std::string ns_mangle;
    bool has_services;
    bool has_types;
    bool is_patch;
    bool needed_by_patch;
  };

  void gather_included_program_namespaces() {
    auto needed_includes = needed_includes_by_patch(program_);
    for (const t_program* included_program :
         program_->get_includes_for_codegen()) {
      bool has_types =
          !(included_program->structured_definitions().empty() &&
            included_program->enums().empty() &&
            included_program->typedefs().empty() &&
            included_program->consts().empty());
      include_namespaces_[included_program->path()] = Namespace{
          get_py3_namespace_with_name_and_prefix(
              included_program, get_option("root_module_prefix")),
          mangle_program_path(
              included_program, get_option("root_module_prefix")),
          !included_program->services().empty(),
          has_types,
          is_patch_program(included_program),
          static_cast<bool>(needed_includes.count(included_program))};
    }
  }

  void add_typedef_namespace(const t_type* type) {
    auto prog = type->program();
    if (prog && prog != program_) {
      const auto& path = prog->path();
      if (include_namespaces_.find(path) != include_namespaces_.end()) {
        return;
      }
      auto ns = Namespace();
      ns.ns = get_py3_namespace_with_name_and_prefix(
          prog, get_option("root_module_prefix"));
      ns.ns_mangle =
          mangle_program_path(prog, get_option("root_module_prefix"));
      ns.has_services = false;
      ns.has_types = true;
      ns.is_patch = is_patch_program(prog);
      ns.needed_by_patch = true;
      include_namespaces_[path] = std::move(ns);
    }
  }

  void visit_type_single_service(const t_service* service) {
    for (const auto& function : service->functions()) {
      for (const auto& field : function.params().fields()) {
        visit_type(field.get_type());
      }
      for (const t_field& field : get_elems(function.exceptions())) {
        visit_type(field.get_type());
      }
      if (const t_stream* stream = function.stream()) {
        if (const t_throws* exceptions = stream->exceptions()) {
          for (const auto& field : exceptions->fields()) {
            visit_type(field.get_type());
          }
        }
        if (function.has_return_type()) {
          visit_type(function.return_type().get_type());
        }
        visit_type(stream->elem_type().get_type());
      } else {
        visit_type(function.return_type().get_type());
      }
    }
  }

  void visit_types_for_services_and_interactions() {
    for (const auto* service : program_->services()) {
      visit_type_single_service(service);
    }
    for (const auto* interaction : program_->interactions()) {
      visit_type_single_service(interaction);
    }
  }

  void visit_types_for_objects() {
    for (const t_structured* object : program_->structured_definitions()) {
      for (auto&& field : object->fields()) {
        visit_type(field.get_type());
      }
    }
  }

  void visit_types_for_constants() {
    for (const auto& constant : program_->consts()) {
      visit_type(constant->type());
    }
  }

  void visit_types_for_typedefs() {
    for (const auto typedef_def : program_->typedefs()) {
      visit_type(typedef_def->get_type());
    }
  }

  void visit_types_for_adapters() {
    for (const t_structured* strct : program_->structs_and_unions()) {
      if (auto annotation = find_structured_adapter_annotation(*strct)) {
        python::extract_modules_and_insert_into(
            get_annotation_property(annotation, "name"), adapter_modules_);
        python::extract_modules_and_insert_into(
            get_annotation_property(annotation, "typeHint"), adapter_modules_);
        python::extract_modules_and_insert_into(
            get_annotation_property(annotation, "typeHint"),
            adapter_type_hint_modules_);
      }
      for (const auto& field : strct->fields()) {
        if (auto annotation = find_structured_adapter_annotation(field)) {
          python::extract_modules_and_insert_into(
              get_annotation_property(annotation, "name"), adapter_modules_);
          python::extract_modules_and_insert_into(
              get_annotation_property(annotation, "typeHint"),
              adapter_type_hint_modules_);
        }
      }
    }
    for (const auto typedef_def : program_->typedefs()) {
      if (auto annotation = find_structured_adapter_annotation(*typedef_def)) {
        python::extract_modules_and_insert_into(
            get_annotation_property(annotation, "name"), adapter_modules_);
        python::extract_modules_and_insert_into(
            get_annotation_property(annotation, "typeHint"), adapter_modules_);
        python::extract_modules_and_insert_into(
            get_annotation_property(annotation, "typeHint"),
            adapter_type_hint_modules_);
      }
    }
  }

  enum TypeDef { NoTypedef, HasTypedef };

  void visit_type(const t_type* orig_type) {
    return visit_type_with_typedef(orig_type, TypeDef::NoTypedef);
  }

  void visit_type_with_typedef(const t_type* orig_type, TypeDef is_typedef) {
    if (!seen_types_.insert(orig_type).second) {
      return;
    }
    if (auto annotation = find_structured_adapter_annotation(*orig_type)) {
      python::extract_modules_and_insert_into(
          get_annotation_property(annotation, "name"), adapter_modules_);
      python::extract_modules_and_insert_into(
          get_annotation_property(annotation, "typeHint"),
          adapter_type_hint_modules_);
    }
    auto true_type = orig_type->get_true_type();
    is_typedef = is_typedef == TypeDef::HasTypedef || orig_type->is<t_typedef>()
        ? TypeDef::HasTypedef
        : TypeDef::NoTypedef;
    if (is_typedef == TypeDef::HasTypedef) {
      add_typedef_namespace(true_type);
    }
    if (true_type->is<t_list>()) {
      visit_type_with_typedef(
          dynamic_cast<const t_list&>(*true_type).get_elem_type(), is_typedef);
    } else if (true_type->is<t_set>()) {
      visit_type_with_typedef(
          dynamic_cast<const t_set&>(*true_type).get_elem_type(), is_typedef);
    } else if (true_type->is<t_map>()) {
      visit_type_with_typedef(
          dynamic_cast<const t_map&>(*true_type).get_key_type(), is_typedef);
      visit_type_with_typedef(
          dynamic_cast<const t_map&>(*true_type).get_val_type(), is_typedef);
    }
  }

  mstch::node module_path_array(
      const std::unordered_set<std::string_view>& modules) {
    mstch::array a;
    for (const auto& m : modules) {
      a.emplace_back(mstch::map{{"module_path", m}});
    }
    return a;
  }

  std::unordered_map<std::string, Namespace> include_namespaces_;
  std::unordered_set<const t_type*> seen_types_;
  std::unordered_set<std::string_view> adapter_modules_;
  std::unordered_set<std::string_view> adapter_type_hint_modules_;
};

class python_mstch_service : public mstch_service {
 public:
  python_mstch_service(
      const t_service* s,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_program* prog,
      const t_service* containing_service = nullptr)
      : mstch_service(s, ctx, pos, containing_service), prog_(prog) {
    register_methods(
        this,
        {
            {"service:module_path", &python_mstch_service::module_path},
            {"service:program_name", &python_mstch_service::program_name},
            {"service:supported_functions",
             &python_mstch_service::supported_functions},
            {"service:supported_service_functions",
             &python_mstch_service::supported_service_functions},
            {"service:external_program?",
             &python_mstch_service::is_external_program},
        });
  }

  mstch::node module_path() {
    return get_py3_namespace_with_name_and_prefix(
        service_->program(), get_option("root_module_prefix"));
  }

  mstch::node program_name() { return service_->program()->name(); }

  std::vector<t_function*> get_supported_functions(
      const std::function<bool(const t_function*)>& func_filter) {
    std::vector<t_function*> funcs;
    for (auto func : service_->get_functions()) {
      if (func_filter(func)) {
        funcs.push_back(func);
      }
    }
    return funcs;
  }

  mstch::node supported_functions() {
    return make_mstch_functions(
        get_supported_functions([](const t_function* func) -> bool {
          return !func->sink() && !func->is_interaction_constructor();
        }),
        service_);
  }

  mstch::node supported_service_functions() {
    return make_mstch_functions(
        get_supported_functions([](const t_function* func) -> bool {
          return !func->sink() && !func->is_interaction_constructor();
        }),
        service_);
  }

  mstch::node is_external_program() { return prog_ != service_->program(); }

 protected:
  const t_program* prog_;
};

class python_mstch_interaction : public python_mstch_service {
 public:
  using ast_type = t_interaction;

  python_mstch_interaction(
      const t_interaction* interaction,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service,
      const t_program* prog)
      : python_mstch_service(interaction, ctx, pos, prog, containing_service) {}
};

// Generator-specific validator that enforces that a reserved key is not used
// as a namespace component.
void validate_no_reserved_key_in_namespace(
    sema_context& ctx, const t_program& prog) {
  auto namespace_tokens = get_py3_namespace(&prog);
  if (namespace_tokens.empty()) {
    return;
  }
  for (const auto& component : namespace_tokens) {
    if (get_python_reserved_names().find(component) !=
        get_python_reserved_names().end()) {
      ctx.report(
          prog,
          "reserved-keyword-in-namespace-rule",
          diagnostic_level::error,
          "Namespace '{}' contains reserved keyword '{}'",
          fmt::join(namespace_tokens, "."),
          component);
    }
  }

  std::vector<std::string> components;
  boost::split(components, prog.path(), boost::is_any_of("\\/."));
  for (const auto& component : components) {
    if (component == "include") {
      ctx.report(
          prog,
          "no-reserved-keyword-in-namespace",
          diagnostic_level::error,
          "Path '{}' contains reserved keyword 'include'",
          prog.path());
    }
  }
}

class python_mstch_function : public mstch_function {
 public:
  python_mstch_function(
      const t_function* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_interface* iface)
      : mstch_function(f, ctx, pos, iface) {
    register_methods(
        this,
        {
            {"function:created_interaction",
             &python_mstch_function::created_interaction},
            {"function:returns_tuple?", &python_mstch_function::returns_tuple},
            {"function:early_client_return?",
             &python_mstch_function::early_client_return},
            {"function:regular_response_type",
             &python_mstch_function::regular_response_type},
            {"function:with_regular_response?",
             &python_mstch_function::with_regular_response},
            {"function:async_only?", &python_mstch_function::async_only},
        });
  }

  mstch::node created_interaction() {
    const auto& interaction = function_->interaction();
    return interaction ? interaction->get_name() : "";
  }

  mstch::node returns_tuple() {
    // TOOD add in sinks, etc
    return (function_->stream() && function_->has_return_type()) ||
        (function_->interaction() && !function_->return_type()->is_void());
  }

  mstch::node early_client_return() {
    // TOOD add in sinks, etc
    return !function_->return_type()->is_void();
  }

  mstch::node regular_response_type() {
    if (function_->qualifier() == t_function_qualifier::oneway) {
      return {};
    }
    const t_type* rettype = function_->return_type()->get_true_type();
    return context_.type_factory->make_mstch_object(rettype, context_, pos_);
  }

  mstch::node with_regular_response() {
    return !function_->return_type()->is_void();
  }

  mstch::node async_only() {
    return function_->sink_or_stream() ||
        function_->is_interaction_constructor() || is_interaction_member() ||
        function_->interaction();
  }
};

class python_mstch_type : public mstch_type {
 public:
  python_mstch_type(
      const t_type* type,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_program* prog)
      : mstch_type(type->get_true_type(), ctx, pos),
        prog_(prog),
        adapter_annotation_(find_structured_adapter_annotation(*type)),
        transitive_adapter_annotation_(
            get_transitive_annotation_of_adapter_or_null(*type)) {
    register_methods(
        this,
        {
            {"type:program_name", &python_mstch_type::program_name},
            {"type:metadata_path", &python_mstch_type::metadata_path},
            {"type:py3_namespace", &python_mstch_type::py3_namespace},
            {"type:external_program?", &python_mstch_type::is_external_program},
            {"type:integer?", &python_mstch_type::is_integer},
            {"type:iobuf?", &python_mstch_type::is_iobuf},
            {"type:contains_patch?", &python_mstch_type::contains_patch},
            {"type:has_adapter?", &python_mstch_type::adapter},

            {"type:module_name",
             {with_no_caching, &python_mstch_type::module_name}},
            {"type:module_mangle",
             {with_no_caching, &python_mstch_type::module_mangle}},
            {"type:patch_module_path",
             {with_no_caching, &python_mstch_type::patch_module_path}},
            {"type:need_module_path?",
             {with_no_caching, &python_mstch_type::need_module_path}},
            {"type:need_patch_module_path?",
             {with_no_caching, &python_mstch_type::need_patch_module_path}},
        });
  }

  mstch::node module_name() {
    std::string_view types_import_path = [this]() {
      if (!get_option("generate_abstract_types").empty()) {
        return ".thrift_abstract_types";
      }
      if (!get_option("generate_mutable_types").empty()) {
        return ".thrift_mutable_types";
      }
      if (!get_option("generate_immutable_types").empty()) {
        return ".thrift_types";
      }
      throw std::runtime_error(
          "Expected one option out of generate_abstract_types, generate_immutable_types, or generate_mutable_types to be set, and none are set.");
    }();

    return get_py3_namespace_with_name_and_prefix(
               get_type_program(), get_option("root_module_prefix"))
        .append(types_import_path);
  }

  mstch::node module_mangle() {
    std::string_view types_import_path = [this]() {
      if (!get_option("generate_abstract_types").empty()) {
        return "__thrift_abstract_types";
      }
      if (!get_option("generate_mutable_types").empty()) {
        return "__thrift_mutable_types";
      }
      if (!get_option("generate_immutable_types").empty()) {
        return "__thrift_types";
      }
      throw std::runtime_error(
          "Expected one option out of generate_abstract_types, generate_immutable_types, or generate_mutable_types to be set, and none are set.");
    }();

    return mangle_program_path(
               get_type_program(), get_option("root_module_prefix"))
        .append(types_import_path);
  }

  mstch::node patch_module_path() {
    return get_py3_namespace_with_name_and_prefix(
               get_type_program(), get_option("root_module_prefix"))
        .append(".thrift_patch");
  }

  mstch::node program_name() { return get_type_program()->name(); }

  mstch::node metadata_path() {
    if (type_->is<t_enum>()) {
      return get_py3_namespace_with_name_and_prefix(
                 get_type_program(), get_option("root_module_prefix")) +
          ".thrift_enums";
    }
    return get_py3_namespace_with_name_and_prefix(
               get_type_program(), get_option("root_module_prefix")) +
        ".thrift_metadata";
  }

  mstch::node py3_namespace() {
    std::ostringstream ss;
    for (const auto& path : get_py3_namespace(get_type_program())) {
      ss << path << ".";
    }
    return ss.str();
  }

  mstch::node need_module_path() {
    if (!has_option("is_types_file")) {
      return true;
    }
    return is_type_defined_in_the_current_program();
  }

  mstch::node need_patch_module_path() {
    if (!has_option("is_patch_file")) {
      return true;
    }
    return is_type_defined_in_the_current_program();
  }

  mstch::node is_external_program() {
    auto p = type_->program();
    return p && p != prog_;
  }

  mstch::node is_integer() { return type_->is_any_int() || type_->is_byte(); }

  mstch::node is_iobuf() { return is_type_iobuf(type_); }

  mstch::node contains_patch() { return type_contains_patch(type_); }

  mstch::node adapter() {
    return adapter_node(
        adapter_annotation_, transitive_adapter_annotation_, context_, pos_);
  }

 protected:
  const t_program* get_type_program() const {
    if (const t_program* p = type_->program()) {
      return p;
    }
    return prog_;
  }

  bool is_type_defined_in_the_current_program() {
    if (const t_program* prog = type_->program()) {
      if (prog != prog_) {
        return true;
      }
    }
    return false;
  }

  const t_program* prog_;
  const t_const* adapter_annotation_;
  const t_const* transitive_adapter_annotation_;
};

class python_mstch_typedef : public mstch_typedef {
 public:
  python_mstch_typedef(
      const t_typedef* t, mstch_context& ctx, mstch_element_position pos)
      : mstch_typedef(t, ctx, pos),
        adapter_annotation_(find_structured_adapter_annotation(*t)) {
    register_methods(
        this,
        {
            {"typedef:has_adapter?", &python_mstch_typedef::adapter},
        });
  }

  mstch::node adapter() {
    return adapter_node(adapter_annotation_, nullptr, context_, pos_);
  }

 private:
  const t_const* adapter_annotation_;
};

class python_mstch_struct : public mstch_struct {
 public:
  python_mstch_struct(
      const t_structured* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos),
        adapter_annotation_(find_structured_adapter_annotation(*s)) {
    register_methods(
        this,
        {
            {"struct:py_name", &python_mstch_struct::py_name},
            {"struct:fields_ordered_by_id",
             &python_mstch_struct::fields_ordered_by_id},
            {"struct:exception_message?",
             &python_mstch_struct::has_exception_message},
            {"struct:exception_message",
             &python_mstch_struct::exception_message},
            {"struct:has_adapter?", &python_mstch_struct::adapter},
            {"struct:has_invariant_field?",
             &python_mstch_struct::has_invariant_field},
            {"struct:legacy_api?", &python_mstch_struct::legacy_api},
            {"struct:num_fields", &python_mstch_struct::num_fields},
            {"struct:allow_inheritance?",
             &python_mstch_struct::allow_inheritance},
        });
  }

  mstch::node py_name() { return python::get_py3_name(*struct_); }

  mstch::node fields_ordered_by_id() {
    std::vector<const t_field*> fields = struct_->fields().copy();
    std::sort(fields.begin(), fields.end(), [](const auto* m, const auto* n) {
      return m->id() < n->id();
    });
    return make_mstch_fields(fields);
  }

  mstch::node has_invariant_field() {
    return std::any_of(
        struct_->fields().begin(),
        struct_->fields().end(),
        [](const auto& field) { return field_has_invariant_type(&field); });
  }

  mstch::node has_exception_message() {
    return !!dynamic_cast<const t_exception&>(*struct_).get_message_field();
  }

  mstch::node exception_message() {
    const auto* message_field =
        dynamic_cast<const t_exception&>(*struct_).get_message_field();
    return message_field ? python::get_py3_name(*message_field) : "";
  }

  mstch::node adapter() {
    return adapter_node(adapter_annotation_, nullptr, context_, pos_);
  }

  mstch::node legacy_api() {
    return ::apache::thrift::compiler::generate_legacy_api(*struct_);
  }

  mstch::node num_fields() { return struct_->fields().size(); }

  // While inheritance is discouraged, there is limited support for py3
  // auto-migraters
  mstch::node allow_inheritance() {
    return struct_->has_structured_annotation(
        kPythonMigrationBlockingAllowInheritanceUri);
  }

 private:
  const t_const* adapter_annotation_;
};

class python_mstch_field : public mstch_field {
 public:
  python_mstch_field(
      const t_field* field,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context)
      : mstch_field(field, ctx, pos, field_context),
        py_name_(python::get_py3_name(*field)),
        adapter_annotation_(find_structured_adapter_annotation(*field)),
        transitive_adapter_annotation_(
            get_transitive_annotation_of_adapter_or_null(*field)) {
    register_methods(
        this,
        {
            {"field:py_name", &python_mstch_field::py_name},
            {"field:tablebased_qualifier",
             &python_mstch_field::tablebased_qualifier},
            {"field:user_default_value",
             &python_mstch_field::user_default_value},
            {"field:has_adapter?", &python_mstch_field::adapter},
            {"field:is_container_type", &python_mstch_field::is_container_type},
            {"field:is_invariant_type?",
             &python_mstch_field::is_invariant_type},
        });
  }

  mstch::node py_name() {
    if (boost::algorithm::starts_with(py_name_, "__") &&
        !boost::algorithm::ends_with(py_name_, "__")) {
      auto class_name = field_context_->strct->name();
      boost::algorithm::trim_left_if(class_name, boost::is_any_of("_"));
      if (class_name.empty()) {
        return py_name_;
      }
      return "_" + class_name + py_name_;
    }
    return py_name_;
  }
  mstch::node tablebased_qualifier() {
    const std::string enum_type = "FieldQualifier.";
    switch (field_->qualifier()) {
      case t_field_qualifier::none:
      case t_field_qualifier::required:
        return enum_type + "Unqualified";
      case t_field_qualifier::optional:
        return enum_type + "Optional";
      case t_field_qualifier::terse:
        return enum_type + "Terse";
      default:
        throw std::runtime_error("unknown qualifier");
    }
  }
  mstch::node user_default_value() {
    const t_const_value* value = field_->get_value();
    if (!value) {
      return mstch::node();
    }
    if (value->is_empty()) {
      auto true_type = field_->get_type()->get_true_type();
      if ((true_type->is<t_list>() || true_type->is<t_set>()) &&
          value->kind() != t_const_value::CV_LIST) {
        const_cast<t_const_value*>(value)->convert_empty_map_to_list();
      }
      if (true_type->is<t_map>() && value->kind() != t_const_value::CV_MAP) {
        const_cast<t_const_value*>(value)->convert_empty_list_to_map();
      }
    }
    return context_.const_value_factory->make_mstch_object(
        value, context_, pos_, nullptr, nullptr);
  }
  mstch::node adapter() {
    return adapter_node(
        adapter_annotation_, transitive_adapter_annotation_, context_, pos_);
  }

  mstch::node is_container_type() {
    const auto* type = field_->get_type();
    return type->get_true_type()->is<t_list>() ||
        type->get_true_type()->is<t_map>() ||
        type->get_true_type()->is<t_set>();
  }

  mstch::node is_invariant_type() { return field_has_invariant_type(field_); }

 private:
  const std::string py_name_;
  const t_const* adapter_annotation_;
  const t_const* transitive_adapter_annotation_;
};

class python_mstch_enum : public mstch_enum {
 public:
  python_mstch_enum(
      const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum(e, ctx, pos) {
    register_methods(
        this,
        {
            {"enum:flags?", &python_mstch_enum::has_flags},
        });
  }

  mstch::node has_flags() {
    return enum_->has_unstructured_annotation("py3.flags") ||
        enum_->has_structured_annotation(kPythonFlagsUri);
  }
};

class python_mstch_enum_value : public mstch_enum_value {
 public:
  python_mstch_enum_value(
      const t_enum_value* ev, mstch_context& ctx, mstch_element_position pos)
      : mstch_enum_value(ev, ctx, pos) {
    register_methods(
        this,
        {
            {"enum_value:py_name", &python_mstch_enum_value::py_name},
        });
  }

  mstch::node py_name() { return python::get_py3_name(*enum_value_); }
};

// Generator-specific validator that enforces "name" and "value" are not used
// as enum member or union field names (thrift-py3).
namespace enum_member_union_field_names_validator {
template <typename Pred>
void validate(
    const t_named* node,
    const std::string& name,
    sema_context& ctx,
    Pred&& field_name_predicate) {
  auto pyname = node->get_unstructured_annotation("py3.name", &name);
  if (const t_const* annot =
          node->find_structured_annotation_or_null(kPythonNameUri)) {
    if (auto annotation_name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      pyname = annotation_name->get_string();
    }
  }
  if (field_name_predicate(pyname)) {
    ctx.report(
        *node,
        "enum-member-union-field-names-rule",
        diagnostic_level::error,
        "'{}' should not be used as an enum/union field name in thrift-py3. "
        "Use a different name or annotate the field with "
        "`@python.Name{{name=\"<new_py_name>\"}}`",
        pyname);
  }
}
bool validate_enum(sema_context& ctx, const t_enum& enm) {
  auto predicate = [](const auto& pyname) {
    return pyname == "name" || pyname == "value";
  };
  for (const t_enum_value* ev : enm.get_enum_values()) {
    validate(ev, ev->get_name(), ctx, predicate);
  }
  return true;
}

bool validate_union(sema_context& ctx, const t_union& s) {
  auto predicate = [](const auto& pyname) {
    return pyname == "type" || pyname == "value" || pyname == "Type";
  };
  for (const t_field& f : s.fields()) {
    validate(&f, f.name(), ctx, predicate);
  }
  return true;
}

} // namespace enum_member_union_field_names_validator

namespace module_name_collision_validator {

void validate_module_name_collision(
    const t_named& node,
    const std::string& name,
    sema_context& ctx,
    diagnostic_level level) {
  // the structured annotation @python.Name overrides unstructured py3.name
  std::reference_wrapper<const std::string> pyname =
      node.get_unstructured_annotation("py3.name", &name);
  if (const t_const* annot =
          node.find_structured_annotation_or_null(kPythonNameUri)) {
    if (auto annotation_name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      pyname = annotation_name->get_string();
    }
  }
  if (pyname.get() == node.program()->name()) {
    ctx.report(
        node,
        "python-empty-namespace-symbol-collides-with-module",
        level,
        "'{}' is declared in module of the same name. "
        "To fix, add a non-empty py3 namespace or change the name to "
        " no longer collide with the module name. ",
        pyname.get());
  }
}

void validate_named(sema_context& ctx, const t_named& d) {
  validate_module_name_collision(d, d.name(), ctx, diagnostic_level::error);
}
void warn_named(sema_context& ctx, const t_named& d) {
  validate_module_name_collision(d, d.name(), ctx, diagnostic_level::warning);
}

} // namespace module_name_collision_validator

std::filesystem::path program_to_path(const t_program& prog) {
  auto package = get_py3_namespace(&prog);
  return fmt::format("{}", fmt::join(package, "/"));
}

class t_mstch_python_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "field:has_adapter?",
        "field:type",
    };
    return opts;
  }

  std::string template_prefix() const override { return "python"; }

  void generate_program() override {
    generate_root_path_ = program_to_path(*get_program());
    out_dir_base_ = "gen-python";
    auto include_prefix = get_option("include_prefix").value_or("");
    if (!include_prefix.empty()) {
      program_->set_include_prefix(std::move(include_prefix));
    }
    set_mstch_factories();
    generate_types();
    generate_metadata();
    generate_clients();
    generate_services();
  }

  void fill_validator_visitors(ast_validator& validator) const override {
    validator.add_program_visitor(validate_no_reserved_key_in_namespace);
    validator.add_enum_visitor(
        enum_member_union_field_names_validator::validate_enum);
    validator.add_union_visitor(
        enum_member_union_field_names_validator::validate_union);
    if (get_py3_namespace(program_).empty()) {
      validator.add_structured_definition_visitor(
          module_name_collision_validator::validate_named);
      validator.add_enum_visitor(
          module_name_collision_validator::validate_named);
      validator.add_const_visitor(
          module_name_collision_validator::validate_named);
      validator.add_typedef_visitor(
          module_name_collision_validator::validate_named);
      validator.add_interface_visitor(
          module_name_collision_validator::warn_named);
    }
  }

  enum class TypesFileKind { NotATypesFile, SourceFile, TypeStub };
  enum class TypeKind { Abstract, Immutable, Mutable };

 protected:
  bool should_resolve_typedefs() const override { return true; }
  void set_mstch_factories();
  void generate_file(
      const std::string& template_name,
      TypesFileKind types_file_kind,
      TypeKind type_kind,
      const std::filesystem::path& base);
  void set_types_file(bool val);
  void generate_types();
  void generate_metadata();
  void generate_clients();
  void generate_services();

  std::filesystem::path generate_root_path_;
};

class python_mstch_const : public mstch_const {
 public:
  python_mstch_const(
      const t_const* c,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      const t_field* field)
      : mstch_const(c, ctx, pos, current_const, expected_type, field),
        adapter_annotation_(find_structured_adapter_annotation(*c)),
        transitive_adapter_annotation_(
            get_transitive_annotation_of_adapter_or_null(*c)) {
    register_methods(
        this,
        {
            {"constant:has_adapter?", &python_mstch_const::has_adapter},
            {"constant:adapter_name", &python_mstch_const::adapter_name},
            {"constant:adapter_type_hint",
             &python_mstch_const::adapter_type_hint},
            {"constant:is_adapter_transitive?",
             &python_mstch_const::is_adapter_transitive},
            {"constant:transitive_adapter_annotation",
             &python_mstch_const::transitive_adapter_annotation},
            {"constant:uri", &python_mstch_const::uri},
        });
  }

  mstch::node has_adapter() { return adapter_annotation_ != nullptr; }

  mstch::node adapter_name() {
    return get_annotation_property(adapter_annotation_, "name");
  }

  mstch::node adapter_type_hint() {
    return get_annotation_property(adapter_annotation_, "typeHint");
  }

  mstch::node is_adapter_transitive() {
    return transitive_adapter_annotation_ != nullptr;
  }

  mstch::node transitive_adapter_annotation() {
    return mstch::make_shared_node<python_mstch_const_value>(
        transitive_adapter_annotation_->value(),
        context_,
        pos_,
        transitive_adapter_annotation_,
        &*transitive_adapter_annotation_->type());
  }

  mstch::node uri() { return const_->uri(); }

 private:
  const t_const* adapter_annotation_;
  const t_const* transitive_adapter_annotation_;
};

class python_mstch_const_value : public mstch_const_value {
 public:
  python_mstch_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type) {
    register_methods(
        this,
        {
            {"value:py3_enum_value_name",
             &python_mstch_const_value::py3_enum_value_name},
            {"value:py3_binary?", &python_mstch_const_value::is_binary},
            {"value:unicode_value", &python_mstch_const_value::unicode_value},
            {"value:const_enum_type",
             &python_mstch_const_value::const_enum_type},
            {"value:value_for_bool?",
             &python_mstch_const_value::value_for_bool},
            {"value:value_for_floating_point?",
             &python_mstch_const_value::value_for_floating_point},
            {"value:list_elem_type", &python_mstch_const_value::list_elem_type},
            {"value:value_for_set?", &python_mstch_const_value::value_for_set},
            {"value:map_key_type", &python_mstch_const_value::map_key_type},
            {"value:map_val_type", &python_mstch_const_value::map_val_type},
        });
  }

  mstch::node unicode_value() {
    if (type_ != cv::CV_STRING) {
      return {};
    }
    return get_escaped_string<nonascii_handling::no_escape>(
        const_value_->get_string());
  }

  mstch::node is_binary() {
    auto& ttype = const_value_->ttype();
    return type_ == cv::CV_STRING && ttype &&
        ttype->get_true_type()->is_binary();
  }

  mstch::node const_enum_type() {
    if (!const_value_->ttype() || type_ != cv::CV_INTEGER ||
        !const_value_->is_enum()) {
      return {};
    }
    const auto* type = const_value_->ttype()->get_true_type();
    if (type->is<t_enum>()) {
      return context_.type_factory->make_mstch_object(type, context_);
    }
    return {};
  }

  mstch::node value_for_bool() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is_bool();
    }
    return false;
  }

  mstch::node value_for_floating_point() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is_floating_point();
    }
    return false;
  }

  mstch::node py3_enum_value_name() {
    if (!const_value_->is_enum() || const_value_->get_enum_value() == nullptr) {
      return mstch::node();
    }
    const auto& enum_name = const_value_->get_enum()->get_name();
    return python::get_py3_name_class_scope(
        *const_value_->get_enum_value(), enum_name);
  }

  mstch::node list_elem_type() {
    if (auto ttype = const_value_->ttype()) {
      const auto* type = ttype->get_true_type();
      const t_type* elem_type = nullptr;
      if (type->is<t_list>()) {
        elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
      } else if (type->is<t_set>()) {
        elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
      } else {
        return {};
      }
      return context_.type_factory->make_mstch_object(
          elem_type, context_, pos_);
    }
    return {};
  }

  mstch::node value_for_set() {
    if (auto ttype = const_value_->ttype()) {
      return ttype->get_true_type()->is<t_set>();
    }
    return false;
  }

  mstch::node map_key_type() {
    if (auto ttype = const_value_->ttype()) {
      const auto* type = ttype->get_true_type();
      if (type->is<t_map>()) {
        return context_.type_factory->make_mstch_object(
            dynamic_cast<const t_map*>(type)->get_key_type(), context_, pos_);
      }
    }
    return {};
  }

  mstch::node map_val_type() {
    if (auto ttype = const_value_->ttype()) {
      const auto* type = ttype->get_true_type();
      if (type->is<t_map>()) {
        return context_.type_factory->make_mstch_object(
            dynamic_cast<const t_map*>(type)->get_val_type(), context_, pos_);
      }
    }
    return {};
  }
};

class python_mstch_deprecated_annotation : public mstch_deprecated_annotation {
 public:
  python_mstch_deprecated_annotation(
      const t_annotation* a, mstch_context& ctx, mstch_element_position pos)
      : mstch_deprecated_annotation(a, ctx, pos) {
    register_methods(
        this,
        {
            {"annotation:value?",
             &python_mstch_deprecated_annotation::has_value},
            {"annotation:py_quoted_key",
             &python_mstch_deprecated_annotation::py_quoted_key},
            {"annotation:py_quoted_value",
             &python_mstch_deprecated_annotation::py_quoted_value},
        });
  }

  mstch::node has_value() { return !val_.value.empty(); }
  mstch::node py_quoted_key() { return to_python_string_literal(key_); }
  mstch::node py_quoted_value() { return to_python_string_literal(val_.value); }

 protected:
  std::string to_python_string_literal(std::string val) const {
    std::string quotes = R"(""")";
    boost::algorithm::replace_all(val, "\\", "\\\\");
    boost::algorithm::replace_all(val, "\"", "\\\"");
    return quotes + val + quotes;
  }
};

void t_mstch_python_generator::set_mstch_factories() {
  mstch_context_.add<python_mstch_program>();
  mstch_context_.add<python_mstch_service>(program_);
  mstch_context_.add<python_mstch_interaction>(program_);
  mstch_context_.add<python_mstch_function>();
  mstch_context_.add<python_mstch_type>(program_);
  mstch_context_.add<python_mstch_typedef>();
  mstch_context_.add<python_mstch_struct>();
  mstch_context_.add<python_mstch_field>();
  mstch_context_.add<python_mstch_enum>();
  mstch_context_.add<python_mstch_enum_value>();
  mstch_context_.add<python_mstch_const>();
  mstch_context_.add<python_mstch_const_value>();
  mstch_context_.add<python_mstch_deprecated_annotation>();
}

void t_mstch_python_generator::generate_file(
    const std::string& template_name,
    TypesFileKind types_file_kind,
    TypeKind type_kind,
    const std::filesystem::path& base = {}) {
  t_program* program = get_program();
  const std::string& program_name = program->name();
  mstch_context_
      .set_or_erase_option(
          types_file_kind != TypesFileKind::NotATypesFile, "is_types_file", "")
      .set_or_erase_option(
          types_file_kind == TypesFileKind::SourceFile, "is_source_file", "")
      .set_or_erase_option(
          types_file_kind == TypesFileKind::TypeStub, "is_type_stub", "")
      .set_or_erase_option(
          type_kind == TypeKind::Abstract, "generate_abstract_types", "yes")
      .set_or_erase_option(
          type_kind == TypeKind::Immutable, "generate_immutable_types", "yes")
      .set_or_erase_option(
          type_kind == TypeKind::Mutable, "generate_mutable_types", "yes");

  std::shared_ptr<mstch_base> mstch_program =
      make_mstch_program_cached(program, mstch_context_);
  render_to_file(
      mstch_program,
      template_name,
      base / program_name / template_name // (output) path
  );
}

void t_mstch_python_generator::generate_types() {
  // DO_BEFORE(satishvk, 20250130): Remove flags related to abstract types after
  // launch.
  const bool enable_abstract_types = !has_option("disable_abstract_types");

  mstch_context_.set_or_erase_option(
      enable_abstract_types, "enable_abstract_types", "true");
  generate_file(
      "thrift_types.py",
      TypesFileKind::SourceFile,
      TypeKind::Immutable,
      generate_root_path_);
  generate_file(
      "thrift_types.pyi",
      TypesFileKind::TypeStub,
      TypeKind::Immutable,
      generate_root_path_);
  generate_file(
      "thrift_enums.py",
      TypesFileKind::SourceFile,
      TypeKind::Immutable,
      generate_root_path_);

  generate_file(
      "thrift_abstract_types.py",
      TypesFileKind::SourceFile,
      TypeKind::Abstract,
      generate_root_path_);

  generate_file(
      "thrift_mutable_types.py",
      TypesFileKind::SourceFile,
      TypeKind::Mutable,
      generate_root_path_);

  generate_file(
      "thrift_mutable_types.pyi",
      TypesFileKind::TypeStub,
      TypeKind::Mutable,
      generate_root_path_);

  mstch_context_.options["enable_abstract_types"] = "true";
}

void t_mstch_python_generator::generate_metadata() {
  generate_file(
      "thrift_metadata.py",
      TypesFileKind::SourceFile,
      TypeKind::Immutable,
      generate_root_path_);
}

void t_mstch_python_generator::generate_clients() {
  if (get_program()->services().empty()) {
    // There is no need to generate empty / broken code for non existent
    // services.
    return;
  }

  generate_file(
      "thrift_clients.py",
      TypesFileKind::NotATypesFile,
      TypeKind::Immutable,
      generate_root_path_);

  generate_file(
      "thrift_mutable_clients.py",
      TypesFileKind::NotATypesFile,
      TypeKind::Mutable,
      generate_root_path_);
}

void t_mstch_python_generator::generate_services() {
  if (get_program()->services().empty()) {
    // There is no need to generate empty / broken code for non existent
    // services.
    return;
  }
  generate_file(
      "thrift_services.py",
      TypesFileKind::NotATypesFile,
      TypeKind::Immutable,
      generate_root_path_);

  generate_file(
      "thrift_mutable_services.py",
      TypesFileKind::NotATypesFile,
      TypeKind::Mutable,
      generate_root_path_);
}

class t_python_patch_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "field:has_adapter?",
        "field:type",
    };
    return opts;
  }

  std::string template_prefix() const override { return "patch"; }

  void generate_program() override {
    out_dir_base_ = "gen-python-patch";

    set_mstch_factories();
    mstch_context_.set_or_erase_option(true, "generate_immutable_types", "yes");
    mstch_context_.set_or_erase_option(true, "is_patch_file", "");
    const auto* program = get_program();
    auto mstch_program = mstch_context_.program_factory->make_mstch_object(
        program, mstch_context_);

    render_to_file(
        std::move(mstch_program),
        "thrift_patch.py",
        program_to_path(*get_program()) / program->name() / "thrift_patch.py");
  }

 private:
  void set_mstch_factories() {
    mstch_context_.add<python_mstch_program>();
    mstch_context_.add<python_mstch_struct>();
    mstch_context_.add<python_mstch_field>();
    mstch_context_.add<python_mstch_type>(program_);
  }
};

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_python,
    "Python",
    "    include_prefix:  Use full include paths in generated files.\n"
    "    disable_abstract_types:\n"
    "      Disable the use of abstract types with thrift-python"
    "      immutable and mutable types.\n");

namespace patch {
THRIFT_REGISTER_GENERATOR(
    python_patch, "Python patch", "Python patch generator\n");
}

} // namespace apache::thrift::compiler
