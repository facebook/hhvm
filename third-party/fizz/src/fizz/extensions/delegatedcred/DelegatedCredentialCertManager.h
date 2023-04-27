/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once
#include <fizz/extensions/delegatedcred/SelfDelegatedCredential.h>
#include <fizz/extensions/delegatedcred/Types.h>
#include <fizz/server/CertManager.h>

namespace fizz {
namespace extensions {

// CertManager implementation that maintains two sets of certs, one with
// delegated credential extensions and one without. Certs are selected based
// on the usual rules with the addition of the split between delegated and
// non-delegated certs.
class DelegatedCredentialCertManager : public server::CertManager {
 public:
  CertMatch getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const std::vector<Extension>& peerExtensions) const override;

  std::shared_ptr<SelfCert> getCert(const std::string& identity) const override;

  void addCert(std::shared_ptr<SelfCert> cert, bool defaultCert = false)
      override;

  void addDelegatedCredential(std::shared_ptr<SelfDelegatedCredential> cred);

 protected:
  server::CertManager mainMgr_;
  server::CertManager dcMgr_;
};
} // namespace extensions
} // namespace fizz
