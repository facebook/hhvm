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

#include <functional>
#include <string>
#include <vector>
#include <folly/portability/GTest.h>
#include <thrift/lib/thrift/gen-cpp2/schema_types.h>
#include <thrift/test/gen-cpp2/schema_constants.h>

using namespace facebook::thrift::test::schema;

TEST(SchemaTest, Empty) {
  auto schema = schema_constants::schemaEmpty();
  EXPECT_EQ(*schema.name(), "Empty");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/Empty");
  EXPECT_TRUE(schema.fields()->empty());
}

TEST(SchemaTest, Renamed) {
  auto schema = schema_constants::RenamedSchema();
  EXPECT_EQ(*schema.name(), "Renamed");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/Renamed");
  EXPECT_TRUE(schema.fields()->empty());
}

static void verify_fields_struct_schema(
    std::vector<apache::thrift::type::Field>& fields) {
  EXPECT_EQ(fields.size(), 3);

  // 1: i32 num;
  auto field = fields.at(0);
  EXPECT_EQ(*field.name(), "num");
  EXPECT_EQ(*field.id(), apache::thrift::type::FieldId{1});
  EXPECT_EQ(*field.qualifier(), apache::thrift::type::FieldQualifier::Fill);
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::i32Type);

  // 3: optional set<string> keyset;
  field = fields.at(1);
  EXPECT_EQ(*field.name(), "keyset");
  EXPECT_EQ(*field.id(), apache::thrift::type::FieldId{3});
  EXPECT_EQ(*field.qualifier(), apache::thrift::type::FieldQualifier::Optional);
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::setType);
  EXPECT_EQ(field.type()->toThrift().params()->size(), 1);
  EXPECT_EQ(
      field.type()->toThrift().params()->at(0).name()->getType(),
      apache::thrift::type::TypeName::Type::stringType);

  // 7: Empty strct;
  field = fields.at(2);
  EXPECT_EQ(*field.name(), "strct");
  EXPECT_EQ(*field.id(), apache::thrift::type::FieldId{7});
  EXPECT_EQ(*field.qualifier(), apache::thrift::type::FieldQualifier::Fill);
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::structType);
  EXPECT_EQ(
      *field.type()->toThrift().name()->structType_ref()->uri_ref(),
      "facebook.com/thrift/test/schema/Empty");
}

TEST(SchemaTest, Fields) {
  apache::thrift::type::Struct schema = schema_constants::schemaFields();
  verify_fields_struct_schema(*schema.fields());
}

TEST(SchemaTest, Defaults) {
  EXPECT_THROW(
      schema_constants::getValue(apache::thrift::type::ValueId{}),
      std::out_of_range);
  EXPECT_THROW(
      schema_constants::getValue(apache::thrift::type::ValueId{42}),
      std::out_of_range);

  auto schema = schema_constants::schemaDefaults();
  EXPECT_EQ(schema.fields()->size(), 2);

  // 1: i32 none;
  auto field = schema.fields()->at(0);
  EXPECT_EQ(*field.name(), "none");
  EXPECT_EQ(*field.customDefault(), apache::thrift::type::ValueId{});

  // 2: i32 some = 42;
  field = schema.fields()->at(1);
  EXPECT_EQ(*field.name(), "some");
  EXPECT_EQ(
      schema_constants::getValue(*field.customDefault())
          .as<apache::thrift::type::i32_t>(),
      42);
}

TEST(SchemaTest, Union) {
  apache::thrift::type::Union schema = schema_constants::schemaUnion();
  EXPECT_EQ(*schema.name(), "Union");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/Union");
  EXPECT_TRUE(schema.fields()->empty());
}

TEST(SchemaTest, Exception) {
  apache::thrift::type::Exception schema =
      schema_constants::schemaSimpleException();
  EXPECT_EQ(*schema.name(), "SimpleException");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/SimpleException");
  EXPECT_TRUE(schema.fields()->empty());
  EXPECT_EQ(*schema.safety(), apache::thrift::type::ErrorSafety::Unspecified);
  EXPECT_EQ(*schema.kind(), apache::thrift::type::ErrorKind::Unspecified);
  EXPECT_EQ(*schema.blame(), apache::thrift::type::ErrorBlame::Unspecified);

  schema = schema_constants::schemaFancyException();
  EXPECT_EQ(*schema.safety(), apache::thrift::type::ErrorSafety::Safe);
  EXPECT_EQ(*schema.kind(), apache::thrift::type::ErrorKind::Transient);
  EXPECT_EQ(*schema.blame(), apache::thrift::type::ErrorBlame::Server);
}

static void verify_function(
    apache::thrift::type::Function func,
    std::string name,
    std::function<void(apache::thrift::type::Paramlist&)> paramsValidator,
    std::function<void(apache::thrift::type::Exceptions&)> exValidator,
    std::function<void(std::vector<apache::thrift::type::ReturnType>&)>
        retValidator) {
  EXPECT_EQ(*func.name(), name);
  EXPECT_TRUE(func.uri()->empty());
  EXPECT_EQ(
      func.qualifier(), apache::thrift::type::FunctionQualifier::Unspecified);

  if (paramsValidator) {
    paramsValidator(*func.paramlist());
  } else {
    EXPECT_TRUE(func.paramlist()->fields()->empty());
  }

  if (exValidator) {
    exValidator(*func.exceptions());
  } else {
    EXPECT_TRUE(func.exceptions()->empty());
  }

  if (retValidator) {
    retValidator(*func.returnTypes());
  } else {
    EXPECT_TRUE(func.returnTypes()->empty());
  }
}

static void verify_void_return(
    std::vector<apache::thrift::type::ReturnType>& rets) {
  EXPECT_EQ(rets.size(), 1);
  auto ret1 = rets.at(0);
  EXPECT_EQ(ret1.getType(), apache::thrift::type::ReturnType::Type::thriftType);
  EXPECT_TRUE(
      ret1.get_thriftType().toThrift() == apache::thrift::type::TypeStruct{});
}

TEST(SchemaTest, EmptyService) {
  auto schema = schema_constants::schemaEmptyService();
  EXPECT_EQ(schema.definitions()->size(), 1);

  auto svc_schema = schema.definitions()->at(0).get_serviceDef();
  EXPECT_EQ(*svc_schema.name(), "EmptyService");
  EXPECT_EQ(*svc_schema.uri(), "facebook.com/thrift/test/schema/EmptyService");
  EXPECT_TRUE(svc_schema.functions()->empty());
}

TEST(SchemaTest, IntConst) {
  apache::thrift::type::Const schema = schema_constants::schemaIntConst();
  EXPECT_EQ(*schema.name(), "IntConst");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/IntConst");
  EXPECT_EQ(
      schema.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::i32Type);
  EXPECT_EQ(
      schema_constants::getValue(*schema.value())
          .as<apache::thrift::type::i32_t>(),
      11);
}

TEST(SchemaTest, ListConst) {
  apache::thrift::type::Const schema = schema_constants::schemaListConst();
  EXPECT_EQ(*schema.name(), "ListConst");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/ListConst");
  EXPECT_EQ(
      schema.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::listType);
  EXPECT_EQ(schema.type()->toThrift().params()->size(), 1);
  EXPECT_EQ(
      schema.type()->toThrift().params()->at(0).name()->getType(),
      apache::thrift::type::TypeName::Type::i32Type);

  auto list_values =
      schema_constants::getValue(*schema.value())
          .as<apache::thrift::type::list<apache::thrift::type::i32_t>>();
  EXPECT_EQ(list_values.size(), 5);
  EXPECT_EQ(list_values[0], 2);
  EXPECT_EQ(list_values[1], 3);
  EXPECT_EQ(list_values[2], 5);
  EXPECT_EQ(list_values[3], 7);
  EXPECT_EQ(list_values[4], 11);
}

TEST(SchemaTest, TestService) {
  auto schema = schema_constants::schemaTestService();
  EXPECT_EQ(schema.definitions()->size(), 4);

  auto ex_schema = schema.definitions()->at(1).get_exceptionDef();
  EXPECT_EQ(*ex_schema.name(), "NonSchematizedException");

  auto struct_schema = schema.definitions()->at(2).get_structDef();
  EXPECT_EQ(*struct_schema.name(), "NonSchematizedStruct");

  auto union_schema = schema.definitions()->at(3).get_unionDef();
  EXPECT_EQ(*union_schema.name(), "NonSchematizedUnion");

  auto svc_schema = schema.definitions()->at(0).get_serviceDef();

  EXPECT_EQ(*svc_schema.name(), "TestService");
  EXPECT_EQ(*svc_schema.uri(), "facebook.com/thrift/test/schema/TestService");
  EXPECT_EQ(svc_schema.functions()->size(), 4);

  verify_function(
      svc_schema.functions()->at(0),
      "noParamsNoReturnNoEx",
      nullptr,
      nullptr,
      &verify_void_return);

  verify_function(
      svc_schema.functions()->at(1),
      "noParamsPrimitiveReturnNoEx",
      nullptr,
      nullptr,
      [](std::vector<apache::thrift::type::ReturnType>& rets) {
        EXPECT_EQ(rets.size(), 1);
        auto ret1 = rets.at(0);
        EXPECT_EQ(
            ret1.getType(), apache::thrift::type::ReturnType::Type::thriftType);
        EXPECT_EQ(
            ret1.get_thriftType().toThrift().name()->getType(),
            apache::thrift::type::TypeName::Type::i32Type);
      });

  verify_function(
      svc_schema.functions()->at(2),
      "primitiveParamsNoReturnEx",
      [](apache::thrift::type::Paramlist& params) {
        auto fields = params.fields();
        EXPECT_EQ(fields->size(), 1);

        auto field0 = fields->at(0);
        EXPECT_EQ(*field0.name(), "param0");
        EXPECT_EQ(*field0.id(), apache::thrift::type::FieldId{1});
        EXPECT_EQ(
            *field0.qualifier(), apache::thrift::type::FieldQualifier::Fill);
        EXPECT_EQ(
            field0.type()->toThrift().name()->getType(),
            apache::thrift::type::TypeName::Type::i32Type);
      },
      [](apache::thrift::type::Exceptions& ex) {
        EXPECT_EQ(ex.size(), 1);

        auto field0 = ex.at(0);
        EXPECT_EQ(*field0.name(), "ex0");
        EXPECT_EQ(*field0.id(), apache::thrift::type::FieldId{1});
        EXPECT_EQ(
            *field0.qualifier(), apache::thrift::type::FieldQualifier::Fill);
        EXPECT_EQ(
            field0.type()->toThrift().name()->getType(),
            apache::thrift::type::TypeName::Type::exceptionType);
      },
      &verify_void_return);

  verify_function(
      svc_schema.functions()->at(3),
      "unionParamStructReturnNoEx",
      [](apache::thrift::type::Paramlist& params) {
        auto fields = params.fields();
        EXPECT_EQ(fields->size(), 1);

        auto field0 = fields->at(0);
        EXPECT_EQ(*field0.name(), "param0");
        EXPECT_EQ(*field0.id(), apache::thrift::type::FieldId{1});
        EXPECT_EQ(
            *field0.qualifier(), apache::thrift::type::FieldQualifier::Fill);
        EXPECT_EQ(
            field0.type()->toThrift().name()->getType(),
            apache::thrift::type::TypeName::Type::unionType);
      },
      nullptr,
      [](std::vector<apache::thrift::type::ReturnType>& rets) {
        EXPECT_EQ(rets.size(), 1);
        auto ret1 = rets.at(0);
        EXPECT_EQ(
            ret1.getType(), apache::thrift::type::ReturnType::Type::thriftType);
        EXPECT_EQ(
            ret1.get_thriftType().toThrift().name()->getType(),
            apache::thrift::type::TypeName::Type::structType);
      });
}

TEST(SchemaTest, Enum) {
  apache::thrift::type::Enum schema = schema_constants::schemaEnum();
  EXPECT_EQ(*schema.name(), "Enum");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/Enum");
  auto values = schema.values().value();

  EXPECT_EQ(values.size(), 2);

  auto value0 = values.at(0);
  auto value1 = values.at(1);

  EXPECT_EQ(value0.name(), "unspecified");
  EXPECT_EQ(value0.value().value(), 0);

  EXPECT_EQ(value1.name(), "test");
  EXPECT_EQ(value1.value().value(), 22);
}

TEST(SchemaTest, TypedefFields) {
  apache::thrift::type::Struct schema = schema_constants::schemaTypedefs();
  EXPECT_EQ(schema.fields()->size(), 2);

  auto field = schema.fields()->at(0);
  EXPECT_EQ(*field.name(), "named");
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::typedefType);
  EXPECT_EQ(
      *field.type()->toThrift().name()->typedefType_ref()->uri_ref(),
      "facebook.com/thrift/test/schema/TD");

  field = schema.fields()->at(1);
  EXPECT_EQ(*field.name(), "unnamed");
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::i32Type);
}
TEST(SchemaTest, Typedef) {
  //@thrift.GenerateRuntimeSchema
  // typedef i32 TD
  apache::thrift::type::Typedef schema = schema_constants::schemaTD();
  EXPECT_EQ(*schema.name(), "TD");
  EXPECT_EQ(*schema.uri(), "facebook.com/thrift/test/schema/TD");
  EXPECT_EQ(
      schema.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::i32Type);
  EXPECT_EQ(schema.attrs()->get_name(), "TD");

  //@thrift.GenerateRuntimeSchema
  // typedef TD TDTD
  apache::thrift::type::Typedef schemaTD = schema_constants::schemaTDTD();
  EXPECT_EQ(*schemaTD.name(), "TDTD");
  EXPECT_EQ(*schemaTD.uri(), "facebook.com/thrift/test/schema/TDTD");
  EXPECT_EQ(
      schemaTD.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::typedefType);
  EXPECT_EQ(schemaTD.attrs()->get_name(), "TDTD");
}

TEST(SchemaTest, Annotations) {
  apache::thrift::type::Struct schema = schema_constants::schemaAnnotated();
  bool found = false;
  for (auto id : *schema.structuredAnnotations()) {
    const auto& annot = schema_constants::getValue(id)
                            .as<apache::thrift::type::struct_t<
                                apache::thrift::type::StructuredAnnotation>>();
    if (*annot.type()->uri_ref() == "facebook.com/thrift/test/schema/Annot") {
      found = true;
      EXPECT_EQ(annot.fields()->at("val").as_i64(), 42);
    }
  }
  EXPECT_TRUE(found);

  EXPECT_EQ(schema.unstructuredAnnotations()->at("annot_with_val"), "2023");
  EXPECT_EQ(schema.unstructuredAnnotations()->at("annot_without_val"), "1");
}
