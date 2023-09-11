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

const t_type* resolve_type(const t_type* type) {
  while (type->is_typedef()) {
    type = dynamic_cast<const t_typedef*>(type)->get_type();
  }
  return type;
}

void match_type_with_const_value(
    diagnostic_context& ctx,
    const t_program& program,
    const t_type* long_type,
    t_const_value* value) {
  const t_type* type = resolve_type(long_type);
  value->set_ttype(t_type_ref::from_req_ptr(type));
  if (type->is_list()) {
    auto* elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
    for (auto list_val : value->get_list()) {
      match_type_with_const_value(ctx, program, elem_type, list_val);
    }
  }
  if (type->is_set()) {
    auto* elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
    for (auto set_val : value->get_list()) {
      match_type_with_const_value(ctx, program, elem_type, set_val);
    }
  }
  if (type->is_map()) {
    auto* key_type = dynamic_cast<const t_map*>(type)->get_key_type();
    auto* val_type = dynamic_cast<const t_map*>(type)->get_val_type();
    for (auto map_val : value->get_map()) {
      match_type_with_const_value(ctx, program, key_type, map_val.first);
      match_type_with_const_value(ctx, program, val_type, map_val.second);
    }
  }
  if (type->is_struct()) {
    auto* struct_type = dynamic_cast<const t_struct*>(type);
    for (auto map_val : value->get_map()) {
      auto name = map_val.first->get_string();
      auto tfield = struct_type->get_field_by_name(name);
      if (!tfield) {
        // Error reported by const_checker.
        return;
      }
      match_type_with_const_value(
          ctx, program, tfield->get_type(), map_val.second);
    }
  }
  // Set constant value types as enums when they are declared with integers
  if (type->is_enum() && !value->is_enum()) {
    value->set_is_enum();
    auto enm = dynamic_cast<const t_enum*>(type);
    value->set_enum(enm);
    if (value->get_type() == t_const_value::CV_STRING) {
      // The enum was defined after the struct field with that type was declared
      // so the field default value, if present, was treated as a string rather
      // than resolving to the enum constant in the parser.
      // So we have to resolve the string to the enum constant here instead.
      auto str = value->get_string();
      const t_const* constant = program.scope()->find_constant(str);
      if (!constant) {
        auto full_str = program.name() + "." + str;
        constant = program.scope()->find_constant(full_str);
      }
      if (!constant) {
        throw std::runtime_error(
            std::string("type error: no matching constant: ") + str);
      }
      std::unique_ptr<t_const_value> value_copy = constant->value()->clone();
      value->assign(std::move(*value_copy));
    }
    if (enm->find_value(value->get_integer())) {
      value->set_enum_value(enm->find_value(value->get_integer()));
    }
  }
  // Remove enum_value if type is a base_type to use the integer instead
  if (type->is_base_type() && value->is_enum()) {
    value->set_enum_value(nullptr);
  }
}

static void match_annotation_types_with_const_values(
    diagnostic_context& ctx, mutator_context& mCtx, t_named& node) {
  for (t_const& tconst : node.structured_annotations()) {
    const t_type* const type = tconst.get_type();
    if (type == nullptr) {
      continue;
    }

    t_const_value* const value = tconst.value();
    if (value == nullptr) {
      continue;
    }

    match_type_with_const_value(ctx, mCtx.program(), type, value);
  }
}

// TODO(afuller): Instead of mutating the AST, readers should look for
// the interaction level annotation and the validation logic should be moved to
// a standard validator.
void propagate_process_in_event_base_annotation(
    diagnostic_context& ctx, mutator_context&, t_interaction& node) {
  for (auto* func : node.get_functions()) {
    func->set_is_interaction_member();
    ctx.check(
        !func->has_annotation("thread"),
        "Interaction methods cannot be individually annotated with "
        "thread='eb'. Use process_in_event_base on the interaction instead.");
  }
  if (node.has_annotation("process_in_event_base")) {
    ctx.check(
        !node.has_annotation("serial"), "EB interactions are already serial");
    for (auto* func : node.get_functions()) {
      func->set_annotation("thread", "eb");
    }
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
    diagnostic_context& ctx, mutator_context&, t_struct& node) {
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

  const auto* structured = dynamic_cast<const t_struct*>(ttype);
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

void normalize_return_type(
    diagnostic_context& ctx, mutator_context&, t_function& node) {
  auto& types = node.return_types();
  if (types.empty()) {
    return;
  }
  if (!types.front().resolve()) {
    ctx.error(node, "Failed to resolve return type of `{}`.", node.name());
    return;
  }

  size_t response_pos = 0;
  t_type* sink_or_stream = node.sink_or_stream();
  if (auto* interaction = dynamic_cast<const t_interaction*>(&*types.front())) {
    // Old syntax treats a returned interaction as a response.
    if (node.is_interaction_constructor()) {
      assert(types.size() == 1 && !sink_or_stream);
      node.set_response_pos(0);
      return;
    }
    node.set_returned_interaction_pos(0);
    if (types.size() == 1) {
      if (!sink_or_stream) {
        node.set_return_type(t_base_type::t_void());
      }
      return;
    }
    response_pos = 1;
  } else if (types.size() > 1) {
    ctx.error("Too many return types");
  }

  // Check the (first) response type.
  auto type = types[response_pos];
  const t_type* true_type = type->get_true_type();
  if (dynamic_cast<const t_service*>(true_type) ||
      dynamic_cast<const t_exception*>(true_type)) {
    ctx.error("Invalid first response type: {}", type->get_full_name());
  }

  if (!sink_or_stream) {
    node.set_response_pos(response_pos);
  } else if (auto* sink = dynamic_cast<t_sink*>(sink_or_stream)) {
    // TODO: move first response out of t_sink.
    sink->set_first_response_type(type);
  } else if (auto* stream = dynamic_cast<t_stream_response*>(sink_or_stream)) {
    // TODO: move first response out of t_stream_response.
    stream->set_first_response_type(type);
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
  program->add_definition(std::move(schemaConst));
}

void generate_struct_schema(
    diagnostic_context& ctx, mutator_context& mCtx, t_struct& node) {
  generate_runtime_schema<t_struct&>(
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
        &program, node_type->get_name(), t_type_ref::from_ptr(node_type));
    for (auto& pair : unstructured) {
      unnamed->set_annotation(pair.first, pair.second);
    }
    node.set_type(t_type_ref::from_ptr(unnamed.get()));
    program.add_unnamed_typedef(std::move(unnamed));
  }
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

void const_to_const_value(
    diagnostic_context& ctx, mutator_context& mCtx, t_const& const_node) {
  maybe_match_type_with_const_value(
      ctx, mCtx, const_node.get_type(), const_node.value());
}

void field_to_const_value(
    diagnostic_context& ctx, mutator_context& mCtx, t_field& field_node) {
  maybe_match_type_with_const_value(
      ctx, mCtx, field_node.get_type(), field_node.get_value());
}

} // namespace

ast_mutators standard_mutators() {
  ast_mutators mutators;
  {
    auto& initial = mutators[standard_mutator_stage::initial];
    initial.add_field_visitor([](auto& ctx, auto& mCtx, auto& node) {
      lower_type_annotations(ctx, mCtx, node);
    });
    initial.add_typedef_visitor([](auto& ctx, auto& mCtx, auto& node) {
      lower_type_annotations(ctx, mCtx, node);
    });
    initial.add_interaction_visitor(
        &propagate_process_in_event_base_annotation);
    initial.add_function_visitor(&normalize_return_type);
    initial.add_definition_visitor(&set_generated);
  }

  {
    auto& main = mutators[standard_mutator_stage::main];
    main.add_struct_visitor(&mutate_terse_write_annotation_structured);
    main.add_exception_visitor(&mutate_terse_write_annotation_structured);
    main.add_struct_visitor(&mutate_inject_metadata_fields);
    main.add_struct_visitor(&generate_struct_schema);
    main.add_union_visitor(&generate_union_schema);
    main.add_exception_visitor(&generate_exception_schema);
    main.add_service_visitor(&generate_service_schema);
    main.add_const_visitor(&generate_const_schema);
    main.add_enum_visitor(&generate_enum_schema);
    main.add_typedef_visitor(&generate_typedef_schema);
    main.add_const_visitor(&const_to_const_value);
    main.add_field_visitor(&field_to_const_value);
    main.add_definition_visitor(&match_annotation_types_with_const_values);
  }

  add_patch_mutators(mutators);
  return mutators;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
