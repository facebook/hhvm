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

#include <folly/base64.h>
#include <folly/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestRequestResponseService.h>

THRIFT_FLAG_DECLARE_bool(thrift_client_compress_request_on_cpu);

using namespace ::testing;

namespace apache::thrift {

namespace {

class RequestResponseServiceE2ETest : public test::E2ETestFixture {};

RequestChannel::Ptr makeCompressedChannel(
    folly::AsyncSocket::UniquePtr socket) {
  auto channel = RocketClientChannel::newChannel(std::move(socket));
  CompressionConfig compressionConfig;
  compressionConfig.codecConfig().ensure().set_zstdConfig();
  channel->setDesiredCompressionConfig(compressionConfig);
  return channel;
}

} // namespace

CO_TEST_F(RequestResponseServiceE2ETest, BasicEcho) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  auto result = co_await client->co_echo("Hello");
  EXPECT_EQ(result, "Hello");
}

CO_TEST_F(RequestResponseServiceE2ETest, VoidReturn) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<void> co_voidMethod() override { co_return; }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  co_await client->co_voidMethod();
}

CO_TEST_F(RequestResponseServiceE2ETest, MethodThrowsDeclaredException) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<void> co_canThrow() override {
      throw detail::test::RRDeclaredException{"declared error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), detail::test::RRDeclaredException);
}

CO_TEST_F(RequestResponseServiceE2ETest, MethodThrowsUndeclaredException) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<void> co_canThrow() override {
      throw std::runtime_error{"undeclared error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), apache::thrift::TApplicationException);
}

CO_TEST_F(RequestResponseServiceE2ETest, MultipleSequentialRequests) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();

  constexpr int kNumRequests = 100;
  for (int i = 0; i < kNumRequests; ++i) {
    auto msg = fmt::format("request-{}", i);
    auto result = co_await client->co_echo(msg);
    EXPECT_EQ(result, msg);
  }
}

// Regression test for the unary `anyException` + caller-thread decompression
// truncation bug.
//
// When `thrift_client_compress_request_on_cpu` is set, the IO thread skips
// response decompression and leaves the payload compressed for the caller
// thread. But `processFirstResponse` eagerly deserializes an
// `anyException` SemiAnyStruct on the IO thread -- before the caller-thread
// decompression runs -- so it reads still-compressed bytes and aborts with
// "anyException deserialization failure: ... the data appears to be truncated".
//
// This handler mimics what SRProxy's `sendServiceRouterError` does: it stashes
// a (large, compressible) serialized payload in the anyex/anyext response
// headers and throws, so the Rocket server re-encodes the reply as a
// PayloadExceptionMetadata::anyException carrying a SemiAnyStruct that gets
// compressed on the wire.
CO_TEST_F(
    RequestResponseServiceE2ETest, AnyExceptionWithCallerThreadDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> /* message */) override {
      auto* header = getRequestContext()->getHeader();
      // "anyext"/"anyex": detail::kHeaderAnyexType / detail::kHeaderAnyex.
      header->setHeader(
          "anyext", "facebook.com/thrift/test/e2e/SimulatedAnyException");
      // Large + repetitive so the resulting SemiAnyStruct is compressed on the
      // wire, which is what makes the eager IO-thread decode see truncated
      // data.
      header->setHeader("anyex", folly::base64Encode(std::string(8192, 'a')));
      throw TApplicationException("simulated overload error");
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestRequestResponseService>();

  try {
    co_await client->co_echo("hello");
    ADD_FAILURE() << "expected co_echo to throw";
  } catch (const TApplicationException& ex) {
    const std::string what = ex.what();
    // Before the fix: what == "anyException deserialization failure: ... the
    // data appears to be truncated". After the fix: the original server
    // exception message is surfaced.
    EXPECT_EQ(what.find("truncated"), std::string::npos) << what;
    EXPECT_NE(what.find("simulated overload error"), std::string::npos) << what;
  }
}

// Companion to AnyExceptionWithCallerThreadDecompression: proves the inline
// exception decompression is *not* over-triggered. A NORMAL (non-exception)
// compressed response under `thrift_client_compress_request_on_cpu` must be
// left untouched by decompressFirstResponseExceptionIfNeeded() and decompressed
// exactly once, on the caller thread. An over-eager inline decompression or a
// double decompression would corrupt the body (or fail), so an intact
// round-trip of a large, compressed payload demonstrates neither happened.
CO_TEST_F(
    RequestResponseServiceE2ETest,
    NormalResponseWithCallerThreadDecompressionNotOverTriggered) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  // Large + repetitive so the response is actually compressed on the wire.
  const std::string payload(8192, 'a');

  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestRequestResponseService>();

  auto result = co_await client->co_echo(payload);
  EXPECT_EQ(result, payload);
}

} // namespace apache::thrift
