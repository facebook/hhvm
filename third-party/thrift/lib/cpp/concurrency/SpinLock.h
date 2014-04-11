/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#include "folly/SmallLocks.h"
#include <glog/logging.h>

namespace apache { namespace thrift { namespace concurrency {

class SpinLock {
 public:
  SpinLock() {
    lock_.init();
  }
  void lock() const {
    lock_.lock();
  }
  void unlock() const {
    lock_.unlock();
  }
  bool trylock() const {
    return lock_.try_lock();
  }
  ~SpinLock() {

  }
 private:
  mutable folly::MicroSpinLock lock_;
};

class SpinLockGuard {
 public:
  explicit SpinLockGuard(SpinLock& lock) : lock_(lock) {
    lock_.lock();
  }
  ~SpinLockGuard() {
    lock_.unlock();
  }
 private:
  SpinLock& lock_;
};

}}}
