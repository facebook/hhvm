/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/CertManager.h>

#include <folly/String.h>

using namespace folly;

namespace fizz {
namespace server {

// Find a matching cert given a key.
CertManager::CertMatch CertManager::findCert(
    const std::string& key,
    const std::vector<SignatureScheme>& supportedSigSchemes,
    const std::vector<SignatureScheme>& peerSigSchemes) const {
  auto it = certs_.find(key);
  if (it == certs_.end()) {
    return none;
  }
  for (auto scheme : supportedSigSchemes) {
    auto cert = it->second.find(scheme);
    if (cert == it->second.end()) {
      continue;
    }
    if (std::find(peerSigSchemes.begin(), peerSigSchemes.end(), scheme) !=
        peerSigSchemes.end()) {
      return CertMatchStruct{cert->second, scheme, MatchType::Direct};
    }
  }
  return none;
}

CertManager::CertMatch CertManager::getCert(
    const Optional<std::string>& sni,
    const std::vector<SignatureScheme>& supportedSigSchemes,
    const std::vector<SignatureScheme>& peerSigSchemes,
    const std::vector<Extension>& /*peerExtensions*/) const {
  if (sni) {
    auto key = *sni;
    toLowerAscii(key);

    auto ret = findCert(key, supportedSigSchemes, peerSigSchemes);
    if (ret) {
      VLOG(8) << "Found exact SNI match for: " << key;
      return ret;
    }

    auto dot = key.find_first_of('.');
    if (dot != std::string::npos) {
      std::string wildcardKey(key, dot);
      ret = findCert(wildcardKey, supportedSigSchemes, peerSigSchemes);
      if (ret) {
        VLOG(8) << "Found wildcard SNI match for: " << key;
        return ret;
      }
    }

    VLOG(8) << "Did not find match for SNI: " << key;
  }

  auto ret = findCert(default_, supportedSigSchemes, peerSigSchemes);
  if (ret) {
    ret->type = MatchType::Default;
    return ret;
  }

  VLOG(8) << "No matching cert for client sig schemes found";
  return folly::none;
}

std::shared_ptr<SelfCert> CertManager::getCert(
    const std::string& identity) const {
  auto it = identMap_.find(identity);
  if (it == identMap_.end()) {
    return nullptr;
  }
  return it->second;
}

std::string CertManager::getKeyFromIdent(const std::string& ident) {
  if (ident.empty()) {
    throw std::runtime_error("empty identity");
  }

  std::string key;
  if (ident.front() == '*') {
    key = std::string(ident, 1);
  } else {
    key = ident;
  }
  toLowerAscii(key);

  return key;
}

void CertManager::addCertIdentity(
    std::shared_ptr<SelfCert> cert,
    const std::string& ident) {
  auto key = getKeyFromIdent(ident);

  if (key.empty() || key == "." || key.find('*') != std::string::npos) {
    throw std::runtime_error(to<std::string>("invalid identity: ", ident));
  }

  auto sigSchemes = cert->getSigSchemes();
  auto& schemeMap = certs_[key];
  for (auto sigScheme : sigSchemes) {
    if (schemeMap.find(sigScheme) != schemeMap.end()) {
      VLOG(8) << "Skipping duplicate certificate for " << key;
    } else {
      schemeMap[sigScheme] = cert;
    }
  }
}

void CertManager::addCert(std::shared_ptr<SelfCert> cert, bool defaultCert) {
  auto primaryIdent = cert->getIdentity();
  addCertIdentity(cert, primaryIdent);

  auto altIdents = cert->getAltIdentities();
  for (const auto& ident : altIdents) {
    if (ident != primaryIdent) {
      addCertIdentity(cert, ident);
    }
  }

  if (defaultCert) {
    default_ = getKeyFromIdent(primaryIdent);
  }

  if (identMap_.find(primaryIdent) == identMap_.end()) {
    identMap_[primaryIdent] = cert;
  }
}
} // namespace server
} // namespace fizz
