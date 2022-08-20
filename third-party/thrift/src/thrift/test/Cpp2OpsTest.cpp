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

#include <glog/logging.h>
#include <folly/portability/GTest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/test/gen-cpp2/Cpp2OpsNative_constants.h>
#include <thrift/test/gen-cpp2/Cpp2OpsNative_types.h>

namespace thrift {
namespace test {
namespace cpp2ops {

template <class T>
class Cpp2OpsTest : public testing::Test {};

TYPED_TEST_CASE_P(Cpp2OpsTest);

TYPED_TEST_P(Cpp2OpsTest, Simple) {
  std::unique_ptr<folly::IOBuf> buf;
  uint32_t size;
  {
    apache::thrift::CompactProtocolWriter writer;

    TypeParam obj;
    *obj.num_ref() = cpp2::Cpp2OpsNative_constants::num_test_value();
    *obj.str_ref() = cpp2::Cpp2OpsNative_constants::str_test_value();

    // This is mainly to test that it compiles...
    EXPECT_NE(
        0, apache::thrift::Cpp2Ops<TypeParam>::serializedSize(&writer, &obj));
    EXPECT_NE(
        0, apache::thrift::Cpp2Ops<TypeParam>::serializedSizeZC(&writer, &obj));

    folly::IOBufQueue queue;
    writer.setOutput(&queue);

    size = apache::thrift::Cpp2Ops<TypeParam>::write(&writer, &obj);
    EXPECT_NE(0, size);
    buf = queue.move();
  }

  {
    apache::thrift::CompactProtocolReader reader;
    reader.setInput(buf.get());

    TypeParam obj;
    apache::thrift::Cpp2Ops<TypeParam>::read(&reader, &obj);
    EXPECT_EQ(size, reader.getCursorPosition());

    EXPECT_EQ(cpp2::Cpp2OpsNative_constants::num_test_value(), *obj.num_ref());
    EXPECT_EQ(cpp2::Cpp2OpsNative_constants::str_test_value(), *obj.str_ref());
  }
}

REGISTER_TYPED_TEST_CASE_P(Cpp2OpsTest, Simple);

typedef testing::Types<cpp2::Native> Types;
INSTANTIATE_TYPED_TEST_CASE_P(Cpp2OpsTest, Cpp2OpsTest, Types);

} // namespace cpp2ops
} // namespace test
} // namespace thrift

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  return RUN_ALL_TESTS();
}
