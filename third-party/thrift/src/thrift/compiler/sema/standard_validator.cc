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

#include <thrift/compiler/sema/standard_validator.h>

#include <algorithm>
#include <set>
#include <string>
#include <unordered_map>

#include <boost/algorithm/string/split.hpp>
#include <fmt/ranges.h>

#include <thrift/common/universal_name.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_interface.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>
#include <thrift/compiler/generate/cpp/reference_type.h>
#include <thrift/compiler/generate/cpp/util.h>
#include <thrift/compiler/sema/ast_uri_utils.h>
#include <thrift/compiler/sema/explicit_include_validator.h>
#include <thrift/compiler/sema/reserved_identifier.h>
#include <thrift/compiler/sema/resolution_mismatch.h>

namespace apache::thrift::compiler {

namespace {
using cpp2::OrderableTypeUtils;

const t_structured* get_mixin_type(const t_field& field) {
  if (cpp2::is_mixin(field)) {
    return dynamic_cast<const t_structured*>(field.type()->get_true_type());
  }
  return nullptr;
}

bool has_experimental_annotation(sema_context& ctx, const t_named& node) {
  if (node.has_structured_annotation(kExperimentalUri)) {
    return true;
  }
  for (int pos = ctx.nodes().size() - 1; pos >= 0; --pos) {
    const auto* parent = dynamic_cast<const t_named*>(ctx.nodes().at(pos));
    if (parent != nullptr &&
        parent->has_structured_annotation(kExperimentalUri)) {
      return true;
    }
  }
  return false;
}

bool has_lazy_field(const t_structured& node) {
  for (const auto& field : node.fields()) {
    if (cpp2::is_lazy(&field)) {
      return true;
    }
  }
  return false;
}

diagnostic_level validation_to_diagnostic_level(
    sema_params::validation_level validation_level) {
  switch (validation_level) {
    case sema_params::validation_level::error:
      return diagnostic_level::error;
    case sema_params::validation_level::warn:
      return diagnostic_level::warning;
    case sema_params::validation_level::none:
      return diagnostic_level::debug;
  }
}

// Reports an existing name was redefined within the given parent node.
void report_redef_diag(
    diagnostics_engine& diags,
    const diagnostic_level& level,
    const char* kind,
    std::string_view name,
    const t_named& parent,
    const t_node& child,
    const t_node& /*existing*/) {
  // TODO(afuller): Use `existing` to provide more detail in the
  // diagnostic.
  diags.report(
      child,
      level,
      "{} `{}` is already defined for `{}`.",
      kind,
      name,
      parent.name());
}

// Helper for checking for the redefinition of a name in the context of a node.
class redef_checker {
 public:
  redef_checker(
      diagnostics_engine& diags,
      const char* kind,
      const t_named& parent,
      const diagnostic_level& level = diagnostic_level::error)
      : diags_(diags), kind_(kind), parent_(parent), level_(level) {}

  // Checks if the given `name`, derived from `node` via `child`, has already
  // been defined.
  //
  // For example, a mixin field causes all fields of the mixin type to be
  // inherited. In this case 'node' wold be the mixin type, from which `name`
  // was derived, while `child` is the mixin field that caused the name to be
  // inherited.
  void check(std::string_view name, const t_named& node, const t_node& child) {
    const t_named* existing = insert(name, node);
    if (!existing) {
      return;
    }
    if (&node == &parent_ && existing == &parent_) {
      // The degenerate case where parent_ is conflicting with itself.
      report_redef_diag(diags_, level_, kind_, name, parent_, child, *existing);
    } else {
      diags_.report(
          child,
          level_,
          "{} `{}.{}` and `{}.{}` can not have same name in `{}`.",
          kind_,
          node.name(),
          name,
          existing->name(),
          name,
          parent_.name());
    }
  }

  // Helpers for the common case where the names are from child t_nameds of
  // the parent.
  //
  // For example, all functions in an interface.
  void check(const t_named& child) {
    if (const t_named* existing = insert(child.name(), child)) {
      report_redef_diag(
          diags_, level_, kind_, child.name(), parent_, child, *existing);
    }
  }

  template <typename Cs>
  void check_all(const Cs& children) {
    for (const t_named& child : children) {
      check(child);
    }
  }

 private:
  const t_named* insert(std::string_view name, const t_named& node) {
    auto [it, success] = seen_.emplace(name, &node);
    if (success) {
      return nullptr;
    }
    const t_named* existing = it->second;
    seen_[name] = &node;
    return existing;
  }

  diagnostics_engine& diags_;
  const char* kind_;
  const t_named& parent_;
  const diagnostic_level level_;

  std::unordered_map<std::string_view, const t_named*> seen_;
};

// Helper for validating the adapters
class adapter_or_wrapper_checker {
 public:
  explicit adapter_or_wrapper_checker(sema_context& ctx) : ctx_(ctx) {}

  // Checks if adapter name is provided
  // Do not allow composing two structured annotations on typedef
  void check(
      const t_named& node,
      const char* structured_adapter_annotation,
      const char* structured_adapter_annotation_error_name,
      const char* name) {
    const t_const* adapter_annotation =
        node.find_structured_annotation_or_null(structured_adapter_annotation);
    if (!adapter_annotation) {
      return;
    }

    try {
      adapter_annotation->get_value_from_structured_annotation(name);
    } catch (const std::exception& e) {
      ctx_.error("{}", e.what());
      return;
    }

    if (const auto* typedf = dynamic_cast<const t_typedef*>(&node)) {
      if (t_typedef::get_first_structured_annotation_or_null(
              &*typedf->type(), structured_adapter_annotation)) {
        ctx_.error(
            "The `{}` annotation cannot be annotated more than once in all "
            "typedef levels in `{}`.",
            structured_adapter_annotation_error_name,
            node.name());
      }
    }
  }

  // Do not allow composing structured annotation on field/typedef
  void check(
      const t_field& field,
      const char* structured_adapter_annotation,
      const char* structured_adapter_annotation_error_name,
      bool disallow_structured_annotations_on_both_field_and_typedef) {
    if (!field.type().resolved()) {
      return;
    }
    auto type = field.type().get_type();

    bool structured_annotation_on_field =
        field.has_structured_annotation(structured_adapter_annotation);

    bool structured_annotation_on_typedef =
        t_typedef::get_first_structured_annotation_or_null(
            type, structured_adapter_annotation) != nullptr;

    if (disallow_structured_annotations_on_both_field_and_typedef &&
        structured_annotation_on_field && structured_annotation_on_typedef) {
      ctx_.error(
          "`{}` cannot be applied on both field and typedef in `{}`.",
          structured_adapter_annotation_error_name,
          field.name());
    }
  }

  // If a type is wrapped itself or is a container of wrapped types, then it
  // cannot be adapted
  void check(
      const t_named& node,
      const char* structured_adapter_annotation,
      const char* structured_wrapper_annotation,
      const char* structured_adapter_annotation_error_name,
      const char* structured_wrapper_annotation_error_name) {
    bool has_adapter_annotation =
        node.has_structured_annotation(structured_adapter_annotation);
    if (!has_adapter_annotation) {
      return;
    }
    const t_type* type;
    if (const auto* field = dynamic_cast<const t_field*>(&node)) {
      if (!field->type().resolved()) {
        return;
      }
      type = field->type().get_type();
    } else if (const auto* typdef = dynamic_cast<const t_typedef*>(&node)) {
      type = typdef->type().get_type();
    } else {
      return;
    }

    auto has_wrapper =
        type->has_structured_annotation(structured_wrapper_annotation);
    std::string typedef_name = type->name();
    while (!has_wrapper) {
      if (const auto* inner_typedf = type->try_as<t_typedef>()) {
        has_wrapper = inner_typedf->has_structured_annotation(
            structured_wrapper_annotation);
        typedef_name = inner_typedf->name();
        type = inner_typedf->type().get_type();
      } else if (type->is<t_container>()) {
        if (const auto* map = type->try_as<t_map>()) {
          type = &map->val_type().deref();
        } else if (const auto* list = type->try_as<t_list>()) {
          type = list->elem_type().get_type();
        } else {
          break;
        }
      } else if (type->is<t_struct>() || type->is<t_union>()) {
        has_wrapper =
            type->has_structured_annotation(structured_wrapper_annotation);
        typedef_name = type->name();
        break;
      } else {
        break;
      }
    }
    if (has_wrapper) {
      ctx_.error(
          "`{}` on `{}` cannot be combined with `{}` on `{}`.",
          structured_adapter_annotation_error_name,
          node.name(),
          structured_wrapper_annotation_error_name,
          typedef_name);
    }
  }

 private:
  sema_context& ctx_;
};

struct service_metadata {
  std::unordered_map<std::string_view, const t_service*>
      function_name_to_service;

  service_metadata(node_metadata_cache& cache, const t_service& node) {
    if (node.extends() != nullptr) {
      // Add all the inherited functions.
      function_name_to_service =
          cache.get<service_metadata>(*node.extends()).function_name_to_service;
    }
    // Add all the directly defined functions.
    for (const auto& function : node.functions()) {
      function_name_to_service[function.name()] = &node;
    }
  }
};

struct structured_metadata {
  std::unordered_map<std::string_view, const t_structured*>
      field_name_to_parent;

  structured_metadata(node_metadata_cache& cache, const t_structured& node) {
    for (const auto& field : node.fields()) {
      if (const auto* mixin = get_mixin_type(field)) {
        // Add all the inherited mixin fields from field.
        auto mixin_metadata = cache.get<structured_metadata>(*mixin);
        for (auto [key, value] : mixin_metadata.field_name_to_parent) {
          field_name_to_parent[key] = value;
        }
      }
      // Add the directly defined field.
      field_name_to_parent[field.name()] = &node;
    }
  }
};

void validate_identifier_is_not_reserved(
    sema_context& ctx, const t_named& node) {
  // Bypass the check any of the cases below.
  //  1. the node was generated.
  //  2. @thrift.AllowedReservedIdentifier is present.
  if (node.generated() ||
      node.has_structured_annotation(kAllowReservedIdentifierUri)) {
    return;
  }
  ctx.check(
      !is_reserved_identifier(node.name()),
      "`{}` is a reserved identifier. Choose a different identifier that does not contain `fbthrift`.",
      node.name());
}

void validate_filename_is_not_reserved(sema_context& ctx, const t_named& node) {
  // Bypass the check any of the cases below.
  //  1. the node was generated.
  //  2. @thrift.AllowReservedFilename is present.
  if (node.generated() ||
      node.has_structured_annotation(kAllowReservedFilenameUri)) {
    return;
  }
  ctx.check(
      !is_reserved_identifier(node.name()),
      "`{}` is a reserved filename. Choose a different filename that does not contain `fbthrift`.",
      node.name());
}

/**
 * Checks that the given program has the recommended python namespaces, i.e.
 * either:
 * 1. no Python namespace at all, or
 * 2. at least namespace py3 (in addition to any py-deprecated namespace)
 */
void validate_python_namespaces(sema_context& ctx, const t_program& program) {
  // If a (non-empty) package is defined, all namespaces are effectively
  // defined, so return.
  if (!program.package().empty()) {
    return;
  }

  const std::map<std::string, std::string>& namespaces = program.namespaces();

  // If there are no Python namespaces, there is nothing to do.
  if (std::none_of(
          namespaces.begin(),
          namespaces.end(),
          [](const std::pair<const std::string, std::string>& it) {
            const std::string& key = it.first;
            return key.find("py") == 0; // i.e., key.starts_with("py")
          })) {
    return;
  }

  // There is at least one namespace that begins with "py", which is used as a
  // heuristic for "Python target language".

  // Require a "py3" namespace
  if (namespaces.find("py3") != namespaces.end()) {
    return;
  }

  ctx.warning(
      program,
      "File has namespaces only for deprecated Thrift Python variants. Please "
      "add `namespace py3` (or a non-empty `package`) to allow generation of "
      "the only officially supported variant: thrift-python.");
}

void validate_program_package(sema_context& ctx, const t_program& program) {
  const t_package& package = program.package();
  if (package.empty()) {
    ctx.report(
        program,
        validation_to_diagnostic_level(ctx.sema_parameters().missing_package),
        "Thrift file should have a (non-empty) package. Packages will soon be "
        "required, at which point missing packages will trigger a Thrift compiler error. "
        "For more details, see https://fburl.com/thrift-uri-add-package");
    return;
  }
  try {
    thrift::detail::check_univeral_name_domain(package.domain());
  } catch (const std::exception& e) {
    ctx.error(package.src_range().begin, "{}", e.what());
  }
  try {
    thrift::detail::check_universal_name_path(package.path());
  } catch (const std::exception& e) {
    ctx.error(package.src_range().begin, "{}", e.what());
  }
}

void validate_interface_function_name_uniqueness(
    sema_context& ctx, const t_interface& node) {
  // Check for a redefinition of a function in the same interface.
  redef_checker(ctx, "Function", node).check_all(node.functions());
}

// Checks for a redefinition of an inherited function.
void validate_extends_service_function_name_uniqueness(
    sema_context& ctx, const t_service& node) {
  if (node.extends() == nullptr) {
    return;
  }

  const auto& extends_metadata =
      ctx.cache().get<service_metadata>(*node.extends());
  for (const auto& function : node.functions()) {
    auto service =
        extends_metadata.function_name_to_service.find(function.name());
    if (service != extends_metadata.function_name_to_service.end()) {
      ctx.error(
          function,
          "Function `{0}.{2}` redefines `{1}.{2}`.",
          node.name(),
          service->second->get_full_name(),
          function.name());
    }
  }
}

void validate_throws_exceptions(sema_context& ctx, const t_field& except) {
  auto except_type = except.type()->get_true_type();
  ctx.check(
      dynamic_cast<const t_exception*>(except_type),
      "Non-exception type, `{}`, in throws.",
      except_type->name());
}

// Checks for a redefinition of a field in the same t_structured, including
// those inherited via mixin fields.
void validate_field_names_uniqueness(
    sema_context& ctx, const t_structured& node) {
  redef_checker checker(ctx, "Field", node);
  for (const auto& field : node.fields()) {
    // Check the directly defined field.
    checker.check(field.name(), node, field);

    // Check any transtively defined fields via a mixin annotation.
    if (const auto* mixin = get_mixin_type(field)) {
      const auto& mixin_metadata = ctx.cache().get<structured_metadata>(*mixin);
      for (auto [name, parent] : mixin_metadata.field_name_to_parent) {
        checker.check(name, *parent, field);
      }
    }
  }
}

// @thrift.ExceptionMessage annotation is only valid in exceptions.
// This validator checks if the node that contains any field
// with that annotation is an exception definiton.
void validate_exception_message_annotation_is_only_in_exceptions(
    sema_context& ctx, const t_structured& node) {
  for (const auto& f : node.fields()) {
    if (f.has_structured_annotation(kExceptionMessageUri)) {
      ctx.check(
          node.is<t_exception>(),
          f,
          "@thrift.ExceptionMessage annotation is only allowed in exception definitions. '{}' is not an exception.",
          node.name());
    }
  }
}

void validate_orderable_structured_types(
    sema_context& ctx, const t_structured& node) {
  switch (OrderableTypeUtils::get_orderable_condition(
      node, true /* enableCustomTypeOrderingIfStructureHasUri */)) {
    case OrderableTypeUtils::StructuredOrderableCondition::Always:
    case OrderableTypeUtils::StructuredOrderableCondition::
        OrderableByNestedLegacyImplicitLogicEnabledByUri: {
      const t_const* annotation = ctx.program().inherit_annotation_or_null(
          node, kCppEnableCustomTypeOrdering);
      if (annotation == nullptr) {
        return;
      }

      // @cpp.EnableCustomTypeOrdering is present, but not needed.

      ctx.report(
          *annotation,
          validation_to_diagnostic_level(
              ctx.sema_parameters().unnecessary_enable_custom_type_ordering),
          "Type `{}` does not need `@cpp.EnableCustomTypeOrdering` to be "
          "orderable in C++: remove the annotation.",
          node.name());
      return;
    }
    case OrderableTypeUtils::StructuredOrderableCondition::
        OrderableByLegacyImplicitLogicEnabledByUri: {
      // warn("Ordering enabled, add @cpp.EnableCustomTypeOrdering");
      ctx.warning(
          node,
          "Type `{}` is implicitly made orderable in C++ because it has a "
          "URI. This legacy behavior is being deprecated: enable ordering "
          "explicitly by annotating the type with "
          "`@cpp.EnableCustomTypeOrdering`.",
          node.name());
      return;
    }
    case OrderableTypeUtils::StructuredOrderableCondition::NotOrderable:
    case OrderableTypeUtils::StructuredOrderableCondition::
        OrderableByExplicitAnnotation:
    case OrderableTypeUtils::StructuredOrderableCondition::
        OrderableByExplicitAnnotationAndNestedLegacyImplicitLogic:
      // Nothing to do
      return;
  }
}

// Checks the attributes of fields in a union.
void validate_union_field_attributes(sema_context& ctx, const t_union& node) {
  for (const auto& field : node.fields()) {
    if (field.qualifier() == t_field_qualifier::optional ||
        field.qualifier() == t_field_qualifier::required) {
      ctx.error(
          field,
          "Unions cannot contain qualified fields. Remove `{}` qualifier from "
          "field `{}`.",
          field.qualifier() == t_field_qualifier::required ? "required"
                                                           : "optional",
          field.name());
    } else if (field.has_structured_annotation(kTerseWriteUri)) {
      ctx.error(
          field,
          "`@thrift.TerseWrite` cannot be applied to union fields (in `{}`).",
          node.name());
    }
  }
}

/**
 * Validates fields with C++ "reference types" (i.e., Box, cpp.Ref, etc.).
 */
void validate_boxed_field_attributes(sema_context& ctx, const t_field& node) {
  if (gen::cpp::find_ref_type(node) == gen::cpp::reference_type::none) {
    return;
  }

  const bool ref = node.has_unstructured_annotation({
                       "cpp.ref",
                       "cpp2.ref",
                       "cpp.ref_type",
                       "cpp2.ref_type",
                   }) ||
      node.has_structured_annotation(kCppRefUri);

  const bool box = node.has_unstructured_annotation({
                       "cpp.box",
                       "thrift.box",
                   }) ||
      node.has_structured_annotation(kBoxUri);

  const bool intern_box = node.has_structured_annotation(kInternBoxUri);

  if (ref + box + intern_box > 1) {
    ctx.error(
        node,
        "The {} annotation cannot be combined with the other reference "
        "annotations. Only annotate a single reference annotation from `{}`.",
        intern_box ? "`@thrift.InternBox`"
            : box  ? "`@thrift.Box`"
                   : "`@cpp.Ref`",
        node.name());
  }

  const t_structured& parent_node =
      dynamic_cast<const t_structured&>(*ctx.parent());

  if (node.qualifier() != t_field_qualifier::optional &&
      !parent_node.is<t_union>()) {
    // Field is not optional (and not in a union)
    // Reminder: all fields in a union are effectively optional.

    if (box) {
      // For thrift.Box, optional fields are always forbidden.
      ctx.error(
          "The `thrift.box` annotation can only be used with optional fields. "
          "Make sure `{}` is optional.",
          node.name());
    } else if (ref) {
      // For @cpp.Ref (and cpp[2].ref[_type]), non-optional fields result in
      // either a warning or an error, depending on whether the field is
      // annotated with `@cpp.AllowLegacyNonOptionalRef`.

      const bool report_error =
          !node.has_structured_annotation(kCppAllowLegacyNonOptionalRefUri);

      ctx.report(
          node,
          report_error ? diagnostic_level::error : diagnostic_level::warning,
          "Field with @cpp.Ref (or similar) annotation {} be optional: "
          "`{}` (in `{}`).",
          report_error ? "must" : "should",
          node.name(),
          parent_node.name());
    }
  }

  if (intern_box) {
    ctx.check(
        node.type()->get_true_type()->is<t_struct>() ||
            node.type()->get_true_type()->is<t_union>(),
        "The `@thrift.InternBox` annotation can only be used with a struct field.");
    // TODO(dokwon): Add support for custom defaults and remove this check.
    ctx.check(
        !node.default_value(),
        "The `@thrift.InternBox` annotation currently does not support a field with custom default.");
    ctx.check(
        node.qualifier() == t_field_qualifier::none ||
            node.qualifier() == t_field_qualifier::terse,
        "The `@thrift.InternBox` annotation can only be used with unqualified or terse fields."
        " Make sure `{}` is unqualified or annotated with `@thrift.TerseWrite`.",
        node.name());
  }
}

// Checks the attributes of a mixin field.
void validate_mixin_field_attributes(sema_context& ctx, const t_field& node) {
  if (!cpp2::is_mixin(node)) {
    return;
  }

  auto* ttype = node.type()->get_true_type();
  ctx.check(
      typeid(*ttype) == typeid(t_struct) || typeid(*ttype) == typeid(t_union),
      "Mixin field `{}` type must be a struct or union. Found `{}`.",
      node.name(),
      ttype->name());

  if (const auto* parent = dynamic_cast<const t_union*>(ctx.parent())) {
    ctx.error(
        "Union `{}` cannot contain mixin field `{}`.",
        parent->name(),
        node.name());
  } else if (node.qualifier() == t_field_qualifier::optional) {
    // Nothing technically stops us from marking optional field mixin.
    // However, this will bring surprising behavior. e.g. `foo.bar_ref()`
    // might throw `bad_field_access` if `bar` is inside optional mixin
    // field.
    ctx.error("Mixin field `{}` cannot be optional.", node.name());
  }
}

void validate_required_field(sema_context& ctx, const t_field& field) {
  if (field.qualifier() != t_field_qualifier::required) {
    return;
  }

  // field qualifier is "required"

  if (field.has_structured_annotation(kAllowUnsafeRequiredFieldQualifierUri)) {
    // Field is annotated with @AllowUnsafeRequiredFieldQualifier => allow
    return;
  }

  const sema_params& sema_parameters = ctx.sema_parameters();

  ctx.report(
      field,
      validation_to_diagnostic_level(sema_parameters.required_field_qualifier),
      "The 'required' qualifier is deprecated and ignored by most language "
      "implementations. Leave the field unqualified instead: `{}` (in `{}`).",
      field.name(),
      dynamic_cast<const t_structured&>(*ctx.parent()).name());
}

void validate_enum_value_name_uniqueness(
    sema_context& ctx, const t_enum& node) {
  redef_checker(ctx, "Enum value", node).check_all(node.values());
}

void validate_enum_value_uniqueness(sema_context& ctx, const t_enum& node) {
  std::unordered_map<int32_t, const t_enum_value*> values;
  for (const auto& value : node.values()) {
    auto prev = values.emplace(value.get_value(), &value);
    ctx.check(
        prev.second,
        value,
        "Duplicate value `{}={}` with value `{}` in enum `{}`.",
        value.name(),
        value.get_value(),
        prev.first->second->name(),
        node.name());
  }
}

void validate_enum_value(sema_context& ctx, const t_enum_value& node) {
  if (!node.has_value()) {
    ctx.error(
        "The enum value, `{}`, must have an explicitly assigned value.",
        node.name());
  }
}

void validate_const_type_and_value(sema_context& ctx, const t_const& node) {
  if (detail::check_initializer(ctx, node, node.type(), node.value())) {
    detail::check_duplicate_keys(ctx, node);
  }
  ctx.check(
      !node.has_structured_annotation(kCppAdapterUri) ||
          has_experimental_annotation(ctx, node),
      "Using adapters on const `{}` is only allowed in the experimental mode.",
      node.name());
}

std::string_view field_qualifier_to_string(t_field_qualifier qualifier) {
  switch (qualifier) {
    case t_field_qualifier::none:
      return "unqualified";
    case t_field_qualifier::required:
      return "required";
    case t_field_qualifier::optional:
      return "optional";
    case t_field_qualifier::terse:
      return "terse";
  }
  abort();
}

void validate_field_default_value(sema_context& ctx, const t_field& field) {
  if (field.default_value() == nullptr) {
    // Field does not have a default value => nothing to validate.
    return;
  }

  if (!detail::check_initializer(
          ctx, field, &field.type().deref(), field.default_value())) {
    // If initializer is not valid to begin with, stop checks and return error.
    return;
  }
  detail::check_duplicate_keys(ctx, field);

  // A custom default value is specified...

  const sema_params& sema_parameters = ctx.sema_parameters();
  const t_structured& parent_node =
      dynamic_cast<const t_structured&>(*ctx.parent());

  // Reminder: all union fields are implicitly optional
  if (parent_node.is<t_union>()) {
    // Allow if @thrift.AllowUnsafeUnionFieldCustomDefaultValue is specified.
    if (field.has_structured_annotation(
            kAllowUnsafeUnionFieldCustomDefaultValueUri)) {
      return;
    }

    ctx.report(
        field,
        validation_to_diagnostic_level(
            sema_parameters.union_field_custom_default),
        "Union field is implicitly optional and should not have custom "
        "default value: `{}` (in union `{}`).",
        field.name(),
        parent_node.name());
    return;
  }

  // field is in a struct or exception (not a union).

  const t_field_qualifier field_qualifier = field.qualifier();

  // Optional fields should not have custom default values.
  if (field_qualifier == t_field_qualifier::optional) {
    // Allow if @thrift.AllowUnsafeOptionalCustomDefaultValue is specified.
    if (field.has_structured_annotation(
            kAllowUnsafeOptionalCustomDefaultValueUri)) {
      return;
    }

    ctx.report(
        field,
        validation_to_diagnostic_level(
            sema_parameters.struct_optional_field_custom_default),
        "Optional field cannot have custom default value: `{}` (in `{}`).",
        field.name(),
        parent_node.name());

    return;
  }

  // struct/exception field is not optional, and has a custom default value.

  if (detail::is_initializer_default_value(
          field.type().deref(), *field.default_value())) {
    ctx.report(
        field,
        validation_to_diagnostic_level(
            sema_parameters.redundant_custom_default_values),
        "Explicit default value is redundant for ({}) field: "
        "`{}` (in `{}`).",
        field_qualifier_to_string(field_qualifier),
        field.name(),
        parent_node.name());
  }

  if (field_qualifier == t_field_qualifier::terse) {
    if (detail::is_initializer_default_value(
            field.type().deref(), *field.default_value())) {
      return;
    }
    ctx.error(
        field,
        "Terse field should not have custom default value: "
        "`{}` (in `{}`).",
        field.name(),
        parent_node.name());
  }
}

void validate_field_name(sema_context& ctx, const t_field& field) {
  const auto* strct = dynamic_cast<const t_structured*>(ctx.parent());
  if (field.name() == strct->name()) {
    std::string parent_structure;
    if (strct->is<t_union>()) {
      parent_structure = "union";
    } else if (strct->is<t_exception>()) {
      parent_structure = "exception";
    } else {
      parent_structure = "struct";
    }
    ctx.warning(
        "Field '{}' has the same name as the containing {}.",
        field.name(),
        parent_structure);
  }
}

void validate_structured_annotation(sema_context& ctx, const t_named& node) {
  std::unordered_map<const t_type*, const t_const*> seen;
  for (const t_const& annot : node.structured_annotations()) {
    auto [it, inserted] = seen.emplace(annot.type(), &annot);
    if (!inserted) {
      report_redef_diag(
          ctx,
          diagnostic_level::error,
          "Structured annotation",
          it->first->name(),
          node,
          annot,
          *it->second);
    }
    validate_const_type_and_value(ctx, annot);
  }
}

void validate_uri_uniqueness(sema_context& ctx, const t_program& prog) {
  // TODO: use string_view as map key
  std::unordered_map<std::string, const t_named*> uri_to_node;
  basic_ast_visitor<true> visit;
  visit.add_named_visitor([&](const t_named& node) {
    const auto& uri = node.uri();
    if (uri.empty() || uri == kTransitiveUri) {
      return;
    }
    auto result = uri_to_node.emplace(uri, &node);
    if (!result.second) {
      report_redef_diag(
          ctx,
          diagnostic_level::error,
          "Thrift URI",
          uri,
          node,
          node,
          *result.first->second);
    }
  });
  for (const auto* p : prog.get_included_programs()) {
    visit(*p);
  }
  visit(prog);
}

void validate_missing_uris(sema_context& ctx, const t_program& program) {
  const bool packageHasAnnotation =
      program.has_structured_annotation(kAllowLegacyMissingUris);

  const diagnostic_level unnecessary_allow_missing_uris_diagnostic_level =
      validation_to_diagnostic_level(
          ctx.sema_parameters().unnecessary_allow_missing_uris);
  bool hasNonAnnotatedDefinitionsWithMissingUris = false;
  const_ast_visitor ast_visitor;
  ast_visitor.add_root_definition_visitor([&](const t_named& node) {
    const bool nodeHasAnnotation =
        node.has_structured_annotation(kAllowLegacyMissingUris);
    if (!AstUriUtils::shouldHaveUri(node)) {
      if (nodeHasAnnotation) {
        ctx.report(
            node,
            unnecessary_allow_missing_uris_diagnostic_level,
            "Unnecessary use of @thrift.AllowLegacyMissingUris: `{}` does not "
            "require a URI.",
            node.name());
      }
      return;
    }

    // node should have a (non-empty) URI.

    if (!node.uri().empty()) {
      // Node has URI, as required.
      // Only need to check if the node has a (unnecessary) annotation, then
      // we're done.
      if (nodeHasAnnotation) {
        ctx.report(
            node,
            unnecessary_allow_missing_uris_diagnostic_level,
            "Unnecessary use of @thrift.AllowLegacyMissingUris: `{}` has a "
            "(non-empty) URI: {}",
            node.name(),
            node.uri());
      }
      return;
    }

    // URI is required, but missing...

    if (!nodeHasAnnotation) {
      hasNonAnnotatedDefinitionsWithMissingUris = true;
    }

    if (nodeHasAnnotation && packageHasAnnotation) {
      ctx.report(
          node,
          unnecessary_allow_missing_uris_diagnostic_level,
          "Unnecessary use of @thrift.AllowLegacyMissingUris on `{}`: the "
          "annotation is already applied at the package (i.e., file) level.",
          node.name());

      // If the unnecessary use is reported above, mark the package annotation
      // as needed (or else we would report warnings to remove both!)
      if (unnecessary_allow_missing_uris_diagnostic_level !=
          diagnostic_level::debug) {
        hasNonAnnotatedDefinitionsWithMissingUris = true;
      }
    }

    const bool shouldReportMissingUri =
        !packageHasAnnotation && !nodeHasAnnotation;
    if (shouldReportMissingUri) {
      ctx.report(
          node,
          validation_to_diagnostic_level(ctx.sema_parameters().missing_uris),
          "Definition `{}` requires a URI: add a non-empty package to the "
          "file, or annotate the type with @thrift.Uri. For more details, "
          "see https://fburl.com/thrift-uri-add-package",
          node.name());
    }
  });
  ast_visitor(program);

  if (packageHasAnnotation && !hasNonAnnotatedDefinitionsWithMissingUris) {
    ctx.report(
        program.package(),
        unnecessary_allow_missing_uris_diagnostic_level,
        "Unnecessary use of @thrift.AllowLegacyMissingUris at the package "
        "level: there are no types who are missing URIs in the file.");
  }
}

void validate_explicit_uri_value(sema_context& ctx, const t_named& node) {
  if (node.uri().empty() || !node.explicit_uri()) {
    // Empty URIs are valid. If a URI is not explicit, we validate the package
    // and emit errors on that if necessary. The compiler owns generating valid
    // implicit URIs for valid packages.
    // If we emit N errors for every implicit URI node in a file with an invalid
    // package (which is already an error), it just creates unnecessary noise.
    return;
  }
  try {
    validate_universal_name(node.uri());
  } catch (const std::exception& e) {
    ctx.error(node.src_range().begin, "{}", e.what());
  }
}

void validate_function_param_id(sema_context& ctx, const t_field& node) {
  if (node.explicit_id() != node.id()) {
    ctx.warning(
        node.src_range().begin, "No param id specified for `{}`", node.name());
  }
}

void validate_field_id(sema_context& ctx, const t_field& node) {
  ctx.check(
      node.explicit_id() == node.id(),
      "No field id specified for `{}`",
      node.name());

  ctx.check(
      node.id() != 0 ||
          node.has_unstructured_annotation(
              "cpp.deprecated_allow_zero_as_field_id"),
      "Zero value (0) not allowed as a field id for `{}`",
      node.name());

  ctx.check(
      node.id() >= t_field::min_id || node.is_injected(),
      "Reserved field id ({}) cannot be used for `{}`.",
      node.id(),
      node.name());
}

void validate_cpp_methods(sema_context& ctx, const t_structured& node) {
  if (node.has_unstructured_annotation("cpp.methods")) {
    if (has_lazy_field(node)) {
      ctx.error(
          "cpp.methods is incompatible with lazy deserialization in struct `{}`",
          node.name());
      return;
    }
    ctx.report(
        node,
        validation_to_diagnostic_level(
            ctx.sema_parameters().deprecated_cpp_methods),
        "cpp.methods is not supported");
  }
}

/**
 * Checks that the given field does not have both the (newer) structured
 * @cpp.Ref annotation and one of the legacy unstructured annotations (cpp.ref,
 * cpp.ref_type, etc.).
 */
void validate_ref_annotation(sema_context& ctx, const t_field& node) {
  const bool hasStructuredAnnotation =
      node.has_structured_annotation(kCppRefUri);

  const bool hasUnstructuredAnnotation = node.has_unstructured_annotation(
      {"cpp.ref", "cpp2.ref", "cpp.ref_type", "cpp2.ref_type"});

  const int count = hasStructuredAnnotation + hasUnstructuredAnnotation;
  if (count == 0) {
    // Neither @cpp.Ref nor cpp[2].ref[_type].
    // Check that there is no @cpp.AllowedLegacyNonOptionalRef
    ctx.check(
        !node.has_structured_annotation(kCppAllowLegacyNonOptionalRefUri),
        "Cannot annotate field with @cpp.AllowLegacyNonOptionalRef unless it "
        "is a reference field (i.e., @cpp.Ref): `{}`.",
        node.name());
    return;
  }

  ctx.check(
      count == 1,
      "The @cpp.Ref annotation cannot be combined with the `cpp.ref` or "
      "`cpp.ref_type` annotations. Remove one of the annotations from `{}`.",
      node.name());
}

void validate_cpp_adapter_annotation(sema_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kCppAdapterUri, "@cpp.Adapter", "name");
}

void validate_hack_adapter_annotation(sema_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kHackAdapterUri, "@hack.Adapter", "name");
}

void validate_hack_wrapper_annotation(sema_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kHackWrapperUri, "@hack.Wrapper", "name");
}
// Do not adapt a wrapped type
void validate_hack_wrapper_and_adapter_annotation(
    sema_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kHackAdapterUri, kHackWrapperUri, "@hack.Adapter", "@hack.Wrapper");
}

void validate_java_adapter_annotation(sema_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kJavaAdapterUri, "@java.Adapter", "adapterClassName");
}

void validate_java_wrapper_annotation(sema_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kJavaWrapperUri, "@java.Wrapper", "wrapperClassName");
}

void validate_java_wrapper_and_adapter_annotation(
    sema_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kJavaAdapterUri, kJavaWrapperUri, "@java.Adapter", "@java.Wrapper");
}

/**
 * Suggest @thrift.Box as a replacement for unique reference fields (@cpp.Ref,
 * etc.). Require it for adapted fields with a reference annotation.
 */
void validate_ref_unique_and_box_annotation(
    sema_context& ctx, const t_field& node) {
  const t_const* adapter_annotation =
      node.find_structured_annotation_or_null(kCppAdapterUri); // @cpp.Adapter

  if (!cpp2::is_unique_ref(&node)) {
    return;
  }

  if (node.has_unstructured_annotation({"cpp.ref", "cpp2.ref"})) {
    if (adapter_annotation) {
      ctx.error(
          "cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box "
          "annotation instead in `{}` with @cpp.Adapter.",
          node.name());
    } else if (node.qualifier() == t_field_qualifier::optional) {
      ctx.warning(
          "cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box "
          "annotation instead in `{}`.",
          node.name());
    }
  }
  if (node.has_unstructured_annotation({"cpp.ref_type", "cpp2.ref_type"})) {
    if (adapter_annotation) {
      ctx.error(
          "cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. "
          "Please use @thrift.Box annotation instead in `{}` with "
          "@cpp.Adapter.",
          node.name());
    } else if (node.qualifier() == t_field_qualifier::optional) {
      ctx.warning(
          "cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. "
          "Please use @thrift.Box annotation instead in `{}`.",
          node.name());
    }
  }
  if (node.has_structured_annotation(kCppRefUri)) {
    if (adapter_annotation) {
      ctx.error(
          "@cpp.Ref{{type = cpp.RefType.Unique}} is deprecated. Please use "
          "@thrift.Box annotation instead in `{}` with @cpp.Adapter.",
          node.name());
    } else if (node.qualifier() == t_field_qualifier::optional) {
      ctx.warning(
          "@cpp.Ref{{type = cpp.RefType.Unique}} is deprecated. Please use "
          "@thrift.Box annotation instead in `{}`.",
          node.name());
    }
  }
}

void validate_function_priority_annotation(
    sema_context& ctx, const t_node& node) {
  if (auto* priority = node.find_unstructured_annotation_or_null("priority")) {
    const std::string choices[] = {
        "HIGH_IMPORTANT", "HIGH", "IMPORTANT", "NORMAL", "BEST_EFFORT"};
    auto* end = choices + sizeof(choices) / sizeof(choices[0]);
    ctx.check(
        std::find(choices, end, *priority) != end,
        "Bad priority '{}'. Choose one of {}.",
        *priority,
        choices);
  }
}

void validate_function_exception_field_name_uniqueness(
    sema_context& ctx, const t_function& node) {
  if (node.exceptions() != nullptr) {
    redef_checker(ctx, "Exception field", node)
        .check_all(node.exceptions()->fields());
  }
}

void validate_exception_message_annotation(
    sema_context& ctx, const t_exception& node) {
  // Check that value of "message" annotation is
  // - a valid member of struct
  // - of type STRING
  const t_field* field = nullptr;
  for (const auto& f : node.fields()) {
    if (f.has_structured_annotation(kExceptionMessageUri)) {
      ctx.check(!field, f, "Duplicate message annotation.");
      field = &f;
    }
  }
  if (node.has_unstructured_annotation("message")) {
    ctx.check(!field, "Duplicate message annotation.");
    const std::string& v = node.get_unstructured_annotation("message");
    field = node.get_field_by_name(v);
    ctx.check(
        field,
        "member specified as exception 'message' should be a valid "
        "struct member, '{}' in '{}' is not",
        v,
        node.name());
  }
  if (field) {
    ctx.check(
        field->type().get_type()->is_string_or_binary(),
        "member specified as exception 'message' should be of type "
        "STRING, '{}' in '{}' is not",
        field->name(),
        node.name());

    if (field->name() != "message" && node.get_field_by_name("message")) {
      ctx.warning(
          "Some generators (e.g. PHP) will ignore annotation 'message' as it is "
          "also used as field");
    }
  }
}

void validate_interaction_nesting(
    sema_context& ctx, const t_interaction& node) {
  for (auto& func : node.functions()) {
    if (func.interaction()) {
      ctx.error(func, "Nested interactions are forbidden: {}", func.name());
    }
  }
}

void validate_interaction_annotations(
    sema_context& ctx, const t_interaction& node) {
  for (auto& func : node.functions()) {
    ctx.check(
        !func.has_unstructured_annotation("thread") &&
            !func.has_structured_annotation(kCppProcessInEbThreadUri),
        "Interaction methods cannot be individually annotated with "
        "thread='eb'. Use process_in_event_base on the interaction instead.");
  }
  if (node.has_unstructured_annotation("process_in_event_base") ||
      node.has_structured_annotation(kCppProcessInEbThreadUri)) {
    ctx.check(
        !node.has_unstructured_annotation("serial") &&
            !node.has_structured_annotation(kSerialUri),
        "EB interactions are already serial");
  }
}

void validate_cpp_deprecated_terse_write_annotation(
    sema_context& ctx, const t_field& field) {
  if (!field.has_structured_annotation(kCppDeprecatedTerseWriteUri)) {
    return;
  }
  if (field.qualifier() != t_field_qualifier::none) {
    ctx.error(
        field,
        "@cpp.DeprecatedTerseWrite can be only used on unqualified field.");
  }
  if (gen::cpp::find_ref_type(field) == gen::cpp::reference_type::unique) {
    return;
  }
  const t_type* type = field.type().get_type()->get_true_type();
  if (type->is<t_structured>()) {
    ctx.error(
        field,
        "@cpp.DeprecatedTerseWrite is not supported for structured types.");
  }
  return;
}

void validate_cpp_field_interceptor_annotation(
    sema_context& ctx, const t_field& field) {
  if (const t_const* annot =
          field.find_structured_annotation_or_null(kCppFieldInterceptorUri)) {
    try {
      annot->get_value_from_structured_annotation("name");
    } catch (const std::exception&) {
      ctx.error(
          "`@cpp.FieldInterceptor` cannot be used without `name` specified in `{}`.",
          field.name());
      return;
    }
  }
}

void validate_cpp_enum_type(sema_context& ctx, const t_enum& e) {
  if (const t_const* annot =
          e.find_structured_annotation_or_null(kCppEnumTypeUri)) {
    try {
      annot->get_value_from_structured_annotation("type");
    } catch (const std::exception&) {
      ctx.error(
          "`@cpp.EnumType` cannot be used without `type` specified in `{}`.",
          e.name());
      return;
    }
  }
}

void validate_cpp_field_adapter_annotation(
    sema_context& ctx, const t_field& field) {
  adapter_or_wrapper_checker(ctx).check(
      field,
      kCppAdapterUri,
      "@cpp.Adapter",
      false /* disallow_structured_annotations_on_both_field_and_typedef */);
}

void validate_hack_field_adapter_annotation(
    sema_context& ctx, const t_field& field) {
  adapter_or_wrapper_checker(ctx).check(
      field,
      kHackAdapterUri,
      "@hack.Adapter",
      true /* disallow_structured_annotations_on_both_field_and_typedef */);
}

void validate_java_field_adapter_annotation(
    sema_context& ctx, const t_field& field) {
  adapter_or_wrapper_checker(ctx).check(
      field,
      kJavaAdapterUri,
      "@java.Adapter",
      false /* disallow_structured_annotations_on_both_field_and_typedef */);
}

class reserved_ids_checker {
 public:
  explicit reserved_ids_checker(sema_context& ctx) : ctx_(ctx) {}

  void check(const t_structured& node) {
    auto reserved_ids = get_reserved_ids(node);
    check_out_of_range_ids(node, reserved_ids);
    for (const auto& field : node.fields()) {
      ctx_.check(
          reserved_ids.count(field.id()) == 0,
          "Fields in {} cannot use reserved ids: {}",
          node.name(),
          field.id());
    }
  }

  void check(const t_enum& node) {
    auto reserved_ids = get_reserved_ids(node);
    for (const auto& enum_value : node.values()) {
      ctx_.check(
          reserved_ids.count(enum_value.get_value()) == 0,
          "Enum values in {} cannot use reserved ids: {}",
          node.name(),
          enum_value.get_value());
    }
  }

 private:
  sema_context& ctx_;

  // Gets all the reserved ids annotated on this node. Returns
  // empty set if the annotation is not present.
  std::unordered_set<int32_t> get_reserved_ids(const t_type& node) {
    std::unordered_set<int32_t> reserved_ids;

    auto* annotation = node.find_structured_annotation_or_null(kReserveIdsUri);
    if (annotation == nullptr) {
      return reserved_ids;
    }

    // Take the union of the list of tag values in `ids` and the range of
    // of values from `id_ranges`
    if (auto ids =
            annotation->get_value_from_structured_annotation_or_null("ids");
        ids != nullptr) {
      ctx_.check(
          ids->kind() == t_const_value::t_const_value_kind::CV_LIST,
          "Field ids must be a list of integers, annotated on {}",
          node.name());
      for (const auto* id : ids->get_list_or_empty_map()) {
        ctx_.check(
            id->kind() == t_const_value::t_const_value_kind::CV_INTEGER,
            "Field ids must be a list of integers, annotated on {}",
            node.name());
        reserved_ids.insert(id->get_integer());
      }
    }
    if (auto id_ranges =
            annotation->get_value_from_structured_annotation_or_null(
                "id_ranges");
        id_ranges != nullptr) {
      ctx_.check(
          id_ranges->kind() == t_const_value::t_const_value_kind::CV_MAP,
          "Field id_ranges must be a map of integer to integer, annotated on {}",
          node.name());
      for (const auto& [id_range_begin, id_range_end] : id_ranges->get_map()) {
        ctx_.check(
            id_range_begin->kind() ==
                    t_const_value::t_const_value_kind::CV_INTEGER &&
                id_range_begin->kind() ==
                    t_const_value::t_const_value_kind::CV_INTEGER,
            "Field id_ranges must be a map of integer to integer, annotated on {}",
            node.name());
        ctx_.check(
            id_range_begin->get_integer() < id_range_end->get_integer(),
            "For each (start: end) in id_ranges, we must have start < end. Got ({}: {}), annotated on {}",
            id_range_begin->get_integer(),
            id_range_end->get_integer(),
            node.name());
        for (int i = id_range_begin->get_integer();
             i < id_range_end->get_integer();
             ++i) {
          reserved_ids.insert(i);
        }
      }
    }
    return reserved_ids;
  }

  void check_out_of_range_ids(
      const t_structured& node,
      const std::unordered_set<int32_t>& reserved_ids) {
    // Insert into std::set to make sure error message is deterministic
    std::set<int32_t> out_of_range_ids;
    for (auto id : reserved_ids) {
      if (id < std::numeric_limits<std::int16_t>::min() ||
          std::numeric_limits<std::int16_t>::max() < id) {
        out_of_range_ids.insert(id);
      }
    }
    for (auto id : out_of_range_ids) {
      ctx_.error(
          "Struct `{}` cannot have reserved id that is out of range: {}",
          node.name(),
          id);
    }
  }
};

void validate_reserved_ids_structured(
    sema_context& ctx, const t_structured& node) {
  reserved_ids_checker(ctx).check(node);
}

void validate_reserved_ids_enum(sema_context& ctx, const t_enum& node) {
  reserved_ids_checker(ctx).check(node);
}

bool owns_annotations(const t_type* type) {
  if (std::none_of(
          type->unstructured_annotations().begin(),
          type->unstructured_annotations().end(),
          [](const auto& a) {
            return a.second.from ==
                deprecated_annotation_value::origin::unstructured;
          })) {
    return false;
  }
  if (type->is<t_container>()) {
    return true;
  }
  if (type->is<t_primitive_type>()) {
    return true;
  }
  if (auto t = dynamic_cast<const t_typedef*>(type)) {
    return t->typedef_kind() != t_typedef::kind::defined;
  }
  return false;
}
bool owns_annotations(t_type_ref type) {
  return owns_annotations(type.get_type());
}

void validate_custom_cpp_type_annotations(
    sema_context& ctx, const t_named& node) {
  const bool hasAdapter = node.has_structured_annotation(kCppAdapterUri);
  bool hasCppType = node.has_unstructured_annotation(
      {"cpp.type", "cpp2.type", "cpp.template", "cpp2.template"});
  const bool hasStructuredCppType = node.has_structured_annotation(kCppTypeUri);

  ctx.check(
      !(hasCppType && hasAdapter),
      "Definition `{}` cannot have both cpp.type/cpp.template and @cpp.Adapter annotations",
      node.name());

  // Excludes annotations that result from annotation lowering.
  auto has_real_annotation = [](const auto& node) {
    if (!owns_annotations(node.type())) {
      return false;
    }
    std::set<std::string> names{
        "cpp.type", "cpp2.type", "cpp.template", "cpp2.template"};
    for (const auto& [k, v] :
         node.type().get_type()->unstructured_annotations()) {
      if (names.count(k) && v.src_range.begin != source_location{}) {
        return true;
      }
    }
    return false;
  };

  bool hasUnnamedCppType = false;
  if (auto f = dynamic_cast<const t_field*>(&node)) {
    if (has_real_annotation(*f)) {
      hasUnnamedCppType = true;
    }
  } else if (auto t = dynamic_cast<const t_typedef*>(&node)) {
    if (t->typedef_kind() == t_typedef::kind::defined &&
        has_real_annotation(*t)) {
      hasUnnamedCppType = true;
    }
  }
  if (hasAdapter && (hasUnnamedCppType || hasStructuredCppType)) {
    // TODO (T169470476): make this an error
    ctx.warning(
        "At most one of @cpp.Type/@cpp.Adapter/cpp.type/cpp.template can be specified on a definition.");
  }
  ctx.check(
      hasCppType + hasStructuredCppType + hasUnnamedCppType <= 1,
      "Duplicate cpp.Type annotation");
}

template <typename Node>
void validate_cpp_type_annotation(sema_context& ctx, const Node& node) {
  if (const t_const* annot =
          node.find_structured_annotation_or_null(kCppTypeUri)) {
    auto type = annot->get_value_from_structured_annotation_or_null("name");
    auto tmplate =
        annot->get_value_from_structured_annotation_or_null("template");
    if (!type == !tmplate) {
      ctx.error(
          "Exactly one of `name` and `template` must be specified for `@cpp.Type` on `{}`.",
          node.name());
    }
    if (tmplate) {
      if (!node.type()->get_true_type()->template is<t_container>()) {
        ctx.error(
            "`@cpp.Type{{template=...}}` can only be used on containers, not on `{}`.",
            node.name());
      }
    }
  }
}

/**
 * Checks that any @thrift.AllowUnsafeOptionalCustomDefaultValue and
 * @thrift.AllowUnsafeUnionFieldCustomDefaultValue annotation on the given
 * `field` are used in a valid context. Reports an error if not.
 */
void validate_field_specific_annotation_scopes(
    sema_context& ctx, const t_field& field) {
  const bool hasDefaultValue = field.default_value() != nullptr;
  const t_structured& parentNode =
      dynamic_cast<const t_structured&>(*ctx.parent());

  if (field.has_structured_annotation(
          kAllowUnsafeOptionalCustomDefaultValueUri)) {
    // @thrift.AllowUnsafeOptionalCustomDefaultValue MUST ONLY be on optional
    // fields, in strucs or exceptions (NOT unions), that have a custom default
    // value.
    const bool fieldHasOptionalQualifier =
        field.qualifier() == t_field_qualifier::optional;
    if (!fieldHasOptionalQualifier || !hasDefaultValue ||
        !(parentNode.is<t_struct>() || parentNode.is<t_exception>())) {
      ctx.error(
          field,
          "Field annotated with @thrift.AllowUnsafeOptionalCustomDefaultValue "
          "must be in a struct or exception, optional and have a custom "
          "default value: `{}` (in `{}`)",
          field.name(),
          parentNode.name());
    }
  }

  if (field.has_structured_annotation(
          kAllowUnsafeUnionFieldCustomDefaultValueUri)) {
    // @thrift.AllowUnsafeUnionFieldCustomDefaultValue/ MUST ONLY be on union
    // fields (NOT structs or exceptions) that have a custom default value.
    if (!hasDefaultValue || !parentNode.is<t_union>()) {
      ctx.error(
          field,
          "Field annotated with @thrift.AllowUnsafeUnionFieldCustomDefaultValue"
          "must be in a union and have a custom default value: `{}` (in `{}`)",
          field.name(),
          parentNode.name());
    }
  }
}

struct ValidateAnnotationPositions {
  void operator()(sema_context& ctx, const t_const& node) {
    if (owns_annotations(node.type())) {
      err(ctx);
    }
  }
  void operator()(sema_context& ctx, const t_typedef& node) {
    if (!ctx.sema_parameters().forbid_unstructured_annotations) {
      return;
    }
    if (owns_annotations(node.type())) {
      err(ctx);
    }
  }
  void operator()(sema_context& ctx, const t_function& node) {
    if (owns_annotations(node.return_type())) {
      err(ctx);
    }
    if (const auto* s = node.sink()) {
      if (owns_annotations(s->elem_type()) ||
          (s->final_response_type() &&
           owns_annotations(s->final_response_type()))) {
        err(ctx);
      }
    }
    if (const auto* s = node.stream()) {
      if (owns_annotations(s->elem_type())) {
        err(ctx);
      }
    }

    for (auto& field : node.params().fields()) {
      auto type = field.type();
      if (owns_annotations(type)) {
        err(ctx);
      }
    }

    if (auto* exs = node.exceptions()) {
      for (auto& ex : exs->fields()) {
        if (owns_annotations(ex.type())) {
          err(ctx);
        }
        if (!ex.unstructured_annotations().empty()) {
          err(ctx);
        }
      }
    }
  }
  void operator()(sema_context& ctx, const t_container& type) {
    if (const t_list* list = type.try_as<t_list>()) {
      if (owns_annotations(list->elem_type())) {
        err(ctx);
      }
    } else if (const t_set* set = type.try_as<t_set>()) {
      if (owns_annotations(set->elem_type())) {
        err(ctx);
      }
    } else if (const t_map* map = type.try_as<t_map>()) {
      if (owns_annotations(map->key_type()) ||
          owns_annotations(map->val_type())) {
        err(ctx);
      }
    } else {
      assert(false && "Unknown container type");
    }
  }
  void operator()(sema_context& ctx, const t_field& node) {
    if (owns_annotations(node.type()) &&
        std::any_of(
            node.type()->unstructured_annotations().begin(),
            node.type()->unstructured_annotations().end(),
            [](const auto& pair) {
              return pair.second.src_range.begin != source_location{};
            })) {
      err(ctx);
    }
  }

 private:
  static void err(sema_context& ctx) {
    ctx.error(
        "Annotations are not allowed in this position. Extract the type into a named typedef instead.");
  }
};

void deprecate_annotations(sema_context& ctx, const t_named& node) {
  auto erlang = [](std::string_view name) {
    return fmt::format("facebook.com/thrift/annotation/erlang/{}", name);
  };
  // cpp[2].ref[_type] are handled in dedicated validators.
  static std::map<std::string, std::string> deprecations = {
      {"cpp.type", kCppTypeUri},
      {"cpp2.type", kCppTypeUri},
      {"cpp.template", kCppTypeUri},
      {"cpp2.template", kCppTypeUri},
      {"cpp.box", kBoxUri},
      {"thrift.box", kBoxUri},
      {"hack.attributes", kHackAttributeUri},
      {"py3.hidden", kPythonPy3HiddenUri},
      {"py3.name", kPythonNameUri},
      {"py3.flags", kPythonFlagsUri},
      {"java.swift.mutable", kJavaMutableUri},
      {"java.swift.annotations", kJavaAnnotationUri},
      {"go.name", kGoNameUri},
      {"go.tag", kGoTagUri},
      {"cpp.coroutine", "Nothing, it's on by default"},
      {"cpp.name", kCppNameUri},
      {"code", "Nothing, it is a no-op"},
      {"message", kExceptionMessageUri},
      {"cpp.minimize_padding", kCppMinimizePaddingUri},
      {"cpp.enum_type", kCppEnumTypeUri},
      {"cpp2.enum_type", kCppEnumTypeUri},
      {"cpp.experimental.lazy", kCppLazyUri},
      {"cpp.mixin", kMixinUri},
      {"bitmask", kBitmaskEnumUri},
      {"cpp.declare_bitwise_ops", kBitmaskEnumUri},
      {"cpp2.declare_bitwise_ops", kBitmaskEnumUri},
      {"thread", kCppProcessInEbThreadUri},
      {"process_in_event_base", kCppProcessInEbThreadUri},
      {"cpp.declare_hash", kCppDeclareHashSpecialization},
      {"cpp2.declare_hash", kCppDeclareHashSpecialization},
      {"cpp.declare_equal_to", kCppDeclareEqualToSpecialization},
      {"cpp2.declare_equal_to", kCppDeclareEqualToSpecialization},
      {"hack.name", kHackNameUri},
      {"thrift.uri", kUriUri},
      {"serial", kSerialUri},
      {"priority", kPriorityUri},
      {"erl.name", erlang("NameOverride")},
      {"erl.struct_repr", erlang("StructRepr")},
      {"erl.default_value", erlang("DefaultValue")},
      {"iq.node_type", erlang("Iq")},
  };
  // Add a replacement to `deprecations` map if a removed unstructured
  // annotation has a structured annotation replacement.
  std::set<std::string> removed_annotations = {
      "code",
      "cpp.indirection",
      "cpp2.declare_bitwise_ops",
      "cpp.enum_type",
      "cpp2.enum_type",
      "cpp2.deprecated_enum_unscoped",
      "process_in_event_base",
  };
  std::map<std::string, std::string> removed_prefixes = {{"rust.", "rust"}};

  for (const auto& [k, v] : node.unstructured_annotations()) {
    // Exclude lowered type annotations.
    if (v.from == deprecated_annotation_value::origin::lowered_cpp_type) {
      continue;
    }
    std::optional<std::string> prefix;
    std::string lang;
    for (const auto& [p, l] : removed_prefixes) {
      if (k.find(p) == 0) {
        prefix = p;
        lang = l;
        break;
      }
    }
    bool directly_deprecated = deprecations.count(k) != 0;
    if (!prefix && !directly_deprecated) {
      if (removed_annotations.count(k) != 0) {
        ctx.error("invalid annotation {}", k);
        continue;
      }
      if (v.from == deprecated_annotation_value::origin::unstructured &&
          ctx.sema_parameters().forbid_unstructured_annotations) {
        ctx.error("Unstructured annotations are not allowed: `{}`.", k);
      }
      continue;
    }
    std::string replacement;
    if (directly_deprecated) {
      std::vector<std::string> parts;
      boost::split(parts, deprecations.at(k), [](char c) { return c == '/'; });
      if (parts.size() == 1) {
        replacement = parts[0];
      } else if (parts.size() == 4) {
        replacement = fmt::format("@thrift.{}", parts[3]);
      } else {
        assert(parts.size() == 5);
        replacement = fmt::format("@{}.{}", parts[3], parts[4]);
      }
    } else {
      replacement = fmt::format(
          "a structured annotation from thrift/annotation/{}.thrift", lang);
    }

    if (directly_deprecated &&
        node.has_structured_annotation(deprecations.at(k).c_str())) {
      ctx.error("Duplicate annotations {} and {}.", k, replacement);
    } else if (
        removed_annotations.count(k) != 0 || prefix ||
        (v.from == deprecated_annotation_value::origin::unstructured &&
         ctx.sema_parameters().forbid_unstructured_annotations)) {
      ctx.error(
          "The annotation {} has been removed. Please use {} instead.",
          k,
          replacement);
    } else {
      ctx.warning(
          "The annotation {} is deprecated. Please use {} instead.",
          k,
          replacement);
    }
  }
}
void deprecate_typedef_type_annotations(
    sema_context& ctx, const t_typedef& node) {
  if (owns_annotations(node.type())) {
    deprecate_annotations(ctx, *node.type());
  }
}

template <typename Node>
bool has_cursor_serialization_adapter(const Node& node) {
  try {
    if (auto* adapter = cpp_name_resolver::find_first_adapter(node)) {
      return adapter->find("apache::thrift::CursorSerializationAdapter") !=
          adapter->npos;
    }
  } catch (const std::runtime_error&) {
    // Adapter annotation is malformed, ignore it.
  }
  return false;
}

void validate_cursor_serialization_adapter_on_field(
    sema_context& ctx, const t_field& node) {
  ctx.check(
      !has_cursor_serialization_adapter(node),
      "CursorSerializationAdapter is not supported on fields. Place it on the top-level struct/union instead.");
}

void validate_cursor_serialization_adapter_on_function(
    sema_context& ctx, const t_function& node) {
  if (node.params().fields().size() <= 1) {
    return;
  }
  for (const auto& field : node.params().fields()) {
    ctx.check(
        !has_cursor_serialization_adapter(field),
        field,
        "CursorSerializationAdapter only supports single-argument functions.");
  }
}

void validate_cursor_serialization_adapter_in_container(
    sema_context& ctx, const t_container& node) {
  auto check = [&](const t_type& type) {
    ctx.check(
        !has_cursor_serialization_adapter(type),
        "CursorSerializationAdapter is not supported inside containers.");
  };
  if (auto* list = dynamic_cast<const t_list*>(&node)) {
    check(*list->elem_type());
  } else if (auto* set = dynamic_cast<const t_set*>(&node)) {
    check(*set->elem_type());
  } else if (auto* map = dynamic_cast<const t_map*>(&node)) {
    check(*map->key_type());
    check(*map->val_type());
  }
}

void validate_py3_enable_cpp_adapter(sema_context& ctx, const t_typedef& node) {
  if (node.has_structured_annotation(kPythonPy3EnableCppAdapterUri)) {
    const auto& true_type = *node.get_true_type();
    if (!true_type.is<t_container>() && !true_type.is_string_or_binary()) {
      ctx.error(
          "The @python.Py3EnableCppAdapter annotation can only be used on containers and strings.");
    }
    if (!node.has_structured_annotation(kCppAdapterUri)) {
      ctx.error(
          "The @python.Py3EnableCppAdapter annotation requires the @cpp.Adapter annotation to be present in the same typedef.");
    }
  }
}

void validate_nonallowed_typedef_with_uri(
    sema_context& ctx, const t_typedef& node) {
  if (node.uri().empty()) {
    // Typedef node does not have a URI => OK, return.
    return;
  }

  if (node.has_structured_annotation(kAllowLegacyTypedefUriUri)) {
    // Typedef node has a URI, but is annotated with
    // `@thrift.AllowLegacyTypedefUri`
    return;
  }

  ctx.report(
      node,
      validation_to_diagnostic_level(
          ctx.sema_parameters().nonallowed_typedef_with_uri),
      "Typedef `{}` has a URI, which is not allowed (see @thrift.AllowLegacyTypedefUri)",
      node.name());
}

// TODO (T191018859): forbid as field type too
void forbid_exception_as_method_type(
    sema_context& ctx, const t_function& node) {
  ctx.check(
      !node.return_type()->get_true_type()->is<t_exception>(),
      "Exceptions cannot be used as function return types");
  for (const auto& field : node.params().fields()) {
    ctx.check(
        !field.type()->get_true_type()->is<t_exception>(),
        "Exceptions cannot be used as function arguments");
  }
}
void forbid_exception_as_const_type(sema_context& ctx, const t_const& node) {
  ctx.check(
      !node.type()->get_true_type()->is<t_exception>(),
      "Exceptions cannot be used as const types");
}

} // namespace

ast_validator standard_validator() {
  ast_validator validator;
  validator.add_definition_visitor(&validate_identifier_is_not_reserved);
  validator.add_program_visitor(&validate_filename_is_not_reserved);
  validator.add_program_visitor(&validate_python_namespaces);
  validator.add_program_visitor(&validate_program_package);
  validator.add_program_visitor(&detail::validate_annotation_scopes<>);

  validator.add_root_definition_visitor(&detail::validate_annotation_scopes<>);
  validator.add_root_definition_visitor(&validate_explicit_uri_value);

  validator.add_interface_visitor(&validate_interface_function_name_uniqueness);
  validator.add_interface_visitor(&validate_function_priority_annotation);
  validator.add_service_visitor(
      &validate_extends_service_function_name_uniqueness);
  validator.add_interaction_visitor(&validate_interaction_nesting);
  validator.add_interaction_visitor(&validate_interaction_annotations);

  validator.add_thrown_exception_visitor(&validate_throws_exceptions);
  validator.add_thrown_exception_visitor(
      &detail::validate_annotation_scopes<
          detail::scope_check_type::thrown_exception>);

  validator.add_function_visitor(&validate_function_priority_annotation);
  validator.add_function_visitor(ValidateAnnotationPositions{});
  validator.add_function_visitor(&detail::validate_annotation_scopes<>);
  validator.add_function_visitor(
      &validate_function_exception_field_name_uniqueness);

  validator.add_structured_definition_visitor(&validate_field_names_uniqueness);
  validator.add_structured_definition_visitor(&validate_cpp_methods);
  validator.add_structured_definition_visitor(
      &validate_reserved_ids_structured);
  validator.add_structured_definition_visitor(
      &validate_exception_message_annotation_is_only_in_exceptions);
  validator.add_structured_definition_visitor(
      &validate_orderable_structured_types);

  validator.add_union_visitor(&validate_union_field_attributes);
  validator.add_exception_visitor(&validate_exception_message_annotation);
  validator.add_field_visitor(&validate_field_id);
  validator.add_field_visitor(&validate_mixin_field_attributes);
  validator.add_field_visitor(&validate_boxed_field_attributes);
  validator.add_field_visitor(&validate_field_default_value);
  validator.add_field_visitor(&validate_ref_annotation);
  validator.add_field_visitor(&validate_ref_unique_and_box_annotation);
  validator.add_field_visitor(&validate_cpp_field_adapter_annotation);
  validator.add_field_visitor(&validate_hack_field_adapter_annotation);
  validator.add_field_visitor(&validate_java_field_adapter_annotation);
  validator.add_field_visitor(&validate_cpp_field_interceptor_annotation);
  validator.add_field_visitor(&validate_cpp_deprecated_terse_write_annotation);
  validator.add_field_visitor(&validate_required_field);
  validator.add_field_visitor(&validate_cpp_type_annotation<t_field>);
  validator.add_field_visitor(&validate_field_name);
  validator.add_field_visitor(ValidateAnnotationPositions{});
  validator.add_field_visitor(&detail::validate_annotation_scopes<>);
  validator.add_field_visitor(&validate_field_specific_annotation_scopes);

  validator.add_enum_visitor(&validate_enum_value_name_uniqueness);
  validator.add_enum_visitor(&validate_enum_value_uniqueness);
  validator.add_enum_visitor(&validate_reserved_ids_enum);
  validator.add_enum_value_visitor(&validate_enum_value);

  validator.add_named_visitor(&validate_structured_annotation);
  validator.add_named_visitor(&validate_cpp_adapter_annotation);
  validator.add_named_visitor(&validate_hack_adapter_annotation);
  validator.add_named_visitor(&validate_hack_wrapper_annotation);
  validator.add_named_visitor(&validate_hack_wrapper_and_adapter_annotation);
  validator.add_named_visitor(&validate_java_adapter_annotation);
  validator.add_named_visitor(&validate_java_wrapper_annotation);
  validator.add_named_visitor(&validate_java_wrapper_and_adapter_annotation);
  validator.add_named_visitor(&validate_custom_cpp_type_annotations);
  validator.add_named_visitor(&deprecate_annotations);

  validator.add_function_param_visitor(
      &detail::validate_annotation_scopes<
          detail::scope_check_type::function_parameter>);
  validator.add_function_param_visitor(&validate_function_param_id);

  validator.add_typedef_visitor(&validate_cpp_type_annotation<t_typedef>);
  validator.add_typedef_visitor(&validate_py3_enable_cpp_adapter);
  validator.add_typedef_visitor(&deprecate_typedef_type_annotations);
  validator.add_typedef_visitor(ValidateAnnotationPositions());
  validator.add_typedef_visitor(&validate_nonallowed_typedef_with_uri);

  validator.add_container_visitor(ValidateAnnotationPositions());
  validator.add_enum_visitor(&validate_cpp_enum_type);
  validator.add_const_visitor(&validate_const_type_and_value);
  validator.add_const_visitor(ValidateAnnotationPositions());
  validator.add_program_visitor(&validate_uri_uniqueness);
  validator.add_program_visitor(&validate_missing_uris);

  validator.add_field_visitor(&validate_cursor_serialization_adapter_on_field);
  validator.add_function_visitor(
      &validate_cursor_serialization_adapter_on_function);
  validator.add_container_visitor(
      &validate_cursor_serialization_adapter_in_container);
  validator.add_function_visitor(&forbid_exception_as_method_type);

  validator.add_enum_value_visitor(&detail::validate_annotation_scopes<>);

  validator.add_const_visitor(&forbid_exception_as_const_type);

  add_explicit_include_validators(validator);

  validator.add_program_visitor(&report_resolution_mismatches);

  return validator;
}

} // namespace apache::thrift::compiler
