/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <fizz/experimental/ktls/AsyncKTLSSocket.h>
#include <glog/logging.h>
#include <system_error>

namespace fizz {
folly::AsyncSocket::ReadResult AsyncKTLSSocket::performReadMsg(
    struct ::msghdr& msg,
    AsyncReader::ReadCallback::ReadMode) {
#if FIZZ_PLATFORM_CAPABLE_KTLS
  // Reading of user ancillary data would have to be implemented, but
  // currently conflicts with usage of `msg_control` below.
  DCHECK(readAncillaryDataCallback_ == nullptr);

  // kTLS sends TLSInnerPlaintext.type in a 1 byte out-of-band cmsg payload.
  // The data that is read in `iov` is TLSInnerPlaintext.content
  char aux_data[CMSG_SPACE(1)];

  // The caller already populated `msg.msg_iov*` and `msg.msg_name*`.
  msg.msg_control = aux_data;
  msg.msg_controllen = sizeof(aux_data);
  msg.msg_flags = MSG_DONTWAIT;

  ssize_t bytes;
  do {
    bytes = ::recvmsg(fd_.toFd(), &msg, MSG_DONTWAIT);
  } while (bytes < 0 && errno == EINTR);

  if (bytes < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return ReadResult(READ_BLOCKING);
    } else {
      return ReadResult(READ_ERROR);
    }
  } else if (bytes == 0) {
    return ReadResult(READ_EOF);
  }

  struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
  if (FOLLY_UNLIKELY(!cmsg)) {
    // This should never happen. If we reach here, this indicates that kTLS
    // changed on the kernel side.
    return ReadResult(
        READ_ERROR,
        std::make_unique<folly::AsyncSocketException>(
            folly::AsyncSocketException::INTERNAL_ERROR,
            "kTLS internal error: no content type"));
  }

  // The only cmsg we know how to handle right now (and that is implemented
  // in kTLS) is the record type.
  if (FOLLY_UNLIKELY(cmsg->cmsg_type != TLS_GET_RECORD_TYPE)) {
    return ReadResult(
        READ_ERROR,
        std::make_unique<folly::AsyncSocketException>(
            folly::AsyncSocketException::INTERNAL_ERROR,
            "kTLS internal error: unknown cmsg"));
  }

  uint8_t content_type = *(CMSG_DATA(cmsg));
  if (FOLLY_LIKELY(
          content_type ==
          static_cast<uint8_t>(fizz::ContentType::application_data))) {
    if (FOLLY_UNLIKELY(unparsedHandshakeData_ != nullptr)) {
      return ReadResult(
          READ_ERROR,
          std::make_unique<folly::AsyncSocketException>(
              folly::AsyncSocketException::SSL_ERROR,
              "kTLS received spliced post-handshake data"));
    }
    appBytesReceived_ += bytes;
    return ReadResult(bytes);
  }

  folly::IOBufQueue dataQ{folly::IOBufQueue::cacheChainLength()};
  dataQ.append(folly::IOBuf::wrapIov(msg.msg_iov, msg.msg_iovlen));
  dataQ.trimEnd(dataQ.chainLength() - bytes);
  return handleNonApplicationData(content_type, std::move(dataQ));
#else
  return ReadResult(
      READ_ERROR,
      std::make_unique<folly::AsyncSocketException>(
          folly::AsyncSocketException::INTERNAL_ERROR,
          "AsyncKTLSSocket used on platform incapable of kTLS"));
#endif
}

folly::AsyncSocket::ReadResult AsyncKTLSSocket::handleNonApplicationData(
    uint8_t type,
    folly::IOBufQueue payload) {
  // Ensure that if we have unparsed handshake data, then we are receiving
  // a continuation of handshake data. Alerts (e.g. close_notify) should not
  // interrupt a handshake message
  if (unparsedHandshakeData_ != nullptr &&
      type != static_cast<uint8_t>(fizz::ContentType::handshake)) {
    return ReadResult(
        READ_ERROR,
        std::make_unique<folly::AsyncSocketException>(
            folly::AsyncSocketException::SSL_ERROR,
            "kTLS received non handshake data while currently processing handshake data"));
  }

  if (type == static_cast<uint8_t>(fizz::ContentType::alert)) {
    return processAlert(std::move(payload));
  } else if (type == static_cast<uint8_t>(fizz::ContentType::handshake)) {
    // If we do not have a tls callback installed, then we cannot possibly
    // handle any post handshake messages.
    if (!tlsCallback_) {
      return ReadResult(
          READ_ERROR,
          std::make_unique<folly::AsyncSocketException>(
              folly::AsyncSocketException::SSL_ERROR,
              "kTLS received handshake data without a tls callback implementation to handle"));
    }
    return processHandshakeData(std::move(payload));
  } else {
    VLOG(7) << "Read a non data record, of type = " << (int)type << ": "
            << folly::hexlify(payload.move()->coalesce());
    return ReadResult(
        READ_ERROR,
        std::make_unique<folly::AsyncSocketException>(
            folly::AsyncSocketException::SSL_ERROR,
            folly::to<std::string>(
                "kTLS received unknown record type: ",
                static_cast<unsigned>(type))));
  }
}

folly::AsyncSocket::ReadResult AsyncKTLSSocket::processAlert(
    folly::IOBufQueue payload) {
  fizz::Alert alert;
  try {
    // Unlike handshake data, we do not buffer alerts that could not be fully
    // read.
    //
    // This situation can only happen if the user supplies a buffer smaller
    // than 2 bytes.
    //
    // If we *start* reading an alert, then it is *guaranteed* that the kernel
    // has the actual full contents of the alert in the kernel buffer, since
    // kTLS operates one record at a time (it does not signal socket readability
    // until there exists at least one full record).
    //
    // Since all alerts signal the abort of a TLS session, we do not need
    // to worry about additional application data.
    folly::io::Cursor cursor(payload.front());
    alert = fizz::decode<fizz::Alert>(cursor);
  } catch (std::exception&) {
    return ReadResult(
        READ_ERROR,
        std::make_unique<folly::AsyncSocketException>(
            folly::AsyncSocketException::SSL_ERROR,
            folly::to<std::string>(
                "error decoding alert received by ktls (buffer too small)")));
  }

  if (alert.description == AlertDescription::close_notify) {
    // TODO: Send close_notify back to peer (in practice, if SO_LINGER
    // is disabled, this may just cause a connection reset).
    return ReadResult(READ_EOF);
  } else {
    return ReadResult(
        READ_ERROR,
        std::make_unique<folly::AsyncSocketException>(
            folly::AsyncSocketException::SSL_ERROR,
            folly::to<std::string>(
                "kTLS received alert from peer: ",
                toString(alert.description))));
  }
}

folly::AsyncSocket::ReadResult AsyncKTLSSocket::processHandshakeData(
    folly::IOBufQueue payload) {
  // [note.handshake_fits_in_record_assumption]
  //
  // It is extremely likely (unless the peer is malicious, or the user
  // provides a readCallback that returns extremely small buffers) that any
  // post handshake message (of which, there are only a few) sent by the
  // peer can fit in one record (and consequently, can be handled directly
  // by this call).
  //
  // We try to parse the handshake message directly off of `payload`
  // first, before we fall back to copying the contents of the buffer into
  // an internal IOBufQueue.
  VLOG(10) << "AsyncKTLSSocket::processHandshakeData()";
  folly::Optional<fizz::Param> handshakeMessage;

  // TODO: This can probably be simplified
  try {
    if (FOLLY_LIKELY(unparsedHandshakeData_ == nullptr)) {
      handshakeMessage = fizz::ReadRecordLayer::decodeHandshakeMessage(payload);
      if (!handshakeMessage) {
        unparsedHandshakeData_ = std::make_unique<folly::IOBufQueue>(
            folly::IOBufQueue::cacheChainLength());
        auto buf = payload.move();
        buf->makeManaged();
        unparsedHandshakeData_->append(buf->clone());
        return ReadResult(READ_BLOCKING);
      }
    } else {
      // We already have data saved from last time
      auto buf = payload.move();
      buf->makeManaged();
      unparsedHandshakeData_->append(buf->clone());
      handshakeMessage = fizz::ReadRecordLayer::decodeHandshakeMessage(
          *unparsedHandshakeData_);
      if (!handshakeMessage) {
        return ReadResult(READ_BLOCKING);
      } else {
        unparsedHandshakeData_.reset();
      }
    }
  } catch (std::exception& ex) {
    // TODO: Send a decode_error alert
    return ReadResult(
        READ_ERROR,
        std::make_unique<folly::AsyncSocketException>(
            folly::AsyncSocketException::SSL_ERROR,
            folly::to<std::string>(
                "error decoding handshake data received by ktls: ",
                ex.what())));
  }

  // At this point, `handshakeMessage` is guaranteed to be valid.
  //
  // However, the *contents* of the handshake struct may be pointing to
  // borrowed memory. TLSCallback implementations must unshare() any
  // buffers prior to retaining them
  //
  // TODO is there a less error prone way to enforce this?
  DCHECK(handshakeMessage.hasValue());
  auto param = std::move(handshakeMessage).value();

  switch (param.type()) {
    case decltype(param)::Type::KeyUpdate_E:
      // The kTLS implementation in the kernel does not currently support
      // switching keys.
      VLOG(10) << "Received key update on kTLS connection";
      return ReadResult(
          READ_ERROR,
          std::make_unique<folly::AsyncSocketException>(
              folly::AsyncSocketException::SSL_ERROR,
              "ktls does not support key_updates"));
    case decltype(param)::Type::NewSessionTicket_E:
      VLOG(10) << "Received NewSessionTicket on KTLS connection";
      tlsCallback_->receivedNewSessionTicket(
          this, std::move(*param.asNewSessionTicket()));
      break;
    default:
      // Treat any other post-handshake message as an unexpected message
      // TODO: Send alert.
      return ReadResult(
          READ_ERROR,
          std::make_unique<folly::AsyncSocketException>(
              folly::AsyncSocketException::SSL_ERROR,
              "ktls received unexpected handshake content"));
  }

  // If the socket is still readable, the next record will be processed
  // in the next loop iteration. AsyncSocket is level triggered.
  return ReadResult(READ_BLOCKING);
}

// AsyncKTLSRxSocket
AsyncKTLSRxSocket::~AsyncKTLSRxSocket() {}

AsyncKTLSRxSocket::QueuedWriteRequest::QueuedWriteRequest(
    AsyncKTLSRxSocket* sock,
    folly::AsyncTransportWrapper::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf> data,
    folly::WriteFlags flags)
    : sock_(sock), callback_(callback), flags_(flags) {
  data_.append(std::move(data));
  entireChainBytesBuffered = data_.chainLength();
}

/**
 * Buffer size above which we should break up shared writes, to avoid storing
 * entire unencrypted and encrypted buffer simultaneously.
 */
static const uint32_t kPartialWriteThreshold = 128 * 1024;

void AsyncKTLSRxSocket::QueuedWriteRequest::startWriting() {
  auto buf = data_.splitAtMost(kPartialWriteThreshold);

  auto flags = flags_;
  if (!data_.empty()) {
    flags |= folly::WriteFlags::CORK;
  }
  size_t len = buf->computeChainDataLength();
  dataWritten_ += len;

  CHECK(sock_);
  CHECK(sock_->tailWriteRequest_);
  sock_->tailWriteRequest_->entireChainBytesBuffered -= len;
  sock_->writeAppData(this, std::move(buf), flags);
}

void AsyncKTLSRxSocket::QueuedWriteRequest::append(
    QueuedWriteRequest* request) {
  DCHECK(!next_);
  next_ = request;
  next_->entireChainBytesBuffered += entireChainBytesBuffered;
  entireChainBytesBuffered = 0;
}

void AsyncKTLSRxSocket::QueuedWriteRequest::unlinkFromBase() {
  sock_ = nullptr;
}

void AsyncKTLSRxSocket::QueuedWriteRequest::writeSuccess() noexcept {
  if (!data_.empty()) {
    startWriting();
  } else {
    advanceOnBase();
    auto callback = callback_;
    auto next = next_;
    auto sock = sock_;
    delete this;

    DelayedDestruction::DestructorGuard dg(sock);

    if (callback) {
      callback->writeSuccess();
    }
    if (next) {
      next->startWriting();
    }
  }
}

void AsyncKTLSRxSocket::QueuedWriteRequest::writeErr(
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

AsyncKTLSRxSocket::QueuedWriteRequest*
AsyncKTLSRxSocket::QueuedWriteRequest::deliverSingleWriteErr(
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

void AsyncKTLSRxSocket::QueuedWriteRequest::advanceOnBase() {
  if (!next_ && sock_) {
    CHECK_EQ(sock_->tailWriteRequest_, this);
    sock_->tailWriteRequest_ = nullptr;
  }
}

void AsyncKTLSRxSocket::writeChain(
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
  bool transportBuffering = this->getRawBytesBuffered() > 0;
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

void AsyncKTLSRxSocket::writeAppData(
    folly::AsyncTransportWrapper::WriteCallback* callback,
    std::unique_ptr<folly::IOBuf>&& buf,
    folly::WriteFlags flags) {
  auto data = writeRecordLayer_->writeAppData(std::move(buf), aeadOptions_);

  if (data.data && data.data->computeChainDataLength() > 0) {
    folly::AsyncSocket::writeChain(callback, std::move(data.data), flags);
  }
}
} // namespace fizz
