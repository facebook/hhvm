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

#include <thrift/lib/cpp2/dynamic/DynamicServiceSchemaBuilder.h>
#include <thrift/lib/cpp2/dynamic/SyntaxGraphServiceSchema.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/DynamicServiceSchemaTestService.h>

namespace apache::thrift::dynamic {
namespace {

using Handler = apache::thrift::ServiceHandler<
    facebook::thrift::dynamic_service_schema_test::
        DynamicServiceSchemaTestService>;

const syntax_graph::ServiceNode& findService(
    const syntax_graph::SyntaxGraph& graph, std::string_view name) {
  for (const auto program : graph.programs()) {
    for (const auto definition : program->definitions()) {
      if (definition->isService() &&
          definition->asService().definition().name() == name) {
        return definition->asService();
      }
    }
  }
  throw std::invalid_argument("Service not found: " + std::string(name));
}

class SyntaxGraphServiceSchemaTest : public ::testing::Test {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<Handler>();
    auto definitionsSchema = handler_->getServiceSchema();
    ASSERT_TRUE(definitionsSchema.has_value());
    syntaxGraph_ = std::make_shared<syntax_graph::SyntaxGraph>(
        syntax_graph::SyntaxGraph::fromSchema(
            apache::thrift::type::Schema(definitionsSchema->schema)));
    schema_ = std::make_unique<SyntaxGraphServiceSchema>(
        syntaxGraph_,
        findService(*syntaxGraph_, "DynamicServiceSchemaTestService"));
  }

  std::shared_ptr<Handler> handler_;
  std::shared_ptr<const syntax_graph::SyntaxGraph> syntaxGraph_;
  std::unique_ptr<SyntaxGraphServiceSchema> schema_;
};

TEST_F(SyntaxGraphServiceSchemaTest, ServiceName) {
  EXPECT_EQ(schema_->serviceName(), "DynamicServiceSchemaTestService");
}

TEST_F(SyntaxGraphServiceSchemaTest, BasicFunction) {
  const auto& fn = schema_->getFunction("add");
  EXPECT_EQ(fn.name, "add");
  ASSERT_EQ(fn.params.size(), 2);
  EXPECT_EQ(fn.params[0].name, "a");
  EXPECT_EQ(fn.params[1].name, "b");
  EXPECT_TRUE(fn.responseType.has_value());
  EXPECT_TRUE(fn.exceptions.empty());
  EXPECT_FALSE(fn.stream.has_value());
  EXPECT_FALSE(fn.sink.has_value());
}

TEST_F(SyntaxGraphServiceSchemaTest, VoidFunction) {
  const auto& fn = schema_->getFunction("ping");
  EXPECT_FALSE(fn.responseType.has_value());
  EXPECT_TRUE(fn.params.empty());
}

TEST_F(SyntaxGraphServiceSchemaTest, FunctionWithException) {
  const auto& fn = schema_->getFunction("greet");
  ASSERT_EQ(fn.params.size(), 1);
  EXPECT_EQ(fn.params[0].name, "name");
  ASSERT_EQ(fn.exceptions.size(), 1);
  EXPECT_EQ(fn.exceptions[0].name, "e");
}

TEST_F(SyntaxGraphServiceSchemaTest, StreamFunction) {
  const auto& fn = schema_->getFunction("streamNames");
  EXPECT_TRUE(fn.responseType.has_value());
  ASSERT_TRUE(fn.stream.has_value());
  EXPECT_TRUE(fn.stream->payloadType.isString());
}

TEST_F(SyntaxGraphServiceSchemaTest, SinkFunction) {
  const auto& fn = schema_->getFunction("collectStrings");
  ASSERT_TRUE(fn.sink.has_value());
  EXPECT_TRUE(fn.sink->payloadType.isString());
  EXPECT_TRUE(fn.sink->finalResponseType.isI32());
}

TEST_F(SyntaxGraphServiceSchemaTest, FunctionNotFound) {
  EXPECT_THROW(schema_->getFunction("nonexistent"), std::invalid_argument);
}

TEST_F(SyntaxGraphServiceSchemaTest, GetTypeSystem) {
  auto ts = schema_->getTypeSystem();
  ASSERT_NE(ts, nullptr);
  auto knownUris = ts->getKnownUris();
  ASSERT_TRUE(knownUris.has_value());
  EXPECT_FALSE(knownUris->empty());
}

TEST_F(SyntaxGraphServiceSchemaTest, FunctionsCount) {
  EXPECT_EQ(schema_->functions().size(), 5);
}

class DynamicServiceSchemaBuilderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<Handler>();
    auto definitionsSchema = handler_->getServiceSchema();
    ASSERT_TRUE(definitionsSchema.has_value());
    auto syntaxGraph = std::make_shared<syntax_graph::SyntaxGraph>(
        syntax_graph::SyntaxGraph::fromSchema(
            apache::thrift::type::Schema(definitionsSchema->schema)));
    typeSystem_ = std::shared_ptr<const type_system::TypeSystem>(
        syntaxGraph, &syntaxGraph->asTypeSystem());
  }

  std::shared_ptr<Handler> handler_;
  std::shared_ptr<const type_system::TypeSystem> typeSystem_;
};

TEST_F(DynamicServiceSchemaBuilderTest, BuildBasicSchema) {
  DynamicServiceSchemaBuilder builder(typeSystem_, "TestService");
  builder.addFunction("ping");
  builder.addFunction("add")
      .addParam("a", FieldId{1}, type_system::TypeSystem::I32())
      .addParam("b", FieldId{2}, type_system::TypeSystem::I32())
      .setResponseType(type_system::TypeSystem::I32());

  auto schema = builder.build();
  EXPECT_EQ(schema->serviceName(), "TestService");
  EXPECT_EQ(schema->functions().size(), 2);

  const auto& ping = schema->getFunction("ping");
  EXPECT_TRUE(ping.params.empty());
  EXPECT_FALSE(ping.responseType.has_value());

  const auto& add = schema->getFunction("add");
  ASSERT_EQ(add.params.size(), 2);
  EXPECT_EQ(add.params[0].name, "a");
  EXPECT_EQ(add.params[1].name, "b");
  EXPECT_TRUE(add.responseType.has_value());
}

TEST_F(DynamicServiceSchemaBuilderTest, BuildWithStreamAndSink) {
  DynamicServiceSchemaBuilder builder(typeSystem_, "StreamService");
  builder.addFunction("streamInts")
      .setResponseType(type_system::TypeSystem::I32())
      .setStream(type_system::TypeSystem::I32());
  builder.addFunction("collectStrings")
      .setSink(
          type_system::TypeSystem::String(), type_system::TypeSystem::I32());

  auto schema = builder.build();
  const auto& streamFn = schema->getFunction("streamInts");
  ASSERT_TRUE(streamFn.stream.has_value());
  EXPECT_TRUE(streamFn.stream->payloadType.isI32());

  const auto& sinkFn = schema->getFunction("collectStrings");
  ASSERT_TRUE(sinkFn.sink.has_value());
  EXPECT_TRUE(sinkFn.sink->payloadType.isString());
  EXPECT_TRUE(sinkFn.sink->finalResponseType.isI32());
}

TEST_F(DynamicServiceSchemaBuilderTest, GetTypeSystem) {
  DynamicServiceSchemaBuilder builder(typeSystem_, "Svc");
  auto schema = builder.build();
  EXPECT_EQ(schema->getTypeSystem(), typeSystem_);
}

} // namespace
} // namespace apache::thrift::dynamic
