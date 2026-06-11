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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionContextHandler.h>

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

using channel_pipeline::erase_and_box;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;

class FakeContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    forwarded.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception = std::move(e);
  }

  std::vector<TypeErasedBox> forwarded;
  folly::exception_wrapper exception;
};

// Simulates the upstream ThriftServerRequestContextHandler stamping a fresh
// empty request context onto each message.
ThriftServerRequestMessage makeRequestWithContext(uint32_t streamId = 1) {
  ThriftServerRequestMessage req;
  req.streamId = streamId;
  req.requestContext = std::make_unique<ThriftRequestContext>();
  return req;
}

} // namespace

TEST(
    ThriftServerConnectionContextHandlerTest,
    StampsConnContextOntoRequestContext) {
  boost::intrusive_ptr<ThriftConnContext> conn{new ThriftConnContext()};
  conn->setSecurityProtocol("TLS1.3");

  ThriftServerConnectionContextHandler<FakeContext> handler{conn};
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(
          ctx, erase_and_box(makeRequestWithContext(/*streamId=*/42))),
      Result::Success);

  ASSERT_EQ(ctx.forwarded.size(), 1);
  auto& forwarded = ctx.forwarded.front().get<ThriftServerRequestMessage>();
  EXPECT_EQ(forwarded.streamId, 42);
  ASSERT_NE(forwarded.requestContext, nullptr);
  EXPECT_EQ(forwarded.requestContext->getConnectionContext(), conn.get());
}

TEST(ThriftServerConnectionContextHandlerTest, EveryRequestSharesTheSameConn) {
  ThriftServerConnectionContextHandler<FakeContext> handler{
      boost::intrusive_ptr<ThriftConnContext>{new ThriftConnContext()}};
  FakeContext ctx;

  for (uint32_t sid = 1; sid <= 3; ++sid) {
    EXPECT_EQ(
        handler.onRead(ctx, erase_and_box(makeRequestWithContext(sid))),
        Result::Success);
  }

  auto* expected = handler.getConnectionContext().get();
  // Handler holds 1 ref; each per-request RequestContext holds 1 ref.
  EXPECT_EQ(expected->use_count(), 1 + 3);
  for (auto& box : ctx.forwarded) {
    auto& m = box.get<ThriftServerRequestMessage>();
    ASSERT_NE(m.requestContext, nullptr);
    EXPECT_EQ(m.requestContext->getConnectionContext(), expected);
  }
}

TEST(
    ThriftServerConnectionContextHandlerTest,
    ConnContextOutlivesHandlerViaStampedMessage) {
  bool destroyed = false;
  TypeErasedBox stamped;
  {
    boost::intrusive_ptr<ThriftConnContext> conn{new ThriftConnContext()};
    conn->setUserData(
        rocket::with_custom_deleter(
            &destroyed,
            +[](void* p) noexcept { *static_cast<bool*>(p) = true; }));

    ThriftServerConnectionContextHandler<FakeContext> handler{std::move(conn)};
    FakeContext ctx;
    EXPECT_EQ(
        handler.onRead(ctx, erase_and_box(makeRequestWithContext())),
        Result::Success);
    stamped = std::move(ctx.forwarded.front());
    // Handler goes out of scope; the stamped RequestContext keeps the conn
    // context alive.
  }
  EXPECT_FALSE(destroyed);

  stamped = {};
  EXPECT_TRUE(destroyed);
}

TEST(ThriftServerConnectionContextHandlerTest, ForwardsExceptions) {
  ThriftServerConnectionContextHandler<FakeContext> handler{
      boost::intrusive_ptr<ThriftConnContext>{new ThriftConnContext()}};
  FakeContext ctx;

  handler.onException(
      ctx, folly::make_exception_wrapper<std::runtime_error>("boom"));

  ASSERT_TRUE(ctx.exception);
  EXPECT_NE(ctx.exception.what().toStdString().find("boom"), std::string::npos);
}

} // namespace apache::thrift::fast_thrift::thrift
