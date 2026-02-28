/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once
#include <fizz/client/CertManager.h>

namespace fizz {
namespace extensions {

// CertManager implementation that maintains two sets of certs, one with
// delegated credential extensions and one without. Certs are selected based
// on the usual rules with the addition of the split between delegated and
// non-delegated certs.
class DelegatedCredentialClientCertManager : public fizz::client::CertManager {
 public:
  /**
   * Select a cert given the servers sig schemes, our own supported sig schemes
   * and peer extensions. The sni value is ignored. Note this implementation
   * makes use of the peer extensions to pick a delegated credential or a x509
   * cert
   */
  CertMatch getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const std::vector<Extension>& peerExtensions) const override;

  /*
   * It is the callers responsibility to call this with an actual delegated
   * cred. If there is already a dc for the signature scheme we will not
   * override the existing entry.
   */
  void addDelegatedCredential(std::shared_ptr<SelfCert> cert);

  /*
   * It is the callers responsibility to call this with an actual delegated
   * cred. If there is already a dc for the signature scheme we will override
   * the existing entry.
   */
  void addDelegatedCredentialAndOverride(std::shared_ptr<SelfCert> cert);

  bool hasDelegatedCredential() const;

 protected:
  client::CertManager dcMgr_;
};
} // namespace extensions
} // namespace fizz
