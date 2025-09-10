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

// This is the server that echoes 1 stream chunk for every 1 sink chunk,
// indefinitely.
//
// When sink completes it completes the stream.
// When stream gets cancelled it cancels the sink.
// It deletes itself when it reaches a terminal state.
class BiDiEchoServer : public BiDiServerCallback, public SimpleStateBase {
 public:
  BiDiEchoServer() : SimpleStateBase("Server") {}

  void resetClientCallback(BiDiClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

  bool onSinkNext(StreamPayload&& payload) override {
    DCHECK(isSinkOpen())
        << "We can only receive a sink chunk when sink is open";

    LOG(INFO) << "Server received sink chunk #" << chunksReceived_++
              << ", will request 1 more";
    if (!clientCallback_->onSinkRequestN(1)) {
      return false;
    }

    if (isStreamOpen()) {
      LOG(INFO) << "  Server will echo stream chunk #" << chunksSent_++;
      if (!clientCallback_->onStreamNext(std::move(payload))) {
        return false;
      }
    } else {
      LOG(ERROR)
          << "  Server couldn't send a stream chunk because stream was already closed by client";
    }

    return true;
  }

  bool onSinkError(folly::exception_wrapper ew) override {
    DCHECK(isSinkOpen()) << "We can only error a sink that's open";

    LOG(INFO)
        << "Server received sink error \"" << ew.what()
        << "\", and will echo the ERROR back (because it's an echo server)";
    closeSink();

    if (isStreamOpen()) {
      closeStream();
      DCHECK(!clientCallback_->onStreamError(std::move(ew)));
    } else {
      LOG(ERROR)
          << "  Server couldn't not error the stream because it's already closed";
    }

    DCHECK(isTerminal());
    LOG(INFO) << "  Server reached terminal state, deleting server";
    delete this;
    return false;
  }

  bool onSinkComplete() override {
    DCHECK(isSinkOpen()) << "We can only complete a sink that's open";

    LOG(INFO)
        << "Server received sink complete, it will now COMPLETE the stream (because it's an echo server)";
    closeSink();

    if (isStreamOpen()) {
      closeStream();
      DCHECK(!clientCallback_->onStreamComplete());
    } else {
      LOG(ERROR)
          << "  Server couldn't not complete the stream because it's already closed";
    }

    DCHECK(isTerminal());
    LOG(INFO) << "  Server reached terminal state, deleting server";
    delete this;
    return false;
  }

  bool onStreamCancel() override {
    DCHECK(isStreamOpen()) << "We can only cancel a stream that's open";

    LOG(INFO)
        << "Server received stream cancel, it will now CANCEL the sink (because it's an echo server)";
    closeStream();

    if (isSinkOpen()) {
      closeSink();
      DCHECK(!clientCallback_->onSinkCancel());
    } else {
      LOG(ERROR)
          << "  Server couldn't not cancel the sink because it's already closed";
    }

    DCHECK(isTerminal());
    LOG(INFO) << "  Server reached terminal state, deleting server";
    delete this;
    return false;
  }

  bool onStreamRequestN(uint64_t n) override {
    DCHECK(isStreamOpen()) << "We can only requestN when stream is open";

    LOG(INFO) << "Server received stream requestN " << n
              << " and will requestN " << n << " sink chunks";
    if (!clientCallback_->onSinkRequestN(n)) {
      return false;
    }
    return true;
  }

 private:
  BiDiClientCallback* clientCallback_ = nullptr;
};
} // namespace apache::thrift::detail::test
