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

#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_program.h>
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
std::unique_ptr<t_const_value> val(t_program::value_id val) {
  return std::make_unique<t_const_value>(
      static_cast<std::underlying_type_t<t_program::value_id>>(val));
}

void add_definition(t_const_value& schema, const t_named& node) {
  auto definition = val();
  definition->set_map();
  definition->add_map(val("name"), val(node.name()));
  if (!node.uri().empty()) {
    definition->add_map(val("uri"), val(node.uri()));
  }
  // TODO: annotations
  schema.add_map(val("definition"), std::move(definition));
}

std::unique_ptr<t_const_value> gen_type(const t_type& type) {
  auto schema = val();
  schema->set_map();
  auto type_name = val();
  type_name->set_map();
  std::unique_ptr<t_const_value> params;
  switch (type.get_type_value()) {
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
} // namespace

std::unique_ptr<t_const_value> schematizer::gen_schema(
    const t_structured& node) {
  auto schema = val();
  schema->set_map();
  add_definition(*schema, node);

  auto fields = val();
  fields->set_list();
  // May be null in unit tests.
  const auto* program = node.program();
  const auto* field_qualifier_enum = program
      ? dynamic_cast<const t_enum*>(program->scope()->find_def(
            "facebook.com/thrift/type/FieldQualifier"))
      : nullptr;

  for (const auto& field : node.fields()) {
    auto field_schema = val();
    field_schema->set_map();
    add_definition(*field_schema, field);
    field_schema->add_map(val("id"), val(field.id()));
    auto qualifierVal = [&] {
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
    }();
    auto qualifier = val(qualifierVal);
    if (field_qualifier_enum) {
      qualifier->set_is_enum();
      qualifier->set_enum(field_qualifier_enum);
      qualifier->set_enum_value(field_qualifier_enum->find_value(qualifierVal));
    }
    field_schema->add_map(val("qualifier"), std::move(qualifier));
    field_schema->add_map(
        val("type"), gen_type(*field.type()->get_true_type()));
    if (auto deflt = field.default_value()) {
      assert(program);
      auto id = const_cast<t_program*>(program)->intern_value(
          deflt->clone(), field.type());
      field_schema->add_map(val("customDefault"), val(id));
    }
    fields->add_list(std::move(field_schema));
  }

  schema->add_map(val("fields"), std::move(fields));

  return schema;
}
} // namespace compiler
} // namespace thrift
} // namespace apache
