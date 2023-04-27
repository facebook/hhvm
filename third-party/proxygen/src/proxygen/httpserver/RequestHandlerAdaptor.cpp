/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/RequestHandlerAdaptor.h>

#include <folly/Range.h>
#include <proxygen/httpserver/ExMessageHandler.h>
#include <proxygen/httpserver/PushHandler.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

namespace proxygen {

RequestHandlerAdaptor::RequestHandlerAdaptor(RequestHandler* requestHandler)
    : ResponseHandler(requestHandler) {
}

void RequestHandlerAdaptor::setTransaction(HTTPTransaction* txn) noexcept {
  txn_ = txn;

  // We become that transparent layer
  upstream_->setResponseHandler(this);
}

void RequestHandlerAdaptor::detachTransaction() noexcept {
  if (upstream_) {
    auto upstream = upstream_;
    upstream_ = nullptr;
    upstream->requestComplete();
  }

  // Otherwise we would have got some error call back and invoked onError
  // on RequestHandler
  delete this;
}

namespace {
constexpr folly::StringPiece k100Continue{"100-continue"};
} // namespace

void RequestHandlerAdaptor::onHeadersComplete(
    std::unique_ptr<HTTPMessage> msg) noexcept {
  if (!upstream_) {
    return;
  }
  if (msg->getHeaders().exists(HTTP_HEADER_EXPECT) &&
      !upstream_->canHandleExpect()) {
    auto expectation = msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_EXPECT);
    if (!k100Continue.equals(expectation, folly::AsciiCaseInsensitive())) {
      setError(kErrorUnsupportedExpectation);

      ResponseBuilder(this)
          .status(417, "Expectation Failed")
          .closeConnection()
          .sendWithEOM();
    } else {
      ResponseBuilder(this).status(100, "Continue").send();
    }
  }

  if (upstream_) {
    upstream_->onRequest(std::move(msg));
  }
}

void RequestHandlerAdaptor::onBody(std::unique_ptr<folly::IOBuf> c) noexcept {
  if (!upstream_) {
    return;
  }
  upstream_->onBody(std::move(c));
}

void RequestHandlerAdaptor::onChunkHeader(size_t /*length*/) noexcept {
}

void RequestHandlerAdaptor::onChunkComplete() noexcept {
}

void RequestHandlerAdaptor::onTrailers(
    std::unique_ptr<HTTPHeaders> /*trailers*/) noexcept {
  // XXX: Support trailers
}

void RequestHandlerAdaptor::onEOM() noexcept {
  if (!upstream_) {
    return;
  }
  upstream_->onEOM();
}

void RequestHandlerAdaptor::onUpgrade(UpgradeProtocol protocol) noexcept {
  if (!upstream_) {
    return;
  }
  upstream_->onUpgrade(protocol);
}

void RequestHandlerAdaptor::onError(const HTTPException& error) noexcept {
  if (!upstream_) {
    return;
  }

  if (error.getProxygenError() == kErrorTimeout) {
    setError(kErrorTimeout);

    if (!txn_->canSendHeaders()) {
      sendAbort();
    } else {
      ResponseBuilder(this)
          .status(408, "Request Timeout")
          .closeConnection()
          .sendWithEOM();
    }
  } else if (error.getDirection() == HTTPException::Direction::INGRESS) {
    setError(kErrorRead);

    if (!txn_->canSendHeaders()) {
      sendAbort();
    } else {
      ResponseBuilder(this)
          .status(400, "Bad Request")
          .closeConnection()
          .sendWithEOM();
    }

  } else {
    setError(error.hasProxygenError() ? error.getProxygenError() : kErrorWrite);
  }

  // Wait for detachTransaction to clean up
}

void RequestHandlerAdaptor::onGoaway(ErrorCode code) noexcept {
  if (!upstream_) {
    return;
  }
  upstream_->onGoaway(code);
}

void RequestHandlerAdaptor::onEgressPaused() noexcept {
  if (!upstream_) {
    return;
  }
  upstream_->onEgressPaused();
}

void RequestHandlerAdaptor::onEgressResumed() noexcept {
  if (!upstream_) {
    return;
  }
  upstream_->onEgressResumed();
}

void RequestHandlerAdaptor::onExTransaction(HTTPTransaction* txn) noexcept {
  if (!upstream_) {
    return;
  }
  // Create handler for child EX transaction.
  auto handler = new RequestHandlerAdaptor(upstream_->getExHandler());
  txn->setHandler(handler);
}

void RequestHandlerAdaptor::sendHeaders(HTTPMessage& msg) noexcept {
  txn_->sendHeaders(msg);
}

void RequestHandlerAdaptor::sendChunkHeader(size_t len) noexcept {
  txn_->sendChunkHeader(len);
}

void RequestHandlerAdaptor::sendBody(std::unique_ptr<folly::IOBuf> b) noexcept {
  txn_->sendBody(std::move(b));
}

void RequestHandlerAdaptor::sendChunkTerminator() noexcept {
  txn_->sendChunkTerminator();
}

void RequestHandlerAdaptor::sendEOM() noexcept {
  txn_->sendEOM();
}

void RequestHandlerAdaptor::sendAbort() noexcept {
  txn_->sendAbort();
}

void RequestHandlerAdaptor::refreshTimeout() noexcept {
  txn_->refreshTimeout();
}

void RequestHandlerAdaptor::pauseIngress() noexcept {
  txn_->pauseIngress();
}

void RequestHandlerAdaptor::resumeIngress() noexcept {
  txn_->resumeIngress();
}

folly::Expected<ResponseHandler*, ProxygenError>
RequestHandlerAdaptor::newPushedResponse(PushHandler* pushHandler) noexcept {
  ProxygenError error;
  auto pushTxn = txn_->newPushedTransaction(pushHandler->getHandler(), &error);
  if (!pushTxn) {
    // Codec doesn't support push
    VLOG(4) << "Failed to create newPushedResponse: "
            << static_cast<uint8_t>(error) << " " << getErrorString(error);
    return folly::makeUnexpected(error);
  }
  auto pushHandlerAdaptor = new RequestHandlerAdaptor(pushHandler);
  if (!pushHandlerAdaptor) {
    VLOG(4) << "Failed to create RequestHandlerAdaptor!";
    return folly::makeUnexpected(kErrorUnknown);
  }
  pushHandlerAdaptor->setTransaction(pushTxn);
  return pushHandlerAdaptor;
}

ResponseHandler* RequestHandlerAdaptor::newExMessage(
    ExMessageHandler* exHandler, bool unidirectional) noexcept {
  RequestHandlerAdaptor* handler = new RequestHandlerAdaptor(exHandler);
  getTransaction()->newExTransaction(handler, unidirectional);
  return handler;
}

const wangle::TransportInfo& RequestHandlerAdaptor::getSetupTransportInfo()
    const noexcept {
  return txn_->getSetupTransportInfo();
}

void RequestHandlerAdaptor::getCurrentTransportInfo(
    wangle::TransportInfo* tinfo) const {
  txn_->getCurrentTransportInfo(tinfo);
}

void RequestHandlerAdaptor::setError(ProxygenError err) noexcept {
  err_ = err;
  auto upstream = upstream_;
  upstream_ = nullptr;
  upstream->onError(err);
}

} // namespace proxygen
