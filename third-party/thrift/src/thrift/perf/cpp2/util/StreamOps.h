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

#include <random>
#include <folly/GLog.h>
#include <folly/system/ThreadName.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/perf/cpp2/if/gen-cpp2/ApiBase_types.h>
#include <thrift/perf/cpp2/util/QPSStats.h>

DECLARE_uint32(chunk_size);
DECLARE_uint32(batch_size);

using apache::thrift::ClientReceiveState;
using apache::thrift::RequestCallback;
using facebook::thrift::benchmarks::Chunk2;
using facebook::thrift::benchmarks::QPSStats;
using facebook::thrift::benchmarks::TwoInts;

template <typename AsyncClient>
class Download {
 public:
  explicit Download(QPSStats* stats) : stats_(stats) {
    stats_->registerCounter(op_name_);
    stats_->registerCounter(timeout_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
  }
  ~Download() = default;

  void async(AsyncClient* client, std::unique_ptr<RequestCallback> cb) {
    // Give a long timeout value to let the download happen
    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setQueueTimeout(std::chrono::seconds(10));
    rpcOptions.setTimeout(std::chrono::seconds(10));
    client->download(rpcOptions, std::move(cb));
  }

  void asyncReceived(AsyncClient* client, ClientReceiveState&& rstate) {
    try {
      client->recv_download(chunk_, rstate);
      stats_->add(op_name_);
    } catch (const apache::thrift::TApplicationException& ex) {
      if (ex.getType() ==
          apache::thrift::TApplicationException::TApplicationExceptionType::
              TIMEOUT) {
        stats_->add(timeout_);
      } else {
        FB_LOG_EVERY_MS(ERROR, 1000)
            << "Error should have caused error() function to be called: "
            << ex.what();
        stats_->add(error_);
      }
    } catch (const std::exception& ex) {
      FB_LOG_EVERY_MS(ERROR, 1000) << "Critical error: " << ex.what();
      stats_->add(fatal_);
    }
  }

  void error(AsyncClient*, ClientReceiveState&& state) {
    if (state.isException()) {
      FB_LOG_EVERY_MS(INFO, 1000) << "Error is: " << state.exception().what();
    }
    stats_->add(error_);
  }

 private:
  QPSStats* stats_;
  std::string op_name_ = "download";
  std::string timeout_ = "timeout";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
  Chunk2 chunk_;
};

template <typename AsyncClient>
class Upload {
 public:
  Upload(QPSStats* stats, uint32_t chunkSize) : stats_(stats) {
    stats_->registerCounter(op_name_);
    stats_->registerCounter(timeout_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
    chunk_.data()->unshare();
    chunk_.data()->reserve(0, chunkSize);
    auto buffer = chunk_.data()->writableData();
    // Make it real data to eliminate network optimizations on sending all 0's.
    srand(time(nullptr));
    for (uint32_t i = 0; i < FLAGS_chunk_size; ++i) {
      buffer[i] = (uint8_t)(rand() % 26 + 'A');
    }
    chunk_.data()->append(chunkSize);
  }
  ~Upload() = default;

  void async(AsyncClient* client, std::unique_ptr<RequestCallback> cb) {
    // Give a long timeout value to let the download happen
    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setQueueTimeout(std::chrono::seconds(10));
    rpcOptions.setTimeout(std::chrono::seconds(10));
    client->upload(rpcOptions, std::move(cb), chunk_);
  }

  void asyncReceived(AsyncClient* client, ClientReceiveState&& rstate) {
    try {
      client->recv_upload(rstate);
      stats_->add(op_name_);
    } catch (const apache::thrift::TApplicationException& ex) {
      if (ex.getType() ==
          apache::thrift::TApplicationException::TApplicationExceptionType::
              TIMEOUT) {
        stats_->add(timeout_);
      } else {
        FB_LOG_EVERY_MS(ERROR, 1000)
            << "Error should have caused error() function to be called: "
            << ex.what();
        stats_->add(error_);
      }
    } catch (const std::exception& ex) {
      FB_LOG_EVERY_MS(ERROR, 1000) << "Critical error: " << ex.what();
      stats_->add(fatal_);
    }
  }

  void error(AsyncClient*, ClientReceiveState&& state) {
    if (state.isException()) {
      FB_LOG_EVERY_MS(INFO, 1000) << "Error is: " << state.exception().what();
    }
    stats_->add(error_);
  }

 private:
  QPSStats* stats_;
  std::string op_name_ = "upload";
  std::string timeout_ = "timeout";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
  Chunk2 chunk_;
};

template <typename AsyncClient>
class StreamDownload {
 public:
  StreamDownload(QPSStats* stats, uint32_t chunkSize) : stats_(stats) {
    stats_->registerCounter(download_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
    chunk_.data()->unshare();
    chunk_.data()->reserve(0, chunkSize);
    auto buffer = chunk_.data()->writableData();
    for (uint32_t i = 0; i < chunkSize; ++i) {
      buffer[i] = (uint8_t)((i + 'A') % 26);
    }
    chunk_.data()->append(chunkSize);
  }
  ~StreamDownload() = default;

  void async(
      AsyncClient* client,
      std::unique_ptr<RequestCallback>,
      int32_t& outstandingOps) {
    // Give a long timeout value to let the download happen
    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setQueueTimeout(std::chrono::seconds(10));
    rpcOptions.setTimeout(std::chrono::seconds(10));
    rpcOptions.setChunkBufferSize(FLAGS_batch_size);

    client->sync_streamDownload(rpcOptions)
        .subscribeExTry(
            folly::EventBaseManager::get()->getEventBase(),
            [this, &outstandingOps](auto&& t) {
              if (t.hasValue()) {
                stats_->add(download_);
              } else if (t.hasException()) {
                stats_->add(fatal_);
                --outstandingOps;

              } else {
                --outstandingOps;
              }
            })
        .detach();
  }

  void asyncReceived(AsyncClient*, ClientReceiveState&&) {}

  void error(AsyncClient*, ClientReceiveState&& state) {
    if (state.isException()) {
      FB_LOG_EVERY_MS(INFO, 1000) << "Error is: " << state.exception().what();
    }
    stats_->add(error_);
  }

 private:
  QPSStats* stats_;
  std::string download_ = "s_download";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
  Chunk2 chunk_;
};
