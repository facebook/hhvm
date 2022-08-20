/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#include <glog/logging.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>
#include <thrift/perf/cpp2/util/Operation.h>
#include <thrift/perf/cpp2/util/QPSStats.h>
#include <thrift/perf/cpp2/util/Util.h>

using apache::thrift::ClientConnectionIf;
using apache::thrift::ClientReceiveState;
using apache::thrift::RequestCallback;
using facebook::thrift::benchmarks::QPSStats;

template <typename AsyncClient>
class LoadCallback;

template <typename AsyncClient>
class Runner {
 public:
  friend class LoadCallback<AsyncClient>;

  Runner(
      std::shared_ptr<folly::EventBase> evb,
      std::unique_ptr<Operation<AsyncClient>> ops,
      std::unique_ptr<std::discrete_distribution<int32_t>> distribution,
      int32_t max_outstanding_ops)
      : evb_(evb),
        ops_(std::move(ops)),
        d_(std::move(distribution)),
        max_outstanding_ops_(max_outstanding_ops) {}

  void run() {
    // TODO: Implement sync calls.
    while (ops_->outstandingOps() < max_outstanding_ops_) {
      auto op = static_cast<OP_TYPE>((*d_)(gen_));
      auto cb =
          std::make_unique<LoadCallback<AsyncClient>>(this, ops_.get(), op);
      ops_->async(op, std::move(cb));
    }
  }

  void finishCall() {
    run(); // Attempt to perform more async calls
  }

 private:
  std::shared_ptr<folly::EventBase> evb_;
  std::unique_ptr<Operation<AsyncClient>> ops_;
  std::unique_ptr<std::discrete_distribution<int32_t>> d_;
  int32_t max_outstanding_ops_;

  std::mt19937 gen_{std::random_device()()};
};

template <typename AsyncClient>
class LoadCallback : public RequestCallbackWithValidator {
 public:
  LoadCallback(
      Runner<AsyncClient>* runner, Operation<AsyncClient>* ops, OP_TYPE op)
      : runner_(runner), ops_(ops), op_(op) {}

  void setIsOneway() { isOneway_ = true; }

  // TODO: Properly handle errors and exceptions
  void requestSent() override {
    if (isOneway_) {
      ops_->onewaySent(op_);
      runner_->finishCall();
    }
  }
  void replyReceived(ClientReceiveState&& rstate) override {
    if (validator) {
      validator(rstate);
    }
    ops_->asyncReceived(op_, std::move(rstate));
    runner_->finishCall();
  }
  void requestError(ClientReceiveState&& rstate) override {
    ops_->asyncErrorReceived(op_, std::move(rstate));
    runner_->finishCall();
  }

 private:
  Runner<AsyncClient>* runner_;
  Operation<AsyncClient>* ops_;
  OP_TYPE op_;
  bool isOneway_{false};
};
