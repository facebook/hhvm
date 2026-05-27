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

#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketBiDiServerCallback.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>

using namespace apache::thrift;
using namespace apache::thrift::rocket;

namespace {

// Minimal mock of BiDiClientCallback. Only stream-side methods are exercised.
class MockBiDiClientCallback final : public BiDiClientCallback {
 public:
  bool onFirstResponse(
      FirstResponsePayload&&, folly::EventBase*, BiDiServerCallback*) override {
    return true;
  }
  void onFirstResponseError(folly::exception_wrapper) override {}

  bool onStreamNext(StreamPayload&&) override { return true; }
  bool onStreamError(folly::exception_wrapper) override {
    streamErrorCount_++;
    return true;
  }
  bool onStreamComplete() override {
    streamCompleteCount_++;
    return true;
  }

  bool onSinkRequestN(int32_t) override { return true; }
  bool onSinkCancel() override { return true; }

  void resetServerCallback(BiDiServerCallback&) override {}

  int streamCompleteCount() const { return streamCompleteCount_; }
  int streamErrorCount() const { return streamErrorCount_; }

 private:
  int streamCompleteCount_{0};
  int streamErrorCount_{0};
};

} // namespace

// Verify that calling onStreamComplete() twice (duplicate stream-complete frame
// from server) does not crash. The first call transitions the state from
// StreamAndSinkOpen to OnlySinkOpen. The second call should be a no-op.
TEST(RocketBiDiServerCallbackTest, DuplicateStreamCompleteDoesNotCrash) {
  MockBiDiClientCallback clientCallback;

  // RocketBiDiServerCallback requires a RocketClient& but onStreamComplete()
  // never accesses it. Use a dummy reference.
  alignas(RocketClient) char storage[sizeof(RocketClient)];
  auto& dummyClient = reinterpret_cast<RocketClient&>(storage);

  RocketBiDiServerCallback serverCallback(
      StreamId{1}, dummyClient, clientCallback, nullptr);

  // Transition to StreamAndSinkOpen.
  serverCallback.state().onFirstResponseSent();
  ASSERT_TRUE(serverCallback.state().isBothOpen());

  // First stream complete: StreamAndSinkOpen -> OnlySinkOpen.
  bool alive = serverCallback.onStreamComplete();
  EXPECT_TRUE(alive); // Sink is still open.
  EXPECT_FALSE(serverCallback.state().isStreamOpen());
  EXPECT_TRUE(serverCallback.state().isSinkOpen());
  EXPECT_EQ(clientCallback.streamCompleteCount(), 1);

  // Second stream complete: should be a no-op (not crash).
  alive = serverCallback.onStreamComplete();
  EXPECT_TRUE(alive); // Sink is still open.
  // clientCallback should NOT have received a second onStreamComplete.
  EXPECT_EQ(clientCallback.streamCompleteCount(), 1);
}

// Regression test for a crash in dvaar.mysql (SIGSEGV in
// ClientCallbackStapler::onStreamError). When the client cancels the stream
// half of a BiDi connection, a late ERROR frame from the server can still reach
// onStreamError because the stream entry remains in RocketClient::streams_
// (the sink half is alive). Without the isStreamOpen() guard, the error
// propagates to ClientCallbackStapler which dereferences null.
TEST(RocketBiDiServerCallbackTest, StreamErrorAfterStreamCancelDoesNotCrash) {
  MockBiDiClientCallback clientCallback;

  alignas(RocketClient) char storage[sizeof(RocketClient)];
  auto& dummyClient = reinterpret_cast<RocketClient&>(storage);

  RocketBiDiServerCallback serverCallback(
      StreamId{1}, dummyClient, clientCallback, nullptr);

  // Transition to StreamAndSinkOpen.
  serverCallback.state().onFirstResponseSent();
  ASSERT_TRUE(serverCallback.state().isBothOpen());

  // Cancel the stream: StreamAndSinkOpen -> OnlySinkOpen.
  serverCallback.state().onStreamCancel();
  EXPECT_FALSE(serverCallback.state().isStreamOpen());
  EXPECT_TRUE(serverCallback.state().isSinkOpen());

  // A late stream error arrives after the stream is already cancelled.
  // This must be a no-op, not a crash.
  serverCallback.onStreamError(
      folly::make_exception_wrapper<std::runtime_error>("late error"));

  // The error should NOT have been forwarded to the client callback.
  EXPECT_EQ(clientCallback.streamErrorCount(), 0);
  EXPECT_TRUE(serverCallback.state().isSinkOpen());
}

TEST(RocketBiDiServerCallbackTest, StreamErrorAfterStreamErrorDoesNotCrash) {
  MockBiDiClientCallback clientCallback;

  alignas(RocketClient) char storage[sizeof(RocketClient)];
  auto& dummyClient = reinterpret_cast<RocketClient&>(storage);

  RocketBiDiServerCallback serverCallback(
      StreamId{1}, dummyClient, clientCallback, nullptr);

  serverCallback.state().onFirstResponseSent();
  ASSERT_TRUE(serverCallback.state().isBothOpen());

  // First stream error: StreamAndSinkOpen -> OnlySinkOpen.
  serverCallback.onStreamError(
      folly::make_exception_wrapper<std::runtime_error>("first error"));
  EXPECT_EQ(clientCallback.streamErrorCount(), 1);
  EXPECT_FALSE(serverCallback.state().isStreamOpen());
  EXPECT_TRUE(serverCallback.state().isSinkOpen());

  // Second stream error: should be a no-op (not crash).
  serverCallback.onStreamError(
      folly::make_exception_wrapper<std::runtime_error>("second error"));
  EXPECT_EQ(clientCallback.streamErrorCount(), 1);
}
