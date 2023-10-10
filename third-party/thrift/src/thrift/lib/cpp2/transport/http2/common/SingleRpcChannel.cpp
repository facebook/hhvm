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

#include <thrift/lib/cpp2/transport/http2/common/SingleRpcChannel.h>

#include <chrono>

#include <glog/logging.h>

#include <folly/Conv.h>
#include <folly/ExceptionWrapper.h>
#include <folly/base64.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/lib/http/HTTPHeaders.h>
#include <proxygen/lib/http/HTTPMethod.h>
#include <proxygen/lib/utils/Logging.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
#include <thrift/lib/cpp2/transport/core/EnvelopeUtil.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>

namespace apache {
namespace thrift {

using apache::thrift::transport::TTransportException;
using folly::EventBase;
using folly::EventBaseManager;
using proxygen::HTTPHeaderCode;
using proxygen::HTTPMessage;
using proxygen::HTTPMethod;
using proxygen::HTTPTransaction;
using proxygen::IOBufPrinter;
using proxygen::ProxygenError;
using std::string;

static constexpr folly::StringPiece kThriftContentType = "application/x-thrift";

SingleRpcChannel::SingleRpcChannel(
    HTTPTransaction* txn,
    ThriftProcessor* processor,
    std::shared_ptr<Cpp2Worker> worker)
    : processor_(processor),
      worker_(std::move(worker)),
      httpTransaction_(txn),
      activeRequestsGuard_{worker_->getActiveRequestsGuard()} {
  evb_ = EventBaseManager::get()->getExistingEventBase();
}

SingleRpcChannel::SingleRpcChannel(
    folly::EventBase& evb,
    folly::Function<proxygen::HTTPTransaction*(SingleRpcChannel*)>
        transactionFactory)
    : evb_(&evb), transactionFactory_(std::move(transactionFactory)) {}

SingleRpcChannel::~SingleRpcChannel() {
  if (receivedH2Stream_ && receivedThriftRPC_) {
    return;
  }
  if (!receivedH2Stream_ && !receivedThriftRPC_) {
    VLOG(2) << "Channel received nothing from Proxygen and Thrift";
  } else if (receivedH2Stream_) {
    VLOG(2) << "Channel received message from Proxygen, but not Thrift";
  } else {
    VLOG(2) << "Channel received message from Thrift, but not Proxygen";
  }
}

namespace {

template <typename HeaderMap>
bool renameHeader(
    HeaderMap& headers, std::string_view from, std::string_view to) {
  if (auto headerPtr = folly::get_ptr(headers, from)) {
    headers.insert({std::string(to), std::move(*headerPtr)});
    headers.erase(from);
    return true;
  }
  return false;
}

void preprocessExceptionHeaders(ResponseRpcMetadata& metadata) {
  auto otherMetadataRef = metadata.otherMetadata_ref();
  if (!otherMetadataRef) {
    return;
  }
  auto& otherMetadata = *otherMetadataRef;
  const bool isProxied = (bool)metadata.proxiedPayloadMetadata_ref();

  auto anyexPtr =
      folly::get_ptr(otherMetadata, apache::thrift::detail::kHeaderAnyex);
  auto anyexTypePtr =
      folly::get_ptr(otherMetadata, apache::thrift::detail::kHeaderAnyexType);

  if (anyexPtr && anyexTypePtr &&
      *anyexTypePtr == "facebook.com/servicerouter/ServiceRouterError") {
    otherMetadata.insert(
        {isProxied ? "servicerouter:sr_error"
                   : "servicerouter:sr_internal_error",
         std::move(*anyexPtr)});
    otherMetadata.erase(apache::thrift::detail::kHeaderAnyex);
    otherMetadata.erase(apache::thrift::detail::kHeaderAnyexType);
  }

  if (!isProxied) {
    return;
  }

  if (renameHeader(
          otherMetadata,
          apache::thrift::detail::kHeaderUex,
          apache::thrift::detail::kHeaderProxiedUex)) {
    renameHeader(
        otherMetadata,
        apache::thrift::detail::kHeaderUexw,
        apache::thrift::detail::kHeaderProxiedUexw);
  }

  renameHeader(
      otherMetadata,
      apache::thrift::detail::kHeaderEx,
      apache::thrift::detail::kHeaderProxiedEx);
}
} // namespace

void SingleRpcChannel::sendThriftResponse(
    ResponseRpcMetadata&& metadata, std::unique_ptr<IOBuf> payload) noexcept {
  DCHECK(evb_->isInEventBaseThread());
  VLOG(4) << "sendThriftResponse:" << std::endl
          << IOBufPrinter::printHexFolly(payload.get(), true);
  preprocessExceptionHeaders(metadata);
  if (httpTransaction_) {
    HTTPMessage msg;
    msg.setStatusCode(200);
    if (auto otherMetadata = metadata.otherMetadata_ref()) {
      encodeHeaders(std::move(*otherMetadata), msg);
    }
    httpTransaction_->sendHeaders(msg);
    httpTransaction_->sendBody(std::move(payload));
    httpTransaction_->sendEOM();
  }
  receivedThriftRPC_ = true;
}

void SingleRpcChannel::sendThriftRequest(
    RequestMetadata&& requestMetadata,
    std::unique_ptr<IOBuf> payload,
    std::unique_ptr<ThriftClientCallback> callback) noexcept {
  auto& metadata = requestMetadata.requestRpcMetadata;
  DCHECK(evb_->isInEventBaseThread());
  DCHECK(metadata.kind_ref());
  DCHECK(
      metadata.kind_ref().value_or(0) ==
          RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE ||
      metadata.kind_ref().value_or(0) == RpcKind::SINGLE_REQUEST_NO_RESPONSE);
  DCHECK(payload);
  DCHECK(callback);
  VLOG(4) << "sendThriftRequest:" << std::endl
          << IOBufPrinter::printHexFolly(payload.get(), true);
  auto callbackEvb = callback->getEventBase();
  try {
    httpTransaction_ = transactionFactory_(this);
  } catch (TTransportException& te) {
    callbackEvb->runInEventBaseThread([evbCallback = std::move(callback),
                                       evbTe = std::move(te)]() mutable {
      evbCallback->onError(
          folly::make_exception_wrapper<TTransportException>(std::move(evbTe)));
    });
    return;
  }
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::POST);
  msg.setURL(std::move(requestMetadata.url));
  auto& msgHeaders = msg.getHeaders();
  msgHeaders.set(
      HTTPHeaderCode::HTTP_HEADER_HOST, std::move(requestMetadata.host));
  msgHeaders.set(HTTPHeaderCode::HTTP_HEADER_USER_AGENT, "C++/THttpClient");
  msgHeaders.set(
      HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/x-thrift");
  if (auto clientTimeoutMs = metadata.clientTimeoutMs_ref()) {
    DCHECK(*clientTimeoutMs > 0);
    httpTransaction_->setIdleTimeout(
        std::chrono::milliseconds(*clientTimeoutMs));
  }

  apache::thrift::transport::THeader::StringToStringMap otherMetadata;
  if (auto other = metadata.otherMetadata_ref()) {
    otherMetadata = std::move(*other);
  }

  if (!otherMetadata.contains(detail::getFrameworkMetadataHttpKey())) {
    if (auto tfm_frc = metadata.frameworkMetadata_ref()) {
      // HTTP headers need to be base64-encoded
      auto bytes = (**tfm_frc).coalesce();
      auto start = bytes.data();
      auto asChar = reinterpret_cast<const char*>(start);
      std::string_view sv(asChar, bytes.size());
      std::string encoded = folly::base64Encode(sv);
      otherMetadata[detail::getFrameworkMetadataHttpKey()] = std::move(encoded);
    }
  }

  if (auto clientTimeoutMs = metadata.clientTimeoutMs_ref()) {
    otherMetadata[transport::THeader::CLIENT_TIMEOUT_HEADER] =
        folly::to<string>(*clientTimeoutMs);
  }
  if (auto queueTimeoutMs = metadata.queueTimeoutMs_ref()) {
    DCHECK(*queueTimeoutMs > 0);
    otherMetadata[transport::THeader::QUEUE_TIMEOUT_HEADER] =
        folly::to<string>(*queueTimeoutMs);
  }
  if (auto priority = metadata.priority_ref()) {
    otherMetadata[transport::THeader::PRIORITY_HEADER] =
        folly::to<string>(*priority);
  }
  if (auto kind = metadata.kind_ref()) {
    otherMetadata[RPC_KIND.str()] = folly::to<string>(*kind);
  }
  encodeHeaders(std::move(otherMetadata), msg);
  httpTransaction_->sendHeaders(msg);

  httpTransaction_->sendBody(std::move(payload));
  httpTransaction_->sendEOM();
  // For oneway calls, we move "callback" to "callbackEvb" since we do
  // not require the callback any more.  For twoway calls, we move
  // "callback" to "callback_" and use a raw pointer to call
  // "onThriftRequestSent()".  This is safe because "callback_" will
  // be eventually moved to this same thread to call either
  // "onThriftResponse()" or "onThriftError()".
  if (metadata.kind_ref().value_or(0) == RpcKind::SINGLE_REQUEST_NO_RESPONSE) {
    callbackEvb->runInEventBaseThread(
        [cb = std::move(callback)]() mutable { cb->onThriftRequestSent(); });
  } else {
    callback_ = std::move(callback);
  }
  receivedThriftRPC_ = true;
}

EventBase* SingleRpcChannel::getEventBase() noexcept {
  return evb_;
}

void SingleRpcChannel::onH2StreamBegin(
    std::unique_ptr<HTTPMessage> headers) noexcept {
  VLOG(4) << "onH2StreamBegin";
  VLOG_IF(4, headers->isResponse())
      << "onH2StreamBegin: " << headers->getStatusCode() << " "
      << headers->getStatusMessage();
  headers_ = std::move(headers);
}

void SingleRpcChannel::onH2BodyFrame(std::unique_ptr<IOBuf> contents) noexcept {
  VLOG(4) << "onH2BodyFrame: " << std::endl
          << IOBufPrinter::printHexFolly(contents.get(), true);
  if (contents_) {
    contents_->prependChain(std::move(contents));
  } else {
    contents_ = std::move(contents);
  }
}

void SingleRpcChannel::onH2StreamEnd() noexcept {
  VLOG(4) << "onH2StreamEnd";
  receivedH2Stream_ = true;
  if (processor_) {
    // Server side
    onThriftRequest();
  } else {
    // Client side
    onThriftResponse();
  }
}

void SingleRpcChannel::onH2StreamClosed(
    proxygen::ProxygenError error, std::string errorDescription) noexcept {
  VLOG(4) << "onH2StreamClosed";
  if (callback_) {
    std::unique_ptr<TTransportException> ex;
    if (error == ProxygenError::kErrorTimeout) {
      ex =
          std::make_unique<TTransportException>(TTransportException::TIMED_OUT);
    } else {
      // Some unknown error.
      ex = std::make_unique<TTransportException>(
          TTransportException::NETWORK_ERROR, errorDescription);
    }
    // We assume that the connection is still valid.  If not, we will
    // get an error the next time we try to create a new transaction
    // and can deal with it then.
    // TODO: We could also try to understand the kind of error be looking
    // at "error" in more detail.
    ex->setOptions(TTransportException::CHANNEL_IS_VALID);
    auto evb = callback_->getEventBase();
    evb->runInEventBaseThread([evbCallback = std::move(callback_),
                               evbEx = std::move(ex)]() mutable {
      evbCallback->onError(folly::make_exception_wrapper<TTransportException>(
          std::move(*evbEx)));
    });
  }
  httpTransaction_ = nullptr;
}

void SingleRpcChannel::onMessageFlushed() noexcept {
  if (callback_) {
    auto evb = callback_->getEventBase();
    // The callbacks are serialized on the EventBase, so it's safe to keep raw
    // ptr here.
    evb->runInEventBaseThread([callbackPtr = callback_.get()]() mutable {
      callbackPtr->onThriftRequestSent();
    });
  }
}

void SingleRpcChannel::onThriftRequest() noexcept {
  if (worker_->isStopping()) {
    sendThriftErrorResponse("Server shutting down");
    return;
  }
  if (!contents_) {
    sendThriftErrorResponse("Proxygen stream has no body");
    return;
  }
  RequestMetadata requestMetadata;
  auto& metadata = requestMetadata.requestRpcMetadata;
  {
    auto envelopeAndRequest =
        EnvelopeUtil::stripRequestEnvelope(std::move(contents_));
    if (!envelopeAndRequest) {
      sendThriftErrorResponse("Invalid envelope: see logs for error");
      return;
    }
    metadata.name_ref() = envelopeAndRequest->first.methodName;
    switch (envelopeAndRequest->first.protocolId) {
      case protocol::T_BINARY_PROTOCOL:
        metadata.protocol_ref() = ProtocolId::BINARY;
        break;
      case protocol::T_COMPACT_PROTOCOL:
        metadata.protocol_ref() = ProtocolId::COMPACT;
        break;
      default:
        std::terminate();
    }
    contents_ = std::move(envelopeAndRequest->second);
  }
  // Default Single Request Single Response
  metadata.kind_ref() = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  extractHeaderInfo(&metadata);

  DCHECK(metadata.protocol_ref());
  DCHECK(metadata.name_ref());
  DCHECK(metadata.kind_ref());
  if (*metadata.kind_ref() == RpcKind::SINGLE_REQUEST_NO_RESPONSE) {
    // Send a dummy response for the oneway call since we need to do
    // this with HTTP2.
    ResponseRpcMetadata responseMetadata;
    auto payload = IOBuf::createCombined(0);
    sendThriftResponse(std::move(responseMetadata), std::move(payload));
    receivedThriftRPC_ = true;
  }
  const folly::AsyncTransport* transport = httpTransaction_
      ? httpTransaction_->getTransport().getUnderlyingTransport()
      : nullptr;
  DCHECK(worker_);
  DCHECK(worker_->getEventBase()->inRunningEventBaseThread());
  auto connContext = std::make_unique<Cpp2ConnContext>(
      &headers_->getClientAddress(),
      transport,
      nullptr,
      nullptr,
      nullptr,
      worker_.get());
  processor_->onThriftRequest(
      std::move(metadata),
      std::move(contents_),
      shared_from_this(),
      std::move(connContext));
}

void SingleRpcChannel::onThriftResponse() noexcept {
  DCHECK(httpTransaction_);
  if (!callback_) {
    return;
  }

  const auto statusCode = headers_->getStatusCode();
  const auto& retryAfter = headers_->getHeaders().getSingleOrEmpty(
      proxygen::HTTP_HEADER_RETRY_AFTER);

  // HTTP 503 (Service Unavailable) + Retry-After
  if (statusCode == 503 && !retryAfter.empty()) {
    // A Retry-After header may contain a http-date or a number of seconds.
    // We do not currently attempt to parse either of these, and assume the
    // presence of the header implies we should try again due to overload.
    auto evb = callback_->getEventBase();
    ResponseRpcMetadata metadata;
    transport::THeader::StringToStringMap headers;
    // send this up the stack to be exposed as a proper app overloaded exception
    headers["ex"] = kAppOverloadedErrorCode;
    metadata.otherMetadata_ref() = std::move(headers);

    evb->runInEventBaseThread([evbCallback = std::move(callback_),
                               evbMetadata = std::move(metadata)]() mutable {
      evbCallback->onThriftResponse(std::move(evbMetadata), nullptr);
    });
    return;
  }

  const auto& contentType = headers_->getHeaders().getSingleOrEmpty(
      proxygen::HTTP_HEADER_CONTENT_TYPE);
  if (contentType != kThriftContentType && statusCode != 100 &&
      statusCode != 200) {
    auto evb = callback_->getEventBase();
    auto exWrapper = folly::make_exception_wrapper<TTransportException>(
        TTransportException::UNKNOWN,
        folly::to<std::string>(
            "Bad status: ", statusCode, " ", headers_->getStatusMessage()));
    evb->runInEventBaseThread([evbCallback = std::move(callback_),
                               exw = std::move(exWrapper)]() mutable {
      evbCallback->onError(std::move(exw));
    });
    return;
  }

  // TODO: contents_ should never be empty. For some reason, once the number
  // of queries starts going past 1.5MQPS, this contents_ will not get set.
  // For now, just drop if contents_ is null. However, this should be
  // investigated further to figure out why contents_ is not being set.
  if (!contents_) {
    VLOG(2) << "Contents has not been set.";
    auto evb = callback_->getEventBase();
    evb->runInEventBaseThread([evbCallback = std::move(callback_)]() mutable {
      evbCallback->onError(folly::make_exception_wrapper<TTransportException>(
          TTransportException::END_OF_FILE, "No content"));
    });
    return;
  }
  auto evb = callback_->getEventBase();
  ResponseRpcMetadata metadata;
  transport::THeader::StringToStringMap headers;
  decodeHeaders(*headers_, headers, /*requestMetadata=*/nullptr);
  if (!headers.empty()) {
    metadata.otherMetadata_ref() = std::move(headers);
  }

  // We don't need to set any of the other fields in metadata currently.
  evb->runInEventBaseThread([evbCallback = std::move(callback_),
                             evbMetadata = std::move(metadata),
                             evbContents = std::move(contents_)]() mutable {
    evbCallback->onThriftResponse(
        std::move(evbMetadata), std::move(evbContents));
  });
}

void SingleRpcChannel::extractHeaderInfo(
    RequestRpcMetadata* metadata) noexcept {
  transport::THeader::StringToStringMap headers;
  decodeHeaders(*headers_, headers, metadata);
  if (!headers.empty()) {
    metadata->otherMetadata_ref() = std::move(headers);
  }
}

void SingleRpcChannel::sendThriftErrorResponse(
    const string& message, ProtocolId protoId, const string& name) noexcept {
  ResponseRpcMetadata responseMetadata;
  // Not setting the "ex" header since these errors do not fit into any
  // of the existing error categories.
  TApplicationException tae(message);
  std::unique_ptr<IOBuf> payload;
  auto proto = static_cast<int16_t>(protoId);
  try {
    payload = serializeError</*includeEnvelope=*/true>(proto, tae, name, 0);
  } catch (const TProtocolException& pe) {
    // Should never happen.  Log an error and return an empty
    // payload.
    LOG(ERROR) << "serializeError failed. type=" << pe.getType()
               << " what()=" << pe.what();
    payload = IOBuf::createCombined(0);
  }
  sendThriftResponse(std::move(responseMetadata), std::move(payload));
  receivedThriftRPC_ = true;
}

} // namespace thrift
} // namespace apache
