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

#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/perf/cpp2/util/QPSStats.h>
#include <thrift/perf/cpp2/util/SimpleOps.h>
#ifdef STREAM_PERF_TEST
#include <thrift/perf/cpp2/util/StreamOps.h>
#endif

DECLARE_uint32(chunk_size);

using apache::thrift::ClientReceiveState;
using apache::thrift::RequestCallback;
using facebook::thrift::benchmarks::QPSStats;

template <typename AsyncClient>
class LoadCallback;

enum OP_TYPE {
  NOOP = 0,
  NOOP_ONEWAY = 1,
  SUM = 2,
  TIMEOUT = 3,
  DOWNLOAD = 4,
  UPLOAD = 5,
  STREAM = 6,
  SEMIFUTURE_SUM = 7,
  CO_SUM = 8,
};

template <typename AsyncClient>
class Operation {
 public:
  Operation(std::unique_ptr<AsyncClient> client, QPSStats* stats)
      : client_(std::move(client)),
        noop_(std::make_unique<Noop<AsyncClient>>(stats)),
        sum_(std::make_unique<Sum<AsyncClient>>(stats)),
        timeout_(std::make_unique<Timeout<AsyncClient>>(stats)),
#ifdef STREAM_PERF_TEST
        download_(std::make_unique<Download<AsyncClient>>(stats)),
        upload_(std::make_unique<Upload<AsyncClient>>(stats, FLAGS_chunk_size)),
        stream_(
            std::make_unique<StreamDownload<AsyncClient>>(
                stats, FLAGS_chunk_size)),
#endif
        semifuture_sum_(std::make_unique<SemiFutureSum<AsyncClient>>(stats)),
        co_sum_(std::make_unique<CoSum<AsyncClient>>(stats)) {
  }
  ~Operation() = default;

  int32_t outstandingOps() { return outstanding_ops_; }

  void async(OP_TYPE op, std::unique_ptr<LoadCallback<AsyncClient>> cb) {
    ++outstanding_ops_;
    switch (op) {
      case NOOP:
        noop_->async(client_.get(), std::move(cb));
        break;
      case NOOP_ONEWAY:
        cb->setIsOneway();
        noop_->onewayAsync(client_.get(), std::move(cb));
        break;
      case SUM:
        sum_->async(client_.get(), std::move(cb));
        break;
      case TIMEOUT:
        timeout_->async(client_.get(), std::move(cb));
        break;
#ifdef STREAM_PERF_TEST
      case DOWNLOAD:
        download_->async(client_.get(), std::move(cb));
        break;
      case UPLOAD:
        upload_->async(client_.get(), std::move(cb));
        break;
      case STREAM:
        stream_->async(client_.get(), std::move(cb), outstanding_ops_);
        break;
#endif
      case SEMIFUTURE_SUM:
        semifuture_sum_->async(client_.get(), std::move(cb));
        break;
      case CO_SUM:
        co_sum_->async(client_.get(), std::move(cb));
        break;
      default:
        break;
    }
  }

  void onewaySent(OP_TYPE op) {
    switch (op) {
      case NOOP_ONEWAY:
        noop_->onewaySent();
        --outstanding_ops_;
        break;
      default:
        LOG(ERROR) << "Should send oneway calls";
        break;
    }
  }

  void asyncReceived(OP_TYPE op, ClientReceiveState&& rstate) {
    --outstanding_ops_;
    switch (op) {
      case NOOP:
        noop_->asyncReceived(client_.get(), std::move(rstate));
        break;
      case SUM:
        sum_->asyncReceived(client_.get(), std::move(rstate));
        break;
      case TIMEOUT:
        timeout_->asyncReceived(client_.get(), std::move(rstate));
        break;
#ifdef STREAM_PERF_TEST
      case DOWNLOAD:
        download_->asyncReceived(client_.get(), std::move(rstate));
        break;
      case UPLOAD:
        upload_->asyncReceived(client_.get(), std::move(rstate));
        break;
#endif
      case SEMIFUTURE_SUM:
      case CO_SUM:
        break;
      default:
        LOG(ERROR) << "Should not have async callback";
        break;
    }
  }

  void asyncErrorReceived(OP_TYPE op, ClientReceiveState&& rstate) {
    --outstanding_ops_;
    switch (op) {
      case NOOP:
        noop_->error(client_.get(), std::move(rstate));
        break;
      case SUM:
        sum_->error(client_.get(), std::move(rstate));
        break;
      case TIMEOUT:
        timeout_->error(client_.get(), std::move(rstate));
        break;
#ifdef STREAM_PERF_TEST
      case DOWNLOAD:
        download_->error(client_.get(), std::move(rstate));
        break;
      case UPLOAD:
        upload_->error(client_.get(), std::move(rstate));
        break;
#endif
      case SEMIFUTURE_SUM:
      case CO_SUM:
        break;
      default:
        LOG(ERROR) << "Should not have async callback";
        break;
    }
  }

 private:
  std::unique_ptr<AsyncClient> client_;
  std::unique_ptr<Noop<AsyncClient>> noop_;
  std::unique_ptr<Sum<AsyncClient>> sum_;
  std::unique_ptr<Timeout<AsyncClient>> timeout_;
#ifdef STREAM_PERF_TEST
  std::unique_ptr<Download<AsyncClient>> download_;
  std::unique_ptr<Upload<AsyncClient>> upload_;
  std::unique_ptr<StreamDownload<AsyncClient>> stream_;
#endif
  std::unique_ptr<SemiFutureSum<AsyncClient>> semifuture_sum_;
  std::unique_ptr<CoSum<AsyncClient>> co_sum_;

  int32_t outstanding_ops_{0};
};
