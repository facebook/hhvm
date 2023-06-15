/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/AsyncFizzBase.h>

#include <folly/Conv.h>
#include <folly/io/Cursor.h>

namespace fizz {

using folly::AsyncSocketException;

/**
 * Buffer size above which we should unset our read callback to apply back
 * pressure on the transport.
 */
static const uint32_t kMaxBufSize = 64 * 1024;

/**
 * Buffer size above which we should break up shared writes, to avoid storing
 * entire unencrypted and encrypted buffer simultaneously.
 */
static const uint32_t kPartialWriteThreshold = 128 * 1024;

AsyncFizzBase::AsyncFizzBase(
    folly::AsyncTransportWrapper::UniquePtr transport,
    TransportOptions options)
    : folly::WriteChainAsyncTransportWrapper<folly::AsyncTransportWrapper>(
          std::move(transport)),
      handshakeTimeout_(*this, transport_->getEventBase()),
      transportOptions_(std::move(options)) {
  setReadMode(transportOptions_.readMode);
}

AsyncFizzBase::~AsyncFizzBase() {
  transport_->setEventCallback(nullptr);
  transport_->setReadCB(nullptr);
  if (tailWriteRequest_) {
    tailWriteRequest_->unlinkFromBase();
  }
}

void AsyncFizzBase::destroy() {
  transport_->closeNow();
  transport_->setEventCallback(nullptr);
  transport_->setReadCB(nullptr);
  DelayedDestruction::destroy();
}

AsyncFizzBase::ReadCallback* AsyncFizzBase::getReadCallback() const {
  return readCallback_;
}

void AsyncFizzBase::setReadCB(AsyncFizzBase::ReadCallback* callback) {
  readCallback_ = callback;

  if (readCallback_) {
    if (appDataBuf_) {
      deliverAppData(nullptr);
    }

    if (!good()) {
      AsyncSocketException ex(
          AsyncSocketException::NOT_OPEN,
          "setReadCB() called with transport in bad state");
      deliverError(ex);
    } else {
      // The read callback may have been unset earlier if our buffer was full.
      startTransportReads();
    }
  }
}

AsyncFizzBase::QueuedWriteRequest::QueuedWriteRequest(
    AsyncFizzBase* base,
    folly::AsyncTransportWrapper::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf> data,
    folly::WriteFlags flags)
    : asyncFizzBase_(base), callback_(callback), flags_(flags) {
  data_.append(std::move(data));
  entireChainBytesBuffered = data_.chainLength();
}

void AsyncFizzBase::QueuedWriteRequest::startWriting() {
  auto buf = data_.splitAtMost(kPartialWriteThreshold);

  auto flags = flags_;
  if (!data_.empty()) {
    flags |= folly::WriteFlags::CORK;
  }
  size_t len = buf->computeChainDataLength();
  dataWritten_ += len;

  CHECK(asyncFizzBase_);
  CHECK(asyncFizzBase_->tailWriteRequest_);
  asyncFizzBase_->tailWriteRequest_->entireChainBytesBuffered -= len;
  asyncFizzBase_->writeAppData(this, std::move(buf), flags);
}

void AsyncFizzBase::QueuedWriteRequest::append(QueuedWriteRequest* request) {
  DCHECK(!next_);
  next_ = request;
  next_->entireChainBytesBuffered += entireChainBytesBuffered;
  entireChainBytesBuffered = 0;
}

void AsyncFizzBase::QueuedWriteRequest::unlinkFromBase() {
  asyncFizzBase_ = nullptr;
}

void AsyncFizzBase::QueuedWriteRequest::fail(
    const folly::AsyncSocketException& ex) {
  writeErr(0, ex);
}

void AsyncFizzBase::QueuedWriteRequest::writeSuccess() noexcept {
  if (!data_.empty()) {
    startWriting();
  } else {
    advanceOnBase();
    auto callback = callback_;
    auto next = next_;
    auto base = asyncFizzBase_;
    delete this;

    DelayedDestruction::DestructorGuard dg(base);

    DCHECK(!base->immediatelyPendingWriteRequest_);
    base->immediatelyPendingWriteRequest_ = next;
    if (callback) {
      callback->writeSuccess();
    }
    if (next && base->immediatelyPendingWriteRequest_ == next) {
      base->immediatelyPendingWriteRequest_ = nullptr;
      next->startWriting();
    }
  }
}

void AsyncFizzBase::QueuedWriteRequest::writeErr(
    size_t /* written */,
    const folly::AsyncSocketException& ex) noexcept {
  // Deliver the error to all queued writes, starting with this one. We avoid
  // recursively calling writeErr as that can cause excesssive stack usage if
  // there are a large number of queued writes.
  QueuedWriteRequest* errorToDeliver = this;
  while (errorToDeliver) {
    errorToDeliver = errorToDeliver->deliverSingleWriteErr(ex);
  }
}

AsyncFizzBase::QueuedWriteRequest*
AsyncFizzBase::QueuedWriteRequest::deliverSingleWriteErr(
    const folly::AsyncSocketException& ex) {
  advanceOnBase();
  auto callback = callback_;
  auto next = next_;
  auto dataWritten = dataWritten_;
  delete this;

  if (callback) {
    callback->writeErr(dataWritten, ex);
  }

  return next;
}

void AsyncFizzBase::QueuedWriteRequest::advanceOnBase() {
  if (!next_ && asyncFizzBase_) {
    CHECK_EQ(asyncFizzBase_->tailWriteRequest_, this);
    asyncFizzBase_->tailWriteRequest_ = nullptr;
  }
}

void AsyncFizzBase::writeChain(
    folly::AsyncTransportWrapper::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf>&& buf,
    folly::WriteFlags flags) {
  auto writeSize = buf->computeChainDataLength();
  appBytesWritten_ += writeSize;

  // We want to split up and queue large writes to avoid simultaneously storing
  // unencrypted and encrypted large buffer in memory. We can skip this if the
  // buffer is unshared (because we can encrypt in-place). We also skip this
  // when sending early data to avoid the possibility of splitting writes
  // between early data and normal data.
  bool largeWrite = writeSize > kPartialWriteThreshold;
  bool transportBuffering = transport_->getRawBytesBuffered() > 0;
  bool needsToQueue = (largeWrite || transportBuffering) && buf->isShared() &&
      !connecting() && isReplaySafe();
  if (tailWriteRequest_ || needsToQueue) {
    auto newWriteRequest =
        new QueuedWriteRequest(this, callback, std::move(buf), flags);

    if (tailWriteRequest_) {
      tailWriteRequest_->append(newWriteRequest);
      tailWriteRequest_ = newWriteRequest;
    } else {
      tailWriteRequest_ = newWriteRequest;
      newWriteRequest->startWriting();
    }
  } else {
    writeAppData(callback, std::move(buf), flags);
  }
}

size_t AsyncFizzBase::getAppBytesWritten() const {
  return appBytesWritten_;
}

size_t AsyncFizzBase::getAppBytesReceived() const {
  return appBytesReceived_;
}

size_t AsyncFizzBase::getAppBytesBuffered() const {
  return transport_->getAppBytesBuffered() +
      (tailWriteRequest_ ? tailWriteRequest_->getEntireChainBytesBuffered()
                         : 0);
}

void AsyncFizzBase::startTransportReads() {
  if (transportOptions_.registerEventCallback) {
    transport_->setEventCallback(this);
  }
  transport_->setReadCB(this);
}

void AsyncFizzBase::startHandshakeTimeout(std::chrono::milliseconds timeout) {
  handshakeTimeout_.scheduleTimeout(timeout);
}

void AsyncFizzBase::cancelHandshakeTimeout() {
  handshakeTimeout_.cancelTimeout();
}

void AsyncFizzBase::deliverAppData(std::unique_ptr<folly::IOBuf> data) {
  if (data) {
    appBytesReceived_ += data->computeChainDataLength();
  }

  if (appDataBuf_) {
    if (data) {
      appDataBuf_->prependChain(std::move(data));
    }
    data = std::move(appDataBuf_);
  }

  while (readCallback_ && data) {
    if (readCallback_->isBufferMovable()) {
      return readCallback_->readBufferAvailable(std::move(data));
    } else {
      folly::io::Cursor cursor(data.get());
      size_t available = 0;
      while ((available = cursor.totalLength()) != 0 && readCallback_ &&
             !readCallback_->isBufferMovable()) {
        void* buf = nullptr;
        size_t buflen = 0;
        try {
          readCallback_->getReadBuffer(&buf, &buflen);
        } catch (const AsyncSocketException& ase) {
          return deliverError(ase);
        } catch (const std::exception& e) {
          AsyncSocketException ase(
              AsyncSocketException::BAD_ARGS,
              folly::to<std::string>("getReadBuffer() threw ", e.what()));
          return deliverError(ase);
        } catch (...) {
          AsyncSocketException ase(
              AsyncSocketException::BAD_ARGS,
              "getReadBuffer() threw unknown exception");
          return deliverError(ase);
        }
        if (buflen == 0 || buf == nullptr) {
          AsyncSocketException ase(
              AsyncSocketException::BAD_ARGS,
              "getReadBuffer() returned empty buffer");
          return deliverError(ase);
        }

        size_t bytesToRead = std::min(buflen, available);
        cursor.pull(buf, bytesToRead);
        readCallback_->readDataAvailable(bytesToRead);
      }

      // If we have data left, it means the read callback changed and we need
      // to save the remaining data (if any)
      if (available != 0) {
        std::unique_ptr<folly::IOBuf> remainingData;
        cursor.clone(remainingData, available);
        data = std::move(remainingData);
      } else {
        // Out of data. Reset the data pointer to end the loop
        data.reset();
      }
    }
  }

  if (data) {
    appDataBuf_ = std::move(data);
  }

  checkBufLen();
}

void AsyncFizzBase::deliverError(
    const AsyncSocketException& ex,
    bool closeTransport) {
  DelayedDestruction::DestructorGuard dg(this);

  if (readCallback_) {
    auto readCallback = readCallback_;
    readCallback_ = nullptr;
    if (ex.getType() == AsyncSocketException::END_OF_FILE) {
      readCallback->readEOF();
    } else {
      readCallback->readErr(ex);
    }
  }

  if (immediatelyPendingWriteRequest_) {
    // If we were about to start writing a QueuedWriteRequest, error it here.
    // This is done to ensure writeErr is invoked synchronously.
    auto immediatelyPendingWriteRequest = immediatelyPendingWriteRequest_;
    immediatelyPendingWriteRequest_ = nullptr;
    immediatelyPendingWriteRequest->fail(ex);
  }

  // Clear the secret callback too.
  if (secretCallback_) {
    secretCallback_ = nullptr;
  }

  if (closeTransport) {
    transport_->close();
  }
}

class AsyncFizzBase::FizzMsgHdr : public folly::EventRecvmsgCallback::MsgHdr {
  FizzMsgHdr() = delete;

 public:
  ~FizzMsgHdr() override = default;
  explicit FizzMsgHdr(AsyncFizzBase* fizzBase) {
    arg_ = fizzBase;
    freeFunc_ = FizzMsgHdr::free;
    cbFunc_ = FizzMsgHdr::cb;
  }

  void reset() {
    data_ = msghdr{};
    auto base = static_cast<AsyncFizzBase*>(arg_);
    base->getReadBuffer(&iov_.iov_base, &iov_.iov_len);
    data_.msg_iov = &iov_;
    data_.msg_iovlen = 1;
  }

  static void free(folly::EventRecvmsgCallback::MsgHdr* msgHdr) {
    delete msgHdr;
  }

  static void cb(folly::EventRecvmsgCallback::MsgHdr* msgHdr, int res) {
    static_cast<AsyncFizzBase*>(msgHdr->arg_)
        ->eventRecvmsgCallback(static_cast<FizzMsgHdr*>(msgHdr), res);
  }

 private:
  iovec iov_;
};

folly::EventRecvmsgCallback::MsgHdr* AsyncFizzBase::allocateData() noexcept {
  auto* ret = msgHdr_.release();
  if (!ret) {
    ret = new FizzMsgHdr(this);
  }
  ret->reset();
  return ret;
}

void AsyncFizzBase::eventRecvmsgCallback(FizzMsgHdr* msgHdr, int res) {
  DelayedDestruction::DestructorGuard dg(this);
  if (res > 0) {
    transportReadBuf_.postallocate(res);
    transportDataAvailable();
    checkBufLen();
  } else if (res == 0) {
    readEOF();
  } else {
    AsyncSocketException ex(
        AsyncSocketException::INTERNAL_ERROR, "event recv failed", (0 - res));
    deliverError(ex);
  }
  msgHdr_.reset(msgHdr);
}

void AsyncFizzBase::getReadBuffer(
    folly::IOBufQueue& buf,
    void** bufReturn,
    size_t* lenReturn) {
  std::pair<void*, uint32_t> readSpace = buf.preallocate(
      transportOptions_.readBufferMinReadSize,
      transportOptions_.readBufferAllocationSize);
  *bufReturn = readSpace.first;

  // `readSizeHint_`, if zero, indicates that we do not care about how much
  // data we read from the underlying socket.
  //
  // `readSizeHint_`, if nonzero, indicates the maximum amount of data we
  // want to read from the underlying socket. This is necessary for kTLS,
  // where we want to ensure that when ReportHandshakeSuccess is called, we
  // are at a known point in the TCP stream, so we can let the kernel start
  // decrypting records for us.
  //
  // For transport with "record aligned reads", we initially set `readSizeHint_`
  // equal to the size of the TLS record header. Subsequently, the state machine
  // will tell us exactly how much data is required to complete the record
  // in WaitForData actions.
  if (readSizeHint_ > 0) {
    *lenReturn = std::min(
        static_cast<decltype(readSizeHint_)>(
            transportOptions_.readBufferMinReadSize),
        readSizeHint_);
  } else {
    *lenReturn = readSpace.second;
  }
}

void AsyncFizzBase::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  getReadBuffer(transportReadBuf_, bufReturn, lenReturn);
}

void AsyncFizzBase::getReadBuffers(folly::IOBufIovecBuilder::IoVecVec& iovs) {
  DCHECK(!!transportOptions_.ioVecQueue);
  transportOptions_.ioVecQueue->allocateBuffers(iovs);
}

void AsyncFizzBase::readDataAvailable(size_t len) noexcept {
  DelayedDestruction::DestructorGuard dg(this);

  if (getReadMode() == folly::AsyncTransport::ReadCallback::ReadMode::ReadVec) {
    DCHECK(!!transportOptions_.ioVecQueue);
    auto tmp = transportOptions_.ioVecQueue->extractIOBufChain(len);
    transportReadBuf_.append(std::move(tmp));
  } else {
    transportReadBuf_.postallocate(len);
  }
  transportDataAvailable();
  checkBufLen();
}

bool AsyncFizzBase::isBufferMovable() noexcept {
  return true;
}

void AsyncFizzBase::readBufferAvailable(
    std::unique_ptr<folly::IOBuf> data) noexcept {
  DelayedDestruction::DestructorGuard dg(this);

  transportReadBuf_.append(std::move(data));
  transportDataAvailable();
  checkBufLen();
}

void AsyncFizzBase::readEOF() noexcept {
  AsyncSocketException eof(AsyncSocketException::END_OF_FILE, "readEOF()");
  transportError(eof);
}

void AsyncFizzBase::readErr(const folly::AsyncSocketException& ex) noexcept {
  transportError(ex);
}

folly::AsyncReader::ReadCallback::ZeroCopyMemStore*
AsyncFizzBase::readZeroCopyEnabled() noexcept {
  return transportOptions_.zeroCopyMemStore;
}

void AsyncFizzBase::getZeroCopyFallbackBuffer(
    void** bufReturn,
    size_t* lenReturn) noexcept {
  getReadBuffer(zeroCopyFallbackReadBuf_, bufReturn, lenReturn);
}

void AsyncFizzBase::readZeroCopyDataAvailable(
    std::unique_ptr<folly::IOBuf>&& zeroCopyData,
    size_t additionalBytes) noexcept {
  DelayedDestruction::DestructorGuard dg(this);

  if (zeroCopyData) {
    transportReadBuf_.append(std::move(zeroCopyData));
  }

  if (additionalBytes) {
    zeroCopyFallbackReadBuf_.postallocate(additionalBytes);
    transportReadBuf_.append(zeroCopyFallbackReadBuf_.move());
  }

  transportDataAvailable();
}

void AsyncFizzBase::writeSuccess() noexcept {}

void AsyncFizzBase::writeErr(
    size_t /* bytesWritten */,
    const folly::AsyncSocketException& ex) noexcept {
  transportError(ex);
}

void AsyncFizzBase::checkBufLen() {
  if (!readCallback_ &&
      (transportReadBuf_.chainLength() >= kMaxBufSize ||
       (appDataBuf_ && appDataBuf_->computeChainDataLength() >= kMaxBufSize))) {
    transport_->setEventCallback(nullptr);
    transport_->setReadCB(nullptr);
  }
}

void AsyncFizzBase::handshakeTimeoutExpired() noexcept {
  AsyncSocketException eof(
      AsyncSocketException::TIMED_OUT, "handshake timeout expired");
  transportError(eof);
}

void AsyncFizzBase::endOfTLS(std::unique_ptr<folly::IOBuf> endOfData) noexcept {
  DelayedDestruction::DestructorGuard dg(this);

  if (connecting()) {
    AsyncSocketException ex(
        AsyncSocketException::INVALID_STATE,
        "tls connection torn down while connecting");
    transportError(ex);
    return;
  }

  if (endOfTLSCallback_) {
    endOfTLSCallback_->endOfTLS(this, std::move(endOfData));
  } else {
    // The end of TLS callback may not want the socket to be closed but by
    // default read callbacks often close on EOF, as such we defer to the setter
    // of the end of tls callback to apply the appropriate behaviour if it's set
    if (readCallback_) {
      auto readCallback = readCallback_;
      readCallback_ = nullptr;
      readCallback->readEOF();
    }
    transport_->close();
  }
}

// The below maps the secret type to the appropriate secret callback function.
namespace {
class SecretVisitor {
 public:
  explicit SecretVisitor(
      AsyncFizzBase::SecretCallback* cb,
      const std::vector<uint8_t>& secretBuf)
      : callback_(cb), secretBuf_(secretBuf) {}
  void operator()(const SecretType& secretType) {
    switch (secretType.type()) {
      case SecretType::Type::EarlySecrets_E:
        operator()(*secretType.asEarlySecrets());
        break;
      case SecretType::Type::HandshakeSecrets_E:
        operator()(*secretType.asHandshakeSecrets());
        break;
      case SecretType::Type::MasterSecrets_E:
        operator()(*secretType.asMasterSecrets());
        break;
      case SecretType::Type::AppTrafficSecrets_E:
        operator()(*secretType.asAppTrafficSecrets());
        break;
    }
  }

  void operator()(const EarlySecrets& secret) {
    switch (secret) {
      case EarlySecrets::ExternalPskBinder:
        callback_->externalPskBinderAvailable(secretBuf_);
        return;
      case EarlySecrets::ResumptionPskBinder:
        callback_->resumptionPskBinderAvailable(secretBuf_);
        return;
      case EarlySecrets::ClientEarlyTraffic:
        callback_->clientEarlyTrafficSecretAvailable(secretBuf_);
        return;
      case EarlySecrets::EarlyExporter:
        callback_->earlyExporterSecretAvailable(secretBuf_);
        return;
      case EarlySecrets::ECHAcceptConfirmation:
        // Not an actual encryption secret
        return;
      case EarlySecrets::HRRECHAcceptConfirmation:
        // Not an actual encryption secret
        return;
    }
  }
  void operator()(const HandshakeSecrets& secret) {
    switch (secret) {
      case HandshakeSecrets::ClientHandshakeTraffic:
        callback_->clientHandshakeTrafficSecretAvailable(secretBuf_);
        return;
      case HandshakeSecrets::ServerHandshakeTraffic:
        callback_->serverHandshakeTrafficSecretAvailable(secretBuf_);
        return;
      case HandshakeSecrets::ECHAcceptConfirmation:
        // Not an actual encryption secret
        return;
    }
  }
  void operator()(const MasterSecrets& secret) {
    switch (secret) {
      case MasterSecrets::ExporterMaster:
        callback_->exporterMasterSecretAvailable(secretBuf_);
        return;
      case MasterSecrets::ResumptionMaster:
        callback_->resumptionMasterSecretAvailable(secretBuf_);
        return;
    }
  }
  void operator()(const AppTrafficSecrets& secret) {
    switch (secret) {
      case AppTrafficSecrets::ClientAppTraffic:
        callback_->clientAppTrafficSecretAvailable(secretBuf_);
        return;
      case AppTrafficSecrets::ServerAppTraffic:
        callback_->serverAppTrafficSecretAvailable(secretBuf_);
        return;
    }
  }

 private:
  AsyncFizzBase::SecretCallback* callback_;
  const std::vector<uint8_t>& secretBuf_;
};
} // namespace

void AsyncFizzBase::secretAvailable(const DerivedSecret& secret) noexcept {
  if (secretCallback_) {
    SecretVisitor visitor(secretCallback_, secret.secret);
    visitor(secret.type);
  }
}
} // namespace fizz
