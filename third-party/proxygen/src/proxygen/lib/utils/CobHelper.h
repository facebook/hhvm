/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <functional>

#include <glog/logging.h>

namespace proxygen {

/**
 * A helper class to schedule N async jobs in (possibly) different threads
 * and invoke a final callback in a given thread once all are done.
 */
class CobHelper {
 public:
  CobHelper(size_t itemsLeft,
            const std::function<void()>& cob,
            const std::function<void(const std::exception&)>& ecob)
      : itemsLeft_(itemsLeft), cob_(cob), ecob_(ecob) {
  }

  void setError(const std::string& emsg) {
    CHECK(!emsg.empty());
    emsg_ = emsg;
  }

  void workerDone() {
    uint32_t oldValue = itemsLeft_.fetch_sub(1);
    if (oldValue != 1) {
      return;
    }

    allDone();
  }

 private:
  void allDone() {
    if (!emsg_.empty()) {
      ecob_(std::runtime_error(emsg_));
    } else {
      cob_();
    }

    delete this;
  }

  std::atomic<uint32_t> itemsLeft_;
  std::string emsg_;
  std::function<void()> cob_;
  std::function<void(const std::exception&)> ecob_;
};

} // namespace proxygen
