/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// The OperationBatch is a class that allows waiting only for a
// certain set of operations. All the operations (Connect, Query)
// must be added to the batch and then drain() can be called to
// wait for completion.
//
// It is useful for parallel queries. Because usually
// the FbAsyncMysqlClient::defaultClient() will be used and it is
// shared among all the threads, calling drain on the client is not
// an option. This class allows waiting only on the operations started
// by that thread.
// An usage example can be found in the TestOperationBatch UNITTEST
// from the AsyncMysqlTest.cpp file.

#pragma once

#include <mutex>
#include <unordered_map>

#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

class OperationBatch {
 public:
  explicit OperationBatch()
      : mutex_(new std::mutex),
        currently_idle_(new std::condition_variable),
        num_living_operations_(0),
        creator_thread_id_(std::this_thread::get_id()),
        successful_(true) {}

  OperationBatch(OperationBatch&& other) = default;
  OperationBatch& operator=(OperationBatch&& other) = default;

  ~OperationBatch() {
    drain();
  }

  void add(std::shared_ptr<Operation> op);

  void drain();

  // Returns false if there was a failure - either an operation failed or
  // it was marked manually with markFailure()
  bool ok();

  // Offers the possibility to mark from a callback that the batch was not
  // successful
  // If any operation failed, it  will be marked automatically as failed.
  void markFailure();

 private:
  // mutex_ is  used by currently_idle_ condition variable and to protect
  // num_living_operations_ and successful_ variables
  std::unique_ptr<std::mutex> mutex_;
  std::unique_ptr<std::condition_variable> currently_idle_;

  // Counter for the number of living operations
  // This is used for draining ;
  uint32_t num_living_operations_;

  std::thread::id creator_thread_id_;

  // Indicator of the success of the batch
  bool successful_;
};

} // namespace facebook::common::mysql_client
