/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include <fizz/protocol/CertMatch.h>

namespace fizz {
namespace client {

class CertManager {
 public:
  virtual ~CertManager() = default;
  /**
   * Select a cert given the servers sig schemes, our own supported sig schemes
   * and peer extensions. The sni value is ignored
   */
  virtual CertMatch getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const std::vector<Extension>& peerExtensions) const;

  /*
   * This will add the cert to the internal map, and will not override existing
   * entry.
   */
  void addCert(std::shared_ptr<SelfCert> cert);

  /*
   * This will add the cert to the internal map, and override any existing
   * entry.
   */
  void addCertAndOverride(std::shared_ptr<SelfCert> cert);

  bool hasCerts() const;

 protected:
  void addCert(std::shared_ptr<SelfCert> cert, bool overrideExistingEntry);

  std::map<SignatureScheme, std::shared_ptr<SelfCert>> certs_;
};
} // namespace client
} // namespace fizz
