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

#include <thrift/lib/cpp2/async/HeaderServerChannel.h>

#include <exception>
#include <utility>

#include <folly/String.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/HeaderChannelTrait.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using std::make_unique;
using std::unique_ptr;
using namespace apache::thrift::transport;
using apache::thrift::protocol::PROTOCOL_TYPES;
using apache::thrift::server::TServerObserver;

namespace apache {
namespace thrift {

std::atomic<uint32_t> HeaderServerChannel::sample_(0);

HeaderServerChannel::HeaderServerChannel(
    const std::shared_ptr<folly::AsyncTransport>& transport)
    : HeaderServerChannel(std::shared_ptr<Cpp2Channel>(Cpp2Channel::newChannel(
          transport, make_unique<ServerFramingHandler>(*this)))) {}

HeaderServerChannel::HeaderServerChannel(
    const std::shared_ptr<Cpp2Channel>& cpp2Channel)
    : callback_(nullptr),
      arrivalSeqId_(1),
      lastWrittenSeqId_(0),
      sampleRate_(0),
      cpp2Channel_(cpp2Channel) {}

void HeaderServerChannel::destroy() {
  DestructorGuard dg(this);

  if (callback_) {
    auto error =
        folly::make_exception_wrapper<TTransportException>("Channel destroyed");
    callback_->channelClosed(std::move(error));
  }

  cpp2Channel_->closeNow();

  folly::DelayedDestruction::destroy();
}

// Header framing
unique_ptr<IOBuf> HeaderServerChannel::ServerFramingHandler::addFrame(
    unique_ptr<IOBuf> buf, THeader* header) {
  // Note: This THeader function may throw.  However, we don't want to catch
  // it here, because this would send an empty message out on the wire.
  // Instead we have to catch it at sendMessage
  return header->addHeader(
      std::move(buf), false /* Data already transformed in AsyncProcessor.h */);
}

std::tuple<unique_ptr<IOBuf>, size_t, unique_ptr<THeader>>
HeaderServerChannel::ServerFramingHandler::removeFrame(IOBufQueue* q) {
  std::unique_ptr<THeader> header(new THeader(THeader::ALLOW_BIG_FRAMES));
  // removeHeader will set seqid in header.
  // For older clients with seqid in the protocol, header
  // will dig in to the protocol to get the seqid correctly.
  if (!q || !q->front() || q->front()->empty()) {
    return make_tuple(std::unique_ptr<IOBuf>(), 0, nullptr);
  }

  std::unique_ptr<folly::IOBuf> buf;
  size_t remaining = 0;
  try {
    buf = header->removeHeader(q, remaining, channel_.persistentReadHeaders_);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Received invalid request from client: "
               << folly::exceptionStr(e) << " "
               << getTransportDebugString(channel_.getTransport());
    throw;
  }
  if (!buf) {
    return make_tuple(std::unique_ptr<IOBuf>(), remaining, nullptr);
  }

  CLIENT_TYPE ct = header->getClientType();
  if (!HeaderChannelTrait::isSupportedClient(ct)) {
    LOG(ERROR) << "Server rejecting unsupported client type " << ct;
    HeaderChannelTrait::checkSupportedClient(ct);
  }

  // Check if protocol used in the buffer is consistent with the protocol
  // id in the header.

  folly::io::Cursor c(buf.get());
  auto byte = c.read<uint8_t>();
  // Initialize it to a value never used on the wire
  PROTOCOL_TYPES protInBuf = PROTOCOL_TYPES::T_DEBUG_PROTOCOL;
  if (byte == 0x82) {
    protInBuf = PROTOCOL_TYPES::T_COMPACT_PROTOCOL;
  } else if (byte == 0x80) {
    protInBuf = PROTOCOL_TYPES::T_BINARY_PROTOCOL;
  } else if (ct != THRIFT_HTTP_SERVER_TYPE) {
    LOG(ERROR) << "Received corrupted request from client: "
               << getTransportDebugString(channel_.getTransport()) << ". "
               << "Corrupted payload in header message. In message header, "
               << "protoId: " << header->getProtocolId() << ", "
               << "clientType: " << folly::to<std::string>(ct) << ". "
               << "First few bytes of payload: "
               << getTHeaderPayloadString(buf.get());
    throw TTransportException(
        TTransportException::INVALID_STATE,
        "Receiving corrupted request from client");
  }

  if (protInBuf != PROTOCOL_TYPES::T_DEBUG_PROTOCOL &&
      header->getProtocolId() != protInBuf) {
    LOG(ERROR) << "Received corrupted request from client: "
               << getTransportDebugString(channel_.getTransport()) << ". "
               << "Protocol mismatch, in message header, protocolId: "
               << folly::to<std::string>(header->getProtocolId()) << ", "
               << "clientType: " << folly::to<std::string>(ct) << ", "
               << "in payload, protocolId: "
               << folly::to<std::string>(protInBuf)
               << ". First few bytes of payload: "
               << getTHeaderPayloadString(buf.get());
  }

  return make_tuple(std::move(buf), 0, std::move(header));
}

std::string HeaderServerChannel::getTHeaderPayloadString(IOBuf* buf) {
  auto len = std::min<size_t>(buf->length(), 20);
  return folly::cEscape<std::string>(
      folly::StringPiece((const char*)buf->data(), len));
}

std::string HeaderServerChannel::getTransportDebugString(
    folly::AsyncTransport* transport) {
  if (!transport) {
    return std::string();
  }

  auto ret = folly::to<std::string>(
      "(transport ", folly::demangle(typeid(*transport)));

  try {
    folly::SocketAddress addr;
    transport->getPeerAddress(&addr);
    folly::toAppend(
        ", address ", addr.getAddressStr(), ", port ", addr.getPort(), &ret);
  } catch (const std::exception&) {
  }

  ret += ')';
  return ret;
}

// Client Interface

HeaderServerChannel::HeaderRequest::HeaderRequest(
    HeaderServerChannel* channel,
    unique_ptr<IOBuf>&& buf,
    unique_ptr<THeader>&& header,
    const SamplingStatus& samplingStatus)
    : channel_(channel),
      header_(std::move(header)),
      samplingStatus_(samplingStatus) {
  this->buf_ = std::move(buf);
}

/**
 * send a reply to the client.
 *
 * Note that to be backwards compatible with thrift1, the generated
 * code calls sendReply(nullptr) for oneway calls where seqid !=
 * ONEWAY_SEQ_ID.  This is so that the sendCatchupRequests code runs
 * correctly for in-order responses to older clients.  sendCatchupRequests
 * does not actually send null buffers, it just ignores them.
 *
 */
void HeaderServerChannel::HeaderRequest::sendReply(
    ResponsePayload&& response,
    MessageChannel::SendCallback* cb,
    folly::Optional<uint32_t>) {
  // timeoutHeader_ is set in ::sendTimeoutResponse below
  auto& header = timeoutHeader_ ? timeoutHeader_ : header_;
  if (!channel_->outOfOrder_.value()) {
    // In order processing, make sure the ordering is correct.
    if (InOrderRecvSeqId_ != channel_->lastWrittenSeqId_ + 1) {
      // Save it until we can send it in order.
      channel_->inOrderRequests_[InOrderRecvSeqId_] =
          std::make_tuple(cb, std::move(response).buffer(), std::move(header));
    } else {
      // Send it now, and send any subsequent requests in order.
      channel_->sendCatchupRequests(
          std::move(response).buffer(), cb, header.get());
    }
  } else {
    if (!response) {
      if (cb) {
        cb->messageSent();
      }
      return;
    }
    try {
      // out of order, send as soon as it is done.
      channel_->sendMessage(cb, std::move(response).buffer(), header.get());
    } catch (const std::exception& e) {
      LOG(ERROR) << "Failed to send message: " << e.what();
    }
  }
}

void HeaderServerChannel::HeaderRequest::serializeAndSendError(
    apache::thrift::transport::THeader& header,
    TApplicationException& tae,
    const std::string& methodName,
    int32_t protoSeqId,
    MessageChannel::SendCallback* cb) {
  if (isOneway()) {
    sendReply(ResponsePayload{}, cb);
    return;
  }

  std::unique_ptr<folly::IOBuf> exbuf;
  uint16_t proto = header.getProtocolId();
  try {
    exbuf = serializeError</*includeEnvelope=*/true>(
        proto, tae, methodName, protoSeqId);
  } catch (const TProtocolException& pe) {
    LOG(ERROR) << "serializeError failed. type=" << pe.getType()
               << " what()=" << pe.what();
    channel_->closeNow();
    return;
  }
  LegacySerializedResponse response{std::move(exbuf)};
  auto [_, responsePayload] =
      std::move(response).extractPayload(/*includeEnvelope=*/true, proto);
  responsePayload.transform(header_->getWriteTransforms());
  sendException(std::move(responsePayload), cb);
}

/**
 * Send a serialized error back to the client.
 * For a header server, this means serializing the exception, and setting
 * an error flag in the header.
 */
void HeaderServerChannel::HeaderRequest::sendErrorWrapped(
    folly::exception_wrapper ew, std::string exCode) {
  // Other types are unimplemented.
  DCHECK(ew.is_compatible_with<TApplicationException>());

  header_->setHeader("ex", exCode);
  ew.with_exception([&](TApplicationException& tae) {
    std::unique_ptr<folly::IOBuf> exbuf;
    uint16_t proto = header_->getProtocolId();
    try {
      exbuf = serializeError</*includeEnvelope=*/true>(proto, tae, getBuf());
    } catch (const TProtocolException& pe) {
      LOG(ERROR) << "serializeError failed. type=" << pe.getType()
                 << " what()=" << pe.what();
      channel_->closeNow();
      return;
    }
    LegacySerializedResponse response{std::move(exbuf)};
    auto [mtype, responsePayload] =
        std::move(response).extractPayload(/*includeEnvelope=*/true, proto);
    responsePayload.transform(header_->getWriteTransforms());
    if (mtype == MessageType::T_EXCEPTION) {
      sendException(std::move(responsePayload), nullptr);
    } else {
      sendReply(std::move(responsePayload), nullptr);
    }
  });
}

void HeaderServerChannel::HeaderRequest::sendErrorWrapped(
    folly::exception_wrapper ew,
    std::string exCode,
    const std::string& methodName,
    int32_t protoSeqId,
    MessageChannel::SendCallback* cb) {
  // Other types are unimplemented.
  DCHECK(ew.is_compatible_with<TApplicationException>());

  header_->setHeader("ex", exCode);
  ew.with_exception([&](TApplicationException& tae) {
    serializeAndSendError(*header_, tae, methodName, protoSeqId, cb);
  });
}

void HeaderServerChannel::HeaderRequest::sendTimeoutResponse(
    const std::string& methodName,
    int32_t protoSeqId,
    MessageChannel::SendCallback* cb,
    const transport::THeader::StringToStringMap& headers,
    TimeoutResponseType responseType) {
  // Sending timeout response always happens on eb thread, while normal
  // request handling might still be work-in-progress on tm thread and
  // touches the per-request THeader at any time. This builds a new THeader
  // and only reads certain fields from header_. To avoid race condition,
  // DO NOT read any header from the per-request THeader.
  timeoutHeader_ = std::make_unique<THeader>();
  timeoutHeader_->copyMetadataFrom(*header_);
  auto errorCode = responseType == TimeoutResponseType::QUEUE
      ? kServerQueueTimeoutErrorCode
      : kTaskExpiredErrorCode;
  timeoutHeader_->setHeader("ex", errorCode);
  auto errorMsg = responseType == TimeoutResponseType::QUEUE ? "Queue Timeout"
                                                             : "Task expired";
  for (const auto& it : headers) {
    timeoutHeader_->setHeader(it.first, it.second);
  }

  TApplicationException tae(
      TApplicationException::TApplicationExceptionType::TIMEOUT, errorMsg);
  serializeAndSendError(*timeoutHeader_, tae, methodName, protoSeqId, cb);
}

void HeaderServerChannel::sendCatchupRequests(
    std::unique_ptr<folly::IOBuf> next_req,
    MessageChannel::SendCallback* cb,
    THeader* header) {
  DestructorGuard dg(this);

  std::unique_ptr<THeader> header_ptr;
  while (true) {
    if (next_req) {
      try {
        sendMessage(cb, std::move(next_req), header);
      } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to send message: " << e.what();
      }
    } else if (nullptr != cb) {
      // There is no message (like a oneway req), but there is a callback
      cb->messageSent();
    }
    lastWrittenSeqId_++;

    // Check for the next req
    auto next = inOrderRequests_.find(lastWrittenSeqId_ + 1);
    if (next != inOrderRequests_.end()) {
      next_req = std::move(std::get<1>(next->second));
      cb = std::get<0>(next->second);
      header_ptr = std::move(std::get<2>(next->second));
      header = header_ptr.get();
      inOrderRequests_.erase(next);
    } else {
      break;
    }
  }
}

TServerObserver::SamplingStatus HeaderServerChannel::shouldSample(
    const apache::thrift::transport::THeader* header) const {
  bool isServerSamplingEnabled =
      (sampleRate_ > 0) && ((sample_++ % sampleRate_) == 0);

  if (auto loggingContext = header->loggingContext()) {
    auto clientLogSampleRatio = *loggingContext->logSampleRatio();
    auto clientLogErrorSampleRatio = *loggingContext->logErrorSampleRatio();
    if (clientLogSampleRatio > 0 || clientLogErrorSampleRatio > 0) {
      return SamplingStatus(
          isServerSamplingEnabled,
          clientLogSampleRatio,
          clientLogErrorSampleRatio);
    }
  }
  bool isClientSamplingEnabledLegacy =
      header->getHeaders().find(kClientLoggingHeader.str()) !=
      header->getHeaders().end();
  return SamplingStatus(isServerSamplingEnabled, isClientSamplingEnabledLegacy);
}

// Interface from MessageChannel::RecvCallback
void HeaderServerChannel::messageReceived(
    unique_ptr<IOBuf>&& buf, unique_ptr<THeader>&& header) {
  DestructorGuard dg(this);

  bool outOfOrder = (header->getFlags() & HEADER_FLAG_SUPPORT_OUT_OF_ORDER);
  if (!outOfOrder_.has_value()) {
    outOfOrder_ = outOfOrder;
  } else if (outOfOrder_.value() != outOfOrder) {
    LOG(ERROR) << "Channel " << (outOfOrder_.value() ? "" : "doesn't ")
               << "support out-of-order, but received a message with the "
               << "out-of-order bit " << (outOfOrder ? "set" : "unset");
    messageReceiveErrorWrapped(
        folly::make_exception_wrapper<TTransportException>(
            "Bad out-of-order flag"));
    return;
  }

  if (callback_) {
    auto sampleStatus = shouldSample(header.get());
    unique_ptr<HeaderRequest> request(new HeaderRequest(
        this, std::move(buf), std::move(header), sampleStatus));

    if (!outOfOrder) {
      if (inOrderRequests_.size() > MAX_REQUEST_SIZE) {
        // There is probably nothing useful we can do here.
        LOG(WARNING) << "Hit in order request buffer limit";
        auto ex = folly::make_exception_wrapper<TTransportException>(
            "Hit in order request buffer limit");
        messageReceiveErrorWrapped(std::move(ex));
        return;
      }
      if (!request->isOneway()) {
        // Create a new seqid for in-order messages because they might not
        // be sequential.  This seqid is only used internally in
        // HeaderServerChannel
        request->setInOrderRecvSequenceId(arrivalSeqId_++);
      }
    }

    auto ew = folly::try_and_catch(
        [&]() { callback_->requestReceived(std::move(request)); });
    if (ew) {
      LOG(WARNING) << "Could not parse request: " << ew.what();
      messageReceiveErrorWrapped(std::move(ew));
      return;
    }
  }
}

void HeaderServerChannel::messageChannelEOF() {
  DestructorGuard dg(this);

  auto ew =
      folly::make_exception_wrapper<TTransportException>("Channel Closed");
  if (callback_) {
    callback_->channelClosed(std::move(ew));
  }
}

void HeaderServerChannel::messageReceiveErrorWrapped(
    folly::exception_wrapper&& ex) {
  DestructorGuard dg(this);

  VLOG(1) << "Receive error: " << ex.what();

  if (callback_) {
    callback_->channelClosed(std::move(ex));
  }
}

void HeaderServerChannel::setCallback(ResponseChannel::Callback* callback) {
  HeaderServerChannel::Callback* cob = dynamic_cast<Callback*>(callback);
  DCHECK(!!cob == !!callback); // assert that dynamic cast succeeded
  callback_ = cob;

  if (cob) {
    cpp2Channel_->setReceiveCallback(this);
  } else {
    cpp2Channel_->setReceiveCallback(nullptr);
  }
}

} // namespace thrift
} // namespace apache
