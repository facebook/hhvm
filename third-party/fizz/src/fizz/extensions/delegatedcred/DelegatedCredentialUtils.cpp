/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace fizz {
namespace extensions {

void DelegatedCredentialUtils::checkExtensions(
    const folly::ssl::X509UniquePtr& cert) {
  if (!hasDelegatedExtension(cert)) {
    throw FizzException(
        "cert is missing DelegationUsage extension",
        AlertDescription::illegal_parameter);
  }

  if ((X509_get_extension_flags(cert.get()) & EXFLAG_KUSAGE) != EXFLAG_KUSAGE) {
    throw FizzException(
        "cert is missing KeyUsage extension",
        AlertDescription::illegal_parameter);
  }

  auto key_usage = X509_get_key_usage(cert.get());
  if ((key_usage & KU_DIGITAL_SIGNATURE) != KU_DIGITAL_SIGNATURE) {
    throw FizzException(
        "cert lacks digital signature key usage",
        AlertDescription::illegal_parameter);
  }
}

namespace {
static constexpr folly::StringPiece kDelegatedOid{"1.3.6.1.4.1.44363.44"};
static const auto kMaxDelegatedCredentialLifetime = std::chrono::hours(24 * 7);

folly::ssl::ASN1ObjUniquePtr generateCredentialOid() {
  folly::ssl::ASN1ObjUniquePtr oid;
  oid.reset(OBJ_txt2obj(kDelegatedOid.data(), 1));
  if (!oid) {
    throw std::runtime_error("Couldn't create OID for delegated credential");
  }
  return oid;
}
} // namespace

bool DelegatedCredentialUtils::hasDelegatedExtension(
    const folly::ssl::X509UniquePtr& cert) {
  static folly::ssl::ASN1ObjUniquePtr credentialOid = generateCredentialOid();
  // To be valid for a credential, it has to have the delegated credential
  // extension and the digitalSignature KeyUsage.
  auto credentialIdx = X509_get_ext_by_OBJ(cert.get(), credentialOid.get(), -1);
  if (credentialIdx == -1) {
    return false;
  }

  return true;
}

Buf DelegatedCredentialUtils::prepareSignatureBuffer(
    const DelegatedCredential& cred,
    Buf certData) {
  auto toSign = folly::IOBuf::create(0);
  folly::io::Appender appender(toSign.get(), 10);
  appender.pushAtMost(certData->data(), certData->length());
  detail::write(cred.valid_time, appender);
  detail::write(cred.expected_verify_scheme, appender);
  detail::writeBuf<detail::bits24>(cred.public_key, appender);
  detail::write(cred.credential_scheme, appender);
  return toSign;
}

DelegatedCredential DelegatedCredentialUtils::generateCredential(
    std::shared_ptr<SelfCert> cert,
    const folly::ssl::EvpPkeyUniquePtr& certKey,
    const folly::ssl::EvpPkeyUniquePtr& credKey,
    SignatureScheme signScheme,
    SignatureScheme verifyScheme,
    CertificateVerifyContext verifyContext,
    std::chrono::seconds validSeconds) {
  DelegatedCredential cred;
  if (verifyContext != CertificateVerifyContext::ServerDelegatedCredential &&
      verifyContext != CertificateVerifyContext::ClientDelegatedCredential) {
    throw std::runtime_error(
        "Requested credential with invalid verification context");
  }
  if (validSeconds > std::chrono::hours(24 * 7)) {
    // Can't be valid longer than a week!
    throw std::runtime_error(
        "Requested credential with exceedingly large validity");
  }

  checkExtensions(cert->getX509());

  if (X509_check_private_key(cert->getX509().get(), certKey.get()) != 1) {
    throw std::runtime_error("Cert does not match private key");
  }

  std::vector<SignatureScheme> credKeySchemes;
  switch (openssl::CertUtils::getKeyType(credKey)) {
    case openssl::KeyType::RSA:
      credKeySchemes =
          openssl::CertUtils::getSigSchemes<openssl::KeyType::RSA>();
      break;
    case openssl::KeyType::P256:
      credKeySchemes =
          openssl::CertUtils::getSigSchemes<openssl::KeyType::P256>();
      break;
    case openssl::KeyType::P384:
      credKeySchemes =
          openssl::CertUtils::getSigSchemes<openssl::KeyType::P384>();
      break;
    case openssl::KeyType::P521:
      credKeySchemes =
          openssl::CertUtils::getSigSchemes<openssl::KeyType::P521>();
      break;
    case openssl::KeyType::ED25519:
      credKeySchemes =
          openssl::CertUtils::getSigSchemes<openssl::KeyType::ED25519>();
      break;
  }

  if (std::find(credKeySchemes.begin(), credKeySchemes.end(), verifyScheme) ==
      credKeySchemes.end()) {
    throw std::runtime_error(
        "selected verification scheme not supported by credential key");
  }

  auto certSchemes = cert->getSigSchemes();
  if (std::find(certSchemes.begin(), certSchemes.end(), signScheme) ==
      certSchemes.end()) {
    throw std::runtime_error(
        "credential signature scheme not valid for parent cert");
  }

  cred.credential_scheme = signScheme;
  cred.expected_verify_scheme = verifyScheme;

  auto notBefore = X509_get0_notBefore(cert->getX509().get());
  auto notBeforeTime =
      folly::ssl::OpenSSLCertUtils::asnTimeToTimepoint(notBefore);
  auto credentialExpiresTime = std::chrono::system_clock::now() + validSeconds;
  cred.valid_time = std::chrono::duration_cast<std::chrono::seconds>(
                        credentialExpiresTime - notBeforeTime)
                        .count();

  int sz = i2d_PUBKEY(credKey.get(), nullptr);
  if (sz < 0) {
    throw std::runtime_error("failed to get delegated pkey size");
  }
  unsigned int uSz = static_cast<unsigned int>(sz);
  cred.public_key = folly::IOBuf::create(uSz);
  auto ptr = reinterpret_cast<unsigned char*>(cred.public_key->writableData());
  if (i2d_PUBKEY(credKey.get(), &ptr) < 0) {
    throw std::runtime_error("failed to convert delegated key to der");
  }
  cred.public_key->append(uSz);

  auto toSign = prepareSignatureBuffer(
      cred, folly::ssl::OpenSSLCertUtils::derEncode(*cert->getX509()));
  cred.signature =
      cert->sign(cred.credential_scheme, verifyContext, toSign->coalesce());

  return cred;
}

std::chrono::system_clock::time_point
DelegatedCredentialUtils::getCredentialExpiresTime(
    const folly::ssl::X509UniquePtr& parentCert,
    const DelegatedCredential& credential) {
  auto notBefore = X509_get0_notBefore(parentCert.get());
  auto notBeforeTime =
      folly::ssl::OpenSSLCertUtils::asnTimeToTimepoint(notBefore);
  return notBeforeTime + std::chrono::seconds(credential.valid_time);
}

void DelegatedCredentialUtils::checkCredentialTimeValidity(
    const folly::ssl::X509UniquePtr& parentCert,
    const DelegatedCredential& credential,
    const std::shared_ptr<Clock>& clock) {
  auto credentialExpiresTime = getCredentialExpiresTime(parentCert, credential);
  auto now = clock->getCurrentTime();
  if (now >= credentialExpiresTime) {
    throw FizzException(
        "credential is no longer valid", AlertDescription::certificate_expired);
  }

  // Credentials may be valid for max 1 week according to spec
  if (credentialExpiresTime - now > kMaxDelegatedCredentialLifetime) {
    throw FizzException(
        "credential validity is longer than a week from now",
        AlertDescription::illegal_parameter);
  }

  auto notAfter = X509_get0_notAfter(parentCert.get());
  auto notAfterTime =
      folly::ssl::OpenSSLCertUtils::asnTimeToTimepoint(notAfter);
  // Credential expiry time must be less than certificate's expiry time
  if (credentialExpiresTime >= notAfterTime) {
    throw FizzException(
        "credential validity is longer than parent cert validity",
        AlertDescription::illegal_parameter);
  }
}
} // namespace extensions
} // namespace fizz
