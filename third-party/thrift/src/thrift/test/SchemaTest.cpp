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
            *field0.qualifier(), apache::thrift::type::FieldQualifier::Default);
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
            *field0.qualifier(), apache::thrift::type::FieldQualifier::Default);
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
            *field0.qualifier(), apache::thrift::type::FieldQualifier::Default);
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
