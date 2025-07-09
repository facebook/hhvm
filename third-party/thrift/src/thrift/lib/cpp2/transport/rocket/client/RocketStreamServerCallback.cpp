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

#include <thrift/lib/cpp2/transport/rocket/client/RocketStreamServerCallback.h>

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>

namespace apache::thrift::rocket {

namespace {
template <typename ServerCallback>
class TimeoutCallback : public folly::HHWheelTimer::Callback {
 public:
  explicit TimeoutCallback(ServerCallback& parent) : parent_(parent) {}
  void timeoutExpired() noexcept override { parent_.timeoutExpired(); }

 private:
  ServerCallback& parent_;
};
} // namespace

// RocketStreamServerCallback
bool RocketStreamServerCallback::onStreamRequestN(uint64_t tokens) {
  return client_.sendRequestN(streamId_, tokens);
}
void RocketStreamServerCallback::onStreamCancel() {
  client_.cancelStream(streamId_);
}
bool RocketStreamServerCallback::onSinkHeaders(HeadersPayload&& payload) {
  return client_.sendHeadersPush(streamId_, std::move(payload));
}

bool RocketStreamServerCallback::onInitialPayload(
    FirstResponsePayload&& payload, folly::EventBase* evb) {
  return clientCallback_->onFirstResponse(std::move(payload), evb, this);
}

void RocketStreamServerCallback::onInitialError(folly::exception_wrapper ew) {
  clientCallback_->onFirstResponseError(std::move(ew));
  client_.cancelStream(streamId_);
}
StreamChannelStatusResponse RocketStreamServerCallback::onStreamPayload(
    StreamPayload&& payload) {
  std::ignore = clientCallback_->onStreamNext(std::move(payload));
  return StreamChannelStatus::Alive;
}
StreamChannelStatusResponse RocketStreamServerCallback::onStreamFinalPayload(
    StreamPayload&& payload) {
  auto& client = client_;
  auto streamId = streamId_;
  onStreamPayload(std::move(payload));
  // It's possible that stream was canceled from the client callback. This
  // object may be already destroyed.
  if (client.streamExists(streamId)) {
    return onStreamComplete();
  }
  return StreamChannelStatus::Alive;
}
StreamChannelStatusResponse RocketStreamServerCallback::onStreamComplete() {
  clientCallback_->onStreamComplete();
  return StreamChannelStatus::Complete;
}
StreamChannelStatusResponse RocketStreamServerCallback::onStreamError(
    folly::exception_wrapper ew) {
  ew.handle(
      [&](RocketException& ex) {
        if (ex.hasErrorData()) {
          if (client_.getServerVersion() >= 8) {
            clientCallback_->onStreamError(
                thrift::detail::EncodedStreamRpcError(ex.moveErrorData()));
          } else {
            clientCallback_->onStreamError(
                thrift::detail::EncodedError(ex.moveErrorData()));
          }
        } else {
          clientCallback_->onStreamError(std::move(ew));
        }
      },
      [&](...) {
        clientCallback_->onStreamError(std::move(ew));
        return false;
      });
  return StreamChannelStatus::Complete;
}
void RocketStreamServerCallback::onStreamHeaders(HeadersPayload&& payload) {
  std::ignore = clientCallback_->onStreamHeaders(std::move(payload));
}

// RocketStreamServerCallbackWithChunkTimeout
bool RocketStreamServerCallbackWithChunkTimeout::onStreamRequestN(
    uint64_t tokens) {
  if (credits_ == 0) {
    scheduleTimeout();
  }
  credits_ += tokens;
  return RocketStreamServerCallback::onStreamRequestN(tokens);
}

bool RocketStreamServerCallbackWithChunkTimeout::onInitialPayload(
    FirstResponsePayload&& payload, folly::EventBase* evb) {
  if (credits_ > 0) {
    scheduleTimeout();
  }
  return RocketStreamServerCallback::onInitialPayload(std::move(payload), evb);
}

StreamChannelStatusResponse
RocketStreamServerCallbackWithChunkTimeout::onStreamPayload(
    StreamPayload&& payload) {
  DCHECK(credits_ != 0);
  if (--credits_ != 0) {
    scheduleTimeout();
  } else {
    cancelTimeout();
  }
  return RocketStreamServerCallback::onStreamPayload(std::move(payload));
}
void RocketStreamServerCallbackWithChunkTimeout::timeoutExpired() noexcept {
  onStreamError(folly::make_exception_wrapper<transport::TTransportException>(
      transport::TTransportException::TTransportExceptionType::TIMED_OUT,
      folly::to<std::string>(
          "stream chunk timeout after ", chunkTimeout_.count(), " ms.")));
  onStreamCancel();
}
void RocketStreamServerCallbackWithChunkTimeout::scheduleTimeout() {
  if (!timeout_) {
    timeout_ = std::make_unique<
        TimeoutCallback<RocketStreamServerCallbackWithChunkTimeout>>(*this);
  }
  client_.scheduleTimeout(timeout_.get(), chunkTimeout_);
}
void RocketStreamServerCallbackWithChunkTimeout::cancelTimeout() {
  timeout_.reset();
}

// RocketSinkServerCallback
bool RocketSinkServerCallback::onSinkNext(StreamPayload&& payload) {
  DCHECK(state_ == State::BothOpen);
  // apply compression if client has specified compression codec
  if (compressionConfig_) {
    CompressionManager().setCompressionCodec(
        *compressionConfig_,
        payload.metadata,
        payload.payload->computeChainDataLength());
  }
  std::ignore =
      client_.sendPayload(streamId_, std::move(payload), Flags().next(true));
  return true;
}
void RocketSinkServerCallback::onSinkError(folly::exception_wrapper ew) {
  DCHECK(state_ == State::BothOpen);
  ew.handle(
      [&](RocketException& rex) {
        client_.sendError(streamId_, std::move(rex));
      },
      [this](::apache::thrift::detail::EncodedStreamError& err) {
        if (compressionConfig_) {
          CompressionManager().setCompressionCodec(
              *compressionConfig_,
              err.encoded.metadata,
              err.encoded.payload->computeChainDataLength());
        }
        std::ignore = client_.sendSinkError(streamId_, std::move(err.encoded));
      },
      [&](...) {
        client_.sendError(
            streamId_,
            RocketException(ErrorCode::APPLICATION_ERROR, ew.what()));
      });
}
bool RocketSinkServerCallback::onSinkComplete() {
  DCHECK(state_ == State::BothOpen);
  state_ = State::StreamOpen;
  client_.sendComplete(streamId_, false);
  return true;
}

bool RocketSinkServerCallback::onInitialPayload(
    FirstResponsePayload&& payload, folly::EventBase* evb) {
  return clientCallback_->onFirstResponse(std::move(payload), evb, this);
}
void RocketSinkServerCallback::onInitialError(folly::exception_wrapper ew) {
  clientCallback_->onFirstResponseError(std::move(ew));
  client_.sendError(streamId_, RocketException(ErrorCode::CANCELED));
}
StreamChannelStatusResponse RocketSinkServerCallback::onFinalResponse(
    StreamPayload&& payload) {
  clientCallback_->onFinalResponse(std::move(payload));
  return StreamChannelStatus::Complete;
}
StreamChannelStatusResponse RocketSinkServerCallback::onFinalResponseError(
    folly::exception_wrapper ew) {
  ew.handle(
      [&](RocketException& ex) {
        if (ex.hasErrorData()) {
          if (client_.getServerVersion() >= 8) {
            clientCallback_->onFinalResponseError(
                thrift::detail::EncodedStreamRpcError(ex.moveErrorData()));
          } else {
            clientCallback_->onFinalResponseError(
                thrift::detail::EncodedError(ex.moveErrorData()));
          }
        } else {
          clientCallback_->onFinalResponseError(std::move(ew));
        }
      },
      [&](...) { clientCallback_->onFinalResponseError(std::move(ew)); });
  return StreamChannelStatus::Complete;
}
StreamChannelStatusResponse RocketSinkServerCallback::onSinkRequestN(
    uint64_t tokens) {
  switch (state_) {
    case State::BothOpen:
      std::ignore = clientCallback_->onSinkRequestN(tokens);
      return StreamChannelStatus::Alive;
    case State::StreamOpen:
      return StreamChannelStatus::Alive;
    default:
      folly::assume_unreachable();
  }
}
} // namespace apache::thrift::rocket
