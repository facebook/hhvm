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

#include <wangle/acceptor/SecurityProtocolContextManager.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thread>

using namespace wangle;
using namespace testing;

template <size_t N>
class LengthPeeker : public PeekingAcceptorHandshakeHelper::PeekCallback {
 public:
  LengthPeeker() : PeekingAcceptorHandshakeHelper::PeekCallback(N) {}

  AcceptorHandshakeHelper::UniquePtr getHelper(
      const std::vector<uint8_t>& /* bytes */,
      const folly::SocketAddress& /* clientAddr */,
      std::chrono::steady_clock::time_point /* acceptTime */,
      TransportInfo&) override {
    return nullptr;
  }
};

class SecurityProtocolContextManagerTest : public Test {
 protected:
  SecurityProtocolContextManager manager_;
  LengthPeeker<0> p0_;
  LengthPeeker<2> p2_;
  LengthPeeker<4> p4_;
  LengthPeeker<9> p9_;
};

TEST_F(SecurityProtocolContextManagerTest, TestZeroLen) {
  manager_.addPeeker(&p0_);

  EXPECT_EQ(manager_.getPeekBytes(), 0);
}

TEST_F(SecurityProtocolContextManagerTest, TestLongAtStart) {
  manager_.addPeeker(&p9_);
  manager_.addPeeker(&p0_);
  manager_.addPeeker(&p4_);
  manager_.addPeeker(&p2_);

  EXPECT_EQ(manager_.getPeekBytes(), 9);
}

TEST_F(SecurityProtocolContextManagerTest, TestLongAtEnd) {
  manager_.addPeeker(&p0_);
  manager_.addPeeker(&p4_);
  manager_.addPeeker(&p2_);
  manager_.addPeeker(&p9_);

  EXPECT_EQ(manager_.getPeekBytes(), 9);
}

TEST_F(SecurityProtocolContextManagerTest, TestLongMiddle) {
  manager_.addPeeker(&p0_);
  manager_.addPeeker(&p9_);
  manager_.addPeeker(&p2_);
  manager_.addPeeker(&p0_);

  EXPECT_EQ(manager_.getPeekBytes(), 9);
}
