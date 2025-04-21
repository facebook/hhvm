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

#pragma once

#include <thrift/perf/cpp2/if/gen-cpp2/StreamBenchmark.h>
#include <thrift/perf/cpp2/util/QPSStats.h>

DEFINE_uint32(chunk_size, 1024, "Number of bytes per chunk");
DEFINE_uint32(batch_size, 16, "Flow control batch size");

namespace facebook::thrift::benchmarks {

using apache::thrift::HandlerCallback;
using apache::thrift::HandlerCallbackBase;
using apache::thrift::HandlerCallbackOneWay;
using apache::thrift::HandlerCallbackPtr;
using apache::thrift::ServerStream;

class BenchmarkHandler
    : virtual public apache::thrift::ServiceHandler<StreamBenchmark> {
 public:
  explicit BenchmarkHandler(QPSStats* stats) : stats_(stats) {
    stats_->registerCounter(kNoop_);
    stats_->registerCounter(kSum_);
    stats_->registerCounter(kTimeout_);
    stats->registerCounter(kDownload_);
    stats->registerCounter(kUpload_);
    stats_->registerCounter(ks_Download_);
    stats_->registerCounter(ks_Upload_);

    chunk_.data()->unshare();
    chunk_.data()->reserve(0, FLAGS_chunk_size);
    auto buffer = chunk_.data()->writableData();
    // Make it real data to eliminate network optimizations on sending all 0's.
    srand(time(nullptr));
    for (uint32_t i = 0; i < FLAGS_chunk_size; ++i) {
      buffer[i] = (uint8_t)(rand() % 26 + 'A');
    }
    chunk_.data()->append(FLAGS_chunk_size);
  }

  void async_eb_noop(HandlerCallbackPtr<void> callback) override {
    stats_->add(kNoop_);
    callback->done();
  }

  void async_eb_onewayNoop(HandlerCallbackOneWay::Ptr callback) override {
    stats_->add(kNoop_);
    callback->done();
  }

  // Make the async/worker thread sleep
  void timeout() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    stats_->add(kTimeout_);
  }

  void async_eb_sum(
      HandlerCallbackPtr<std::unique_ptr<TwoInts>> callback,
      std::unique_ptr<TwoInts> input) override {
    stats_->add(kSum_);
    auto result = std::make_unique<TwoInts>();
    result->x() =
        static_cast<uint32_t>(input->x().value_or(0)) + input->y().value_or(0);
    result->y() =
        static_cast<uint32_t>(input->x().value_or(0)) - input->y().value_or(0);
    callback->result(std::move(result));
  }

  void download(::facebook::thrift::benchmarks::Chunk2& result) override {
    stats_->add(kUpload_);
    result = chunk_;
  }

  void upload(std::unique_ptr<Chunk2>) override { stats_->add(kDownload_); }

  ServerStream<Chunk2> streamDownload() override {
    return folly::coro::co_invoke(
        [this]() -> folly::coro::AsyncGenerator<Chunk2&&> {
          while (true) {
            co_yield folly::copy(chunk_);
            stats_->add(ks_Upload_);
          }
        });
  }

 private:
  QPSStats* stats_;
  std::string kNoop_ = "noop";
  std::string kSum_ = "sum";
  std::string kTimeout_ = "timeout";
  std::string kDownload_ = "download";
  std::string kUpload_ = "upload";
  std::string ks_Download_ = "s_download";
  std::string ks_Upload_ = "s_upload";
  Chunk2 chunk_;
};

} // namespace facebook::thrift::benchmarks
