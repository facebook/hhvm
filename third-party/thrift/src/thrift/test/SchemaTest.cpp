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
