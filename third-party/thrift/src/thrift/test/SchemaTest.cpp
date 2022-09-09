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

TEST(SchemaTest, Fields) {
  auto schema = schema_constants::schemaFields();
  EXPECT_EQ(schema.fields()->size(), 3);

  // 1: i32 num;
  auto field = schema.fields()->at(0);
  EXPECT_EQ(*field.name(), "num");
  EXPECT_EQ(*field.id(), apache::thrift::type::FieldId{1});
  EXPECT_EQ(*field.qualifier(), facebook::thrift::type::FieldQualifier::Fill);
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::i32Type);

  // 3: optional set<string> keyset;
  field = schema.fields()->at(1);
  EXPECT_EQ(*field.name(), "keyset");
  EXPECT_EQ(*field.id(), apache::thrift::type::FieldId{3});
  EXPECT_EQ(
      *field.qualifier(), facebook::thrift::type::FieldQualifier::Optional);
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::setType);
  EXPECT_EQ(field.type()->toThrift().params()->size(), 1);
  EXPECT_EQ(
      field.type()->toThrift().params()->at(0).name()->getType(),
      apache::thrift::type::TypeName::Type::stringType);

  // 7: Empty strct;
  field = schema.fields()->at(2);
  EXPECT_EQ(*field.name(), "strct");
  EXPECT_EQ(*field.id(), apache::thrift::type::FieldId{7});
  EXPECT_EQ(*field.qualifier(), facebook::thrift::type::FieldQualifier::Fill);
  EXPECT_EQ(
      field.type()->toThrift().name()->getType(),
      apache::thrift::type::TypeName::Type::structType);
  EXPECT_EQ(
      *field.type()->toThrift().name()->structType_ref()->uri_ref(),
      "facebook.com/thrift/test/schema/Empty");
}
