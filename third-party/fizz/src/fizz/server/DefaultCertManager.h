/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <unordered_map>

#include "fizz/server/CertManager.h"

#include <fizz/protocol/CertMatch.h>

namespace fizz {
namespace server {

class DefaultCertManager : public CertManager {
 public:
  virtual CertMatch getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const ClientHello& chlo) const override;

  virtual std::shared_ptr<SelfCert> getCert(
      const std::string& identity) const override;

  void addCertAndSetDefault(std::shared_ptr<SelfCert> cert);

  void addCert(std::shared_ptr<SelfCert> cert);

  bool hasCerts() const;

  using SigSchemeMap = std::map<SignatureScheme, std::shared_ptr<SelfCert>>;
  const std::unordered_map<std::string, SigSchemeMap>&
  getCertificatesByIdentity() const;

 protected:
  CertMatch findCert(
      const std::string& key,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes) const;

  void addCertIdentity(
      std::shared_ptr<SelfCert> cert,
      const std::string& ident);

  void addCert(std::shared_ptr<SelfCert> cert, bool defaultCert);

  static std::string getKeyFromIdent(const std::string& ident);

  std::unordered_map<std::string, SigSchemeMap> certs_;
  std::unordered_map<std::string, std::shared_ptr<SelfCert>> identMap_;
  std::string default_;
};
} // namespace server
} // namespace fizz
