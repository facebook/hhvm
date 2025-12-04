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

#include <thrift/lib/cpp2/protocol/detail/DynamicCursorSerializer.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/cursor_types.h>
#include <thrift/lib/cpp2/reflection/testing.h>
#include <thrift/lib/cpp2/type/AnyDebugWriter.h>

using namespace apache::thrift;
using namespace apache::thrift::detail;
using namespace apache::thrift::test;
using namespace ::testing;

Struct createStruct() {
  Struct s;
  s.string_field() = "hello";
  s.i32_field() = 42;
  s.union_field()->binary_field() = "world";
  s.list_field() = {'f', 'o', 'o'};
  Stringish stringish;
  stringish.binary_field() =
      folly::IOBuf::wrapBufferAsValue(folly::Range("bar"));
  s.set_nested_field() = {{stringish}};
  s.map_field() = {{'a', 1}, {'b', 2}};
  s.bool_field() = true;
  return s;
}

std::string debugPrint(const folly::IOBuf& s) {
  type::AnyStruct any;
  any.type() = type::Type::get<type::struct_t<Struct>>();
  any.protocol() = type::StandardProtocol::Binary;
  any.data() = s;
  return anyDebugString(any);
}

std::unique_ptr<type_system::TypeSystem> buildTestTypeSystem() {
  using namespace apache::thrift::type_system;
  using def = TypeSystemBuilder::DefinitionHelper;

  TypeSystemBuilder builder;

  // Define Inner union type
  builder.addType(
      "facebook.com/thrift/test/Inner",
      def::Union({
          def::Field(
              def::Identity(1, "binary_field"), def::Optional, TypeIds::Binary),
      }));

  // Define Stringish struct type
  builder.addType(
      "facebook.com/thrift/test/Stringish",
      def::Struct({
          def::Field(
              def::Identity(2, "binary_field"), def::Optional, TypeIds::Binary),
      }));

  // Define main Struct type with various field types
  builder.addType(
      "facebook.com/thrift/test/Struct",
      def::Struct({
          def::Field(
              def::Identity(1, "string_field"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(2, "i32_field"), def::Optional, TypeIds::I32),
          def::Field(
              def::Identity(3, "union_field"),
              def::Optional,
              TypeIds::uri("facebook.com/thrift/test/Inner")),
          def::Field(
              def::Identity(4, "list_field"),
              def::Optional,
              TypeIds::list(TypeIds::Byte)),
          def::Field(
              def::Identity(5, "set_nested_field"),
              def::Optional,
              TypeIds::list(
                  TypeIds::set(
                      TypeIds::uri("facebook.com/thrift/test/Stringish")))),
          def::Field(
              def::Identity(6, "map_field"),
              def::Optional,
              TypeIds::map(TypeIds::Byte, TypeIds::Byte)),
          def::Field(
              def::Identity(7, "bool_field"), def::Optional, TypeIds::Bool),
      }));

  return std::move(builder).build();
}

TEST(DynamicCursorSerializer, UnschematizedRead) {
  DynamicCursorSerializationWrapper<BinaryProtocolReader, CompactProtocolWriter>
      wrapper(
          BinarySerializer::serialize<folly::IOBufQueue>(createStruct())
              .move());

  // Native type reads
  auto reader = wrapper.beginRead();
  EXPECT_EQ(reader.read(type::binary_t{}), "hello");
  EXPECT_EQ(reader.read(type::i32_t{}), 42);
  EXPECT_EQ(reader.read(type::union_t<Inner>{}).binary_field(), "world");
  EXPECT_THAT(
      reader.read(type::list<type::byte_t>{}), ElementsAre('f', 'o', 'o'));
  reader.skip(); // set_nested_field
  EXPECT_THAT(
      reader.read(type::map<type::byte_t, type::byte_t>{}),
      UnorderedElementsAre(Pair('a', 1), Pair('b', 2)));
  EXPECT_TRUE(reader.read(type::bool_t{}));
  wrapper.endRead(std::move(reader));

  // Protocol Value reads
  reader = wrapper.beginRead();
  auto value = reader.readValue();
  EXPECT_EQ(value.as_binary().moveToFbString(), "hello");
  value = reader.readValue();
  EXPECT_EQ(value.as_i32(), 42);
  value = reader.readValue();
  EXPECT_EQ(
      value.as_object().at(FieldId{1}).as_binary().moveToFbString(), "world");
  reader.skip(); // list_field
  reader.skip(); // set_nested_field
  reader.skip(); // map_field
  value = reader.readValue();
  EXPECT_TRUE(value.as_bool());
  wrapper.endRead(std::move(reader));

  // Cursor reads
  reader = wrapper.beginRead();
  reader.skip(); // string_field
  reader.skip(); // i32_field
  auto unionReader = reader.beginReadStructured();
  EXPECT_EQ(unionReader.fieldId(), 1);
  EXPECT_EQ(unionReader.fieldType(), protocol::TType::T_STRING);
  EXPECT_EQ(unionReader.read(type::string_t{}), "world");
  reader.endRead(std::move(unionReader));

  auto listOfByteReader = reader.beginReadContainer();
  EXPECT_EQ(listOfByteReader.remaining(), 3);
  std::array<signed char, 5> buf{};
  EXPECT_THAT(
      listOfByteReader.readChunk(type::byte_t{}, buf),
      ElementsAre('f', 'o', 'o'));
  EXPECT_EQ(listOfByteReader.remaining(), 0);
  reader.endRead(std::move(listOfByteReader));

  auto list_of_set_of_stringish_reader = reader.beginReadContainer();
  auto set_of_stringish_reader =
      list_of_set_of_stringish_reader.beginReadContainer();
  auto stringish_reader = set_of_stringish_reader.beginReadStructured();
  EXPECT_EQ(stringish_reader.fieldId(), 2);
  EXPECT_EQ(stringish_reader.fieldType(), protocol::TType::T_STRING);
  EXPECT_EQ(stringish_reader.read(type::binary_t{}), "bar");
  set_of_stringish_reader.endRead(std::move(stringish_reader));
  list_of_set_of_stringish_reader.endRead(std::move(set_of_stringish_reader));
  reader.endRead(std::move(list_of_set_of_stringish_reader));
  wrapper.endRead(std::move(reader));

  // Raw read
  reader = wrapper.beginRead();
  auto str = reader.readRaw();
  auto i32 = reader.readRawCursor();
  auto inner = reader.readRawCursor();
  reader.skip(); // list_field
  reader.skip(); // set_nested_field;
  reader.skip(); // map_field;
  auto boolf = reader.readRaw();
  wrapper.endRead(std::move(reader));
  auto read = [&]<typename Tag>(Tag, auto buf) {
    type::native_type<Tag> ret;
    BinaryProtocolReader rawReader;
    rawReader.setInput(buf);
    op::decode<Tag>(rawReader, ret);
    return ret;
  };
  EXPECT_EQ(read(type::string_t{}, str.get()), "hello");
  EXPECT_EQ(read(type::i32_t{}, i32), 42);
  EXPECT_EQ(read(type::struct_t<Inner>{}, inner).binary_field(), "world");
  EXPECT_TRUE(read(type::bool_t{}, boolf.get()));
}

TEST(DynamicCursorSerializer, UnschematizedWrite) {
  DynamicCursorSerializationWrapper<BinaryProtocolReader, BinaryProtocolWriter>
      wrapper;

  Stringish stringish;
  stringish.binary_field() =
      folly::IOBuf::wrapBufferAsValue(folly::Range("bar"));

  // Native type writes
  auto writer = wrapper.beginWrite();
  writer.write(1, type::string_t{}, "hello");
  writer.write(2, type::i32_t{}, 42);
  Inner inner;
  inner.binary_field() = "world";
  writer.write(3, type::struct_t<Inner>{}, inner);
  writer.write(4, type::list<type::byte_t>{}, {'f', 'o', 'o'});
  writer.write(
      5, type::list<type::set<type::union_t<Stringish>>>{}, {{stringish}});
  writer.write(
      6, type::map<type::byte_t, type::byte_t>{}, {{'a', 1}, {'b', 2}});
  writer.write(7, type::bool_t{}, true);
  wrapper.endWrite(std::move(writer));
  EXPECT_THRIFT_EQ(wrapper.deserialize<Struct>(), createStruct())
      << debugPrint(wrapper.serializedData());

  // Protocol Value writes
  writer = wrapper.beginWrite();
  writer.writeValue(1, protocol::asValueStruct<type::string_t>("hello"));
  writer.writeValue(2, protocol::asValueStruct<type::i32_t>(42));
  writer.writeValue(3, protocol::asValueStruct<type::struct_t<Inner>>(inner));
  writer.writeValue(
      4, protocol::asValueStruct<type::list<type::byte_t>>({'f', 'o', 'o'}));
  writer.writeValue(
      5,
      protocol::asValueStruct<type::list<type::set<type::union_t<Stringish>>>>(
          {{stringish}}));
  writer.writeValue(
      6,
      protocol::asValueStruct<type::map<typename type::byte_t, type::byte_t>>(
          {{'a', 1}, {'b', 2}}));
  writer.writeValue(7, protocol::asValueStruct<type::bool_t>(true));
  wrapper.endWrite(std::move(writer));
  EXPECT_THRIFT_EQ(wrapper.deserialize<Struct>(), createStruct())
      << debugPrint(wrapper.serializedData());

  // Cursor writes
  writer = wrapper.beginWrite();
  writer.write(1, type::string_t{}, "hello");
  writer.write(2, type::i32_t{}, 42);

  auto structWriter = writer.beginWriteStructured(3);
  structWriter.write(1, type::string_t{}, "world");
  writer.endWrite(std::move(structWriter));

  auto listOfByteWriter =
      writer.beginWriteContainer(4, type::list<type::byte_t>{}, 3);
  listOfByteWriter.write(type::byte_t{}, 'f');
  signed char o = 'o';
  listOfByteWriter.writeChunk(type::byte_t{}, std::array{o, o});
  writer.endWrite(std::move(listOfByteWriter));

  auto list_of_set_of_stringish_writer = writer.beginWriteContainer(
      5, type::list<type::set<type::union_t<Stringish>>>{}, 1);
  auto set_of_stringish_writer =
      list_of_set_of_stringish_writer.beginWriteContainer(
          type::set<type::union_t<Stringish>>{}, 1);
  auto stringish_writer = set_of_stringish_writer.beginWriteStructured();
  stringish_writer.write(2, type::string_t{}, "bar");
  set_of_stringish_writer.endWrite(std::move(stringish_writer));
  list_of_set_of_stringish_writer.endWrite(std::move(set_of_stringish_writer));
  writer.endWrite(std::move(list_of_set_of_stringish_writer));

  auto mapWriter =
      writer.beginWriteContainer(6, type::map<type::byte_t, type::byte_t>{}, 2);
  mapWriter.write(type::byte_t{}, 'a');
  mapWriter.write(type::byte_t{}, 1);
  mapWriter.write(type::byte_t{}, 'b');
  mapWriter.write(type::byte_t{}, 2);
  writer.endWrite(std::move(mapWriter));

  writer.write(7, type::bool_t{}, true);
  wrapper.endWrite(std::move(writer));
  EXPECT_THRIFT_EQ(wrapper.deserialize<Struct>(), createStruct())
      << debugPrint(wrapper.serializedData());

  // Raw write
  writer = wrapper.beginWrite();
  auto serialize = [&]<typename T>(const T& t) {
    folly::IOBufQueue queue;
    BinaryProtocolWriter rawWriter;
    rawWriter.setOutput(&queue);
    op::encode<type::infer_tag<T, true>>(rawWriter, t);
    return queue.move();
  };
  writer.writeRaw(1, type::string_t{}, *serialize(std::string("hello")));
  writer.writeRaw(2, type::i32_t{}, *serialize(int32_t{42}));
  Inner inner2;
  inner2.binary_field() = "world";
  writer.writeRaw(3, type::struct_t<Inner>{}, *serialize(inner2));
  listOfByteWriter =
      writer.beginWriteContainer(4, type::list<type::byte_t>{}, 3);
  listOfByteWriter.write(type::byte_t{}, 'f');
  listOfByteWriter.writeRaw(*serialize(o));
  listOfByteWriter.writeRaw(
      folly::io::Cursor(
          &*serialize(o), serialize(o)->computeChainDataLength()));
  writer.endWrite(std::move(listOfByteWriter));
  writer.write(
      5, type::list<type::set<type::union_t<Stringish>>>{}, {{stringish}});
  writer.write(
      6, type::map<type::byte_t, type::byte_t>{}, {{'a', 1}, {'b', 2}});
  writer.writeRaw(7, type::bool_t{}, *serialize(true));
  wrapper.endWrite(std::move(writer));
  EXPECT_THRIFT_EQ(wrapper.deserialize<Struct>(), createStruct())
      << debugPrint(wrapper.serializedData());
}

TEST(DynamicCursorSerializer, WithTypeRef) {
  auto typeSystem = buildTestTypeSystem();
  auto structTypeRef =
      typeSystem->getUserDefinedTypeOrThrow("facebook.com/thrift/test/Struct")
          .asStruct()
          .asRef();

  // Test reading with TypeRef
  DynamicCursorSerializationWrapper<BinaryProtocolReader, BinaryProtocolWriter>
      wrapperWithType(
          BinarySerializer::serialize<folly::IOBufQueue>(createStruct()).move(),
          structTypeRef);

  auto reader = wrapperWithType.beginRead();

  // Verify field type information is available
  auto stringFieldTypeRef = reader.fieldTypeRef();
  ASSERT_TRUE(stringFieldTypeRef.has_value());
  EXPECT_TRUE(stringFieldTypeRef->isString());
  EXPECT_EQ(reader.read(type::string_t{}), "hello");

  auto i32FieldTypeRef = reader.fieldTypeRef();
  ASSERT_TRUE(i32FieldTypeRef.has_value());
  EXPECT_TRUE(i32FieldTypeRef->isI32());
  EXPECT_EQ(reader.read(type::i32_t{}), 42);

  // Test nested structured read with TypeRef propagation
  auto unionFieldTypeRef = reader.fieldTypeRef();
  ASSERT_TRUE(unionFieldTypeRef.has_value());
  EXPECT_TRUE(unionFieldTypeRef->isUnion());
  auto unionReader = reader.beginReadStructured();
  EXPECT_EQ(unionReader.fieldId(), 1);
  auto innerFieldTypeRef = unionReader.fieldTypeRef();
  ASSERT_TRUE(innerFieldTypeRef.has_value());
  EXPECT_TRUE(innerFieldTypeRef->isBinary());
  EXPECT_EQ(unionReader.read(type::binary_t{}), "world");
  reader.endRead(std::move(unionReader));

  // Test container read with TypeRef propagation
  auto listFieldTypeRef = reader.fieldTypeRef();
  ASSERT_TRUE(listFieldTypeRef.has_value());
  EXPECT_TRUE(listFieldTypeRef->isList());
  auto listReader = reader.beginReadContainer();
  auto elementTypeRef = listReader.nextTypeRef();
  ASSERT_TRUE(elementTypeRef.has_value());
  EXPECT_TRUE(elementTypeRef->isByte());
  EXPECT_EQ(listReader.read(type::byte_t{}), 'f');
  EXPECT_EQ(listReader.read(type::byte_t{}), 'o');
  EXPECT_EQ(listReader.read(type::byte_t{}), 'o');
  reader.endRead(std::move(listReader));

  // Test nested container with TypeRef propagation
  auto setNestedFieldTypeRef = reader.fieldTypeRef();
  ASSERT_TRUE(setNestedFieldTypeRef.has_value());
  EXPECT_TRUE(setNestedFieldTypeRef->isList());
  auto outerListReader = reader.beginReadContainer();
  auto setTypeRef = outerListReader.nextTypeRef();
  ASSERT_TRUE(setTypeRef.has_value());
  EXPECT_TRUE(setTypeRef->isSet());
  auto setReader = outerListReader.beginReadContainer();
  auto stringishTypeRef = setReader.nextTypeRef();
  ASSERT_TRUE(stringishTypeRef.has_value());
  EXPECT_TRUE(stringishTypeRef->isStruct());
  auto stringishReader = setReader.beginReadStructured();
  EXPECT_EQ(stringishReader.fieldId(), 2);
  auto binaryFieldTypeRef = stringishReader.fieldTypeRef();
  ASSERT_TRUE(binaryFieldTypeRef.has_value());
  EXPECT_TRUE(binaryFieldTypeRef->isBinary());
  EXPECT_EQ(stringishReader.read(type::binary_t{}), "bar");
  setReader.endRead(std::move(stringishReader));
  outerListReader.endRead(std::move(setReader));
  reader.endRead(std::move(outerListReader));

  wrapperWithType.endRead(std::move(reader));

  // Test writing with TypeRef
  DynamicCursorSerializationWrapper<BinaryProtocolReader, BinaryProtocolWriter>
      writeWrapper(structTypeRef);

  auto writer = writeWrapper.beginWrite();

  // Verify field type information during write
  auto field1TypeRef = writer.getFieldTypeRef(1);
  ASSERT_TRUE(field1TypeRef.has_value());
  EXPECT_TRUE(field1TypeRef->isString());
  writer.write(1, type::string_t{}, "hello");

  auto field2TypeRef = writer.getFieldTypeRef(2);
  ASSERT_TRUE(field2TypeRef.has_value());
  EXPECT_TRUE(field2TypeRef->isI32());
  writer.write(2, type::i32_t{}, 42);

  auto field3TypeRef = writer.getFieldTypeRef(3);
  ASSERT_TRUE(field3TypeRef.has_value());
  EXPECT_TRUE(field3TypeRef->isUnion());
  auto innerWriter = writer.beginWriteStructured(3);
  innerWriter.write(1, type::string_t{}, "world");
  writer.endWrite(std::move(innerWriter));

  auto listWriter =
      writer.beginWriteContainer(4, type::list<type::byte_t>{}, 3);
  listWriter.write(type::byte_t{}, 'f');
  listWriter.write(type::byte_t{}, 'o');
  listWriter.write(type::byte_t{}, 'o');
  writer.endWrite(std::move(listWriter));

  Stringish stringish;
  stringish.binary_field() =
      folly::IOBuf::wrapBufferAsValue(folly::Range("bar"));
  writer.write(
      5, type::list<type::set<type::union_t<Stringish>>>{}, {{stringish}});
  writer.write(
      6, type::map<type::byte_t, type::byte_t>{}, {{'a', 1}, {'b', 2}});
  writer.write(7, type::bool_t{}, true);

  writeWrapper.endWrite(std::move(writer));
  EXPECT_THRIFT_EQ(writeWrapper.deserialize<Struct>(), createStruct());
}

TEST(DynamicCursorSerializer, TypeRefValidation) {
  using namespace apache::thrift::type_system;
  using def = TypeSystemBuilder::DefinitionHelper;

  // Build a TypeSystem with incompatible types for testing validation
  TypeSystemBuilder builder;
  builder.addType(
      "facebook.com/thrift/test/TestStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "string_field"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(2, "list_field"),
              def::Optional,
              TypeIds::list(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();
  auto structTypeRef =
      typeSystem
          ->getUserDefinedTypeOrThrow("facebook.com/thrift/test/TestStruct")
          .asStruct()
          .asRef();

  Struct s = createStruct();
  DynamicCursorSerializationWrapper<BinaryProtocolReader, BinaryProtocolWriter>
      wrapper(
          BinarySerializer::serialize<folly::IOBufQueue>(s).move(),
          structTypeRef);

  auto reader = wrapper.beginRead();

  // Skip string field (field 1)
  reader.skip();

  // Try to read i32_field (field 2) as structured, but TypeRef says field 2 is
  // a list
  try {
    reader.beginReadStructured();
    ADD_FAILURE() << "Expected exception";
  } catch (const std::runtime_error&) {
  }

  // After the exception, abandon the reader to avoid destructor check failure
  wrapper.abandonRead(std::move(reader));
}
