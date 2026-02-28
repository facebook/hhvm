/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTPChecks.h>

#include <proxygen/lib/http/RFC2616.h>

namespace proxygen {

void HTTPChecks::onHeadersComplete(StreamID stream,
                                   std::unique_ptr<HTTPMessage> msg) {

  if (msg->isRequest() &&
      (RFC2616::isRequestBodyAllowed(msg->getMethod()) ==
       RFC2616::BodyAllowed::NOT_ALLOWED) &&
      RFC2616::bodyImplied(msg->getHeaders())) {
    HTTPException ex(HTTPException::Direction::INGRESS,
                     "RFC2616: Request Body Not Allowed");
    ex.setProxygenError(kErrorParseHeader);
    // setting the status code means that the error is at the HTTP layer and
    // that parsing succeeded.
    ex.setHttpStatusCode(400);
    callback_->onError(stream, ex, true);
    return;
  }

  callback_->onHeadersComplete(stream, std::move(msg));
}

void HTTPChecks::generateHeader(
    folly::IOBufQueue& writeBuf,
    StreamID stream,
    const HTTPMessage& msg,
    bool eom,
    HTTPHeaderSize* sizeOut,
    const folly::Optional<HTTPHeaders>& extraHeaders) {
  if (msg.isRequest() && RFC2616::bodyImplied(msg.getHeaders())) {
    CHECK(RFC2616::isRequestBodyAllowed(msg.getMethod()) !=
          RFC2616::BodyAllowed::NOT_ALLOWED);
    // We could also add a "strict" mode that disallows sending body on GET
    // requests here too.
  }

  call_->generateHeader(writeBuf, stream, msg, eom, sizeOut, extraHeaders);
}

} // namespace proxygen
