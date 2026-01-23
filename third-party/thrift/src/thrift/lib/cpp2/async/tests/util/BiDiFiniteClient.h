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

// It's the client that has N chunks to send. When it's done sending them, it
// will either complete, or error or cancel stream.
class BiDiFiniteClient : public BiDiClientCallback, public SimpleStateBase {
 public:
  BiDiFiniteClient(
      size_t chunksToSend,
      size_t maxChunksToReceive,
      std::shared_ptr<CompletionSignal> done)
      : SimpleStateBase("Client"),
        chunksToSend_(chunksToSend),
        maxChunksToReceive_(maxChunksToReceive),
        done_(std::move(done)) {}

  ~BiDiFiniteClient() noexcept override { done_->post(); }

  BiDiFiniteClient(const BiDiFiniteClient&) = delete;
  BiDiFiniteClient& operator=(const BiDiFiniteClient&) = delete;
  BiDiFiniteClient(BiDiFiniteClient&&) noexcept = delete;
  BiDiFiniteClient& operator=(BiDiFiniteClient&&) noexcept = delete;

  void resetServerCallback(BiDiServerCallback& /* serverCallback */) override {
    LOG(FATAL) << "resetServerCallback shouldn't be called on this test client";
  }

  bool onFirstResponse(
      FirstResponsePayload&&,
      folly::EventBase*,
      BiDiServerCallback* serverCallback) override {
    serverCallback_ = serverCallback;
    LOG(INFO) << "Client received initial response";
    return true;
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    LOG(INFO) << "Server returned error on first response: " << ew.what();
    delete this;
  }

  bool onStreamNext(StreamPayload&&) override {
    DCHECK(isStreamOpen())
        << "We can only receive stream chunks when stream is open";
    DCHECK(serverCallback_->onStreamRequestN(1));

    LOG(INFO) << "Client received stream chunk #" << chunksReceived_++;
    if (chunksReceived_ == maxChunksToReceive_) {
      LOG(INFO) << "  Client reached the limit (" << chunksReceived_ << "/"
                << maxChunksToReceive_
                << ") and will perform StreamLimitAction";

      switch (streamLimitAction_) {
        case StreamLimitAction::FATAL:
          LOG(FATAL) << "onStreamNext";
        case StreamLimitAction::CANCEL_STREAM: {
          LOG(INFO) << "    Client will CANCEL the stream";
          closeStream();
          bool wasTerminal = isTerminal();
          if (!serverCallback_->onStreamCancel()) {
            if (wasTerminal) {
              LOG(INFO) << "  Client reached terminal state, deleting client";
              delete this;
            }
            return false;
          }
        } break;
      }
    }

    if (isTerminal()) {
      LOG(INFO) << "  Client reached terminal state, deleting client";
      delete this;
      return false;
    }
    return true;
  }

  bool onStreamError(folly::exception_wrapper ew) override {
    DCHECK(isStreamOpen()) << "We can only error a stream that's open";

    LOG(INFO) << "Client received stream error " << ew.what();
    closeStream();

    if (isTerminal()) {
      delete this;
      return false;
    }
    return true;
  }

  bool onStreamComplete() override {
    DCHECK(isStreamOpen()) << "We can only complete a stream that's open";

    LOG(INFO) << "Client received stream complete";
    closeStream();

    if (isTerminal()) {
      delete this;
      return false;
    }
    return true;
  }

  bool onSinkRequestN(int32_t n) override {
    DCHECK(isSinkOpen()) << "We can only requestN on a sink that's open";

    LOG(INFO) << "Client received sink requestN " << n << ", it has "
              << chunksToSend_ << " more chunks to send";

    for (int32_t i = 0; i < n && chunksToSend_ > 0; i++) {
      LOG(INFO) << "  Client will send sink chunk #" << chunksSent_++
                << ", it has " << --chunksToSend_
                << " stream chunks left to send";
      if (!serverCallback_->onSinkNext(makeSinkPayload())) {
        return false;
      }
    }

    if (chunksToSend_ == 0) {
      LOG(INFO)
          << "  Client has no more chunks to send, will perform the SinkLimitAction";

      switch (sinkLimitAction_) {
        case SinkLimitAction::FATAL:
          LOG(FATAL) << "onSinkRequestN";
        case SinkLimitAction::COMPLETE: {
          LOG(INFO) << "    Client will COMPLETE the sink";
          closeSink();
          bool wasTerminal = isTerminal();
          if (!serverCallback_->onSinkComplete()) {
            if (wasTerminal) {
              LOG(INFO) << "  Client reached terminal state, deleting client";
              delete this;
            }
            return false;
          }

        } break;
        case SinkLimitAction::ERROR: {
          LOG(INFO) << "    Client will ERROR the sink";
          closeSink();
          bool wasTerminal = isTerminal();
          if (!serverCallback_->onSinkError(
                  folly::make_exception_wrapper<std::runtime_error>(
                      "Sink limit reached"))) {
            if (wasTerminal) {
              LOG(INFO) << "  Client reached terminal state, deleting client";
              delete this;
            }
            return false;
          }
          break;
        }
      }
    }

    if (isTerminal()) {
      delete this;
      return false;
    }
    return true;
  }

  bool onSinkCancel() override {
    DCHECK(isSinkOpen());

    LOG(INFO) << "Client received sink cancel";
    closeSink();

    if (isTerminal()) {
      delete this;
      return false;
    }
    return true;
  }

 private:
  BiDiServerCallback* serverCallback_ = nullptr;

  size_t chunksToSend_;
  size_t maxChunksToReceive_;

  std::shared_ptr<CompletionSignal> done_;

 public:
  // What to do when we sent all the chunks we had
  enum class SinkLimitAction {
    FATAL, // call LOG(FATAL)
    COMPLETE, // complete the sink and wait for terminal state produced by the
              // server OR by achieving the stream limit
    ERROR, // error the sink and wait for terminal state produced by the server
           // OR by achieving the stream limit
  };

  // What to do when we received all the chunks we could
  enum class StreamLimitAction {
    FATAL, // call LOG(FATAL)
    CANCEL_STREAM
  };

  void setSinkLimitAction(SinkLimitAction action) { sinkLimitAction_ = action; }

  void setStreamLimitAction(StreamLimitAction action) {
    streamLimitAction_ = action;
  }

 private:
  SinkLimitAction sinkLimitAction_ = SinkLimitAction::FATAL;
  StreamLimitAction streamLimitAction_ = StreamLimitAction::FATAL;
};
} // namespace apache::thrift::detail::test
