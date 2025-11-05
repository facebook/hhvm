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

#include <thrift/common/BaseType.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/python/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/sema/ast_validator.h>
#include <thrift/compiler/whisker/mstch_compat.h>

using ::apache::thrift::type::BaseType;

namespace apache::thrift::compiler {

namespace {

enum class types_file_kind { not_a_types_file, source_file, type_stub };
enum class type_kind { abstract, immutable, mutable_ };

class python_generator_context {
 public:
  python_generator_context(bool is_patch_file, type_kind type_kind)
      : is_patch_file_(is_patch_file), type_kind_(type_kind) {}

  python_generator_context(python_generator_context&&) = default;
  python_generator_context& operator=(python_generator_context&&) = default;

  void reset(
      const types_file_kind& types_file_kind,
      const type_kind& type_kind) noexcept {
    types_file_kind_ = types_file_kind;
    type_kind_ = type_kind;
  }

  void set_enable_abstract_types(const bool& value) noexcept {
    enable_abstract_types_ = value;
  }
  bool enable_abstract_types() const noexcept { return enable_abstract_types_; }

  bool is_patch_file() const noexcept { return is_patch_file_; }

  bool is_types_file() const noexcept {
    return types_file_kind_ != types_file_kind::not_a_types_file;
  }
  bool is_source_file() const noexcept {
    return types_file_kind_ == types_file_kind::source_file;
  }
  bool is_type_stub() const noexcept {
    return types_file_kind_ == types_file_kind::type_stub;
  }

  bool generate_abstract_types() const noexcept {
    return type_kind_ == type_kind::abstract;
  }
  bool generate_immutable_types() const noexcept {
    return type_kind_ == type_kind::immutable;
  }
  bool generate_mutable_types() const noexcept {
    return type_kind_ == type_kind::mutable_;
  }

  const std::string_view types_import_path() const {
    switch (type_kind_) {
      case type_kind::abstract:
        return "thrift_abstract_types";
      case type_kind::mutable_:
        return "thrift_mutable_types";
      case type_kind::immutable:
        return "thrift_types";
    }
  }

 private:
  bool enable_abstract_types_ = false;
  bool is_patch_file_ = false;
  types_file_kind types_file_kind_ = types_file_kind::not_a_types_file;
  type_kind type_kind_;
};

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
    const t_program* program,
    const std::optional<std::string>& root_module_prefix) {
  std::string prefix =
      !root_module_prefix.has_value() || root_module_prefix.value().empty()
      ? std::string("_fbthrift")
      : root_module_prefix.value();
  boost::algorithm::replace_all(prefix, ".", "__");
  return get_py3_namespace_with_name_and_prefix(program, prefix, "__");
}

/** Map of template values related to adapters attached to a node. */
whisker::object adapter_node(
    const whisker::prototype_database& proto,
    const t_named& self,
    const t_const* adapter_annotation) {
  if (adapter_annotation == nullptr) {
    return whisker::make::null;
  }
  auto type_hint = get_annotation_property(adapter_annotation, "typeHint");
  bool is_generic = type_hint.ends_with("[]");
  // For consts, we need to retain the `[]` suffix to indicate that the const is
  // a container type
  if (is_generic && dynamic_cast<const t_const*>(&self) == nullptr) {
    type_hint = type_hint.substr(0, type_hint.size() - 2);
  }
  whisker::map::raw node;
  node.emplace(
      "name",
      whisker::make::string(
          get_annotation_property(adapter_annotation, "name")));
  node.emplace("type_hint", whisker::make::string(type_hint));
  node.emplace("is_generic?", is_generic);

  const t_const* transitive_adapter_annotation =
      get_transitive_annotation_of_adapter_or_null(self);
  node.emplace(
      "transitive_annotation",
      proto.create_nullable<t_const_value>(
          transitive_adapter_annotation == nullptr
              ? nullptr
              : transitive_adapter_annotation->value()));

  return whisker::make::map(std::move(node));
}

bool is_invariant_adapter(
    const t_const* adapter_annotation, const t_type* true_type) {
  if (true_type->is<t_primitive_type>() || !adapter_annotation) {
    return false;
  }

  auto type_hint = get_annotation_property(adapter_annotation, "typeHint");
  return type_hint.ends_with("[]");
}

bool is_invariant_container_type(const t_type* type) {
  // Mapping is invariant in its key type
  // For example, if `Derived` extends `Base`,
  // then Mapping[Derived, Any] is not compatible with Mapping[Base, Any].
  // We first check if the map has an `Base` key type (for abstract types).
  // Then, we recursively verify whether the map's value type, or the element
  // type of a list or set, contains any such mapping incompatibility.
  const t_type* true_type = type->get_true_type();
  if (const t_map* map_type = true_type->try_as<t_map>()) {
    const t_type* key_type = map_type->key_type().deref().get_true_type();
    const t_type* val_type = map_type->val_type().deref().get_true_type();
    return key_type->is<t_structured>() || key_type->is<t_container>() ||
        is_invariant_adapter(
               find_structured_adapter_annotation(*key_type), key_type) ||
        is_invariant_container_type(val_type) ||
        is_invariant_adapter(
               find_structured_adapter_annotation(*val_type), val_type);
  } else if (const t_list* list = true_type->try_as<t_list>()) {
    return is_invariant_container_type(list->elem_type().get_type());
  } else if (const t_set* set = true_type->try_as<t_set>()) {
    return is_invariant_container_type(set->elem_type().get_type());
  }

  return false;
}

bool field_has_invariant_type(const t_field* field) {
  if (is_invariant_adapter(
          find_structured_adapter_annotation(*field),
          field->type()->get_true_type())) {
    return true;
  }

  if (is_invariant_adapter(
          find_structured_adapter_annotation(*field->type()->get_true_type()),
          field->type()->get_true_type())) {
    return true;
  }

  return ::apache::thrift::compiler::is_invariant_container_type(
      field->type().get_type());
}

/**
 * Integer corresponding to the Thrift IDL type of the field, as defined by
 * `enum BaseType`.
 *
 * This corresponds to the "true" IDL type (i.e., after resolving aliases) and
 * does not include type parameters (such as map key and values, container
 * element types, etc.).
 */
int field_idl_type(const t_field& field) {
  // Mapping from compiler implementation details to a public enum `BaseType`
  BaseType idl_type = std::invoke(
      [](const t_type* true_type) -> BaseType {
        if (const auto* primitive = true_type->try_as<t_primitive_type>()) {
          switch (primitive->primitive_type()) {
            case t_primitive_type::type::t_void:
              return BaseType::Void;
            case t_primitive_type::type::t_bool:
              return BaseType::Bool;
            case t_primitive_type::type::t_byte:
              return BaseType::Byte;
            case t_primitive_type::type::t_i16:
              return BaseType::I16;
            case t_primitive_type::type::t_i32:
              return BaseType::I32;
            case t_primitive_type::type::t_i64:
              return BaseType::I64;
            case t_primitive_type::type::t_float:
              return BaseType::Float;
            case t_primitive_type::type::t_double:
              return BaseType::Double;
            case t_primitive_type::type::t_string:
              return BaseType::String;
            case t_primitive_type::type::t_binary:
              return BaseType::Binary;
          }
        } else if (true_type->is<t_list>()) {
          return BaseType::List;
        } else if (true_type->is<t_set>()) {
          return BaseType::Set;
        } else if (true_type->is<t_map>()) {
          return BaseType::Map;
        } else if (true_type->is<t_enum>()) {
          return BaseType::Enum;
        } else if (true_type->is<t_structured>()) {
          return BaseType::Struct;
        }

        // AST types which are not valid to be a field type - e.g. service
        throw std::runtime_error(
            fmt::format(
                "Mapping Error: Failed to map type '{}' to 'BaseType'",
                true_type->get_full_name()));
      },
      field.type()->get_true_type());

  return static_cast<std::underlying_type_t<BaseType>>(idl_type);
}

bool service_has_any_streaming_types(const t_service* service) {
  return std::any_of(
      service->functions().begin(),
      service->functions().end(),
      [](const auto& fn_iter) -> bool {
        return fn_iter.sink_or_stream() || fn_iter.interaction();
      });
}

bool service_has_any_sink_types(const t_service* service) {
  return std::any_of(
      service->functions().begin(),
      service->functions().end(),
      [](const auto& fn_iter) -> bool { return fn_iter.sink(); });
}

bool service_has_any_bidi_types(const t_service* service) {
  return std::any_of(
      service->functions().begin(),
      service->functions().end(),
      [](const auto& fn_iter) -> bool {
        return fn_iter.is_bidirectional_stream();
      });
}

class python_mstch_program : public mstch_program {
 public:
  python_mstch_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:include_namespaces",
             &python_mstch_program::include_namespaces},
            {"program:adapter_modules", &python_mstch_program::adapter_modules},
            {"program:adapter_type_hint_modules",
             &python_mstch_program::adapter_type_hint_modules},
        });
    gather_included_program_namespaces();
    visit_types_for_services_and_interactions();
    visit_types_for_objects();
    visit_types_for_constants();
    visit_types_for_typedefs();
    visit_types_for_adapters();
  }

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
      a.emplace_back(
          mstch::map{
              {"included_module_path", it->ns},
              {"included_module_mangle", it->ns_mangle},
              {"has_services?", it->has_services},
              {"has_types?", it->has_types},
              {"is_patch?", it->is_patch},
              {"needed_by_patch?", it->needed_by_patch}});
    }
    return a;
  }

  mstch::node adapter_modules() { return module_path_array(adapter_modules_); }

  mstch::node adapter_type_hint_modules() {
    return module_path_array(adapter_type_hint_modules_);
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

  /**
   * In addition to typedefs that can result in a type's namespace
   * different from the current program, @internal.InjectMetadataFields
   * may also pull in a namespace that is not in the current program.
   * This function includes namespaces for any type whose program
   * is different from the current program.
   */
  void add_type_namespace(const t_type* type) {
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
        visit_type(field.type().get_type());
      }
      for (const t_field& field : get_elems(function.exceptions())) {
        visit_type(field.type().get_type());
      }
      if (const t_stream* stream = function.stream()) {
        if (const t_throws* exceptions = stream->exceptions()) {
          for (const auto& field : exceptions->fields()) {
            visit_type(field.type().get_type());
          }
        }
        if (!function.has_void_initial_response()) {
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
        visit_type(field.type().get_type());
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
    // See the description of add_type_namespace
    // for the reason for this call.
    add_type_namespace(true_type);
    if (const t_list* list = true_type->try_as<t_list>()) {
      visit_type_with_typedef(list->elem_type().get_type(), is_typedef);
    } else if (const t_set* set = true_type->try_as<t_set>()) {
      visit_type_with_typedef(set->elem_type().get_type(), is_typedef);
    } else if (const t_map* map = true_type->try_as<t_map>()) {
      visit_type_with_typedef(&map->key_type().deref(), is_typedef);
      visit_type_with_typedef(&map->val_type().deref(), is_typedef);
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
      const t_service* containing_service = nullptr)
      : mstch_service(s, ctx, pos, containing_service) {
    register_methods(
        this,
        {
            {"service:supported_functions",
             &python_mstch_service::supported_functions},
            {"service:supported_service_functions",
             &python_mstch_service::supported_service_functions},
        });
  }

  std::vector<const t_function*> get_supported_functions(
      const std::function<bool(const t_function*)>& func_filter) {
    std::vector<const t_function*> funcs;
    for (const auto& func : service_->functions()) {
      if (func_filter(&func)) {
        funcs.push_back(&func);
      }
    }
    return funcs;
  }

  mstch::node supported_functions() {
    return make_mstch_functions(
        get_supported_functions([](const t_function* func) -> bool {
          return !func->is_interaction_constructor();
        }));
  }

  mstch::node supported_service_functions() {
    return make_mstch_functions(
        get_supported_functions([](const t_function* func) -> bool {
          return !func->is_interaction_constructor();
        }));
  }
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

class python_mstch_struct : public mstch_struct {
 public:
  python_mstch_struct(
      const t_structured* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_struct(s, ctx, pos) {
    register_methods(
        this,
        {
            {"struct:fields_ordered_by_id",
             &python_mstch_struct::fields_ordered_by_id},
        });
  }

  mstch::node fields_ordered_by_id() {
    std::vector<const t_field*> fields = struct_->fields().copy();
    std::sort(fields.begin(), fields.end(), [](const auto* m, const auto* n) {
      return m->id() < n->id();
    });
    return make_mstch_fields(fields);
  }
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
  for (const t_enum_value& ev : enm.values()) {
    validate(&ev, ev.name(), ctx, predicate);
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

void validate_no_dict_as_key(sema_context& ctx, const t_field& f) {
  auto report_warning = [&ctx](const t_field& f) {
    ctx.report(
        f,
        "python-dict-as-key",
        diagnostic_level::warning,
        "Field `{}`: `map` is not a supported key type for `map` or `set` in thrift-python.",
        f.name());
  };
  const t_type* type = f.type()->get_true_type();
  if (const t_map* map = type->try_as<t_map>()) {
    const auto* key_type = map->key_type()->get_true_type();
    if (key_type->try_as<t_map>()) {
      report_warning(f);
    }
  } else if (const t_set* set = type->try_as<t_set>()) {
    const auto* elem_type = set->elem_type()->get_true_type();
    if (elem_type->try_as<t_map>()) {
      report_warning(f);
    }
  }
}

std::filesystem::path program_to_path(const t_program& prog) {
  auto package = get_py3_namespace(&prog);
  return fmt::format("{}", fmt::join(package, "/"));
}

// Shared base class for Python and Python Patch generator, to provide common
// Whisker prototype extensions.
class t_mstch_python_prototypes_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

 protected:
  std::shared_ptr<python_generator_context> python_context_;

  whisker::map::raw globals() const override {
    assert(python_context_ != nullptr);
    whisker::map::raw globals = t_mstch_generator::globals();
    globals["python"] = whisker::object(
        whisker::native_handle<python_generator_context>(
            python_context_, make_prototype_for_context()));
    globals["py_string_literal"] = whisker::dsl::make_function(
        "py_string_literal",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(
              python::to_python_string_literal(
                  ctx.argument<whisker::string>(0)));
        });
    return globals;
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_mstch_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    def.property("py3_enum_value_name", [](const t_const_value& self) {
      return self.is_enum() && self.get_enum_value() != nullptr
          ? whisker::make::string(
                python::get_py3_name_class_scope(
                    *self.get_enum_value(), self.get_enum()->name()))
          : whisker::make::null;
    });
    def.property("unicode_value", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_STRING
          ? whisker::make::string(
                get_escaped_string<nonascii_handling::no_escape>(
                    self.get_string()))
          : whisker::make::null;
    });

    return std::move(def).make();
  }

  prototype<t_enum>::ptr make_prototype_for_enum(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_enum(proto);
    auto def = whisker::dsl::prototype_builder<h_enum>::extends(base);

    def.property("metadata_path", [this](const t_enum& self) {
      return get_py3_namespace_with_name_and_prefix(
                 self.program(),
                 get_option("root_module_prefix").value_or("")) +
          ".thrift_enums";
    });
    def.property("flags?", [](const t_enum& self) {
      return self.has_unstructured_annotation("py3.flags") ||
          self.has_structured_annotation(kPythonFlagsUri);
    });

    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def = whisker::dsl::prototype_builder<h_field>::extends(base);

    def.property("py_name", [this](const t_field& self) {
      const std::string py_name = python::get_py3_name(self);
      if (py_name.starts_with("__") && !py_name.ends_with("__")) {
        const t_structured* parent = context().get_field_parent(&self);
        std::string class_name = parent == nullptr ? "" : parent->name();

        // Trim leading _ from class name, and use as prefix if non-empty
        if (size_t first_non_underscore = class_name.find_first_not_of('_');
            first_non_underscore < class_name.size()) {
          return "_" + class_name.substr(first_non_underscore) + py_name;
        }
      }
      return py_name;
    });
    def.property("idl_type", [](const t_field& self) {
      return whisker::make::i64(field_idl_type(self));
    });
    def.property(
        "tablebased_qualifier", [](const t_field& self) -> whisker::string {
          switch (self.qualifier()) {
            case t_field_qualifier::none:
            case t_field_qualifier::required:
              return "FieldQualifier.Unqualified";
            case t_field_qualifier::optional:
              return "FieldQualifier.Optional";
            case t_field_qualifier::terse:
              return "FieldQualifier.Terse";
            default:
              throw std::runtime_error("unknown qualifier");
          }
        });
    def.property("is_invariant_type?", [](const t_field& self) {
      return field_has_invariant_type(&self);
    });
    def.property("sorted_key_serialize?", [](const t_field& self) {
      return self.has_structured_annotation(kPythonSortSetOnSerializeUri) ||
          self.has_structured_annotation(kPythonKeySortMapOnSerializeUri);
    });

    return std::move(def).make();
  }

  prototype<t_function>::ptr make_prototype_for_function(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_function(proto);
    auto def = whisker::dsl::prototype_builder<h_function>::extends(base);

    def.property("returns_tuple?", [](const t_function& self) {
      return !self.has_void_initial_response() &&
          (self.sink_or_stream() || self.interaction());
    });
    def.property("early_client_return?", [](const t_function& self) {
      return !self.return_type()->is_void();
    });
    def.property("with_regular_response?", [](const t_function& self) {
      return !self.return_type()->is_void();
    });

    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);

    def.property("py_name", &python::get_py3_name);

    // NOTE: `t_type` has overrides for these adapter related properties, as it
    // handles checking for adapters along a chain of typedefs using the
    // `t_type` overload of `find_structured_adapter_annotation`
    def.property("has_adapter?", [](const t_named& self) {
      return find_structured_adapter_annotation(self) != nullptr;
    });
    def.property("adapter", [&proto](const t_named& self) {
      return adapter_node(
          proto, self, find_structured_adapter_annotation(self));
    });

    return std::move(def).make();
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);

    def.property("module_mangle", [this](const t_program& self) {
      return mangle_program_path(&self, get_option("root_module_prefix"));
    });
    def.property("module_path", [this](const t_program& self) {
      return get_py3_namespace_with_name_and_prefix(
          &self, get_option("root_module_prefix").value_or(""));
    });
    def.property("safe_patch?", [](const t_program& self) {
      return self.name().starts_with("gen_safe_patch_");
    });
    def.property("safe_patch_module_path", [this](const t_program& self) {
      std::string ns = get_py3_namespace_with_name_and_prefix(
          &self, get_option("root_module_prefix").value_or(""));
      // Change the namespace from "path.to.file" to
      // "path.to.gen_safe_patch_file"
      auto pos = ns.rfind('.');
      return fmt::format(
          "{}.gen_safe_patch_{}", ns.substr(0, pos), ns.substr(pos + 1));
    });
    def.property("py_deprecated_module_path", [](const t_program& self) {
      std::string module_path = self.get_namespace("py");
      return module_path.empty() ? self.name() : module_path;
    });
    def.property("py_asyncio_module_path", [](const t_program& self) {
      std::string module_path = self.get_namespace("py.asyncio");
      return module_path.empty() ? self.name() : module_path;
    });
    def.property("base_library_package", [this](const t_program&) {
      std::optional<std::string> option = get_option("base_library_package");
      return !option.has_value() || option->empty() ? "thrift.python"
                                                    : option.value();
    });
    def.property("root_module_prefix", [this](const t_program&) {
      std::optional<std::string> prefix = get_option("root_module_prefix");
      return !prefix.has_value() || prefix->empty() ? "" : prefix.value() + ".";
    });
    def.property("has_streaming_types?", [](const t_program& self) {
      return std::any_of(
                 self.services().begin(),
                 self.services().end(),
                 service_has_any_streaming_types) ||
          std::any_of(
                 self.interactions().begin(),
                 self.interactions().end(),
                 service_has_any_streaming_types);
    });
    def.property("has_sink_functions?", [](const t_program& self) {
      return std::any_of(
                 self.services().begin(),
                 self.services().end(),
                 service_has_any_sink_types) ||
          std::any_of(
                 self.interactions().begin(),
                 self.interactions().end(),
                 service_has_any_sink_types);
    });
    def.property("has_bidi_functions?", [](const t_program& self) {
      return std::any_of(
                 self.services().begin(),
                 self.services().end(),
                 service_has_any_bidi_types) ||
          std::any_of(
                 self.interactions().begin(),
                 self.interactions().end(),
                 service_has_any_bidi_types);
    });

    return std::move(def).make();
  }

  prototype<t_interface>::ptr make_prototype_for_interface(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interface(proto);
    auto def = whisker::dsl::prototype_builder<h_interface>::extends(base);

    def.property("external_program?", [this](const t_interface& self) {
      return get_program() != self.program();
    });

    return std::move(def).make();
  }

  prototype<t_structured>::ptr make_prototype_for_structured(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_structured(proto);
    auto def = whisker::dsl::prototype_builder<h_structured>::extends(base);

    def.property("has_invariant_field?", [](const t_structured& self) {
      return std::any_of(
          self.fields().begin(), self.fields().end(), [](const auto& field) {
            return field_has_invariant_type(&field);
          });
    });
    def.property("legacy_api?", [](const t_structured&) { return true; });
    def.property("num_fields", [](const t_structured& self) {
      return whisker::i64(self.fields().size());
    });
    def.property("allow_inheritance?", [](const t_structured& self) {
      // While inheritance is discouraged, there is limited support for py3
      // auto-migraters
      return self.has_structured_annotation(
          kPythonMigrationBlockingAllowInheritanceUri);
    });
    def.property("disable_field_caching?", [this](const t_structured& self) {
      return has_option("disable_field_cache") ||
          self.has_structured_annotation(kPythonDisableFieldCacheUri);
    });
    def.property("should_generate_patch?", [](const t_structured& self) {
      return should_generate_patch(&self);
    });

    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    def.property("program", [this, &proto](const t_type& self) {
      // Override default program getter for t_type with Python-specific
      // behaviour
      return proto.create<t_program>(*get_true_type_program(self));
    });
    def.property("module_mangle", [this](const t_type& self) {
      return mangle_program_path(
                 get_true_type_program(self), get_option("root_module_prefix"))
          .append(fmt::format("__{}", python_context_->types_import_path()));
    });
    def.property("module_name", [this](const t_type& self) {
      return get_py3_namespace_with_name_and_prefix(
                 get_true_type_program(self),
                 get_option("root_module_prefix").value_or(""))
          .append(fmt::format(".{}", python_context_->types_import_path()));
    });
    def.property("patch_module_path", [this](const t_type& self) {
      return get_py3_namespace_with_name_and_prefix(
                 get_true_type_program(self),
                 get_option("root_module_prefix").value_or(""))
          .append(".thrift_patch");
    });
    def.property("need_module_path?", [this](const t_type& self) {
      return !python_context_->is_types_file() ||
          get_true_type_program(self) != get_program();
    });
    def.property("need_patch_module_path?", [this](const t_type& self) {
      return !python_context_->is_patch_file() ||
          get_true_type_program(self) != get_program();
    });
    def.property("metadata_path", [this](const t_type& self) {
      return get_py3_namespace_with_name_and_prefix(
                 get_true_type_program(self),
                 get_option("root_module_prefix").value_or("")) +
          ".thrift_metadata";
    });
    def.property("py3_namespace", [this](const t_type& self) {
      std::ostringstream ss;
      for (const auto& path : get_py3_namespace(get_true_type_program(self))) {
        ss << path << ".";
      }
      return ss.str();
    });
    def.property("external_program?", [this](const t_type& self) {
      return get_true_type_program(self) != get_program();
    });
    def.property("integer?", [](const t_type& self) {
      const t_type& true_type = *self.get_true_type();
      return true_type.is_any_int() || true_type.is_byte();
    });
    def.property("iobuf?", [](const t_type& self) {
      return is_type_iobuf(self.get_true_type());
    });
    def.property("contains_patch?", [](const t_type& self) {
      return type_contains_patch(self.get_true_type());
    });
    def.property("has_adapter?", [](const t_type& self) {
      // Check for adapters on NON-RESOLVED type (i.e. including on typedefs)
      return find_structured_adapter_annotation(self) != nullptr;
    });
    def.property("adapter", [&proto](const t_type& self) {
      // Check for adapters on NON-RESOLVED type (i.e. including on typedefs)
      return adapter_node(
          proto, self, find_structured_adapter_annotation(self));
    });
    def.property("legacy_float_behavior?", [](const t_type& self) {
      return self.has_structured_annotation(
          kPythonEnableUnsafeUnconstrainedFloat32);
    });

    return std::move(def).make();
  }

  void initialize_context(context_visitor& visitor) override {
    // Fix fields with mismatched empty const containers
    visitor.add_field_visitor([](const t_field& node) {
      const t_const_value* value = node.default_value();
      if (value != nullptr && value->is_empty()) {
        const t_type& true_type = *node.type()->get_true_type();
        if (value->kind() == t_const_value::CV_MAP &&
            (true_type.is<t_list>() || true_type.is<t_set>())) {
          const_cast<t_const_value*>(value)->convert_empty_map_to_list();
        } else if (
            value->kind() == t_const_value::CV_LIST && true_type.is<t_map>()) {
          const_cast<t_const_value*>(value)->convert_empty_list_to_map();
        }
      }
    });
  }

 private:
  prototype<python_generator_context>::ptr make_prototype_for_context() const {
    whisker::dsl::prototype_builder<
        whisker::native_handle<python_generator_context>>
        ctx;

    ctx.property(
        "is_types_file?", mem_fn(&python_generator_context::is_types_file));
    ctx.property(
        "is_source_file?", mem_fn(&python_generator_context::is_source_file));
    ctx.property(
        "is_type_stub?", mem_fn(&python_generator_context::is_type_stub));
    ctx.property(
        "generate_abstract_types?",
        mem_fn(&python_generator_context::generate_abstract_types));
    ctx.property(
        "generate_mutable_types?",
        mem_fn(&python_generator_context::generate_mutable_types));
    ctx.property(
        "generate_immutable_types?",
        mem_fn(&python_generator_context::generate_immutable_types));
    ctx.property(
        "enable_abstract_types?",
        mem_fn(&python_generator_context::enable_abstract_types));

    return std::move(ctx).make();
  }

  const t_program* get_true_type_program(const t_type& type) const {
    if (const t_program* p = type.get_true_type()->program()) {
      return p;
    }
    // t_primitive_type instances are singletons and their program property is
    // always nullptr, but we can treat them as locally defined (e.g. don't
    // require imports/qualification)
    return get_program();
  }
};

class t_mstch_python_generator : public t_mstch_python_prototypes_generator {
 public:
  using t_mstch_python_prototypes_generator::
      t_mstch_python_prototypes_generator;

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
    validator.add_field_visitor(validate_no_dict_as_key);
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

 protected:
  void set_mstch_factories();
  void generate_file(
      const std::string& template_name,
      types_file_kind types_file_kind,
      type_kind type_kind,
      const std::filesystem::path& base);
  void set_types_file(bool val);
  void generate_types();
  void generate_metadata();
  void generate_clients();
  void generate_services();

  void initialize_context(context_visitor& visitor) override {
    t_mstch_python_prototypes_generator::initialize_context(visitor);
    python_context_ = std::make_shared<python_generator_context>(
        /*is_patch_file=*/false, type_kind::abstract);
  }

  std::filesystem::path generate_root_path_;
};

void t_mstch_python_generator::set_mstch_factories() {
  mstch_context_.add<python_mstch_program>();
  mstch_context_.add<python_mstch_service>();
  mstch_context_.add<python_mstch_struct>();
}

void t_mstch_python_generator::generate_file(
    const std::string& template_name,
    types_file_kind types_file_kind,
    type_kind type_kind,
    const std::filesystem::path& base = {}) {
  t_program* program = get_program();
  const std::string& program_name = program->name();
  python_context_->reset(types_file_kind, type_kind);

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
  python_context_->set_enable_abstract_types(
      !has_option("disable_abstract_types"));

  generate_file(
      "thrift_types.py",
      types_file_kind::source_file,
      type_kind::immutable,
      generate_root_path_);
  generate_file(
      "thrift_types.pyi",
      types_file_kind::type_stub,
      type_kind::immutable,
      generate_root_path_);
  generate_file(
      "thrift_enums.py",
      types_file_kind::source_file,
      type_kind::immutable,
      generate_root_path_);

  generate_file(
      "thrift_abstract_types.py",
      types_file_kind::source_file,
      type_kind::abstract,
      generate_root_path_);

  generate_file(
      "thrift_mutable_types.py",
      types_file_kind::source_file,
      type_kind::mutable_,
      generate_root_path_);

  generate_file(
      "thrift_mutable_types.pyi",
      types_file_kind::type_stub,
      type_kind::mutable_,
      generate_root_path_);

  python_context_->set_enable_abstract_types(true);
}

void t_mstch_python_generator::generate_metadata() {
  generate_file(
      "thrift_metadata.py",
      types_file_kind::source_file,
      type_kind::immutable,
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
      types_file_kind::not_a_types_file,
      type_kind::immutable,
      generate_root_path_);

  generate_file(
      "thrift_mutable_clients.py",
      types_file_kind::not_a_types_file,
      type_kind::mutable_,
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
      types_file_kind::not_a_types_file,
      type_kind::immutable,
      generate_root_path_);

  generate_file(
      "thrift_mutable_services.py",
      types_file_kind::not_a_types_file,
      type_kind::mutable_,
      generate_root_path_);
}

class t_python_patch_generator : public t_mstch_python_prototypes_generator {
 public:
  using t_mstch_python_prototypes_generator::
      t_mstch_python_prototypes_generator;

  std::string template_prefix() const override { return "patch"; }

  void generate_program() override {
    out_dir_base_ = "gen-python-patch";

    set_mstch_factories();
    const auto* program = get_program();
    auto mstch_program = mstch_context_.program_factory->make_mstch_object(
        program, mstch_context_);

    render_to_file(
        std::move(mstch_program),
        "thrift_patch.py",
        program_to_path(*get_program()) / program->name() / "thrift_patch.py");
  }

 protected:
  void initialize_context(context_visitor& visitor) override {
    t_mstch_python_prototypes_generator::initialize_context(visitor);
    python_context_ = std::make_shared<python_generator_context>(
        /*is_patch_file=*/true, type_kind::immutable);
  }

 private:
  void set_mstch_factories() {
    mstch_context_.add<python_mstch_program>();
    mstch_context_.add<python_mstch_struct>();
  }
};

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_python,
    "Python",
    "    include_prefix:  Use full include paths in generated files.\n"
    "    disable_abstract_types:\n"
    "      Disable the use of abstract types with thrift-python"
    "      immutable and mutable types.\n"
    "    does_not_have_py_deprecated:\n"
    "      Specify that the generated code does not have thrift-py-deprecated.\n"
    "    does_not_have_py_deprecated_asyncio:\n"
    "      Specify that the generated code does not have thrift-py-deprecated-asyncio.\n");

namespace patch {
THRIFT_REGISTER_GENERATOR(
    python_patch, "Python patch", "Python patch generator\n");
}

} // namespace apache::thrift::compiler
