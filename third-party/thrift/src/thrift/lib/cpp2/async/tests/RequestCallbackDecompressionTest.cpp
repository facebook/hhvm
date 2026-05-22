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

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/FutureRequest.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>

using namespace apache::thrift;
using namespace apache::thrift::transport;

namespace thrift_detail = apache::thrift::detail;

namespace {

folly::exception_wrapper noopProcessor(ClientReceiveState&) {
  return {};
}

} // namespace

// Exercises the new state.isException() guard in RequestCallback::onResponse().
//
// Flow being tested (the fix path):
//   1. onResponse() calls responseDecompressor(state)
//   2. Decompressor fails → sets state.exception() with TTransportException
//   3. onResponse() checks state.isException() → TRUE
//   4. Routes to requestError(state) instead of replyReceived(state)
//   5. FutureCallbackBase::requestError() fulfills the promise with an error
//
// Before the fix, step 3 did not exist: onResponse() fell through to
// replyReceived() unconditionally, hitting CHECK(!state.isException()) in
// FutureCallback::replyReceived() (FutureRequest.h:163) → FATAL crash.
TEST(
    RequestCallbackDecompressionTest, DecompressorFailureRoutesToRequestError) {
  using Helper = thrift_detail::FutureCallbackHelper<folly::Unit>;
  folly::Promise<Helper::PromiseResult> promise;
  auto future = promise.getSemiFuture();

  // FutureCallback has CHECK(!state.isException()) in replyReceived() —
  // this is the callback type that crashed before the fix.
  auto callback = std::make_unique<FutureCallback<folly::Unit>>(
      std::move(promise), noopProcessor);

  RequestCallback::Context ctx;
  ctx.oneWay = false;
  ctx.protocolId = 0;
  // Simulate what happens when decompressResponse() catches a decompression
  // failure: it sets state.exception() rather than rethrowing (because
  // onResponse is noexcept).
  ctx.responseDecompressor = [](ClientReceiveState& state) {
    state.exception() = folly::make_exception_wrapper<TTransportException>(
        TTransportException::CORRUPTED_DATA,
        "Response decompression failed: simulated");
  };

  auto cbPtr = toRequestClientCallbackPtr(std::move(callback), std::move(ctx));
  auto* rawCb = cbPtr.release();

  // Build a valid (non-exception) ClientReceiveState with a response buffer.
  // This mirrors a normal response from the transport before decompression.
  auto header = std::make_unique<THeader>();
  SerializedResponse response(folly::IOBuf::copyBuffer("test"));
  ClientReceiveState state(
      0,
      MessageType::T_REPLY,
      std::move(response),
      std::move(header),
      nullptr,
      RpcTransportStats{});

  // This would FATAL before the fix (CHECK failure in replyReceived).
  // After the fix, the exception state is detected and routed to requestError.
  rawCb->onResponse(std::move(state));

  // Verify the promise was fulfilled via requestError() (error path),
  // not replyReceived() (which would have crashed).
  auto result = std::move(future).get();
  ASSERT_TRUE(result.hasError());
  auto& [ew, recvState] = result.error();
  EXPECT_TRUE(ew.is_compatible_with<TTransportException>());
  ew.with_exception([](const TTransportException& tex) {
    EXPECT_EQ(tex.getType(), TTransportException::CORRUPTED_DATA);
    EXPECT_NE(
        std::string(tex.what()).find("decompression failed"),
        std::string::npos);
  });
}

// Exercises the existing replyReceived() path through the new guard.
//
// Flow being tested (the normal path, unchanged by the fix):
//   1. onResponse() calls responseDecompressor(state)
//   2. Decompressor succeeds → state.isException() remains false
//   3. onResponse() checks state.isException() → FALSE
//   4. Routes to replyReceived(state) as before
//   5. FutureCallback::replyReceived() processes the response normally
//
// This ensures the new state.isException() guard does not regress the
// success path — decompression that succeeds must still reach replyReceived().
TEST(
    RequestCallbackDecompressionTest,
    SuccessfulDecompressionRoutesToReplyReceived) {
  using Helper = thrift_detail::FutureCallbackHelper<folly::Unit>;
  folly::Promise<Helper::PromiseResult> promise;
  auto future = promise.getSemiFuture();

  auto callback = std::make_unique<FutureCallback<folly::Unit>>(
      std::move(promise), noopProcessor);

  RequestCallback::Context ctx;
  ctx.oneWay = false;
  ctx.protocolId = 0;
  // No-op decompressor: simulates successful decompression (no exception set).
  ctx.responseDecompressor = [](ClientReceiveState& /*state*/) {};

  auto cbPtr = toRequestClientCallbackPtr(std::move(callback), std::move(ctx));
  auto* rawCb = cbPtr.release();

  auto header = std::make_unique<THeader>();
  SerializedResponse response(folly::IOBuf::copyBuffer("test"));
  ClientReceiveState state(
      0,
      MessageType::T_REPLY,
      std::move(response),
      std::move(header),
      nullptr,
      RpcTransportStats{});

  rawCb->onResponse(std::move(state));

  // Verify the promise was fulfilled via replyReceived() (success path).
  auto result = std::move(future).get();
  EXPECT_TRUE(result.hasValue());
}
