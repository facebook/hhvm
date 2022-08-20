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

#ifndef THRIFT_ASYNC_TSTREAMASYNCCHANNEL_TCC_
#define THRIFT_ASYNC_TSTREAMASYNCCHANNEL_TCC_ 1

#include <folly/SocketAddress.h>
#include <thrift/lib/cpp/async/TStreamAsyncChannel.h>

namespace apache {
namespace thrift {
namespace async {

template <typename WriteRequest_, typename ReadState_>
TStreamAsyncChannel<WriteRequest_, ReadState_>::TStreamAsyncChannel(
    const std::shared_ptr<folly::AsyncTransport>& transport)
    : folly::AsyncTimeout(transport->getEventBase()),
      transport_(transport),
      writeReqHead_(nullptr),
      writeReqTail_(nullptr),
      readState_(),
      readCallback_(),
      readErrorCallback_(),
      recvTimeout_(0),
      timedOut_(false),
      errorType_(
          transport::TTransportException::TTransportExceptionType::UNKNOWN) {}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::destroy() {
  // When destroy is called, close the channel immediately
  closeNow();

  // Then call DelayedDestruction::destroy() to take care of
  // whether or not we need immediate or delayed destruction
  folly::DelayedDestruction::destroy();
}

template <typename WriteRequest_, typename ReadState_>
bool TStreamAsyncChannel<WriteRequest_, ReadState_>::readable() const {
  return transport_->readable();
}

template <typename WriteRequest_, typename ReadState_>
bool TStreamAsyncChannel<WriteRequest_, ReadState_>::good() const {
  return transport_->good();
}

template <typename WriteRequest_, typename ReadState_>
bool TStreamAsyncChannel<WriteRequest_, ReadState_>::error() const {
  return (timedOut_ || transport_->error());
}

template <typename WriteRequest_, typename ReadState_>
bool TStreamAsyncChannel<WriteRequest_, ReadState_>::timedOut() const {
  return timedOut_;
}

template <typename WriteRequest_, typename ReadState_>
transport::TTransportException::TTransportExceptionType
TStreamAsyncChannel<WriteRequest_, ReadState_>::errorType() const {
  return errorType_;
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::sendMessage(
    const VoidCallback& cob,
    const VoidCallback& errorCob,
    transport::TMemoryBuffer* message) {
  assert(message);
  DestructorGuard dg(this);

  if (!good()) {
    T_DEBUG_T("sendMessage: transport went bad, bailing out.");
    return errorCob();
  }

  if (message->available_read() == 0) {
    T_ERROR("sendMessage: buffer is empty");
    return errorCob();
  }

  WriteRequest_* writeReq;
  try {
    writeReq = new WriteRequest_(cob, errorCob, message, this);
  } catch (const std::exception&) {
    T_ERROR("sendMessage: failed to allocate new write request object");
    errorCob();
    return;
  }

  pushWriteRequest(writeReq);
  writeReq->write(transport_.get(), this);
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::sendOnewayMessage(
    const VoidCallback& cob,
    const VoidCallback& errorCob,
    transport::TMemoryBuffer* message) {
  sendMessage(cob, errorCob, message);
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::recvMessage(
    const VoidCallback& cob,
    const VoidCallback& errorCob,
    transport::TMemoryBuffer* message) {
  assert(message);
  DestructorGuard dg(this);

  if (!good()) {
    T_DEBUG_T("recvMessage: transport went bad, bailing out.");
    return errorCob();
  }

  if (message->available_read() != 0) {
    T_ERROR("recvMessage: buffer is not empty.");
    return errorCob();
  }

  if (readCallbackQ_.empty() && readCallback_ == nullptr) {
    readState_.setCallbackBuffer(message);
    readCallback_ = cob;
    readErrorCallback_ = errorCob;
  } else {
    readCallbackQ_.push_back(ReadQueueEntry(cob, errorCob, message));
    return;
  }

  // Some ReadState implementations perform read-ahead,
  // and they may already have data waiting to be processed.
  // If so, we need to invoke readDataAvailable() immediately, rather than
  // waiting for new data from the transport.
  if (readState_.hasReadAheadData()) {
    if (invokeReadDataAvailable(0)) {
      // We already invoked the callback
      return;
    }
  }

  // start the read timeout
  if (recvTimeout_ > 0) {
    scheduleTimeout(recvTimeout_);
  }

  // start reading from the transport
  // Note that setReadCallback() may invoke our read callback methods
  // immediately, so the read may complete before setReadCallback() returns.
  transport_->setReadCB(this);
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::sendAndRecvMessage(
    const VoidCallback& cob,
    const VoidCallback& errorCob,
    transport::TMemoryBuffer* sendBuf,
    transport::TMemoryBuffer* recvBuf) {
  // TODO: it would be better to perform this bind once, rather than
  // each time sendAndRecvMessage() is called.
  const VoidCallback& send_done = std::bind(
      &TStreamAsyncChannel::recvMessage, this, cob, errorCob, recvBuf);

  return sendMessage(send_done, errorCob, sendBuf);
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::close() {
  DestructorGuard dg(this); // transport::close can invoke callbacks

  transport_->setReadCB(nullptr);
  transport_->close();

  if (readCallback_) {
    processReadEOF();
  }

  // no need to free the write-queue here.  The underlying transport will
  // drain the writes first
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::closeNow() {
  DestructorGuard dg(this); // transport::closeNow can invoke callbacks

  transport_->setReadCB(nullptr);
  transport_->closeNow();

  if (readCallback_) {
    processReadEOF();
  }

  // no need to free the write-queue here.  The underlying transport will
  // fail pending writes first
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::attachEventBase(
    folly::EventBase* eventBase) {
  folly::AsyncTimeout::attachEventBase(eventBase);
  transport_->attachEventBase(eventBase);
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::detachEventBase() {
  // detachEventBase() may not be called while in the middle of reading or
  // writing a message.  Make sure there are no read callbacks
  assert(!readCallback_ && readCallbackQ_.empty());
  // Even though readCallback_ is unset, the read timeout might still be
  // installed.  This happens when detachEventBase() is invoked by the
  // recvMessage() callback, because invokeReadDataAvailable() optimizes and
  // leaves the timeout and transport read callback installed while invoking
  // the recvMessage() callback.  Make sure we cancel the read timeout before
  // detaching from the event base.
  if (transport_->getReadCallback() == this) {
    cancelTimeout();
    transport_->setReadCB(nullptr);
  }

  folly::AsyncTimeout::detachEventBase();
  transport_->detachEventBase();
}

template <typename WriteRequest_, typename ReadState_>
folly::EventBase* TStreamAsyncChannel<WriteRequest_, ReadState_>::getEventBase()
    const {
  return transport_->getEventBase();
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::setRecvTimeout(
    uint32_t milliseconds) {
  recvTimeout_ = milliseconds;
  // If we are currently reading, update the timeout
  if (transport_->getReadCallback() == this) {
    if (milliseconds > 0) {
      scheduleTimeout(milliseconds);
    } else {
      cancelTimeout();
    }
  }
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::getReadBuffer(
    void** bufReturn, size_t* lenReturn) {
  readState_.getReadBuffer(bufReturn, lenReturn);
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::readDataAvailable(
    size_t len) noexcept {
  invokeReadDataAvailable(len);
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::readEOF() noexcept {
  // readCallback_ may be nullptr if readEOF() is invoked while the read
  // callback is already running inside invokeReadDataAvailable(), since
  // invokeReadDataAvailable() leaves the transport read callback installed
  // while calling the channel read callback.
  if (readCallback_) {
    processReadEOF();
  }
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::readErr(
    const folly::AsyncSocketException&) noexcept {
  // readCallback_ may be nullptr if readEOF() is invoked while the read
  // callback is already running inside invokeReadDataAvailable(), since
  // invokeReadDataAvailable() leaves the transport read callback installed
  // while calling the channel read callback.
  if (!readCallback_) {
    return;
  }

  DestructorGuard dg(this);

  cancelTimeout();
  failAllReads();
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::writeSuccess() noexcept {
  DestructorGuard dg(this);

  WriteRequest_* req = popWriteRequest();
  req->writeSuccess();
  delete req;
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::writeErr(
    size_t bytesWritten, const folly::AsyncSocketException& ex) noexcept {
  DestructorGuard dg(this);

  transport::TTransportException tex(ex);

  // Set exception type
  errorType_ = tex.getType();

  if (errorType_ == transport::TTransportException::TIMED_OUT) {
    timedOut_ = true;
  }

  WriteRequest_* req = popWriteRequest();
  req->writeError(bytesWritten, tex);
  delete req;
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::timeoutExpired() noexcept {
  DestructorGuard dg(this);

  errorType_ = transport::TTransportException::TIMED_OUT;
  timedOut_ = true;

  std::string addressStr;
  try {
    folly::SocketAddress addr;
    transport_->getPeerAddress(&addr);
    addressStr = addr.describe();
  } catch (const std::exception&) {
    addressStr = "unknown";
  }
  // Close the transport.  It isn't usable anymore, since we are leaving
  // it in a state with a partial message outstanding.
  transport_->setReadCB(nullptr);
  transport_->close();

  // TODO: It would be nice not to have to always log an error message here;
  // ideally the callback should decide if this is worth logging or not.
  // Unfortunately the TAsyncChannel API doesn't allow us to pass any error
  // info back to the callback.
  T_ERROR("TStreamAsyncChannel: read timeout for %s", addressStr.c_str());

  failAllReads();
}

template <typename WriteRequest_, typename ReadState_>
bool TStreamAsyncChannel<WriteRequest_, ReadState_>::invokeReadDataAvailable(
    size_t len) noexcept {
  DestructorGuard dg(this);
  assert(readCallback_);

  bool readDone;
  try {
    readDone = readState_.readDataAvailable(len);
  } catch (const std::exception& ex) {
    // The channel is in an unknown state after an error processing read data.
    // Close the channel to ensure that callers cannot try to read from this
    // channel again.
    //
    // Make sure we do this after clearing our callbacks, so that the
    // channel won't call our readEOF() method.
    cancelTimeout();
    transport_->setReadCB(nullptr);

    std::string addressStr;
    try {
      folly::SocketAddress addr;
      transport_->getPeerAddress(&addr);
      addressStr = addr.describe();
    } catch (const std::exception&) {
      addressStr = "unknown";
    }

    T_ERROR("error reading message from %s: %s", addressStr.c_str(), ex.what());
    failAllReads();
    return true;
  }

  if (!readDone) {
    // We read some data, but didn't finish reading a full message.
    if (recvTimeout_ > 0) {
      // Reset the timeout whenever we receive any data.
      // TODO: This matches the old TAsyncChannel behavior, but it seems like
      // it would make more sense to have the timeout apply to the entire
      // message as a whole.  Eventually we should remove this code that resets
      // the timeout.
      scheduleTimeout(recvTimeout_);
    }
    return false;
  }

  folly::EventBase* ourEventBase = transport_->getEventBase();

  // We read a full message.  Invoke the read callback.
  invokeReadCallback(readCallback_, "read callback");

  // Note that we cleared readCallback_ and readErrorCallback_ before invoking
  // the callback, but left ourself installed as the AsyncTransport read
  // callback.
  //
  // This allows us to avoid changing the AsyncTransport read callback if
  // the channel read callback immediately called recvMessage() again.  This is
  // fairly common, and we avoid 2 unnecessary epoll_ctl() calls by not
  // changing the transport read callback.  This results in a noticeable
  // performance improvement.
  //
  // If readCallback_ is set again after the callback returns, we're still
  // reading.  recvMessage() will have taken care of reseting the receive
  // timeout, so we have nothing else to do.
  //
  // If readCallback_ is unset, recvMessage() wasn't called again and we need
  // to stop reading.  If our EventBase has changed, detachEventBase() will
  // have already stopped reading.  (Note that if the EventBase has changed,
  // it's possible that readCallback_ has already been set again to start
  // reading in the other thread.)
  if (transport_->getEventBase() == ourEventBase && !readCallback_) {
    if (readCallbackQ_.empty()) {
      cancelTimeout();
      transport_->setReadCB(nullptr);
    } else {
      // There are queued readers, pop one.  This block should have the same
      // effect as if recvMessage were called
      const ReadQueueEntry& qentry = readCallbackQ_.front();
      readCallback_ = qentry.readCallback;
      readErrorCallback_ = qentry.readErrorCallback;
      readState_.setCallbackBuffer(qentry.readBuffer);
      readCallbackQ_.pop_front();

      if (readState_.hasReadAheadData()) {
        return invokeReadDataAvailable(0);
      } else if (recvTimeout_ > 0) {
        scheduleTimeout(recvTimeout_);
      }
    }
  }
  return true;
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::failAllReads() {
  invokeReadCallback(readErrorCallback_, "read error callback");

  while (!readCallbackQ_.empty()) {
    const ReadQueueEntry& qentry = readCallbackQ_.front();
    invokeReadCallback(qentry.readErrorCallback, "read error callback");
    readCallbackQ_.pop_front();
  }
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::processReadEOF() noexcept {
  DestructorGuard dg(this);
  assert(readCallback_);

  VoidCallback cb;
  const char* cbName;
  if (readState_.hasPartialMessage()) {
    cb = readErrorCallback_;
    cbName = "read error callback";
  } else {
    // We call the normal (non-error) callback if no data has been received yet
    // when EOF occurs.
    //
    // TODO: It would be nicer to have a mechanism to indicate to the caller
    // that EOF was received, instead of treating this just like 0-sized
    // message.
    cb = readCallback_;
    cbName = "read callback";
  }

  cancelTimeout();
  invokeReadCallback(cb, cbName);

  // Any queued reads should be notified like the else case above as only
  // the first reader can have partial data.
  while (!readCallbackQ_.empty()) {
    const ReadQueueEntry& qentry = readCallbackQ_.front();
    invokeReadCallback(qentry.readCallback, cbName);
    readCallbackQ_.pop_front();
  }
}

template <typename WriteRequest_, typename ReadState_>
void TStreamAsyncChannel<WriteRequest_, ReadState_>::invokeReadCallback(
    VoidCallback cb, const char* callbackName) noexcept {
  readState_.unsetCallbackBuffer();
  readCallback_ = VoidCallback();
  readErrorCallback_ = VoidCallback();

  try {
    cb();
  } catch (const std::exception& ex) {
    // If you're hitting this abort, your (error) read callback threw
    // an exception. Perhaps you called a Thrift function
    // (e.g., recv_XXX) which threw this exception and you didn't catch it,
    // or you threw it yourself.
    // To debug this, you can use fbcode/folly/experimental/exception_tracer
    // to trace the exception call stack. For this:
    // - Add "@/folly/experimental/exception_tracer:exception_tracer" as a dep
    //   to your TARGETS file,
    // - #include <folly/experimental/exception_tracer/ExceptionTracer.h>
    //   in this file
    // - Uncomment the following line:
    //     for (const auto& e : getCurrentExceptions()) { LOG(ERROR) << e; }
    // Note that exception_tracer has overhead for each throw statement, thus
    // only use it when debugging.
    T_ERROR(
        "TAsyncChannel: %s threw %s exception: %s",
        callbackName,
        typeid(ex).name(),
        ex.what());
    abort();
  } catch (...) {
    // The big comment above also applies here.
    T_ERROR("TAsyncChannel: %s threw exception", callbackName);
    abort();
  }
}

} // namespace async
} // namespace thrift
} // namespace apache

#endif // THRIFT_ASYNC_TSTREAMASYNCCHANNEL_TCC_
