/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/container_size_types.h>

namespace apache::thrift::test {

template <class T>
struct ContainerSizeTest : testing::Test {
  void SetUp() override {
    auto buf = folly::IOBuf::takeOwnership(
        getAddr<folly::IOBuf>(), kMemSize, [](void*, void*) {});
    buf->clear();
    q.append(std::move(buf));
    writer.setOutput(&q);
  }

  folly::IOBufQueue q;
  T writer;
};

using Writers = ::testing::Types<CompactProtocolWriter, BinaryProtocolWriter>;
TYPED_TEST_SUITE(ContainerSizeTest, Writers);

TYPED_TEST(ContainerSizeTest, string) {
  Struct obj;
  obj.str().emplace();
  obj.str()->resize(kMaxSize);
  obj.write(&this->writer);
  obj.str()->resize(kExceededSize);
  EXPECT_THROW(
      obj.write(&this->writer), apache::thrift::protocol::TProtocolException);
}

TYPED_TEST(ContainerSizeTest, iobuf) {
  Struct obj;
  obj.iobuf() = std::move(
      *folly::IOBuf::takeOwnership(getAddr(), kMaxSize, [](void*, void*) {}));
  obj.write(&this->writer);
  obj.iobuf() = std::move(*folly::IOBuf::takeOwnership(
      getAddr(), kExceededSize, [](void*, void*) {}));
  EXPECT_THROW(
      obj.write(&this->writer), apache::thrift::protocol::TProtocolException);
}

TYPED_TEST(ContainerSizeTest, unique_iobuf) {
  Struct obj;
  obj.unique_iobuf() =
      folly::IOBuf::takeOwnership(getAddr(), kMaxSize, [](void*, void*) {});
  obj.write(&this->writer);
  obj.unique_iobuf() = folly::IOBuf::takeOwnership(
      getAddr(), kExceededSize, [](void*, void*) {});
  EXPECT_THROW(
      obj.write(&this->writer), apache::thrift::protocol::TProtocolException);
}

TYPED_TEST(ContainerSizeTest, list) {
  Struct obj;
  obj.l().emplace().mockedSize = kMaxSize;
  obj.write(&this->writer);
  obj.l().emplace().mockedSize = kExceededSize;
  EXPECT_THROW(
      obj.write(&this->writer), apache::thrift::protocol::TProtocolException);
}

TYPED_TEST(ContainerSizeTest, set) {
  Struct obj;
  obj.s().emplace().mockedSize = kMaxSize;
  obj.write(&this->writer);
  obj.s().emplace().mockedSize = kExceededSize;
  EXPECT_THROW(
      obj.write(&this->writer), apache::thrift::protocol::TProtocolException);
}

TYPED_TEST(ContainerSizeTest, map) {
  Struct obj;
  obj.m().emplace().mockedSize = kMaxSize;
  obj.write(&this->writer);
  obj.m().emplace().mockedSize = kExceededSize;
  EXPECT_THROW(
      obj.write(&this->writer), apache::thrift::protocol::TProtocolException);
}
} // namespace apache::thrift::test
