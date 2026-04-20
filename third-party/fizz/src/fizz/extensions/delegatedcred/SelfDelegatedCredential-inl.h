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

template <openssl::KeyType T>
/* static */ Status SelfDelegatedCredentialImpl<T>::InternalSelfCert::create(
    std::unique_ptr<SelfDelegatedCredentialImpl<T>::InternalSelfCert>& ret,
    Error& err,
    std::vector<folly::ssl::X509UniquePtr> certs,
    folly::ssl::EvpPkeyUniquePtr privateKey) {
  if (certs.empty()) {
    return err.error("Must supply at least 1 cert");
  }
  if (openssl::CertUtils::getKeyType(privateKey) != T) {
    return err.error("Key and credential type don't match");
  }
  FIZZ_RETURN_ON_ERROR(
      DelegatedCredentialUtils::checkExtensions(err, certs.front()));

  ret = std::unique_ptr<InternalSelfCert>(
      new InternalSelfCert(std::move(certs), std::move(privateKey)));
  return Status::Success;
}

template <openssl::KeyType T>
/* static */ Status SelfDelegatedCredentialImpl<T>::create(
    std::unique_ptr<SelfDelegatedCredentialImpl>& ret,
    Error& err,
    DelegatedCredentialMode mode,
    std::vector<folly::ssl::X509UniquePtr> certs,
    folly::ssl::EvpPkeyUniquePtr privateKey,
    DelegatedCredential credential,
    const std::vector<std::shared_ptr<CertificateCompressor>>& compressors) {
  // Create InternalSelfCert (validates certs, key type, extensions).
  std::unique_ptr<InternalSelfCert> selfCert;
  FIZZ_RETURN_ON_ERROR(
      InternalSelfCert::create(
          selfCert, err, std::move(certs), std::move(privateKey)));

  // Validate supported algorithms.
  auto supportedAlgs = openssl::CertUtils::getSigSchemes<T>();
  if (std::find(
          supportedAlgs.begin(),
          supportedAlgs.end(),
          credential.expected_verify_scheme) == supportedAlgs.end()) {
    return err.error("expected verify algorithm not supported by credential");
  }

  // Verify credential signature.
  Buf signBuffer;
  FIZZ_RETURN_ON_ERROR(
      DelegatedCredentialUtils::prepareSignatureBuffer(
          signBuffer,
          err,
          credential,
          folly::ssl::OpenSSLCertUtils::derEncode(*selfCert->getX509())));
  auto parentCert = openssl::CertUtils::makePeerCert(selfCert->getX509());
  FIZZ_RETURN_ON_ERROR(parentCert->verify(
      err,
      credential.credential_scheme,
      mode == DelegatedCredentialMode::Server
          ? CertificateVerifyContext::ServerDelegatedCredential
          : CertificateVerifyContext::ClientDelegatedCredential,
      signBuffer->coalesce(),
      credential.signature->coalesce()));

  // Prepare compressed certs.
  std::map<CertificateCompressionAlgorithm, CompressedCertificate>
      compressedCerts;
  for (const auto& compressor : compressors) {
    CertificateMsg certMsg;
    FIZZ_RETURN_ON_ERROR(selfCert->getCertMessage(certMsg, err, nullptr));
    Extension ext;
    FIZZ_RETURN_ON_ERROR(encodeExtension(ext, err, credential));
    certMsg.certificate_list.at(0).extensions.push_back(std::move(ext));
    compressedCerts[compressor->getAlgorithm()] = compressor->compress(certMsg);
  }

  ret = std::unique_ptr<SelfDelegatedCredentialImpl<T>>(
      new SelfDelegatedCredentialImpl<T>(
          std::move(selfCert),
          std::move(credential),
          std::move(compressedCerts)));
  return Status::Success;
}

template <openssl::KeyType T>
std::string SelfDelegatedCredentialImpl<T>::getIdentity() const {
  return selfCertImpl_->getIdentity();
}

template <openssl::KeyType T>
std::vector<std::string> SelfDelegatedCredentialImpl<T>::getAltIdentities()
    const {
  return selfCertImpl_->getAltIdentities();
}

template <openssl::KeyType T>
std::vector<SignatureScheme> SelfDelegatedCredentialImpl<T>::getSigSchemes()
    const {
  return {credential_.expected_verify_scheme};
}

template <openssl::KeyType T>
Status SelfDelegatedCredentialImpl<T>::getCertMessage(
    CertificateMsg& ret,
    Error& err,
    Buf certificateRequestContext) const {
  CertificateMsg msg;
  FIZZ_RETURN_ON_ERROR(selfCertImpl_->getCertMessage(
      msg, err, std::move(certificateRequestContext)));
  Extension ext;
  FIZZ_RETURN_ON_ERROR(encodeExtension(ext, err, credential_));
  msg.certificate_list.at(0).extensions.push_back(std::move(ext));
  ret = std::move(msg);
  return Status::Success;
}

template <openssl::KeyType T>
CompressedCertificate SelfDelegatedCredentialImpl<T>::getCompressedCert(
    CertificateCompressionAlgorithm algo) const {
  return openssl::CertUtils::cloneCompressedCert(compressedCerts_.at(algo));
}

template <openssl::KeyType T>
Buf SelfDelegatedCredentialImpl<T>::sign(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  return selfCertImpl_->sign(scheme, context, std::move(toBeSigned));
}

template <openssl::KeyType T>
folly::ssl::X509UniquePtr SelfDelegatedCredentialImpl<T>::getX509() const {
  return selfCertImpl_->getX509();
}

template <openssl::KeyType T>
const DelegatedCredential&
SelfDelegatedCredentialImpl<T>::getDelegatedCredential() const {
  return credential_;
}

} // namespace extensions
} // namespace fizz
