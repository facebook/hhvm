/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
#include "squangle/mysql_client/OperationBatch.h"

#include <thread>

namespace facebook::common::mysql_client {

void OperationBatch::add(std::shared_ptr<Operation> op) {
  CHECK_THROW(
      op->state() == OperationState::Unstarted, db::OperationStateException);
  {
    std::lock_guard<std::mutex> lock(*mutex_);
    num_living_operations_++;
  }

  op->setObserverCallback([this](Operation& op) {
    std::lock_guard<std::mutex> lock(*mutex_);
    if (!op.ok()) {
      successful_ = false;
    }
    if (--num_living_operations_ == 0) {
      currently_idle_->notify_one();
    }
  });
}

/* Only the creator of the OperationBatch can call drain on it */
void OperationBatch::drain() {
  // Drain (and hence our destructor) should be a no-op if mutex_ was
  // std::move'd away (which indicates this batch is the withered husk
  // of a std::move'd batch).
  if (!mutex_) {
    return;
  }

  DCHECK_EQ(std::this_thread::get_id(), creator_thread_id_);

  std::unique_lock<std::mutex> lock(*mutex_);

  // no operation to wait on
  if (num_living_operations_ == 0) {
    return;
  }

  // Now wait for the operations to complete.
  currently_idle_->wait(lock, [this] { return (num_living_operations_ == 0); });
}

bool OperationBatch::ok() {
  std::lock_guard<std::mutex> lock(*mutex_);
  return num_living_operations_ == 0 && successful_;
}

void OperationBatch::markFailure() {
  std::lock_guard<std::mutex> lock(*mutex_);
  successful_ = false;
}

} // namespace facebook::common::mysql_client
