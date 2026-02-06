/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/DelegatedCredentialCertManager.h>
#include <fizz/util/Logging.h>

using namespace fizz::server;

namespace fizz {
namespace extensions {

CertMatch DelegatedCredentialCertManager::getCert(
    const folly::Optional<std::string>& sni,
    const std::vector<SignatureScheme>& supportedSigSchemes,
    const std::vector<SignatureScheme>& peerSigSchemes,
    const ClientHello& chlo) const {
  auto credential = getExtension<DelegatedCredentialSupport>(chlo.extensions);

  if (credential) {
    auto dcRes = dcMgr_.getCert(
        sni,
        supportedSigSchemes,
        credential->supported_signature_algorithms,
        chlo);
    if (dcRes) {
      return dcRes;
    }
  }
  return DefaultCertManager::getCert(
      sni, supportedSigSchemes, peerSigSchemes, chlo);
}

// Falls back to non-delegated if no match.
std::shared_ptr<SelfCert> DelegatedCredentialCertManager::getCert(
    const std::string& identity) const {
  auto dcRes = dcMgr_.getCert(identity);
  return dcRes ? dcRes : DefaultCertManager::getCert(identity);
}

void DelegatedCredentialCertManager::addDelegatedCredentialAndSetDefault(
    std::shared_ptr<SelfDelegatedCredential> cred) {
  FIZZ_VLOG(8) << "Adding delegated credential";
  dcMgr_.addCertAndSetDefault(std::move(cred));
}

void DelegatedCredentialCertManager::addDelegatedCredential(
    std::shared_ptr<SelfDelegatedCredential> cred) {
  FIZZ_VLOG(8) << "Adding delegated credential";
  dcMgr_.addCert(std::move(cred));
}

bool DelegatedCredentialCertManager::hasDelegatedCredential() const {
  return dcMgr_.hasCerts();
}
} // namespace extensions
} // namespace fizz
