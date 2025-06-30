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

#include <common/services/cpp/security/SecureThriftUtil.h>
#include <thrift/conformance/stresstest/StressTest.h>

using namespace apache::thrift::stress;

/**
 * Simple ping requests, e.g. to benchmark minimum round trip times.
 */
THRIFT_STRESS_TEST(Ping) {
  co_await client->co_ping();
}

THRIFT_STRESS_TEST(Echo256b) {
  static std::string const s(256, '?');
  co_await client->co_echo(s);
}

THRIFT_STRESS_TEST(Echo256b_eb) {
  static std::string const s(256, '?');
  co_await client->co_echoEb(s);
}

THRIFT_STRESS_TEST(Echo4096b) {
  static std::string const s(4096, '?');
  co_await client->co_echo(s);
}

THRIFT_STRESS_TEST(Echo32k) {
  static std::string const s(1 << 15, '?');
  co_await client->co_echo(s);
}

THRIFT_STRESS_TEST(Echo128k) {
  static std::string const s(1 << 17, '?');
  co_await client->co_echo(s);
}

THRIFT_STRESS_TEST(Echo512k) {
  static std::string const s(1 << 19, '?');
  co_await client->co_echo(s);
}

THRIFT_STRESS_TEST(Echo512k_eb) {
  static std::string const s(1 << 19, '?');
  co_await client->co_echoEb(s);
}

THRIFT_STRESS_TEST(Echo4M) {
  static std::string const s(4096000, '?');
  co_await client->co_echo(s);
}

THRIFT_STRESS_TEST(Echo4M_eb) {
  static std::string const s(4096000, '?');
  co_await client->co_echoEb(s);
}

THRIFT_STRESS_TEST(RequestResponseTm_32k_unencrypted) {
  size_t const payloadSize = 1 << 15;
  BasicRequest req;
  req.processInfo()->processingTimeMs() = 0;
  req.processInfo()->responseSize() = payloadSize;
  req.processInfo()->workSimulationMode() = WorkSimulationMode::Sleep;

  static std::string const s(payloadSize, '?');
  folly::ByteRange br(reinterpret_cast<const uint8_t*>(s.data()), payloadSize);
  // Create unencrypted buf from the byte range
  auto buf = facebook::services::SecureThriftUtil::createUnencryptedBuf(
      br, [](void* /*buf*/, void* /*userData*/) {});
  if (!buf) {
    co_return;
  }

  req.stoptls_payload() = std::move(buf);
  co_await client->co_requestResponseTm(req);
}

THRIFT_STRESS_TEST(RequestResponseTm_32k) {
  size_t const payloadSize = 1 << 15;
  BasicRequest req;
  req.processInfo()->processingTimeMs() = 0;
  req.processInfo()->responseSize() = payloadSize;
  req.processInfo()->workSimulationMode() = WorkSimulationMode::Sleep;

  static std::string const s(payloadSize, '?');
  req.payload() = s;
  co_await client->co_requestResponseTm(req);
}

/**
 * Send a request with a small payload, have the server "process" it for 50ms on
 * the CPU threadpool via sleeping instead of the default busy wait, and return
 * a response with a 1024 byte payload.
 */
THRIFT_STRESS_TEST(RequestResponseTm) {
  // TODO: provide more convenient way to create request objects
  BasicRequest req;
  req.processInfo()->processingTimeMs() = 50;
  req.processInfo()->responseSize() = 1024;
  req.processInfo()->workSimulationMode() = WorkSimulationMode::Sleep;
  req.payload() = std::string('x', 64);
  co_await client->co_requestResponseTm(req);
}

/**
 * Send multiple requests asynchronously before awaiting the results, then sleep
 * for 100ms.
 */
THRIFT_STRESS_TEST(AsynchronousRequests) {
  BasicRequest req;
  req.processInfo()->processingTimeMs() = 100;
  req.processInfo()->responseSize() = 4096;

  // execute three requests asynchronously
  co_await folly::coro::collectAll(
      client->co_requestResponseEb(req),
      client->co_requestResponseEb(req),
      client->co_requestResponseEb(req));

  // sleep for 100ms
  co_await folly::coro::sleep(std::chrono::milliseconds(100));
}

/**
 * Send a request to stream 10 chunks of 512 bytes each, with 50ms processing
 * time per chunk on the server side and 10ms delay between chunks on client
 * side.
 * Then, use a sink to send 5 chunks of 256 bytes each back to the server, with
 * 50ms processing time per chunk on the server side and 10ms delay between
 * chunks on client side.
 */
THRIFT_STRESS_TEST(SinkAndStream) {
  StreamRequest streamReq;
  streamReq.processInfo()->serverChunkProcessingTimeMs() = 50;
  streamReq.processInfo()->clientChunkProcessingTimeMs() = 10;
  streamReq.processInfo()->numChunks() = 10;
  streamReq.processInfo()->chunkSize() = 512;
  co_await client->co_streamTm(streamReq);

  StreamRequest sinkReq;
  sinkReq.processInfo()->serverChunkProcessingTimeMs() = 50;
  sinkReq.processInfo()->clientChunkProcessingTimeMs() = 10;
  sinkReq.processInfo()->numChunks() = 5;
  sinkReq.processInfo()->chunkSize() = 256;
  co_await client->co_sinkTm(sinkReq);
}

THRIFT_STRESS_TEST(CoroTest_10) {
  co_await client->co_calculateSquares(10);
}

THRIFT_STRESS_TEST(Stream512_64Chunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 64;
  req.processInfo()->numChunks() = 8;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream512_128Chunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 128;
  req.processInfo()->numChunks() = 4;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream512_256Chunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 256;
  req.processInfo()->numChunks() = 2;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream5k_128Chunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 128;
  req.processInfo()->numChunks() = 40;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream5k_256Chunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 256;
  req.processInfo()->numChunks() = 20;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream5k_512Chunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 512;
  req.processInfo()->numChunks() = 10;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream5M_5kChunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 5 * 1'024;
  req.processInfo()->numChunks() = 1000;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream5M_10kChunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 10 * 1'024;
  req.processInfo()->numChunks() = 500;
  co_await client->co_streamTm(std::move(req));
}

THRIFT_STRESS_TEST(Stream5M_20kChunks) {
  using namespace apache::thrift::stress;
  StreamRequest req;
  req.processInfo() = StreamProcessInfo();
  req.processInfo()->chunkSize() = 20 * 1'024;
  req.processInfo()->numChunks() = 250;
  co_await client->co_streamTm(std::move(req));
}
