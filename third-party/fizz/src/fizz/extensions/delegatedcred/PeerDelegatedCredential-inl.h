/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace fizz {
namespace extensions {
template <KeyType T>
PeerDelegatedCredential<T>::PeerDelegatedCredential(
    folly::ssl::X509UniquePtr cert,
    folly::ssl::EvpPkeyUniquePtr pubKey,
    DelegatedCredential credential)
    : OpenSSLPeerCertImpl<T>(std::move(cert)),
      credential_(std::move(credential)) {
  this->signature_.setKey(std::move(pubKey));
}

template <KeyType T>
void PeerDelegatedCredential<T>::verify(
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

  // Verify signature
  auto parentCert = std::make_unique<OpenSSLPeerCertImpl<T>>(
      OpenSSLPeerCertImpl<T>::getX509());
  auto credSignBuf = DelegatedCredentialUtils::prepareSignatureBuffer(
      credential_,
      folly::ssl::OpenSSLCertUtils::derEncode(
          *OpenSSLPeerCertImpl<T>::getX509()));

  try {
    parentCert->verify(
        credential_.credential_scheme,
        CertificateVerifyContext::DelegatedCredential,
        credSignBuf->coalesce(),
        credential_.signature->coalesce());
  } catch (const std::exception& e) {
    throw FizzException(
        folly::to<std::string>(
            "failed to verify signature on credential: ", e.what()),
        AlertDescription::illegal_parameter);
  }

  // Call the parent verify method
  OpenSSLPeerCertImpl<T>::verify(
      scheme, context, std::move(toBeSigned), std::move(signature));
}

template <KeyType T>
SignatureScheme PeerDelegatedCredential<T>::getExpectedScheme() const {
  return credential_.expected_verify_scheme;
}

} // namespace extensions
} // namespace fizz
