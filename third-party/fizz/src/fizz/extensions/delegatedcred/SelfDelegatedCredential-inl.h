/*
 *  Copyright (c) Facebook, Inc. and its affiliates.
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
SelfDelegatedCredentialImpl<T>::InternalSelfCert::InternalSelfCert(
    std::vector<folly::ssl::X509UniquePtr> certs,
    folly::ssl::EvpPkeyUniquePtr privateKey)
    : OpenSSLSelfCertImpl<T>(std::move(certs)) {
  if (certs_.empty()) {
    throw std::runtime_error("Must supply at least 1 cert");
  }

  if (CertUtils::getKeyType(privateKey) != T) {
    throw std::runtime_error("Key and credential type don't match");
  }

  DelegatedCredentialUtils::checkExtensions(certs_.front());

  signature_.setKey(std::move(privateKey));
}

template <KeyType T>
SelfDelegatedCredentialImpl<T>::SelfDelegatedCredentialImpl(
    std::vector<folly::ssl::X509UniquePtr> certs,
    folly::ssl::EvpPkeyUniquePtr privateKey,
    DelegatedCredential credential,
    const std::vector<std::shared_ptr<CertificateCompressor>>& compressors)
    : selfCertImpl_(std::move(certs), std::move(privateKey)),
      credential_(std::move(credential)) {
  auto supportedAlgs = CertUtils::getSigSchemes<T>();
  if (std::find(
          supportedAlgs.begin(),
          supportedAlgs.end(),
          credential_.expected_verify_scheme) == supportedAlgs.end()) {
    throw std::runtime_error(
        "expected verify algorithm not supported by credential");
  }

  // Verify credential signature.
  auto signBuffer = DelegatedCredentialUtils::prepareSignatureBuffer(
      credential_, folly::ssl::OpenSSLCertUtils::derEncode(*getX509()));
  auto parentCert = CertUtils::makePeerCert(getX509());
  parentCert->verify(
      credential_.credential_scheme,
      CertificateVerifyContext::DelegatedCredential,
      signBuffer->coalesce(),
      credential_.signature->coalesce());

  for (const auto& compressor : compressors) {
    compressedCerts_[compressor->getAlgorithm()] =
        compressor->compress(getCertMessage());
  }
}

template <KeyType T>
std::string SelfDelegatedCredentialImpl<T>::getIdentity() const {
  return selfCertImpl_.getIdentity();
}

template <KeyType T>
std::vector<std::string> SelfDelegatedCredentialImpl<T>::getAltIdentities()
    const {
  return selfCertImpl_.getAltIdentities();
}

template <KeyType T>
std::vector<SignatureScheme> SelfDelegatedCredentialImpl<T>::getSigSchemes()
    const {
  return {credential_.expected_verify_scheme};
}

template <KeyType T>
CertificateMsg SelfDelegatedCredentialImpl<T>::getCertMessage(
    Buf certificateRequestContext) const {
  auto msg = selfCertImpl_.getCertMessage();
  msg.certificate_list.at(0).extensions.push_back(encodeExtension(credential_));
  return msg;
}

template <KeyType T>
CompressedCertificate SelfDelegatedCredentialImpl<T>::getCompressedCert(
    CertificateCompressionAlgorithm algo) const {
  return CertUtils::cloneCompressedCert(compressedCerts_.at(algo));
}

template <KeyType T>
Buf SelfDelegatedCredentialImpl<T>::sign(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  return selfCertImpl_.sign(scheme, context, std::move(toBeSigned));
}

template <KeyType T>
folly::ssl::X509UniquePtr SelfDelegatedCredentialImpl<T>::getX509() const {
  return selfCertImpl_.getX509();
}

template <KeyType T>
const DelegatedCredential&
SelfDelegatedCredentialImpl<T>::getDelegatedCredential() const {
  return credential_;
}

} // namespace extensions
} // namespace fizz
