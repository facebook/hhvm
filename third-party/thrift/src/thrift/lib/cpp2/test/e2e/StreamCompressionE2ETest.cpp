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

#include <fmt/core.h>
#include <folly/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestStreamE2EService.h>

THRIFT_FLAG_DECLARE_bool(thrift_server_compress_response_on_cpu);

using namespace ::testing;

namespace apache::thrift {

namespace {

class StreamCompressionE2ETest : public test::E2ETestFixture {};

// Verifies that a basic stream round-trips correctly when CPU-thread
// compression is enabled with ZSTD. The flag gates both the CPU-side
// (ServerStreamFactory compresses items) and IO-side
// (RocketStreamClientCallback bypasses applyCompressionConfigIfNeeded). The
// client decompresses transparently.
CO_TEST_F(StreamCompressionE2ETest, BasicStreamWithCpuCompression) {
  THRIFT_FLAG_SET_MOCK(thrift_server_compress_response_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<std::string> stringRange(int32_t from, int32_t to) override {
      for (int32_t i = from; i <= to; ++i) {
        co_yield std::string(256, 'a' + (i % 26));
      }
    }
  };

  testConfig(
      {std::make_shared<Handler>(),
       /* serverConfigCb */ {},
       [](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
         auto channel = RocketClientChannel::newChannel(std::move(socket));
         CompressionConfig compressionConfig;
         compressionConfig.codecConfig().ensure().set_zstdConfig();
         channel->setDesiredCompressionConfig(compressionConfig);
         return channel;
       }});

  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_stringRange(0, 9)).toAsyncGenerator();

  for (int32_t expected = 0; expected <= 9; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, std::string(256, 'a' + (expected % 26)));
  }
  EXPECT_FALSE((co_await gen.next()).has_value());
}

// Verifies that a stream with a first response also works correctly with
// CPU-thread compression enabled.
CO_TEST_F(StreamCompressionE2ETest, StreamWithResponseAndCpuCompression) {
  THRIFT_FLAG_SET_MOCK(thrift_server_compress_response_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ResponseAndServerStream<std::string, std::string>
    sync_stringRangeWithResponse(int32_t from, int32_t to) override {
      auto stream = folly::coro::co_invoke(
          [from, to]() -> folly::coro::AsyncGenerator<std::string&&> {
            for (int32_t i = from; i <= to; ++i) {
              co_yield std::string(256, 'a' + (i % 26));
            }
          });
      return {fmt::format("count:{}", to - from + 1), std::move(stream)};
    }
  };

  testConfig(
      {std::make_shared<Handler>(),
       /* serverConfigCb */ {},
       [](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
         auto channel = RocketClientChannel::newChannel(std::move(socket));
         CompressionConfig compressionConfig;
         compressionConfig.codecConfig().ensure().set_zstdConfig();
         channel->setDesiredCompressionConfig(compressionConfig);
         return channel;
       }});

  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto result = co_await client->co_stringRangeWithResponse(0, 4);
  EXPECT_EQ(result.response, "count:5");

  auto gen = std::move(result.stream).toAsyncGenerator();
  for (int32_t expected = 0; expected <= 4; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, std::string(256, 'a' + (expected % 26)));
  }
  EXPECT_FALSE((co_await gen.next()).has_value());
}

} // namespace

} // namespace apache::thrift
