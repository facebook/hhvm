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

#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>

namespace apache::thrift::rocket {

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
        std::ignore = client_.sendError(streamId_, std::move(rex));
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
        std::ignore = client_.sendError(
            streamId_,
            RocketException(ErrorCode::APPLICATION_ERROR, ew.what()));
      });
}

bool RocketSinkServerCallback::onSinkComplete() {
  DCHECK(state_ == State::BothOpen);
  state_ = State::StreamOpen;
  std::ignore = client_.sendComplete(streamId_, false);
  return true;
}

bool RocketSinkServerCallback::onInitialPayload(
    FirstResponsePayload&& payload, folly::EventBase* evb) {
  return clientCallback_->onFirstResponse(std::move(payload), evb, this);
}

void RocketSinkServerCallback::onInitialError(folly::exception_wrapper ew) {
  clientCallback_->onFirstResponseError(std::move(ew));
  std::ignore =
      client_.sendError(streamId_, RocketException(ErrorCode::CANCELED));
}

void RocketSinkServerCallback::onFinalResponse(StreamPayload&& payload) {
  clientCallback_->onFinalResponse(std::move(payload));
}

void RocketSinkServerCallback::onFinalResponseError(
    folly::exception_wrapper ew) {
  ew.handle(
      [&](RocketException& ex) {
        if (ex.hasErrorData()) {
          clientCallback_->onFinalResponseError(
              thrift::detail::EncodedStreamRpcError(ex.moveErrorData()));
        } else {
          clientCallback_->onFinalResponseError(std::move(ew));
        }
      },
      [&](...) { clientCallback_->onFinalResponseError(std::move(ew)); });
}

void RocketSinkServerCallback::onSinkRequestN(uint64_t tokens) {
  switch (state_) {
    case State::BothOpen:
      std::ignore = clientCallback_->onSinkRequestN(tokens);
      return;
    case State::StreamOpen:
      return;
    default:
      folly::assume_unreachable();
  }
}
} // namespace apache::thrift::rocket
