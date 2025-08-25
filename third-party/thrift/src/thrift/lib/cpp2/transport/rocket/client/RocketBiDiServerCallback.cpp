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

#include <thrift/lib/cpp2/transport/rocket/client/RocketBiDiServerCallback.h>

#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>

namespace apache::thrift::rocket {

bool RocketBiDiServerCallback::onSinkNext(StreamPayload&& payload) {
  DCHECK(state_.sinkAlive());
  // apply compression if client has specified compression codec
  if (compressionConfig_) {
    CompressionManager().setCompressionCodec(
        *compressionConfig_,
        payload.metadata,
        payload.payload->computeChainDataLength());
  }
  return client_.sendPayload(streamId_, std::move(payload), Flags().next(true));
}

bool RocketBiDiServerCallback::onSinkError(folly::exception_wrapper ew) {
  state_.closeSink();
  bool alive = state_.streamAlive();
  ew.handle(
      [&](RocketException& rex) {
        alive = client_.sendError(streamId_, std::move(rex), !alive) && alive;
      },
      [&](::apache::thrift::detail::EncodedStreamError& err) {
        if (compressionConfig_) {
          CompressionManager().setCompressionCodec(
              *compressionConfig_,
              err.encoded.metadata,
              err.encoded.payload->computeChainDataLength());
        }
        alive =
            client_.sendSinkError(streamId_, std::move(err.encoded), !alive) &&
            alive;
      },
      [&](...) {
        alive = client_.sendError(
                    streamId_,
                    RocketException(ErrorCode::APPLICATION_ERROR, ew.what()),
                    !alive) &&
            alive;
      });
  return alive;
}

bool RocketBiDiServerCallback::onSinkComplete() {
  state_.closeSink();
  bool alive = state_.streamAlive();
  return client_.sendComplete(streamId_, !alive) && alive;
}

bool RocketBiDiServerCallback::onInitialPayload(
    FirstResponsePayload&& payload, folly::EventBase* evb) {
  state_.markFirstResponseSent();
  return clientCallback_->onFirstResponse(std::move(payload), evb, this);
}

void RocketBiDiServerCallback::onInitialError(folly::exception_wrapper ew) {
  state_.earlyClose();
  clientCallback_->onFirstResponseError(std::move(ew));
  std::ignore =
      client_.sendError(streamId_, RocketException(ErrorCode::CANCELED));
}

void RocketBiDiServerCallback::onSinkRequestN(uint64_t tokens) {
  if (state_.sinkAlive()) {
    std::ignore = clientCallback_->onSinkRequestN(tokens);
  }
}

void RocketBiDiServerCallback::onSinkCancel() {
  if (state_.sinkAlive()) {
    state_.closeSink();
    std::ignore = clientCallback_->onSinkCancel();
  }
}

bool RocketBiDiServerCallback::onStreamRequestN(uint64_t tokens) {
  return client_.sendRequestN(streamId_, tokens);
}

bool RocketBiDiServerCallback::onStreamCancel() {
  state_.closeStream();
  bool alive = state_.sinkAlive();
  client_.cancelStream(streamId_, !alive);
  return alive;
}

bool RocketBiDiServerCallback::onStreamPayload(StreamPayload&& payload) {
  return clientCallback_->onStreamNext(std::move(payload));
}

bool RocketBiDiServerCallback::onStreamFinalPayload(StreamPayload&& payload) {
  state_.closeStream();
  return onStreamPayload(std::move(payload)) && onStreamComplete();
}

bool RocketBiDiServerCallback::onStreamComplete() {
  state_.closeStream();
  return clientCallback_->onStreamComplete();
}

void RocketBiDiServerCallback::onStreamError(folly::exception_wrapper ew) {
  state_.closeStream();
  ew.handle(
      [&](RocketException& ex) {
        if (ex.hasErrorData()) {
          std::ignore = clientCallback_->onStreamError(
              thrift::detail::EncodedStreamRpcError(ex.moveErrorData()));
        } else {
          std::ignore = clientCallback_->onStreamError(std::move(ew));
        }
      },
      [&](...) {
        std::ignore = clientCallback_->onStreamError(std::move(ew));
      });
}

} // namespace apache::thrift::rocket
