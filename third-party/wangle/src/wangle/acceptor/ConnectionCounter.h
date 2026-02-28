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

#include <cstdint>

namespace wangle {

class IConnectionCounter {
 public:
  virtual uint64_t getNumConnections() const = 0;

  /**
   * Get the maximum number of non-whitelisted client-side connections
   * across all Acceptors managed by this. A value
   * of zero means "unlimited."
   */
  virtual uint64_t getMaxConnections() const = 0;

  /**
   * Increment the count of client-side connections.
   */
  virtual void onConnectionAdded() = 0;

  /**
   * Decrement the count of client-side connections.
   */
  virtual void onConnectionRemoved() = 0;
  virtual ~IConnectionCounter() = default;
};

class SimpleConnectionCounter : public IConnectionCounter {
 public:
  uint64_t getNumConnections() const override {
    return numConnections_;
  }
  uint64_t getMaxConnections() const override {
    return maxConnections_;
  }
  void setMaxConnections(uint64_t maxConnections) {
    maxConnections_ = maxConnections;
  }

  void onConnectionAdded() override {
    numConnections_++;
  }
  void onConnectionRemoved() override {
    numConnections_--;
  }
  ~SimpleConnectionCounter() override = default;

 protected:
  uint64_t maxConnections_{0};
  uint64_t numConnections_{0};
};

} // namespace wangle
