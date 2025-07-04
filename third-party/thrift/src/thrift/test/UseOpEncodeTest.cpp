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

#include <gtest/gtest.h>
#include <thrift/conformance/cpp2/internal/AnyStructSerializer.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/UseOpEncodeProgram_types.h>
#include <thrift/test/gen-cpp2/UseOpEncode_types.h>

// TODO: Remove this. Specify namespace explicitly instead.
using namespace ::apache::thrift::conformance;

using detail::protocol_reader_t;
using detail::protocol_writer_t;

namespace apache::thrift::test {

template <StandardProtocol Protocol>
void testUseOpEncode() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  protocol_writer_t<Protocol> writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  OpEncodeStruct original;
  Foo foo;
  foo.field() = 3;
  original.foo_field() = foo;
  original.int_field() = 5;
  original.enum_field() = Enum::first;
  auto adaptedFoo = test::TemplatedTestAdapter::fromThrift(foo);
  original.adapted_field() = adaptedFoo;
  original.list_field().emplace().push_back(adaptedFoo);
  original.list_shared_ptr_field() =
      std::make_shared<std::vector<test::Wrapper<Foo>>>(
          original.list_field().value());
  original.list_cpp_type_field().emplace().push_back(adaptedFoo);
  original.set_field().emplace().insert(adaptedFoo);
  original.map_field().emplace()[adaptedFoo] = adaptedFoo;
  original.nested_field().emplace()[1] = original.list_field().value();
  original.bar_field().emplace().list_field() = original.list_field().value();

  auto adaptedBar = test::TemplatedTestAdapter::fromThrift(Bar{});
  auto adaptedInt = test::TemplatedTestAdapter::fromThrift(1);
  original.adapted_int_field() = adaptedInt;
  original.list_int_field().emplace().push_back(adaptedInt);
  original.meta() = "some metadata";
  original.adapted_list_field()->value.push_back(adaptedFoo);
  original.inplace_adapted_list_field()->value.push_back(adaptedFoo);
  original.nested_map_field().emplace()[adaptedFoo] = {
      {adaptedBar, adaptedInt}};
  original.field20().emplace().push_back(
      test::TemplatedTestAdapter::fromThrift(std::set<int32_t>{1}));
  original.field21().emplace().push_back(
      test::Wrapper<std::set<test::Wrapper<int32_t>>>{{{1}}});
  original.field22().emplace().push_back(
      test::Wrapper<std::set<test::Wrapper<std::set<test::Wrapper<int32_t>>>>>{
          {{{{1}}}}});

  original.write(&writer);

  protocol_reader_t<Protocol> reader;
  auto serialized = queue.move();
  reader.setInput(serialized.get());
  OpEncodeStruct result;

  // Make sure the fields are cleared
  ++*foo.field();
  result.list_field()->push_back(test::TemplatedTestAdapter::fromThrift(foo));
  result.adapted_list_field()->value.push_back(
      test::TemplatedTestAdapter::fromThrift(foo));
  result.inplace_adapted_list_field()->value.push_back(
      test::TemplatedTestAdapter::fromThrift(foo));
  result.field20()->emplace_back();
  result.field21()->emplace_back();
  result.field22()->emplace_back();

  result.read(&reader);
  EXPECT_EQ(result, original);

  // Test whether field adapter is used
  EXPECT_EQ(result.adapted_list_field()->fieldId, 14);
  EXPECT_EQ(*result.adapted_list_field()->meta, "some metadata");
  EXPECT_EQ(result.nested_map_field()[adaptedFoo][adaptedBar], adaptedInt);
}

TEST(UseOpEncodeTest, UseOpEncode) {
  testUseOpEncode<StandardProtocol::Binary>();
  testUseOpEncode<StandardProtocol::Compact>();
  testUseOpEncode<StandardProtocol::SimpleJson>();
}

template <StandardProtocol Protocol>
void testSerializedSize() {
  SCOPED_TRACE(apache::thrift::util::enumNameSafe(Protocol));
  protocol_writer_t<Protocol> writer;
  // Construct BazWithUseOpEncode and Baz such that they have the same
  // serialized data.
  BazWithUseOpEncode bazWithUseOpEncode;
  Baz baz;
  bazWithUseOpEncode.int_field() = 5;
  baz.int_field() = 5;
  bazWithUseOpEncode.enum_field() = Enum::second;
  baz.enum_field() = Enum::second;
  Foo foo;
  foo.field() = 3;
  auto adaptedFoo = test::TemplatedTestAdapter::fromThrift(foo);
  bazWithUseOpEncode.list_field().emplace().push_back(adaptedFoo);
  baz.list_field().emplace().push_back(foo);
  bazWithUseOpEncode.map_field().emplace()[3] = adaptedFoo;
  baz.map_field().emplace()[3] = foo;
  bazWithUseOpEncode.list_shared_ptr_field() =
      std::make_shared<std::vector<test::Wrapper<Foo>>>(
          bazWithUseOpEncode.list_field().value());
  baz.list_shared_ptr_field() =
      std::make_shared<std::vector<Foo>>(baz.list_field().value());

  auto size = bazWithUseOpEncode.serializedSize(&writer);
  auto expected = baz.serializedSize(&writer);
  EXPECT_EQ(size, expected);
}

TEST(UseOpEncodeTest, SerializedSize) {
  testSerializedSize<StandardProtocol::Binary>();
  testSerializedSize<StandardProtocol::Compact>();
  testSerializedSize<StandardProtocol::SimpleJson>();
}
TEST(UseOpEncodeTest, ProgramScopeAnnotation) {
  FooList a;
  a.adapted_list_field()->value.emplace_back().value.field() = 42;
  auto b = apache::thrift::CompactSerializer::deserialize<FooList>(
      apache::thrift::CompactSerializer::serialize<std::string>(a));
  EXPECT_EQ(b.adapted_list_field()->value.at(0).value.field(), 42);
}

template <class T>
constexpr bool kStructUsesOpEncode =
    decltype(apache::thrift::detail::st::struct_private_access::
                 __fbthrift_cpp2_uses_op_encode<T>())::value;

TEST(UseOpEncodeTest, StructUsesOpEncode) {
  static_assert(!kStructUsesOpEncode<Foo>);
  static_assert(kStructUsesOpEncode<Bar>);
  static_assert(!kStructUsesOpEncode<FooUnion>);
  static_assert(kStructUsesOpEncode<BarUnion>);
  static_assert(!kStructUsesOpEncode<Baz>);
  static_assert(kStructUsesOpEncode<BazWithUseOpEncode>);
  static_assert(!kStructUsesOpEncode<BarWrapper1>);
  static_assert(!kStructUsesOpEncode<BarWrapper2>);
  static_assert(!kStructUsesOpEncode<BarWrapper3>);
  static_assert(!kStructUsesOpEncode<BarWrapper4>);
}
} // namespace apache::thrift::test
