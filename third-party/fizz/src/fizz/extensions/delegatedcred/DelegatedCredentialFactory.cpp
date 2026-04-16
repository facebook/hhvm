/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/delegatedcred/DelegatedCredentialFactory.h>
#include <fizz/extensions/delegatedcred/PeerDelegatedCredential.h>
#include <folly/portability/OpenSSL.h>

namespace fizz {
namespace extensions {

namespace {
Status makeCredential(
    std::unique_ptr<PeerCert>& ret,
    Error& err,
    DelegatedCredential&& credential,
    folly::ssl::X509UniquePtr cert) {
  FIZZ_VLOG(4) << "Making delegated credential";
  // Parse pubkey
  auto pubKeyRange = credential.public_key->coalesce();
  auto addr = pubKeyRange.data();
  folly::ssl::EvpPkeyUniquePtr pubKey(
      d2i_PUBKEY(nullptr, &addr, pubKeyRange.size()));
  if (!pubKey) {
    return err.error(
        "failed to create credential pubkey",
        AlertDescription::illegal_parameter);
  }

  switch (openssl::CertUtils::getKeyType(pubKey)) {
    case openssl::KeyType::RSA: {
      std::unique_ptr<PeerDelegatedCredentialImpl<openssl::KeyType::RSA>>
          credRSA;
      FIZZ_RETURN_ON_ERROR(
          PeerDelegatedCredentialImpl<openssl::KeyType::RSA>::create(
              credRSA,
              err,
              std::move(cert),
              std::move(pubKey),
              std::move(credential)));
      ret = std::move(credRSA);
      return Status::Success;
    }
    case openssl::KeyType::P256: {
      std::unique_ptr<PeerDelegatedCredentialImpl<openssl::KeyType::P256>>
          credP256;
      FIZZ_RETURN_ON_ERROR(
          PeerDelegatedCredentialImpl<openssl::KeyType::P256>::create(
              credP256,
              err,
              std::move(cert),
              std::move(pubKey),
              std::move(credential)));
      ret = std::move(credP256);
      return Status::Success;
    }
    case openssl::KeyType::P384: {
      std::unique_ptr<PeerDelegatedCredentialImpl<openssl::KeyType::P384>>
          credP384;
      FIZZ_RETURN_ON_ERROR(
          PeerDelegatedCredentialImpl<openssl::KeyType::P384>::create(
              credP384,
              err,
              std::move(cert),
              std::move(pubKey),
              std::move(credential)));
      ret = std::move(credP384);
      return Status::Success;
    }
    case openssl::KeyType::P521: {
      std::unique_ptr<PeerDelegatedCredentialImpl<openssl::KeyType::P521>>
          credP521;
      FIZZ_RETURN_ON_ERROR(
          PeerDelegatedCredentialImpl<openssl::KeyType::P521>::create(
              credP521,
              err,
              std::move(cert),
              std::move(pubKey),
              std::move(credential)));
      ret = std::move(credP521);
      return Status::Success;
    }
    case openssl::KeyType::ED25519: {
      std::unique_ptr<PeerDelegatedCredentialImpl<openssl::KeyType::ED25519>>
          credED25519;
      FIZZ_RETURN_ON_ERROR(
          PeerDelegatedCredentialImpl<openssl::KeyType::ED25519>::create(
              credED25519,
              err,
              std::move(cert),
              std::move(pubKey),
              std::move(credential)));
      ret = std::move(credED25519);
      return Status::Success;
    }
  }

  return err.error(
      "unknown cert type for delegated credential",
      AlertDescription::illegal_parameter);
}
} // namespace

Status DelegatedCredentialFactory::makePeerCertStatic(
    std::unique_ptr<PeerCert>& ret,
    Error& err,
    CertificateEntry entry,
    bool leaf) {
  if (!leaf || entry.extensions.empty()) {
    ret = openssl::CertUtils::makePeerCert(std::move(entry.cert_data));
    return Status::Success;
  }
  auto parentCert = openssl::CertUtils::makePeerCert(entry.cert_data->clone());
  auto parentX509 = parentCert->getX509();
  folly::Optional<DelegatedCredential> credential;
  FIZZ_RETURN_ON_ERROR(
      getExtension<DelegatedCredential>(credential, err, entry.extensions));

  // No credential, just leave as is
  if (!credential) {
    ret = std::move(parentCert);
    return Status::Success;
  }

  // Create credential
  FIZZ_RETURN_ON_ERROR(makeCredential(
      ret, err, std::move(credential.value()), std::move(parentX509)));
  return Status::Success;
}

Status DelegatedCredentialFactory::makePeerCert(
    std::unique_ptr<PeerCert>& ret,
    Error& err,
    CertificateEntry entry,
    bool leaf) const {
  return makePeerCertStatic(ret, err, std::move(entry), leaf);
}
} // namespace extensions
} // namespace fizz
