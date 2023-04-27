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

static const auto kMaxDelegatedCredentialLifetime = std::chrono::hours(24 * 7);

std::shared_ptr<PeerCert> DelegatedCredentialFactory::makePeerCert(
    CertificateEntry entry,
    bool leaf) const {
  // Only leaf cert needs different processing, and only if it has the extension
  if (!leaf || entry.extensions.empty()) {
    return CertUtils::makePeerCert(std::move(entry.cert_data));
  }

  auto parentCert = CertUtils::makePeerCert(entry.cert_data->clone());
  auto parentX509 = parentCert->getX509();
  auto credential = getExtension<DelegatedCredential>(entry.extensions);

  // No credential, just leave as is
  if (!credential) {
    return std::move(parentCert);
  }

  // Check validity period first.
  auto notBefore = X509_get0_notBefore(parentX509.get());
  auto notBeforeTime =
      folly::ssl::OpenSSLCertUtils::asnTimeToTimepoint(notBefore);
  auto credentialExpiresTime =
      notBeforeTime + std::chrono::seconds(credential->valid_time);
  auto now = clock_->getCurrentTime();
  if (now >= credentialExpiresTime) {
    throw FizzException(
        "credential is no longer valid", AlertDescription::illegal_parameter);
  }

  // Credentials may be valid for max 1 week according to spec
  if (credentialExpiresTime - now > kMaxDelegatedCredentialLifetime) {
    throw FizzException(
        "credential validity is longer than a week from now",
        AlertDescription::illegal_parameter);
  }

  // Check extensions on cert
  DelegatedCredentialUtils::checkExtensions(parentX509);

  // Create credential
  return makeCredential(std::move(credential.value()), std::move(parentX509));
}

std::shared_ptr<PeerCert> DelegatedCredentialFactory::makeCredential(
    DelegatedCredential&& credential,
    folly::ssl::X509UniquePtr cert) const {
  VLOG(4) << "Making delegated credential";
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

  switch (CertUtils::getKeyType(pubKey)) {
    case KeyType::RSA:
      return std::make_shared<PeerDelegatedCredential<KeyType::RSA>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case KeyType::P256:
      return std::make_shared<PeerDelegatedCredential<KeyType::P256>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case KeyType::P384:
      return std::make_shared<PeerDelegatedCredential<KeyType::P384>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case KeyType::P521:
      return std::make_shared<PeerDelegatedCredential<KeyType::P521>>(
          std::move(cert), std::move(pubKey), std::move(credential));
    case KeyType::ED25519:
      return std::make_shared<PeerDelegatedCredential<KeyType::ED25519>>(
          std::move(cert), std::move(pubKey), std::move(credential));
  }

  throw FizzException(
      "unknown cert type for delegated credential",
      AlertDescription::illegal_parameter);
}

void DelegatedCredentialFactory::setClock(std::shared_ptr<Clock> clock) {
  clock_ = clock;
}

} // namespace extensions
} // namespace fizz
