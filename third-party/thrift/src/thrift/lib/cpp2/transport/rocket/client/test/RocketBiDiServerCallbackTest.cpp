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
  bool onStreamError(folly::exception_wrapper) override { return true; }
  bool onStreamComplete() override {
    streamCompleteCount_++;
    return true;
  }

  bool onSinkRequestN(int32_t) override { return true; }
  bool onSinkCancel() override { return true; }

  void resetServerCallback(BiDiServerCallback&) override {}

  int streamCompleteCount() const { return streamCompleteCount_; }

 private:
  int streamCompleteCount_{0};
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
