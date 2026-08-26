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

#include <thrift/lib/cpp2/dynamic/Path.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <gtest/gtest.h>

namespace apache::thrift::dynamic {
namespace {

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

// Helper to create container type caches
type_system::detail::ContainerTypeCache& containerCache() {
  static type_system::detail::ContainerTypeCache cache;
  return cache;
}

inline type_system::TypeRef::List makeListType(
    type_system::TypeRef elementType) {
  return type_system::TypeRef::List::of(elementType, containerCache());
}

inline type_system::TypeRef::Set makeSetType(type_system::TypeRef elementType) {
  return type_system::TypeRef::Set::of(elementType, containerCache());
}

inline type_system::TypeRef::Map makeMapType(
    type_system::TypeRef keyType, type_system::TypeRef valueType) {
  return type_system::TypeRef::Map::of(keyType, valueType, containerCache());
}

class PathTest : public ::testing::Test {
 protected:
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  static constexpr auto kUserProfileUri = "meta.com/thrift/test/UserProfile";
  static constexpr auto kMyStructUri = "meta.com/thrift/test/MyStruct";
  static constexpr auto kRootUri = "meta.com/thrift/test/Root";
  static constexpr auto kInner1Uri = "meta.com/thrift/test/Inner1";
  static constexpr auto kInner2Uri = "meta.com/thrift/test/Inner2";

  PathTest() {
    type_system::TypeSystemBuilder builder;

    // Create a nested struct for the users map value
    // struct UserProfile {
    //   1: string name
    //   2: list<i32> scores
    //   3: set<i32> tags
    //   4: any metadata
    // }
    builder.addType(
        kUserProfileUri,
        def::Struct({
            def::Field(
                def::Identity(1, "name"), def::AlwaysPresent, TypeIds::String),
            def::Field(
                def::Identity(2, "scores"),
                def::AlwaysPresent,
                TypeIds::list(TypeIds::I32)),
            def::Field(
                def::Identity(3, "tags"),
                def::AlwaysPresent,
                TypeIds::set(TypeIds::I32)),
            def::Field(
                def::Identity(4, "metadata"), def::Optional, TypeIds::Any),
        }));

    // Create the main struct
    // struct MyStruct {
    //   1: map<string, UserProfile> users
    //   2: map<i32, string> counts
    // }
    builder.addType(
        kMyStructUri,
        def::Struct({
            def::Field(
                def::Identity(1, "users"),
                def::AlwaysPresent,
                TypeIds::map(TypeIds::String, TypeIds::uri(kUserProfileUri))),
            def::Field(
                def::Identity(2, "counts"),
                def::AlwaysPresent,
                TypeIds::map(TypeIds::I32, TypeIds::String)),
        }));

    // Create a simple struct for basic tests
    builder.addType(
        kRootUri,
        def::Struct({
            def::Field(
                def::Identity(1, "a"),
                def::AlwaysPresent,
                TypeIds::uri(kInner1Uri)),
        }));

    builder.addType(
        kInner1Uri,
        def::Struct({
            def::Field(
                def::Identity(1, "b"),
                def::AlwaysPresent,
                TypeIds::uri(kInner2Uri)),
        }));

    builder.addType(
        kInner2Uri,
        def::Struct({
            def::Field(def::Identity(1, "c"), def::AlwaysPresent, TypeIds::I32),
        }));

    typeSystem = std::move(builder).build();
  }

  type_system::TypeRef getMyStructType() {
    return type_system::TypeRef(
        typeSystem->getUserDefinedTypeOrThrow(kMyStructUri).asStruct());
  }

  type_system::TypeRef getUserProfileType() {
    return type_system::TypeRef(
        typeSystem->getUserDefinedTypeOrThrow(kUserProfileUri).asStruct());
  }

  type_system::TypeRef getRootType() {
    return type_system::TypeRef(
        typeSystem->getUserDefinedTypeOrThrow(kRootUri).asStruct());
  }
};

// Tests for Path via PathBuilder

TEST_F(PathTest, PathBasics) {
  PathBuilder builder(getMyStructType());

  EXPECT_EQ(builder.path().toString(), "MyStruct");

  {
    auto g1 = builder.enterField("users");
    EXPECT_EQ(builder.path().toString(), "MyStruct.users");

    {
      auto g2 = builder.enterMapValue("alice");
      EXPECT_EQ(builder.path().toString(), "MyStruct.users[\"alice\"]");

      {
        auto g3 = builder.enterField(FieldId{2});
        auto g4 = builder.enterListElement(0);
        EXPECT_EQ(
            builder.path().toString(), "MyStruct.users[\"alice\"].scores[0]");
      }

      EXPECT_EQ(builder.path().toString(), "MyStruct.users[\"alice\"]");
    }
  }

  EXPECT_EQ(builder.path().toString(), "MyStruct");
}

TEST_F(PathTest, PathAllComponentTypes) {
  PathBuilder builder(getMyStructType());

  {
    auto g1 = builder.enterField("users");
    EXPECT_EQ(builder.path().toString(), "MyStruct.users");
  }

  {
    auto g1 = builder.enterField("users");
    auto g2 = builder.enterMapValue("alice");
    auto g3 = builder.enterField("scores");
    auto g4 = builder.enterListElement(42);
    EXPECT_EQ(
        builder.path().toString(), "MyStruct.users[\"alice\"].scores[42]");
  }

  {
    auto g1 = builder.enterField("users");
    auto g2 = builder.enterMapValue("alice");
    auto g3 = builder.enterField("tags");
    auto g4 = builder.enterSetElement(123);
    EXPECT_EQ(builder.path().toString(), "MyStruct.users[\"alice\"].tags{123}");
  }

  {
    auto g1 = builder.enterField("users");
    auto g2 = builder.enterMapKey("key");
    EXPECT_EQ(builder.path().toString(), "MyStruct.users{\"key\"}");
  }

  {
    auto g1 = builder.enterField("users");
    auto g2 = builder.enterMapValue("alice");
    auto g3 = builder.enterField("metadata");
    auto g4 = builder.enterAnyType(getUserProfileType());
    EXPECT_EQ(
        builder.path().toString(),
        "MyStruct.users[\"alice\"].metadata[meta.com/thrift/test/UserProfile]");
  }
}

// Tests for PathBuilder (typed builder with validation)

TEST_F(PathTest, BuilderAllAccessTypes) {
  PathBuilder builder(getMyStructType());

  // Empty path should just be the root type name
  EXPECT_EQ(builder.toString(), "MyStruct");

  {
    // Enter a field
    auto g1 = builder.enterField("users");
    EXPECT_EQ(builder.toString(), "MyStruct.users");

    {
      // Enter a map value with string key
      auto g2 = builder.enterMapValue("alice");
      EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"]");

      {
        // Enter a nested field
        auto g3 = builder.enterField("scores");
        EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].scores");

        {
          // Enter a list element by index
          auto g4 = builder.enterListElement(0);
          EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].scores[0]");
        }

        // After scope guard destruction, back to scores
        EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].scores");
      }

      // Back to user profile
      EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"]");

      {
        // Enter the tags field (a set)
        auto g3 = builder.enterField("tags");
        EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].tags");

        {
          // Enter a set element with int value
          auto g4 = builder.enterSetElement(42);
          EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].tags{42}");
        }

        EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].tags");
      }

      // Test enterAnyType with known type
      {
        auto g3 = builder.enterField("metadata");
        EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].metadata");

        {
          auto g4 = builder.enterAnyType(getUserProfileType());
          EXPECT_EQ(
              builder.toString(),
              "MyStruct.users[\"alice\"].metadata[meta.com/thrift/test/UserProfile]");

          // Can navigate further since type is known
          {
            auto g5 = builder.enterField("name");
            EXPECT_EQ(
                builder.toString(),
                "MyStruct.users[\"alice\"].metadata[meta.com/thrift/test/UserProfile].name");
          }
        }

        EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].metadata");
      }
    }

    // Back to users field
    EXPECT_EQ(builder.toString(), "MyStruct.users");

    {
      // Enter a map key with string key
      auto g2 = builder.enterMapKey("bob");
      EXPECT_EQ(builder.toString(), "MyStruct.users{\"bob\"}");
    }
  }

  // Back to root
  EXPECT_EQ(builder.toString(), "MyStruct");

  // Test with integer map keys
  {
    auto g1 = builder.enterField("counts");
    EXPECT_EQ(builder.toString(), "MyStruct.counts");

    {
      auto g2 = builder.enterMapKey(123);
      EXPECT_EQ(builder.toString(), "MyStruct.counts{123}");
    }
  }

  // Final check: back to root
  EXPECT_EQ(builder.toString(), "MyStruct");
}

TEST_F(PathTest, BuilderNestedScopes) {
  PathBuilder builder(getRootType());

  {
    auto g1 = builder.enterField("a");
    {
      auto g2 = builder.enterField("b");
      {
        auto g3 = builder.enterField("c");
        EXPECT_EQ(builder.toString(), "Root.a.b.c");
      }
      EXPECT_EQ(builder.toString(), "Root.a.b");
    }
    EXPECT_EQ(builder.toString(), "Root.a");
  }
  EXPECT_EQ(builder.toString(), "Root");
}

TEST_F(PathTest, BuilderExportPath) {
  PathBuilder builder(getMyStructType());

  auto g1 = builder.enterField("users");
  auto g2 = builder.enterMapValue("alice");
  auto g3 = builder.enterField("scores");
  auto g4 = builder.enterListElement(0);

  Path exported = builder.path();
  EXPECT_EQ(exported.toString(), "MyStruct.users[\"alice\"].scores[0]");
}

TEST_F(PathTest, BuilderInvalidFieldAccess) {
  PathBuilder builder(getMyStructType());

  // Accessing a non-existent field should throw
  EXPECT_THROW((void)builder.enterField("nonexistent"), InvalidPathAccessError);

  // Accessing a field on a non-structured type should throw
  {
    auto g1 = builder.enterField("users");
    auto g2 = builder.enterMapValue("alice");
    auto g3 = builder.enterField("scores");
    auto g4 = builder.enterListElement(0);
    // Now current type is i32
    EXPECT_THROW((void)builder.enterField("foo"), InvalidPathAccessError);
  }
}

TEST_F(PathTest, BuilderInvalidListAccess) {
  PathBuilder builder(getMyStructType());

  // Accessing list element on non-list type should throw
  EXPECT_THROW((void)builder.enterListElement(0), InvalidPathAccessError);
}

TEST_F(PathTest, BuilderInvalidSetAccess) {
  PathBuilder builder(getMyStructType());

  // Accessing set element on non-set type should throw
  EXPECT_THROW((void)builder.enterSetElement(42), InvalidPathAccessError);
}

TEST_F(PathTest, BuilderInvalidMapAccess) {
  PathBuilder builder(getMyStructType());

  // Accessing map key on non-map type should throw
  EXPECT_THROW((void)builder.enterMapKey("key"), InvalidPathAccessError);

  // Accessing map value on non-map type should throw
  EXPECT_THROW((void)builder.enterMapValue("key"), InvalidPathAccessError);
}

TEST_F(PathTest, BuilderInvalidAnyAccess) {
  PathBuilder builder(getMyStructType());

  // Accessing any type on non-any type should throw
  EXPECT_THROW(
      (void)builder.enterAnyType(getUserProfileType()), InvalidPathAccessError);
}

TEST_F(PathTest, BuilderAnyType) {
  PathBuilder builder(getMyStructType());

  auto g1 = builder.enterField("users");
  auto g2 = builder.enterMapValue("alice");
  auto g3 = builder.enterField("metadata");

  // Current type is any
  EXPECT_TRUE(builder.currentType().isAny());

  {
    // Enter any type with known inner type (UserProfile)
    auto g4 = builder.enterAnyType(getUserProfileType());
    EXPECT_EQ(
        builder.toString(),
        "MyStruct.users[\"alice\"].metadata[meta.com/thrift/test/UserProfile]");

    // Current type is now UserProfile
    EXPECT_TRUE(builder.currentType().isStruct());

    // Further typed access should work since the inner type is known
    {
      auto g5 = builder.enterField("name");
      EXPECT_EQ(
          builder.toString(),
          "MyStruct.users[\"alice\"].metadata[meta.com/thrift/test/UserProfile].name");
      EXPECT_TRUE(builder.currentType().isString());
    }

    // Can also access other fields
    {
      auto g5 = builder.enterField("scores");
      EXPECT_TRUE(builder.currentType().isList());

      {
        auto g6 = builder.enterListElement(0);
        EXPECT_TRUE(builder.currentType().isI32());
      }
    }

    // Invalid field still throws
    EXPECT_THROW(
        (void)builder.enterField("nonexistent"), InvalidPathAccessError);
  }

  // Back to metadata
  EXPECT_EQ(builder.toString(), "MyStruct.users[\"alice\"].metadata");
  EXPECT_TRUE(builder.currentType().isAny());
}

TEST_F(PathTest, BuilderCurrentType) {
  PathBuilder builder(getMyStructType());

  // Initially, current type is the root struct
  EXPECT_TRUE(builder.currentType().isStruct());

  {
    auto g1 = builder.enterField("users");
    // After entering users field, current type is map<string, UserProfile>
    EXPECT_TRUE(builder.currentType().isMap());

    {
      auto g2 = builder.enterMapValue("alice");
      // After entering map value, current type is UserProfile
      EXPECT_TRUE(builder.currentType().isStruct());

      {
        auto g3 = builder.enterField("scores");
        // After entering scores field, current type is list<i32>
        EXPECT_TRUE(builder.currentType().isList());

        {
          auto g4 = builder.enterListElement(0);
          // After entering list element, current type is i32
          EXPECT_TRUE(builder.currentType().isI32());
        }

        // Back to list
        EXPECT_TRUE(builder.currentType().isList());
      }

      // Back to UserProfile
      EXPECT_TRUE(builder.currentType().isStruct());
    }

    // Back to map
    EXPECT_TRUE(builder.currentType().isMap());
  }

  // Back to root struct
  EXPECT_TRUE(builder.currentType().isStruct());
}

TEST_F(PathTest, BuilderPrimitiveTypes) {
  // Test with primitive types
  PathBuilder boolBuilder(type_system::TypeSystem::Bool());
  EXPECT_EQ(boolBuilder.toString(), "bool");

  PathBuilder i32Builder(type_system::TypeSystem::I32());
  EXPECT_EQ(i32Builder.toString(), "i32");

  PathBuilder stringBuilder(type_system::TypeSystem::String());
  EXPECT_EQ(stringBuilder.toString(), "string");
}

TEST_F(PathTest, BuilderContainerTypes) {
  // Test with container types
  auto listType =
      type_system::TypeRef(makeListType(type_system::TypeSystem::I32()));
  PathBuilder listBuilder(listType);
  EXPECT_EQ(listBuilder.toString(), "list<i32>");

  {
    auto g = listBuilder.enterListElement(5);
    EXPECT_EQ(listBuilder.toString(), "list<i32>[5]");
    EXPECT_TRUE(listBuilder.currentType().isI32());
  }

  auto setType =
      type_system::TypeRef(makeSetType(type_system::TypeSystem::String()));
  PathBuilder setBuilder(setType);
  EXPECT_EQ(setBuilder.toString(), "set<string>");

  {
    auto g = setBuilder.enterSetElement("hello");
    EXPECT_EQ(setBuilder.toString(), "set<string>{\"hello\"}");
    // After entering set element, current type is the element type
    EXPECT_TRUE(setBuilder.currentType().isString());
  }

  auto mapType = type_system::TypeRef(makeMapType(
      type_system::TypeSystem::I32(), type_system::TypeSystem::String()));
  PathBuilder mapBuilder(mapType);
  EXPECT_EQ(mapBuilder.toString(), "map<i32, string>");

  {
    auto g = mapBuilder.enterMapValue(42);
    EXPECT_EQ(mapBuilder.toString(), "map<i32, string>[42]");
    EXPECT_TRUE(mapBuilder.currentType().isString());
  }

  {
    auto g = mapBuilder.enterMapKey(42);
    EXPECT_EQ(mapBuilder.toString(), "map<i32, string>{42}");
    // After entering map key, current type is the key type
    EXPECT_TRUE(mapBuilder.currentType().isI32());
  }
}

} // namespace
} // namespace apache::thrift::dynamic
