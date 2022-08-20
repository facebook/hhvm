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
#include <fizz/record/RecordLayer.h>
#include <glog/logging.h>
#include <system_error>

namespace fizz {
folly::AsyncSocket::ReadResult
AsyncKTLSSocket::performRead(void** buf, size_t* buflen, size_t* /* offset */) {
#if FIZZ_PLATFORM_CAPABLE_KTLS
  struct iovec iov;
  iov.iov_base = *buf;
  iov.iov_len = *buflen;

  // kTLS sends TLSInnerPlaintext.type in a 1 byte out-of-band cmsg payload.
  // The data that is read in `iov` is TLSInnerPlaintext.content
  char aux_data[CMSG_SPACE(1)];

  struct msghdr msg {};
  msg.msg_name = nullptr;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
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

  return handleNonApplicationData(
      content_type, folly::ByteRange(static_cast<unsigned char*>(*buf), bytes));
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
    folly::ByteRange payload) {
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
    return processAlert(payload);
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
    return processHandshakeData(payload);
  } else {
    VLOG(7) << "Read a non data record, of type = " << (int)type << ": "
            << folly::hexlify(payload);
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
    folly::ByteRange payload) {
  auto payloadBuf =
      folly::IOBuf::wrapBufferAsValue(payload.data(), payload.size());

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
    folly::io::Cursor cursor(&payloadBuf);
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
    folly::ByteRange payload) {
  // [note.handshake_fits_in_record_assumption]
  //
  // It is extremely likely (unless the peer is malicious, or the user
  // provides a readCallback that returns extremely small buffers) that any
  // post handshake message (of which, there are only a few) sent by the
  // peer can fit in one record (and consequently, can be handled directly
  // by this call).
  //
  // We try to parse the handshake message directly off of `payloadBuf`
  // first, before we fall back to copying the contents of the buffer into
  // an internal IOBufQueue.
  auto payloadBuf =
      folly::IOBuf::wrapBufferAsValue(payload.data(), payload.size());

  VLOG(10) << "AsyncKTLSSocket::processHandshakeData()";
  folly::Optional<fizz::Param> handshakeMessage;

  // TODO: This can probably be simplified
  try {
    if (FOLLY_LIKELY(unparsedHandshakeData_ == nullptr)) {
      // TODO: Refactor fizz::ReadRecordLayer decoding logic to work on IOBuf
      // so we can avoid constructing an iobufqueue with a single element (
      // this forces a heap allocation...)
      folly::IOBufQueue tmp{folly::IOBufQueue::cacheChainLength()};
      tmp.append(payloadBuf);
      handshakeMessage = fizz::ReadRecordLayer::decodeHandshakeMessage(tmp);
      if (!handshakeMessage) {
        unparsedHandshakeData_ = std::make_unique<folly::IOBufQueue>(
            folly::IOBufQueue::cacheChainLength());
        payloadBuf.makeManaged();
        unparsedHandshakeData_->append(payloadBuf.clone());
        return ReadResult(READ_BLOCKING);
      }
    } else {
      // We already have data saved from last time
      payloadBuf.makeManaged();
      unparsedHandshakeData_->append(payloadBuf.clone());
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

} // namespace fizz
