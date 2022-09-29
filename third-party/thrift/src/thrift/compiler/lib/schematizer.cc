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

#include <utility>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/lib/schematizer.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {
template <typename... Args>
std::unique_ptr<t_const_value> val(Args... args) {
  return std::make_unique<t_const_value>(std::forward<Args>(args)...);
}
template <typename Enm, typename = std::enable_if_t<std::is_enum<Enm>::value>>
std::unique_ptr<t_const_value> val(Enm val) {
  return std::make_unique<t_const_value>(
      static_cast<std::underlying_type_t<Enm>>(val));
}

void add_definition(t_const_value& schema, const t_named& node) {
  auto definition = val();
  definition->set_map();
  definition->add_map(val("name"), val(node.name()));
  if (!node.uri().empty()) {
    definition->add_map(val("uri"), val(node.uri()));
  }
  // TODO: annotations
  schema.add_map(val("attrs"), std::move(definition));
}

std::unique_ptr<t_const_value> gen_type(const t_type& type) {
  auto schema = val();
  schema->set_map();
  auto type_name = val();
  type_name->set_map();
  std::unique_ptr<t_const_value> params;
  switch (type.get_type_value()) {
    case t_type::type::t_void:
      break;
    case t_type::type::t_bool:
      type_name->add_map(val("boolType"), val(0));
      break;
    case t_type::type::t_byte:
      type_name->add_map(val("byteType"), val(0));
      break;
    case t_type::type::t_i16:
      type_name->add_map(val("i16Type"), val(0));
      break;
    case t_type::type::t_i32:
      type_name->add_map(val("i32Type"), val(0));
      break;
    case t_type::type::t_i64:
      type_name->add_map(val("i64Type"), val(0));
      break;
    case t_type::type::t_double:
      type_name->add_map(val("doubleType"), val(0));
      break;
    case t_type::type::t_float:
      type_name->add_map(val("floatType"), val(0));
      break;
    case t_type::type::t_string:
      type_name->add_map(val("stringType"), val(0));
      break;
    case t_type::type::t_binary:
      type_name->add_map(val("binaryType"), val(0));
      break;
    case t_type::type::t_list:
      type_name->add_map(val("listType"), val(0));
      params = val();
      params->set_list();
      params->add_list(gen_type(
          *static_cast<const t_list&>(type).elem_type()->get_true_type()));
      break;
    case t_type::type::t_set:
      type_name->add_map(val("setType"), val(0));
      params = val();
      params->set_list();
      params->add_list(gen_type(
          *static_cast<const t_set&>(type).elem_type()->get_true_type()));
      break;
    case t_type::type::t_map:
      type_name->add_map(val("mapType"), val(0));
      params = val();
      params->set_list();
      {
        const auto& map = static_cast<const t_map&>(type);
        params->add_list(gen_type(*map.key_type()->get_true_type()));
        params->add_list(gen_type(*map.val_type()->get_true_type()));
      }
      break;
    case t_type::type::t_enum:
      type_name->add_map(val("enumType"), val(type.uri()));
      break;
    case t_type::type::t_struct: {
      auto structured = val();
      structured->set_map();
      structured->add_map(val("uri"), val(type.uri()));
      type_name->add_map(
          val([&] {
            if (dynamic_cast<const t_union*>(&type)) {
              return "unionType";
            } else if (dynamic_cast<const t_exception*>(&type)) {
              return "exceptionType";
            } else {
              return "structType";
            }
          }()),
          std::move(structured));
      break;
    }
    default:
      assert(false);
  }
  schema->add_map(val("name"), std::move(type_name));
  if (params) {
    schema->add_map(val("params"), std::move(params));
  }
  return schema;
}

const t_enum* find_enum(const t_program* program, std::string enum_uri) {
  // May be null in unit tests.
  return program
      ? dynamic_cast<const t_enum*>(program->scope()->find_def(enum_uri))
      : nullptr;
}

void add_qualifier(const t_enum* t_enum, t_const_value& schema, int enum_val) {
  auto qualifier_schema = val(enum_val);
  if (t_enum) {
    qualifier_schema->set_is_enum();
    qualifier_schema->set_enum(t_enum);
    qualifier_schema->set_enum_value(t_enum->find_value(enum_val));
  }
  schema.add_map(val("qualifier"), std::move(qualifier_schema));
}

void add_fields(
    const t_program* program,
    t_const_value& schema,
    node_list_view<const t_field> fields) {
  auto fields_schema = val();
  fields_schema->set_list();

  const auto* field_qualifier_enum =
      find_enum(program, "facebook.com/thrift/type/FieldQualifier");

  for (const auto& field : fields) {
    auto field_schema = val();
    field_schema->set_map();
    add_definition(*field_schema, field);
    field_schema->add_map(val("id"), val(field.id()));

    add_qualifier(field_qualifier_enum, *field_schema, [&] {
      switch (field.qualifier()) {
        case t_field_qualifier::none:
        case t_field_qualifier::required:
          return 3; // Fill
        case t_field_qualifier::optional:
          return 1; // Optional
        case t_field_qualifier::terse:
          return 2; // Terse
      }
      assert(false);
      return 0; // Default
    }());

    field_schema->add_map(
        val("type"), gen_type(*field.type()->get_true_type()));
    if (auto deflt = field.default_value()) {
      assert(program);
      auto id = const_cast<t_program*>(program)->intern_value(
          deflt->clone(), field.type());
      field_schema->add_map(val("customDefault"), val(id));
    }
    fields_schema->add_list(std::move(field_schema));
  }

  schema.add_map(val("fields"), std::move(fields_schema));
}
} // namespace

std::unique_ptr<t_const_value> schematizer::gen_schema(
    const t_structured& node) {
  auto schema = val();
  schema->set_map();
  add_definition(*schema, node);
  add_fields(node.program(), *schema, node.fields());

  if (node.is_exception()) {
    const auto& ex = static_cast<const t_exception&>(node);
    schema->add_map(val("safety"), val(ex.safety()));
    schema->add_map(val("kind"), val(ex.kind()));
    schema->add_map(val("blame"), val(ex.blame()));
  }

  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_service& node) {
  auto schema = val();
  schema->set_map();
  add_definition(*schema, node);

  auto functions_schema = val();
  functions_schema->set_list();

  const auto* func_qualifier_enum =
      find_enum(node.program(), "facebook.com/thrift/type/FunctionQualifier");

  for (const auto& func : node.functions()) {
    auto func_schema = val();
    func_schema->set_map();
    add_definition(*func_schema, func);

    add_qualifier(func_qualifier_enum, *func_schema, [&] {
      switch (func.qualifier()) {
        case t_function_qualifier::unspecified:
          return 0;
        case t_function_qualifier::one_way:
          return 1;
        case t_function_qualifier::idempotent:
          return 2;
        case t_function_qualifier::read_only:
          return 3;
      }
      assert(false);
      return 0; // Default
    }());

    auto return_types_schema = val();
    return_types_schema->set_list();
    for (const auto& ret : func.return_types()) {
      // TODO: Handle sink, stream, interactions
      if (!ret->is_sink() && !ret->is_streamresponse()) {
        auto return_type_schema = val();
        return_type_schema->set_map();
        return_type_schema->add_map(
            val("thriftType"), gen_type(*ret->get_true_type()));
        return_types_schema->add_list(std::move(return_type_schema));
      }
    }
    func_schema->add_map(val("returnTypes"), std::move(return_types_schema));

    auto param_list_schema = val();
    param_list_schema->set_map();
    add_fields(node.program(), *param_list_schema, func.params().fields());
    func_schema->add_map(val("paramlist"), std::move(param_list_schema));

    // TODO: add exceptions

    functions_schema->add_list(std::move(func_schema));
  }
  schema->add_map(val("functions"), std::move(functions_schema));

  // TODO: add inheritedService

  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_const& node) {
  auto schema = val();
  schema->set_map();
  add_definition(*schema, node);

  schema->add_map(val("type"), gen_type(*node.type()->get_true_type()));

  const auto* program = node.program();
  assert(program);
  auto id = const_cast<t_program*>(program)->intern_value(
      node.value()->clone(), node.type());
  schema->add_map(val("value"), val(id));

  return schema;
}

std::unique_ptr<t_const_value> schematizer::gen_schema(const t_enum& node) {
  auto schema = val();
  schema->set_map();
  add_definition(*schema, node);

  auto values = val();
  values->set_list();

  for (const auto& value : node.values()) {
    auto value_schema = val();
    value_schema->set_map();
    add_definition(*value_schema, value);
    value_schema->add_map(val("value"), val(value.get_value()));
    values->add_list(std::move(value_schema));
  }

  schema->add_map(val("values"), std::move(values));

  return schema;
}
} // namespace compiler
} // namespace thrift
} // namespace apache
