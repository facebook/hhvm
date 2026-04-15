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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fmt/format.h>

#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>
#include <thrift/compiler/generate/cpp/reference_type.h>
#include <thrift/compiler/generate/cpp/util.h>
#include <thrift/compiler/generate/python/util.h>
#include <thrift/compiler/generate/t_whisker_generator.h>
#include <thrift/compiler/generate/templates.h>

namespace apache::thrift::compiler {

namespace {

std::vector<const t_function*> lifecycleFunctions() {
  static t_function onStartServing_{
      nullptr, t_primitive_type::t_void(), "onStartServing"};
  static t_function onStopRequested_{
      nullptr, t_primitive_type::t_void(), "onStopRequested"};

  return {&onStartServing_, &onStopRequested_};
}

whisker::object to_whisker_string_array(
    const std::vector<std::string>& values) {
  whisker::array::raw arr;
  arr.reserve(values.size());
  for (const std::string& s : values) {
    arr.emplace_back(whisker::make::string(s));
  }
  return whisker::make::array(std::move(arr));
}

// TO-DO: remove duplicate in pyi
bool has_types(const t_program* program) {
  assert(program != nullptr);

  return !(
      program->structured_definitions().empty() && program->enums().empty() &&
      program->typedefs().empty() && program->consts().empty());
}

std::vector<std::string> get_py3_namespace_with_name(const t_program* program) {
  auto ns = get_py3_namespace(program);
  ns.push_back(program->name());
  return ns;
}

bool is_number(const t_type& resolved) {
  return resolved.is_any_int() || resolved.is_byte() ||
      resolved.is_floating_point();
}

bool is_iobuf(const std::string& cpp_type) {
  return cpp_type == "folly::IOBuf";
}

bool is_iobuf_ref(const std::string& cpp_type) {
  return cpp_type == "std::unique_ptr<folly::IOBuf>";
}

bool is_flexible_binary(const t_type& resolved, const std::string& cpp_type) {
  return resolved.is_binary() && !cpp_type.empty() && !is_iobuf(cpp_type) &&
      !is_iobuf_ref(cpp_type) &&
      // We know that folly::fbstring is completely substitutable for
      // std::string and it's a common-enough type to special-case:
      cpp_type != "folly::fbstring" && cpp_type != "::folly::fbstring";
}

bool is_custom_binary_type(
    const t_type& resolved, const std::string& cpp_type) {
  return is_iobuf(cpp_type) || is_iobuf_ref(cpp_type) ||
      is_flexible_binary(resolved, cpp_type);
}

bool type_needs_convert(const t_type* type) {
  return type->is<t_structured>() || type->is<t_container>();
}

bool container_needs_convert(const t_type* type) {
  const t_type* true_type = type->get_true_type();

  if (const t_map* map_type = dynamic_cast<const t_map*>(true_type)) {
    return container_needs_convert(&map_type->key_type().deref()) ||
        container_needs_convert(&map_type->val_type().deref());
  } else if (const t_list* list_type = dynamic_cast<const t_list*>(true_type)) {
    return container_needs_convert(list_type->elem_type().get_type());
  } else if (const t_set* set_type = dynamic_cast<const t_set*>(true_type)) {
    return container_needs_convert(set_type->elem_type().get_type());
  } else if (true_type->is<t_structured>()) {
    return true;
  }
  return false;
}

std::string get_cpp_template(const t_type& type) {
  if (const auto* val = cpp_name_resolver::find_template(type)) {
    return *val;
  }
  const auto* resolved = type.get_true_type();
  if (resolved->is<t_list>()) {
    return "std::vector";
  }
  if (resolved->is<t_set>()) {
    return "std::set";
  }
  if (resolved->is<t_map>()) {
    return "std::map";
  }

  return {};
}

bool is_hidden(const t_named& node) {
  return node.has_unstructured_annotation("py3.hidden") ||
      node.has_structured_annotation(kPythonPy3HiddenUri) ||
      node.uri() == kScopeEnumUri;
}
bool is_hidden(const t_typedef& node) {
  return node.generated() || node.has_unstructured_annotation("py3.hidden") ||
      node.has_structured_annotation(kPythonPy3HiddenUri) ||
      node.uri() == kScopeEnumUri || is_hidden(*node.get_true_type());
}
bool is_hidden(const t_type& node) {
  return node.generated() || node.has_unstructured_annotation("py3.hidden") ||
      node.has_structured_annotation(kPythonPy3HiddenUri) ||
      node.uri() == kScopeEnumUri ||
      cpp_name_resolver::is_directly_adapted(node);
}

bool is_func_supported(bool no_stream, const t_function* func) {
  return !is_hidden(*func) && !(no_stream && func->stream()) && !func->sink() &&
      !func->is_interaction_constructor();
}

enum class field_cpp_kind : uint8_t {
  /** Plain value or boxed ref with no special handling */
  value,
  unique_ptr,
  shared_ptr_mutable,
  shared_ptr_const,
  iobuf,
};

bool field_has_default_value(
    const t_field& field, const field_cpp_kind cpp_kind) {
  return cpp_kind == field_cpp_kind::value &&
      (field.default_value() != nullptr ||
       field.qualifier() != t_field_qualifier::optional);
}

enum class FileType { CBindingsFile, TypesFile, NotTypesFile };

/** Metadata for a program included from the main program being generated */
struct py3_included_program {
  const t_program* program;
  bool importServices;
};

struct py3_structured_context {
  /** Whether any fields are marked as hidden */
  bool hasHiddenFields{false};
  /** Whether any fields have a default value */
  bool hasDefaultedFields{false};
  /** Fields not marked as `py3.hidden` / `@python.Py3Hidden` */
  std::vector<const t_field*> nonHiddenFields;
};

class py3_generator_context {
 public:
  using cached_type_properties =
      apache::thrift::compiler::python::cached_properties;

  py3_generator_context(
      const t_program* program,
      const std::map<std::string, std::string, std::less<>>& compiler_options)
      : root_program_(program), compiler_options_(compiler_options) {}

  cached_type_properties& get_cached_type_props(
      const t_type* type, const t_field* field = nullptr) const;

  field_cpp_kind get_field_cpp_kind(const t_field& field) const {
    assert(field_cpp_kinds_.contains(&field));
    return field_cpp_kinds_.at(&field);
  }

  bool has_field_cpp_type_annotation(const t_field& field) const {
    return field_cpp_type_annotations_.contains(&field);
  }
  // Data for generating ctypedefs for shared types (primitives, structs, enums)
  // with field-level @cpp.Type. These can't go in custom_cpp_types_ because the
  // shared type pointer would poison container child type rendering.
  struct field_custom_ctypedef {
    const t_type* type; // underlying shared type (for base cython type)
    std::string cython_name; // e.g., "std_uint64_t"
    std::string cpp_type; // e.g., "std::uint64_t"
    bool flexible_binary; // binary with non-IOBuf/fbstring custom type
  };

  const py3_structured_context& get_structured_context(
      const t_structured& structured) const {
    assert(structured_contexts_.contains(&structured));
    return structured_contexts_.at(&structured);
  }

  const std::map<std::string, py3_included_program>& included_programs(
      const t_program& program) const {
    check_root_program(program);
    return included_programs_;
  }

  const std::vector<const t_type*>& container_types(
      const t_program& program) const {
    check_root_program(program);
    return container_types_;
  }

  const std::vector<const t_type*>& custom_templates(
      const t_program& program) const {
    check_root_program(program);
    return custom_templates_;
  }

  const std::vector<const t_type*>& custom_cpp_types(
      const t_program& program) const {
    check_root_program(program);
    return custom_cpp_types_;
  }

  const std::vector<field_custom_ctypedef>& field_custom_ctypedefs(
      const t_program& program) const {
    check_root_program(program);
    return field_custom_ctypedefs_;
  }

  const std::map<std::tuple<std::string, bool>, const t_function*>&
  unique_functions_by_return_type(const t_program& program) const {
    check_root_program(program);
    return unique_functions_by_return_type_;
  }

  const std::map<std::string, const t_type*>& stream_types(
      const t_program& program) const {
    check_root_program(program);
    return stream_types_;
  }

  const std::map<std::string, const t_type*>& stream_exceptions(
      const t_program& program) const {
    check_root_program(program);
    return stream_exceptions_;
  }

  const std::vector<const t_function*>& response_and_stream_functions(
      const t_program& program) const {
    check_root_program(program);
    return response_and_stream_functions_;
  }

  const std::vector<const t_interaction*>& supported_interactions(
      const t_service& service) const {
    static const std::vector<const t_interaction*> kEmpty;
    check_root_program(*service.program());
    const auto& it = supported_interactions_by_service_.find(&service);
    return it == supported_interactions_by_service_.end() ? kEmpty : it->second;
  }

  void visit_function(const t_function& func);

  void register_visitors(t_whisker_generator::context_visitor& visitor) {
    using context = t_whisker_generator::whisker_generator_visitor_context;
    visitor.add_program_visitor([this](const context&, const t_program& p) {
      if (&p == root_program_) {
        for (const t_program* included_program : p.get_includes_for_codegen()) {
          included_programs_[included_program->path()] = py3_included_program{
              .program = included_program, .importServices = true};
        }
      }
    });
    visitor.add_structured_definition_visitor(
        [this](const context& ctx, const t_structured& node) {
          if (&ctx.program() != root_program_ || is_hidden(node)) {
            return;
          }
          py3_structured_context& node_ctx = structured_contexts_[&node];
          node_ctx.nonHiddenFields.reserve(node.fields().size());
          for (const t_field& field : node.fields()) {
            if (is_hidden(field)) {
              node_ctx.hasHiddenFields = true;
            } else {
              node_ctx.nonHiddenFields.emplace_back(&field);
              node_ctx.hasDefaultedFields |= field.default_value() != nullptr;
            }
          }
        });
    visitor.add_field_visitor([this](const context& ctx, const t_field& field) {
      if (&ctx.program() != root_program_ || is_hidden(field) ||
          is_hidden(static_cast<const t_structured&>(*ctx.parent()))) {
        return;
      }
      const t_type* visit_node = &field.type().deref();
      if (auto* annot = field.find_structured_annotation_or_null(kCppTypeUri)) {
        field_cpp_type_annotations_[&field] = annot;
      }
      visit_type(visit_node, /*fromTypeDef=*/false, /*field=*/&field);
      switch (gen::cpp::find_ref_type(field)) {
        case gen::cpp::reference_type::unique: {
          field_cpp_kinds_[&field] = field_cpp_kind::unique_ptr;
          return;
        }
        case gen::cpp::reference_type::shared_const: {
          field_cpp_kinds_[&field] = field_cpp_kind::shared_ptr_const;
          return;
        }
        case gen::cpp::reference_type::shared_mutable: {
          field_cpp_kinds_[&field] = field_cpp_kind::shared_ptr_mutable;
          return;
        }
        case gen::cpp::reference_type::boxed_intern:
        case gen::cpp::reference_type::boxed: {
          field_cpp_kinds_[&field] = field_cpp_kind::value;
          return;
        }
        case gen::cpp::reference_type::none: {
          field_cpp_kinds_[&field] = cpp2::is_binary_iobuf_unique_ptr(field)
              ? field_cpp_kind::iobuf
              : field_cpp_kind::value;
          return;
        }
        default:
          throw std::logic_error{"Unhandled ref_type"};
      }
    });
    visitor.add_function_visitor(
        [this](const context& ctx, const t_function& func) {
          if (&ctx.program() != root_program_ || is_hidden(func)) {
            return;
          }
          // Visit for populating program-level type information
          visit_function(func);
          // Collect supported interactions
          if (func.is_interaction_constructor()) {
            if (const auto* service =
                    dynamic_cast<const t_service*>(ctx.parent())) {
              supported_interactions_by_service_[service].emplace_back(
                  &func.interaction()->as<t_interaction>());
            }
          }
        });
    visitor.add_const_visitor(
        [this](const context& ctx, const t_const& constant) {
          if (&ctx.program() == root_program_) {
            visit_type(&constant.type_ref().deref(), /*fromTypeDef=*/false);
          }
        });
    visitor.add_typedef_visitor(
        [this](const context& ctx, const t_typedef& td) {
          if (&ctx.program() == root_program_ && !is_hidden(td) &&
              !is_hidden(*td.get_true_type())) {
            visit_type(&td, /*fromTypeDef=*/true);
          }
        });
  }

 private:
  const t_program* root_program_;
  const std::map<std::string, std::string, std::less<>>& compiler_options_;

  // Computed properties for the root program
  std::map<std::string, py3_included_program> included_programs_;
  std::unordered_map<const t_structured*, py3_structured_context>
      structured_contexts_;
  std::unordered_map<const t_field*, field_cpp_kind> field_cpp_kinds_;

  std::vector<const t_type*> container_types_;
  std::vector<const t_type*> custom_templates_;
  std::vector<const t_type*> custom_cpp_types_;
  std::unordered_set<std::string> seen_type_names_;
  std::map<std::tuple<std::string, bool>, const t_function*>
      unique_functions_by_return_type_;
  std::map<std::string, const t_type*> stream_types_;
  std::map<std::string, const t_type*> stream_exceptions_;
  // Functions with a stream and an initial response.
  std::vector<const t_function*> response_and_stream_functions_;

  std::unordered_map<const t_service*, std::vector<const t_interaction*>>
      supported_interactions_by_service_;

  // Maps fields with @cpp.Type to their annotation.
  std::unordered_map<const t_field*, const t_const*>
      field_cpp_type_annotations_;

  // These properties are mutable as they are (or contain) caches which must be
  // accessed from a const method context
  mutable cpp_name_resolver name_resolver_;
  mutable std::unordered_map<const t_type*, cached_type_properties>
      type_properties_;
  // For shared types (primitives, structs, enums) with field-level @cpp.Type,
  // the type pointer is shared across fields and can't serve as a unique cache
  // key. field_type_properties_ is keyed by field for field rendering context.
  mutable std::unordered_map<const t_field*, cached_type_properties>
      field_type_properties_;
  // ctypedefs for shared types with field-level @cpp.Type. Stored separately
  // from custom_cpp_types_ to avoid poisoning container child type rendering.
  std::vector<field_custom_ctypedef> field_custom_ctypedefs_;

  void check_root_program(const t_program& program) const {
    if (&program != root_program_) {
      throw whisker::eval_error(
          "This property is only implemented for the root program");
    }
  }

  std::string visit_type(
      const t_type* orig_type,
      bool fromTypeDef,
      const t_field* field = nullptr);

  void add_typedef_namespace(const t_type* type) {
    const t_program* prog = type->program();
    if (prog != nullptr && prog != root_program_ &&
        !included_programs_.contains(prog->path())) {
      included_programs_[prog->path()] =
          py3_included_program{.program = prog, .importServices = false};
    }
  }

  void add_function_by_unique_return_type(
      const t_function& function, std::string return_type_name) {
    unique_functions_by_return_type_.insert(
        {{std::move(return_type_name),
          cpp2::is_stack_arguments(compiler_options_, function)},
         &function});
  }
};

std::string py3_generator_context::visit_type(
    const t_type* orig_type, bool fromTypeDef, const t_field* field) {
  bool hasPy3EnableCppAdapterAnnot =
      orig_type->has_structured_annotation(kPythonPy3EnableCppAdapterUri);
  auto trueType = orig_type->get_true_type();
  py3_generator_context::cached_type_properties& props =
      get_cached_type_props(orig_type, field);
  const std::string& flatName = props.flat_name();
  // Import all types either beneath a typedef, even if the current type is
  // not directly a typedef
  fromTypeDef = fromTypeDef || orig_type->is<t_typedef>();
  if (flatName.empty()) {
    // Recursive calls pass nullptr for field — child types should not
    // inherit the field's @cpp.Type annotation.
    std::string extra;
    if (const t_list* list = trueType->try_as<t_list>()) {
      extra = fmt::format(
          "List__{}", visit_type(&list->elem_type().deref(), fromTypeDef));
    } else if (const t_set* set = trueType->try_as<t_set>()) {
      extra = fmt::format(
          "Set__{}", visit_type(&set->elem_type().deref(), fromTypeDef));
    } else if (const t_map* map = trueType->try_as<t_map>()) {
      extra = fmt::format(
          "Map__{}_{}",
          visit_type(&map->key_type().deref(), fromTypeDef),
          visit_type(&map->val_type().deref(), fromTypeDef));
    } else if (trueType->is_binary()) {
      extra = "binary";
    } else {
      extra = trueType->name();
    }
    props.set_flat_name(root_program_, trueType, extra);
  }
  assert(!flatName.empty());
  // For typedef-wrapping-containers with field-level annotations, the
  // properties live in field_type_properties_ (keyed by field). Propagate
  // to the type-keyed cache (first field wins via emplace) so that
  // program-level rendering from container_types_ / custom_templates_ /
  // custom_cpp_types_ can find the properties without a field parameter.
  if (field && orig_type->is<t_typedef>() && trueType->is<t_container>() &&
      field_cpp_type_annotations_.count(field)) {
    type_properties_.emplace(orig_type, props);
  }
  // If this type or a parent of this type is a typedef,
  // then add the namespace of the *resolved* type:
  // (parent matters if you have eg. typedef list<list<type>>)
  if (fromTypeDef) {
    add_typedef_namespace(trueType);
  }
  // For shared types (primitives, structs, enums) with field-level @cpp.Type,
  // store ctypedef info separately. Using custom_cpp_types_ with a shared type
  // pointer would poison container child type rendering (the same pointer
  // is used for unrelated container key/val/elem types).
  const bool is_shared_field_annot = field &&
      field_cpp_type_annotations_.count(field) &&
      !orig_type->is<t_container>() &&
      !(orig_type->is<t_typedef>() && trueType->is<t_container>());
  if (seen_type_names_.insert(flatName).second) {
    // When the type has a non-default template from a field-level annotation
    // and orig_type is a typedef, use orig_type in the type lists so that
    // rendering lookups via get_cached_type_props find the correct entry
    // (which is keyed by the typedef node, not the container true_type).
    const bool use_orig_for_field_annot =
        orig_type != trueType && !props.is_default_template(trueType);
    if (trueType->is<t_container>()) {
      container_types_.push_back(
          (hasPy3EnableCppAdapterAnnot || use_orig_for_field_annot) ? orig_type
                                                                    : trueType);
    }
    if (!props.is_default_template(trueType)) {
      custom_templates_.push_back(
          use_orig_for_field_annot ? orig_type : trueType);
    }
    if (!props.cpp_type().empty()) {
      if (is_shared_field_annot) {
        // Generate ctypedef via separate template section to avoid
        // poisoning container child type rendering.
        if (!is_iobuf(props.cpp_type()) && !is_iobuf_ref(props.cpp_type())) {
          field_custom_ctypedefs_.push_back(
              field_custom_ctypedef{
                  orig_type,
                  props.to_cython_type(),
                  props.cpp_type(),
                  is_flexible_binary(*trueType, props.cpp_type())});
        }
      } else {
        custom_cpp_types_.push_back(
            (!hasPy3EnableCppAdapterAnnot && trueType->is<t_container>() &&
             !use_orig_for_field_annot)
                ? trueType
                : orig_type);
      }
    }
  }
  return flatName;
}

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

// Generator-specific validator that enforces "name" and "value" are not used
// as enum member or union field names (thrift-py3).
namespace enum_member_union_field_names_validator {
void validate(const t_named& node, const std::string& name, sema_context& ctx) {
  auto pyname = node.get_unstructured_annotation("py3.name", &name);
  if (const t_const* annot =
          node.find_structured_annotation_or_null(kPythonNameUri)) {
    if (auto annotation_name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      pyname = annotation_name->get_string();
    }
  }
  if (pyname == "name" || pyname == "value") {
    ctx.report(
        node,
        "enum-member-union-field-names-rule",
        diagnostic_level::error,
        "'{}' should not be used as an enum/union field name in thrift-py3. "
        "Use a different name or annotate the field with "
        "`(py3.name=\"<new_py_name>\")`",
        pyname);
  }
}
bool validate_enum(sema_context& ctx, const t_enum& enm) {
  for (const t_enum_value& ev : enm.values()) {
    validate(ev, ev.name(), ctx);
  }
  return true;
}

bool validate_union(sema_context& ctx, const t_union& s) {
  for (const t_field& f : s.fields()) {
    validate(f, f.name(), ctx);
  }
  return true;
}

} // namespace enum_member_union_field_names_validator

void py3_generator_context::visit_function(const t_function& function) {
  for (const auto& field : function.params().fields()) {
    visit_type(&field.type().deref(), /*fromTypeDef=*/false);
  }
  const t_stream* stream = function.stream();
  if (const t_throws* exceptions = stream ? stream->exceptions() : nullptr) {
    for (const t_field& field : exceptions->fields()) {
      const t_type* exType = &field.type().deref();
      stream_exceptions_.emplace(
          visit_type(&field.type().deref(), /*fromTypeDef=*/false), exType);
    }
  }
  for (const t_field& field : get_elems(function.exceptions())) {
    visit_type(&field.type().deref(), /*fromTypeDef=*/false);
  }

  std::string return_type_name;
  if (stream && !function.is_interaction_constructor()) {
    return_type_name = "Stream__";
    const t_type* elem_type = &stream->elem_type().deref();
    if (!function.has_void_initial_response()) {
      return_type_name = fmt::format(
          "ResponseAndStream__{}_",
          visit_type(&function.return_type().deref(), /*fromTypeDef=*/false));
    }
    std::string elem_type_name = visit_type(elem_type, /*fromTypeDef=*/false);
    return_type_name += elem_type_name;
    stream_types_.emplace(elem_type_name, elem_type);
    if (seen_type_names_.insert(return_type_name).second &&
        !function.has_void_initial_response()) {
      response_and_stream_functions_.push_back(&function);
    }
  } else if (!function.sink()) {
    const t_type_ref& type = function.is_interaction_constructor()
        ? function.interaction()
        : function.return_type();
    return_type_name = visit_type(&type.deref(), /*fromTypeDef=*/false);
  }
  add_function_by_unique_return_type(function, std::move(return_type_name));
}

class t_mstch_py3_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  void generate_program() override {
    generateRootPath_ = package_to_path();
    out_dir_base_ = "gen-py3";
    if (std::string_view include_prefix =
            get_compiler_option("include_prefix").value_or("");
        !include_prefix.empty()) {
      program_->set_include_prefix(std::string(include_prefix));
    }
    generate_init_files();
    generate_types();
    generate_services();
  }

  void fill_validator_visitors(ast_validator& validator) const override {
    validator.add_program_visitor(validate_no_reserved_key_in_namespace);
    validator.add_enum_visitor(
        enum_member_union_field_names_validator::validate_enum);
    validator.add_union_visitor(
        enum_member_union_field_names_validator::validate_union);
  }

 private:
  std::string template_prefix() const final { return "py3"; }

  whisker::source_manager template_source_manager() const final {
    return whisker::source_manager{
        std::make_unique<in_memory_source_manager_backend>(
            create_templates_by_path())};
  }

  void generate_init_files();
  void generate_whisker_file(
      const std::string& template_name,
      FileType file_type,
      const std::filesystem::path& base);
  void generate_types();
  void generate_services();
  std::filesystem::path package_to_path();

  std::filesystem::path generateRootPath_;
  FileType file_type_ = FileType::NotTypesFile;
  std::unique_ptr<py3_generator_context> context_;

  void initialize_context(t_whisker_generator::context_visitor& visitor) final {
    context_ =
        std::make_unique<py3_generator_context>(program_, compiler_options());
    context_->register_visitors(visitor);
    // Lifecycle functions aren't actually part of the program's AST, so we have
    // to visit them manually.
    for (const t_function* func : lifecycleFunctions()) {
      context_->visit_function(*func);
    }
  }

  whisker::map::raw globals(prototype_database& proto) const override {
    whisker::map::raw globals = t_whisker_generator::globals(proto);
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

  const t_program* get_true_type_program(const t_type& type) const {
    const t_type* resolved = type.get_true_type();
    return resolved->program() == nullptr ? program_ : resolved->program();
  }

  void define_additional_prototypes(prototype_database& proto) const override {
    proto.define(make_prototype_for_included_program(proto));
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    def.property("py3_enum_value_name", [](const t_const_value& self) {
      if (!self.is_enum() || self.get_enum_value() == nullptr) {
        return whisker::make::null;
      }
      const auto& enum_name = self.get_enum()->name();
      return whisker::make::string(
          python::get_py3_name_class_scope(*self.get_enum_value(), enum_name));
    });
    /*
     * Use this function (instead of the version used by C++) to render unicode
     * strings, i.e., normal python strings "".
     * For binary bytes b"", use string_value, which has octal escapes for
     * unicode characters.
     */
    def.property("unicode_value", [](const t_const_value& self) {
      return self.kind() == t_const_value::CV_STRING
          ? get_escaped_string<nonascii_handling::no_escape>(self.get_string())
          : "";
    });

    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def =
        whisker::dsl::prototype_builder<h_field>::extends(std::move(base));

    // Field-level type properties that resolve @cpp.Type annotations from
    // the field, not just the type. These are used by templates to access
    // field-aware type information without relying on mutable state.
    auto get_field_type_props = [this](const t_field& self)
        -> const py3_generator_context::cached_type_properties& {
      return context_->get_cached_type_props(&self.type().deref(), &self);
    };
    def.property("has_field_cpp_type?", [this](const t_field& self) {
      return context_->has_field_cpp_type_annotation(self);
    });
    def.property("type_iobuf?", [get_field_type_props](const t_field& self) {
      return is_iobuf(get_field_type_props(self).cpp_type());
    });
    def.property("type_iobufRef?", [get_field_type_props](const t_field& self) {
      return is_iobuf_ref(get_field_type_props(self).cpp_type());
    });
    def.property(
        "type_iobufWrapper?", [get_field_type_props](const t_field& self) {
          const std::string& cpp_type = get_field_type_props(self).cpp_type();
          return is_iobuf(cpp_type) || is_iobuf_ref(cpp_type);
        });
    def.property(
        "type_flexibleBinary?", [get_field_type_props](const t_field& self) {
          return is_flexible_binary(
              *self.type()->get_true_type(),
              get_field_type_props(self).cpp_type());
        });
    def.property(
        "type_customCppType?", [get_field_type_props](const t_field& self) {
          return !get_field_type_props(self).cpp_type().empty();
        });
    def.property(
        "type_customCppType", [get_field_type_props](const t_field& self) {
          return get_field_type_props(self).cpp_type();
        });
    def.property(
        "type_cythonCustomType", [get_field_type_props](const t_field& self) {
          return get_field_type_props(self).to_cython_type();
        });
    def.property(
        "type_cppTemplate", [get_field_type_props](const t_field& self) {
          return get_field_type_props(self).cpp_template();
        });
    def.property("type_flat_name", [get_field_type_props](const t_field& self) {
      return get_field_type_props(self).flat_name();
    });
    def.property(
        "type_need_cbinding_path?",
        [this, get_field_type_props](const t_field& self) {
          const t_program* type_program;
          if (!get_field_type_props(self).cpp_type().empty() &&
              !self.type()->get_true_type()->is<t_container>()) {
            type_program = self.type().deref().program();
            if (type_program == nullptr) {
              type_program = program_;
            }
          } else {
            type_program = get_true_type_program(self.type().deref());
          }
          return file_type_ != FileType::CBindingsFile ||
              type_program != get_program();
        });

    def.property("reference?", [this](const t_field& self) {
      return context_->get_field_cpp_kind(self) != field_cpp_kind::value;
    });
    def.property("unique_ref?", [this](const t_field& self) {
      return context_->get_field_cpp_kind(self) == field_cpp_kind::unique_ptr;
    });
    def.property("shared_ref?", [this](const t_field& self) {
      return context_->get_field_cpp_kind(self) ==
          field_cpp_kind::shared_ptr_mutable;
    });
    def.property("shared_const_ref?", [this](const t_field& self) {
      return context_->get_field_cpp_kind(self) ==
          field_cpp_kind::shared_ptr_const;
    });
    def.property("iobuf_ref?", [this](const t_field& self) {
      return context_->get_field_cpp_kind(self) == field_cpp_kind::iobuf;
    });
    def.property("has_ref_accessor?", [this](const t_field& self) {
      const field_cpp_kind cpp_kind = context_->get_field_cpp_kind(self);
      return cpp_kind == field_cpp_kind::value ||
          cpp_kind == field_cpp_kind::iobuf;
    });
    def.property("hasDefaultValue?", [this](const t_field& self) {
      return field_has_default_value(self, context_->get_field_cpp_kind(self));
    });
    def.property("optional_default?", [](const t_field& self) {
      return self.qualifier() == t_field_qualifier::optional &&
          self.default_value() != nullptr;
    });
    def.property("boxed_ref?", [](const t_field& self) {
      return gen::cpp::find_ref_type(self) == gen::cpp::reference_type::boxed;
    });

    return std::move(def).make();
  }

  prototype<t_function>::ptr make_prototype_for_function(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_function(proto);
    auto def =
        whisker::dsl::prototype_builder<h_function>::extends(std::move(base));

    def.property("eb?", [this](const t_function& self) {
      if (self.has_structured_annotation(kCppProcessInEbThreadUri)) {
        return true;
      }
      const t_interface* parent = context().get_function_parent(&self);
      assert(parent != nullptr);
      return parent->has_structured_annotation(kCppProcessInEbThreadUri);
    });
    def.property("stack_arguments?", [this](const t_function& self) {
      return cpp2::is_stack_arguments(compiler_options(), self);
    });

    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);
    def.property("py_name", &python::get_py3_name);
    def.property("hasPyName?", [](const t_named& self) {
      return python::get_py3_name(self) != self.name();
    });
    def.property(
        "cpp_name", [](const t_named& self) { return cpp2::get_name(&self); });
    return std::move(def).make();
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);

    def.property("consts", [&proto](const t_program& self) {
      // Overrides t_whisker_generator's `consts`, ignoring hidden consts
      whisker::array::raw consts;
      for (const t_const* constant : self.consts()) {
        if (!is_hidden(*constant)) {
          consts.emplace_back(proto.create<t_const>(*constant));
        }
      }
      return whisker::make::array(std::move(consts));
    });
    def.property(
        "py3_visible_structured_definitions", [&proto](const t_program& self) {
          whisker::array::raw visible;
          visible.reserve(self.structured_definitions().size());
          for (const t_structured* s : self.structured_definitions()) {
            if (!is_hidden(*s)) {
              visible.emplace_back(resolve_derived_t_type(proto, *s));
            }
          }
          return whisker::make::array(std::move(visible));
        });
    def.property("py3_visible_typedefs", [&proto](const t_program& self) {
      whisker::array::raw visible;
      visible.reserve(self.typedefs().size());
      for (const t_typedef* t : self.typedefs()) {
        if (!is_hidden(*t) && !is_hidden(*t->get_true_type())) {
          visible.emplace_back(proto.create<t_typedef>(*t));
        }
      }
      return whisker::make::array(std::move(visible));
    });
    def.property(
        "has_types?", [](const t_program& self) { return has_types(&self); });
    def.property("has_visible_union_types?", [](const t_program& self) {
      return std::any_of(
          self.structured_definitions().begin(),
          self.structured_definitions().end(),
          [](const t_structured* s) {
            return !is_hidden(*s) && s->is<t_union>();
          });
    });
    def.property("py_deprecated_module_path", [](const t_program& self) {
      const std::string& module_path = self.get_namespace("py");
      return module_path.empty() ? self.name() : module_path;
    });
    def.property("cpp_gen_path", [this](const t_program&) {
      return whisker::make::string(
          has_compiler_option("py3cpp") ? "gen-py3cpp" : "gen-cpp2");
    });
    def.property("python_capi_converter?", [this](const t_program&) {
      return has_compiler_option("python_capi_converter");
    });
    def.property("capi_module_prefix", [](const t_program& self) {
      return python::gen_capi_module_prefix_impl(&self);
    });
    def.property("auto_migrate?", [this](const t_program&) {
      return has_compiler_option("auto_migrate");
    });
    def.property("gen_legacy_container_converters?", [this](const t_program&) {
      return has_compiler_option("gen_legacy_container_converters");
    });
    def.property("inplace_migrate?", [this](const t_program&) {
      // this option triggers generation of py3 structs as wrappers around
      // thrift-python structs
      return has_compiler_option("inplace_migrate");
    });
    def.property("gen_py3_cython?", [this](const t_program&) {
      return !(
          has_compiler_option("auto_migrate") ||
          has_compiler_option("inplace_migrate"));
    });
    def.property("cppIncludes", [](const t_program& self) {
      if (const auto& it = self.language_includes().find("cpp");
          it != self.language_includes().end()) {
        return to_whisker_string_array(it->second);
      }
      return whisker::make::array();
    });
    def.property("cpp_namespace", [](const t_program& self) {
      return fmt::format(
          "{}", fmt::join(cpp2::get_gen_namespace_components(self), "::"));
    });
    def.property("module_path", [](const t_program& self) {
      std::vector<std::string> segments = get_py3_namespace_with_name(&self);
      assert(!segments.empty());
      return fmt::format("{}", fmt::join(segments, "."));
    });
    def.property("module_path_for_alias", [](const t_program& self) {
      std::vector<std::string> segments = get_py3_namespace_with_name(&self);
      assert(!segments.empty());
      return fmt::format("_{}", fmt::join(segments, "_"));
    });
    def.property("module_path_cpp_import", [](const t_program& self) {
      std::vector<std::string> segments = get_py3_namespace_with_name(&self);
      assert(!segments.empty());
      return fmt::format("{}", fmt::join(segments, "__"));
    });
    def.property("includeNamespaces", [&](const t_program& self) {
      whisker::array::raw includes;
      for (const auto& [_, include] : context_->included_programs(self)) {
        includes.emplace_back(proto.create<py3_included_program>(include));
      }
      return whisker::make::array(std::move(includes));
    });
    def.property("has_container_types?", [this](const t_program& self) {
      return !context_->container_types(self).empty();
    });
    def.property("needs_container_converters?", [this](const t_program& self) {
      return !context_->container_types(self).empty() &&
          (!self.services().empty() ||
           has_compiler_option("gen_legacy_container_converters"));
    });
    def.property("has_stream?", [this](const t_program& self) {
      return !has_compiler_option("no_stream") &&
          !context_->stream_types(self).empty();
    });
    def.property("container_types", [this, &proto](const t_program& self) {
      return to_type_array(context_->container_types(self), proto);
    });
    def.property("stream_types", [&](const t_program& self) {
      std::vector<const t_type*> types;
      if (!has_compiler_option("no_stream")) {
        for (const auto& [_, type] : context_->stream_types(self)) {
          types.emplace_back(type);
        }
      }
      return to_type_array(types, proto);
    });
    def.property("stream_exceptions", [&](const t_program& self) {
      std::vector<const t_type*> exceptions;
      for (const auto& [_, type] : context_->stream_exceptions(self)) {
        exceptions.emplace_back(type);
      }
      return to_type_array(exceptions, proto);
    });
    def.property("response_and_stream_functions", [&](const t_program& self) {
      return to_array(
          context_->response_and_stream_functions(self),
          proto.of<t_function>());
    });
    def.property("unique_functions_by_return_type", [&](const t_program& self) {
      std::vector<const t_function*> functions;
      bool no_stream = has_compiler_option("no_stream");
      for (const auto& [_, func] :
           context_->unique_functions_by_return_type(self)) {
        if (is_func_supported(no_stream, func)) {
          functions.emplace_back(func);
        }
      }
      return to_array(functions, proto.of<t_function>());
    });
    def.property("custom_templates", [this, &proto](const t_program& self) {
      return to_type_array(context_->custom_templates(self), proto);
    });
    def.property("custom_cpp_types", [this, &proto](const t_program& self) {
      return to_type_array(context_->custom_cpp_types(self), proto);
    });
    def.property(
        "field_custom_type_ctypedefs", [this, &proto](const t_program& self) {
          const auto& entries = context_->field_custom_ctypedefs(self);
          whisker::array::raw arr;
          arr.reserve(entries.size());
          for (const auto& entry : entries) {
            whisker::map::raw m;
            m["type"] = resolve_derived_t_type(proto, *entry.type);
            m["cython_name"] = whisker::make::string(entry.cython_name);
            m["cpp_type"] = whisker::make::string(entry.cpp_type);
            m["flexible_binary?"] = entry.flexible_binary;
            arr.emplace_back(whisker::make::map(std::move(m)));
          }
          return whisker::make::array(std::move(arr));
        });

    return std::move(def).make();
  }

  prototype<py3_included_program>::ptr make_prototype_for_included_program(
      const prototype_database& proto) const {
    whisker::dsl::prototype_builder<
        whisker::native_handle<py3_included_program>>
        def;

    def.property("program", [&proto](const py3_included_program& self) {
      return proto.create<t_program>(*self.program);
    });
    def.property("import_services?", [](const py3_included_program& self) {
      return self.importServices && !self.program->services().empty();
    });

    return std::move(def).make();
  }

  prototype<t_interface>::ptr make_prototype_for_interface(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interface(proto);
    auto def =
        whisker::dsl::prototype_builder<h_interface>::extends(std::move(base));

    def.property("supportedFunctions", [&](const t_interface& self) {
      whisker::array::raw functions;
      bool no_stream = has_compiler_option("no_stream");
      for (const t_function& func : self.functions()) {
        if (is_func_supported(no_stream, &func)) {
          functions.emplace_back(proto.create<t_function>(func));
        }
      }
      return whisker::make::array(std::move(functions));
    });

    return std::move(def).make();
  }

  prototype<t_service>::ptr make_prototype_for_service(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_service(proto);
    auto def =
        whisker::dsl::prototype_builder<h_service>::extends(std::move(base));

    def.property("externalProgram?", [this](const t_service& self) {
      return self.program() != program_;
    });
    def.property("lifecycleFunctions", [&proto](const t_service&) {
      return to_array(lifecycleFunctions(), proto.of<t_function>());
    });
    def.property("supportedInteractions", [&](const t_service& self) {
      return to_array(
          context_->supported_interactions(self), proto.of<t_interaction>());
    });

    return std::move(def).make();
  }

  prototype<t_structured>::ptr make_prototype_for_structured(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_structured(proto);
    auto def =
        whisker::dsl::prototype_builder<h_structured>::extends(std::move(base));

    def.property("allow_inheritance?", [](const t_structured& self) {
      return self.has_structured_annotation(
          kPythonMigrationBlockingAllowInheritanceUri);
    });
    def.property("cpp_noncomparable?", [](const t_structured& self) {
      return self.has_unstructured_annotation(
          {"cpp.noncomparable", "cpp2.noncomparable"});
    });
    def.property("cpp_noncopyable?", [](const t_structured& self) {
      return self.has_unstructured_annotation(
          {"cpp.noncopyable", "cpp2.noncopyable"});
    });
    def.property("is_struct_orderable?", [](const t_structured& self) {
      return !self.has_unstructured_annotation("no_default_comparators") &&
          cpp2::OrderableTypeUtils::is_orderable(self);
    });
    def.property("has_hidden_fields?", [this](const t_structured& self) {
      return context_->get_structured_context(self).hasHiddenFields;
    });
    def.property("has_defaulted_fields?", [this](const t_structured& self) {
      return context_->get_structured_context(self).hasDefaultedFields &&
          !self.is<t_union>();
    });
    def.property("py3_fields", [&](const t_structured& self) {
      return to_array(
          context_->get_structured_context(self).nonHiddenFields,
          proto.of<t_field>());
    });

    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    // Override for base Whisker generator's `program` property, falling back to
    // the current program for primitives/containers which have a `nullptr`
    // program. This allows us to use the `t_program` module path properties
    // without a mess of conditions in the template.
    def.property("program", [&](const t_type& self) {
      return proto.create<t_program>(
          self.program() == nullptr ? *program_ : *self.program());
    });

    // Overrides for `t_named` properties, resolving typedefs before computing
    // the value.
    def.property("py_name", [](const t_type& self) {
      return python::get_py3_name(*self.get_true_type());
    });
    def.property("hasPyName?", [](const t_type& self) {
      const t_type* true_type = self.get_true_type();
      return python::get_py3_name(*true_type) != true_type->name();
    });
    def.property("cpp_name", [](const t_type& self) {
      return cpp2::get_name(self.get_true_type());
    });
    def.property("need_module_path?", [this](const t_type& self) {
      return file_type_ == FileType::NotTypesFile ||
          get_true_type_program(self) != get_program();
    });
    def.property("iobufWrapper?", [this](const t_type& self) {
      const py3_generator_context::cached_type_properties& cached_props =
          context_->get_cached_type_props(&self);
      return cached_props.cpp_type() == "folly::IOBuf" ||
          cached_props.cpp_type() == "std::unique_ptr<folly::IOBuf>";
    });
    def.property("flat_name", [this](const t_type& self) {
      return context_->get_cached_type_props(&self).flat_name();
    });
    def.property("need_cbinding_path?", [this](const t_type& self) {
      // Need import if in a different declaration file, or type originated in a
      // different Thrift program.
      // For non-container custom cpp types (typedefs with @cpp.Type), use
      // the type's own program rather than the true type's program, because
      // the ctypedef is generated in the type's (typedef's) program's
      // cbindings. Container custom types must use get_true_type_program to
      // preserve consistent type resolution between cbindings.pxd field_ref
      // declarations and the from_cpp function parameter types.
      const t_program* type_program;
      if (!context_->get_cached_type_props(&self).cpp_type().empty() &&
          !self.get_true_type()->is<t_container>()) {
        type_program = self.program();
        if (type_program == nullptr) {
          type_program = program_;
        }
      } else {
        type_program = get_true_type_program(self);
      }
      return file_type_ != FileType::CBindingsFile ||
          type_program != get_program();
    });
    def.property("cppTemplate", [this](const t_type& self) {
      return context_->get_cached_type_props(&self).cpp_template();
    });
    def.property("cythonTemplate", [this](const t_type& self) {
      return context_->get_cached_type_props(&self).to_cython_template();
    });
    def.property("defaultTemplate?", [this](const t_type& self) {
      return context_->get_cached_type_props(&self).is_default_template(
          self.get_true_type());
    });
    def.property("customCppType", [this](const t_type& self) {
      return context_->get_cached_type_props(&self).cpp_type();
    });
    def.property("cythonCustomType", [this](const t_type& self) {
      return context_->get_cached_type_props(&self).to_cython_type();
    });
    def.property("customCppType?", [this](const t_type& self) {
      return !context_->get_cached_type_props(&self).cpp_type().empty();
    });
    def.property("number?", [](const t_type& self) {
      return is_number(*self.get_true_type());
    });
    def.property("integer?", [](const t_type& self) {
      const t_type& resolved = *self.get_true_type();
      return resolved.is_any_int() || resolved.is_byte();
    });
    def.property("containerOfString?", [](const t_type& self) {
      const t_type& resolved = *self.get_true_type();
      if (const t_list* list = resolved.try_as<t_list>()) {
        return list->elem_type()->is_string_or_binary();
      }
      if (const t_set* set = resolved.try_as<t_set>()) {
        return set->elem_type()->is_string_or_binary();
      }
      return false;
    });
    def.property("cythonTypeNoneable?", [](const t_type& self) {
      const t_type& resolved = *self.get_true_type();
      return !is_number(resolved) && !resolved.is<t_container>();
    });
    def.property("hasCythonType?", [this](const t_type& self) {
      const t_type& resolved = *self.get_true_type();
      return has_compiler_option("inplace_migrate")
          ? !(resolved.is<t_container>() || resolved.is<t_structured>())
          : !resolved.is<t_container>();
    });
    def.property("iobuf?", [this](const t_type& self) {
      return is_iobuf(context_->get_cached_type_props(&self).cpp_type());
    });
    def.property("iobufRef?", [this](const t_type& self) {
      return is_iobuf_ref(context_->get_cached_type_props(&self).cpp_type());
    });
    def.property("flexibleBinary?", [this](const t_type& self) {
      return is_flexible_binary(
          *self.get_true_type(),
          context_->get_cached_type_props(&self).cpp_type());
    });
    def.property("customBinaryType?", [this](const t_type& self) {
      return is_custom_binary_type(
          *self.get_true_type(),
          context_->get_cached_type_props(&self).cpp_type());
    });
    // types that need conversion to py3 if accessed from thrift-python struct
    // fields
    def.property("needs_convert?", [](const t_type& self) {
      return type_needs_convert(self.get_true_type());
    });
    def.property("is_container_of_struct?", [](const t_type& self) {
      const t_type& resolved = *self.get_true_type();
      return resolved.is<t_container>() && container_needs_convert(&resolved);
    });

    return std::move(def).make();
  }
};

py3_generator_context::cached_type_properties&
py3_generator_context::get_cached_type_props(
    const t_type* type, const t_field* field) const {
  // @python.Py3EnableCppAdapter treats C++ Adapter on typedef as a custom
  // cpp.type.
  auto true_type = type->get_true_type();
  if (type->has_structured_annotation(kPythonPy3EnableCppAdapterUri)) {
    return type_properties_
        .emplace(
            type,
            cached_type_properties{
                get_cpp_template(*type),
                name_resolver_.get_native_type(*type),
                {}})
        .first->second;
  }
  // Look for @cpp.Type annotation from:
  // 1. The typedef chain (for typedef-level @cpp.Type).
  // 2. The explicit field parameter (for field-level @cpp.Type).
  const t_const* annot =
      t_typedef::get_first_structured_annotation_or_null(type, kCppTypeUri);
  bool annot_from_field = false;
  if (!annot && field) {
    auto it = field_cpp_type_annotations_.find(field);
    // Only apply the field's annotation to the field's own type, not to
    // child types (key/val/elem).
    if (it != field_cpp_type_annotations_.end() &&
        &field->type().deref() == type) {
      annot = it->second;
      annot_from_field = true;
    }
  }
  if (annot) {
    // Extract the template value from the annotation we found (which may be
    // on an intermediate typedef or the owning field), since
    // get_cpp_template(*type) only checks the immediate node.
    auto get_template_from_annot = [&]() -> std::string {
      if (auto* tmpl =
              annot->get_value_from_structured_annotation_or_null("template")) {
        return tmpl->get_string();
      }
      return get_cpp_template(*type);
    };
    auto* name = annot->get_value_from_structured_annotation_or_null("name");
    bool has_template = annot->get_value_from_structured_annotation_or_null(
                            "template") != nullptr;
    if (name || has_template) {
      auto make_props = [&](std::string cpp_type_name) {
        return cached_type_properties{
            get_template_from_annot(), std::move(cpp_type_name), {}};
      };
      std::string cpp_type_name =
          name ? name->get_string() : fmt::to_string(cpp2::get_type(type));
      // For non-container types with field-level annotations, use a
      // field-keyed cache since the type pointer may be shared across fields
      // with different annotations. This includes typedef-wrapping-containers
      // (the typedef node is shared). Raw containers have unique AST nodes
      // per declaration and use the type-keyed cache below.
      if (annot_from_field && field && !type->is<t_container>()) {
        return field_type_properties_
            .emplace(field, make_props(std::move(cpp_type_name)))
            .first->second;
      }
      // For containers (unique AST nodes) or typedef-chain annotations,
      // use the type-keyed cache.
      auto cache_key = annot_from_field
          ? type
          : (true_type->is<t_container>() ? true_type : type);
      return type_properties_
          .emplace(cache_key, make_props(std::move(cpp_type_name)))
          .first->second;
    }
  }
  // Check for an existing entry by the original type pointer first (catches
  // entries propagated from field-level annotations in visit_type, as well
  // as entries stored by the annotation block above for non-container
  // typedefs). Then fall back to true_type (the canonical lookup for
  // non-typedef types and typedef-chain annotations on containers).
  auto it = type_properties_.find(type);
  if (it == type_properties_.end()) {
    it = type_properties_.find(true_type);
  }
  if (it == type_properties_.end()) {
    it = type_properties_
             .emplace(
                 true_type,
                 cached_type_properties{
                     get_cpp_template(*type),
                     fmt::to_string(cpp2::get_type(type)),
                     {}})
             .first;
  }
  return it->second;
}

void t_mstch_py3_generator::generate_init_files() {
  std::filesystem::path p = generateRootPath_;
  while (!p.empty()) {
    t_whisker_generator::render_to_file(
        /*output_file=*/p / "__init__.py",
        /*template_file=*/"common/auto_generated_py",
        /*context=*/whisker::make::null);
    p = p.parent_path();
  }
}

std::filesystem::path t_mstch_py3_generator::package_to_path() {
  auto package = get_py3_namespace(get_program());
  std::filesystem::path path;
  for (const auto& path_part : package) {
    path /= path_part;
  }
  return path;
}

void t_mstch_py3_generator::generate_whisker_file(
    const std::string& template_name,
    FileType file_type,
    const std::filesystem::path& base = {}) {
  file_type_ = file_type;
  whisker::object context = whisker::make::map({
      {"program",
       whisker::make::native_handle(
           render_state().prototypes->create<t_program>(*program_))},
  });
  t_whisker_generator::render_to_file(
      /*output_file=*/base / program_->name() / template_name,
      /*template_file=*/template_name,
      /*context=*/context);
}

void t_mstch_py3_generator::generate_types() {
  std::vector<std::string> autoMigrateFilesWithTypeContext{
      "types.py",
      "types_auto_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "types_auto_migrated.py",
  };

  std::vector<std::string> autoMigrateFilesNoTypeContext{
      "metadata.py",
  };

  std::vector<std::string> converterFiles{
      "converter.pxd",
      "converter.pyx",
  };

  std::vector<std::string> cythonFilesWithTypeContext{
      "types.pyx",
      "types.pxd",
      "types.pyi",
  };

  std::vector<std::string> cythonFilesNoTypeContext{
      "builders.py",
      "constants_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "containers_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "metadata.pxd",
      "metadata.pyi",
      "metadata.pyx",
      "types_auto_migrated.py",
      "types_empty.pyx",
      "types_fields.pxd",
      "types_fields.pyx",
      "types_impl_FBTHRIFT_ONLY_DO_NOT_USE.py",
      "types_reflection.py",
  };

  std::vector<std::string> cppFilesWithTypeContext{
      "types.h",
  };

  std::vector<std::string> cppFilesWithNoTypeContext{
      "metadata.h",
      "metadata.cpp",
  };

  generate_whisker_file(
      "cbindings.pxd", FileType::CBindingsFile, generateRootPath_);

  if (has_compiler_option("enable_container_pickling_DO_NOT_USE")) {
    generate_whisker_file(
        "__init__.py", FileType::TypesFile, generateRootPath_);
  }
  if (has_compiler_option("inplace_migrate")) {
    generate_whisker_file(
        "types_inplace_FBTHRIFT_ONLY_DO_NOT_USE.py",
        FileType::TypesFile,
        generateRootPath_);
  }
  for (const auto& file : converterFiles) {
    generate_whisker_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  // - if auto_migrate is present, generate types.pxd, and types.py
  // - else, just generate normal cython files
  for (const auto& file : autoMigrateFilesWithTypeContext) {
    generate_whisker_file(file, FileType::TypesFile, generateRootPath_);
  }
  for (const auto& file : autoMigrateFilesNoTypeContext) {
    generate_whisker_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cythonFilesWithTypeContext) {
    generate_whisker_file(file, FileType::TypesFile, generateRootPath_);
  }
  for (const auto& file : cppFilesWithTypeContext) {
    generate_whisker_file(file, FileType::TypesFile);
  }
  for (const auto& file : cythonFilesNoTypeContext) {
    generate_whisker_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cppFilesWithNoTypeContext) {
    generate_whisker_file(file, FileType::NotTypesFile);
  }
}

void t_mstch_py3_generator::generate_services() {
  if (get_program()->services().empty() &&
      !has_compiler_option("single_file_service")) {
    // There is no need to generate empty / broken code for non existent
    // services. However, in single_file_service mode, the build system may
    // not know ahead of time if these files can exist - so we should always
    // generate them.
    return;
  }

  std::vector<std::string> pythonFiles{
      "clients.py",
      "services.py",
  };

  std::vector<std::string> normalCythonFiles{
      "clients.pxd",
      "clients.pyx",
      "clients.pyi",
      "services.pxd",
      "services.pyx",
      "services.pyi",
  };

  std::vector<std::string> cythonFiles{
      "clients_wrapper.pxd",
      "services_wrapper.pxd",
      "services_interface.pxd",
  };

  std::vector<std::string> cppFiles{
      "clients_wrapper.h",
      "clients_wrapper.cpp",
      "services_wrapper.h",
      "services_wrapper.cpp",
  };

  // TODO this logic is a complete mess and I intend to clean it up later
  // the gist is:
  // - if auto_migrate is present, generate py3_clients and clients.px
  // - if auto_migrate isn't present, just generate all the normal files

  for (const auto& file : pythonFiles) {
    generate_whisker_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : normalCythonFiles) {
    generate_whisker_file(file, FileType::NotTypesFile, generateRootPath_);
  }
  for (const auto& file : cppFiles) {
    generate_whisker_file(file, FileType::NotTypesFile);
  }
  for (const auto& file : cythonFiles) {
    generate_whisker_file(file, FileType::NotTypesFile, generateRootPath_);
  }
}

} // namespace

THRIFT_REGISTER_GENERATOR(
    mstch_py3,
    "Python 3",
    "    include_prefix:  Use full include paths in generated files.\n");

} // namespace apache::thrift::compiler
