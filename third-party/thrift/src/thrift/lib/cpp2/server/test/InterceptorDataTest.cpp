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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

using namespace apache::thrift;

class InterceptorDataTest : public testing::Test {
 public:
  void SetUp() override {
    auto server1_ = std::make_unique<ThriftServer>();
    auto server2_ = std::make_unique<ThriftServer>();
    // NOLINTBEGIN(facebook-hte-DetailCall)
    interceptorData_ = detail::getInterceptorData(*server1_.get());
    interceptorData2_ = detail::getInterceptorData(*server2_.get());
    // NOLINTEND(facebook-hte-DetailCall)
  }
  InterceptorData interceptorData_, interceptorData2_;
};

TEST_F(InterceptorDataTest, TestGenerateInterceptorToken) {
  auto token1 = interceptorData_.generateThriftInterceptorToken("server1");
  auto token2 = interceptorData_.generateThriftInterceptorToken("server2");
  auto token3 = interceptorData_.generateThriftInterceptorToken("server1");
  auto token4 = interceptorData2_.generateThriftInterceptorToken("server1");
  auto token5 = interceptorData_.generateThriftInterceptorToken("server2");
  // tokens should be unique
  EXPECT_NE(token1, token2);
  // tokens for the same tpeh and same server should be equal
  EXPECT_EQ(token1, token3);
  // even though token 3 and 4 are the same tpeh, since different server, they
  // are not equal
  EXPECT_NE(token3, token4);
  // test that get() returns the expected value
  EXPECT_EQ(token1.get(), 0);
  EXPECT_EQ(token2.get(), 1);
  EXPECT_EQ(token3.get(), 0);
  EXPECT_EQ(token4.get(), 0);
  EXPECT_EQ(token5.get(), 1);
}

TEST_F(InterceptorDataTest, TestGetInterceptorToken) {
  interceptorData_.generateThriftInterceptorToken("server1");
  interceptorData_.generateThriftInterceptorToken("server2");
  auto token1 = interceptorData_.getThriftInterceptorToken("server1");
  auto token2 = interceptorData_.getThriftInterceptorToken("server2");
  auto token3 = interceptorData_.getThriftInterceptorToken("server3");
  // valid storage ids return valid tokens
  EXPECT_NE(token1, std::nullopt);
  EXPECT_NE(token2, std::nullopt);
  // we used an invalid storage id (one that hasn't been used to generate a
  // token). so returned token is empty
  EXPECT_EQ(token3, std::nullopt);
}
