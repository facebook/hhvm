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

class BiDiConfigurableServer : public BiDiServerCallback,
                               public SimpleStateBase {
 public:
  BiDiConfigurableServer() : SimpleStateBase("Server") {}

  void resetClientCallback(BiDiClientCallback& clientCallback) override {
    clientCallback_ = &clientCallback;
  }

  bool onSinkNext(StreamPayload&& payload) override {
    DCHECK(isSinkOpen())
        << "We can only receive a sink chunk when sink is open";

    LOG(INFO) << "Server received sink chunk #" << chunksReceived_++
              << " and will request 1 more";
    if (!clientCallback_->onSinkRequestN(1)) {
      return false;
    }

    LOG(INFO) << "  Server will echo stream chunk #" << chunksSent_++;
    if (!clientCallback_->onStreamNext(std::move(payload))) {
      return false;
    }

    return true;
  }

  bool onSinkError(folly::exception_wrapper ew) override {
    DCHECK(isSinkOpen()) << "We can only error a sink that's open";

    LOG(INFO) << "Server received sink error \"" << ew.what()
              << "\" and will perform configured SinkErrorAction";
    closeSink();
    bool wasTerminal = isTerminal();

    switch (sinkErrorAction_) {
      case SinkErrorAction::FATAL: {
        LOG(FATAL) << "onSinkError";
      } break;
      case SinkErrorAction::IGNORE: {
        // do nothing
      } break;
      case SinkErrorAction::ERROR_STREAM: {
        DCHECK(isStreamOpen()) << "We can only error a stream that's open";

        LOG(INFO) << "  Server will error the stream";
        closeStream();
        DCHECK(isTerminal());
        auto result = clientCallback_->onStreamError(
            folly::make_exception_wrapper<std::runtime_error>(
                "Server received sink error"));
        DCHECK(!result);
      } break;
      case SinkErrorAction::CONTINUE_STREAM: {
        // This statement guarantees that wasTerminal == false
        DCHECK(isStreamOpen()) << "We can only continue a stream that's open";

        LOG(INFO) << "  Server will continue the stream";
        LOG(INFO) << "    Server will send stream chunk #" << chunksSent_++;
        if (!clientCallback_->onStreamNext(makeStreamPayload())) {
          DCHECK(!wasTerminal)
              << "Couldn't be in terminal state prior to the onStreamNext call";
          return false;
        }
        LOG(INFO) << "    Server will send stream chunk #" << chunksSent_++;
        if (!clientCallback_->onStreamNext(makeStreamPayload())) {
          DCHECK(!wasTerminal)
              << "Couldn't be in terminal state prior to the onStreamNext call";
          return false;
        }

        LOG(INFO) << "  Server will now COMPLETE the stream";
        closeStream();
        DCHECK(isTerminal());
        DCHECK(!clientCallback_->onStreamComplete());
      } break;
      case SinkErrorAction::COMPLETE_STREAM: {
        DCHECK(isStreamOpen());

        LOG(INFO) << "  Server will now COMPLETE the stream";
        closeStream();
        DCHECK(isTerminal());
        DCHECK(!clientCallback_->onStreamComplete());
      } break;
    };

    if (isTerminal()) {
      LOG(INFO) << "  Server reached terminal state, deleting server";
      delete this;
      return false;
    }
    return true;
  }

  bool onSinkComplete() override {
    DCHECK(isSinkOpen()) << "We can only complete a sink that's open";

    LOG(INFO)
        << "Server received sink complete and will perform configured SinkCompleteAction";
    closeSink();
    bool wasTerminal = isTerminal();

    switch (sinkCompleteAction_) {
      case SinkCompleteAction::FATAL: {
        LOG(FATAL) << "onSinkComplete";
      } break;
      case SinkCompleteAction::IGNORE: {
        // do nothing
      } break;
      case SinkCompleteAction::ERROR_STREAM: {
        DCHECK(isStreamOpen()) << "We can only complete a stream that's open";

        LOG(INFO) << "  Server will ERROR the stream";
        closeStream();
        DCHECK(isTerminal());
        DCHECK(!clientCallback_->onStreamError(
            folly::make_exception_wrapper<std::runtime_error>(
                "Server received sink complete")));
      } break;
      case SinkCompleteAction::CONTINUE_STREAM: {
        // This statement guarantees that wasTerminal == false
        DCHECK(isStreamOpen()) << "We can only complete a stream that's open";

        LOG(INFO) << "  Server will continue the stream";
        LOG(INFO) << "    Server will send stream chunk #" << chunksSent_++;
        if (!clientCallback_->onStreamNext(makeStreamPayload())) {
          DCHECK(!wasTerminal)
              << "Couldn't be in terminal state prior to the onStreamNext call";
          return false;
        }
        LOG(INFO) << "    Server will send stream chunk #" << chunksSent_++;
        if (!clientCallback_->onStreamNext(makeStreamPayload())) {
          DCHECK(!wasTerminal)
              << "Couldn't be in terminal state prior to the onStreamNext call";
          return false;
        }

        LOG(INFO) << "  Server will now COMPLETE the stream";
        closeStream();
        DCHECK(isTerminal());
        DCHECK(!clientCallback_->onStreamComplete());
      } break;
      case SinkCompleteAction::COMPLETE_STREAM: {
        DCHECK(isStreamOpen());

        LOG(INFO) << "  Server will now COMPLETE the stream";
        closeStream();
        DCHECK(isTerminal());
        DCHECK(!clientCallback_->onStreamComplete());
      } break;
    }

    if (isTerminal()) {
      LOG(INFO) << "  Server reached terminal state, deleting server";
      delete this;
      return false;
    }
    return true;
  }

  bool onStreamRequestN(uint64_t n) override {
    LOG(INFO) << "Server received stream requestN " << n
              << " and will requestN " << n << " sink chunks";
    return clientCallback_->onSinkRequestN(n);
  }

  bool onStreamCancel() override {
    DCHECK(isStreamOpen()) << "We can only cancel a stream that's open";

    LOG(INFO)
        << "Server received stream cancel and will perform configured StreamCancelAction";
    closeStream();

    switch (streamCancelAction_) {
      case StreamCancelAction::FATAL: {
        LOG(FATAL) << "onStreamCancel";
      } break;
      case StreamCancelAction::IGNORE: {
        // do nothing
      } break;
      case StreamCancelAction::CANCEL_SINK: {
        LOG(INFO) << "  Server will CANCEL the sink";
        closeSink();
        DCHECK(isTerminal());
        DCHECK(!clientCallback_->onSinkCancel());
      } break;
    }

    if (isTerminal()) {
      LOG(INFO) << "  Server reached terminal state, deleting server";
      delete this;
      return false;
    }
    return true;
  }

 private:
  BiDiClientCallback* clientCallback_ = nullptr;

 public:
  enum class SinkErrorAction {
    FATAL, // call LOG(FATAL)
    IGNORE, // do nothing, might lead to a deadlock
    ERROR_STREAM, // error the stream immediately
    CONTINUE_STREAM, // send 2 more payloads and then complete
    COMPLETE_STREAM, // complete stream immediately
  };

  enum class SinkCompleteAction {
    FATAL, // call LOG(FATAL)
    IGNORE, // do nothing, might lead to a deadlock
    ERROR_STREAM, // error the stream immediately
    CONTINUE_STREAM, // send 2 more payloads and then complete
    COMPLETE_STREAM, // complete stream immediately
  };

  enum class StreamCancelAction {
    FATAL, // call LOG(FATAL)
    IGNORE, // do nothing
    CANCEL_SINK,
  };

  void setSinkErrorAction(SinkErrorAction action) { sinkErrorAction_ = action; }
  void setSinkCompleteAction(SinkCompleteAction action) {
    sinkCompleteAction_ = action;
  }
  void setStreamCancelAction(StreamCancelAction action) {
    streamCancelAction_ = action;
  }

 private:
  SinkErrorAction sinkErrorAction_ = SinkErrorAction::FATAL;
  SinkCompleteAction sinkCompleteAction_ = SinkCompleteAction::FATAL;
  StreamCancelAction streamCancelAction_ = StreamCancelAction::FATAL;
};

} // namespace apache::thrift::detail::test
