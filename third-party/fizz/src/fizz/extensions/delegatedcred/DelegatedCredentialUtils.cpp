/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <fizz/util/Logging.h>
#include <folly/ssl/OpenSSLCertUtils.h>

namespace fizz {
namespace extensions {

Status DelegatedCredentialUtils::checkExtensions(
    Error& err,
    const folly::ssl::X509UniquePtr& cert) {
  bool hasDelegated = false;
  FIZZ_RETURN_ON_ERROR(hasDelegatedExtension(hasDelegated, err, cert));
  if (!hasDelegated) {
    return err.error(
        "cert is missing DelegationUsage extension",
        AlertDescription::illegal_parameter);
  }

  if ((X509_get_extension_flags(cert.get()) & EXFLAG_KUSAGE) != EXFLAG_KUSAGE) {
    return err.error(
        "cert is missing KeyUsage extension",
        AlertDescription::illegal_parameter);
  }

  auto key_usage = X509_get_key_usage(cert.get());
  if ((key_usage & KU_DIGITAL_SIGNATURE) != KU_DIGITAL_SIGNATURE) {
    return err.error(
        "cert lacks digital signature key usage",
        AlertDescription::illegal_parameter);
  }
  return Status::Success;
}

namespace {
static constexpr folly::StringPiece kDelegatedOid{"1.3.6.1.4.1.44363.44"};
static const auto kMaxDelegatedCredentialLifetime = std::chrono::hours(24 * 7);

Status generateCredentialOid(const ASN1_OBJECT*& ret, Error& err) {
  static const ASN1_OBJECT* oid = OBJ_txt2obj(kDelegatedOid.data(), 1);
  if (!oid) {
    return err.error("Couldn't create OID for delegated credential");
  }
  ret = oid;
  return Status::Success;
}
} // namespace

Status DelegatedCredentialUtils::hasDelegatedExtension(
    bool& ret,
    Error& err,
    const folly::ssl::X509UniquePtr& cert) {
  const ASN1_OBJECT* credentialOid = nullptr;
  FIZZ_RETURN_ON_ERROR(generateCredentialOid(credentialOid, err));
  // To be valid for a credential, it has to have the delegated credential
  // extension and the digitalSignature KeyUsage.
  FIZZ_DCHECK_NE(credentialOid, nullptr);
  auto credentialIdx = X509_get_ext_by_OBJ(cert.get(), credentialOid, -1);
  ret = (credentialIdx != -1);
  return Status::Success;
}

Status DelegatedCredentialUtils::prepareSignatureBuffer(
    Buf& ret,
    Error& err,
    const DelegatedCredential& cred,
    Buf certData) {
  auto toSign = folly::IOBuf::create(0);
  folly::io::Appender appender(toSign.get(), 10);
  appender.pushAtMost(certData->data(), certData->length());
  FIZZ_RETURN_ON_ERROR(detail::write(err, cred.valid_time, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::write(err, cred.expected_verify_scheme, appender));
  FIZZ_RETURN_ON_ERROR(
      detail::writeBuf<detail::bits24>(err, cred.public_key, appender));
  FIZZ_RETURN_ON_ERROR(detail::write(err, cred.credential_scheme, appender));
  ret = std::move(toSign);
  return Status::Success;
}

Status DelegatedCredentialUtils::generateCredential(
    DelegatedCredential& ret,
    Error& err,
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
    return err.error("Requested credential with invalid verification context");
  }
  if (validSeconds > std::chrono::hours(24 * 7)) {
    return err.error("Requested credential with exceedingly large validity");
  }

  FIZZ_RETURN_ON_ERROR(checkExtensions(err, cert->getX509()));

  if (X509_check_private_key(cert->getX509().get(), certKey.get()) != 1) {
    return err.error("Cert does not match private key");
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
    return err.error(
        "selected verification scheme not supported by credential key");
  }

  auto certSchemes = cert->getSigSchemes();
  if (std::find(certSchemes.begin(), certSchemes.end(), signScheme) ==
      certSchemes.end()) {
    return err.error("credential signature scheme not valid for parent cert");
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
    return err.error("failed to get delegated pkey size");
  }
  unsigned int uSz = static_cast<unsigned int>(sz);
  cred.public_key = folly::IOBuf::create(uSz);
  auto ptr = reinterpret_cast<unsigned char*>(cred.public_key->writableData());
  if (i2d_PUBKEY(credKey.get(), &ptr) < 0) {
    return err.error("failed to convert delegated key to der");
  }
  cred.public_key->append(uSz);

  Buf toSign;
  FIZZ_RETURN_ON_ERROR(prepareSignatureBuffer(
      toSign,
      err,
      cred,
      folly::ssl::OpenSSLCertUtils::derEncode(*cert->getX509())));
  cred.signature =
      cert->sign(cred.credential_scheme, verifyContext, toSign->coalesce());

  ret = std::move(cred);
  return Status::Success;
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

Status DelegatedCredentialUtils::checkCredentialTimeValidity(
    Error& err,
    const folly::ssl::X509UniquePtr& parentCert,
    const DelegatedCredential& credential,
    const std::shared_ptr<Clock>& clock) {
  auto credentialExpiresTime = getCredentialExpiresTime(parentCert, credential);
  auto now = clock->getCurrentTime();
  if (now >= credentialExpiresTime) {
    return err.error(
        "credential is no longer valid", AlertDescription::certificate_expired);
  }

  // Credentials may be valid for max 1 week according to spec
  if (credentialExpiresTime - now > kMaxDelegatedCredentialLifetime) {
    return err.error(
        "credential validity is longer than a week from now",
        AlertDescription::illegal_parameter);
  }

  auto notAfter = X509_get0_notAfter(parentCert.get());
  auto notAfterTime =
      folly::ssl::OpenSSLCertUtils::asnTimeToTimepoint(notAfter);
  // Credential expiry time must be less than certificate's expiry time
  if (credentialExpiresTime >= notAfterTime) {
    return err.error(
        "credential validity is longer than parent cert validity",
        AlertDescription::illegal_parameter);
  }
  return Status::Success;
}
} // namespace extensions
} // namespace fizz
