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
  static constexpr db::OperationType kOperationType =
      db::OperationType::MultiQueryStream;

  ~MultiQueryStreamOperation() override = default;

  using StreamCallback = std::function<void(FetchOperation&, StreamState)>;

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

  const Query& getQuery(int index) const {
    return queries_.getQuery(index);
  }

  const std::vector<Query>& getQueries() const {
    return queries_.getQueries();
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
  friend class MysqlClientBase;

  MultiQueryStreamOperation(
      std::unique_ptr<FetchOperationImpl> opImpl,
      MultiQuery&& multi_query);
  MultiQueryStreamOperation(
      std::unique_ptr<FetchOperationImpl> opImpl,
      std::vector<Query>&& queries);

 private:
  // wrapper to invoke the stream callback
  void invokeCallback(StreamState state);

  StreamCallback stream_callback_;
};

} // namespace facebook::common::mysql_client
