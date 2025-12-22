/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPDownstreamSession.h>

#include <proxygen/lib/http/codec/HTTPCodecFactory.h>
#include <proxygen/lib/http/session/HTTPSessionController.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

namespace proxygen {

HTTPDownstreamSession::~HTTPDownstreamSession() = default;

void HTTPDownstreamSession::startNow() {
  HTTPSession::startNow();
}

void HTTPDownstreamSession::setupOnHeadersComplete(HTTPTransaction* txn,
                                                   HTTPMessage* msg) {
  VLOG(5) << "setupOnHeadersComplete txn=" << txn << ", id=" << txn->getID()
          << ", handlder=" << txn->getHandler() << ", msg=" << msg;
  if (txn->getHandler()) {
    // handler is installed before setupOnHeadersComplete callback. It must be
    // an EX_HEADERS from client side, and ENABLE_EX_HEADERS == 1
    const auto* settings = codec_->getIngressSettings();
    CHECK(settings && settings->getSetting(SettingsId::ENABLE_EX_HEADERS, 0));
    CHECK(txn->getControlStream());
    return;
  }

  // We need to find a Handler to process the transaction.
  // Note: The handler is responsible for freeing itself
  // when it has finished processing the transaction.  The
  // transaction is responsible for freeing itself when both the
  // ingress and egress messages have completed (or failed).
  HTTPTransaction::Handler* handler = nullptr;

  // In the general case, delegate to the handler factory to generate
  // a handler for the transaction.
  handler = getController()->getRequestHandler(*txn, msg);
  CHECK(handler);

  DestructorGuard dg(this);
  txn->setHandler(handler);
}

HTTPTransaction::Handler* HTTPDownstreamSession::getTransactionTimeoutHandler(
    HTTPTransaction* txn) {
  return getController()->getTransactionTimeoutHandler(txn, getLocalAddress());
}

void HTTPDownstreamSession::onHeadersSent(const HTTPMessage& headers,
                                          bool codecWasReusable) {
  if (!codec_->isReusable()) {
    // If the codec turned unreusable, some thing wrong must have happened.
    // Basically, the proxy decides the connection is not reusable.
    // e.g, an error message is being sent with Connection: close
    if (codecWasReusable) {
      uint32_t statusCode = headers.getStatusCode();
      if (statusCode >= 500) {
        setCloseReason(ConnectionCloseReason::REMOTE_ERROR);
      } else {
        if (statusCode >= 400) {
          setCloseReason(ConnectionCloseReason::ERR_RESP);
        } else {
          // should not be here
          setCloseReason(ConnectionCloseReason::UNKNOWN);
        }
      }
    } else {
      // shouldn't happen... this case is detected by REQ_NOTREUSABLE
      setCloseReason(ConnectionCloseReason::UNKNOWN);
    }
  }
}

bool HTTPDownstreamSession::allTransactionsStarted() const {
  for (const auto& txn : transactions_) {
    if (txn.second.isPushed() && !txn.second.isEgressStarted()) {
      return false;
    }
  }
  return true;
}

} // namespace proxygen
