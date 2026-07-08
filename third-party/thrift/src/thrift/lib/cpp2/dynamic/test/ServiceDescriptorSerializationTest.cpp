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

#include <thrift/lib/cpp2/dynamic/ServiceDescriptorBuilder.h>
#include <thrift/lib/cpp2/dynamic/ServiceDescriptorSerialization.h>
#include <thrift/lib/cpp2/dynamic/SyntaxGraphServiceDescriptor.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/ServiceDescriptorTestService.h>

namespace apache::thrift::dynamic {
namespace {

using Handler = apache::thrift::ServiceHandler<
    facebook::thrift::service_descriptor_test::ServiceDescriptorTestService>;

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

class ServiceDescriptorSerializationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<Handler>();
    auto definitionsSchema = handler_->getServiceSchema();
    ASSERT_TRUE(definitionsSchema.has_value());
    syntaxGraph_ = std::make_shared<syntax_graph::SyntaxGraph>(
        syntax_graph::SyntaxGraph::fromSchema(
            apache::thrift::type::Schema(definitionsSchema->schema)));
    typeSystem_ = std::shared_ptr<const type_system::TypeSystem>(
        syntaxGraph_, &syntaxGraph_->asTypeSystem());
  }

  std::shared_ptr<Handler> handler_;
  std::shared_ptr<const syntax_graph::SyntaxGraph> syntaxGraph_;
  std::shared_ptr<const type_system::TypeSystem> typeSystem_;
};

TEST_F(ServiceDescriptorSerializationTest, BuilderRoundTrip) {
  ServiceDescriptorBuilder builder(
      typeSystem_, "TestService", "test.com/TestService");
  builder.addFunction("ping");
  builder.addFunction("add")
      .addParam("a", FieldId{1}, type_system::TypeSystem::I32())
      .addParam("b", FieldId{2}, type_system::TypeSystem::I32())
      .setResponseType(type_system::TypeSystem::I32())
      .setQualifier(FunctionQualifier::Idempotent);
  builder.addFunction("makeInteraction")
      .setCreatedInteractionUri("test.com/TestInteraction");
  builder.addInteraction("TestInteraction", "test.com/TestInteraction")
      .addFunction("getValue")
      .setResponseType(type_system::TypeSystem::I32());

  auto original = builder.build();

  auto serialized = toSerializable(*original, "test.com/TestService");
  ASSERT_EQ(serialized.interfaces()->size(), 2);
  ASSERT_TRUE(serialized.interfaces()->count("test.com/TestService"));
  ASSERT_TRUE(serialized.interfaces()->count("test.com/TestInteraction"));
  const auto& serviceDef =
      *serialized.interfaces()->at("test.com/TestService").serviceDef_ref();
  const auto& makeInteraction =
      serviceDef.functions()->at(2).response()->createsInteraction();
  ASSERT_TRUE(makeInteraction.has_value());
  EXPECT_EQ(*makeInteraction, "test.com/TestInteraction");

  auto deserialized =
      fromSerializable(std::move(serialized), "test.com/TestService");
  EXPECT_EQ(deserialized->serviceName(), "TestService");
  EXPECT_EQ(deserialized->functions().size(), 3);

  const auto& ping = deserialized->getFunctionByName("ping");
  EXPECT_TRUE(ping.params.empty());
  EXPECT_FALSE(ping.responseType.has_value());
  EXPECT_EQ(ping.rpcKind, RpcKind::Unary);

  const auto& add = deserialized->getFunctionByName("add");
  EXPECT_EQ(&deserialized->getFunction("test.com/TestService/add"), &add);
  ASSERT_EQ(add.params.size(), 2);
  EXPECT_EQ(add.params[0].name, "a");
  EXPECT_EQ(add.params[1].name, "b");
  EXPECT_TRUE(add.responseType.has_value());
  EXPECT_TRUE(add.responseType->isI32());
  EXPECT_EQ(add.qualifier, FunctionQualifier::Idempotent);

  const auto& interactionCtor =
      deserialized->getFunctionByName("makeInteraction");
  ASSERT_TRUE(interactionCtor.createdInteractionUri.has_value());
  EXPECT_EQ(*interactionCtor.createdInteractionUri, "test.com/TestInteraction");

  ASSERT_EQ(deserialized->interactions().size(), 1);
  const auto& interaction =
      deserialized->getInteraction(*interactionCtor.createdInteractionUri);
  EXPECT_EQ(interaction.name, "TestInteraction");
  EXPECT_EQ(interaction.uri, "test.com/TestInteraction");
  ASSERT_EQ(interaction.functions.size(), 1);
  const auto& getValue =
      interaction.getFunction("test.com/TestInteraction/getValue");
  EXPECT_EQ(&interaction.getFunctionByName("getValue"), &getValue);
  EXPECT_TRUE(getValue.responseType.has_value());
  EXPECT_TRUE(getValue.responseType->isI32());
}

TEST_F(ServiceDescriptorSerializationTest, StreamRoundTrip) {
  ServiceDescriptorBuilder builder(typeSystem_, "StreamService");
  builder.addFunction("streamInts")
      .setResponseType(type_system::TypeSystem::I32())
      .setStream(type_system::TypeSystem::I32());

  auto original = builder.build();
  auto serialized = toSerializable(*original, "test.com/StreamService");
  auto deserialized =
      fromSerializable(std::move(serialized), "test.com/StreamService");

  const auto& fn = deserialized->getFunctionByName("streamInts");
  ASSERT_TRUE(fn.stream.has_value());
  EXPECT_TRUE(fn.stream->payloadType.isI32());
  EXPECT_EQ(fn.rpcKind, RpcKind::Stream);
}

TEST_F(ServiceDescriptorSerializationTest, SinkRoundTrip) {
  ServiceDescriptorBuilder builder(typeSystem_, "SinkService");
  builder.addFunction("collectStrings")
      .setSink(
          type_system::TypeSystem::String(), type_system::TypeSystem::I32());

  auto original = builder.build();
  auto serialized = toSerializable(*original, "test.com/SinkService");
  auto deserialized =
      fromSerializable(std::move(serialized), "test.com/SinkService");

  const auto& fn = deserialized->getFunctionByName("collectStrings");
  ASSERT_TRUE(fn.sink.has_value());
  EXPECT_TRUE(fn.sink->payloadType.isString());
  ASSERT_TRUE(fn.sink->finalResponseType.has_value());
  EXPECT_TRUE(fn.sink->finalResponseType->isI32());
  EXPECT_EQ(fn.rpcKind, RpcKind::Sink);
}

TEST_F(ServiceDescriptorSerializationTest, BidiRoundTrip) {
  ServiceDescriptorBuilder builder(typeSystem_, "BidiService");
  builder.addFunction("bidiEcho")
      .setBidirectionalStream(
          type_system::TypeSystem::I32(), type_system::TypeSystem::String());

  auto original = builder.build();
  auto serialized = toSerializable(*original, "test.com/BidiService");
  auto deserialized =
      fromSerializable(std::move(serialized), "test.com/BidiService");

  const auto& fn = deserialized->getFunctionByName("bidiEcho");
  ASSERT_TRUE(fn.stream.has_value());
  ASSERT_TRUE(fn.sink.has_value());
  EXPECT_TRUE(fn.stream->payloadType.isI32());
  EXPECT_TRUE(fn.sink->payloadType.isString());
  EXPECT_FALSE(fn.sink->finalResponseType.has_value());
  EXPECT_EQ(fn.rpcKind, RpcKind::BidirectionalStream);
}

TEST_F(ServiceDescriptorSerializationTest, OneWayRoundTrip) {
  ServiceDescriptorBuilder builder(typeSystem_, "OneWayService");
  builder.addFunction("fire").setOneWay();

  auto original = builder.build();
  auto serialized = toSerializable(*original, "test.com/OneWayService");
  auto deserialized =
      fromSerializable(std::move(serialized), "test.com/OneWayService");

  const auto& fn = deserialized->getFunctionByName("fire");
  EXPECT_EQ(fn.rpcKind, RpcKind::OneWay);
}

TEST_F(ServiceDescriptorSerializationTest, MissingInteractionDefinitionThrows) {
  ServiceDescriptorBuilder builder(
      typeSystem_, "TestService", "test.com/TestService");
  builder.addFunction("makeInteraction")
      .setCreatedInteractionUri("test.com/TestInteraction");

  auto catalog = builder.build();
  EXPECT_THROW(
      toSerializable(*catalog, "test.com/TestService"), std::invalid_argument);
}

TEST_F(ServiceDescriptorSerializationTest, SyntaxGraphRoundTrip) {
  auto catalog = std::make_unique<SyntaxGraphServiceDescriptor>(
      syntaxGraph_, findService(*syntaxGraph_, "ServiceDescriptorTestService"));

  auto serialized =
      toSerializable(*catalog, "test.com/ServiceDescriptorTestService");

  auto deserialized = fromSerializable(
      std::move(serialized), "test.com/ServiceDescriptorTestService");

  EXPECT_EQ(deserialized->serviceName(), "ServiceDescriptorTestService");
  EXPECT_EQ(deserialized->functions().size(), catalog->functions().size());

  const auto& add = deserialized->getFunctionByName("add");
  ASSERT_EQ(add.params.size(), 2);
  EXPECT_EQ(add.params[0].name, "a");
  EXPECT_TRUE(add.params[0].type.isI32());
  EXPECT_TRUE(add.responseType.has_value());
  EXPECT_TRUE(add.responseType->isI32());

  const auto& greet = deserialized->getFunctionByName("greet");
  ASSERT_EQ(greet.exceptions.size(), 1);
  EXPECT_EQ(greet.exceptions[0].name, "e");
  EXPECT_TRUE(greet.exceptions[0].type.isStruct());

  const auto& streamFn = deserialized->getFunctionByName("streamNames");
  ASSERT_TRUE(streamFn.stream.has_value());
  EXPECT_TRUE(streamFn.stream->payloadType.isString());
  EXPECT_EQ(streamFn.rpcKind, RpcKind::Stream);

  const auto& createInteraction =
      deserialized->getFunctionByName("createInteraction");
  ASSERT_TRUE(createInteraction.createdInteractionUri.has_value());
  EXPECT_EQ(
      *createInteraction.createdInteractionUri,
      "facebook.com/thrift/service_descriptor_test/TestInteraction");

  const auto& interaction =
      deserialized->getInteraction(*createInteraction.createdInteractionUri);
  EXPECT_EQ(interaction.name, "TestInteraction");
  EXPECT_EQ(
      interaction.uri,
      "facebook.com/thrift/service_descriptor_test/TestInteraction");
  const auto& getValue = interaction.getFunction(
      "facebook.com/thrift/service_descriptor_test/TestInteraction/getValue");
  EXPECT_EQ(&interaction.getFunctionByName("getValue"), &getValue);
  EXPECT_TRUE(getValue.responseType.has_value());
  EXPECT_TRUE(getValue.responseType->isI32());
}

TEST_F(ServiceDescriptorSerializationTest, ServiceNotFound) {
  ServiceDescriptorBuilder builder(typeSystem_, "Svc");
  auto catalog = builder.build();
  auto serialized = toSerializable(*catalog, "test.com/Svc");
  EXPECT_THROW(
      fromSerializable(std::move(serialized), "test.com/Wrong"),
      std::invalid_argument);
}

} // namespace
} // namespace apache::thrift::dynamic
