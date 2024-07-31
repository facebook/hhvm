/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <fizz/protocol/clock/SystemClock.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace fizz {
namespace extensions {

template <openssl::KeyType T>
PeerDelegatedCredentialImpl<T>::InternalPeerCert::InternalPeerCert(
    folly::ssl::X509UniquePtr cert,
    folly::ssl::EvpPkeyUniquePtr pubKey)
    : openssl::OpenSSLPeerCertImpl<T>(std::move(cert)) {
  if (openssl::CertUtils::getKeyType(pubKey) != T) {
    throw std::runtime_error("Key and credential type don't match");
  }
  signature_.setKey(std::move(pubKey));
}

template <openssl::KeyType T>
PeerDelegatedCredentialImpl<T>::PeerDelegatedCredentialImpl(
    folly::ssl::X509UniquePtr cert,
    folly::ssl::EvpPkeyUniquePtr pubKey,
    DelegatedCredential credential)
    : peerCertImpl_(std::move(cert), std::move(pubKey)),
      credential_(std::move(credential)) {}

template <openssl::KeyType T>
void PeerDelegatedCredentialImpl<T>::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  // Verify that scheme matches expected scheme.
  if (scheme != getExpectedScheme()) {
    throw FizzException(
        "certificate verify didn't use credential's algorithm",
        AlertDescription::illegal_parameter);
  }
  auto x509 = peerCertImpl_.getX509();
  // Check extensions on cert
  DelegatedCredentialUtils::checkExtensions(x509);
  DelegatedCredentialUtils::checkCredentialTimeValidity(
      x509, credential_, clock_);

  // Verify signature
  auto credSignBuf = DelegatedCredentialUtils::prepareSignatureBuffer(
      credential_, folly::ssl::OpenSSLCertUtils::derEncode(*x509));
  auto parentCert =
      std::make_unique<openssl::OpenSSLPeerCertImpl<T>>(std::move(x509));

  try {
    parentCert->verify(
        credential_.credential_scheme,
        CertificateVerifyContext::ServerDelegatedCredential,
        credSignBuf->coalesce(),
        credential_.signature->coalesce());
  } catch (const std::exception& e) {
    throw FizzException(
        folly::to<std::string>(
            "failed to verify signature on credential: ", e.what()),
        AlertDescription::illegal_parameter);
  }

  // Call the parent verify method
  peerCertImpl_.verify(
      scheme, context, std::move(toBeSigned), std::move(signature));
}

template <openssl::KeyType T>
SignatureScheme PeerDelegatedCredentialImpl<T>::getExpectedScheme() const {
  return credential_.expected_verify_scheme;
}
} // namespace extensions
} // namespace fizz
