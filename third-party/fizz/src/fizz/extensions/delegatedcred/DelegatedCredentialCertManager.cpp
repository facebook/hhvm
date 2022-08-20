/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/DelegatedCredentialCertManager.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>

using namespace fizz::server;

namespace fizz {
namespace extensions {

CertManager::CertMatch DelegatedCredentialCertManager::getCert(
    const folly::Optional<std::string>& sni,
    const std::vector<SignatureScheme>& supportedSigSchemes,
    const std::vector<SignatureScheme>& peerSigSchemes,
    const std::vector<Extension>& peerExtensions) const {
  auto credential = getExtension<DelegatedCredentialSupport>(peerExtensions);

  if (credential) {
    auto dcRes = dcMgr_.getCert(
        sni,
        supportedSigSchemes,
        credential->supported_signature_algorithms,
        peerExtensions);
    if (dcRes && dcRes->type == MatchType::Direct) {
      return dcRes;
    }
  }
  return mainMgr_.getCert(
      sni, supportedSigSchemes, peerSigSchemes, peerExtensions);
}

// Falls back to non-delegated if no match.
std::shared_ptr<SelfCert> DelegatedCredentialCertManager::getCert(
    const std::string& identity) const {
  auto dcRes = dcMgr_.getCert(identity);
  return dcRes ? dcRes : mainMgr_.getCert(identity);
}

void DelegatedCredentialCertManager::addCert(
    std::shared_ptr<SelfCert> cert,
    bool defaultCert) {
  VLOG(8) << "Adding undelegated cert";
  mainMgr_.addCert(std::move(cert), defaultCert);
}

void DelegatedCredentialCertManager::addDelegatedCredential(
    std::shared_ptr<SelfDelegatedCredential> cred) {
  VLOG(8) << "Adding delegated credential";
  dcMgr_.addCert(std::move(cred), false);
}

} // namespace extensions
} // namespace fizz
