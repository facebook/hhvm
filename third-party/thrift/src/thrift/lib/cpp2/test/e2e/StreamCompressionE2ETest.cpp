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
#include <folly/base64.h>
#include <folly/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestBiDiService.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestSinkE2EService.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestStreamE2EService.h>

THRIFT_FLAG_DECLARE_bool(thrift_server_compress_response_on_cpu);
THRIFT_FLAG_DECLARE_bool(thrift_client_compress_request_on_cpu);

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

// Verifies that a basic stream round-trips correctly with CPU-thread
// compression enabled when the method is dispatched in EB mode (via the
// @cpp.ProcessInEbThreadUnsafe annotation, which forces async_eb_*
// dispatch on the IO thread). In EB mode, executor_ is null in
// HandlerCallback, so this exercises the getCompressionExecutorFallback()
// path in doResult() that ensures stream subsequent items are compressed
// off the IO thread.
CO_TEST_F(StreamCompressionE2ETest, BasicStreamWithCpuCompressionEBMode) {
  THRIFT_FLAG_SET_MOCK(thrift_server_compress_response_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    void async_eb_stringRangeEb(
        HandlerCallbackPtr<ServerStream<std::string>> callback,
        int32_t from,
        int32_t to) override {
      // Sanity check: if this assertion fires, the @cpp.ProcessInEbThreadUnsafe
      // annotation did not produce EB-mode dispatch and the test is no longer
      // exercising the code path it intends to cover.
      auto* evb = callback->getEventBase();
      EXPECT_TRUE(evb->inRunningEventBaseThread());
      auto stream = folly::coro::co_invoke(
          [from, to, evb]() -> folly::coro::AsyncGenerator<std::string&&> {
            for (int32_t i = from; i <= to; ++i) {
              // The generator must run on a CPU executor, NOT on the IO
              // EventBase thread. This proves the compression offload in
              // doResult() is working — without the fix, this assertion
              // would fire because the stream would run on eb_.
              EXPECT_FALSE(evb->inRunningEventBaseThread());
              co_yield std::string(256, 'a' + (i % 26));
            }
          });
      callback->result(std::move(stream));
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
  auto gen = (co_await client->co_stringRangeEb(0, 9)).toAsyncGenerator();

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

// Verifies that ServerPublisherStream (push-based) compresses stream items
// on the CPU thread when the flag is enabled.
CO_TEST_F(StreamCompressionE2ETest, PublisherStreamWithCpuCompression) {
  THRIFT_FLAG_SET_MOCK(thrift_server_compress_response_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> publisherRange(int32_t from, int32_t to) override {
      auto [stream, publisher] = ServerStream<int32_t>::createPublisher([] {});
      for (int32_t i = from; i <= to; ++i) {
        publisher.next(i);
      }
      std::move(publisher).complete();
      // @lint-ignore ASTGREP
      return std::move(stream);
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
  auto gen = (co_await client->co_publisherRange(0, 9)).toAsyncGenerator();

  for (int32_t expected = 0; expected <= 9; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_FALSE((co_await gen.next()).has_value());
}

RequestChannel::Ptr makeCompressedChannel(
    folly::AsyncSocket::UniquePtr socket) {
  auto channel = RocketClientChannel::newChannel(std::move(socket));
  CompressionConfig compressionConfig;
  compressionConfig.codecConfig().ensure().set_zstdConfig();
  channel->setDesiredCompressionConfig(compressionConfig);
  return channel;
}

// Verifies that a declared stream exception is correctly decompressed and
// delivered to the client when CPU-thread decompression is enabled. Without the
// fix in decode_stream_exception, the client would attempt to deserialize
// still-compressed exception bytes and crash.
CO_TEST_F(
    StreamCompressionE2ETest, StreamDeclaredExceptionWithCpuDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> canThrow(int32_t from, int32_t to) override {
      for (int32_t i = from; i < to; ++i) {
        co_yield int32_t(i);
      }
      throw detail::test::StreamDeclaredException{std::string(256, 'E')};
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_canThrow(0, 3)).toAsyncGenerator();

  for (int32_t expected = 0; expected < 3; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_THROW(co_await gen.next(), detail::test::StreamDeclaredException);
}

CO_TEST_F(
    StreamCompressionE2ETest, StreamUndeclaredExceptionWithCpuDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> canThrow(int32_t from, int32_t to) override {
      for (int32_t i = from; i < to; ++i) {
        co_yield int32_t(i);
      }
      throw std::runtime_error{std::string(256, 'E')};
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_canThrow(0, 3)).toAsyncGenerator();

  for (int32_t expected = 0; expected < 3; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_THROW(co_await gen.next(), apache::thrift::TApplicationException);
}

// HIGH-1 repro: a direct (non-SR) Rocket STREAM whose FIRST response is a
// compressed `anyException`. FirstRequestProcessorStream::onFirstResponse calls
// processFirstResponse eagerly on the IO thread -- before the caller-thread
// decompressResponse() runs -- so with the flag ON the still-compressed payload
// is decoded as if uncompressed and aborts with "... truncated". The unary fix
// in D108684232 did NOT cover this streaming first-response path.
//
// The handler mimics SRProxy's sendServiceRouterError: it stashes a large,
// compressible payload in the anyex/anyext response headers and throws, so the
// first response is encoded as a (compressed) PayloadExceptionMetadata::
// anyException.
CO_TEST_F(
    StreamCompressionE2ETest,
    StreamFirstResponseAnyExceptionWithCpuDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> canThrow(int32_t, int32_t) override {
      auto* header = getRequestContext()->getHeader();
      header->setHeader(
          "anyext", "facebook.com/thrift/test/e2e/SimulatedAnyException");
      header->setHeader("anyex", folly::base64Encode(std::string(8192, 'a')));
      throw TApplicationException("simulated overload error");
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestStreamE2EService>();

  try {
    co_await client->co_canThrow(0, 3);
    ADD_FAILURE() << "expected co_canThrow first response to throw";
  } catch (const TApplicationException& ex) {
    const std::string what = ex.what();
    EXPECT_EQ(what.find("truncated"), std::string::npos) << what;
    EXPECT_NE(what.find("simulated overload error"), std::string::npos) << what;
  }
}

// Companion to StreamFirstResponseAnyExceptionWithCpuDecompression: proves the
// inline exception decompression is *not* over-triggered on the stream path. A
// NORMAL stream (non-exception first response) under
// thrift_client_compress_request_on_cpu must round-trip its compressed items
// intact -- decompressFirstResponseExceptionIfNeeded() must leave the first
// response untouched and each item must be decompressed exactly once on the
// caller thread. An over-eager or double decompression would corrupt the items.
CO_TEST_F(
    StreamCompressionE2ETest,
    StreamNormalResponseWithCpuDecompressionNotOverTriggered) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<std::string> stringRange(int32_t from, int32_t to) override {
      for (int32_t i = from; i <= to; ++i) {
        co_yield std::string(256, 'a' + (i % 26));
      }
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_stringRange(0, 9)).toAsyncGenerator();

  for (int32_t expected = 0; expected <= 9; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, std::string(256, 'a' + (expected % 26)));
  }
  EXPECT_FALSE((co_await gen.next()).has_value());
}

class SinkCompressionE2ETest : public test::E2ETestFixture {};

CO_TEST_F(
    SinkCompressionE2ETest, SinkFinalResponseExceptionWithCpuDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> canThrow() override {
      return SinkConsumer<int32_t, std::string>{
          [](folly::coro::AsyncGenerator<int32_t&&> gen)
              -> folly::coro::Task<std::string> {
            while (auto item = co_await gen.next()) {
            }
            throw detail::test::SinkFinalException{std::string(256, 'E')};
          },
          10};
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  auto sink = co_await client->co_canThrow();
  EXPECT_THROW(
      co_await sink.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
        co_yield int32_t(1);
      }()),
      detail::test::SinkFinalException);
}

// HIGH-1 repro (sink): same first-response anyException path through
// FirstRequestProcessorSink::onFirstResponse.
CO_TEST_F(
    SinkCompressionE2ETest, SinkFirstResponseAnyExceptionWithCpuDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> canThrow() override {
      auto* header = getRequestContext()->getHeader();
      header->setHeader(
          "anyext", "facebook.com/thrift/test/e2e/SimulatedAnyException");
      header->setHeader("anyex", folly::base64Encode(std::string(8192, 'a')));
      throw TApplicationException("simulated overload error");
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestSinkE2EService>();

  try {
    co_await client->co_canThrow();
    ADD_FAILURE() << "expected co_canThrow first response to throw";
  } catch (const TApplicationException& ex) {
    const std::string what = ex.what();
    EXPECT_EQ(what.find("truncated"), std::string::npos) << what;
    EXPECT_NE(what.find("simulated overload error"), std::string::npos) << what;
  }
}

class BiDiCompressionE2ETest : public test::E2ETestFixture {};

CO_TEST_F(BiDiCompressionE2ETest, BiDiStreamExceptionWithCpuDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
            throw detail::test::BiDiStreamException{std::string(256, 'E')};
          }};
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> bidi = co_await client->co_canThrow();

  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        co_yield int64_t(1);
        co_yield int64_t(2);
      });
  co_await bidi.sink.sink(std::move(sinkGen));

  auto streamGen = std::move(bidi.stream).toAsyncGenerator();
  auto first = co_await streamGen.next();
  EXPECT_TRUE(first.has_value());
  EXPECT_EQ(*first, 1);

  auto second = co_await streamGen.next();
  EXPECT_TRUE(second.has_value());
  EXPECT_EQ(*second, 2);

  EXPECT_THROW(co_await streamGen.next(), detail::test::BiDiStreamException);
}

// HIGH-1 repro (bidi): same first-response anyException path through
// FirstRequestProcessorBiDi::onFirstResponse.
CO_TEST_F(
    BiDiCompressionE2ETest, BiDiFirstResponseAnyExceptionWithCpuDecompression) {
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      auto* header = getRequestContext()->getHeader();
      header->setHeader(
          "anyext", "facebook.com/thrift/test/e2e/SimulatedAnyException");
      header->setHeader("anyex", folly::base64Encode(std::string(8192, 'a')));
      throw TApplicationException("simulated overload error");
    }
  };

  testConfig({std::make_shared<Handler>(), {}, makeCompressedChannel});
  auto client = makeClient<detail::test::TestBiDiService>();

  try {
    co_await client->co_canThrow();
    ADD_FAILURE() << "expected co_canThrow first response to throw";
  } catch (const TApplicationException& ex) {
    const std::string what = ex.what();
    EXPECT_EQ(what.find("truncated"), std::string::npos) << what;
    EXPECT_NE(what.find("simulated overload error"), std::string::npos) << what;
  }
}

} // namespace

} // namespace apache::thrift
