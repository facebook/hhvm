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

// This is a server that will send N chunks and then complete, no matter what
// happens to the sink.
class BiDiFiniteServer : public BiDiServerCallback, public SimpleStateBase {
 public:
  BiDiFiniteServer(size_t chunksToSend, size_t maxChunksToReceive)
      : SimpleStateBase("Server"),
        chunksToSend_(chunksToSend),
        maxChunksToReceive_(maxChunksToReceive) {}

  void resetClientCallback(BiDiClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

  bool onSinkNext(StreamPayload&&) override {
    DCHECK(isSinkOpen())
        << "We can only receive a sink chunk when sink is open";

    LOG(INFO) << "Server received sink chunk #" << chunksReceived_++
              << " and will request 1 more";

    if (!clientCallback_->onSinkRequestN(1)) {
      return false;
    }

    if (chunksReceived_ == maxChunksToReceive_) {
      LOG(INFO) << "  Server received all the chunks it could receive ("
                << chunksReceived_ << "), the limit is " << maxChunksToReceive_
                << ", it will now perform the SinkLimitAction";

      switch (sinkLimitAction_) {
        case SinkLimitAction::FATAL: {
          LOG(FATAL) << "onSinkNext";
        }
        case SinkLimitAction::CANCEL_SINK: {
          LOG(INFO) << "    Server will CANCEL the sink";

          closeSink();
          bool wasTerminal = isTerminal();
          if (!clientCallback_->onSinkCancel()) {
            if (wasTerminal) {
              LOG(INFO) << "  Server reached terminal state, deleting server";
              delete this;
            }
            return false;
          }

        } break;
      }
    }

    DCHECK(isAlive());
    return true;
  }

  bool onSinkError(folly::exception_wrapper ew) override {
    DCHECK(isSinkOpen()) << "We can only error a sink that's open";

    LOG(INFO) << "Server received sink error \"" << ew.what() << "\"";
    closeSink();

    if (isTerminal()) {
      LOG(INFO) << "  Server reached terminal state, deleting server";
      delete this;
      return false;
    }
    return true;
  }

  bool onSinkComplete() override {
    DCHECK(isSinkOpen()) << "We can only complete a sink that's open";

    LOG(INFO) << "Server received sink complete";
    closeSink();

    if (isTerminal()) {
      LOG(INFO) << "  Server reached terminal state, deleting server";
      delete this;
      return false;
    }
    return true;
  }

  bool onStreamRequestN(int32_t n) override {
    DCHECK(isStreamOpen()) << "We can only requestN when stream is open";

    LOG(INFO) << "Server received stream requestN " << n
              << " and will now send as many chunks as it has";

    if (!clientCallback_->onSinkRequestN(n)) {
      return false;
    }

    for (int32_t i = 0; (i < n && chunksToSend_ > 0); i++) {
      LOG(INFO) << "  Server will send stream chunk #" << chunksSent_++
                << ", it has " << chunksToSend_ << " chunks left";
      if (!clientCallback_->onStreamNext(makeStreamPayload())) {
        return false;
      }
      chunksToSend_--;
    }

    if (chunksToSend_ == 0) {
      LOG(INFO)
          << "  Server has no more chunks to send, it will now perform the StreamLimitAction";
      switch (streamLimitAction_) {
        case StreamLimitAction::FATAL: {
          LOG(FATAL) << "onStreamRequestN";
        }
        case StreamLimitAction::COMPLETE: {
          LOG(INFO) << "    Server will COMPLETE the stream";
          closeStream();
          bool wasTerminal = isTerminal();
          if (!clientCallback_->onStreamComplete()) {
            if (wasTerminal) {
              LOG(INFO) << "  Server reached terminal state, deleting server";
              delete this;
            }
            return false;
          }

        } break;
        case StreamLimitAction::ERROR: {
          LOG(INFO) << "    Server will ERROR the stream";
          closeStream();
          bool wasTerminal = isTerminal();
          if (!clientCallback_->onStreamError(
                  folly::make_exception_wrapper<std::runtime_error>(
                      "Stream limit reached"))) {
            if (wasTerminal) {
              LOG(INFO) << "  Server reached terminal state, deleting server";
              delete this;
            }
            return false;
          }
        } break;
      }
    }

    DCHECK(isAlive());
    return true;
  }

  bool onStreamCancel() override {
    DCHECK(isStreamOpen()) << "We can only cancel a stream that's open";

    LOG(INFO) << "Server received stream cancel";
    closeStream();

    if (isTerminal()) {
      LOG(INFO) << "  Server reached terminal state, deleting server";
      delete this;
      return false;
    }
    return true;
  }

 private:
  BiDiClientCallback* clientCallback_ = nullptr;

  size_t chunksToSend_{0};
  size_t maxChunksToReceive_{0};

 public:
  // What to do when server sent all the chunks it meant to send (chunksToSend)
  enum class StreamLimitAction {
    FATAL, // call LOG(FATAL)
    COMPLETE, // complete the stream
    ERROR, // error the stream
  };

  // What to do when server received all the chunks it could receive
  // (maxChunksToReceive)
  enum class SinkLimitAction {
    FATAL, // call LOG(FATAL)
    CANCEL_SINK, // cancel the sink
  };

  void setStreamLimitAction(StreamLimitAction action) {
    streamLimitAction_ = action;
  }

  void setSinkLimitAction(SinkLimitAction action) { sinkLimitAction_ = action; }

 private:
  StreamLimitAction streamLimitAction_ = StreamLimitAction::FATAL;
  SinkLimitAction sinkLimitAction_ = SinkLimitAction::FATAL;
};

} // namespace apache::thrift::detail::test
