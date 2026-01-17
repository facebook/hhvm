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

#include <thrift/lib/cpp2/protocol/CursorBasedSerializer.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/cursor_clients.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/cursor_handlers.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/cursor_types.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/cpp2/util/gtest/Matcher.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace ::testing;

namespace {
struct Handler : ServiceHandler<Example> {
  void sync_identity(
      EmptyWrapper& ret, std::unique_ptr<EmptyWrapper> empty) override {
    ret = EmptyWrapper(empty->deserialize());
  }
};
} // namespace

TEST(CursorSerializerTest, RpcExample) {
  auto handler = std::make_shared<Handler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  EmptyWrapper empty(Empty{});
  EmptyWrapper ret;
  client->sync_identity(ret, empty);
  std::ignore = ret.deserialize();

  client =
      makeTestClient(handler, nullptr, nullptr, protocol::T_COMPACT_PROTOCOL);
  EXPECT_THAT(
      [&] { client->sync_identity(ret, empty); },
      ThrowsMessage<std::runtime_error>(
          "Single pass serialization only supports binary protocol."));
}

TEST(CursorSerializer, QualifierRead) {
  Qualifiers obj;
  obj.opt() = 3;

  // Reading from a serialized default-constructed object sees the written
  // (default) values).
  CursorSerializationWrapper<Qualifiers> wrapper(obj);
  auto reader = wrapper.beginRead();
  EXPECT_EQ(reader.read<ident::opt>(), 3);
  EXPECT_EQ(reader.read<ident::unq>(), 1);
  EXPECT_EQ(reader.read<ident::terse>(), 0);
  wrapper.endRead(std::move(reader));

  // Reading from a serialized empty object applies the appropriate default
  // based on the qualifier.
  folly::IOBufQueue q;
  BinarySerializer::serialize(Empty{}, &q);
  wrapper = CursorSerializationWrapper<Qualifiers>(q.move());
  reader = wrapper.beginRead();
  EXPECT_FALSE(reader.read<ident::opt>());
  EXPECT_EQ(reader.read<ident::unq>(), 1);
  EXPECT_EQ(reader.read<ident::terse>(), 0);
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, ReadWithSkip) {
  CursorSerializationWrapper<Meal> wrapper(Meal{});
  auto reader = wrapper.beginRead();
  EXPECT_EQ(reader.read<ident::appetizer>(), 1);
  EXPECT_EQ(reader.read<ident::main>(), 2);
  EXPECT_EQ(reader.read<ident::dessert>(), 3);
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, UnionRead) {
  Stringish string;

  CursorSerializationWrapper<Stringish> wrapper(string);
  auto reader = wrapper.beginRead();
  EXPECT_EQ(reader.readType(), Stringish::Type::__EMPTY__);
  wrapper.endRead(std::move(reader));

  string.string_field() = "foo";
  wrapper = CursorSerializationWrapper<Stringish>(string);
  reader = wrapper.beginRead();
  EXPECT_EQ(reader.readType(), Stringish::Type::string_field);
  EXPECT_EQ(reader.read<ident::string_field>(), "foo");
  EXPECT_FALSE(reader.read<ident::binary_field>());
  wrapper.endRead(std::move(reader));

  string.binary_field() = folly::IOBuf::wrapBufferAsValue(
      folly::ByteRange(std::string_view("bar")));
  wrapper = CursorSerializationWrapper<Stringish>(string);
  reader = wrapper.beginRead();
  if (auto str = reader.read<ident::string_field>()) {
    ADD_FAILURE();
  } else if (auto buf = reader.read<ident::binary_field>()) {
    EXPECT_EQ(buf->moveToFbString(), "bar");
  } else {
    ADD_FAILURE();
  }
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, ManagedStringViewRead) {
  StructWithCppType obj;
  obj.someId() = 15u;
  obj.someName() = "baz";

  CursorSerializationWrapper<StructWithCppType> wrapper(obj);
  auto reader = wrapper.beginRead();
  EXPECT_EQ(reader.read<ident::someId>(), 15u);
  EXPECT_EQ(reader.read<ident::someName>().str(), "baz");
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, StructWithOptionalRead) {
  {
    StructWithOptional obj;
    obj.optional_string() = "foo";

    CursorSerializationWrapper<StructWithOptional> wrapper(obj);
    auto reader = wrapper.beginRead();
    std::string str;

    EXPECT_TRUE(reader.read<ident::optional_string>(str));
    EXPECT_EQ(str, "foo");

    auto listReader = reader.beginRead<ident::optional_list>();
    EXPECT_FALSE(listReader.has_value());

    auto map = reader.read<ident::optional_map>();
    EXPECT_FALSE(map.has_value());

    auto innieReader = reader.beginRead<ident::optional_containers>();
    EXPECT_FALSE(innieReader.has_value());
    wrapper.endRead(std::move(reader));
  }

  {
    StructWithOptional obj;
    obj.optional_string() = "foo";
    obj.optional_list() = {1, 2, 3};
    obj.optional_map() = {{1, 2}, {3, 4}};
    Containers containers;
    containers.list_of_string() = {"foo", "bar"};
    obj.optional_containers() = containers;

    CursorSerializationWrapper<StructWithOptional> wrapper(obj);
    auto reader = wrapper.beginRead();
    std::string str;

    EXPECT_TRUE(reader.read<ident::optional_string>(str));
    EXPECT_EQ(str, "foo");

    auto listReader = reader.beginRead<ident::optional_list>();
    EXPECT_TRUE(listReader.has_value());
    EXPECT_EQ(listReader->size(), 3);
    std::vector<int32_t> numbers(listReader->begin(), listReader->end());
    EXPECT_THAT(numbers, ElementsAreArray({1, 2, 3}));
    reader.endRead(std::move(listReader.value()));

    auto map = reader.read<ident::optional_map>();
    EXPECT_TRUE(map.has_value());
    EXPECT_EQ(map->size(), 2);
    EXPECT_EQ(map->at(1), 2);
    EXPECT_EQ(map->at(3), 4);

    auto innerReader = reader.beginRead<ident::optional_containers>();
    EXPECT_TRUE(innerReader.has_value());
    auto string_list = innerReader->read<ident::list_of_string>();
    EXPECT_EQ(string_list.size(), 2);
    EXPECT_TRUE(string_list[0] == "foo" && string_list[1] == "bar");
    reader.endRead(std::move(innerReader.value()));
    wrapper.endRead(std::move(reader));
  }
}

TEST(CursorSerializer, NumericRead) {
  Numerics obj;
  obj.int16() = 1;
  obj.uint32() = 2;
  obj.enm() = E::A;
  obj.flt() = 3;

  CursorSerializationWrapper wrapper(obj);

  auto reader = wrapper.beginRead();
  int16_t i;
  EXPECT_TRUE(reader.read<ident::int16>(i));
  EXPECT_EQ(i, *obj.int16());
  uint32_t u;
  reader.read<ident::uint32>(u);
  EXPECT_EQ(u, *obj.uint32());
  E e;
  reader.read<ident::enm>(e);
  EXPECT_EQ(e, *obj.enm());
  float f;
  reader.read<ident::flt>(f);
  EXPECT_FLOAT_EQ(f, *obj.flt());
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, StringRead) {
  Struct obj;
  obj.string_field() = "foo";

  StructCursor wrapper(obj);
  auto contiguousReader = wrapper.beginRead</* Contiguous = */ true>();
  std::string_view str;
  EXPECT_TRUE(contiguousReader.read<ident::string_field>(str));
  EXPECT_EQ("foo", str);
  wrapper.endRead(std::move(contiguousReader));

  auto reader = wrapper.beginRead();
  std::string str2;
  EXPECT_TRUE(reader.read<ident::string_field>(str2));
  EXPECT_EQ("foo", str2);
  wrapper.endRead(std::move(reader));

  reader = wrapper.beginRead();
  folly::IOBuf str3;
  EXPECT_TRUE(reader.read<ident::string_field>(str3));
  EXPECT_EQ("foo", str3.moveToFbString());
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, ContainerRead) {
  CursorSerializationWrapper<Cookie> wrapper(Cookie{});
  auto reader = wrapper.beginRead();
  auto listReader = reader.beginRead<ident::lucky_numbers>();
  EXPECT_EQ(listReader.size(), 3);
  std::vector<int32_t> numbers(listReader.begin(), listReader.end());
  EXPECT_THAT(numbers, ElementsAreArray({508, 493, 425}));

  // Can't use parent reader while child reader is active.
  EXPECT_THAT(
      [&] { reader.read<ident::flavor>(); },
      ThrowsMessage<std::runtime_error>("Child reader not passed to endRead"));
  reader.endRead(std::move(listReader));
  wrapper.endRead(std::move(reader));

  auto contiguousReader = wrapper.beginRead</* Contiguous = */ true>();
  auto contiguousListReader =
      contiguousReader.beginRead<ident::lucky_numbers>();
  for (auto i : contiguousListReader) {
    EXPECT_GT(i, 500);
    break;
  }
  // Ending read in the middle of iteration still allows reading next field.
  contiguousReader.endRead(std::move(contiguousListReader));
  EXPECT_EQ(contiguousReader.read<ident::flavor>(), "Sugar");
  static_assert( //
      std::is_same_v<
          decltype(contiguousReader.read<ident::flavor>()),
          std::string_view>);
  wrapper.endRead(std::move(contiguousReader));

  // Reading from a finalized reader is not allowed (besides the obvious
  // use-after-move).
  EXPECT_THAT(
      // @lint-ignore CLANGTIDY bugprone-use-after-move
      [&] { listReader.begin(); },
      ThrowsMessage<std::runtime_error>("Reader already finalized"));

  // 0 and 1-element containers exercise separate edge cases
  Cookie c;
  c.lucky_numbers() = {};
  wrapper = CursorSerializationWrapper<Cookie>(c);
  reader = wrapper.beginRead();
  listReader = reader.beginRead<ident::lucky_numbers>();
  EXPECT_EQ(listReader.size(), 0);
  for (auto i : listReader) {
    ADD_FAILURE() << i;
  }
  reader.endRead(std::move(listReader));
  wrapper.endRead(std::move(reader));

  c.lucky_numbers() = {1};
  wrapper = CursorSerializationWrapper<Cookie>(c);
  reader = wrapper.beginRead();
  listReader = reader.beginRead<ident::lucky_numbers>();
  EXPECT_EQ(listReader.size(), 1);
  for (auto i : listReader) {
    EXPECT_EQ(i, 1);
  }
  reader.endRead(std::move(listReader));
  wrapper.endRead(std::move(reader));

  // Contiguous buffer means string turns into std::string_view
  Containers containers;
  containers.list_of_string() = {"foo", "bar"};
  CursorSerializationWrapper containersWrapper(containers);
  auto containersReader =
      containersWrapper.beginRead</* Contiguous = */ true>();
  auto listOfStringReader = containersReader.beginRead<ident::list_of_string>();
  for (std::string_view& str : listOfStringReader) {
    EXPECT_TRUE(str == "foo" || str == "bar");
  }
  containersReader.endRead(std::move(listOfStringReader));
  containersWrapper.endRead(std::move(containersReader));
}

TEST(CursorSerializer, NestedStructRead) {
  CursorSerializationWrapper<Meal> wrapper(Meal{});
  auto outerReader = wrapper.beginRead();
  auto innerReader = outerReader.beginRead<ident::cookie>();

  // Can't use parent reader while child reader is active.
  EXPECT_THAT(
      [&] { outerReader.read<ident::dessert>(); },
      ThrowsMessage<std::runtime_error>("Child reader not passed to endRead"));

  // endRead in the middle of reading the child skips to the end.
  EXPECT_EQ(innerReader.read<ident::id>(), 2);
  outerReader.endRead(std::move(innerReader));
  EXPECT_EQ(outerReader.read<ident::dessert>(), 3);
  wrapper.endRead(std::move(outerReader));
}

TEST(CursorSerializer, AbandonNestedStructRead) {
  CursorSerializationWrapper<Meal> wrapper(Meal{});
  auto outerReader = wrapper.beginRead();
  auto innerReader = outerReader.beginRead<ident::cookie>();

  outerReader.abandonRead(std::move(innerReader));

  // Can't use parent reader after abandoning child.
  EXPECT_THAT(
      [&] { outerReader.read<ident::dessert>(); },
      ThrowsMessage<std::runtime_error>("Reader abandoned"));

  // Can't call endRead after abandoning child.
  EXPECT_THAT(
      [&] { wrapper.endRead(std::move(outerReader)); },
      ThrowsMessage<std::runtime_error>("Reader abandoned"));

  // @lint-ignore CLANGTIDY bugprone-use-after-move
  wrapper.abandonRead(std::move(outerReader));
}

TEST(CursorSerializer, CursorReadInContainer) {
  Struct s;
  Stringish inner;
  inner.string_field() = "foo";
  s.set_nested_field() = std::vector{std::set{inner}};
  inner.string_field() = "bar";
  s.set_nested_field()[0].insert(inner);

  StructCursor wrapper(s);
  auto reader = wrapper.beginRead();
  auto listReader = reader.beginRead<ident::set_nested_field>();
  auto setReader = listReader.beginRead();
  auto innerReader = setReader.beginRead();
  EXPECT_EQ(innerReader.read<ident::string_field>(), "bar");
  setReader.endRead(std::move(innerReader));
  innerReader = setReader.beginRead();
  EXPECT_EQ(innerReader.read<ident::string_field>(), "foo");
  setReader.endRead(std::move(innerReader));
  EXPECT_THROW(setReader.beginRead(), std::out_of_range);
  listReader.endRead(std::move(setReader));
  reader.endRead(std::move(listReader));
  wrapper.endRead(std::move(reader));

  wrapper = StructCursor(Struct());
  reader = wrapper.beginRead();
  listReader = reader.beginRead<ident::set_nested_field>();
  EXPECT_THROW(listReader.beginRead(), std::out_of_range);
  reader.endRead(std::move(listReader));
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, TypesRead) {
  test::Types obj;
  obj.iobuf() = folly::IOBuf::wrapBufferAsValue("foo", 3);
  obj.iobufptr() = folly::IOBuf::wrapBuffer("bar", 3);
  obj.ms() = std::chrono::milliseconds(123456789);
  CursorSerializationWrapper wrapper(obj);
  auto reader = wrapper.beginRead();
  EXPECT_EQ(reader.read<ident::iobuf>().toString(), "foo");
  EXPECT_EQ(reader.read<ident::iobufptr>()->toString(), "bar");
  EXPECT_EQ(reader.read<ident::ms>().count(), 123456789);
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, QualifierWrite) {
  CursorSerializationWrapper<Qualifiers> wrapper;
  auto writer = wrapper.beginWrite();
  writer.write<ident::opt>(3);
  writer.write<ident::unq>(1);
  writer.write<ident::terse>(2);
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_EQ(*obj.opt(), 3);
  EXPECT_EQ(*obj.unq(), 1);
  EXPECT_EQ(*obj.terse(), 2);
  auto serializedLen = wrapper.serializedData().computeChainDataLength();

  // Skipping an optional field decreases serialized size.
  writer = wrapper.beginWrite();
  writer.write<ident::unq>(1);
  writer.write<ident::terse>(2);
  wrapper.endWrite(std::move(writer));

  obj = wrapper.deserialize();
  EXPECT_FALSE(obj.opt());
  EXPECT_EQ(*obj.unq(), 1);
  EXPECT_EQ(*obj.terse(), 2);
  EXPECT_LT(wrapper.serializedData().computeChainDataLength(), serializedLen);

  // Setting a terse field to its intrinsic default decreases serialized size.
  writer = wrapper.beginWrite();
  writer.write<ident::opt>(3);
  writer.write<ident::unq>(1);
  writer.write<ident::terse>(0);
  wrapper.endWrite(std::move(writer));

  obj = wrapper.deserialize();
  EXPECT_EQ(*obj.opt(), 3);
  EXPECT_EQ(*obj.unq(), 1);
  EXPECT_EQ(*obj.terse(), 0);
  EXPECT_LT(wrapper.serializedData().computeChainDataLength(), serializedLen);
}

TEST(CursorSerializer, WriteWithSkip) {
  StructCursor wrapper;
  auto writer = wrapper.beginWrite();
  writer.write<ident::i32_field>(42);
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_FALSE(obj.string_field());
  EXPECT_EQ(*obj.i32_field(), 42);
  EXPECT_TRUE(empty(*obj.union_field()));
  EXPECT_TRUE(obj.list_field()->empty());

  CursorSerializationWrapper<Meal> wrapperWithDefaults;
  auto writerWithDefaults = wrapperWithDefaults.beginWrite();
  writerWithDefaults.write<ident::main>(1);
  wrapperWithDefaults.endWrite(std::move(writerWithDefaults));

  auto meal = wrapperWithDefaults.deserialize();
  EXPECT_EQ(*meal.appetizer(), 1);
  EXPECT_EQ(*meal.main(), 1);
  EXPECT_EQ(*meal.dessert(), 3);
  CursorSerializationWrapper<OutOfOrder> oooWrapper;
  auto oooWriter = oooWrapper.beginWrite();
  oooWriter.write<ident::field5>(2);
  oooWrapper.endWrite(std::move(oooWriter));

  // Ensure we've actually written the skipped fields.
  BinaryProtocolReader reader;
  reader.setInput(&oooWrapper.serializedData());
  std::string name;
  reader.readStructBegin(name);

  auto checkField = [&](int16_t expectedId, int16_t expectedVal) {
    int16_t id;
    int16_t val;
    apache::thrift::protocol::TType type;
    reader.readFieldBegin(name, type, id);
    reader.readI16(val);
    reader.readFieldEnd();
    EXPECT_EQ(id, expectedId);
    EXPECT_EQ(val, expectedVal);
  };

  checkField(1, 0);
  checkField(4, 2); // field5
  checkField(5, 0);
  checkField(7, 0);
  checkField(12, 1);
}

TEST(CursorSerializer, UnionWrite) {
  CursorSerializationWrapper<Stringish> wrapper;
  auto writer = wrapper.beginWrite();
  writer.write<ident::string_field>("foo");
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_EQ(*obj.string_field(), "foo");
}

TEST(CursorSerializer, ManagedStringViewWrite) {
  CursorSerializationWrapper<StructWithCppType> wrapper;
  auto writer = wrapper.beginWrite();
  writer.write<ident::someId>(14);
  writer.write<ident::someName>("foobar");
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_EQ(obj.someId(), 14u);
  EXPECT_EQ(obj.someName()->str(), "foobar");
}

TEST(CursorSerializer, StructWithOptionalWrite) {
  {
    CursorSerializationWrapper<StructWithOptional> wrapper;
    auto writer = wrapper.beginWrite();
    writer.write<ident::optional_string>("baz");
    wrapper.endWrite(std::move(writer));

    auto obj = wrapper.deserialize();
    EXPECT_EQ(obj.optional_string(), "baz");
    EXPECT_FALSE(obj.optional_list().has_value());
    EXPECT_FALSE(obj.optional_map().has_value());
    EXPECT_FALSE(obj.optional_containers().has_value());
  }

  {
    CursorSerializationWrapper<StructWithOptional> wrapper;
    auto writer = wrapper.beginWrite();
    writer.write<ident::optional_string>("baz");
    auto list_writer = writer.beginWrite<ident::optional_list>();
    list_writer.write(1);
    list_writer.write(2);
    list_writer.write(3);
    writer.endWrite(std::move(list_writer));

    auto map = std::unordered_map<int, int>{{1, 2}, {3, 4}};
    writer.write<ident::optional_map>(map);

    auto container_writer = writer.beginWrite<ident::optional_containers>();
    auto string_list = {"foo", "bar"};
    container_writer.write<ident::list_of_string>(string_list);
    writer.endWrite(std::move(container_writer));

    wrapper.endWrite(std::move(writer));

    auto obj = wrapper.deserialize();
    EXPECT_EQ(obj.optional_string(), "baz");
    EXPECT_TRUE(obj.optional_list().has_value());
    EXPECT_TRUE(obj.optional_list()->size() == 3);
    EXPECT_TRUE(obj.optional_map().has_value());
    EXPECT_TRUE(obj.optional_map()->size() == 2);
    EXPECT_TRUE(obj.optional_containers().has_value());
    EXPECT_EQ(obj.optional_containers()->list_of_string()->size(), 2);
  }
}

TEST(CursorSerializer, NumericWrite) {
  CursorSerializationWrapper<Numerics> wrapper;
  auto writer = wrapper.beginWrite();
  writer.write<ident::int16>(1);
  writer.write<ident::uint32>(2);
  writer.write<ident::enm>(E::B);
  writer.write<ident::flt>(3.0);
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_EQ(*obj.int16(), 1);
  EXPECT_EQ(*obj.uint32(), 2);
  EXPECT_EQ(*obj.enm(), E::B);
  EXPECT_FLOAT_EQ(*obj.flt(), 3.0);
}

TEST(CursorSerializer, StringWrite) {
  StructCursor wrapper;

  auto writer = wrapper.beginWrite();
  writer.write<ident::string_field>(std::string_view("foo"));
  wrapper.endWrite(std::move(writer));
  EXPECT_EQ(*wrapper.deserialize().string_field(), "foo");

  writer = wrapper.beginWrite();
  writer.write<ident::string_field>(std::string("foo"));
  wrapper.endWrite(std::move(writer));
  EXPECT_EQ(*wrapper.deserialize().string_field(), "foo");

  writer = wrapper.beginWrite();
  writer.write<ident::string_field>(*folly::IOBuf::copyBuffer("foo"));
  wrapper.endWrite(std::move(writer));
  EXPECT_EQ(*wrapper.deserialize().string_field(), "foo");

  writer = wrapper.beginWrite();
  auto str = writer.beginWrite<ident::string_field>(12);
  memcpy(str.writeableData(), "foo", 3);
  writer.endWrite(std::move(str), 3);
  writer.write<ident::i32_field>(42);
  wrapper.endWrite(std::move(writer));
  EXPECT_EQ(*wrapper.deserialize().string_field(), "foo");
  EXPECT_EQ(*wrapper.deserialize().i32_field(), 42);
}

TEST(CursorSerializer, ContainerWrite) {
  StructCursor wrapper;
  auto writer = wrapper.beginWrite();
  auto list = writer.beginWrite<ident::list_field>();
  list.write('a');
  list.write('b');
  list.write('c');
  writer.endWrite(std::move(list));

  auto map = std::unordered_map<int8_t, int8_t>{{'a', 'b'}, {'c', 'd'}};
  writer.write<ident::map_field>(map);
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_THAT(*obj.list_field(), UnorderedElementsAreArray({'a', 'b', 'c'}));
  EXPECT_EQ(*obj.map_field(), map);
}

TEST(CursorSerializer, KnownSizeContainerWrite) {
  // Test writing a container with known size from StructuredCursorWriter
  StructCursor wrapper;
  auto writer = wrapper.beginWrite();
  auto list = writer.beginWrite<ident::list_field>(3);
  list.write('a');
  list.write('b');
  list.write('c');
  writer.endWrite(std::move(list));

  auto map = std::unordered_map<int8_t, int8_t>{{'a', 'b'}, {'c', 'd'}};
  writer.write<ident::map_field>(map);
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_THAT(*obj.list_field(), UnorderedElementsAreArray({'a', 'b', 'c'}));
  EXPECT_EQ(*obj.map_field(), map);
}

TEST(CursorSerializer, KnownSizeContainerWriteSizeMismatch) {
  // Test that writing fewer elements than declared throws
  {
    StructCursor wrapper;
    auto writer = wrapper.beginWrite();
    auto list = writer.beginWrite<ident::list_field>(3);
    list.write('a');
    list.write('b');
    // Only wrote 2 elements but declared 3
    EXPECT_THAT(
        [&] { writer.endWrite(std::move(list)); },
        ThrowsMessage<std::runtime_error>("Expected 3 elements but wrote 2"));
    // After finalize throws, parent is Active (exception-safe endWrite) and
    // child is Done (finalize sets state before throwing). Just abandon parent.
    wrapper.abandonWrite(std::move(writer));
  }

  // Test that writing more elements than declared throws
  {
    StructCursor wrapper;
    auto writer = wrapper.beginWrite();
    auto list = writer.beginWrite<ident::list_field>(2);
    list.write('a');
    list.write('b');
    list.write('c');
    // Wrote 3 elements but declared 2
    EXPECT_THAT(
        [&] { writer.endWrite(std::move(list)); },
        ThrowsMessage<std::runtime_error>("Expected 2 elements but wrote 3"));
    wrapper.abandonWrite(std::move(writer));
  }
}

TEST(CursorSerializer, KnownSizeNestedContainerWrite) {
  // Test nested containers with known size from ContainerCursorWriter
  // Using set_nested_field which is list<set<Stringish>>
  StructCursor wrapper;
  auto writer = wrapper.beginWrite();

  // Write set_nested_field with known outer size
  auto outerList = writer.beginWrite<ident::set_nested_field>(1);
  auto innerSet = outerList.beginWrite();
  auto unionWriter = innerSet.beginWrite();
  unionWriter.write<ident::string_field>("test");
  innerSet.endWrite(std::move(unionWriter));
  outerList.endWrite(std::move(innerSet));
  writer.endWrite(std::move(outerList));

  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_EQ(obj.set_nested_field()->size(), 1);
  EXPECT_EQ(obj.set_nested_field()->at(0).size(), 1);
}

TEST(CursorSerializer, NestedStructWrite) {
  StructCursor wrapper;
  auto writer = wrapper.beginWrite();
  auto innerWriter = writer.beginWrite<ident::union_field>();

  // Can't use parent writer while child writer is active.
  EXPECT_THAT(
      [&] { writer.write<ident::list_field>(std::array<int8_t, 1>{42}); },
      ThrowsMessage<std::runtime_error>("Child writer not passed to endWrite"));

  // endWrite in the middle of writing the child skips to the end.
  writer.endWrite(std::move(innerWriter));
  writer.write<ident::list_field>(std::array<int8_t, 1>{42});
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_EQ(obj.union_field()->getType(), Inner::Type::__EMPTY__);
  EXPECT_THAT(*obj.list_field(), ElementsAre(42));
}

TEST(CursorSerializer, NestedStructWriteWithSeparateCursor) {
  CursorSerializationWrapper<Meal> mealCursor;
  CursorSerializationWrapper<Cookie> cookieCursor;
  CursorSerializationWrapper<Struct> structCursor;

  auto cookieWriter = cookieCursor.beginWrite();
  cookieWriter.write<ident::id>(42);
  cookieWriter.write<ident::fortune>("Meaning of life...");
  cookieWriter.write<ident::flavor>("None");
  cookieCursor.endWrite(std::move(cookieWriter));

  auto structWriter = structCursor.beginWrite();
  structWriter.write<ident::i32_field>(42);
  structCursor.endWrite(std::move(structWriter));

  auto mealWriter = mealCursor.beginWrite();
  mealWriter.write<ident::appetizer>(2);
  mealWriter.write<ident::main>(4);
  mealWriter.writeSerialized<ident::cookie>(std::move(cookieCursor));
  // Doesn't compile
  // mealWriter.writeSerialized<ident::cookie>(std::move(structCursor));
  mealCursor.endWrite(std::move(mealWriter));

  auto deserialized = mealCursor.deserialize();
  EXPECT_EQ(*deserialized.appetizer(), 2);
  EXPECT_EQ(*deserialized.main(), 4);
  EXPECT_EQ(*deserialized.cookie()->flavor(), "None");
  EXPECT_EQ(*deserialized.cookie()->id(), 42);
  EXPECT_EQ(*deserialized.cookie()->fortune(), "Meaning of life...");

  {
    auto mealReader = mealCursor.beginRead();
    EXPECT_EQ(mealReader.read<ident::appetizer>(), 2);
    EXPECT_EQ(mealReader.read<ident::main>(), 4);

    auto cookieReader = mealReader.beginRead<ident::cookie>();
    EXPECT_EQ(cookieReader.read<ident::id>(), 42);
    EXPECT_EQ(cookieReader.read<ident::fortune>(), "Meaning of life...");
    EXPECT_EQ(cookieReader.read<ident::flavor>(), "None");
    mealReader.endRead(std::move(cookieReader));
    mealCursor.endRead(std::move(mealReader));
  }
}

TEST(CursorSerializer, CursorWriteInContainer) {
  StructCursor wrapper;
  auto writer = wrapper.beginWrite();
  auto listWriter = writer.beginWrite<ident::set_nested_field>();
  auto setWriter = listWriter.beginWrite();
  auto innerWriter = setWriter.beginWrite();
  innerWriter.write<ident::string_field>("foo");
  setWriter.endWrite(std::move(innerWriter));
  listWriter.endWrite(std::move(setWriter));
  writer.endWrite(std::move(listWriter));
  wrapper.endWrite(std::move(writer));

  auto obj = wrapper.deserialize();
  EXPECT_THAT(
      *obj.set_nested_field(),
      Contains(Contains(IsThriftUnionWith<ident::string_field>(Eq("foo")))));
}

void doCursorReadRemainEndTest(int count) {
  std::unique_ptr<folly::IOBuf> buf;
  {
    ReadRemainingWrapper wrapper;
    auto writer = wrapper.beginWrite();

    auto listWriter = writer.beginWrite<ident::aaa>();
    for (int i = 0; i < count; ++i) {
      auto s = "a string " + std::to_string(i);
      listWriter.write(s);
    }
    writer.endWrite(std::move(listWriter));

    auto bwriter = writer.beginWrite<ident::bbb>();
    for (int i = 0; i < count; ++i) {
      bwriter.write(i);
    }
    writer.endWrite(std::move(bwriter));

    writer.write<ident::ccc>(true);

    wrapper.endWrite(std::move(writer));

    buf = std::move(wrapper).serializedData();
  }

  {
    ReadRemainingWrapper wrapper = ReadRemainingWrapper(std::move(buf));
    auto reader = wrapper.beginRead();
    auto listReader = reader.beginRead<ident::aaa>();
    listReader.remaining();
    reader.endRead(std::move(listReader));
    auto breader = reader.beginRead<ident::bbb>();
    reader.endRead(std::move(breader));
    wrapper.endRead(std::move(reader));
  }
}

TEST(CursorSerializer, Refs) {
  Refs wrapper;

  auto writer = wrapper.beginWrite();
  writer.write<ident::unique>(Empty{});
  writer.write<ident::shared>(Empty{});
  writer.write<ident::shared_mutable>(Empty{});
  writer.write<ident::box>(Empty{});
  writer.write<ident::intern_box>(Empty{});
  wrapper.endWrite(std::move(writer));

  auto reader = wrapper.beginRead();
  EXPECT_EQ(reader.read<ident::unique>(), Empty{});
  EXPECT_EQ(reader.read<ident::shared>(), Empty{});
  EXPECT_EQ(reader.read<ident::shared_mutable>(), Empty{});
  EXPECT_EQ(reader.read<ident::box>(), Empty{});
  EXPECT_EQ(reader.read<ident::intern_box>(), Empty{});
  wrapper.endRead(std::move(reader));
}

TEST(CursorBasedSerializer, CursorReadRemainingEndOne) {
  doCursorReadRemainEndTest(1);
}

TEST(CursorBasedSerializer, CursorReadRemainingEndMany) {
  doCursorReadRemainEndTest(10);
}

TEST(CursorBasedSerializer, ConcurrentAccess) {
  EmptyWrapper wrapper;
  auto writer = wrapper.beginWrite();
  EXPECT_THROW(wrapper.beginRead(), std::runtime_error);
  EXPECT_THROW(wrapper.deserialize(), std::runtime_error);
  EXPECT_THROW(wrapper.beginWrite(), std::runtime_error);
  wrapper.endWrite(std::move(writer));

  auto reader = wrapper.beginRead();
  EXPECT_THROW(wrapper.beginRead(), std::runtime_error);
  EXPECT_EQ(wrapper.deserialize(), Empty{});
  EXPECT_THROW(wrapper.beginWrite(), std::runtime_error);
  wrapper.endRead(std::move(reader));
}

TEST(CursorSerializer, AbandonedWrite) {
  CursorSerializationWrapper<Qualifiers> wrapper;
  auto writer = wrapper.beginWrite();
  writer.write<ident::opt>(3);
  wrapper.abandonWrite(std::move(writer));
}

TEST(CursorSerializer, MoveActiveWrapper) {
  // @lint-ignore-all CLANGTIDY bugprone-use-after-move
  CursorSerializationWrapper<Qualifiers> wrapper;
  auto writer = wrapper.beginWrite();
  std::optional<CursorSerializationWrapper<Qualifiers>> wrapper2;
  EXPECT_THROW(wrapper2.emplace(std::move(wrapper)), std::runtime_error);
  wrapper2.emplace();
  EXPECT_THROW((*wrapper2 = std::move(wrapper)), std::runtime_error);
  wrapper.abandonWrite(std::move(writer));

  wrapper2.emplace(std::move(wrapper));

  auto writer2 = wrapper2->beginWrite();
  EXPECT_THROW((*wrapper2 = std::move(wrapper)), std::runtime_error);
  wrapper2->abandonWrite(std::move(writer2));

  *wrapper2 = std::move(wrapper);
}
