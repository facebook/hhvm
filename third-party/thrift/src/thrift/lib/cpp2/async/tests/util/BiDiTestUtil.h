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

#include <memory>
#include <string>

#include <glog/logging.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/tests/util/Util.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::detail::test {

std::unique_ptr<folly::IOBuf> makePayload(const std::string& s);
std::unique_ptr<folly::IOBuf> makeRequest(const std::string& method);
std::unique_ptr<folly::IOBuf> makeResponse(const std::string& s);

StreamPayload makeStreamPayload();
StreamPayload makeSinkPayload();

class CompletionSignal {
 public:
  folly::coro::Task<void> waitForDone() { co_return co_await done_; }
  void post() { done_.post(); }

 protected:
  folly::coro::Baton done_;
};

struct SimpleStateBase {
 public:
  size_t chunksSent() const { return chunksSent_; }
  size_t chunksReceived() const { return chunksReceived_; }

 protected:
  explicit SimpleStateBase(const std::string& name) : name_(name) {}

  bool isSinkOpen() const { return sinkOpen_; }
  bool isStreamOpen() const { return streamOpen_; }
  bool isAlive() const { return sinkOpen_ || streamOpen_; }
  bool isTerminal() const { return !isAlive(); }

  void closeSink() {
    DCHECK(sinkOpen_) << "Sink already closed";
    sinkOpen_ = false;
    if (isTerminal()) {
      LOG(INFO) << red_ << "..." << name_
                << " closed sink and reached terminal state" << reset_;
    }
  }

  void closeStream() {
    DCHECK(streamOpen_) << "Stream already closed";
    streamOpen_ = false;
    if (isTerminal()) {
      LOG(INFO) << red_ << "..." << name_
                << " closed stream and reached terminal state" << reset_;
    }
  }

  size_t chunksSent_{0};
  size_t chunksReceived_{0};

 private:
  const std::string name_;
  bool sinkOpen_{true};
  bool streamOpen_{true};

  static inline constexpr auto red_ = "\x1b[31m";
  static inline constexpr auto reset_ = "\x1b[0m";
};

// How to use ManagedStateBase
// 1. construct an instance of DestructionGuard on every method of
// BiDi{Client|Server}Callback.
// 2. std::ignore the result returned by the delegate callback.
// 3. return isAlive() from all the methods that return bool.
// 4. call firstResponseReceived() onFirstResponse
// 5. call closeSink on sink complete or error, call closeStream on stream
// complete or error.
template <typename T>
class ManagedStateBase {
 public:
  struct DestructionGuard {
    explicit DestructionGuard(T* self) : self_(self) { self_->depth_++; }
    ~DestructionGuard() {
      self_->depth_--;
      if (self_->depth_ == 0 && !self_->isSinkOpen() &&
          !self_->isStreamOpen()) {
        delete self_;
      }
    }
    T* self_;
  };

  bool isAlive() const { return sinkOpen_ || streamOpen_; }
  bool isTerminal() const { return !isAlive(); }

  bool isSinkOpen() const { return sinkOpen_; }
  bool isStreamOpen() const { return streamOpen_; }

  void closeSink() {
    sinkOpen_ = false;
    if (isTerminal()) {
      LOG(INFO) << name_ << " closed sink and reached terminal state";
    }
  }

  void closeStream() {
    streamOpen_ = false;
    if (isTerminal()) {
      LOG(INFO) << name_ << " closed stream and reached terminal state";
    }
  }

  void firstResponseReceived() {
    sinkOpen_ = true;
    streamOpen_ = true;
  }

 protected:
  explicit ManagedStateBase(const std::string& name) : name_(name) {}

  const std::string name_;
  bool sinkOpen_{false};
  bool streamOpen_{false};
  size_t depth_{0};
  friend struct DestructionGuard;
};

} // namespace apache::thrift::detail::test
