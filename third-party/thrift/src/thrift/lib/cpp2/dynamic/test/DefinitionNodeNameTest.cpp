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

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

namespace apache::thrift::type_system {

using def = TypeSystemBuilder::DefinitionHelper;

TEST(DefinitionNodeNameTest, StructNode_NameSetWithSourceInfo) {
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct({}),
      def::SourceInfo("path/to/file.thrift", "MyStruct"));

  auto typeSystem = std::move(builder).build();

  const StructNode& node =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/thrift/test/MyStruct")
          .asStruct();
  EXPECT_EQ(node.debugName(), "MyStruct");
}

TEST(DefinitionNodeNameTest, UnionNode_NameSetWithSourceInfo) {
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/thrift/test/MyUnion",
      def::Union({}),
      def::SourceInfo("path/to/file.thrift", "MyUnion"));

  auto typeSystem = std::move(builder).build();

  const UnionNode& node =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/thrift/test/MyUnion")
          .asUnion();
  EXPECT_EQ(node.debugName(), "MyUnion");
}

TEST(DefinitionNodeNameTest, EnumNode_NameSetWithSourceInfo) {
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/thrift/test/MyEnum",
      def::Enum({{"VALUE1", 1}}),
      def::SourceInfo("path/to/file.thrift", "MyEnum"));

  auto typeSystem = std::move(builder).build();

  const EnumNode& node =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/thrift/test/MyEnum")
          .asEnum();
  EXPECT_EQ(node.debugName(), "MyEnum");
}

TEST(DefinitionNodeNameTest, OpaqueAliasNode_NameSetWithSourceInfo) {
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/thrift/test/MyAlias",
      def::OpaqueAlias(TypeIds::I32),
      def::SourceInfo("path/to/file.thrift", "MyAlias"));

  auto typeSystem = std::move(builder).build();

  const OpaqueAliasNode& node =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/thrift/test/MyAlias")
          .asOpaqueAlias();
  EXPECT_EQ(node.debugName(), "MyAlias");
}

TEST(DefinitionNodeNameTest, Uri_EmptyUri_ThrowsErrorWithTypeName) {
  StructNode node{Uri{}, {}, false, {}, "MyStruct"};
  EXPECT_THAT(
      [&] { node.uri(); },
      testing::ThrowsMessage<InvalidTypeError>(
          testing::HasSubstr("Type `MyStruct` does not have URI set.")));
}

TEST(DefinitionNodeNameTest, Uri_EmptyUri_ThrowsErrorWithPackageHint) {
  StructNode node{Uri{}, {}, false, {}, "MyStruct"};
  EXPECT_THAT(
      [&] { node.uri(); },
      testing::ThrowsMessage<InvalidTypeError>(
          testing::HasSubstr("missing a package declaration")));
}

TEST(DefinitionNodeNameTest, Uri_EmptyUri_UnionNode) {
  UnionNode node{Uri{}, {}, false, {}, "MyUnion"};
  EXPECT_THAT(
      [&] { node.uri(); },
      testing::ThrowsMessage<InvalidTypeError>(
          testing::HasSubstr("Type `MyUnion` does not have URI set.")));
}

TEST(DefinitionNodeNameTest, Uri_EmptyUri_EnumNode) {
  EnumNode node{Uri{}, {}, {}, "MyEnum"};
  EXPECT_THAT(
      [&] { node.uri(); },
      testing::ThrowsMessage<InvalidTypeError>(
          testing::HasSubstr("Type `MyEnum` does not have URI set.")));
}

TEST(DefinitionNodeNameTest, Uri_EmptyUri_OpaqueAliasNode) {
  OpaqueAliasNode node{Uri{}, TypeRef{TypeRef::Bool{}}, {}, "MyAlias"};
  EXPECT_THAT(
      [&] { node.uri(); },
      testing::ThrowsMessage<InvalidTypeError>(
          testing::HasSubstr("Type `MyAlias` does not have URI set.")));
}

TEST(DefinitionNodeNameTest, Uri_NonEmpty_ReturnsNormally) {
  StructNode node{Uri{"meta.com/thrift/test/Foo"}, {}, false, {}, "Foo"};
  EXPECT_EQ(node.uri(), "meta.com/thrift/test/Foo");
}

TEST(DefinitionNodeNameTest, DefinitionRef_NameAccessor) {
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      def::SourceInfo("path/to/file.thrift", "MyStruct"));

  auto typeSystem = std::move(builder).build();

  DefinitionRef ref =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/thrift/test/MyStruct");
  EXPECT_EQ(ref.asStruct().debugName(), "MyStruct");
}

} // namespace apache::thrift::type_system
