/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/FlowControlFilter.h>

namespace proxygen {

namespace {
HTTPException getException(const std::string& msg) {
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS, msg);
  ex.setCodecStatusCode(ErrorCode::FLOW_CONTROL_ERROR);
  return ex;
}

} // namespace

FlowControlFilter::FlowControlFilter(Callback& callback,
                                     folly::IOBufQueue& writeBuf,
                                     HTTPCodec* codec,
                                     uint32_t recvCapacity)
    : notify_(callback),
      recvWindow_(codec->getDefaultWindowSize()),
      sendWindow_(codec->getDefaultWindowSize()),
      error_(false),
      sendsBlocked_(false) {
  if (recvCapacity > 0) {
    if (recvCapacity < codec->getDefaultWindowSize()) {
      VLOG(4) << "Ignoring low conn-level recv window size of " << recvCapacity;
    } else if (recvCapacity > codec->getDefaultWindowSize()) {
      auto delta = recvCapacity - codec->getDefaultWindowSize();
      VLOG(4) << "Incrementing default conn-level recv window by " << delta;
      CHECK(recvWindow_.setCapacity(recvCapacity));
      codec->generateWindowUpdate(writeBuf, 0, delta);
    }
  }
}

void FlowControlFilter::setReceiveWindowSize(folly::IOBufQueue& writeBuf,
                                             uint32_t capacity) {
  if (capacity < recvWindow_.getCapacity()) {
    VLOG(4) << "Ignoring low conn-level recv window size of " << capacity;
    return;
  }
  int32_t delta = capacity - recvWindow_.getCapacity();
  if (delta < 0) {
    // For now, we're disallowing shrinking the window, since it can lead
    // to FLOW_CONTROL_ERRORs if there is data in flight.
    VLOG(4) << "Refusing to shrink the recv window";
    return;
  }
  VLOG(4) << "Incrementing default conn-level recv window by " << delta;
  if (!recvWindow_.setCapacity(capacity)) {
    VLOG(2) << "Failed setting conn-level recv window capacity to " << capacity;
    return;
  }
  toAck_ += delta;
  if (toAck_ > 0) {
    call_->generateWindowUpdate(writeBuf, 0, delta);
    toAck_ = 0;
  }
}

bool FlowControlFilter::ingressBytesProcessed(folly::IOBufQueue& writeBuf,
                                              uint32_t delta) {
  toAck_ += delta;
  bool willAck =
      (toAck_ > 0 && uint32_t(toAck_) > recvWindow_.getCapacity() / 2);
  VLOG(4) << "processed " << delta << " toAck_=" << toAck_
          << " bytes, will ack=" << willAck;
  if (willAck) {
    CHECK(recvWindow_.free(toAck_));
    call_->generateWindowUpdate(writeBuf, 0, toAck_);
    toAck_ = 0;
    return true;
  }
  return false;
}

uint32_t FlowControlFilter::getAvailableSend() const {
  return sendWindow_.getNonNegativeSize();
}

bool FlowControlFilter::isReusable() const {
  if (error_) {
    return false;
  }
  return call_->isReusable();
}

void FlowControlFilter::onBody(StreamID stream,
                               std::unique_ptr<folly::IOBuf> chain,
                               uint16_t padding) {
  uint64_t amount = chain->computeChainDataLength();
  if (!recvWindow_.reserve(amount + padding)) {
    error_ = true;
    HTTPException ex = getException(
        folly::to<std::string>("Failed to reserve receive window, window size=",
                               recvWindow_.getSize(),
                               ", amount=",
                               amount));
    callback_->onError(0, ex, false);
  } else {
    if (VLOG_IS_ON(4) && recvWindow_.getSize() == 0) {
      VLOG(4) << "recvWindow full";
    }
    toAck_ += padding;
    CHECK(recvWindow_.free(padding));
    callback_->onBody(stream, std::move(chain), padding);
  }
}

void FlowControlFilter::onWindowUpdate(StreamID stream, uint32_t amount) {
  if (!stream) {
    bool success = sendWindow_.free(amount);
    VLOG(4) << "Remote side ack'd " << amount
            << " bytes, sendWindow=" << sendWindow_.getSize();
    if (!success) {
      LOG(WARNING) << "Remote side sent connection-level WINDOW_UPDATE "
                   << "that could not be applied. Aborting session.";
      // If something went wrong applying the flow control change, abort
      // the entire session.
      error_ = true;
      HTTPException ex = getException(
          folly::to<std::string>("Failed to update send window, outstanding=",
                                 sendWindow_.getOutstanding(),
                                 ", amount=",
                                 amount));
      callback_->onError(stream, ex, false);
    }
    if (sendsBlocked_ && sendWindow_.getNonNegativeSize()) {
      VLOG(4) << "Send window opened";
      sendsBlocked_ = false;
      notify_.onConnectionSendWindowOpen();
    }
    // Don't forward.
  } else {
    callback_->onWindowUpdate(stream, amount);
  }
}

size_t FlowControlFilter::generateBody(folly::IOBufQueue& writeBuf,
                                       StreamID stream,
                                       std::unique_ptr<folly::IOBuf> chain,
                                       folly::Optional<uint8_t> padding,
                                       bool eom) {
  uint8_t padLen = padding ? *padding : 0;
  bool success = sendWindow_.reserve(chain->computeChainDataLength() + padLen);
  VLOG(5) << "Sending " << chain->computeChainDataLength()
          << " bytes, sendWindow=" << sendWindow_.getSize();

  // In the future, maybe make this DCHECK
  CHECK(success) << "Session-level send window underflowed! "
                 << "Too much data sent without WINDOW_UPDATES!";

  if (sendWindow_.getNonNegativeSize() == 0) {
    // Need to inform when the send window is no longer full
    VLOG(4) << "Send window closed";
    sendsBlocked_ = true;
    notify_.onConnectionSendWindowClosed();
  }

  return call_->generateBody(writeBuf, stream, std::move(chain), padding, eom);
}

size_t FlowControlFilter::generateWindowUpdate(folly::IOBufQueue& writeBuf,
                                               StreamID stream,
                                               uint32_t delta) {
  CHECK(stream) << " someone tried to manually manipulate a conn-level window";
  return call_->generateWindowUpdate(writeBuf, stream, delta);
}

} // namespace proxygen
