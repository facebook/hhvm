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

#include <thrift/compiler/sema/standard_mutator.h>

#include <functional>
#include <type_traits>

#include <thrift/compiler/lib/cpp2/util.h>
#include <thrift/compiler/lib/schematizer.h>
#include <thrift/compiler/lib/uri.h>
#include <thrift/compiler/sema/patch_mutator.h>
#include <thrift/compiler/sema/standard_mutator_stage.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

void match_type_with_const_value(
    diagnostic_context& ctx,
    const t_program& program,
    const t_type* long_type,
    t_const_value* value) {
  const t_type* type = long_type->get_true_type();
  if (!type) {
    // type is unresolved, which will fail in validation.
    return;
  }

  switch (type->get_type_value()) {
    case t_type::type::t_list: {
      auto* elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
      for (auto list_val : value->get_list()) {
        match_type_with_const_value(ctx, program, elem_type, list_val);
      }
      break;
    }
    case t_type::type::t_set: {
      auto* elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
      for (auto set_val : value->get_list()) {
        match_type_with_const_value(ctx, program, elem_type, set_val);
      }
      break;
    }
    case t_type::type::t_map: {
      auto* key_type = dynamic_cast<const t_map*>(type)->get_key_type();
      auto* val_type = dynamic_cast<const t_map*>(type)->get_val_type();
      for (auto map_val : value->get_map()) {
        match_type_with_const_value(ctx, program, key_type, map_val.first);
        match_type_with_const_value(ctx, program, val_type, map_val.second);
      }
      break;
    }
    case t_type::type::t_structured: {
      const auto* structured = dynamic_cast<const t_structured*>(type);
      if (auto ttype = value->ttype();
          ttype && ttype->get_true_type() != type) {
        ctx.error(
            value->ref_range().begin,
            "type mismatch: expected {}, got {}",
            type->get_full_name(),
            ttype->get_full_name());
      }
      for (const auto& [map_key, map_val] : value->get_map()) {
        bool resolved = map_key->kind() != t_const_value::CV_IDENTIFIER;
        auto name =
            resolved ? map_key->get_string() : map_key->get_identifier();
        auto field = structured->get_field_by_name(name);
        if (!field) {
          // Error reported by const_checker.
          return;
        }
        if (!resolved) {
          map_key->set_string(name);
        }
        match_type_with_const_value(ctx, program, field->get_type(), map_val);
      }
      break;
    }
    case t_type::type::t_enum:
      // Set constant value types as enums when they are declared with integers
      // or identifiers.
      if (!value->is_enum()) {
        value->set_is_enum();
        auto enm = dynamic_cast<const t_enum*>(type);
        value->set_enum(enm);
        if (value->kind() == t_const_value::CV_INTEGER) {
          if (const auto* enum_value = enm->find_value(value->get_integer())) {
            value->set_enum_value(enum_value);
          }
        } else if (value->kind() == t_const_value::CV_IDENTIFIER) {
          // Resolve enum values defined after use.
          const std::string& id = value->get_identifier();
          const t_const* constant = program.scope()->find_constant(id);
          if (!constant) {
            constant =
                program.scope()->find_constant(value->program().scope_name(id));
          }
          if (!constant) {
            // Try to resolve enum values from typedefs.
            auto last_dot_pos = id.find_last_of('.');
            std::string enum_name = id.substr(0, last_dot_pos);
            if (std::count(enum_name.begin(), enum_name.end(), '.') == 0) {
              enum_name = value->program().scope_name(enum_name);
            }
            if (auto* def = dynamic_cast<const t_type*>(
                    program.scope()->find(enum_name))) {
              if (auto* enum_def =
                      dynamic_cast<const t_enum*>(def->get_true_type())) {
                constant =
                    enum_def->find_const_by_name(id.substr(last_dot_pos + 1));
              }
            }
          }
          if (!constant) {
            ctx.error(
                value->ref_range().begin,
                "use of undeclared identifier '{}'",
                id);
            return;
          }
          value->assign(t_const_value(*constant->value()));
        }
      }
      break;
    case t_type::type::t_bool:
    case t_type::type::t_byte:
    case t_type::type::t_i16:
    case t_type::type::t_i32:
    case t_type::type::t_i64:
    case t_type::type::t_float:
    case t_type::type::t_double:
    case t_type::type::t_string:
    case t_type::type::t_binary:
      // Remove enum_value if type is a base_type to use the integer instead.
      if (value->is_enum()) {
        value->set_enum_value(nullptr);
      }
      break;
    case t_type::type::t_void:
    case t_type::type::t_service:
    case t_type::type::t_stream:
    case t_type::type::t_program:
      assert(false);
  }

  value->set_ttype(t_type_ref::from_req_ptr(type));
}

void maybe_match_type_with_const_value(
    diagnostic_context& ctx,
    mutator_context& mCtx,
    const t_type* type,
    t_const_value* value) {
  if (type == nullptr || value == nullptr) {
    return;
  }

  match_type_with_const_value(ctx, mCtx.program(), type, value);
}

void match_const_type_with_value(
    diagnostic_context& ctx, mutator_context& mCtx, t_const& const_node) {
  maybe_match_type_with_const_value(
      ctx, mCtx, const_node.type(), const_node.value());
}

void match_field_type_with_default_value(
    diagnostic_context& ctx, mutator_context& mCtx, t_field& field_node) {
  maybe_match_type_with_const_value(
      ctx, mCtx, field_node.get_type(), field_node.get_default_value());
}

static void match_annotation_types_with_const_values(
    diagnostic_context& ctx, mutator_context& mCtx, t_named& node) {
  for (t_const& tconst : node.structured_annotations()) {
    maybe_match_type_with_const_value(ctx, mCtx, tconst.type(), tconst.value());
  }
}

// Only an unqualified field is eligible for terse write.
void mutate_terse_write_annotation_structured(
    diagnostic_context& ctx, mutator_context&, t_structured& node) {
  bool program_has_terse_write =
      ctx.program().inherit_annotation_or_null(node, kTerseWriteUri);
  for (auto& field : node.fields()) {
    bool field_has_terse_write =
        field.find_structured_annotation_or_null(kTerseWriteUri);
    if (!field_has_terse_write && !program_has_terse_write) {
      continue;
    }
    if (field.qualifier() == t_field_qualifier::none) {
      field.set_qualifier(t_field_qualifier::terse);
    } else if (field_has_terse_write) {
      ctx.error(
          field, "`@thrift.TerseWrite` cannot be used with qualified fields");
    }
  }
}

void mutate_inject_metadata_fields(
    diagnostic_context& ctx, mutator_context&, t_structured& node) {
  // TODO(dokwon): Currently field injection doesn't work for structs used as
  // transitive annotations. Skipping as a workaround.
  if (is_transitive_annotation(node)) {
    return;
  }

  const t_const* annotation =
      node.find_structured_annotation_or_null(kInjectMetadataFieldsUri);
  if (!annotation) {
    return;
  }

  std::string type_string;
  try {
    type_string =
        annotation->get_value_from_structured_annotation("type").get_string();
  } catch (const std::exception& e) {
    ctx.error("{}", e.what());
    return;
  }
  // If the specified type and annotation are from the same program, append
  // the current program name.
  if (type_string.find(".") == std::string::npos) {
    type_string = annotation->program()->name() + "." + type_string;
  }

  const auto* ttype = node.program()->scope()->find_type(type_string);
  if (!ttype) {
    ctx.error(
        "Can not find expected type `{}` specified in "
        "`@internal.InjectMetadataFields` in the current scope. "
        "Please check the include.",
        type_string);
    return;
  }

  const auto* structured = dynamic_cast<const t_structured*>(ttype);
  // We only allow injecting fields from a struct type.
  if (structured == nullptr || ttype->is_union() || ttype->is_exception() ||
      ttype->is_paramlist()) {
    ctx.error(
        "`{}` is not a struct type. `@internal.InjectMetadataFields` can be "
        "only used with a struct type.",
        type_string);
    return;
  }
  for (const auto& field : structured->fields()) {
    t_field_id injected_id;
    try {
      injected_id = cpp2::get_internal_injected_field_id(field.id());
    } catch (const std::exception& e) {
      ctx.error("{}", e.what());
      // Iterate all fields to find more errors.
      continue;
    }
    std::unique_ptr<t_field> cloned_field = field.clone_DO_NOT_USE();
    cloned_field->set_injected_id(injected_id);
    ctx.check(
        node.try_append_field(cloned_field),
        "Field id `{}` is already used in `{}`.",
        field.id(),
        node.name());
  }
}

void set_generated(diagnostic_context&, mutator_context&, t_named& node) {
  if (node.find_structured_annotation_or_null(kSetGeneratedUri)) {
    node.set_generated();
  }
}

void lower_deprecated_annotations(
    diagnostic_context& ctx, mutator_context&, t_named& node) {
  if (auto cnst = node.find_structured_annotation_or_null(
          kDeprecatedUnvalidatedAnnotationsUri)) {
    ctx.check(
        node.annotations().empty(),
        "Cannot combine @thrift.DeprecatedUnvalidatedAnnotations with legacy annotation syntax.");
    auto val = cnst->get_value_from_structured_annotation_or_null("items");
    if (!val || val->get_map().empty()) {
      ctx.error(
          "Must specify at least one item in @thrift.DeprecatedUnvalidatedAnnotations.");
      return;
    }
    deprecated_annotation_map map;
    for (auto& [k, v] : val->get_map()) {
      map[k->get_string()] = {{}, v->get_string()};
    }
    node.reset_annotations(std::move(map));
  }
}

void normalize_return_type(
    diagnostic_context& ctx, mutator_context&, t_function& node) {
  if (!node.has_return_type()) {
    return;
  }
  auto& type = node.return_type();
  if (!type.resolve()) {
    ctx.error(node, "Failed to resolve return type of `{}`.", node.name());
    return;
  }

  // Check the (first) response type.
  const t_type* true_type = type->get_true_type();
  if (dynamic_cast<const t_service*>(true_type) ||
      dynamic_cast<const t_exception*>(true_type)) {
    ctx.error("Invalid first response type: {}", type->get_full_name());
  }
}

template <typename Nde>
void generate_runtime_schema(
    diagnostic_context& ctx,
    mutator_context&,
    bool annotation_required,
    std::string schemaTypeUri,
    Nde node,
    std::function<std::unique_ptr<apache::thrift::compiler::t_const_value>()>
        generator) {
  const t_const* annotation =
      node.find_structured_annotation_or_null(kGenerateRuntimeSchemaUri);
  if (annotation_required && !annotation) {
    return;
  }

  std::string name;
  if (auto nameOverride = annotation
          ? annotation->get_value_from_structured_annotation_or_null("name")
          : nullptr) {
    name = nameOverride->get_string();
  } else {
    name = fmt::format("schema{}", node.name());
  }

  auto program = const_cast<t_program*>(node.program());
  auto schemaType =
      dynamic_cast<const t_type*>(program->scope()->find_by_uri(schemaTypeUri));
  if (!schemaType) {
    ctx.error("Must include thrift/lib/thrift/schema.thrift");
    return;
  }

  std::unique_ptr<apache::thrift::compiler::t_const_value> schema = generator();

  auto schemaConst = std::make_unique<t_const>(
      program, schemaType, std::move(name), std::move(schema));
  schemaConst->set_generated();
  schemaConst->set_src_range(node.src_range());
  program->add_definition(std::move(schemaConst));
}

void generate_struct_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_structured& node) {
  generate_runtime_schema<t_structured&>(
      ctx, mCtx, true, "facebook.com/thrift/type/Struct", node, [&]() {
        return schematizer(mCtx.bundle).gen_schema(node);
      });
}

void generate_union_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_union& node) {
  generate_runtime_schema<t_union&>(
      ctx, mCtx, true, "facebook.com/thrift/type/Union", node, [&]() {
        return schematizer(mCtx.bundle).gen_schema(node);
      });
}

void generate_exception_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_exception& node) {
  generate_runtime_schema<t_exception&>(
      ctx, mCtx, true, "facebook.com/thrift/type/Exception", node, [&]() {
        return schematizer(mCtx.bundle).gen_schema(node);
      });
}

void generate_service_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_service& node) {
  generate_runtime_schema<t_service&>(
      ctx, mCtx, true, "facebook.com/thrift/type/Schema", node, [&]() {
        return schematizer(mCtx.bundle).gen_full_schema(node);
      });
}

void generate_const_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_const& node) {
  generate_runtime_schema<t_const&>(
      ctx, mCtx, true, "facebook.com/thrift/type/Const", node, [&]() {
        return schematizer(mCtx.bundle).gen_schema(node);
      });
}

void generate_typedef_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_typedef& node) {
  generate_runtime_schema<t_typedef&>(
      ctx, mCtx, true, "facebook.com/thrift/type/Typedef", node, [&]() {
        return schematizer(mCtx.bundle).gen_schema(node);
      });
}

void generate_enum_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_enum& node) {
  generate_runtime_schema<t_enum&>(
      ctx, mCtx, true, "facebook.com/thrift/type/Enum", node, [&]() {
        return schematizer(mCtx.bundle).gen_schema(node);
      });
}

template <typename Node>
void lower_type_annotations(
    diagnostic_context&, mutator_context& mCtx, Node& node) {
  std::map<std::string, std::string> unstructured;

  if (const t_const* annot =
          node.find_structured_annotation_or_null(kCppTypeUri)) {
    if (auto type =
            annot->get_value_from_structured_annotation_or_null("name")) {
      unstructured.insert({"cpp.type", type->get_string()});
    } else if (
        auto tmplate =
            annot->get_value_from_structured_annotation_or_null("template")) {
      unstructured.insert({"cpp.template", tmplate->get_string()});
    }
  }

  if (unstructured.empty()) {
    return;
  }

  const t_type* node_type = node.get_type();
  if (node_type->is_container() ||
      (node_type->is_typedef() &&
       static_cast<const t_typedef*>(node_type)->typedef_kind() !=
           t_typedef::kind::defined) ||
      (node_type->is_base_type() && !node_type->annotations().empty())) {
    // This is a new type we can modify in place
    for (auto& pair : unstructured) {
      const_cast<t_type*>(node_type)->set_annotation(pair.first, pair.second);
    }
  } else if (node_type->is_base_type()) {
    // Copy type as we don't handle unnamed typedefs to base types :(
    auto& program = mCtx.program();
    auto unnamed = std::make_unique<t_base_type>(
        *static_cast<const t_base_type*>(node_type));
    for (auto& pair : unstructured) {
      unnamed->set_annotation(pair.first, pair.second);
    }
    node.set_type(t_type_ref::from_ptr(unnamed.get()));
    program.add_unnamed_type(std::move(unnamed));
  } else {
    // Wrap in an unnamed typedef :(
    auto& program = mCtx.program();
    auto unnamed = t_typedef::make_unnamed(
        const_cast<t_program*>(node_type->get_program()),
        node_type->get_name(),
        t_type_ref::from_ptr(node_type));
    for (auto& pair : unstructured) {
      unnamed->set_annotation(pair.first, pair.second);
    }
    node.set_type(t_type_ref::from_ptr(unnamed.get()));
    program.add_unnamed_typedef(std::move(unnamed));
  }
}

} // namespace

ast_mutators standard_mutators(bool use_legacy_type_ref_resolution) {
  ast_mutators mutators(use_legacy_type_ref_resolution);
  {
    auto& initial = mutators[standard_mutator_stage::initial];
    initial.add_field_visitor(&lower_type_annotations<t_field>);
    initial.add_typedef_visitor(&lower_type_annotations<t_typedef>);
    initial.add_function_visitor(&normalize_return_type);
    initial.add_definition_visitor(&set_generated);
    initial.add_definition_visitor(&lower_deprecated_annotations);
  }

  {
    auto& main = mutators[standard_mutator_stage::main];
    main.add_structured_visitor(&mutate_terse_write_annotation_structured);
    main.add_exception_visitor(&mutate_terse_write_annotation_structured);
    main.add_structured_visitor(&mutate_inject_metadata_fields);
    main.add_structured_visitor(&generate_struct_schema);
    main.add_union_visitor(&generate_union_schema);
    main.add_exception_visitor(&generate_exception_schema);
    main.add_service_visitor(&generate_service_schema);
    main.add_const_visitor(&generate_const_schema);
    main.add_enum_visitor(&generate_enum_schema);
    main.add_typedef_visitor(&generate_typedef_schema);
    main.add_const_visitor(&match_const_type_with_value);
    main.add_field_visitor(&match_field_type_with_default_value);
    main.add_definition_visitor(&match_annotation_types_with_const_values);
  }

  add_patch_mutators(mutators);
  return mutators;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
