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

#include <thrift/lib/cpp2/dynamic/Any.h>
#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>
#include <thrift/lib/cpp2/type/Any.h>

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::dynamic {

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

// Helper to get a TypeSystem instance for testing
static type_system::TypeSystem& getTestTypeSystem() {
  static auto ts = [] {
    // Create a TypeSystem with a struct that has an Any field
    type_system::TypeSystemBuilder builder;

    // Define a struct with fields:
    // 1: i32 id
    // 2: string name
    // 3: Any metadata (optional)
    builder.addType(
        "facebook.com/thrift/test/StructWithAny",
        def::Struct({
            def::Field(
                def::Identity(1, "id"), def::AlwaysPresent, TypeIds::I32),
            def::Field(
                def::Identity(2, "name"), def::AlwaysPresent, TypeIds::String),
            def::Field(
                def::Identity(3, "metadata"), def::Optional, TypeIds::Any),
        }));

    return std::move(builder).build();
  }();
  return *ts;
}

TEST(AnyTest, BasicUsage) {
  // Create an Any from AnyData
  auto anyData = type::AnyData::toAny<type::i32_t>(42);
  Any any(anyData);

  EXPECT_TRUE(any.hasValue());
}

TEST(AnyTest, DefaultConstruction) {
  Any any;
  EXPECT_FALSE(any.hasValue());
}

TEST(AnyTest, MemoryResource) {
  std::pmr::monotonic_buffer_resource mbr;
  auto anyData = type::AnyData::toAny<type::i32_t>(42);
  Any any(anyData, &mbr);

  EXPECT_TRUE(any.hasValue());
}

TEST(AnyTest, Equality) {
  auto anyData1 = type::AnyData::toAny<type::i32_t>(42);
  auto anyData2 = type::AnyData::toAny<type::i32_t>(42);
  auto anyData3 = type::AnyData::toAny<type::i32_t>(99);

  Any any1(anyData1);
  Any any2(anyData2);
  Any any3(anyData3);

  EXPECT_EQ(any1, any2);
  EXPECT_NE(any1, any3);
}

TEST(AnyTest, CopyAndMove) {
  auto anyData = type::AnyData::toAny<type::i32_t>(42);
  Any any1(anyData);

  // Copy
  Any any2(any1);
  EXPECT_EQ(any1, any2);

  // Move
  Any any3(std::move(any1));
  EXPECT_EQ(any3, any2);
}

TEST(AnyTest, MakeAnyFactory) {
  // Test makeAny with AnyData
  auto anyData = type::AnyData::toAny<type::i32_t>(42);
  auto value = DynamicValue::makeAny(anyData);

  EXPECT_EQ(value.type().kind(), type_system::TypeRef::Kind::ANY);
  EXPECT_TRUE(value.asAny().hasValue());
  EXPECT_EQ(value.asAny().type(), type::Type::get<type::i32_t>());

  // Load and verify the contained value
  auto loaded = value.asAny().load(getTestTypeSystem());
  EXPECT_EQ(loaded.asI32(), 42);

  // Test makeAny with DynamicValue
  auto originalValue = DynamicValue::makeString("test string");
  auto anyValue = DynamicValue::makeAny(originalValue);

  EXPECT_EQ(anyValue.type().kind(), type_system::TypeRef::Kind::ANY);
  EXPECT_TRUE(anyValue.asAny().hasValue());
  EXPECT_EQ(anyValue.asAny().type(), type::Type::get<type::string_t>());

  // Load and verify
  auto loadedString = anyValue.asAny().load(getTestTypeSystem());
  EXPECT_EQ(loadedString.asString().view(), "test string");
}

TEST(AnyTest, LoadFromAny) {
  auto anyData = type::AnyData::toAny<type::i32_t>(42);
  Any any(anyData);

  // Load the value
  auto value = any.load(getTestTypeSystem());
  EXPECT_EQ(value.type().kind(), type_system::TypeRef::Kind::I32);
  EXPECT_EQ(value.asI32(), 42);
}

TEST(AnyTest, StoreAndLoad) {
  // Create a DynamicValue
  auto originalValue = DynamicValue::makeI32(12345);

  // Store it in an Any
  auto any = Any::store(originalValue);
  EXPECT_TRUE(any.hasValue());

  // Load it back
  auto loadedValue = any.load(getTestTypeSystem());
  EXPECT_EQ(loadedValue.type().kind(), type_system::TypeRef::Kind::I32);
  EXPECT_EQ(loadedValue.asI32(), 12345);
}

TEST(AnyTest, LoadWithTypeRef) {
  // Create an Any containing an i32
  auto originalValue = DynamicValue::makeI32(42);
  auto any = Any::store(originalValue);

  // Load using the TypeRef overload - useful when you know the type
  // and want to avoid needing a TypeSystem instance
  auto loadedValue = any.load(type_system::TypeSystem::I32());
  EXPECT_EQ(loadedValue.type().kind(), type_system::TypeRef::Kind::I32);
  EXPECT_EQ(loadedValue.asI32(), 42);

  // Also works with other types like strings
  auto stringValue = DynamicValue::makeString("hello");
  auto stringAny = Any::store(stringValue);
  auto loadedString = stringAny.load(type_system::TypeSystem::String());
  EXPECT_EQ(loadedString.type().kind(), type_system::TypeRef::Kind::STRING);
  EXPECT_EQ(loadedString.asString().view(), "hello");

  // Attempting to load with the wrong type should throw
  EXPECT_THROW(any.load(type_system::TypeSystem::String()), std::runtime_error);
  EXPECT_THROW(
      stringAny.load(type_system::TypeSystem::I32()), std::runtime_error);
}

TEST(AnyTest, StoreWithDifferentProtocols) {
  auto value = DynamicValue::makeBool(true);

  // Store with Binary protocol
  {
    auto any = Any::store(value, type::StandardProtocol::Binary);
    auto loaded = any.load(getTestTypeSystem());
    EXPECT_TRUE(loaded.asBool());
  }

  // Store with Compact protocol (default)
  {
    auto any = Any::store(value);
    auto loaded = any.load(getTestTypeSystem());
    EXPECT_TRUE(loaded.asBool());
  }
}

TEST(AnyTest, TypeAndProtocolAccessors) {
  // Test type() and protocol() accessors
  auto value = DynamicValue::makeI32(42);

  // Store with Binary protocol
  auto anyBinary = Any::store(value, type::StandardProtocol::Binary);
  EXPECT_EQ(anyBinary.type(), type::Type::get<type::i32_t>());
  EXPECT_EQ(
      anyBinary.protocol(),
      type::Protocol::get<type::StandardProtocol::Binary>());

  // Store with Compact protocol
  auto anyCompact = Any::store(value, type::StandardProtocol::Compact);
  EXPECT_EQ(anyCompact.type(), type::Type::get<type::i32_t>());
  EXPECT_EQ(
      anyCompact.protocol(),
      type::Protocol::get<type::StandardProtocol::Compact>());

  // Test with different type
  auto stringValue = DynamicValue::makeString("test");
  auto anyString = Any::store(stringValue);
  EXPECT_EQ(anyString.type(), type::Type::get<type::string_t>());
}

TEST(AnyTest, WrapDifferentTypes) {
  auto& ts = getTestTypeSystem();

  // Test with bool
  {
    auto anyData = type::AnyData::toAny<type::bool_t>(true);
    Any any(anyData);
    EXPECT_TRUE(any.hasValue());

    auto value = any.load(ts);
    EXPECT_TRUE(value.asBool());
  }

  // Test with string
  {
    std::string str = "hello world";
    auto anyData = type::AnyData::toAny<type::string_t>(str);
    Any any(anyData);
    EXPECT_TRUE(any.hasValue());

    auto value = any.load(ts);
    EXPECT_EQ(value.asString().view(), "hello world");
  }

  // Test with double
  {
    auto anyData = type::AnyData::toAny<type::double_t>(3.14159);
    Any any(anyData);
    EXPECT_TRUE(any.hasValue());

    auto value = any.load(ts);
    EXPECT_DOUBLE_EQ(value.asDouble(), 3.14159);
  }
}

TEST(AnyTest, SerializationRoundTrip) {
  auto& ts = getTestTypeSystem();

  // Create an Any wrapping an i32 value
  auto anyData = type::AnyData::toAny<type::i32_t>(12345);
  auto value = DynamicValue::makeAny(anyData);

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(
      reader, type_system::TypeRef(type_system::TypeRef::Any()));

  // Verify the Any was deserialized correctly
  EXPECT_EQ(value, deserValue);
  const Any& deserAny = deserValue.asAny();
  EXPECT_TRUE(deserAny.hasValue());

  // Extract and verify the original value
  auto extractedValue = deserAny.load(ts);
  EXPECT_EQ(extractedValue.type().kind(), type_system::TypeRef::Kind::I32);
  EXPECT_EQ(extractedValue.asI32(), 12345);
}

TEST(AnyTest, SerializationRoundTripString) {
  auto& ts = getTestTypeSystem();

  // Create an Any wrapping a string value
  std::string testString = "test string value";
  auto anyData = type::AnyData::toAny<type::string_t>(testString);
  auto value = DynamicValue::makeAny(anyData);

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(
      reader, type_system::TypeRef(type_system::TypeRef::Any()));

  // Verify
  EXPECT_EQ(value, deserValue);
  const Any& deserAny = deserValue.asAny();
  EXPECT_TRUE(deserAny.hasValue());

  // Extract and verify the original value
  auto extractedValue = deserAny.load(ts);
  EXPECT_EQ(extractedValue.type().kind(), type_system::TypeRef::Kind::STRING);
  EXPECT_EQ(extractedValue.asString().view(), testString);
}

TEST(AnyTest, SerializationRoundTripBool) {
  auto& ts = getTestTypeSystem();

  // Create an Any wrapping a bool value
  auto anyData = type::AnyData::toAny<type::bool_t>(true);
  auto value = DynamicValue::makeAny(anyData);

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(
      reader, type_system::TypeRef(type_system::TypeRef::Any()));

  // Verify
  EXPECT_EQ(value, deserValue);
  const Any& deserAny = deserValue.asAny();
  EXPECT_TRUE(deserAny.hasValue());

  // Extract and verify the original value
  auto extractedValue = deserAny.load(ts);
  EXPECT_EQ(extractedValue.type().kind(), type_system::TypeRef::Kind::BOOL);
  EXPECT_TRUE(extractedValue.asBool());
}

TEST(AnyTest, StructWithAnyField) {
  auto& ts = getTestTypeSystem();
  auto& structNode =
      ts.getUserDefinedTypeOrThrow("facebook.com/thrift/test/StructWithAny")
          .asStruct();

  // Create a DynamicValue with the struct
  auto value = DynamicValue::makeDefault(type_system::TypeRef(structNode));
  auto& structValue = value.asStruct();

  // Set basic fields
  structValue.setField(type_system::FieldId{1}, DynamicValue::makeI32(123));
  structValue.setField(
      type_system::FieldId{2}, DynamicValue::makeString("test struct"));

  // Create an Any value and store it in the metadata field
  structValue.setField(
      type_system::FieldId{3},
      DynamicValue::makeAny(DynamicValue::makeDouble(3.14159)));

  // Verify the Any field was set correctly
  auto metadata = structValue.getField(type_system::FieldId{3});
  ASSERT_TRUE(metadata.has_value());
  EXPECT_TRUE(metadata->asAny().hasValue());
  EXPECT_EQ(metadata->asAny().type(), type::Type::get<type::double_t>());

  // Serialize the struct
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize the struct
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(reader, type_system::TypeRef(structNode));

  // Verify the basic fields
  auto& deserStruct = deserValue.asStruct();
  auto id = deserStruct.getField(type_system::FieldId{1});
  ASSERT_TRUE(id.has_value());
  EXPECT_EQ(id->asI32(), 123);

  auto name = deserStruct.getField(type_system::FieldId{2});
  ASSERT_TRUE(name.has_value());
  EXPECT_EQ(name->asString().view(), "test struct");

  // Verify the Any field
  auto deserMetadata = deserStruct.getField(type_system::FieldId{3});
  ASSERT_TRUE(deserMetadata.has_value());
  EXPECT_TRUE(deserMetadata->asAny().hasValue());
  EXPECT_EQ(deserMetadata->asAny().type(), type::Type::get<type::double_t>());

  // Load the value from the Any field
  auto loadedValue = deserMetadata->asAny().load(ts);
  EXPECT_EQ(loadedValue.type().kind(), type_system::TypeRef::Kind::DOUBLE);
  EXPECT_DOUBLE_EQ(loadedValue.asDouble(), 3.14159);
}

} // namespace apache::thrift::dynamic
