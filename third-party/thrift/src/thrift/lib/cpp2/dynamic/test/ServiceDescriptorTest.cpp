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

#include <thrift/lib/cpp2/dynamic/AnnotationValue.h>
#include <thrift/lib/cpp2/dynamic/ServiceDescriptorBuilder.h>
#include <thrift/lib/cpp2/dynamic/SyntaxGraphServiceDescriptor.h>

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
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

class SyntaxGraphServiceDescriptorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<Handler>();
    auto definitionsSchema = handler_->getServiceSchema();
    ASSERT_TRUE(definitionsSchema.has_value());
    syntaxGraph_ = std::make_shared<syntax_graph::SyntaxGraph>(
        syntax_graph::SyntaxGraph::fromSchema(
            apache::thrift::type::Schema(definitionsSchema->schema)));
    catalog_ = std::make_unique<SyntaxGraphServiceDescriptor>(
        syntaxGraph_,
        findService(*syntaxGraph_, "ServiceDescriptorTestService"));
  }

  std::shared_ptr<Handler> handler_;
  std::shared_ptr<const syntax_graph::SyntaxGraph> syntaxGraph_;
  std::unique_ptr<SyntaxGraphServiceDescriptor> catalog_;
};

TEST_F(SyntaxGraphServiceDescriptorTest, ServiceName) {
  EXPECT_EQ(catalog_->serviceName(), "ServiceDescriptorTestService");
}

TEST_F(SyntaxGraphServiceDescriptorTest, BasicFunction) {
  const auto& fn = catalog_->getFunction("add");
  EXPECT_EQ(fn.name, "add");
  ASSERT_EQ(fn.params.size(), 2);
  EXPECT_EQ(fn.params[0].name, "a");
  EXPECT_EQ(fn.params[1].name, "b");
  EXPECT_TRUE(fn.responseType.has_value());
  EXPECT_TRUE(fn.exceptions.empty());
  EXPECT_FALSE(fn.stream.has_value());
  EXPECT_FALSE(fn.sink.has_value());
  EXPECT_EQ(fn.qualifier, FunctionQualifier::Unspecified);
  EXPECT_EQ(fn.rpcKind, RpcKind::Unary);
  EXPECT_FALSE(fn.createsInteraction);
  EXPECT_FALSE(fn.isPerforms);
}

TEST_F(SyntaxGraphServiceDescriptorTest, VoidFunction) {
  const auto& fn = catalog_->getFunction("ping");
  EXPECT_FALSE(fn.responseType.has_value());
  EXPECT_TRUE(fn.params.empty());
}

TEST_F(SyntaxGraphServiceDescriptorTest, FunctionWithException) {
  const auto& fn = catalog_->getFunction("greet");
  ASSERT_EQ(fn.params.size(), 1);
  EXPECT_EQ(fn.params[0].name, "name");
  ASSERT_EQ(fn.exceptions.size(), 1);
  EXPECT_EQ(fn.exceptions[0].name, "e");
}

TEST_F(SyntaxGraphServiceDescriptorTest, StreamFunction) {
  const auto& fn = catalog_->getFunction("streamNames");
  EXPECT_TRUE(fn.responseType.has_value());
  ASSERT_TRUE(fn.stream.has_value());
  EXPECT_TRUE(fn.stream->payloadType.isString());
  EXPECT_EQ(fn.rpcKind, RpcKind::Stream);
}

TEST_F(SyntaxGraphServiceDescriptorTest, SinkFunction) {
  const auto& fn = catalog_->getFunction("collectStrings");
  ASSERT_TRUE(fn.sink.has_value());
  EXPECT_TRUE(fn.sink->payloadType.isString());
  EXPECT_TRUE(
      fn.sink->finalResponseType && fn.sink->finalResponseType->isI32());
  EXPECT_EQ(fn.rpcKind, RpcKind::Sink);
}

TEST_F(SyntaxGraphServiceDescriptorTest, BidiFunction) {
  const auto& fn = catalog_->getFunction("bidiEcho");
  ASSERT_TRUE(fn.stream.has_value());
  ASSERT_TRUE(fn.sink.has_value());
  EXPECT_TRUE(fn.stream->payloadType.isI32());
  EXPECT_TRUE(fn.sink->payloadType.isString());
  EXPECT_FALSE(fn.sink->finalResponseType.has_value());
  EXPECT_EQ(fn.rpcKind, RpcKind::BidirectionalStream);
}

TEST_F(SyntaxGraphServiceDescriptorTest, OneWayFunction) {
  const auto& fn = catalog_->getFunction("fireAndForget");
  EXPECT_EQ(fn.rpcKind, RpcKind::OneWay);
  EXPECT_EQ(fn.qualifier, FunctionQualifier::Unspecified);
  EXPECT_FALSE(fn.responseType.has_value());
}

TEST_F(SyntaxGraphServiceDescriptorTest, AnnotatedFunction) {
  const auto& fn = catalog_->getFunction("annotated");
  EXPECT_EQ(fn.rpcKind, RpcKind::Unary);

  ASSERT_EQ(fn.annotations.size(), 1);
  const auto& annotation = fn.annotations[0];
  ASSERT_TRUE(annotation.type().isStruct());
  const auto& fields = annotation.asStruct();
  EXPECT_EQ(*fields.getField("label"), DynamicValue::makeString("hello"));
  EXPECT_EQ(*fields.getField("number"), DynamicValue::makeI32(7));

  auto tags = fields.getField("tags");
  ASSERT_TRUE(tags.has_value());
  const auto& tagList = tags->asList();
  ASSERT_EQ(tagList.size(), 2);
  EXPECT_EQ(tagList.at(0), DynamicValue::makeString("a"));
  EXPECT_EQ(tagList.at(1), DynamicValue::makeString("b"));
}

TEST_F(SyntaxGraphServiceDescriptorTest, AnnotatedParam) {
  const auto& fn = catalog_->getFunction("add");
  ASSERT_EQ(fn.params.size(), 2);
  ASSERT_EQ(fn.params[0].annotations.size(), 1);
  const auto& fields = fn.params[0].annotations[0].asStruct();
  EXPECT_EQ(*fields.getField("label"), DynamicValue::makeString("param"));
  EXPECT_EQ(*fields.getField("number"), DynamicValue::makeI32(1));
  EXPECT_TRUE(fn.params[1].annotations.empty());
}

TEST_F(SyntaxGraphServiceDescriptorTest, AnnotatedException) {
  const auto& fn = catalog_->getFunction("greet");
  ASSERT_EQ(fn.exceptions.size(), 1);
  ASSERT_EQ(fn.exceptions[0].annotations.size(), 1);
  const auto& fields = fn.exceptions[0].annotations[0].asStruct();
  EXPECT_EQ(*fields.getField("label"), DynamicValue::makeString("exception"));
  EXPECT_EQ(*fields.getField("number"), DynamicValue::makeI32(2));
}

TEST_F(SyntaxGraphServiceDescriptorTest, AnnotatedService) {
  ASSERT_EQ(catalog_->annotations().size(), 1);
  const auto& fields = catalog_->annotations()[0].asStruct();
  EXPECT_EQ(*fields.getField("label"), DynamicValue::makeString("service"));
  EXPECT_EQ(*fields.getField("number"), DynamicValue::makeI32(100));
}

TEST(ToSerializableRecordTest, Scalars) {
  EXPECT_TRUE(toSerializableRecord(DynamicValue::makeBool(true)).asBool());
  EXPECT_EQ(toSerializableRecord(DynamicValue::makeByte(7)).asInt8(), 7);
  EXPECT_EQ(toSerializableRecord(DynamicValue::makeI16(300)).asInt16(), 300);
  EXPECT_EQ(
      toSerializableRecord(DynamicValue::makeI32(70000)).asInt32(), 70000);
  EXPECT_EQ(
      toSerializableRecord(DynamicValue::makeI64(int64_t{1} << 40)).asInt64(),
      int64_t{1} << 40);
  EXPECT_EQ(
      toSerializableRecord(DynamicValue::makeFloat(1.5f)).asFloat32(), 1.5f);
  EXPECT_EQ(
      toSerializableRecord(DynamicValue::makeDouble(2.5)).asFloat64(), 2.5);
  EXPECT_EQ(
      toSerializableRecord(DynamicValue::makeString("hi")).asText(), "hi");
  EXPECT_TRUE(
      toSerializableRecord(
          DynamicValue::makeBinary(folly::IOBuf::copyBuffer("bytes"))) ==
      folly::IOBuf::copyBuffer("bytes"));
}

TEST_F(SyntaxGraphServiceDescriptorTest, ToSerializableRecordStruct) {
  using type_system::SerializableRecord;
  const auto& fn = catalog_->getFunction("annotated");
  ASSERT_EQ(fn.annotations.size(), 1);
  auto record = toSerializableRecord(fn.annotations[0]);
  const auto& fields = record.asFieldSet();
  EXPECT_EQ(fields.at(FieldId{1}).asText(), "hello");
  EXPECT_EQ(fields.at(FieldId{2}).asInt32(), 7);
  EXPECT_EQ(fields.at(FieldId{4}).asInt32(), 2); // Color.Blue
  const auto& tags = fields.at(FieldId{3}).asList();
  ASSERT_EQ(tags.size(), 2);
  EXPECT_EQ(tags.at(0).asText(), "a");
  EXPECT_EQ(tags.at(1).asText(), "b");
  const auto& scores = fields.at(FieldId{5}).asMap();
  ASSERT_EQ(scores.size(), 1);
  EXPECT_EQ(
      scores.at(SerializableRecord{SerializableRecord::Text("x")}).asInt32(),
      5);
}

TEST_F(SyntaxGraphServiceDescriptorTest, SerializeAnnotations) {
  const auto& fn = catalog_->getFunction("annotated");
  auto serialized = serializeAnnotations(fn.annotations);
  const auto& uri = fn.annotations[0].type().asStruct().uri();
  ASSERT_TRUE(serialized.contains(uri));
  auto wire = serialized.at(uri);
  auto record = type_system::SerializableRecord::fromThrift(std::move(wire));
  EXPECT_EQ(record.asFieldSet().at(FieldId{1}).asText(), "hello");
}

TEST_F(SyntaxGraphServiceDescriptorTest, FunctionNotFound) {
  EXPECT_THROW(catalog_->getFunction("nonexistent"), std::invalid_argument);
}

TEST_F(SyntaxGraphServiceDescriptorTest, GetTypeSystem) {
  auto ts = catalog_->getTypeSystem();
  ASSERT_NE(ts, nullptr);
  auto knownUris = ts->getKnownUris();
  ASSERT_TRUE(knownUris.has_value());
  EXPECT_FALSE(knownUris->empty());
}

TEST_F(SyntaxGraphServiceDescriptorTest, FunctionsCount) {
  EXPECT_EQ(catalog_->functions().size(), 8);
}

class ServiceDescriptorBuilderTest : public ::testing::Test {
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

TEST_F(ServiceDescriptorBuilderTest, BuildBasicSchema) {
  ServiceDescriptorBuilder builder(typeSystem_, "TestService");
  builder.addFunction("ping");
  builder.addFunction("add")
      .addParam("a", FieldId{1}, type_system::TypeSystem::I32())
      .addParam("b", FieldId{2}, type_system::TypeSystem::I32())
      .setResponseType(type_system::TypeSystem::I32());

  auto catalog = builder.build();
  EXPECT_EQ(catalog->serviceName(), "TestService");
  EXPECT_EQ(catalog->functions().size(), 2);

  const auto& ping = catalog->getFunction("ping");
  EXPECT_TRUE(ping.params.empty());
  EXPECT_FALSE(ping.responseType.has_value());

  const auto& add = catalog->getFunction("add");
  ASSERT_EQ(add.params.size(), 2);
  EXPECT_EQ(add.params[0].name, "a");
  EXPECT_EQ(add.params[1].name, "b");
  EXPECT_TRUE(add.responseType.has_value());
}

TEST_F(ServiceDescriptorBuilderTest, BuildWithStreamAndSink) {
  ServiceDescriptorBuilder builder(typeSystem_, "StreamService");
  builder.addFunction("streamInts")
      .setResponseType(type_system::TypeSystem::I32())
      .setStream(type_system::TypeSystem::I32());
  builder.addFunction("collectStrings")
      .setSink(
          type_system::TypeSystem::String(), type_system::TypeSystem::I32());

  auto catalog = builder.build();
  const auto& streamFn = catalog->getFunction("streamInts");
  ASSERT_TRUE(streamFn.stream.has_value());
  EXPECT_TRUE(streamFn.stream->payloadType.isI32());
  EXPECT_EQ(streamFn.rpcKind, RpcKind::Stream);

  const auto& sinkFn = catalog->getFunction("collectStrings");
  ASSERT_TRUE(sinkFn.sink.has_value());
  EXPECT_TRUE(sinkFn.sink->payloadType.isString());
  EXPECT_TRUE(
      sinkFn.sink->finalResponseType &&
      sinkFn.sink->finalResponseType->isI32());
  EXPECT_EQ(sinkFn.rpcKind, RpcKind::Sink);
}

TEST_F(ServiceDescriptorBuilderTest, BuildWithBidi) {
  ServiceDescriptorBuilder builder(typeSystem_, "BidiService");
  builder.addFunction("bidiEcho")
      .setBidirectionalStream(
          type_system::TypeSystem::I32(), type_system::TypeSystem::String());

  auto catalog = builder.build();
  const auto& fn = catalog->getFunction("bidiEcho");
  ASSERT_TRUE(fn.stream.has_value());
  ASSERT_TRUE(fn.sink.has_value());
  EXPECT_TRUE(fn.stream->payloadType.isI32());
  EXPECT_TRUE(fn.sink->payloadType.isString());
  EXPECT_FALSE(fn.sink->finalResponseType.has_value());
  EXPECT_EQ(fn.rpcKind, RpcKind::BidirectionalStream);
}

TEST_F(ServiceDescriptorBuilderTest, BuildWithOneWay) {
  ServiceDescriptorBuilder builder(typeSystem_, "OneWayService");
  builder.addFunction("fire").setOneWay();

  auto catalog = builder.build();
  const auto& fn = catalog->getFunction("fire");
  EXPECT_EQ(fn.rpcKind, RpcKind::OneWay);
}

TEST_F(ServiceDescriptorBuilderTest, BuildWithQualifier) {
  ServiceDescriptorBuilder builder(typeSystem_, "QualService");
  builder.addFunction("readOp")
      .setQualifier(FunctionQualifier::ReadOnly)
      .setResponseType(type_system::TypeSystem::I32());

  auto catalog = builder.build();
  const auto& fn = catalog->getFunction("readOp");
  EXPECT_EQ(fn.qualifier, FunctionQualifier::ReadOnly);
}

TEST_F(ServiceDescriptorBuilderTest, BuildWithFunctionMetadata) {
  ServiceDescriptorBuilder builder(typeSystem_, "MetaService");
  builder.addFunction("doThing")
      .setCreatesInteraction(true)
      .setIsPerforms(true)
      .setDocBlock("Performs the thing.")
      .addAnnotation(DynamicValue::makeString("cpp.name"))
      .setResponseType(type_system::TypeSystem::I32());

  auto catalog = builder.build();
  const auto& fn = catalog->getFunction("doThing");
  EXPECT_TRUE(fn.createsInteraction);
  EXPECT_TRUE(fn.isPerforms);
  ASSERT_TRUE(fn.docBlock.has_value());
  EXPECT_EQ(*fn.docBlock, "Performs the thing.");
  ASSERT_EQ(fn.annotations.size(), 1);
  EXPECT_EQ(fn.annotations[0], DynamicValue::makeString("cpp.name"));
}

TEST_F(ServiceDescriptorBuilderTest, BuildWithAnnotations) {
  ServiceDescriptorBuilder builder(typeSystem_, "AnnoService");
  builder.addServiceAnnotation(DynamicValue::makeString("svc.anno"));
  builder.addFunction("op")
      .addParam(
          "p",
          FieldId{1},
          type_system::TypeSystem::I32(),
          {DynamicValue::makeString("param.anno")})
      .addException(
          "e",
          FieldId{2},
          type_system::TypeSystem::I32(),
          {DynamicValue::makeString("ex.anno")})
      .setResponseType(type_system::TypeSystem::I32());

  auto catalog = builder.build();
  ASSERT_EQ(catalog->annotations().size(), 1);
  EXPECT_EQ(catalog->annotations()[0], DynamicValue::makeString("svc.anno"));

  const auto& fn = catalog->getFunction("op");
  ASSERT_EQ(fn.params.size(), 1);
  ASSERT_EQ(fn.params[0].annotations.size(), 1);
  EXPECT_EQ(
      fn.params[0].annotations[0], DynamicValue::makeString("param.anno"));
  ASSERT_EQ(fn.exceptions.size(), 1);
  ASSERT_EQ(fn.exceptions[0].annotations.size(), 1);
  EXPECT_EQ(
      fn.exceptions[0].annotations[0], DynamicValue::makeString("ex.anno"));
}

TEST_F(ServiceDescriptorBuilderTest, GetTypeSystem) {
  ServiceDescriptorBuilder builder(typeSystem_, "Svc");
  auto catalog = builder.build();
  EXPECT_EQ(catalog->getTypeSystem(), typeSystem_);
}

} // namespace
} // namespace apache::thrift::dynamic
