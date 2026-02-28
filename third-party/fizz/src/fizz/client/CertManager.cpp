/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/CertManager.h>
#include <fizz/util/Logging.h>

using namespace folly;

namespace fizz {
namespace client {

CertMatch CertManager::getCert(
    const folly::Optional<std::string>& /* sni */,
    const std::vector<SignatureScheme>& supportedSigSchemes,
    const std::vector<SignatureScheme>& peerSigSchemes,
    const std::vector<Extension>& /* peerExtensions */) const {
  for (auto scheme : supportedSigSchemes) {
    auto cert = certs_.find(scheme);
    if (cert == certs_.end()) {
      continue;
    }
    if (std::find(peerSigSchemes.begin(), peerSigSchemes.end(), scheme) !=
        peerSigSchemes.end()) {
      return CertMatchStruct{cert->second, scheme, MatchType::Default};
    }
  }
  return none;
}

void CertManager::addCert(std::shared_ptr<SelfCert> cert) {
  addCert(cert, false);
}

void CertManager::addCertAndOverride(std::shared_ptr<SelfCert> cert) {
  addCert(cert, true);
}

void CertManager::addCert(
    std::shared_ptr<SelfCert> cert,
    bool overrideExistingEntry) {
  if (cert == nullptr) {
    return;
  }
  auto sigSchemes = cert->getSigSchemes();
  for (auto sigScheme : sigSchemes) {
    if (certs_.find(sigScheme) == certs_.end() || overrideExistingEntry) {
      certs_[sigScheme] = cert;
    } else {
      FIZZ_VLOG(8) << "Skipping duplicate certificate for signature scheme"
                   << toString(sigScheme);
    }
  }
}

bool CertManager::hasCerts() const {
  return !certs_.empty();
}

} // namespace client
} // namespace fizz
