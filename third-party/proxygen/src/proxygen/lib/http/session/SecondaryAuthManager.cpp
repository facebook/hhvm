/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/SecondaryAuthManager.h>

#include <fizz/extensions/exportedauth/ExportedAuthenticator.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
using folly::io::QueueAppender;

namespace proxygen {

SecondaryAuthManager::SecondaryAuthManager(
    std::unique_ptr<fizz::SelfCert> cert) {
  cert_ = std::move(cert);
}

SecondaryAuthManager::~SecondaryAuthManager() {
}

std::pair<uint16_t, std::unique_ptr<folly::IOBuf>>
SecondaryAuthManager::createAuthRequest(
    std::unique_ptr<folly::IOBuf> certRequestContext,
    std::vector<fizz::Extension> extensions) {
  // The certificate_request_context has to include the two octets Request-ID.
  uint16_t requestId = requestIdCounter_++;
  folly::IOBufQueue contextQueue{folly::IOBufQueue::cacheChainLength()};
  auto contextLen =
      sizeof(requestId) + certRequestContext->computeChainDataLength();
  QueueAppender appender(&contextQueue, contextLen);
  appender.writeBE<uint16_t>(requestId);
  contextQueue.append(std::move(certRequestContext));
  auto secureContext = contextQueue.move();
  auto authRequest = fizz::ExportedAuthenticator::getAuthenticatorRequest(
      std::move(secureContext), std::move(extensions));
  auto authRequestClone = authRequest->clone();
  outstandingRequests_.insert(
      std::make_pair(requestId, std::move(authRequest)));
  return std::make_pair(requestId, std::move(authRequestClone));
}

std::pair<uint16_t, std::unique_ptr<folly::IOBuf>>
SecondaryAuthManager::getAuthenticator(
    const fizz::AsyncFizzBase& transport,
    TransportDirection dir,
    uint16_t requestId,
    std::unique_ptr<folly::IOBuf> authRequest) {
  uint16_t certId = certIdCounter_++;
  std::unique_ptr<folly::IOBuf> authenticator;
  if (dir == TransportDirection::UPSTREAM) {
    authenticator = fizz::ExportedAuthenticator::getAuthenticator(
        transport, fizz::Direction::UPSTREAM, *cert_, std::move(authRequest));
  } else {
    authenticator = fizz::ExportedAuthenticator::getAuthenticator(
        transport, fizz::Direction::DOWNSTREAM, *cert_, std::move(authRequest));
  }
  requestCertMap_.insert(std::make_pair(requestId, certId));
  return std::make_pair(certId, std::move(authenticator));
}

bool SecondaryAuthManager::validateAuthenticator(
    const fizz::AsyncFizzBase& transport,
    TransportDirection dir,
    uint16_t certId,
    std::unique_ptr<folly::IOBuf> authenticator) {
  // Verify the certificate_request_context contains the Request-ID of a
  // previously-sent "CERTIFICATE_REQUEST".
  auto authClone = authenticator->clone();
  auto authRequest = verifyContext(std::move(authClone));
  if (!authRequest) {
    return false;
  }
  // Validate the authenticator with regard to the authenticator request.
  folly::Optional<std::vector<fizz::CertificateEntry>> certs;
  if (dir == TransportDirection::UPSTREAM) {
    certs = fizz::ExportedAuthenticator::validateAuthenticator(
        transport,
        fizz::Direction::DOWNSTREAM,
        std::move(*authRequest),
        std::move(authenticator));
  } else {
    certs = fizz::ExportedAuthenticator::validateAuthenticator(
        transport,
        fizz::Direction::UPSTREAM,
        std::move(*authRequest),
        std::move(authenticator));
  }
  if (!certs) {
    return false;
  } else if ((*certs).size() == 0) {
    VLOG(4) << "Peer does not have appropriate certificate or does not want to "
               "provide one, empty authenticator received";
  } else {
    receivedCerts_.insert(std::make_pair(certId, std::move(*certs)));
  }
  return true;
}

folly::Optional<std::unique_ptr<folly::IOBuf>>
SecondaryAuthManager::verifyContext(
    std::unique_ptr<folly::IOBuf> authenticator) {
  auto certRequestContext =
      fizz::ExportedAuthenticator::getAuthenticatorContext(
          std::move(authenticator));
  folly::io::Cursor cursor(certRequestContext.get());
  uint16_t requestId = cursor.readBE<uint16_t>();
  if (outstandingRequests_.find(requestId) == outstandingRequests_.end()) {
    VLOG(4) << "No previous CERTIFICATE_REQUEST matches the the CERTIFICATE "
               "with Request-ID="
            << requestId;
    return folly::none;
  }
  auto authRequest = std::move(outstandingRequests_[requestId]);
  return authRequest;
}

folly::Optional<uint16_t> SecondaryAuthManager::getCertId(uint16_t requestId) {
  if (requestCertMap_.find(requestId) == requestCertMap_.end()) {
    return folly::none;
  } else {
    folly::Optional<uint16_t> certId = requestCertMap_[requestId];
    return certId;
  }
}

folly::Optional<std::vector<fizz::CertificateEntry>>
SecondaryAuthManager::getPeerCert(uint16_t certId) {
  folly::Optional<std::vector<fizz::CertificateEntry>> certChain;
  if (receivedCerts_.find(certId) == receivedCerts_.end()) {
    return folly::none;
  } else {
    certChain = std::move(receivedCerts_[certId]);
    return certChain;
  }
}

} // namespace proxygen
