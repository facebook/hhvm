/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQSession.h>
#include <proxygen/lib/http/sink/HTTPTransactionSink.h>

#include <proxygen/lib/http/RFC2616.h>

namespace proxygen {

quic::QuicSocket* HTTPTransactionSink::getQUICTransport() const {
  auto session = httpTransaction_->getTransport().getHTTPSessionBase();
  if (auto hqSession = dynamic_cast<HQSession*>(session)) {
    return hqSession->getQuicSocket();
  }
  return nullptr;
}

bool HTTPTransactionSink::safeToUpgrade(HTTPMessage* req) const {
  // There's a race condition if we haven't seen the end of a POST
  // terminated by content-length.  If that's the case, don't request an upgrade
  // on this connection.  We can live with a little HTTP/1.1 for now.

  // Because HTTP/2 and SPDY now reported "chunked" messages for requests that
  // will see DATA frames, the only unsafe type of request is HTTP/1.1 with
  // Content-Length terminated body.  We could force this to use chunked
  // encoding to make it safe, but for now we won't.

  // One subtle case here is a pure HTTP/1.1 GET request with no body.  It is
  // safeToUpgrade per this function, but it relies on the fact that the
  // resumeIngress call after sendingHeaders will trigger onClientEOM
  // immediately, causing the sendEOM to happen before the 101 response arrives

  // And because we force chunk this request, it can't be a MUST not have body
  if (req && RFC2616::isRequestBodyAllowed(req->getMethod()) ==
                 RFC2616::BodyAllowed::NOT_ALLOWED) {
    return false;
  }
  return (httpTransaction_ && httpTransaction_->isIngressEOMSeen()) ||
         (req && (!req->getHeaders().exists(HTTP_HEADER_CONTENT_LENGTH) ||
                  req->getIsChunked()));
}

} // namespace proxygen
