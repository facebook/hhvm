/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Singleton.h>
#include <folly/io/async/EventBase.h>
#include <proxygen/lib/http/webtransport/test/Mocks.h>

namespace proxygen::test {

TEST(WebTransportTest, ErrorCodes) {
  EXPECT_EQ(WebTransport::toHTTPErrorCode(0), WebTransport::kFirstErrorCode);
  EXPECT_EQ(WebTransport::toHTTPErrorCode(std::numeric_limits<uint32_t>::max()),
            WebTransport::kLastErrorCode);
  EXPECT_EQ(WebTransport::toApplicationErrorCode(0).error(),
            WebTransport::ErrorCode::GENERIC_ERROR);
  EXPECT_EQ(WebTransport::toApplicationErrorCode(WebTransport::kFirstErrorCode)
                .value(),
            0);
  EXPECT_EQ(WebTransport::toApplicationErrorCode(WebTransport::kLastErrorCode)
                .value(),
            std::numeric_limits<uint32_t>::max());
}

TEST(WebTransportTest, AwaitNextRead) {
  folly::SingletonVault::singleton()->registrationComplete();
  folly::EventBase evb;
  MockStreamReadHandle readHandle(0);
  EXPECT_CALL(readHandle, readStreamData()).WillOnce(testing::Invoke([] {
    return folly::makeFuture(WebTransport::StreamData({nullptr, true}));
  }));

  readHandle.awaitNextRead(
      &evb,
      [&](WebTransport::StreamReadHandle* handle,
          folly::Try<WebTransport::StreamData> streamData) {
        EXPECT_EQ(handle, &readHandle);
        EXPECT_EQ(streamData.value().data, nullptr);
        EXPECT_TRUE(streamData.value().fin);
      });
  evb.loopOnce();
}

TEST(WebTransportTest, AwaitNextReadTimeout) {
  folly::SingletonVault::singleton()->registrationComplete();
  folly::EventBase evb;
  MockStreamReadHandle readHandle(0);
  auto contract = folly::makePromiseContract<WebTransport::StreamData>();
  EXPECT_CALL(readHandle, readStreamData()).WillOnce(testing::Invoke([&] {
    return std::move(contract.second);
  }));

  readHandle.awaitNextRead(
      &evb,
      [&](WebTransport::StreamReadHandle* handle,
          folly::Try<WebTransport::StreamData> streamData) {
        EXPECT_EQ(handle, &readHandle);
        EXPECT_NE(streamData.tryGetExceptionObject<folly::FutureException>(),
                  nullptr);
        // The promise core holds a keep alive to the event base, kill it here
        auto deadPromise = std::move(contract.first);
      },
      std::chrono::milliseconds(100));
  evb.loop();
}

} // namespace proxygen::test
