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

class BiDiSimplifiedEchoClient
    : public BiDiClientCallback,
      public ManagedStateBase<BiDiSimplifiedEchoClient> {
 public:
  explicit BiDiSimplifiedEchoClient(
      size_t chunksToSend, std::shared_ptr<CompletionSignal> completion)
      : ManagedStateBase("Client"),
        chunksToSend_(chunksToSend),
        completion_(std::move(completion)) {}

  ~BiDiSimplifiedEchoClient() noexcept override { completion_->post(); }

  void resetServerCallback(BiDiServerCallback& /* serverCallback */) override {
    LOG(FATAL) << "resetServerCallback shouldn't be called on this test client";
  }

  bool onFirstResponse(
      FirstResponsePayload&&,
      folly::EventBase*,
      BiDiServerCallback* serverCallback) override {
    DestructionGuard dg(this);
    firstResponseReceived();
    serverCallback_ = serverCallback;
    LOG(INFO) << "Client received initial response, will send a chunk #"
              << chunksSent_++;
    std::ignore = serverCallback_->onSinkNext(makeSinkPayload());
    return isAlive();
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    DestructionGuard dg(this);
    LOG(INFO) << "Server returned error on first response: " << ew.what();
  }

  bool onStreamNext(StreamPayload&&) override {
    DestructionGuard dg(this);
    LOG(INFO) << "Client received stream chunk #" << chunksReceived_++;
    if (chunksSent_ == chunksToSend_) {
      LOG(INFO) << "Client will complete sink";
      std::ignore = serverCallback_->onSinkComplete();
    } else {
      LOG(INFO) << "Client will send chunk " << chunksSent_++;
      std::ignore = serverCallback_->onSinkNext(makeSinkPayload());
    }

    return isAlive();
  }

  bool onStreamError(folly::exception_wrapper ew) override {
    DestructionGuard dg(this);
    LOG(INFO) << "Client received stream error " << ew.what();
    closeStream();
    closeSink();
    std::ignore = serverCallback_->onSinkError(
        folly::make_exception_wrapper<TApplicationException>("onStreamError"));
    return isAlive();
  }

  bool onStreamComplete() override {
    DestructionGuard dg(this);
    LOG(INFO) << "Client received stream complete, will complete sink";
    closeStream();
    closeSink();
    std::ignore = serverCallback_->onSinkComplete();
    return isAlive();
  }

  bool onSinkRequestN(int32_t n) override {
    DestructionGuard dg(this);
    LOG(INFO) << "Client received sink requestN " << n;
    std::ignore = serverCallback_->onSinkNext(makeSinkPayload());
    return isAlive();
  }

  bool onSinkCancel() override {
    DestructionGuard dg(this);
    LOG(INFO) << "Client received sink cancel";
    closeSink();
    std::ignore = serverCallback_->onStreamCancel();
    return isAlive();
  }

 protected:
  size_t chunksToSend_{0};
  std::shared_ptr<CompletionSignal> completion_;

  BiDiServerCallback* serverCallback_ = nullptr;

  size_t chunksReceived_{0};
  size_t chunksSent_{0};
};
} // namespace apache::thrift::detail::test
