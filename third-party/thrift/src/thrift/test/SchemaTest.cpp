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
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>

// These deps pull in the corresponding schemas when enabled.
#include <thrift/annotation/gen-cpp2/thrift_types.h>
#include <thrift/test/gen-cpp2/TopologicallySortObjectsTest_types.h>
#include <thrift/test/gen-cpp2/schema_types.h>

using namespace apache::thrift;

TEST(SchemaTest, not_linked) {
  const auto& schema = SchemaRegistry::getMergedSchema();
  for (const auto& program : *schema.programs()) {
    EXPECT_NE(
        program.path(), "thrift/test/TopologicallySortObjectsTest.thrift");
  }

  // Use the types target
  (void)apache::thrift::test::IncompleteMap{};
}

TEST(SchemaTest, not_linked_but_included) {
  const auto& schema = SchemaRegistry::getMergedSchema();
  for (const auto& program : *schema.programs()) {
    if (program.path() != "thrift/annotation/thrift.thrift") {
      continue;
    }
    for (const auto& def : *program.definitionKeys()) {
      EXPECT_FALSE(schema.definitionsMap()->contains(def));
    }
  }

  // Use the types target
  (void)facebook::thrift::annotation::Uri{};
}

TEST(SchemaTest, linked) {
  bool found = false;
  const auto& schema = SchemaRegistry::getMergedSchema();
  for (const auto& program : *schema.programs()) {
    if (program.path() == "thrift/test/schema.thrift") {
      found = true;
    } else {
      continue;
    }

    // Includes definitions from root program
    EXPECT_EQ(
        schema.definitionsMap()
            ->at(program.definitionKeys()[0])
            .structDef_ref()
            ->name(),
        "Empty");

    // Only includes definitions from root program
    EXPECT_EQ(
        schema.definitionsMap()->size(), program.definitionKeys()->size());

    // Transitive includes are listed
    EXPECT_GT(schema.programs()->size(), 4);
  }
  EXPECT_TRUE(found);

  // Use the types target
  (void)facebook::thrift::test::schema::Empty{};
}
