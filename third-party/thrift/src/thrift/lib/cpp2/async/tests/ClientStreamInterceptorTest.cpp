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

#include <thrift/lib/cpp/StreamEventHandler.h>
#include <thrift/lib/cpp2/async/ClientInterceptor.h>
#include <thrift/lib/cpp2/async/ClientStreamInterceptorContext.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace apache::thrift;
using namespace ::testing;

namespace {

struct TestRequestState {
  std::string methodName;
  int payloadCount = 0; // Tracks payload count through stream lifetime
};

// Test interceptor that records all stream lifecycle events
class RecordingInterceptor : public ClientInterceptor<TestRequestState> {
 public:
  std::string getName() const override { return "RecordingInterceptor"; }

  std::optional<TestRequestState> onRequest(RequestInfo info) override {
    return TestRequestState{std::string(info.methodName), 0};
  }

  void onResponse(TestRequestState*, ResponseInfo) override {}

  void onStreamBegin(TestRequestState* reqState) override {
    streamBeginCalls++;
    if (reqState) {
      lastRequestMethod = reqState->methodName;
    }
  }

  void onStreamPayload(
      TestRequestState* reqState, StreamPayloadInfo info) override {
    streamPayloadCalls++;
    lastPayloadIndex = info.payloadIndex;
    if (reqState) {
      reqState->payloadCount++;
    }
  }

  void onStreamEnd(
      TestRequestState* reqState, const StreamEndInfo& info) noexcept override {
    streamEndCalls++;
    lastEndReason = info.endReason;
    lastTotalPayloads = info.totalPayloads;
    if (reqState) {
      finalPayloadCount = reqState->payloadCount;
    }
  }

  // Counters for verification
  int streamBeginCalls = 0;
  int streamPayloadCalls = 0;
  int streamEndCalls = 0;
  std::string lastRequestMethod;
  std::size_t lastPayloadIndex = 0;
  details::STREAM_ENDING_TYPES lastEndReason =
      details::STREAM_ENDING_TYPES::COMPLETE;
  std::size_t lastTotalPayloads = 0;
  int finalPayloadCount = 0;
};

} // namespace

// Tests for ClientStreamInterceptorContext

TEST(ClientStreamInterceptorContextTest, BasicLifecycle) {
  auto interceptor = std::make_shared<RecordingInterceptor>();
  auto interceptors =
      std::make_shared<ClientStreamInterceptorContext::InterceptorList>(
          ClientStreamInterceptorContext::InterceptorList{interceptor});
  std::vector<detail::ClientInterceptorOnRequestStorage> requestStorages(1);

  auto ctx = std::make_shared<ClientStreamInterceptorContext>(
      std::move(interceptors), std::move(requestStorages));

  EXPECT_FALSE(ctx->hasEnded());
  EXPECT_EQ(ctx->payloadCount(), 0);

  ctx->onStreamBegin();
  EXPECT_EQ(interceptor->streamBeginCalls, 1);

  std::string payload1 = "payload1";
  ctx->onStreamPayload(payload1);
  EXPECT_EQ(interceptor->streamPayloadCalls, 1);
  EXPECT_EQ(interceptor->lastPayloadIndex, 0);
  EXPECT_EQ(ctx->payloadCount(), 1);

  std::string payload2 = "payload2";
  ctx->onStreamPayload(payload2);
  EXPECT_EQ(interceptor->streamPayloadCalls, 2);
  EXPECT_EQ(interceptor->lastPayloadIndex, 1);
  EXPECT_EQ(ctx->payloadCount(), 2);

  ctx->onStreamEnd(details::STREAM_ENDING_TYPES::COMPLETE);
  EXPECT_TRUE(ctx->hasEnded());
  EXPECT_EQ(interceptor->streamEndCalls, 1);
  EXPECT_EQ(interceptor->lastEndReason, details::STREAM_ENDING_TYPES::COMPLETE);
  EXPECT_EQ(interceptor->lastTotalPayloads, 2);
}

TEST(ClientStreamInterceptorContextTest, MultipleInterceptorsForwardOrder) {
  auto interceptor1 = std::make_shared<RecordingInterceptor>();
  auto interceptor2 = std::make_shared<RecordingInterceptor>();
  auto interceptors =
      std::make_shared<ClientStreamInterceptorContext::InterceptorList>(
          ClientStreamInterceptorContext::InterceptorList{
              interceptor1, interceptor2});
  std::vector<detail::ClientInterceptorOnRequestStorage> requestStorages(2);

  auto ctx = std::make_shared<ClientStreamInterceptorContext>(
      std::move(interceptors), std::move(requestStorages));

  ctx->onStreamBegin();
  EXPECT_EQ(interceptor1->streamBeginCalls, 1);
  EXPECT_EQ(interceptor2->streamBeginCalls, 1);

  std::string payload = "test";
  ctx->onStreamPayload(payload);
  EXPECT_EQ(interceptor1->streamPayloadCalls, 1);
  EXPECT_EQ(interceptor2->streamPayloadCalls, 1);

  ctx->onStreamEnd(details::STREAM_ENDING_TYPES::COMPLETE);
  EXPECT_EQ(interceptor1->streamEndCalls, 1);
  EXPECT_EQ(interceptor2->streamEndCalls, 1);
}

TEST(ClientStreamInterceptorContextTest, DestructorCallsOnStreamEnd) {
  auto interceptor = std::make_shared<RecordingInterceptor>();
  auto interceptors =
      std::make_shared<ClientStreamInterceptorContext::InterceptorList>(
          ClientStreamInterceptorContext::InterceptorList{interceptor});
  std::vector<detail::ClientInterceptorOnRequestStorage> requestStorages(1);

  {
    auto ctx = std::make_shared<ClientStreamInterceptorContext>(
        std::move(interceptors), std::move(requestStorages));
    ctx->onStreamBegin();
    // Don't call onStreamEnd - destructor should handle it
  }

  EXPECT_EQ(interceptor->streamEndCalls, 1);
  EXPECT_EQ(interceptor->lastEndReason, details::STREAM_ENDING_TYPES::CANCEL);
}

TEST(ClientStreamInterceptorContextTest, OnStreamEndIdempotent) {
  auto interceptor = std::make_shared<RecordingInterceptor>();
  auto interceptors =
      std::make_shared<ClientStreamInterceptorContext::InterceptorList>(
          ClientStreamInterceptorContext::InterceptorList{interceptor});
  std::vector<detail::ClientInterceptorOnRequestStorage> requestStorages(1);

  auto ctx = std::make_shared<ClientStreamInterceptorContext>(
      std::move(interceptors), std::move(requestStorages));

  ctx->onStreamBegin();
  ctx->onStreamEnd(details::STREAM_ENDING_TYPES::COMPLETE);
  EXPECT_EQ(interceptor->streamEndCalls, 1);

  // Second call should be a no-op
  ctx->onStreamEnd(details::STREAM_ENDING_TYPES::ERROR);
  EXPECT_EQ(interceptor->streamEndCalls, 1);
  EXPECT_EQ(interceptor->lastEndReason, details::STREAM_ENDING_TYPES::COMPLETE);
}
