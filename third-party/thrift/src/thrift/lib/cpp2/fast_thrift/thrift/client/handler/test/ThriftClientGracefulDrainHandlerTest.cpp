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

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientGracefulDrainHandler.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

class MockDrainContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  // Mirror the real pipeline: deactivate() synchronously cascades
  // onPipelineInactive back through the handler, which is where it settles
  // into Closed.
  void deactivate() noexcept {
    ++deactivateCount_;
    if (handler_) {
      handler_->onPipelineInactive(*this);
    }
  }

  void setHandler(ThriftClientGracefulDrainHandler* handler) noexcept {
    handler_ = handler;
  }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }
  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }
  bool hasException() const { return static_cast<bool>(exception_); }
  int deactivateCount() const { return deactivateCount_; }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    deactivateCount_ = 0;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  int deactivateCount_{0};
  ThriftClientGracefulDrainHandler* handler_{nullptr};
};

TypeErasedBox makeCloseEvent() {
  // CloseConnection carries no payload — the type alone is the signal.
  return TypeErasedBox{};
}

// Any inbound response — content is irrelevant to in-flight accounting; the
// handler decrements and forwards every inbound message.
ThriftResponseMessage makeResponse() {
  ThriftResponseMessage resp;
  resp.payload = ThriftClientResponseError{};
  return resp;
}

ThriftRequestMessage makeRequest(void* requestContext) {
  return ThriftRequestMessage{
      .payload =
          ThriftClientOutboundPayloadVariant{ThriftRequestResponsePayload{}},
      .requestContext =
          apache::thrift::fast_thrift::rocket::borrow(requestContext),
  };
}

} // namespace

class ThriftClientGracefulDrainHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ctx_.reset();
    ctx_.setHandler(&handler_);
  }

  MockDrainContext ctx_;
  ThriftClientGracefulDrainHandler handler_;
};

TEST_F(ThriftClientGracefulDrainHandlerTest, StartsOpen) {
  EXPECT_FALSE(handler_.isDraining());
  EXPECT_FALSE(handler_.isClosed());
  EXPECT_EQ(handler_.inFlight(), 0u);
}

TEST_F(ThriftClientGracefulDrainHandlerTest, WriteForwardedAndCountedWhenOpen) {
  auto result = handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_EQ(handler_.inFlight(), 1u);
}

TEST_F(ThriftClientGracefulDrainHandlerTest, ResponseDecrementsInFlight) {
  (void)handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));
  (void)handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));
  ASSERT_EQ(handler_.inFlight(), 2u);

  (void)handler_.onRead(ctx_, erase_and_box(makeResponse()));
  EXPECT_EQ(handler_.inFlight(), 1u);
  (void)handler_.onRead(ctx_, erase_and_box(makeResponse()));
  EXPECT_EQ(handler_.inFlight(), 0u);

  // Responses are forwarded to the tail; no spurious deactivation while Open.
  EXPECT_EQ(ctx_.readMessages().size(), 2);
  EXPECT_EQ(ctx_.deactivateCount(), 0);
}

TEST_F(ThriftClientGracefulDrainHandlerTest, CloseWithInFlightEntersDraining) {
  (void)handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));
  ASSERT_EQ(handler_.inFlight(), 1u);

  handler_.onEvent(
      ctx_, ThriftClientEventType::CloseConnection, makeCloseEvent());

  // In-flight work remains; we drain rather than close immediately.
  EXPECT_TRUE(handler_.isDraining());
  EXPECT_FALSE(handler_.isClosed());
  EXPECT_EQ(ctx_.deactivateCount(), 0);
}

TEST_F(
    ThriftClientGracefulDrainHandlerTest,
    CloseWithNoInFlightDeactivatesImmediately) {
  handler_.onEvent(
      ctx_, ThriftClientEventType::CloseConnection, makeCloseEvent());

  // Nothing to wait for — close immediately.
  EXPECT_TRUE(handler_.isClosed());
  EXPECT_FALSE(handler_.isDraining());
  EXPECT_EQ(ctx_.deactivateCount(), 1);
}

TEST_F(ThriftClientGracefulDrainHandlerTest, DrainCompletesOnLastResponse) {
  (void)handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));
  (void)handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));
  handler_.onEvent(
      ctx_, ThriftClientEventType::CloseConnection, makeCloseEvent());
  ASSERT_TRUE(handler_.isDraining());

  // First response: still one in flight, no close yet.
  (void)handler_.onRead(ctx_, erase_and_box(makeResponse()));
  EXPECT_TRUE(handler_.isDraining());
  EXPECT_EQ(ctx_.deactivateCount(), 0);

  // Last response drains the connection: close + deactivate.
  (void)handler_.onRead(ctx_, erase_and_box(makeResponse()));
  EXPECT_TRUE(handler_.isClosed());
  EXPECT_EQ(handler_.inFlight(), 0u);
  EXPECT_EQ(ctx_.deactivateCount(), 1);
}

TEST_F(ThriftClientGracefulDrainHandlerTest, WriteRejectedWhenDraining) {
  // Enter draining with one request still in flight so we stay in Closing.
  (void)handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));
  handler_.onEvent(
      ctx_, ThriftClientEventType::CloseConnection, makeCloseEvent());
  ASSERT_TRUE(handler_.isDraining());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  void* const kHandle = reinterpret_cast<void*>(0x1234);
  auto result = handler_.onWrite(ctx_, erase_and_box(makeRequest(kHandle)));

  // Request is rejected, not forwarded: no new downstream write, one inbound
  // failure carrying the request's context and a retryable NOT_OPEN. The
  // reject does not count toward in-flight.
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(handler_.inFlight(), 1u);

  auto& resp = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  ASSERT_TRUE(resp.payload.is<ThriftClientResponseError>());
  EXPECT_EQ(resp.requestContext.get(), kHandle);

  auto* tex =
      resp.payload.get<ThriftClientResponseError>()
          .ew.get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(tex, nullptr);
  EXPECT_EQ(
      tex->getType(), apache::thrift::transport::TTransportException::NOT_OPEN);
  EXPECT_NE(std::string(tex->what()).find("draining"), std::string::npos);
}

TEST_F(ThriftClientGracefulDrainHandlerTest, PipelineInactiveSetsClosed) {
  // External teardown (peer FIN / transport error) closes us directly.
  handler_.onPipelineInactive(ctx_);
  EXPECT_TRUE(handler_.isClosed());
}

TEST_F(ThriftClientGracefulDrainHandlerTest, PipelineActiveResetsState) {
  (void)handler_.onWrite(ctx_, erase_and_box(makeRequest(nullptr)));
  handler_.onEvent(
      ctx_, ThriftClientEventType::CloseConnection, makeCloseEvent());
  ASSERT_TRUE(handler_.isDraining());

  // A reconnect re-activates the pipeline and clears drain state.
  handler_.onPipelineActive(ctx_);
  EXPECT_FALSE(handler_.isDraining());
  EXPECT_FALSE(handler_.isClosed());
  EXPECT_EQ(handler_.inFlight(), 0u);
}

} // namespace apache::thrift::fast_thrift::thrift::client::handler
