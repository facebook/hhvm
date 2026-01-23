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

#include <thrift/lib/cpp2/transport/rocket/server/RocketBiDiClientCallback.h>

namespace apache::thrift::rocket {

//
//  BiDiClientCallback methods, called by server-side (bridge).
//  These are "outgoing" methods, e.g. server calls client
//

bool RocketBiDiClientCallback::onFirstResponse(
    FirstResponsePayload&& firstResponse,
    folly::EventBase* /* evb */,
    BiDiServerCallback* serverCallback) {
  if (state_.isCancelledEarly()) {
    if (serverCallback_->onStreamCancel()) {
      std::ignore = serverCallback_->onSinkComplete();
    }

    DCHECK(state_.isTerminal());
    return freeStreamAndReturn(false);
  }

  DCHECK(state_.isAwaitingFirstResponse());

  DCHECK(serverCallback_ == nullptr) << "Server callback must not be set yet";
  serverCallback_ = serverCallback;

  firstResponse.metadata.streamId() = static_cast<uint32_t>(streamId_);
  sendPayload(std::move(firstResponse), /* next= */ true);
  state_.onFirstResponseSent();

  if (initialTokens_ > 0) {
    onStreamRequestN(initialTokens_);
  }

  DCHECK(state_.isAlive());
  return true;
}

void RocketBiDiClientCallback::onFirstResponseError(
    folly::exception_wrapper ew) {
  state_.onFirstResponseError();
  ew.handle(
      [this](RocketException& rex) {
        connection_.sendError(streamId_, std::move(rex));
      },
      [this](EncodedFirstResponseError& err) {
        DCHECK(err.encoded.payload);
        sendPayload(err.encoded, /* next */ true, /* complete */ true);
      });

  DCHECK(state_.isTerminal());
  std::ignore = freeStreamAndReturn(state_.isAlive());
}

bool RocketBiDiClientCallback::onSinkCancel() {
  if (!state_.isSinkOpen()) {
    return state_.isAlive();
  }
  state_.onSinkCancel();
  connection_.sendCancel(streamId_);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onSinkRequestN(uint64_t n) {
  connection_.sendRequestN(streamId_, n);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamNext(StreamPayload&& payload) {
  applyCompressionConfigIfNeeded(payload);
  sendPayload(std::move(payload), /* next */ true);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamError(folly::exception_wrapper ew) {
  state_.onStreamError();
  ew.handle(
      [this](RocketException& rex) {
        sendError(ErrorCode::APPLICATION_ERROR, rex.moveErrorData());
      },
      [this](EncodedError& err) {
        sendError(ErrorCode::APPLICATION_ERROR, std::move(err.encoded));
      },
      [this](EncodedStreamError& err) {
        applyCompressionConfigIfNeeded(err.encoded);
        sendPayload(
            std::move(err.encoded), /* next */ true, /* complete */ true);
      },
      [this, &ew](...) { sendError(ErrorCode::APPLICATION_ERROR, ew.what()); });

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

bool RocketBiDiClientCallback::onStreamComplete() {
  state_.onStreamComplete();
  sendEmptyPayload(/* complete */ true);

  return state_.isAlive() ? true : freeStreamAndReturn(false);
}

//
// end of BiDiClientCallback methods
//

//
// Incoming methods: server receives client calls to forward them to server
// callback
//

bool RocketBiDiClientCallback::onStreamRequestN(int32_t n) {
  DCHECK(state_.isStreamOpen());
  return !serverCallback_->onStreamRequestN(n);
}

bool RocketBiDiClientCallback::onStreamCancel() {
  DCHECK(state_.isStreamOpen());

  state_.onStreamCancel();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onStreamCancel()) {
    if (wasTerminal) {
      return freeStreamAndReturn(false);
    }
    return false;
  }
  DCHECK(state_.isAlive());
  return true;
}

bool RocketBiDiClientCallback::onSinkNext(StreamPayload&& payload) {
  DCHECK(state_.isSinkOpen());

  return serverCallback_->onSinkNext(std::move(payload));
}

bool RocketBiDiClientCallback::onSinkError(folly::exception_wrapper ew) {
  DCHECK(state_.isSinkOpen());

  state_.onSinkError();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onSinkError(std::move(ew))) {
    if (wasTerminal) {
      return freeStreamAndReturn(false);
    }
    return false;
  }
  DCHECK(state_.isAlive());
  return true;
}

bool RocketBiDiClientCallback::onSinkComplete() {
  DCHECK(state_.isSinkOpen());

  state_.onSinkComplete();
  bool wasTerminal = state_.isTerminal();
  if (!serverCallback_->onSinkComplete()) {
    if (wasTerminal) {
      return freeStreamAndReturn(false);
    }
    return false;
  }
  DCHECK(state_.isAlive());
  return true;
}

//
// end of Incoming methods
//

void RocketBiDiClientCallback::sendEmptyPayload(bool complete) {
  connection_.sendPayload(
      streamId_,
      Payload::makeFromData(std::unique_ptr<folly::IOBuf>{}),
      Flags().complete(complete));
}

void RocketBiDiClientCallback::applyCompressionConfigIfNeeded(
    StreamPayload& payload) {
  if (compressionConfig_) {
    CompressionManager().setCompressionCodec(
        *compressionConfig_,
        payload.metadata,
        payload.payload ? payload.payload->computeChainDataLength() : 0);
  }
}

} // namespace apache::thrift::rocket
