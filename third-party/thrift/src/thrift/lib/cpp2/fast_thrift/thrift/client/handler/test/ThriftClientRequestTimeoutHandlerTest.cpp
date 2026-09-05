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

#include <memory>
#include <vector>

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/RpcTransportStats.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ThriftRequestContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientRequestTimeoutHandler.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
namespace rocket = apache::thrift::fast_thrift::rocket;

namespace {

// Records what the handler forwards toward head (write) and tail (read), and
// exposes the fixture's EventBase so the handler can arm its HHWheelTimer.
class MockContext {
 public:
  explicit MockContext(folly::EventBase* evb) noexcept : evb_(evb) {}

  folly::EventBase* eventBase() const noexcept { return evb_; }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return Result::Success;
  }
  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }
  void fireException(folly::exception_wrapper&& e) noexcept {
    exceptions_.push_back(std::move(e));
  }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }
  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

 private:
  folly::EventBase* evb_;
  std::vector<TypeErasedBox> writeMessages_;
  std::vector<TypeErasedBox> readMessages_;
  std::vector<folly::exception_wrapper> exceptions_;
};

// Captures the fate of the caller's response handler.
struct CallerResult {
  int calls{0};
  bool failed{false};
  folly::exception_wrapper ew;
};

RequestResponseHandler makeCaptureHandler(CallerResult* out) {
  return RequestResponseHandler{
      [out](
          folly::Expected<
              apache::thrift::fast_thrift::thrift::client::FastResponse,
              folly::exception_wrapper>&& result,
          const apache::thrift::RpcTransportStats&) noexcept {
        out->calls++;
        out->failed = !result.hasValue();
        if (out->failed) {
          out->ew = result.error();
        }
      }};
}

// Builds a request whose context captures into `out`, arming the timer when
// `timeoutMs > 0`. The raw context is returned via `rcOut` so the test can
// build a matching inbound response.
ThriftRequestMessage makeRequest(
    int32_t timeoutMs, CallerResult* out, ThriftRequestContext** rcOut) {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  if (timeoutMs > 0) {
    metadata->clientTimeoutMs() = timeoutMs;
  }
  auto rc = std::make_unique<ThriftRequestContext>(makeCaptureHandler(out));
  *rcOut = rc.get();
  return ThriftRequestMessage{
      .payload =
          ThriftRequestResponsePayload{
              .data = folly::IOBuf::copyBuffer("req"),
              .metadata = std::move(metadata),
          },
      .requestContext = rocket::from_unique_ptr(std::move(rc)),
  };
}

// Inbound response borrowing the same context (the request box retains
// ownership), so the handler can look it up to disarm/drop.
ThriftResponseMessage makeResponse(ThriftRequestContext* rc) {
  ThriftResponseMessage response;
  response.requestContext = rocket::borrow(rc);
  return response;
}

bool isTimedOut(const folly::exception_wrapper& ew) {
  const auto* tex =
      ew.get_exception<apache::thrift::transport::TTransportException>();
  return tex != nullptr &&
      tex->getType() ==
      apache::thrift::transport::TTransportException::TIMED_OUT;
}

} // namespace

class ThriftClientRequestTimeoutHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evb_ = std::make_unique<folly::EventBase>();
    ctx_ = std::make_unique<MockContext>(evb_.get());
  }

  std::unique_ptr<folly::EventBase> evb_;
  std::unique_ptr<MockContext> ctx_;
  ThriftClientRequestTimeoutHandler handler_;
};

TEST_F(ThriftClientRequestTimeoutHandlerTest, FiresTimedOutWhenNoResponse) {
  CallerResult caller;
  ThriftRequestContext* rc = nullptr;
  auto request = makeRequest(/*timeoutMs=*/5, &caller, &rc);

  EXPECT_EQ(
      handler_.onWrite(*ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_EQ(ctx_->writeMessages().size(), 1u); // request forwarded downstream
  EXPECT_EQ(caller.calls, 0); // not yet expired

  evb_->loop(); // let the timer fire

  EXPECT_EQ(caller.calls, 1);
  EXPECT_TRUE(caller.failed);
  EXPECT_TRUE(isTimedOut(caller.ew));
}

TEST_F(ThriftClientRequestTimeoutHandlerTest, ResponseDisarmsTimeout) {
  CallerResult caller;
  ThriftRequestContext* rc = nullptr;
  auto request = makeRequest(/*timeoutMs=*/5, &caller, &rc);
  (void)handler_.onWrite(*ctx_, erase_and_box(std::move(request)));

  EXPECT_EQ(
      handler_.onRead(*ctx_, erase_and_box(makeResponse(rc))), Result::Success);
  EXPECT_EQ(ctx_->readMessages().size(), 1u); // response forwarded to tail

  evb_->loop(); // timer was cancelled, nothing should fire

  EXPECT_EQ(caller.calls, 0);
}

TEST_F(ThriftClientRequestTimeoutHandlerTest, LateResponseDroppedAfterTimeout) {
  CallerResult caller;
  ThriftRequestContext* rc = nullptr;
  auto request = makeRequest(/*timeoutMs=*/5, &caller, &rc);
  (void)handler_.onWrite(*ctx_, erase_and_box(std::move(request)));

  evb_->loop(); // fire the timeout
  ASSERT_EQ(caller.calls, 1);
  ASSERT_TRUE(caller.failed);

  // A late response for the already-failed request is swallowed, not
  // forwarded, and the caller is not invoked again.
  EXPECT_EQ(
      handler_.onRead(*ctx_, erase_and_box(makeResponse(rc))), Result::Success);
  EXPECT_TRUE(ctx_->readMessages().empty());
  EXPECT_EQ(caller.calls, 1);
}

TEST_F(ThriftClientRequestTimeoutHandlerTest, NoTimeoutIsNotArmed) {
  CallerResult caller;
  ThriftRequestContext* rc = nullptr;
  auto request = makeRequest(/*timeoutMs=*/0, &caller, &rc);

  EXPECT_EQ(
      handler_.onWrite(*ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_EQ(ctx_->writeMessages().size(), 1u);

  evb_->loop();

  EXPECT_EQ(caller.calls, 0);
}

TEST_F(
    ThriftClientRequestTimeoutHandlerTest,
    DefaultTimeoutBoundsARequestThatCarriesNone) {
  CallerResult caller;
  ThriftRequestContext* rc = nullptr;
  auto request = makeRequest(/*timeoutMs=*/0, &caller, &rc);

  ThriftClientRequestTimeoutHandler handler{std::chrono::milliseconds(5)};
  EXPECT_EQ(
      handler.onWrite(*ctx_, erase_and_box(std::move(request))),
      Result::Success);

  evb_->loop();

  EXPECT_EQ(caller.calls, 1);
  EXPECT_TRUE(isTimedOut(caller.ew));
}

TEST_F(
    ThriftClientRequestTimeoutHandlerTest,
    PerRequestTimeoutWinsOverTheDefault) {
  // A request that names its own deadline must not be stretched to the
  // connection default. The margin between the two is what makes this
  // assertable: firing on the request's 1ms rather than the default's 10s.
  CallerResult caller;
  ThriftRequestContext* rc = nullptr;
  auto request = makeRequest(/*timeoutMs=*/1, &caller, &rc);

  ThriftClientRequestTimeoutHandler handler{std::chrono::seconds(10)};
  const auto start = std::chrono::steady_clock::now();
  (void)handler.onWrite(*ctx_, erase_and_box(std::move(request)));

  evb_->loop();
  const auto elapsed = std::chrono::steady_clock::now() - start;

  EXPECT_EQ(caller.calls, 1);
  EXPECT_TRUE(isTimedOut(caller.ew));
  EXPECT_LT(elapsed, std::chrono::seconds(5));
}

TEST_F(
    ThriftClientRequestTimeoutHandlerTest, ContextDestructionCancelsTimeout) {
  CallerResult caller;
  ThriftRequestContext* rc = nullptr;
  auto request = makeRequest(/*timeoutMs=*/5, &caller, &rc);
  (void)handler_.onWrite(*ctx_, erase_and_box(std::move(request)));

  // Destroying the request (and thus the context) must cancel the armed timer.
  ctx_->writeMessages().clear();

  evb_->loop();

  EXPECT_EQ(caller.calls, 0);
}

} // namespace apache::thrift::fast_thrift::thrift::client::handler
