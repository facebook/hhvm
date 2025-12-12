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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/Utility.h>

#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

#include <thrift/annotation/gen-cpp2/thrift_types.h>
#include <thrift/lib/cpp2/schema/test/gen-cpp2/schema_registry_types.h>
#include <thrift/lib/cpp2/schema/test/gen-cpp2/syntax_graph_2_handlers.h>
#include <thrift/lib/cpp2/schema/test/gen-cpp2/syntax_graph_3_types.h>
#include <thrift/lib/cpp2/schema/test/gen-cpp2/syntax_graph_handlers.h>
#include <thrift/lib/cpp2/schema/test/gen-cpp2/syntax_graph_types.h>

#include <fmt/core.h>

#include <algorithm>

namespace type = apache::thrift::type;

namespace apache::thrift::syntax_graph {

namespace {

const Annotation& findAnnotationOrThrow(
    folly::span<const Annotation> annotations, std::string_view name) {
  for (auto& i : annotations) {
    if (i.type().asStruct().definition().name() == name) {
      return i;
    }
  }

  throw std::out_of_range{std::string(name) + " not found"};
}

class ServiceSchemaTest : public testing::Test {
 public:
  template <typename ServiceTag>
  static type::Schema schemaFor() {
    return apache::thrift::ServiceHandler<ServiceTag>()
        .getServiceSchema()
        .value()
        .schema;
  }

  template <typename ServiceTag>
  std::vector<type::DefinitionKey> definitionKeysFor() {
    return apache::thrift::ServiceHandler<ServiceTag>()
        .getServiceSchema()
        .value()
        .definitions;
  }
};

} // namespace

TEST_F(ServiceSchemaTest, Programs) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto programs = syntaxGraph.programs();

  auto& mainProgram = syntaxGraph.findProgramByName("syntax_graph");
  EXPECT_EQ(
      &mainProgram,
      &syntaxGraph.findProgramByPath(
          "thrift/lib/cpp2/schema/test/syntax_graph.thrift"));
  EXPECT_EQ(mainProgram.definitionsByName().size(), 17);
  EXPECT_EQ(mainProgram.namespaces().size(), 1);
  EXPECT_EQ(
      mainProgram.namespaces().at("cpp2"), "apache.thrift.syntax_graph.test");
  {
    ProgramNode::IncludesList includes = mainProgram.includes();
    EXPECT_EQ(includes.size(), 2);

    EXPECT_EQ(includes[0]->name(), "scope");
    EXPECT_EQ(includes[1]->name(), "thrift");

    EXPECT_EQ(includes[0], &syntaxGraph.findProgramByName("scope"));
    EXPECT_EQ(includes[1], &syntaxGraph.findProgramByName("thrift"));
  }

  EXPECT_THROW(
      { syntaxGraph.findProgramByName("syntax_graph_2"); }, std::out_of_range);
}

TEST_F(ServiceSchemaTest, TransitivePrograms) {
  // TestService2 is in a different file, so we should have an *extra* program.
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService2>());
  auto programs = syntaxGraph.programs();

  auto& mainProgram = syntaxGraph.findProgramByName("syntax_graph");
  {
    ProgramNode::IncludesList includes = mainProgram.includes();
    EXPECT_EQ(includes.size(), 2);
    EXPECT_EQ(includes[0], &syntaxGraph.findProgramByName("scope"));
    EXPECT_EQ(includes[1], &syntaxGraph.findProgramByName("thrift"));
  }

  auto& secondProgram = syntaxGraph.findProgramByName("syntax_graph_2");
  EXPECT_EQ(secondProgram.definitionsByName().size(), 2);
  {
    ProgramNode::IncludesList includes = secondProgram.includes();
    EXPECT_EQ(includes.size(), 1);

    EXPECT_EQ(includes[0]->name(), "syntax_graph");
    EXPECT_EQ(includes[0], &mainProgram);
  }
}

TEST_F(ServiceSchemaTest, RawSchemaLifetime) {
  // moved type::Schema&&
  {
    std::optional<type::Schema> schema = schemaFor<test::TestService>();
    auto syntaxGraph = SyntaxGraph::fromSchema(std::move(schema.value()));
    schema.reset();
    EXPECT_NO_THROW({ syntaxGraph.findProgramByName("syntax_graph"); });
  }
  // copied type::Schema&&
  {
    std::optional<type::Schema> schema = schemaFor<test::TestService>();
    auto syntaxGraph = SyntaxGraph::fromSchema(folly::copy(schema.value()));
    schema.reset();
    EXPECT_NO_THROW({ syntaxGraph.findProgramByName("syntax_graph"); });
  }
  // const type::Schema&
  {
    const type::Schema& schema = schemaFor<test::TestService>();
    auto syntaxGraph = SyntaxGraph::fromSchema(&schema);
    EXPECT_NO_THROW({ syntaxGraph.findProgramByName("syntax_graph"); });
  }
}

TEST_F(ServiceSchemaTest, Enum) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testEnum =
      program.definitionsByName().at("TestEnum");
  EXPECT_EQ(&testEnum->program(), &program);
  EXPECT_EQ(testEnum->kind(), DefinitionNode::Kind::ENUM);
  EXPECT_EQ(testEnum->name(), "TestEnum");
  const EnumNode& e = testEnum->asEnum();
  EXPECT_EQ(&e.definition(), testEnum.unwrap());
  EXPECT_EQ(e.uri(), "meta.com/thrift_test/TestEnum");
  EXPECT_EQ(e.definition().annotations().size(), 2);
  EXPECT_EQ(e.definition().annotations()[1].value()["field1"], 3);

  const auto& unset = e.values()[0];
  const auto& value1 = e.values()[1];
  const auto& value2 = e.values()[2];

  EXPECT_EQ(unset.name(), "UNSET");
  EXPECT_EQ(unset.i32(), 0);
  EXPECT_EQ(unset.annotations().size(), 1);
  EXPECT_EQ(unset.annotations()[0].value()["field1"], 4);
  EXPECT_EQ(value1.name(), "VALUE_1");
  EXPECT_EQ(value1.i32(), 1);
  EXPECT_EQ(value2.name(), "VALUE_2");
  EXPECT_EQ(value2.i32(), 2);

  EXPECT_EQ(
      e.toDebugString(),
      "EnumNode 'TestEnum'\n"
      "├─ 'UNSET' → 0\n"
      "├─ 'VALUE_1' → 1\n"
      "╰─ 'VALUE_2' → 2\n");
}

TEST_F(ServiceSchemaTest, Struct) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testStruct =
      program.definitionsByName().at("TestStruct");
  EXPECT_EQ(&testStruct->program(), &program);
  EXPECT_EQ(testStruct->kind(), DefinitionNode::Kind::STRUCT);
  EXPECT_EQ(testStruct->name(), "TestStruct");
  EXPECT_STREQ(testStruct->name().data(), "TestStruct");
  const StructNode& s = testStruct->asStruct();
  EXPECT_EQ(&s.definition(), testStruct.unwrap());
  EXPECT_EQ(s.uri(), "");

  EXPECT_EQ(s.fields().size(), 2);

  auto checkField1 = [](const auto& field) {
    EXPECT_EQ(field.id(), FieldId{1});
    EXPECT_EQ(field.presence(), FieldNode::PresenceQualifier::UNQUALIFIED);
    EXPECT_EQ(field.type().asPrimitive(), Primitive::I32);
    EXPECT_EQ(field.name(), "field1");
    EXPECT_STREQ(field.name().data(), "field1");
    EXPECT_EQ(field.customDefault()->as_i32(), 10);
  };

  checkField1(s.at(FieldId{1}));
  checkField1(s.at("field1"));

  auto checkField2 = [&](const auto& field) {
    EXPECT_EQ(field.id(), FieldId{2});
    EXPECT_EQ(field.presence(), FieldNode::PresenceQualifier::OPTIONAL_);
    EXPECT_EQ(
        &field.type().asEnum(),
        &program.definitionsByName().at("TestEnum")->asEnum());
    EXPECT_EQ(field.name(), "field2");
    EXPECT_STREQ(field.name().data(), "field2");
    EXPECT_EQ(field.customDefault(), nullptr);
  };

  checkField2(s.at(FieldId{2}));
  checkField2(s.at("field2"));

  EXPECT_THROW(s.at(FieldId{3}), std::out_of_range);
  EXPECT_THROW(s.at("field3"), std::out_of_range);

  EXPECT_EQ(
      s.toDebugString(),
      "StructNode 'TestStruct'\n"
      "├─ FieldNode (id=1, presence=UNQUALIFIED, name='field1')\n"
      "│  ├─ type = I32\n"
      "│  ╰─ customDefault = ...\n"
      "╰─ FieldNode (id=2, presence=OPTIONAL, name='field2')\n"
      "   ╰─ type = EnumNode 'TestEnum'\n"
      "      ├─ 'UNSET' → 0\n"
      "      ├─ 'VALUE_1' → 1\n"
      "      ╰─ 'VALUE_2' → 2\n");
}

TEST_F(ServiceSchemaTest, Union) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testUnion =
      program.definitionsByName().at("TestUnion");
  EXPECT_EQ(&testUnion->program(), &program);
  EXPECT_EQ(testUnion->kind(), DefinitionNode::Kind::UNION);
  EXPECT_EQ(testUnion->name(), "TestUnion");
  EXPECT_STREQ(testUnion->name().data(), "TestUnion");
  const UnionNode& u = testUnion->asUnion();

  EXPECT_EQ(u.fields().size(), 2);

  auto checkField1 = [&](const auto& field) {
    EXPECT_EQ(field.id(), FieldId{1});
    EXPECT_EQ(field.presence(), FieldNode::PresenceQualifier::UNQUALIFIED);
    EXPECT_EQ(
        &field.type().asStruct(),
        &program.definitionsByName().at("TestStruct")->asStruct());
    EXPECT_EQ(field.name(), "s");
    EXPECT_STREQ(field.name().data(), "s");
  };

  checkField1(u.at(FieldId{1}));
  checkField1(u.at("s"));

  auto checkField2 = [&](const auto& field) {
    EXPECT_EQ(field.id(), FieldId{2});
    EXPECT_EQ(field.presence(), FieldNode::PresenceQualifier::UNQUALIFIED);
    EXPECT_EQ(
        &field.type().asEnum(),
        &program.definitionsByName().at("TestEnum")->asEnum());
    EXPECT_EQ(field.name(), "e");
  };

  checkField2(u.at(FieldId{2}));
  checkField2(u.at("e"));

  EXPECT_THROW(u.at(FieldId{3}), std::out_of_range);
  EXPECT_THROW(u.at("field3"), std::out_of_range);

  EXPECT_EQ(
      u.toDebugString(),
      "UnionNode 'TestUnion'\n"
      "├─ FieldNode (id=1, presence=UNQUALIFIED, name='s')\n"
      "│  ╰─ type = StructNode 'TestStruct'\n"
      "│     ├─ FieldNode (id=1, presence=UNQUALIFIED, name='field1')\n"
      "│     │  ├─ type = I32\n"
      "│     │  ╰─ customDefault = ...\n"
      "│     ╰─ FieldNode (id=2, presence=OPTIONAL, name='field2')\n"
      "│        ╰─ type = EnumNode 'TestEnum'\n"
      "│           ├─ 'UNSET' → 0\n"
      "│           ├─ 'VALUE_1' → 1\n"
      "│           ╰─ 'VALUE_2' → 2\n"
      "╰─ FieldNode (id=2, presence=UNQUALIFIED, name='e')\n"
      "   ╰─ type = EnumNode 'TestEnum'\n");
}

TEST_F(ServiceSchemaTest, Typedefs) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> listOfTestStructTypedef =
      program.definitionsByName().at("ListOfTestStruct");
  EXPECT_EQ(&listOfTestStructTypedef->program(), &program);
  EXPECT_EQ(listOfTestStructTypedef->kind(), DefinitionNode::Kind::TYPEDEF);
  EXPECT_EQ(listOfTestStructTypedef->name(), "ListOfTestStruct");
  const TypedefNode& t = listOfTestStructTypedef->asTypedef();

  EXPECT_EQ(
      &t.targetType().asList().elementType().asStruct(),
      &program.definitionsByName().at("TestStruct")->asStruct());

  folly::not_null<const DefinitionNode*> typedefToListOfTestStructTypedef =
      program.definitionsByName().at("TypedefToListOfTestStruct");
  EXPECT_EQ(&typedefToListOfTestStructTypedef->program(), &program);
  EXPECT_EQ(
      typedefToListOfTestStructTypedef->kind(), DefinitionNode::Kind::TYPEDEF);
  EXPECT_EQ(
      typedefToListOfTestStructTypedef->name(), "TypedefToListOfTestStruct");
  const TypedefNode& t2 = typedefToListOfTestStructTypedef->asTypedef();
  EXPECT_EQ(t2.targetType().kind(), TypeRef::Kind::TYPEDEF);
  TypeRef t2TypeRef = TypeRef::of(t2);
  EXPECT_EQ(t2TypeRef.kind(), TypeRef::Kind::TYPEDEF);
  EXPECT_EQ(*typedefToListOfTestStructTypedef, t2TypeRef);

  EXPECT_EQ(t2.targetType(), *listOfTestStructTypedef);
  EXPECT_EQ(t2.targetType().trueType(), t.targetType());

  EXPECT_EQ(
      t.toDebugString(),
      "TypedefNode 'ListOfTestStruct'\n"
      "╰─ targetType = List\n"
      "   ╰─ elementType = StructNode 'TestStruct'\n"
      "      ├─ FieldNode (id=1, presence=UNQUALIFIED, name='field1')\n"
      "      │  ├─ type = I32\n"
      "      │  ╰─ customDefault = ...\n"
      "      ╰─ FieldNode (id=2, presence=OPTIONAL, name='field2')\n"
      "         ╰─ type = EnumNode 'TestEnum'\n"
      "            ├─ 'UNSET' → 0\n"
      "            ├─ 'VALUE_1' → 1\n"
      "            ╰─ 'VALUE_2' → 2\n");

  EXPECT_EQ(
      t2.toDebugString(),
      "TypedefNode 'TypedefToListOfTestStruct'\n"
      "╰─ targetType = TypedefNode 'ListOfTestStruct'\n"
      "   ╰─ targetType = List\n"
      "      ╰─ elementType = StructNode 'TestStruct'\n"
      "         ├─ FieldNode (id=1, presence=UNQUALIFIED, name='field1')\n"
      "         │  ├─ type = I32\n"
      "         │  ╰─ customDefault = ...\n"
      "         ╰─ FieldNode (id=2, presence=OPTIONAL, name='field2')\n"
      "            ╰─ type = EnumNode 'TestEnum'\n"
      "               ├─ 'UNSET' → 0\n"
      "               ├─ 'VALUE_1' → 1\n"
      "               ╰─ 'VALUE_2' → 2\n");
}

TEST_F(ServiceSchemaTest, Exception) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testException =
      program.definitionsByName().at("TestException");
  EXPECT_EQ(&testException->program(), &program);
  EXPECT_EQ(testException->kind(), DefinitionNode::Kind::EXCEPTION);
  EXPECT_EQ(testException->name(), "TestException");
  const ExceptionNode& e = testException->asException();
  EXPECT_EQ(e.uri(), "meta.com/thrift_test/TestException");

  EXPECT_EQ(e.fields().size(), 2);

  EXPECT_EQ(e.fields()[0].id(), FieldId{1});
  EXPECT_EQ(e.fields()[0].name(), "blob");
  EXPECT_EQ(e.fields()[0].type().asPrimitive(), Primitive::BINARY);

  EXPECT_EQ(e.fields()[1].id(), FieldId{2});
  EXPECT_EQ(e.fields()[1].name(), "s");
  EXPECT_EQ(
      e.fields()[1].type().asStruct().definition().name(),
      "TestRecursiveStruct");

  EXPECT_EQ(
      e.toDebugString(),
      "ExceptionNode 'TestException'\n"
      "├─ FieldNode (id=1, presence=UNQUALIFIED, name='blob')\n"
      "│  ╰─ type = BINARY\n"
      "╰─ FieldNode (id=2, presence=UNQUALIFIED, name='s')\n"
      "   ╰─ type = StructNode 'TestRecursiveStruct'\n"
      "      ╰─ FieldNode (id=1, presence=OPTIONAL, name='myself')\n"
      "         ╰─ type = StructNode 'TestRecursiveStruct'\n");
}

TEST_F(ServiceSchemaTest, Constant) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testConst =
      program.definitionsByName().at("testConst");
  EXPECT_EQ(&testConst->program(), &program);
  EXPECT_EQ(testConst->kind(), DefinitionNode::Kind::CONSTANT);
  EXPECT_EQ(testConst->name(), "testConst");
  const ConstantNode& c = testConst->asConstant();

  EXPECT_EQ(
      &c.type().asStruct(),
      &program.definitionsByName().at("TestStruct")->asStruct());
  const auto& value = c.value().as_object();
  EXPECT_EQ(value.at(FieldId{1}).as_i32(), 2);

  EXPECT_EQ(
      c.toDebugString(),
      "ConstantNode 'testConst'\n"
      "├─ type = StructNode 'TestStruct'\n"
      "│  ├─ FieldNode (id=1, presence=UNQUALIFIED, name='field1')\n"
      "│  │  ├─ type = I32\n"
      "│  │  ╰─ customDefault = ...\n"
      "│  ╰─ FieldNode (id=2, presence=OPTIONAL, name='field2')\n"
      "│     ╰─ type = EnumNode 'TestEnum'\n"
      "│        ├─ 'UNSET' → 0\n"
      "│        ├─ 'VALUE_1' → 1\n"
      "│        ╰─ 'VALUE_2' → 2\n"
      "╰─ value = ...\n");
}

TEST_F(ServiceSchemaTest, NestedConstant) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testNestedConst =
      program.definitionsByName().at("testNestedConst");
  EXPECT_EQ(&testNestedConst->program(), &program);
  EXPECT_EQ(testNestedConst->kind(), DefinitionNode::Kind::CONSTANT);
  EXPECT_EQ(testNestedConst->name(), "testNestedConst");
  const ConstantNode& c = testNestedConst->asConstant();

  EXPECT_EQ(
      &c.type().asStruct(),
      &program.definitionsByName().at("TestStructuredAnnotation")->asStruct());
  const auto& value = c.value().as_object();
  EXPECT_EQ(value.at(FieldId{1}).as_i64(), 3);
  EXPECT_TRUE(value.at(FieldId{2}).is_object());
  const auto& inner = value.at(FieldId{2}).as_object();
  EXPECT_EQ(inner.at(FieldId{1}).as_i64(), 4);
}

TEST_F(ServiceSchemaTest, Service) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService2>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");
  auto& program2 = syntaxGraph.findProgramByName("syntax_graph_2");

  folly::not_null<const DefinitionNode*> testService =
      program.definitionsByName().at("TestService");
  EXPECT_EQ(&testService->program(), &program);
  EXPECT_EQ(testService->kind(), DefinitionNode::Kind::SERVICE);
  EXPECT_EQ(testService->name(), "TestService");
  const ServiceNode& s = testService->asService();
  EXPECT_EQ(s.baseService(), nullptr);

  EXPECT_EQ(s.functions().size(), 6);

  EXPECT_EQ(s.functions()[0].name(), "foo");
  EXPECT_EQ(
      s.functions()[0].response().type()->asStruct().definition().name(),
      "TestStruct");
  EXPECT_EQ(s.functions()[0].response().sink(), nullptr);
  EXPECT_EQ(s.functions()[0].response().stream(), nullptr);
  EXPECT_EQ(s.functions()[0].response().interaction(), nullptr);
  EXPECT_EQ(s.functions()[0].params().size(), 1);
  EXPECT_EQ(s.functions()[0].params()[0].id(), FieldId{1});
  EXPECT_EQ(s.functions()[0].params()[0].name(), "input");
  EXPECT_EQ(s.functions()[0].params()[0].type(), TypeRef::of(Primitive::I32));
  EXPECT_EQ(s.functions()[0].params()[0].annotations().size(), 1);
  EXPECT_EQ(
      s.functions()[0].params()[0].annotations()[0].value()["field1"].asInt(),
      4);
  EXPECT_EQ(s.functions()[0].exceptions().size(), 1);
  EXPECT_EQ(s.functions()[0].exceptions()[0].id(), FieldId{1});
  EXPECT_EQ(s.functions()[0].exceptions()[0].name(), "ex");
  EXPECT_EQ(
      s.functions()[0].exceptions()[0].type().asException().definition().name(),
      "TestException");
  EXPECT_EQ(s.functions()[0].exceptions()[0].annotations().size(), 1);
  EXPECT_EQ(
      s.functions()[0]
          .exceptions()[0]
          .annotations()[0]
          .value()["field1"]
          .asInt(),
      5);
  EXPECT_EQ(s.functions()[0].qualifier(), type::FunctionQualifier::Unspecified);
  EXPECT_FALSE(s.functions()[0].isPerforms());

  const InteractionNode& i =
      program.definitionsByName().at("TestInteraction")->asInteraction();

  EXPECT_EQ(s.functions()[1].name(), "createInteraction");
  EXPECT_EQ(s.functions()[1].response().type(), nullptr);
  EXPECT_EQ(s.functions()[1].response().sink(), nullptr);
  EXPECT_EQ(s.functions()[1].response().stream(), nullptr);
  EXPECT_EQ(s.functions()[1].response().interaction(), &i);
  EXPECT_EQ(s.functions()[1].qualifier(), type::FunctionQualifier::Unspecified);
  EXPECT_FALSE(s.functions()[1].isPerforms());

  EXPECT_EQ(s.functions()[2].name(), "createStream");
  EXPECT_EQ(*s.functions()[2].response().type(), TypeRef::of(Primitive::I32));
  EXPECT_EQ(
      s.functions()[2].response().stream()->payloadType(),
      TypeRef::of(Primitive::I32));
  EXPECT_EQ(s.functions()[2].qualifier(), type::FunctionQualifier::Unspecified);
  EXPECT_FALSE(s.functions()[2].isPerforms());

  EXPECT_EQ(s.functions()[3].name(), "createInteractionAndStream");
  EXPECT_EQ(*s.functions()[3].response().type(), TypeRef::of(Primitive::I32));
  EXPECT_EQ(s.functions()[3].response().interaction(), &i);
  EXPECT_EQ(
      s.functions()[3].response().stream()->payloadType(),
      TypeRef::of(Primitive::I32));
  EXPECT_EQ(s.functions()[3].response().sink(), nullptr);
  EXPECT_EQ(s.functions()[3].qualifier(), type::FunctionQualifier::Unspecified);
  EXPECT_FALSE(s.functions()[3].isPerforms());

  EXPECT_EQ(s.functions()[4].name(), "createTestInteraction");
  EXPECT_EQ(s.functions()[4].response().type(), nullptr);
  EXPECT_EQ(s.functions()[4].response().sink(), nullptr);
  EXPECT_EQ(s.functions()[4].response().stream(), nullptr);
  EXPECT_EQ(s.functions()[4].response().interaction(), &i);
  EXPECT_EQ(s.functions()[4].qualifier(), type::FunctionQualifier::Unspecified);
  EXPECT_TRUE(s.functions()[4].isPerforms());

  EXPECT_EQ(s.functions()[5].name(), "noReturn");
  EXPECT_EQ(s.functions()[5].response().type(), nullptr);
  EXPECT_EQ(s.functions()[5].response().sink(), nullptr);
  EXPECT_EQ(s.functions()[5].response().stream(), nullptr);
  EXPECT_EQ(s.functions()[5].response().interaction(), nullptr);
  EXPECT_EQ(s.functions()[5].params().size(), 0);
  EXPECT_EQ(s.functions()[5].qualifier(), type::FunctionQualifier::OneWay);
  EXPECT_FALSE(s.functions()[5].isPerforms());

  folly::not_null<const DefinitionNode*> testService2 =
      program2.definitionsByName().at("TestService2");
  EXPECT_EQ(&testService2->program(), &program2);
  EXPECT_EQ(testService2->kind(), DefinitionNode::Kind::SERVICE);
  EXPECT_EQ(testService2->name(), "TestService2");
  const ServiceNode& s2 = testService2->asService();

  EXPECT_EQ(s2.functions().size(), 0);
  EXPECT_EQ(s2.baseService(), &s);

  EXPECT_EQ(
      s2.toDebugString(),
      "ServiceNode (name='TestService2')\n"
      "╰─ baseService = ServiceNode (name='TestService')\n"
      "   ╰─ functions\n"
      "      ├─ FunctionNode (name='foo')\n"
      "      │  ├─ FunctionNode::Response\n"
      "      │  │  ╰─ returnType = StructNode 'TestStruct'\n"
      "      │  │     ├─ FieldNode (id=1, presence=UNQUALIFIED, name='field1')\n"
      "      │  │     │  ├─ type = I32\n"
      "      │  │     │  ╰─ customDefault = ...\n"
      "      │  │     ╰─ FieldNode (id=2, presence=OPTIONAL, name='field2')\n"
      "      │  │        ╰─ type = EnumNode 'TestEnum'\n"
      "      │  │           ├─ 'UNSET' → 0\n"
      "      │  │           ├─ 'VALUE_1' → 1\n"
      "      │  │           ╰─ 'VALUE_2' → 2\n"
      "      │  ├─ params\n"
      "      │  │  ╰─ FunctionNode::Param (id=1, name='input')\n"
      "      │  │     ╰─ type = I32\n"
      "      │  ╰─ exceptions\n"
      "      │     ╰─ FunctionNode::Exception (id=1, name='ex')\n"
      "      │        ╰─ type = ExceptionNode 'TestException'\n"
      "      │           ├─ FieldNode (id=1, presence=UNQUALIFIED, name='blob')\n"
      "      │           │  ╰─ type = BINARY\n"
      "      │           ╰─ FieldNode (id=2, presence=UNQUALIFIED, name='s')\n"
      "      │              ╰─ type = StructNode 'TestRecursiveStruct'\n"
      "      │                 ╰─ FieldNode (id=1, presence=OPTIONAL, name='myself')\n"
      "      │                    ╰─ type = StructNode 'TestRecursiveStruct'\n"
      "      ├─ FunctionNode (name='createInteraction')\n"
      "      │  ╰─ FunctionNode::Response\n"
      "      │     ├─ returnType = void\n"
      "      │     ╰─ InteractionNode (name='TestInteraction')\n"
      "      │        ╰─ functions\n"
      "      │           ╰─ FunctionNode (name='foo')\n"
      "      │              ├─ FunctionNode::Response\n"
      "      │              │  ╰─ returnType = I32\n"
      "      │              ├─ params\n"
      "      │              │  ╰─ FunctionNode::Param (id=1, name='input')\n"
      "      │              │     ╰─ type = StructNode 'TestRecursiveStruct'\n"
      "      │              ╰─ exceptions\n"
      "      │                 ╰─ FunctionNode::Exception (id=1, name='ex')\n"
      "      │                    ╰─ type = ExceptionNode 'TestException'\n"
      "      ├─ FunctionNode (name='createStream')\n"
      "      │  ╰─ FunctionNode::Response\n"
      "      │     ├─ returnType = I32\n"
      "      │     ╰─ FunctionNode::Stream\n"
      "      │        ╰─ payloadType = I32\n"
      "      ├─ FunctionNode (name='createInteractionAndStream')\n"
      "      │  ╰─ FunctionNode::Response\n"
      "      │     ├─ returnType = I32\n"
      "      │     ├─ InteractionNode (name='TestInteraction')\n"
      "      │     ╰─ FunctionNode::Stream\n"
      "      │        ╰─ payloadType = I32\n"
      "      ├─ FunctionNode (name='createTestInteraction')\n"
      "      │  ╰─ FunctionNode::Response\n"
      "      │     ├─ returnType = void\n"
      "      │     ╰─ InteractionNode (name='TestInteraction')\n"
      "      ╰─ FunctionNode (name='noReturn')\n"
      "         ╰─ FunctionNode::Response\n"
      "            ╰─ returnType = void\n");
}

TEST_F(ServiceSchemaTest, Interaction) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testInteraction =
      program.definitionsByName().at("TestInteraction");
  EXPECT_EQ(&testInteraction->program(), &program);
  EXPECT_EQ(testInteraction->kind(), DefinitionNode::Kind::INTERACTION);
  EXPECT_EQ(testInteraction->name(), "TestInteraction");
  const InteractionNode& i = testInteraction->asInteraction();

  EXPECT_EQ(i.functions().size(), 1);
  EXPECT_EQ(i.functions()[0].name(), "foo");
  EXPECT_EQ(*i.functions()[0].response().type(), TypeRef::of(Primitive::I32));
  EXPECT_EQ(i.functions()[0].response().sink(), nullptr);
  EXPECT_EQ(i.functions()[0].response().stream(), nullptr);
  EXPECT_EQ(i.functions()[0].response().interaction(), nullptr);

  const StructNode& testRecursiveStruct =
      program.definitionsByName().at("TestRecursiveStruct")->asStruct();
  EXPECT_EQ(i.functions()[0].params().size(), 1);
  EXPECT_EQ(i.functions()[0].params()[0].id(), FieldId{1});
  EXPECT_EQ(i.functions()[0].params()[0].name(), "input");
  EXPECT_EQ(
      i.functions()[0].params()[0].type(), TypeRef::of(testRecursiveStruct));

  EXPECT_EQ(
      i.toDebugString(),
      "InteractionNode (name='TestInteraction')\n"
      "╰─ functions\n"
      "   ╰─ FunctionNode (name='foo')\n"
      "      ├─ FunctionNode::Response\n"
      "      │  ╰─ returnType = I32\n"
      "      ├─ params\n"
      "      │  ╰─ FunctionNode::Param (id=1, name='input')\n"
      "      │     ╰─ type = StructNode 'TestRecursiveStruct'\n"
      "      │        ╰─ FieldNode (id=1, presence=OPTIONAL, name='myself')\n"
      "      │           ╰─ type = StructNode 'TestRecursiveStruct'\n"
      "      ╰─ exceptions\n"
      "         ╰─ FunctionNode::Exception (id=1, name='ex')\n"
      "            ╰─ type = ExceptionNode 'TestException'\n"
      "               ├─ FieldNode (id=1, presence=UNQUALIFIED, name='blob')\n"
      "               │  ╰─ type = BINARY\n"
      "               ╰─ FieldNode (id=2, presence=UNQUALIFIED, name='s')\n"
      "                  ╰─ type = StructNode 'TestRecursiveStruct'\n");
}

void checkAnnotationsOnTestUnion(const UnionNode& node) {
  const auto& annotations = node.definition().annotations();
  EXPECT_EQ(annotations.size(), 3);
  auto& withUri =
      findAnnotationOrThrow(annotations, "TestStructuredAnnotation");
  EXPECT_EQ(
      &withUri.type().asStruct(),
      &node.definition()
           .program()
           .definitionsByName()
           .at("TestStructuredAnnotation")
           ->asStruct());
  EXPECT_EQ(withUri.value().size(), 2);
  EXPECT_EQ(withUri.value()["field1"].asInt(), 3);
  EXPECT_TRUE(withUri.value()["field2"].isObject());
  const auto& innerField = withUri.value()["field2"];
  EXPECT_EQ(innerField["field1"].asInt(), 4);

  auto& complex = findAnnotationOrThrow(annotations, "ComplexAnnotation");
  EXPECT_EQ(
      &complex.type().asStruct(),
      &node.definition()
           .program()
           .definitionsByName()
           .at("ComplexAnnotation")
           ->asStruct());
  EXPECT_EQ(complex.value().size(), 3);
  EXPECT_EQ(complex.value()["l"][0]["field1"].asInt(), 1);
  EXPECT_EQ(complex.value()["s"][0], "foo");
  EXPECT_EQ(complex.value()["m"]["bar"]["field1"].asInt(), 2);
}

TEST_F(ServiceSchemaTest, StructuredAnnotation) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");
  checkAnnotationsOnTestUnion(
      program.definitionsByName().at("TestUnion")->asUnion());
}

TEST_F(ServiceSchemaTest, StructuredAnnotationWithoutUri) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testException =
      program.definitionsByName().at("TestException");

  const auto& annotations = testException->annotations();
  EXPECT_EQ(annotations.size(), 3);
  const auto& withoutUri =
      findAnnotationOrThrow(annotations, "TestStructuredAnnotationWithoutUri");
  EXPECT_EQ(
      &withoutUri.type().asStruct(),
      &program.definitionsByName()
           .at("TestStructuredAnnotationWithoutUri")
           ->asStruct());
  EXPECT_EQ(withoutUri.value().size(), 1);
  EXPECT_EQ(withoutUri.value()["field1"].asInt(), 3);
}

TEST_F(ServiceSchemaTest, StructuredAnnotationWhichIsATypedef) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  const ServiceNode& testService =
      program.definitionsByName().at("TestService")->asService();
  folly::span<const FunctionNode> functions = testService.functions();
  const FunctionNode& foo = *std::find_if(
      functions.begin(), functions.end(), [](const FunctionNode& f) {
        return f.name() == "foo";
      });

  const auto& annotations = foo.annotations();
  EXPECT_EQ(annotations.size(), 1);
  EXPECT_EQ(
      &annotations[0].type().asTypedef(),
      &program.definitionsByName()
           .at("TypedefToTestStructuredAnnotation")
           ->asTypedef());
  EXPECT_EQ(
      &annotations[0].type().trueType().asStruct(),
      &program.definitionsByName().at("TestStructuredAnnotation")->asStruct());
}

TEST_F(ServiceSchemaTest, StructuredAnnotationOnField) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  const ExceptionNode& testException =
      program.definitionsByName().at("TestException")->asException();
  folly::span<const Annotation> annotations =
      testException.fields()[0].annotations();
  EXPECT_EQ(annotations.size(), 1);

  EXPECT_EQ(
      &annotations[0].type().asStruct(),
      &program.definitionsByName().at("TestStructuredAnnotation")->asStruct());
}

TEST_F(ServiceSchemaTest, RecursiveStruct) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& program = syntaxGraph.findProgramByName("syntax_graph");

  folly::not_null<const DefinitionNode*> testRecursiveStruct =
      program.definitionsByName().at("TestRecursiveStruct");
  const StructNode& s = testRecursiveStruct->asStruct();

  EXPECT_EQ(s.fields().size(), 1);
  EXPECT_EQ(s.fields()[0].type(), TypeRef::of(s));

  EXPECT_EQ(
      s.toDebugString(),
      "StructNode 'TestRecursiveStruct'\n"
      "╰─ FieldNode (id=1, presence=OPTIONAL, name='myself')\n"
      "   ╰─ type = StructNode 'TestRecursiveStruct'\n");
}

TEST_F(ServiceSchemaTest, LookupByDefinitionKeys) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  std::vector<type::DefinitionKey> serviceDefinitionKeys =
      definitionKeysFor<test::TestService>();

  // Otherwise, the test is not meaningful
  ASSERT_FALSE(serviceDefinitionKeys.empty());

  for (const auto& definitionKey : serviceDefinitionKeys) {
    const DefinitionNode& definition =
        detail::lookUpDefinition(syntaxGraph, definitionKey);
    EXPECT_TRUE(definition.isService());
  }
}

TEST_F(ServiceSchemaTest, getServiceSchemaNodes) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  std::vector<type::DefinitionKey> serviceDefinitionKeys =
      definitionKeysFor<test::TestService>();

  ASSERT_EQ(serviceDefinitionKeys.size(), 1);

  const auto* dynamicService =
      &detail::lookUpDefinition(syntaxGraph, serviceDefinitionKeys.back())
           .asService();

  const auto staticServiceNodes =
      apache::thrift::ServiceHandler<test::TestService>()
          .getServiceSchemaNodes();
  ASSERT_EQ(staticServiceNodes.size(), 1);

  const auto* staticService = staticServiceNodes.back().unwrap();

  // static service uses the global registry while dynamic service has its  own
  // SyntaxGraph instance.
  EXPECT_NE(dynamicService, staticService);

  EXPECT_EQ(
      dynamicService->definition().name(), staticService->definition().name());
}

TEST(SyntaxGraphTest, getDefinitionNode) {
  auto& registry = SchemaRegistry::get();

  const DefinitionNode& testStruct =
      registry.getDefinitionNode<test::TestStruct>();
  const StructNode& stct = testStruct.asStruct();
  EXPECT_EQ(stct.definition().name(), "TestStruct");

  const DefinitionNode& testEnum = registry.getDefinitionNode<test::TestEnum>();
  const EnumNode& enm = testEnum.asEnum();
  EXPECT_EQ(enm.definition().name(), "TestEnum");

  const DefinitionNode& testService =
      registry.getDefinitionNode<test::TestService>();
  const ServiceNode& serv = testService.asService();
  EXPECT_EQ(serv.definition().name(), "TestService");

  // Adding to the SyntaxGraph should not invalidate old nodes
  registry.getDefinitionNode<test::OtherTestStruct>();
  EXPECT_EQ(serv.definition().name(), "TestService");
}

TEST(SyntaxGraphTest, getNode) {
  auto& registry = SchemaRegistry::get();

  const StructNode& stct = registry.getNode<test::TestStruct>();
  EXPECT_EQ(stct.definition().name(), "TestStruct");

  const EnumNode& enm = registry.getNode<test::TestEnum>();
  EXPECT_EQ(enm.definition().name(), "TestEnum");

  const ServiceNode& serv = registry.getNode<test::TestService>();
  EXPECT_EQ(serv.definition().name(), "TestService");

  const StructNode& annot =
      registry.getNode<facebook::thrift::annotation::Experimental>();
  EXPECT_EQ(annot.definition().name(), "Experimental");
}

TEST_F(ServiceSchemaTest, Package) {
  auto& registry = SchemaRegistry::get();
  const StructNode& structC =
      registry.getNode<facebook::thrift::test::schema::C>();
  EXPECT_EQ(
      structC.definition().program().package(),
      "facebook.com/thrift/test/schema");
}

TEST(SyntaxGraphTest, getTypeSystemDefinitionRefBySourceIdentifier) {
  auto& registry = SchemaRegistry::get();

  EXPECT_EQ(
      &registry.getTypeSystemDefinitionRef<test::TestUnion>().asUnion(),
      &registry
           .getTypeSystemDefinitionRefBySourceIdentifier(
               {"file://thrift/lib/cpp2/schema/test/syntax_graph.thrift",
                "TestUnion"})
           ->asUnion());
  EXPECT_EQ(
      &registry.getTypeSystemDefinitionRef<test::TestEnum>().asEnum(),
      &registry
           .getTypeSystemDefinitionRefBySourceIdentifier(
               {"file://thrift/lib/cpp2/schema/test/syntax_graph.thrift",
                "TestEnum"})
           ->asEnum());
}

TEST(SyntaxGraphTest, getSourceIdentiferForUserDefinedType) {
  auto& registry = SchemaRegistry::get();

  EXPECT_EQ(
      *registry.getSourceIdentifierByTypeSystemDefinitionRef(
          registry.getTypeSystemDefinitionRef<test::TestUnion>()),
      (type_system::SourceIdentifier{
          "file://thrift/lib/cpp2/schema/test/syntax_graph.thrift",
          "TestUnion"}));
  EXPECT_EQ(
      *registry.getSourceIdentifierByTypeSystemDefinitionRef(
          registry.getTypeSystemDefinitionRef<test::TestEnum>()),
      (type_system::SourceIdentifier{
          "file://thrift/lib/cpp2/schema/test/syntax_graph.thrift",
          "TestEnum"}));
}

TEST_F(ServiceSchemaTest, asTypeSystem) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& mainProgram = syntaxGraph.findProgramByName("syntax_graph");
  auto def = mainProgram.definitionsByName().at("TestRecursiveStruct");
  auto uri = "meta.com/thrift_test/TestRecursiveStruct";
  ASSERT_EQ(def->asStruct().uri(), uri);

  auto& typeSystem = syntaxGraph.asTypeSystem();

  const auto& ref = typeSystem.getUserDefinedTypeOrThrow(uri);
  const type_system::StructNode& structNode = ref.asStruct();
  EXPECT_EQ(structNode.uri(), uri);
  EXPECT_EQ(structNode.fields().size(), 1);
  EXPECT_EQ(structNode.fields()[0].identity().name(), "myself");
  EXPECT_EQ(structNode.fields()[0].identity().id(), FieldId{1});
  EXPECT_EQ(
      structNode.fields()[0].presence(),
      type_system::PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(structNode.fields()[0].type().asStruct().uri(), uri);
  EXPECT_EQ(&structNode.fields()[0].type().asStruct(), &structNode);
  type_system::SourceIdentifierView ident = {
      "file://thrift/lib/cpp2/schema/test/syntax_graph.thrift",
      "TestRecursiveStruct"};
  EXPECT_EQ(
      &structNode,
      &typeSystem.getUserDefinedTypeBySourceIdentifier(ident)->asStruct());
  EXPECT_EQ(ident, typeSystem.getSourceIdentiferForUserDefinedType(ref));

  EXPECT_EQ(&syntaxGraph.asTypeSystemStructNode(def->asStruct()), &structNode);
  const auto& sgStructNode = syntaxGraph.asSyntaxGraphStructNode(structNode);
  EXPECT_EQ(&sgStructNode, &def->asStruct());

  uri = "meta.com/thrift_test/TestUnion";
  const type_system::UnionNode& unionNode =
      typeSystem.getUserDefinedTypeOrThrow(uri).asUnion();
  EXPECT_EQ(unionNode.uri(), uri);
  EXPECT_EQ(unionNode.fields().size(), 2);
  EXPECT_EQ(unionNode.fields()[0].identity().name(), "s");
  EXPECT_EQ(unionNode.fields()[0].identity().id(), FieldId{1});
  EXPECT_EQ(
      unionNode.fields()[0].presence(),
      type_system::PresenceQualifier::OPTIONAL_);
  const auto& sgUnionNode = syntaxGraph.asSyntaxGraphUnionNode(unionNode);
  EXPECT_EQ(
      &mainProgram.definitionsByName().at("TestUnion")->asUnion(),
      &sgUnionNode);
  EXPECT_EQ(
      &unionNode,
      &typeSystem
           .getUserDefinedTypeBySourceIdentifier(
               {"file://thrift/lib/cpp2/schema/test/syntax_graph.thrift",
                "TestUnion"})
           ->asUnion());
  checkAnnotationsOnTestUnion(sgUnionNode);
  {
    const auto& annot = *unionNode.getAnnotationOrNull(
        "meta.com/thrift_test/TestStructuredAnnotation");
    EXPECT_EQ(annot.asFieldSet().at(FieldId{1}).asInt64(), 3);
    EXPECT_EQ(
        annot.asFieldSet().at(FieldId{2}).asFieldSet().at(FieldId{1}).asInt64(),
        4);
    auto s = type_system::embed<type::struct_t<test::TestStructuredAnnotation>>(
        annot);
    EXPECT_EQ(s.field1(), 3);
    EXPECT_EQ(s.field2()->field1(), 4);
    EXPECT_THROW(
        type_system::embed<type::struct_t<test::ComplexAnnotation>>(annot),
        std::runtime_error);
  }
  {
    const auto& annot = *unionNode.getAnnotationOrNull(
        "meta.com/thrift_test/ComplexAnnotation");
    EXPECT_EQ(
        annot.asFieldSet()
            .at(FieldId{1})
            .asList()[0]
            .asFieldSet()
            .at(FieldId{1})
            .asInt64(),
        1);
    EXPECT_TRUE(annot.asFieldSet()
                    .at(FieldId{2})
                    .asSet()
                    .contains(type_system::SerializableRecord::Text("foo")));
    EXPECT_EQ(
        annot.asFieldSet()
            .at(FieldId{3})
            .asMap()
            .at(type_system::SerializableRecord::Text("bar"))
            .asFieldSet()
            .at(FieldId{1})
            .asInt64(),
        2);
    auto s = type_system::embed<type::struct_t<test::ComplexAnnotation>>(annot);
    EXPECT_EQ(s.l()[0].field1(), 1);
    EXPECT_TRUE(s.s()->contains("foo"));
    EXPECT_EQ(s.m()->at("bar").field1(), 2);
    EXPECT_THROW(
        type_system::embed<type::struct_t<test::TestStructuredAnnotation>>(
            annot),
        std::runtime_error);
  }

  uri = "meta.com/thrift_test/TestEnum";
  const type_system::EnumNode& enumNode =
      typeSystem.getUserDefinedTypeOrThrow(uri).asEnum();
  EXPECT_EQ(enumNode.uri(), uri);
  EXPECT_EQ(
      enumNode
          .getAnnotationOrNull("meta.com/thrift_test/TestStructuredAnnotation")
          ->asFieldSet()
          .at(FieldId{1})
          .asInt64(),
      3);
  EXPECT_EQ(enumNode.values().size(), 3);
  EXPECT_EQ(enumNode.values()[0].name, "UNSET");
  EXPECT_EQ(enumNode.values()[0].i32, 0);
  EXPECT_EQ(
      enumNode.values()[0]
          .getAnnotationOrNull("meta.com/thrift_test/TestStructuredAnnotation")
          ->asFieldSet()
          .at(FieldId{1})
          .asInt64(),
      4);
  EXPECT_EQ(enumNode.values()[1].name, "VALUE_1");
  EXPECT_EQ(enumNode.values()[1].i32, 1);
  EXPECT_EQ(enumNode.values()[2].name, "VALUE_2");
  EXPECT_EQ(enumNode.values()[2].i32, 2);
  EXPECT_EQ(
      &enumNode,
      &typeSystem
           .getUserDefinedTypeBySourceIdentifier(
               {"file://thrift/lib/cpp2/schema/test/syntax_graph.thrift",
                "TestEnum"})
           ->asEnum());
  const auto& sgEnumNode = syntaxGraph.asSyntaxGraphEnumNode(enumNode);
  EXPECT_EQ(
      &mainProgram.definitionsByName().at("TestEnum")->asEnum(), &sgEnumNode);

  uri = "meta.com/thrift_test/TestException";
  const type_system::StructNode& exceptionStructNode =
      typeSystem.getUserDefinedTypeOrThrow(uri).asStruct();
  EXPECT_EQ(exceptionStructNode.uri(), uri);
  EXPECT_EQ(exceptionStructNode.fields().size(), 2);
  EXPECT_EQ(exceptionStructNode.fields()[0].identity().name(), "blob");
  EXPECT_TRUE(exceptionStructNode.fields()[0].type().isBinary());
  EXPECT_EQ(exceptionStructNode.fields()[0].identity().id(), FieldId{1});
  EXPECT_EQ(exceptionStructNode.fields()[1].identity().name(), "s");
  EXPECT_EQ(
      exceptionStructNode.fields()[1].type().asStruct().uri(),
      "meta.com/thrift_test/TestRecursiveStruct");
  EXPECT_EQ(exceptionStructNode.fields()[1].identity().id(), FieldId{2});
}

TEST_F(ServiceSchemaTest, asTypeSystemTypeRef) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& mainProgram = syntaxGraph.findProgramByName("syntax_graph");
  {
    auto def = mainProgram.definitionsByName().at("TestRecursiveStruct");
    auto ref = TypeRef::of(def->asStruct());
    auto tsRef = syntaxGraph.asTypeSystemTypeRef(ref);
    EXPECT_EQ(
        tsRef.asStruct().uri(), "meta.com/thrift_test/TestRecursiveStruct");
  }
  {
    auto def =
        mainProgram.definitionsByName().at("TypedefToTestStructuredAnnotation");
    auto ref = TypeRef::of(def->asTypedef());
    auto tsRef = syntaxGraph.asTypeSystemTypeRef(ref);
    EXPECT_EQ(
        tsRef.asStruct().uri(),
        "meta.com/thrift_test/TestStructuredAnnotation");
  }
  {
    auto def = mainProgram.definitionsByName().at("TestException");
    auto ref = TypeRef::of(def->asException());
    EXPECT_THROW(syntaxGraph.asTypeSystemTypeRef(ref), std::runtime_error);
  }
}

TEST_F(ServiceSchemaTest, asSyntaxGraphTypeRef) {
  auto syntaxGraph = SyntaxGraph::fromSchema(schemaFor<test::TestService>());
  auto& typeSystem = syntaxGraph.asTypeSystem();
  auto& mainProgram = syntaxGraph.findProgramByName("syntax_graph");

  // Test primitive types
  {
    auto tsRef = type_system::TypeSystem::Bool();
    auto sgRef = syntaxGraph.asSyntaxGraphTypeRef(tsRef);
    EXPECT_EQ(sgRef.asPrimitive(), Primitive::BOOL);
  }
  {
    auto tsRef = type_system::TypeSystem::I32();
    auto sgRef = syntaxGraph.asSyntaxGraphTypeRef(tsRef);
    EXPECT_EQ(sgRef.asPrimitive(), Primitive::I32);
  }
  {
    auto tsRef = type_system::TypeSystem::String();
    auto sgRef = syntaxGraph.asSyntaxGraphTypeRef(tsRef);
    EXPECT_EQ(sgRef.asPrimitive(), Primitive::STRING);
  }
  {
    auto tsRef = type_system::TypeSystem::Any();
    EXPECT_THROW(syntaxGraph.asSyntaxGraphTypeRef(tsRef), std::runtime_error);
  }

  // Test container types
  {
    auto tsRef = typeSystem.ListOf(type_system::TypeSystem::I32());
    auto sgRef = syntaxGraph.asSyntaxGraphTypeRef(tsRef);
    EXPECT_TRUE(sgRef.isList());
    EXPECT_EQ(sgRef.asList().elementType().asPrimitive(), Primitive::I32);
  }
  {
    auto tsRef = typeSystem.SetOf(type_system::TypeSystem::String());
    auto sgRef = syntaxGraph.asSyntaxGraphTypeRef(tsRef);
    EXPECT_TRUE(sgRef.isSet());
    EXPECT_EQ(sgRef.asSet().elementType().asPrimitive(), Primitive::STRING);
  }
  {
    auto tsRef = typeSystem.MapOf(
        type_system::TypeSystem::I32(), type_system::TypeSystem::String());
    auto sgRef = syntaxGraph.asSyntaxGraphTypeRef(tsRef);
    EXPECT_TRUE(sgRef.isMap());
    EXPECT_EQ(sgRef.asMap().keyType().asPrimitive(), Primitive::I32);
    EXPECT_EQ(sgRef.asMap().valueType().asPrimitive(), Primitive::STRING);
  }

  // Test user-defined types
  {
    auto tsRef = typeSystem.getUserDefinedTypeOrThrow(
        "meta.com/thrift_test/TestRecursiveStruct");
    auto sgRef = syntaxGraph.asSyntaxGraphTypeRef(
        type_system::TypeRef(tsRef.asStruct()));
    EXPECT_TRUE(sgRef.isStruct());
    EXPECT_EQ(
        &sgRef.asStruct(),
        &mainProgram.definitionsByName().at("TestRecursiveStruct")->asStruct());
  }
  {
    auto tsRef =
        typeSystem.getUserDefinedTypeOrThrow("meta.com/thrift_test/TestUnion");
    auto sgRef =
        syntaxGraph.asSyntaxGraphTypeRef(type_system::TypeRef(tsRef.asUnion()));
    EXPECT_TRUE(sgRef.isUnion());
    EXPECT_EQ(
        &sgRef.asUnion(),
        &mainProgram.definitionsByName().at("TestUnion")->asUnion());
  }
  {
    auto tsRef =
        typeSystem.getUserDefinedTypeOrThrow("meta.com/thrift_test/TestEnum");
    auto sgRef =
        syntaxGraph.asSyntaxGraphTypeRef(type_system::TypeRef(tsRef.asEnum()));
    EXPECT_TRUE(sgRef.isEnum());
    EXPECT_EQ(
        &sgRef.asEnum(),
        &mainProgram.definitionsByName().at("TestEnum")->asEnum());
  }
}

TEST(SyntaxGraphTest, getSchemaMerge) {
  auto& registry = SchemaRegistry::get();

  // getNode will merge new schemas to the existing one.
  // This test will check after the merging, old nodes are still valid.
  const auto& other = registry.getNode<test::OtherTestStruct>();
  const auto& test = registry.getNode<test::TestStruct>();
  const auto& prog = registry.getDefinitionNode<test::TestStruct2>().program();
  ASSERT_EQ(prog.includes().size(), 1);
  EXPECT_EQ(prog.includes()[0]->name(), "syntax_graph");
  EXPECT_EQ(test.definition().name(), "TestStruct");
  EXPECT_EQ(other.definition().name(), "OtherTestStruct");
}

TEST(SyntaxGraphTest, anyField) {
  auto& registry = SchemaRegistry::get();
  {
    const auto& s = registry.getNode<test::StructWithAny>();
    const auto& f = s.fields()[0];
    const auto& anyStruct = f.type().trueType();
    EXPECT_EQ(anyStruct.asStruct().uri(), "facebook.com/thrift/type/Any");
  }

  {
    const auto& s = registry.getTypeSystemNode<test::StructWithAny>();
    const auto& f = s.fields()[0];
    EXPECT_TRUE(f.type().isAny());
  }
}

TEST(SyntaxGraphTest, StructWithCustomDefault) {
  auto& registry = SchemaRegistry::get();
  const auto& sgStructNode = registry.getNode<test::StructWithCustomDefault>();

  test::TestUnion testUnion;
  testUnion.e() = test::TestEnum::VALUE_1;
  protocol::Value testUnionValue =
      protocol::asValueStruct<type::union_t<test::TestUnion>>(testUnion);
  testUnionValue.as_object().type() = apache::thrift::uri<test::TestUnion>();

  type_system::SerializableRecord::FieldSet testUnionFieldSet;
  testUnionFieldSet.emplace(
      FieldId{2}, type_system::SerializableRecord::Int32(1));
  type_system::SerializableRecord testUnionRecord(std::move(testUnionFieldSet));

  // SyntaxGraph
  EXPECT_EQ(sgStructNode.fields().size(), 8);
  EXPECT_EQ(
      *sgStructNode.at(FieldId{1}).customDefault(),
      protocol::asValueStruct<type::bool_t>(true));
  EXPECT_EQ(
      *sgStructNode.at(FieldId{2}).customDefault(),
      protocol::asValueStruct<type::i32_t>(10));
  EXPECT_EQ(
      *sgStructNode.at(FieldId{3}).customDefault(),
      protocol::asValueStruct<type::string_t>("foo"));
  EXPECT_EQ(
      *sgStructNode.at(FieldId{4}).customDefault(),
      protocol::asValueStruct<type::binary_t>("bar"));
  EXPECT_EQ(
      sgStructNode.at(FieldId{5}).customDefault()->as_list()[0],
      testUnionValue);
  EXPECT_EQ(
      *sgStructNode.at(FieldId{6}).customDefault(),
      protocol::asValueStruct<type::set<type::i32_t>>({1, 2, 3}));
  EXPECT_EQ(
      sgStructNode.at(FieldId{7})
          .customDefault()
          ->as_map()
          .at(protocol::asValueStruct<type::i32_t>(1)),
      testUnionValue);
  EXPECT_EQ(*sgStructNode.at(FieldId{8}).customDefault(), testUnionValue);

  // TypeSystemFacade
  const auto& tsStructNode =
      registry.getTypeSystemNode<test::StructWithCustomDefault>();
  EXPECT_EQ(tsStructNode.fields().size(), 8);
  EXPECT_EQ(
      *tsStructNode.at(FieldId{1}).customDefault(),
      type_system::SerializableRecord::Bool(true));
  EXPECT_EQ(
      *tsStructNode.at(FieldId{2}).customDefault(),
      type_system::SerializableRecord::Int32(10));
  EXPECT_EQ(
      *tsStructNode.at(FieldId{3}).customDefault(),
      type_system::SerializableRecord::Text("foo"));
  EXPECT_EQ(
      *tsStructNode.at(FieldId{4}).customDefault(),
      type_system::SerializableRecord::ByteArray(
          folly::IOBuf::fromString("bar")));
  EXPECT_EQ(
      tsStructNode.at(FieldId{5}).customDefault()->asList()[0],
      testUnionRecord);
  EXPECT_EQ(
      *tsStructNode.at(FieldId{6}).customDefault(),
      type_system::SerializableRecord::Set(
          {type_system::SerializableRecord::Int32(1),
           type_system::SerializableRecord::Int32(2),
           type_system::SerializableRecord::Int32(3)}));
  EXPECT_EQ(
      tsStructNode.at(FieldId{7})
          .customDefault()
          ->asMap()
          .at(type_system::SerializableRecord::Int32(1)),
      testUnionRecord);
  EXPECT_EQ(*tsStructNode.at(FieldId{8}).customDefault(), testUnionRecord);
}

TEST(SyntaxGraphTest, SerializableTypeSystemBuilder) {
  auto testA = [](const type_system::SerializableTypeSystem& sts) {
    auto& structA = sts.types()
                        ->at("facebook.com/thrift/test/schema/A")
                        .definition()
                        ->structDef()
                        .value();
    EXPECT_EQ(structA.fields()->size(), 1);
    auto& field1 = structA.fields()[0];
    EXPECT_EQ(field1.identity()->id(), FieldId{1});
    EXPECT_EQ(field1.identity()->name(), "field");
    EXPECT_EQ(field1.type()->asUri(), "facebook.com/thrift/test/schema/B");
  };
  auto testB = [](const type_system::SerializableTypeSystem& sts) {
    auto& structB = sts.types()
                        ->at("facebook.com/thrift/test/schema/B")
                        .definition()
                        ->structDef()
                        .value();
    EXPECT_EQ(structB.fields()->size(), 1);
    auto& field1 = structB.fields()[0];
    EXPECT_EQ(field1.identity()->id(), FieldId{1});
    EXPECT_EQ(field1.identity()->name(), "field");
    EXPECT_EQ(field1.type()->asUri(), "facebook.com/thrift/test/schema/C");
  };
  auto testC = [](const type_system::SerializableTypeSystem& sts) {
    auto& structC = sts.types()
                        ->at("facebook.com/thrift/test/schema/C")
                        .definition()
                        ->structDef()
                        .value();
    EXPECT_EQ(structC.fields()->size(), 1);
    EXPECT_EQ(
        structC.annotations()
            ->at("facebook.com/thrift/test/schema/TestAnnot")
            .fieldSetDatum()
            ->at(FieldId{1})
            .textDatum()
            .value(),
        "struct");
    auto& field1 = structC.fields()[0];
    EXPECT_EQ(field1.identity()->id(), FieldId{1});
    EXPECT_EQ(field1.identity()->name(), "field");
    EXPECT_TRUE(field1.type()->isI32());
    EXPECT_FALSE(field1.annotations()->contains(
        "facebook.com/thrift/annotation/cpp/Type"));
  };

  auto& registry = SchemaRegistry::get();
  {
    auto builder =
        type_system::SerializableTypeSystemBuilder::withoutSourceInfo(registry);
    builder.addDefinition("facebook.com/thrift/test/schema/A");
    auto sts = *std::move(builder).build();
    EXPECT_EQ(sts.types()->size(), 4);
    testA(sts);
    testB(sts);
    testC(sts);
  }
  {
    auto builder =
        type_system::SerializableTypeSystemBuilder::withoutSourceInfo(registry);
    builder.addDefinition("facebook.com/thrift/test/schema/B");
    auto sts = *std::move(builder).build();
    EXPECT_EQ(sts.types()->size(), 3);
    testB(sts);
    testC(sts);
  }
  {
    auto builder =
        type_system::SerializableTypeSystemBuilder::withoutSourceInfo(registry);
    builder.addDefinition("facebook.com/thrift/test/schema/C");
    auto sts = *std::move(builder).build();
    EXPECT_EQ(sts.types()->size(), 2);
    testC(sts);
  }
}

} // namespace apache::thrift::syntax_graph
