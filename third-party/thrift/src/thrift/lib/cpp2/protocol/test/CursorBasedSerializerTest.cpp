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
