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

#include <fizz/protocol/CertManagerBase.h>

namespace fizz {
namespace server {

class CertManager : public CertManagerBase {
 public:
  virtual ~CertManager() = default;
  /**
   * Select a cert given a client supplied SNI value, server
   * supportedSigSchemes, client peerSigSchemes, and ClientHello
   *
   * Will ignore peerSigSchemes if no matching certificate is found.
   */
  virtual CertMatch getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const ClientHello& chlo) const;

  /**
   * Return a certificate with the a primary identity exactly matching identity.
   * Will return nullptr if no matching cert is found.
   */
  virtual std::shared_ptr<SelfCert> getCert(const std::string& identity) const;

  void addCertAndSetDefault(std::shared_ptr<SelfCert> cert);

  void addCert(std::shared_ptr<SelfCert> cert);

  bool hasCerts() const;

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
