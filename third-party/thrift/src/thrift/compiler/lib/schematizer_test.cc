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

#include <thrift/compiler/lib/schematizer.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_paramlist.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_struct.h>

namespace apache::thrift::compiler {
namespace {
void validateDefinition(
    std::unordered_map<std::string, t_const_value*> schema,
    std::string name,
    std::string uri) {
  EXPECT_EQ(schema.at("name")->get_string(), name);
  EXPECT_EQ(schema.at("uri")->get_string(), uri);
}

// Converts map const val to c++ map and flattens definition mixin
std::unordered_map<std::string, t_const_value*> flatten_map(
    const t_const_value& val) {
  EXPECT_EQ(val.get_type(), t_const_value::CV_MAP);
  std::unordered_map<std::string, t_const_value*> map;
  for (const auto& pair : val.get_map()) {
    map[pair.first->get_string()] = pair.second;
  }
  if (auto def = map.find("attrs"); def != map.end()) {
    for (const auto& pair : def->second->get_map()) {
      map[pair.first->get_string()] = pair.second;
    }
  }
  return map;
}

void validate_nested_type(
    t_const_value& type, std::string type_string, std::string uri) {
  auto schema = flatten_map(type);
  EXPECT_EQ(
      schema.at("name")->get_map().at(0).first->get_string(), type_string);
  if (!uri.empty()) {
    EXPECT_EQ(
        schema.at("name")
            ->get_map()
            .at(0)
            .second->get_map()
            .at(0)
            .second->get_string(),
        uri);
  }
  EXPECT_FALSE(schema.count("params"));
}

void validate_nested_struct(
    t_const_value& field,
    std::string struct_name,
    std::string struct_uri,
    int id,
    int qualifier) {
  auto field_schema = flatten_map(field);
  EXPECT_EQ(field_schema.at("id")->get_integer(), id);
  EXPECT_EQ(field_schema.at("qualifier")->get_integer(), qualifier);
  EXPECT_EQ(field_schema.at("name")->get_string(), struct_name);
  validate_nested_type(*field_schema.at("type"), "structType", struct_uri);
}

void validate_exception(
    t_const_value& field,
    std::string name,
    std::string uri,
    int id,
    int qualifier) {
  auto field_schema = flatten_map(field);
  EXPECT_EQ(field_schema.at("id")->get_integer(), id);
  EXPECT_EQ(field_schema.at("qualifier")->get_integer(), qualifier);
  EXPECT_EQ(field_schema.at("name")->get_string(), name);
  validate_nested_type(*field_schema.at("type"), "exceptionType", uri);
}

void validate_test_struct(
    t_const_value& schema, std::string struct_name, std::string struct_uri) {
  auto map = flatten_map(schema);
  const auto& fields = map.at("fields")->get_list();

  validateDefinition(map, struct_name, struct_uri);

  EXPECT_EQ(fields.size(), 3);

  auto field1 = flatten_map(*fields.at(0));
  EXPECT_EQ(field1.at("id")->get_integer(), 1);
  EXPECT_EQ(field1.at("qualifier")->get_integer(), 3);
  EXPECT_EQ(field1.at("name")->get_string(), "i16");
  auto type1 = flatten_map(*field1.at("type"));
  EXPECT_EQ(type1.at("name")->get_map().at(0).first->get_string(), "i16Type");
  EXPECT_FALSE(type1.count("params"));

  validate_nested_struct(*fields.at(1), struct_name, struct_uri, 2, 1);

  auto field3 = flatten_map(*fields.at(2));
  EXPECT_EQ(field3.at("name")->get_string(), "Map");
  auto type3 = flatten_map(*field3.at("type"));
  EXPECT_EQ(type3.at("name")->get_map().at(0).first->get_string(), "mapType");
  auto params3 = type3.at("params")->get_list();
  EXPECT_EQ(params3.size(), 2);
  EXPECT_EQ(
      flatten_map(*params3.at(0))
          .at("name")
          ->get_map()
          .at(0)
          .first->get_string(),
      "stringType");
  EXPECT_EQ(
      flatten_map(*params3.at(1))
          .at("name")
          ->get_map()
          .at(0)
          .first->get_string(),
      "doubleType");
}
} // namespace

TEST(SchematizerTest, Service) {
  std::string service_name("Service");
  std::string service_uri("path/to/Service");

  t_service svc(nullptr, service_name);
  svc.set_uri(service_uri);
  std::string struct_name("Struct");
  std::string struct_uri("path/to/Struct");

  t_struct return_type(nullptr, struct_name);
  return_type.set_uri(struct_uri);

  auto func0 = std::make_unique<t_function>(nullptr, return_type, "my_rpc");

  t_struct param0(nullptr, struct_name);
  param0.set_uri(struct_uri);
  func0->params().create_field(param0, "param0");

  t_exception ex0(nullptr, "MyException");
  auto ex = std::make_unique<t_throws>();
  ex->create_field(ex0, "ex0");
  func0->set_exceptions(std::move(ex));

  svc.add_function(std::move(func0));

  auto schema = schematizer().gen_full_schema(svc);
  auto map = flatten_map(*schema);
  auto dfns = map.at("definitions")->get_list();
  EXPECT_EQ(dfns.size(), 4);
  auto dfn_map = flatten_map(*dfns.at(0));

  auto svc_map = flatten_map(*dfn_map.at("serviceDef"));
  validateDefinition(svc_map, service_name, service_uri);

  auto funcs = svc_map.at("functions")->get_list();
  EXPECT_EQ(funcs.size(), 1);
  auto func0_schema = flatten_map(*funcs.at(0));
  auto func0_params =
      flatten_map(*func0_schema.at("paramlist")).at("fields")->get_list();
  EXPECT_EQ(func0_params.size(), 1);
  auto param0_schema = func0_params.at(0);
  validate_nested_struct(*param0_schema, "param0", struct_uri, 0, 3);

  auto retTypes = func0_schema.at("returnTypes")->get_list();
  EXPECT_EQ(retTypes.size(), 1);
  auto ret0_type = flatten_map(*retTypes.at(0)).at("thriftType");
  validate_nested_type(*ret0_type, "structType", struct_uri);

  auto func0_exs = func0_schema.at("exceptions")->get_list();
  EXPECT_EQ(func0_exs.size(), 1);
  auto ex0_schema = func0_exs.at(0);
  validate_exception(*ex0_schema, "ex0", "", 0, 3);
}

TEST(SchematizerTest, Structured) {
  std::string struct_name("Struct");
  std::string struct_uri("path/to/Struct");

  t_struct s(nullptr, struct_name);
  s.set_uri(struct_uri);
  s.create_field(t_base_type::t_i16(), "i16", 1);
  s.create_field(s, "Struct", 2).set_qualifier(t_field_qualifier::optional);
  t_map tmap(t_base_type::t_string(), t_base_type::t_double());
  s.create_field(tmap, "Map", 3);

  auto schema = schematizer().gen_schema(s);
  validate_test_struct(*schema, struct_name, struct_uri);
}

TEST(SchematizerTest, Const) {
  std::string program_path("path/to/Program.thrift");
  t_program program(program_path);

  std::string const_name("Const");
  std::string const_uri("path/to/Const");
  t_type_ref const_type(t_base_type::t_string());

  t_const c(
      &program,
      const_type,
      const_name,
      std::make_unique<t_const_value>("Hello"));
  c.set_uri(const_uri);

  auto schema = schematizer().gen_schema(c);
  auto map = flatten_map(*schema);

  validateDefinition(map, const_name, const_uri);

  auto type = flatten_map(*map.at("type"));
  EXPECT_EQ(type.at("name")->get_map().at(0).first->get_string(), "stringType");

  auto& container = program.intern_list();
  EXPECT_EQ(container.size(), 1);
  EXPECT_EQ(container[0]->get_value()->get_string(), "Hello");
}

TEST(SchematizerTest, Enum) {
  std::string enum_name("Enum");
  std::string enum_uri("path/to/Enum");

  t_enum e(nullptr, enum_name);
  e.set_uri(enum_uri);

  std::string ev_0_name("EnumValue0");
  int32_t ev_0_value(0);
  t_enum_value enum_value_0(ev_0_name, ev_0_value);

  std::string ev_1_name("EnumValue1");
  int32_t ev_1_value(1);
  t_enum_value enum_value_1(ev_1_name, ev_1_value);

  e.append_value(std::make_unique<t_enum_value>(enum_value_0));
  e.append_value(std::make_unique<t_enum_value>(enum_value_1));

  auto schema = schematizer().gen_schema(e);
  auto map = flatten_map(*schema);
  const auto& values = map.at("values")->get_list();

  validateDefinition(map, enum_name, enum_uri);

  EXPECT_EQ(values.size(), 2);

  auto value0 = flatten_map(*values.at(0));
  EXPECT_EQ(value0.at("name")->get_string(), ev_0_name);
  EXPECT_EQ(value0.at("value")->get_integer(), ev_0_value);

  auto value1 = flatten_map(*values.at(1));
  EXPECT_EQ(value1.at("name")->get_string(), ev_1_name);
  EXPECT_EQ(value1.at("value")->get_integer(), ev_1_value);
}

TEST(SchematizerTest, WrapWithProtocolValue) {
  t_const_value str("foo");
  auto value = wrap_with_protocol_value(str, {});
  auto map = value->get_map();
  EXPECT_EQ(map.at(0).first->get_string(), "stringValue");
  EXPECT_EQ(map.at(0).second->get_string(), "foo");
}

} // namespace apache::thrift::compiler
