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

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/KeepAliveHandler.h>

namespace apache::thrift::rocket {
namespace {

struct HandlerOwner {
  size_t sent{0};
  bool respond{true};
  std::unique_ptr<folly::IOBuf> data;
};

template <typename Owner>
class TestConnectionAdapter {
 public:
  explicit TestConnectionAdapter(Owner& owner) : owner_(&owner) {}

  void sendFrame(KeepAliveFrame&& frame) {
    ++owner_->sent;
    owner_->respond = frame.hasRespondFlag();
    owner_->data = std::move(frame).data();
  }

 private:
  Owner* owner_;
};

using Handler = KeepAliveHandler<HandlerOwner, TestConnectionAdapter>;

TEST(KeepAliveHandlerTest, RespondsWithoutRespondFlag) {
  HandlerOwner owner;
  TestConnectionAdapter<HandlerOwner> connection(owner);
  Handler handler(connection);
  auto data = folly::IOBuf::copyBuffer("payload");

  EXPECT_TRUE(
      handler.handle(KeepAliveFrame{Flags().respond(true), data->clone()}));

  EXPECT_EQ(owner.sent, 1);
  EXPECT_FALSE(owner.respond);
  ASSERT_NE(owner.data, nullptr);
  EXPECT_TRUE(folly::IOBufEqualTo()(owner.data, data));
}

TEST(KeepAliveHandlerTest, DoesNotRespondWithoutRespondFlag) {
  HandlerOwner owner;
  TestConnectionAdapter<HandlerOwner> connection(owner);
  Handler handler(connection);

  EXPECT_TRUE(handler.handle(
      KeepAliveFrame{Flags(), folly::IOBuf::copyBuffer("payload")}));

  EXPECT_EQ(owner.sent, 0);
}

} // namespace
} // namespace apache::thrift::rocket
