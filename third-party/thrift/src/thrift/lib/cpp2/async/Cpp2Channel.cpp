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

#include <thrift/lib/cpp2/async/Cpp2Channel.h>

#include <glog/logging.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

using namespace apache::thrift::transport;
using folly::EventBase;

namespace apache::thrift {

Cpp2Channel::Cpp2Channel(
    const std::shared_ptr<folly::AsyncTransport>& transport,
    std::unique_ptr<FramingHandler> framingHandler)
    : transport_(transport),
      evb_(transport->getEventBase()),
      recvCallback_(nullptr),
      eofInvoked_(false),
      outputBufferingHandler_(
          std::make_shared<wangle::OutputBufferingHandler>()),
      framingHandler_(std::move(framingHandler)) {
  pipeline_ = Pipeline::create(
      TAsyncTransportHandler(transport),
      outputBufferingHandler_,
      framingHandler_,
      this);
  // Let the pipeline know that this handler owns the pipeline itself.
  // The pipeline will then avoid destruction order issues.
  // CHECK that this operation is successful.
  CHECK(pipeline_->setOwner(this));
  pipeline_->transportActive();
  // TODO getHandler() with no index should return first valid handler?
  transportHandler_ = pipeline_->getHandler<TAsyncTransportHandler>(0);
}

folly::Future<folly::Unit> Cpp2Channel::close(Context* ctx) {
  DestructorGuard dg(this);
  if (transport_) {
    // If transport is taken over, no need to call processReadEOF. This can
    // happen when processing HTTP GET request, where the ownership of the
    // socket is transferred to GetHandler.
    processReadEOF();
  }
  return CHECK_NOTNULL(ctx)->fireClose();
}

void Cpp2Channel::closeNow() {
  // closeNow can invoke callbacks
  DestructorGuard dg(this);

  if (pipeline_) {
    pipeline_->close();
  }

  failPendingWrites(TTransportException("Channel is closed"));

  // Note that close() above might kill the pipeline_, so let's check again.
  if (pipeline_) {
    pipeline_.reset();
  }
}

void Cpp2Channel::destroy() {
  closeNow();
  MessageChannel::destroy();
}

void Cpp2Channel::attachEventBase(EventBase* eventBase) {
  evb_ = eventBase;
  CHECK_NOTNULL(transportHandler_)->attachEventBase(eventBase);
}

void Cpp2Channel::detachEventBase() {
  CHECK_NOTNULL(getEventBase())->dcheckIsInEventBaseThread();
  evb_ = nullptr;
  outputBufferingHandler_->cleanUp();
  CHECK_NOTNULL(transportHandler_)->detachEventBase();
}

EventBase* Cpp2Channel::getEventBase() {
  return evb_;
}

void Cpp2Channel::read(
    Context*,
    std::pair<std::unique_ptr<folly::IOBuf>, std::unique_ptr<THeader>>
        bufAndHeader) {
  DestructorGuard dg(this);

  if (!recvCallback_) {
    VLOG(5) << "Received a message, but no recvCallback_ installed!";
    return;
  }

  recvCallback_->messageReceived(
      std::move(bufAndHeader.first), std::move(bufAndHeader.second));
}

void Cpp2Channel::readEOF(Context*) {
  processReadEOF();
}

void Cpp2Channel::readException(Context*, folly::exception_wrapper e) {
  DestructorGuard dg(this);
  VLOG(5) << "Got a read error: " << folly::exceptionStr(e);
  if (recvCallback_) {
    recvCallback_->messageReceiveErrorWrapped(std::move(e));
  }
  processReadEOF();
}

void Cpp2Channel::writeSuccess() noexcept {
  DestructorGuard dg(this);
  if (sendCallbacks_.empty()) {
    return;
  }

  auto* cb = sendCallbacks_.front();
  sendCallbacks_.pop_front();
  if (cb) {
    cb->messageSent();
  }
}

void Cpp2Channel::writeError(
    size_t /* bytesWritten */, const TTransportException& ex) noexcept {
  // Pop last write request, call error callback

  DestructorGuard dg(this);
  if (sendCallbacks_.empty()) {
    return;
  }

  VLOG(5) << "Got a write error: " << folly::exceptionStr(ex);
  auto* cb = sendCallbacks_.front();
  sendCallbacks_.pop_front();
  if (cb) {
    cb->messageSendError(
        folly::make_exception_wrapper<TTransportException>(ex));
  }
}

void Cpp2Channel::failPendingWrites(const TTransportException& ex) noexcept {
  while (!sendCallbacks_.empty()) {
    auto* cb = sendCallbacks_.front();
    sendCallbacks_.pop_front();
    if (cb) {
      cb->messageSendError(
          folly::make_exception_wrapper<TTransportException>(ex));
    }
  }
}

void Cpp2Channel::processReadEOF() noexcept {
  transport_->setReadCB(nullptr);

  VLOG(5) << "Got an EOF on channel";
  if (recvCallback_ && !eofInvoked_) {
    eofInvoked_ = true;
    recvCallback_->messageChannelEOF();
  }
}

// Low level interface
void Cpp2Channel::sendMessage(
    SendCallback* callback,
    std::unique_ptr<folly::IOBuf>&& buf,
    apache::thrift::transport::THeader* header) {
  // Callback may be null.
  assert(buf);

  DestructorGuard dg(this);

  if (!transport_->good()) {
    VLOG(5) << "Channel is !good() in sendMessage";
    // Callback must be last thing in sendMessage, or use guard
    if (callback) {
      callback->messageSendError(
          folly::make_exception_wrapper<TTransportException>(
              "Channel is !good()"));
    }
    return;
  }

  if (callback) {
    callback->sendQueued();
  }

  if (getDestroyPending() || !pipeline_) {
    if (callback) {
      callback->messageSendError(
          folly::make_exception_wrapper<TTransportException>(
              "Channel is closed"));
    }
    return;
  }

  sendCallbacks_.push_back(callback);

  auto future = pipeline_->write(std::make_pair(std::move(buf), header));
  std::move(future).thenTry([this, dg](folly::Try<folly::Unit>&& t) {
    if (t.withException<TTransportException>(
            [&](const TTransportException& ex) { writeError(0, ex); }) ||
        t.withException<std::exception>([&](const std::exception& ex) {
          writeError(0, TTransportException(ex.what()));
        })) {
      return;
    } else {
      writeSuccess();
    }
  });
}

void Cpp2Channel::setReceiveCallback(RecvCallback* callback) {
  if (recvCallback_ == callback) {
    return;
  }

  // Still want to set recvCallback_ for outstanding EOFs
  recvCallback_ = callback;

  if (!transport_->good()) {
    transport_->setReadCB(nullptr);
    return;
  }

  auto& transportHandler = *CHECK_NOTNULL(transportHandler_);
  if (callback) {
    transportHandler.attachReadCallback();
  } else {
    transportHandler.detachReadCallback();
  }

  // Transport might have gotten into a bad state (e.g., closed) while attaching
  // the read callback, which itself may have tried immediately reading from the
  // transport.
  // Note also that the call to attachReadCallback() may have indirectly stolen
  // transport_, which we must guard against.
  if (transport_ && !transport_->good()) {
    throw TTransportException("Channel is !good()");
  }
}

} // namespace apache::thrift
