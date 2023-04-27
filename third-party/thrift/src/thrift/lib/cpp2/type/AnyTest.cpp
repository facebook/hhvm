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

#include <thrift/lib/cpp2/type/Any.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache::thrift::type {
namespace {

TEST(AnyTest, BaseApi) {
  SemiAny builder;
  EXPECT_THROW(AnyData{builder}, std::runtime_error);
  builder.type() = i16_t{};
  EXPECT_THROW(AnyData{builder}, std::runtime_error);
  builder.protocol() = StandardProtocol::Compact;
  builder.data() = folly::IOBuf::wrapBufferAsValue("hi", 2);

  AnyData any(builder);
  EXPECT_EQ(any.type(), Type::get<i16_t>());
  EXPECT_EQ(any.protocol(), Protocol::get<StandardProtocol::Compact>());
  EXPECT_EQ(any.data().data(), builder.data()->data());
  EXPECT_EQ(any.data().length(), 2);

  builder.type() = {};
  EXPECT_THROW(AnyData{builder}, std::runtime_error);
}

} // namespace
} // namespace apache::thrift::type
