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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/coro/GtestHelpers.h>

#include <thrift/lib/cpp2/async/ClientInterceptor.h>
#include <thrift/lib/cpp2/async/tests/ClientInterceptorTest.cpp>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace ::testing;

// Test that the result field in responseInfo contains the correct type and
// value for streams without initial response
CO_TEST_P(ClientInterceptorTestP, StreamResponseInfoTypeAndValue) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support streaming
    co_return;
  }

  // Capture information about the response
  struct StreamInfo {
    std::string methodName;
    std::string typeName;
    bool hasValue = false;
    bool hasException = false;
  };

  StreamInfo streamInfo;
  auto interceptor = std::make_shared<ClientInterceptorOnResponse>(
      "StreamResponseInterceptor", [&streamInfo](ResponseInfo responseInfo) {
        streamInfo.methodName = std::string(responseInfo.methodName);
        streamInfo.hasException = responseInfo.result.hasException();

        if (!streamInfo.hasException && responseInfo.result.hasValue()) {
          streamInfo.typeName = folly::demangle(responseInfo.result->type());
          streamInfo.hasValue = true;
        }
      });

  auto client = makeClient(makeInterceptorsList(interceptor));

  // Test stream without initial response
  {
    auto stream = co_await client->iota(1);

    // Verify the interceptor captured the response info correctly
    EXPECT_EQ(streamInfo.methodName, "iota");
    EXPECT_FALSE(streamInfo.hasException);
    EXPECT_TRUE(streamInfo.hasValue);

    // The result should be a ServerStream<int32_t>
    EXPECT_THAT(streamInfo.typeName, HasSubstr("ServerStream"));
    EXPECT_THAT(streamInfo.typeName, HasSubstr("int32_t"));

    // Consume some values from the stream to verify it works
    EXPECT_EQ((co_await stream.next()).value(), 1);
    EXPECT_EQ((co_await stream.next()).value(), 2);
  }

  // Test stream with initial response
  {
    // Reset captured values
    streamInfo = StreamInfo();

    auto stream = co_await client->iotaWithResponse(10);

    // Verify the interceptor captured the response info correctly
    EXPECT_EQ(streamInfo.methodName, "iotaWithResponse");
    EXPECT_FALSE(streamInfo.hasException);
    EXPECT_TRUE(streamInfo.hasValue);

    // The result should be a ServerStream<int32_t>
    EXPECT_THAT(streamInfo.typeName, HasSubstr("ServerStream"));
    EXPECT_THAT(streamInfo.typeName, HasSubstr("int32_t"));

    // Consume some values from the stream to verify it works
    EXPECT_EQ((co_await stream.next()).value(), 10);
    EXPECT_EQ((co_await stream.next()).value(), 11);
  }
}

// Test that we can use makeResultCaptureInterceptor to capture stream objects
CO_TEST_P(ClientInterceptorTestP, StreamResultCapture) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support streaming
    co_return;
  }

  // Define a type to capture the stream
  struct StreamCapture {
    folly::coro::AsyncGenerator<std::int32_t&&> stream;
    bool hasValue = false;
  };

  // Create a capture object
  StreamCapture capture;

  // Create an interceptor that captures the stream
  auto interceptor = std::make_shared<ClientInterceptorOnResponse>(
      "StreamCaptureInterceptor", [&capture](ResponseInfo responseInfo) {
        if (responseInfo.result.hasValue()) {
          const auto& result = responseInfo.result.value();

          // Check if the result is a ServerStream<int32_t>
          if (folly::demangle(result.type()).find("ServerStream") !=
              std::string::npos) {
            // Extract the stream from the result
            auto serverStream =
                result.value<apache::thrift::ServerStream<std::int32_t>>();

            // Convert to AsyncGenerator and store
            capture.stream = serverStream.toAsyncGenerator();
            capture.hasValue = true;
          }
        }
      });

  auto client = makeClient(makeInterceptorsList(interceptor));

  // Test stream without initial response
  {
    auto stream = co_await client->iota(1);

    // Verify the interceptor captured the stream
    EXPECT_TRUE(capture.hasValue);

    // Consume some values from the original stream
    EXPECT_EQ((co_await stream.next()).value(), 1);
    EXPECT_EQ((co_await stream.next()).value(), 2);

    // Consume some values from the captured stream
    EXPECT_EQ((co_await capture.stream.next()).value(), 1);
    EXPECT_EQ((co_await capture.stream.next()).value(), 2);
  }

  // Test stream with initial response
  {
    // Reset captured values
    capture = StreamCapture();

    auto stream = co_await client->iotaWithResponse(10);

    // Verify the interceptor captured the stream
    EXPECT_TRUE(capture.hasValue);

    // Consume some values from the original stream
    EXPECT_EQ((co_await stream.next()).value(), 10);
    EXPECT_EQ((co_await stream.next()).value(), 11);

    // Consume some values from the captured stream
    EXPECT_EQ((co_await capture.stream.next()).value(), 10);
    EXPECT_EQ((co_await capture.stream.next()).value(), 11);
  }
}
