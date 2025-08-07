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

#include <vector>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/runtime/BaseSchemaRegistry.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/detail/Merge.h>

// These deps pull in the corresponding schemas when enabled.
#include <thrift/annotation/gen-cpp2/thrift_types.h>
#include <thrift/test/gen-cpp2/TopologicallySortObjectsTest_types.h>
#include <thrift/test/gen-cpp2/UseOpEncode_types.h>
#include <thrift/test/gen-cpp2/schema_types.h>

#include <thrift/test/gen-cpp2/schema_constants.h>
#include <thrift/test/gen-cpp2/schema_handlers.h>

using namespace apache::thrift;
using namespace apache::thrift::schema;
using namespace apache::thrift::schema::detail;
using namespace apache::thrift::syntax_graph;

namespace apache::thrift::test {
struct SchemaTest : public testing::Test {
  auto getMergedSchema(SchemaRegistry& reg) { return reg.getMergedSchema(); }
  auto getMergedSchema() { return SchemaRegistry::get().getMergedSchema(); }
  auto getSyntaxGraphDefinitionNodeByUri(std::string_view uri) {
    return SchemaRegistry::get().getSyntaxGraphDefinitionNodeByUri(uri);
  }
};
TEST_F(SchemaTest, not_linked) {
  auto schemaPtr = getMergedSchema();
  const auto& schema = *schemaPtr;
  for (const auto& program : *schema.programs()) {
    EXPECT_NE(
        program.path(), "thrift/test/TopologicallySortObjectsTest.thrift");
  }

  // Use the types target
  (void)apache::thrift::test::IncompleteMap{};
}

TEST_F(SchemaTest, not_linked_but_included) {
  auto schemaPtr = getMergedSchema();
  const auto& schema = *schemaPtr;
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

TEST_F(SchemaTest, linked) {
  bool found = false;
  auto schemaPtr = getMergedSchema();
  const auto& schema = *schemaPtr;
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
            .structDef()
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

TEST_F(SchemaTest, static_schema) {
  auto static_schema = schema::detail::mergeSchemas(
      facebook::thrift::test::schema::schema_constants::
          _fbthrift_schema_3199f28ddfae7bfa_includes());
  const type::Program* static_program = nullptr;
  for (const auto& program : *static_schema.programs()) {
    if (program.path() == "thrift/test/schema.thrift") {
      static_program = &program;
    }
  }
  ASSERT_TRUE(static_program);

  auto dynamic_schema_ptr = getMergedSchema();
  const type::Program* dynamic_program = nullptr;
  for (const auto& program : *dynamic_schema_ptr->programs()) {
    if (program.path() == "thrift/test/schema.thrift") {
      dynamic_program = &program;
    }
  }
  ASSERT_TRUE(dynamic_program);

  EXPECT_EQ(*static_program, *dynamic_program);
}

TEST_F(SchemaTest, merged_schema_add_after_access) {
  auto data = facebook::thrift::test::schema::schema_constants::
      _fbthrift_schema_3199f28ddfae7bfa();

  BaseSchemaRegistry base;
  SchemaRegistry registry(base);
  auto schemaPtr = getMergedSchema(registry);
  base.registerSchema("schema", {{data}}, "schema.thrift", {}, {});
  auto newSchemaPtr = getMergedSchema(registry);
  // Data is not reused after access.
  EXPECT_EQ(schemaPtr->programs()->size(), 0);
  EXPECT_GT(newSchemaPtr->programs()->size(), 0);
}

TEST_F(SchemaTest, service_schema) {
  ServiceHandler<facebook::thrift::test::schema::TestService> handler;
  auto metadata = handler.getServiceSchema();
  EXPECT_TRUE(metadata);
  EXPECT_EQ(metadata->definitions.size(), 1);
  const auto& service = *metadata->schema.definitionsMap()
                             ->at(metadata->definitions[0])
                             .serviceDef();
  EXPECT_EQ(service.name(), "TestService");
  EXPECT_EQ(service.functions()->size(), 4);
  EXPECT_EQ(service.functions()[0].name(), "noParamsNoReturnNoEx");
  EXPECT_EQ(
      service.functions()[0].returnType()->baseType(), type::BaseType::Void);
}

TEST_F(SchemaTest, schema_data_traits) {
  using apache::thrift::detail::TSchemaAssociation;
  EXPECT_GT(
      TSchemaAssociation<facebook::thrift::test::schema::TestService>::bundle()
          .size(),
      0);
  EXPECT_GT(
      TSchemaAssociation<facebook::thrift::test::schema::Empty>::bundle()
          .size(),
      0);
  EXPECT_GT(
      TSchemaAssociation<facebook::thrift::test::schema::Enum>::bundle().size(),
      0);

  EXPECT_EQ(
      TSchemaAssociation<facebook::thrift::annotation::Experimental>::bundle,
      nullptr);

  EXPECT_EQ(
      TSchemaAssociation<
          facebook::thrift::test::schema::TestService>::programId,
      TSchemaAssociation<facebook::thrift::test::schema::Empty>::programId);
  EXPECT_NE(
      TSchemaAssociation<
          facebook::thrift::test::schema::TestService>::programId,
      TSchemaAssociation<
          facebook::thrift::annotation::Experimental>::programId);

  EXPECT_NE(
      TSchemaAssociation<
          facebook::thrift::test::schema::TestService>::definitionKey,
      TSchemaAssociation<
          facebook::thrift::annotation::Experimental>::definitionKey);
  ServiceHandler<facebook::thrift::test::schema::TestService> handler;
  EXPECT_EQ(
      handler.getServiceSchema()->definitions[0],
      TSchemaAssociation<
          facebook::thrift::test::schema::TestService>::definitionKey);
}

TEST_F(SchemaTest, getSyntaxGraphDefinitionNodeByUri) {
  auto& registry = SchemaRegistry::get();

  auto uri = "facebook.com/thrift/test/schema/Empty";

  const DefinitionNode* dynamicNode = getSyntaxGraphDefinitionNodeByUri(uri);
  EXPECT_TRUE(dynamicNode);

  const DefinitionNode& staticNode =
      registry.getDefinitionNode<facebook::thrift::test::schema::Empty>();
  EXPECT_EQ(uri, staticNode.asStruct().uri());

  EXPECT_EQ(dynamicNode, &staticNode);

  // With `any` disabled, getSyntaxGraphDefinitionNodeByUri fails even after
  // static access.
  const DefinitionNode& staticNodeWithoutAny =
      registry.getDefinitionNode<apache::thrift::test::Foo>();
  auto uriWithoutAny = staticNodeWithoutAny.asStruct().uri();
  EXPECT_FALSE(getSyntaxGraphDefinitionNodeByUri(uriWithoutAny));
}

TEST_F(SchemaTest, getTypeSystemDefinitionRef) {
  auto& registry = SchemaRegistry::get();

  auto uri = "facebook.com/thrift/test/schema/Empty";

  auto dynamicDefinitionRef = registry.getTypeSystemDefinitionRefByUri(uri);
  EXPECT_TRUE(dynamicDefinitionRef);

  const type_system::StructNode& dynamicStructNode =
      dynamicDefinitionRef->asStruct();
  EXPECT_EQ(uri, dynamicStructNode.uri());

  auto staticDefinitionRef =
      registry
          .getTypeSystemDefinitionRef<facebook::thrift::test::schema::Empty>();
  const type_system::StructNode& staticStructNode =
      staticDefinitionRef.asStruct();

  const type_system::StructNode& directStructNode =
      registry.getTypeSystemNode<facebook::thrift::test::schema::Empty>();

  EXPECT_EQ(&dynamicStructNode, &staticStructNode);
  EXPECT_EQ(&dynamicStructNode, &directStructNode);

  // Ensure fields with container types pull in schema of the element type.
  registry.getTypeSystemDefinitionRef<
      facebook::thrift::test::schema::NonSchematizedStruct>();

  dynamicDefinitionRef =
      static_cast<type_system::TypeSystem&>(registry).getUserDefinedType(uri);
  EXPECT_EQ(uri, dynamicDefinitionRef->asStruct().uri());
}

TEST_F(SchemaTest, getSyntaxGraphNode) {
  auto& registry = SchemaRegistry::get();

  auto uri = "facebook.com/thrift/test/schema/Empty";

  auto tsNodeRef = registry.getTypeSystemDefinitionRefByUri(uri);
  EXPECT_TRUE(tsNodeRef);
  const auto& sgNode = registry.getSyntaxGraphNode(tsNodeRef->asStruct());

  EXPECT_EQ(getSyntaxGraphDefinitionNodeByUri(uri), &sgNode);
}

} // namespace apache::thrift::test
