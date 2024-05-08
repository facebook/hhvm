/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include <fizz/protocol/CertManagerBase.h>

namespace fizz {
namespace client {

class CertManager : public CertManagerBase {
 public:
  /**
   * Select a cert given the servers sig schemes, our own supported sig schemes
   * and peer extensions. The sni value is ignored
   */
  CertMatch getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const std::vector<Extension>& peerExtensions) const override;

  /*
   * This will add the cert to the internal map, note we expect only a single
   * cert for a particular signature scheme, by default we will override any
   * existing entry. The caller may choose to not do so.
   */
  void addCert(
      std::shared_ptr<SelfCert> cert,
      bool overrideExistingEntry = true);

 protected:
  SigSchemeMap certs_;
};
} // namespace client
} // namespace fizz
