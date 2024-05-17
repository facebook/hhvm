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

#include <ranges>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/cursor_clients.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/cursor_handlers.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/cursor_types.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

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
  EXPECT_EQ(reader.read<ident::terse>(), 2);
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

  string.string_field_ref() = "foo";
  wrapper = CursorSerializationWrapper<Stringish>(string);
  reader = wrapper.beginRead();
  EXPECT_EQ(reader.readType(), Stringish::Type::string_field);
  EXPECT_EQ(reader.read<ident::string_field>(), "foo");
  EXPECT_FALSE(reader.read<ident::binary_field>());
  wrapper.endRead(std::move(reader));

  string.binary_field_ref() = folly::IOBuf::wrapBufferAsValue(
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
  auto reader = wrapper.beginRead();
  std::string_view str;
  EXPECT_TRUE(reader.read<ident::string_field>(str));
  EXPECT_EQ("foo", str);
  wrapper.endRead(std::move(reader));

  reader = wrapper.beginRead();
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

  reader = wrapper.beginRead();
  listReader = reader.beginRead<ident::lucky_numbers>();
  for (auto i : listReader) {
    EXPECT_GT(i, 500);
    break;
  }
  // Ending read in the middle of iteration still allows reading next field.
  reader.endRead(std::move(listReader));
  EXPECT_EQ(reader.read<ident::flavor>(), "Sugar");
  wrapper.endRead(std::move(reader));

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
  EXPECT_EQ(*obj.string_field_ref(), "foo");
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
