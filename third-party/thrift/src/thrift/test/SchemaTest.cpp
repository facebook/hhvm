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

#include <thrift/test/gen-cpp2/schema_constants.h>
#include <thrift/test/gen-cpp2/schema_handlers.h>

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
    EXPECT_GT(schema.programs()->size(), 3);
  }
  EXPECT_TRUE(found);

  // Use the types target
  (void)facebook::thrift::test::schema::Empty{};
}

TEST(SchemaTest, static_schema) {
  auto static_schema = SchemaRegistry::mergeSchemas(
      facebook::thrift::test::schema::schema_constants::
          _fbthrift_schema_b5658a7a6a556e3f_includes());
  const type::Program* static_program = nullptr;
  for (const auto& program : *static_schema.programs()) {
    if (program.path() == "thrift/test/schema.thrift") {
      static_program = &program;
    }
  }
  ASSERT_TRUE(static_program);

  const auto& dynamic_schema = SchemaRegistry::getMergedSchema();
  const type::Program* dynamic_program = nullptr;
  for (const auto& program : *dynamic_schema.programs()) {
    if (program.path() == "thrift/test/schema.thrift") {
      dynamic_program = &program;
    }
  }
  ASSERT_TRUE(dynamic_program);

  EXPECT_EQ(*static_program, *dynamic_program);
}

TEST(SchemaTest, service_schema) {
  ServiceHandler<facebook::thrift::test::schema::TestService> handler;
  auto metadata = handler.getServiceSchema();
  EXPECT_TRUE(metadata);
  EXPECT_EQ(metadata->definitions.size(), 1);
  const auto& service = *metadata->schema.definitionsMap()
                             ->at(metadata->definitions[0])
                             .serviceDef_ref();
  EXPECT_EQ(service.name(), "TestService");
  EXPECT_EQ(service.functions()->size(), 4);
  EXPECT_EQ(service.functions()[0].name(), "noParamsNoReturnNoEx");
  EXPECT_EQ(
      service.functions()[0].returnType()->baseType(), type::BaseType::Void);
}
