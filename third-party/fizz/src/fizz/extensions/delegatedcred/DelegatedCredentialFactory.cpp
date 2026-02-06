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
std::unique_ptr<PeerCert> makeCredential(
    DelegatedCredential&& credential,
    folly::ssl::X509UniquePtr cert) {
  FIZZ_VLOG(4) << "Making delegated credential";
  // Parse pubkey
  auto pubKeyRange = credential.public_key->coalesce();
  auto addr = pubKeyRange.data();
  folly::ssl::EvpPkeyUniquePtr pubKey(
      d2i_PUBKEY(nullptr, &addr, pubKeyRange.size()));
  if (!pubKey) {
    throw FizzException(
        "failed to create credential pubkey",
        AlertDescription::illegal_parameter);
  }

  switch (openssl::CertUtils::getKeyType(pubKey)) {
    case openssl::KeyType::RSA:
      return std::make_unique<
          PeerDelegatedCredentialImpl<openssl::KeyType::RSA>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case openssl::KeyType::P256:
      return std::make_unique<
          PeerDelegatedCredentialImpl<openssl::KeyType::P256>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case openssl::KeyType::P384:
      return std::make_unique<
          PeerDelegatedCredentialImpl<openssl::KeyType::P384>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case openssl::KeyType::P521:
      return std::make_unique<
          PeerDelegatedCredentialImpl<openssl::KeyType::P521>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case openssl::KeyType::ED25519:
      return std::make_unique<
          PeerDelegatedCredentialImpl<openssl::KeyType::ED25519>>(
          std::move(cert), std::move(pubKey), std::move(credential));
  }

  throw FizzException(
      "unknown cert type for delegated credential",
      AlertDescription::illegal_parameter);
}
} // namespace

std::unique_ptr<PeerCert> DelegatedCredentialFactory::makePeerCertStatic(
    CertificateEntry entry,
    bool leaf) {
  if (!leaf || entry.extensions.empty()) {
    return openssl::CertUtils::makePeerCert(std::move(entry.cert_data));
  }
  auto parentCert = openssl::CertUtils::makePeerCert(entry.cert_data->clone());
  auto parentX509 = parentCert->getX509();
  auto credential = getExtension<DelegatedCredential>(entry.extensions);

  // No credential, just leave as is
  if (!credential) {
    return parentCert;
  }

  // Create credential
  return makeCredential(std::move(credential.value()), std::move(parentX509));
}

std::unique_ptr<PeerCert> DelegatedCredentialFactory::makePeerCert(
    CertificateEntry entry,
    bool leaf) const {
  return makePeerCertStatic(std::move(entry), leaf);
}
} // namespace extensions
} // namespace fizz
