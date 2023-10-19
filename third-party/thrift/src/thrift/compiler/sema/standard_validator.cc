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

#include <fmt/ranges.h>

#include <thrift/compiler/ast/name_index.h>
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
#include <thrift/compiler/gen/cpp/reference_type.h>
#include <thrift/compiler/lib/cpp2/util.h>
#include <thrift/compiler/lib/uri.h>
#include <thrift/compiler/sema/const_checker.h>
#include <thrift/compiler/sema/scope_validator.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {
constexpr auto kCppUnstructuredAdapter = "cpp.adapter";
constexpr auto kHackUnstructuredAdapter = "hack.adapter";

const t_structured* get_mixin_type(const t_field& field) {
  if (cpp2::is_mixin(field)) {
    return dynamic_cast<const t_structured*>(field.type()->get_true_type());
  }
  return nullptr;
}

bool has_experimental_annotation(diagnostic_context& ctx, const t_named& node) {
  if (node.find_structured_annotation_or_null(kExperimentalUri)) {
    return true;
  }
  for (int pos = ctx.nodes().size() - 1; pos >= 0; --pos) {
    const auto* parent = dynamic_cast<const t_named*>(ctx.nodes().at(pos));
    if (parent != nullptr &&
        parent->find_structured_annotation_or_null(kExperimentalUri)) {
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

// Reports an existing name was redefined within the given parent node.
void report_redef_error(
    diagnostics_engine& diags,
    const char* kind,
    const std::string& name,
    const t_named& parent,
    const t_node& child,
    const t_node& /*existing*/) {
  // TODO(afuller): Use `existing` to provide more detail in the
  // diagnostic.
  diags.error(
      child, "{} `{}` is already defined for `{}`.", kind, name, parent.name());
}

// Helper for checking for the redefinition of a name in the context of a node.
class redef_checker {
 public:
  redef_checker(
      diagnostics_engine& diags, const char* kind, const t_named& parent)
      : diags_(diags), kind_(kind), parent_(parent) {}

  // Checks if the given `name`, derived from `node` via `child`, has already
  // been defined.
  //
  // For example, a mixin field causes all fields of the mixin type to be
  // inherited. In this case 'node' wold be the mixin type, from which `name`
  // was derived, while `child` is the mixin field that caused the name to be
  // inherited.
  void check(
      const std::string& name, const t_named& node, const t_node& child) {
    if (const auto* existing = seen_.put(name, node)) {
      if (&node == &parent_ && existing == &parent_) {
        // The degenerate case where parent_ is conflicting with itself.
        report_redef_error(diags_, kind_, name, parent_, child, *existing);
      } else {
        diags_.error(
            child,
            "{} `{}.{}` and `{}.{}` can not have same name in `{}`.",
            kind_,
            node.name(),
            name,
            existing->name(),
            name,
            parent_.name());
      }
    }
  }
  void check(std::string&&, const t_named&, const t_node&) = delete;
  void check(const std::string&, t_named&&, const t_node&) = delete;

  // Helpers for the common case where the names are from child t_nameds of
  // the parent.
  //
  // For example, all functions in an interface.
  void check(const t_named& child) {
    if (const auto* existing = seen_.put(child)) {
      report_redef_error(
          diags_, kind_, child.name(), parent_, child, *existing);
    }
  }
  void check(t_named&& child) = delete;

  template <typename Cs>
  void check_all(const Cs& children) {
    for (const t_named& child : children) {
      check(child);
    }
  }

 private:
  diagnostics_engine& diags_;
  const char* kind_;
  const t_named& parent_;

  name_index<t_named> seen_;
};

// Helper for validating the adapters
class adapter_or_wrapper_checker {
 public:
  explicit adapter_or_wrapper_checker(diagnostic_context& ctx) : ctx_(ctx) {}

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

  void check(
      const t_named&& node,
      const char* structured_adapter_annotation,
      const char* structured_adapter_annotation_error_name) = delete;

  // Do not allow composing structured annotation on field/typedef
  // and unstructured annotation on typedef/type
  void check(
      const t_field& field,
      const char* structured_adapter_annotation,
      const char* unstructured_adapter_annotation,
      const char* structured_adapter_annotation_error_name,
      bool disallow_structured_annotations_on_both_field_and_typedef) {
    if (!field.type().resolved()) {
      return;
    }
    auto type = field.type().get_type();

    bool structured_annotation_on_field =
        field.find_structured_annotation_or_null(structured_adapter_annotation);

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

    if (structured_annotation_on_typedef || structured_annotation_on_field) {
      if (t_typedef::get_first_annotation_or_null(
              type, {unstructured_adapter_annotation})) {
        ctx_.error(
            "`{}` cannot be combined with `{}` in `{}`.",
            structured_adapter_annotation_error_name,
            unstructured_adapter_annotation,
            field.name());
      }
    }
  }

  void check(
      const t_field&& field,
      const char* structured_adapter_annotation,
      const char* unstructured_adapter_annotation,
      const char* structured_adapter_annotation_error_name,
      bool disallow_structured_annotations_on_both_field_and_typedef) = delete;

  // If a type is wrapped itself or is a container of wrapped types, then it
  // cannot be adapted
  void check(
      const t_named& node,
      const char* structured_adapter_annotation,
      const char* structured_wrapper_annotation,
      const char* structured_adapter_annotation_error_name,
      const char* structured_wrapper_annotation_error_name) {
    bool has_adapter_annotation =
        node.find_structured_annotation_or_null(structured_adapter_annotation);
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
      type = typdef->get_type();
    } else {
      return;
    }

    auto has_wrapper =
        type->find_structured_annotation_or_null(structured_wrapper_annotation);
    std::string typedef_name = type->name();
    while (!has_wrapper) {
      if (const auto* inner_typedf = dynamic_cast<const t_typedef*>(type)) {
        has_wrapper = inner_typedf->find_structured_annotation_or_null(
            structured_wrapper_annotation);
        typedef_name = inner_typedf->name();
        type = inner_typedf->get_type();
      } else if (type->is_container()) {
        if (const auto* map = dynamic_cast<const t_map*>(type)) {
          type = map->get_val_type();
        } else if (const auto* list = dynamic_cast<const t_list*>(type)) {
          type = list->get_elem_type();
        } else {
          break;
        }
      } else if (type->is_struct()) {
        has_wrapper = type->find_structured_annotation_or_null(
            structured_wrapper_annotation);
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

  void check(
      const t_named&& node,
      const char* structured_adapter_annotation,
      const char* unstructured_adapter_annotation,
      const char* structured_adapter_annotation_error_name) = delete;

 private:
  diagnostic_context& ctx_;
};

struct service_metadata {
  name_index<t_service> function_name_to_service;

  service_metadata(node_metadata_cache& cache, const t_service& node) {
    if (node.extends() != nullptr) {
      // Add all the inherited functions.
      function_name_to_service =
          cache.get<service_metadata>(*node.extends()).function_name_to_service;
    }
    // Add all the directly defined functions.
    for (const auto& function : node.functions()) {
      function_name_to_service.put(function.name(), node);
    }
  }
};

struct structured_metadata {
  name_index<t_structured> field_name_to_parent;

  structured_metadata(node_metadata_cache& cache, const t_structured& node) {
    for (const auto& field : node.fields()) {
      if (const auto* mixin = get_mixin_type(field)) {
        // Add all the inherited mixin fields from field.
        auto mixin_metadata = cache.get<structured_metadata>(*mixin);
        field_name_to_parent.put_all(mixin_metadata.field_name_to_parent);
      }
      // Add the directly defined field.
      field_name_to_parent.put(field.name(), node);
    }
  }
};

void validate_interface_function_name_uniqueness(
    diagnostic_context& ctx, const t_interface& node) {
  // Check for a redefinition of a function in the same interface.
  redef_checker(ctx, "Function", node).check_all(node.functions());
}

// Checks for a redefinition of an inherited function.
void validate_extends_service_function_name_uniqueness(
    diagnostic_context& ctx, const t_service& node) {
  if (node.extends() == nullptr) {
    return;
  }

  const auto& extends_metadata =
      ctx.cache().get<service_metadata>(*node.extends());
  for (const auto& function : node.functions()) {
    if (const auto* existing_service =
            extends_metadata.function_name_to_service.find(function.name())) {
      ctx.error(
          function,
          "Function `{0}.{2}` redefines `{1}.{2}`.",
          node.name(),
          existing_service->get_full_name(),
          function.name());
    }
  }
}

void validate_throws_exceptions(diagnostic_context& ctx, const t_throws& node) {
  for (const auto& except : node.fields()) {
    auto except_type = except.type()->get_true_type();
    ctx.check(
        dynamic_cast<const t_exception*>(except_type),
        except,
        "Non-exception type, `{}`, in throws.",
        except_type->name());
  }
}

// Checks for a redefinition of a field in the same t_structured, including
// those inherited via mixin fields.
void validate_field_names_uniqueness(
    diagnostic_context& ctx, const t_structured& node) {
  redef_checker checker(ctx, "Field", node);
  for (const auto& field : node.fields()) {
    // Check the directly defined field.
    checker.check(field.name(), node, field);

    // Check any transtively defined fields via a mixin annotation.
    if (const auto* mixin = get_mixin_type(field)) {
      const auto& mixin_metadata = ctx.cache().get<structured_metadata>(*mixin);
      mixin_metadata.field_name_to_parent.for_each(
          [&](const std::string& name, const t_structured& parent) {
            checker.check(name, parent, field);
          });
    }
  }
}

// Checks the attributes of fields in a union.
void validate_union_field_attributes(
    diagnostic_context& ctx, const t_union& node) {
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
    } else if (field.find_structured_annotation_or_null(kTerseWriteUri)) {
      ctx.error(
          field,
          "`@thrift.TerseWrite` cannot be applied to union fields (in `{}`).",
          node.name());
    }
  }
}

void validate_boxed_field_attributes(
    diagnostic_context& ctx, const t_field& node) {
  if (gen::cpp::find_ref_type(node) == gen::cpp::reference_type::none) {
    return;
  }

  bool ref = node.has_annotation({
                 "cpp.ref",
                 "cpp2.ref",
                 "cpp.ref_type",
                 "cpp2.ref_type",
             }) ||
      node.find_structured_annotation_or_null(kCppRefUri);

  bool box = node.has_annotation({
                 "cpp.box",
                 "thrift.box",
             }) ||
      node.find_structured_annotation_or_null(kBoxUri);

  bool intern_box = node.find_structured_annotation_or_null(kInternBoxUri);

  if (ref + box + intern_box > 1) {
    ctx.error(
        node,
        "The {} annotation cannot be combined with the other reference annotations. "
        "Only annotate a single reference annotations from `{}`.",
        intern_box ? "`@thrift.InternBox`"
            : box  ? "`@thrift.Box`"
                   : "`@cpp.Ref`",
        node.name());
  }

  if (box) {
    ctx.check(
        dynamic_cast<const t_union*>(ctx.parent()) ||
            node.qualifier() == t_field_qualifier::optional,
        "The `thrift.box` annotation can only be used with optional fields. "
        "Make sure `{}` is optional.",
        node.name());
  }

  if (intern_box) {
    ctx.check(
        node.type()->get_true_type()->is_struct(),
        "The `@thrift.InternBox` annotation can only be used with a struct field.");
    // TODO(dokwon): Add support for custom defaults and remove this check.
    ctx.check(
        !node.get_value(),
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
void validate_mixin_field_attributes(
    diagnostic_context& ctx, const t_field& node) {
  if (!cpp2::is_mixin(node)) {
    return;
  }

  auto* ttype = node.type()->get_true_type();
  ctx.check(
      typeid(*ttype) == typeid(t_struct) || typeid(*ttype) == typeid(t_union),
      "Mixin field `{}` type must be a struct or union. Found `{}`.",
      node.name(),
      ttype->get_name());

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

void validate_required_field(diagnostic_context& ctx, const t_field& field) {
  if (field.qualifier() == t_field_qualifier::required) {
    ctx.warning(field, "Required field is deprecated: `{}`.", field.name());
  }
}

void validate_enum_value_name_uniqueness(
    diagnostic_context& ctx, const t_enum& node) {
  redef_checker(ctx, "Enum value", node).check_all(node.values());
}

void validate_enum_value_uniqueness(
    diagnostic_context& ctx, const t_enum& node) {
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

void validate_enum_value(diagnostic_context& ctx, const t_enum_value& node) {
  if (!node.has_value()) {
    ctx.error(
        "The enum value, `{}`, must have an explicitly assigned value.",
        node.name());
  }
}

void validate_const_type_and_value(
    diagnostic_context& ctx, const t_const& node) {
  check_const_rec(ctx, node, node.type(), node.value());
  ctx.check(
      !node.find_structured_annotation_or_null(kCppAdapterUri) ||
          has_experimental_annotation(ctx, node),
      "Using adapters on const `{}` is only allowed in the experimental mode.",
      node.name());
}

void validate_field_default_value(
    diagnostic_context& ctx, const t_field& field) {
  if (field.get_default_value() != nullptr) {
    check_const_rec(ctx, field, &field.type().deref(), field.default_value());
  }
}

void validate_structured_annotation(
    diagnostic_context& ctx, const t_named& node) {
  std::unordered_map<const t_type*, const t_const*> seen;
  for (const t_const& annot : node.structured_annotations()) {
    auto [it, inserted] = seen.emplace(annot.type(), &annot);
    if (!inserted) {
      report_redef_error(
          ctx,
          "Structured annotation",
          it->first->name(),
          node,
          annot,
          *it->second);
    }
    validate_const_type_and_value(ctx, annot);
  }
}

void validate_uri_uniqueness(diagnostic_context& ctx, const t_program& prog) {
  // TODO: use string_view as map key
  std::unordered_map<std::string, const t_named*> uri_to_node;
  basic_ast_visitor<true> visit;
  visit.add_definition_visitor([&](const t_named& node) {
    const auto& uri = node.uri();
    if (uri.empty() || uri == kTransitiveUri) {
      return;
    }
    auto result = uri_to_node.emplace(uri, &node);
    if (!result.second) {
      report_redef_error(
          ctx, "Thrift URI", uri, node, node, *result.first->second);
    }
  });
  for (const auto* p : prog.get_included_programs()) {
    visit(*p);
  }
  visit(prog);
}

void limit_terse_write_on_experimental_mode(
    diagnostic_context& ctx, const t_named& node) {
  ctx.check(
      !node.find_structured_annotation_or_null(kTerseWriteUri) ||
          has_experimental_annotation(ctx, node),
      "Using @thrift.TerseWrite on field `{}` is only allowed in the experimental mode.",
      node.name());
}

void validate_field_id(diagnostic_context& ctx, const t_field& node) {
  if (node.explicit_id() != node.id()) {
    ctx.warning(
        node,
        "No field id specified for `{}`, resulting protocol may have conflicts "
        "or not be backwards compatible!",
        node.name());
  }

  ctx.check(
      node.id() != 0 ||
          node.has_annotation("cpp.deprecated_allow_zero_as_field_id"),
      "Zero value (0) not allowed as a field id for `{}`",
      node.get_name());

  ctx.check(
      node.id() >= t_field::min_id || node.is_injected(),
      "Reserved field id ({}) cannot be used for `{}`.",
      node.id(),
      node.name());
}

void validate_compatibility_with_lazy_field(
    diagnostic_context& ctx, const t_structured& node) {
  if (has_lazy_field(node) && node.has_annotation("cpp.methods")) {
    ctx.error(
        "cpp.methods is incompatible with lazy deserialization in struct `{}`",
        node.get_name());
  }
}

void validate_ref_annotation(diagnostic_context& ctx, const t_field& node) {
  if (node.find_structured_annotation_or_null(kCppRefUri) &&
      node.has_annotation(
          {"cpp.ref", "cpp2.ref", "cpp.ref_type", "cpp2.ref_type"})) {
    ctx.error(
        "The @cpp.Ref annotation cannot be combined with the `cpp.ref` or "
        "`cpp.ref_type` annotations. Remove one of the annotations from `{}`.",
        node.name());
  }
}

void validate_cpp_adapter_annotation(
    diagnostic_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kCppAdapterUri, "@cpp.Adapter", "name");
}

void validate_hack_adapter_annotation(
    diagnostic_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kHackAdapterUri, "@hack.Adapter", "name");
}

void validate_hack_wrapper_annotation(
    diagnostic_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kHackWrapperUri, "@hack.Wrapper", "name");
}
// Do not adapt a wrapped type
void validate_hack_wrapper_and_adapter_annotation(
    diagnostic_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kHackAdapterUri, kHackWrapperUri, "@hack.Adapter", "@hack.Wrapper");
}

void validate_java_adapter_annotation(
    diagnostic_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kJavaAdapterUri, "@java.Adapter", "adapterClassName");
}

void validate_java_wrapper_annotation(
    diagnostic_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kJavaWrapperUri, "@java.Wrapper", "wrapperClassName");
}

void validate_java_wrapper_and_adapter_annotation(
    diagnostic_context& ctx, const t_named& node) {
  adapter_or_wrapper_checker(ctx).check(
      node, kJavaAdapterUri, kJavaWrapperUri, "@java.Adapter", "@java.Wrapper");
}

void validate_box_annotation(
    diagnostic_context& ctx, const t_structured& node) {
  if (node.generated()) {
    return;
  }

  for (const auto& field : node.fields()) {
    if (field.has_annotation({"cpp.box", "thrift.box"})) {
      ctx.warning(
          field,
          "cpp.box and thrift.box are deprecated. Please use @thrift.Box "
          "annotation instead in `{}`.",
          field.name());
    }
  }
}

void validate_ref_unique_and_box_annotation(
    diagnostic_context& ctx, const t_field& node) {
  const t_const* adapter_annotation =
      node.find_structured_annotation_or_null(kCppAdapterUri);

  if (cpp2::is_unique_ref(&node)) {
    if (node.has_annotation({"cpp.ref", "cpp2.ref"})) {
      if (adapter_annotation) {
        ctx.error(
            "cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box "
            "annotation instead in `{}` with @cpp.Adapter.",
            node.name());
      } else {
        ctx.warning(
            "cpp.ref, cpp2.ref are deprecated. Please use @thrift.Box "
            "annotation instead in `{}`.",
            node.name());
      }
    }
    if (node.has_annotation({"cpp.ref_type", "cpp2.ref_type"})) {
      if (adapter_annotation) {
        ctx.error(
            "cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. "
            "Please use @thrift.Box annotation instead in `{}` with "
            "@cpp.Adapter.",
            node.name());
      } else {
        ctx.warning(
            "cpp.ref_type = `unique`, cpp2.ref_type = `unique` are deprecated. "
            "Please use @thrift.Box annotation instead in `{}`.",
            node.name());
      }
    }
    if (node.find_structured_annotation_or_null(kCppRefUri) != nullptr) {
      if (adapter_annotation) {
        ctx.error(
            "@cpp.Ref{{type = cpp.RefType.Unique}} is deprecated. Please use "
            "@thrift.Box annotation instead in `{}` with @cpp.Adapter.",
            node.name());
      } else {
        ctx.warning(
            "@cpp.Ref{{type = cpp.RefType.Unique}} is deprecated. Please use "
            "@thrift.Box annotation instead in `{}`.",
            node.name());
      }
    }
  }
}

void validate_function_priority_annotation(
    diagnostic_context& ctx, const t_node& node) {
  if (auto* priority = node.find_annotation_or_null("priority")) {
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

void validate_exception_php_annotations(
    diagnostic_context& ctx, const t_exception& node) {
  constexpr const char* annotations[] = {"message", "code"};
  for (const auto& annotation : annotations) {
    if (node.get_field_by_name(annotation) != nullptr &&
        strcmp(annotation, node.get_annotation(annotation).c_str()) != 0) {
      ctx.warning(
          "Some generators (e.g. PHP) will ignore annotation '{}' as it is "
          "also used as field",
          annotation);
    }
  }

  // Check that value of "message" annotation is
  // - a valid member of struct
  // - of type STRING
  if (node.has_annotation("message")) {
    const std::string& v = node.get_annotation("message");
    const auto* field = node.get_field_by_name(v);
    if (field == nullptr) {
      ctx.error(
          "member specified as exception 'message' should be a valid "
          "struct member, '{}' in '{}' is not",
          v,
          node.name());
    } else if (!field->get_type()->is_string_or_binary()) {
      ctx.error(
          "member specified as exception 'message' should be of type "
          "STRING, '{}' in '{}' is not",
          v,
          node.name());
    }
  }
}

void validate_interaction_nesting(
    diagnostic_context& ctx, const t_interaction& node) {
  for (auto* func : node.get_functions()) {
    if (func->interaction()) {
      ctx.error(*func, "Nested interactions are forbidden: {}", func->name());
    }
  }
}

void validate_cpp_field_interceptor_annotation(
    diagnostic_context& ctx, const t_field& field) {
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

void validate_cpp_enum_type(diagnostic_context& ctx, const t_enum& e) {
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
    diagnostic_context& ctx, const t_field& field) {
  adapter_or_wrapper_checker(ctx).check(
      field,
      kCppAdapterUri,
      kCppUnstructuredAdapter,
      "@cpp.Adapter",
      false /* disallow_structured_annotations_on_both_field_and_typedef */);
}

void validate_hack_field_adapter_annotation(
    diagnostic_context& ctx, const t_field& field) {
  adapter_or_wrapper_checker(ctx).check(
      field,
      kHackAdapterUri,
      kHackUnstructuredAdapter,
      "@hack.Adapter",
      true /* disallow_structured_annotations_on_both_field_and_typedef */);
}

void validate_java_field_adapter_annotation(
    diagnostic_context& ctx, const t_field& field) {
  adapter_or_wrapper_checker(ctx).check(
      field,
      kJavaAdapterUri,
      kJavaAdapterUri,
      "@java.Adapter",
      false /* disallow_structured_annotations_on_both_field_and_typedef */);
}

class reserved_ids_checker {
 public:
  explicit reserved_ids_checker(diagnostic_context& ctx) : ctx_(ctx) {}

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
  diagnostic_context& ctx_;

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
          ids->get_type() == t_const_value::t_const_value_type::CV_LIST,
          "Field ids must be a list of integers, annotated on {}",
          node.name());
      for (const auto* id : ids->get_list()) {
        ctx_.check(
            id->get_type() == t_const_value::t_const_value_type::CV_INTEGER,
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
          id_ranges->get_type() == t_const_value::t_const_value_type::CV_MAP,
          "Field id_ranges must be a map of integer to integer, annotated on {}",
          node.name());
      for (const auto& [id_range_begin, id_range_end] : id_ranges->get_map()) {
        ctx_.check(
            id_range_begin->get_type() ==
                    t_const_value::t_const_value_type::CV_INTEGER &&
                id_range_begin->get_type() ==
                    t_const_value::t_const_value_type::CV_INTEGER,
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
    diagnostic_context& ctx, const t_structured& node) {
  reserved_ids_checker(ctx).check(node);
}

void validate_reserved_ids_enum(diagnostic_context& ctx, const t_enum& node) {
  reserved_ids_checker(ctx).check(node);
}

void validate_custom_cpp_type_annotations(
    diagnostic_context& ctx, const t_named& node) {
  const bool hasAdapter =
      node.find_structured_annotation_or_null(kCppAdapterUri);
  const bool hasCppType = node.has_annotation(
      {"cpp.type", "cpp2.type", "cpp.template", "cpp2.template"});
  ctx.check(
      !(hasCppType && hasAdapter),
      "Definition `{}` cannot have both cpp.type/cpp.template and @cpp.Adapter annotations",
      node.name());
}

template <typename Node>
void validate_cpp_type_annotation(diagnostic_context& ctx, const Node& node) {
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
      if (!node.type()->get_true_type()->is_container()) {
        ctx.error(
            "`@cpp.Type{{template=...}}` can only be used on containers, not on `{}`.",
            node.name());
      }
    }
  }
}

bool owns_annotations_type(const t_type* type) {
  if (type->annotations().empty()) {
    return false;
  }
  if (dynamic_cast<const t_container*>(type)) {
    return true;
  }
  if (dynamic_cast<const t_base_type*>(type)) {
    return true;
  }
  if (auto t = dynamic_cast<const t_typedef*>(type)) {
    return t->typedef_kind() != t_typedef::kind::defined;
  }
  return false;
}

bool owns_annotations(t_type_ref type) {
  return owns_annotations_type(type.get_type());
}

struct ValidateAnnotationPositions {
  void operator()(diagnostic_context& ctx, const t_const& node) {
    if (owns_annotations_type(node.type())) {
      err(ctx);
    }
  }
  void operator()(diagnostic_context& ctx, const t_function& node) {
    if (owns_annotations(node.return_type())) {
      err(ctx);
    }
    if (const auto* s = node.sink()) {
      if (owns_annotations(s->elem_type()) ||
          owns_annotations(s->final_response_type())) {
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
  }
  void operator()(diagnostic_context& ctx, const t_container& type) {
    switch (type.get_type_value()) {
      case t_type::type::t_list:
        if (owns_annotations(static_cast<const t_list&>(type).elem_type())) {
          err(ctx);
        }
        break;
      case t_type::type::t_set:
        if (owns_annotations(static_cast<const t_set&>(type).elem_type())) {
          err(ctx);
        }
        break;
      case t_type::type::t_map: {
        const auto& t = static_cast<const t_map&>(type);
        if (owns_annotations(t.key_type()) || owns_annotations(t.val_type())) {
          err(ctx);
        }
        break;
      }
      default:
        assert(false && "Unknown container type");
    }
  }

 private:
  static void err(diagnostic_context& ctx) {
    ctx.error(
        "Annotations are not allowed in this position. Extract the type into a named typedef instead.");
  }
};

} // namespace

ast_validator standard_validator() {
  ast_validator validator;
  validator.add_interface_visitor(&validate_interface_function_name_uniqueness);
  validator.add_interface_visitor(&validate_function_priority_annotation);
  validator.add_service_visitor(
      &validate_extends_service_function_name_uniqueness);
  validator.add_interaction_visitor(&validate_interaction_nesting);
  validator.add_throws_visitor(&validate_throws_exceptions);
  validator.add_function_visitor(&validate_function_priority_annotation);
  validator.add_function_visitor(ValidateAnnotationPositions{});

  validator.add_structured_definition_visitor(&validate_field_names_uniqueness);
  validator.add_structured_definition_visitor(
      &validate_compatibility_with_lazy_field);
  validator.add_structured_definition_visitor(&validate_box_annotation);
  validator.add_structured_definition_visitor(
      &validate_reserved_ids_structured);
  validator.add_union_visitor(&validate_union_field_attributes);
  validator.add_exception_visitor(&validate_exception_php_annotations);
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
  validator.add_field_visitor(&validate_required_field);
  validator.add_field_visitor([](auto& ctx, const auto& node) {
    validate_cpp_type_annotation(ctx, node);
  });

  validator.add_enum_visitor(&validate_enum_value_name_uniqueness);
  validator.add_enum_visitor(&validate_enum_value_uniqueness);
  validator.add_enum_visitor(&validate_reserved_ids_enum);
  validator.add_enum_value_visitor(&validate_enum_value);

  validator.add_definition_visitor(&validate_structured_annotation);
  validator.add_definition_visitor(&validate_annotation_scopes);
  validator.add_definition_visitor(&validate_cpp_adapter_annotation);
  validator.add_definition_visitor(&validate_hack_adapter_annotation);
  validator.add_definition_visitor(&validate_hack_wrapper_annotation);
  validator.add_definition_visitor(
      &validate_hack_wrapper_and_adapter_annotation);
  validator.add_definition_visitor(&validate_java_adapter_annotation);
  validator.add_definition_visitor(&validate_java_wrapper_annotation);
  validator.add_definition_visitor(
      &validate_java_wrapper_and_adapter_annotation);
  validator.add_definition_visitor(&limit_terse_write_on_experimental_mode);
  validator.add_definition_visitor(&validate_custom_cpp_type_annotations);

  validator.add_typedef_visitor([](auto& ctx, const auto& node) {
    validate_cpp_type_annotation(ctx, node);
  });
  validator.add_container_visitor(ValidateAnnotationPositions());
  validator.add_enum_visitor(&validate_cpp_enum_type);
  validator.add_const_visitor(&validate_const_type_and_value);
  validator.add_const_visitor(ValidateAnnotationPositions());
  validator.add_program_visitor(&validate_uri_uniqueness);
  return validator;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
