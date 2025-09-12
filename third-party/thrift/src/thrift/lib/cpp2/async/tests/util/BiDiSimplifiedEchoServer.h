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

#include <thrift/lib/cpp2/async/tests/util/BiDiTestUtil.h>

namespace apache::thrift::detail::test {

class BiDiSimplifiedEchoServer
    : public BiDiServerCallback,
      public ManagedStateBase<BiDiSimplifiedEchoServer> {
 public:
  explicit BiDiSimplifiedEchoServer() : ManagedStateBase("Server") {
    firstResponseReceived();
  }

  void resetClientCallback(BiDiClientCallback& clientCallback) override {
    DestructionGuard dg(this);
    clientCallback_ = &clientCallback;
  }

  bool onSinkNext(StreamPayload&&) override {
    DestructionGuard dg(this);
    LOG(INFO) << "Server received sink chunk #" << chunksReceived_++;
    if (isStreamOpen()) {
      std::ignore = clientCallback_->onStreamNext(makeStreamPayload());
    }
    return isAlive();
  }

  bool onSinkError(folly::exception_wrapper ew) override {
    DestructionGuard dg(this);
    LOG(INFO) << "Server received sink error";
    closeSink();
    if (isStreamOpen()) {
      closeStream();
      std::ignore = clientCallback_->onStreamError(std::move(ew));
    }
    return isAlive();
  }

  bool onSinkComplete() override {
    DestructionGuard dg(this);
    LOG(INFO) << "Server received sink complete";
    closeSink();
    if (isStreamOpen()) {
      closeStream();
      std::ignore = clientCallback_->onStreamComplete();
    }
    return isAlive();
  }

  bool onStreamCancel() override {
    DestructionGuard dg(this);
    LOG(INFO) << "Server received stream cancel";
    closeStream();
    if (isSinkOpen()) {
      closeSink();
      std::ignore = clientCallback_->onSinkCancel();
    }
    return isAlive();
  }

  bool onStreamRequestN(uint64_t n) override {
    DestructionGuard dg(this);
    LOG(INFO) << "Server received stream requestN " << n;
    return isAlive();
  }

 protected:
  BiDiClientCallback* clientCallback_ = nullptr;
  size_t chunksReceived_{0};
};
} // namespace apache::thrift::detail::test
