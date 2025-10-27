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
  listOfByteWriter.writeRaw(*serialize(o));
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
