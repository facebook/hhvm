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

#include <folly/synchronization/CallOnce.h>
#include <thrift/conformance/stresstest/client/StressTestClient.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>

#include <folly/coro/Sleep.h>

namespace apache::thrift::stress {

namespace {
constexpr double kHistogramMax = 1000.0 * 60.0; // 1 minute

folly::coro::Task<void> maybeSleep(const StreamRequest& req) {
  if (auto dur = *req.processInfo()->clientChunkProcessingTimeMs(); dur > 0) {
    co_await folly::coro::sleep(std::chrono::milliseconds(dur));
  }
}
} // namespace

ClientRpcStats::ClientRpcStats() : latencyHistogram(50, 0.0, kHistogramMax) {}

void ClientRpcStats::combine(const ClientRpcStats& other) {
  latencyHistogram.merge(other.latencyHistogram);
  numSuccess += other.numSuccess;
  numFailure += other.numFailure;
}

folly::coro::Task<void> ThriftStressTestClient::co_ping() {
  co_await timedExecute([&]() { return client_->co_ping(); });
}

folly::coro::Task<std::string> ThriftStressTestClient::co_echo(
    const std::string& x) {
  std::string ret;
  co_await timedExecute([&]() -> folly::coro::Task<void> {
    RpcOptions rpcOptions;
    if (enableChecksum_) {
      static folly::once_flag flag;

      folly::call_once(flag, [] {
        LOG(INFO) << "Initializing checksum payload serializer" << std::endl;
        rocket::PayloadSerializer::initialize(
            rocket::ChecksumPayloadSerializerStrategy<
                rocket::DefaultPayloadSerializerStrategy>(
                rocket::ChecksumPayloadSerializerStrategyOptions{
                    .recordChecksumFailure =
                        [] { LOG(FATAL) << "Checksum failure detected"; },
                    .recordChecksumSuccess =
                        [] {
                          LOG_EVERY_N(INFO, 1'000)
                              << "Checksum success detected";
                        },
                    .recordChecksumCalculated =
                        [] {
                          LOG_EVERY_N(INFO, 1'000) << "Checksum calculated";
                        },
                    .recordChecksumSkipped =
                        [] {
                          LOG_EVERY_N(INFO, 1'000) << "Checksum skipped";
                        }}));
      });

      rpcOptions.setChecksum(RpcOptions::Checksum::XXH3_64);
    }
    ret = co_await client_->co_echo(rpcOptions, x);
  });
  co_return ret;
}

folly::coro::Task<std::string> ThriftStressTestClient::co_echoEb(
    const std::string& x) {
  std::string ret;
  co_await timedExecute([&]() -> folly::coro::Task<void> {
    RpcOptions rpcOptions;
    if (enableChecksum_) {
      static folly::once_flag flag;

      folly::call_once(flag, [] {
        LOG(INFO) << "Initializing checksum payload serializer" << std::endl;
        rocket::PayloadSerializer::initialize(
            rocket::ChecksumPayloadSerializerStrategy<
                rocket::DefaultPayloadSerializerStrategy>(
                rocket::ChecksumPayloadSerializerStrategyOptions{
                    .recordChecksumFailure =
                        [] { LOG(FATAL) << "Checksum failure detected"; },
                    .recordChecksumSuccess =
                        [] {
                          LOG_EVERY_N(INFO, 1'000)
                              << "Checksum success detected";
                        },
                    .recordChecksumCalculated =
                        [] {
                          LOG_EVERY_N(INFO, 1'000) << "Checksum calculated";
                        }}));
      });

      rpcOptions.setChecksum(RpcOptions::Checksum::XXH3_64);
    }
    ret = co_await client_->co_echoEb(rpcOptions, x);
  });
  co_return ret;
}

folly::coro::Task<void> ThriftStressTestClient::co_requestResponseEb(
    const BasicRequest& req) {
  co_await timedExecute([&]() { return client_->co_requestResponseEb(req); });
}

folly::coro::Task<void> ThriftStressTestClient::co_requestResponseTm(
    const BasicRequest& req) {
  co_await timedExecute([&]() { return client_->co_requestResponseTm(req); });
}

folly::coro::Task<void> ThriftStressTestClient::co_streamTm(
    const StreamRequest& req) {
  // time the entire stream from start to finish
  co_await timedExecute([&]() -> folly::coro::Task<void> {
    auto result = co_await client_->co_streamTm(req);
    auto gen = std::move(result).stream.toAsyncGenerator();
    while (co_await gen.next()) {
      co_await maybeSleep(req);
    }
  });
}

folly::coro::Task<void> ThriftStressTestClient::co_sinkTm(
    const StreamRequest& req) {
  // time the entire sink from start to finish
  co_await timedExecute([&]() -> folly::coro::Task<void> {
    auto result = co_await client_->co_sinkTm(req);
    std::ignore =
        result.sink.sink([&]() -> folly::coro::AsyncGenerator<BasicResponse&&> {
          for (int64_t i = 0; i < req.processInfo()->numChunks(); i++) {
            co_await maybeSleep(req);
            BasicResponse chunk;
            if (auto size = *req.processInfo()->chunkSize(); size > 0) {
              chunk.payload() = std::string('x', size);
            }
            co_yield std::move(chunk);
          }
        }());
    // (void)finalResponse; // we don't care about this
  });
}

folly::coro::Task<double> ThriftStressTestClient::co_calculateSquares(
    int32_t count) {
  double ret;
  co_await timedExecute([&]() -> folly::coro::Task<void> {
    ret = co_await client_->co_calculateSquares(count);
  });
  co_return ret;
}

folly::coro::Task<AlignedResponse> ThriftStressTestClient::co_aligned(
    const AlignedRequest& req) {
  AlignedResponse ret;
  co_await timedExecute([&]() -> folly::coro::Task<void> {
    ret = co_await client_->co_aligned(req);
  });
  co_return ret;
}

template <class Fn>
folly::coro::Task<void> ThriftStressTestClient::timedExecute(Fn&& fn) {
  if (!connectionGood_) {
    co_return;
  }
  auto start = std::chrono::steady_clock::now();
  try {
    co_await fn();
  } catch (folly::OperationCancelled&) {
    // cancelled requests do not count as failures
    throw;
  } catch (transport::TTransportException& e) {
    // assume fatal issue with connection, stop using this client
    // TODO: Improve handling of connection issues
    LOG(ERROR) << e.what();
    connectionGood_ = false;
    co_return;
  } catch (...) {
    // LOG(1) << "Request failed: " << e.what();
    stats_.numFailure++;
    co_return;
  }
  auto elapsed = std::chrono::steady_clock::now() - start;
  stats_.latencyHistogram.addValue(
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
  stats_.numSuccess++;
}

} // namespace apache::thrift::stress
