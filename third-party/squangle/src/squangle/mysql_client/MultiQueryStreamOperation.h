/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/FetchOperation.h"

namespace facebook::common::mysql_client {

class MultiQueryStreamHandler;
class SyncConnection;

// This operation only supports one mode: streaming callback. This is a
// simple layer on top of FetchOperation to adapt from `notify` to
// StreamCallback.
// This is an experimental class. Please don't use directly.
class MultiQueryStreamOperation : public FetchOperation {
 public:
  ~MultiQueryStreamOperation() override = default;

  using Callback = std::function<void(FetchOperation&, StreamState)>;
  using StreamCallback = std::variant<MultiQueryStreamHandler*, Callback>;

  void notifyInitQuery() override;
  void notifyRowsReady() override;
  bool notifyQuerySuccess(bool more_results) override;
  void notifyFailure(OperationResult result) override;
  void notifyOperationCompleted(OperationResult result) override;

  // Overriding to narrow the return type
  MultiQueryStreamOperation& setTimeout(Duration timeout) {
    Operation::setTimeout(timeout);
    return *this;
  }

  db::OperationType getOperationType() const override {
    return db::OperationType::MultiQueryStream;
  }

  void setCallback(StreamCallback cb) {
    stream_callback_ = std::move(cb);
  }

 protected:
  static std::shared_ptr<MultiQueryStreamOperation> create(
      std::unique_ptr<FetchOperationImpl> opImpl,
      MultiQuery&& multi_query);

  friend Connection;
  friend SyncConnection;

 private:
  MultiQueryStreamOperation(
      std::unique_ptr<FetchOperationImpl> opImpl,
      MultiQuery&& multi_query);
  MultiQueryStreamOperation(
      std::unique_ptr<FetchOperationImpl> opImpl,
      std::vector<Query>&& queries);

  // wrapper to construct CallbackVistor and invoke the
  // right callback
  void invokeCallback(StreamState state);

  // Vistor to invoke the right callback depending on the type stored
  // in the variant 'stream_callback_'
  struct CallbackVisitor {
    CallbackVisitor(MultiQueryStreamOperation& op, StreamState state)
        : op_(op), state_(state) {}

    void operator()(MultiQueryStreamHandler* handler) const {
      if (handler != nullptr) {
        handler->streamCallback(op_, state_);
      }
    }

    void operator()(const Callback& cb) const {
      if (cb != nullptr) {
        cb(op_, state_);
      }
    }

   private:
    MultiQueryStreamOperation& op_;
    StreamState state_;
  };

  StreamCallback stream_callback_;
};

} // namespace facebook::common::mysql_client
