/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertMatch.h>

namespace fizz {
namespace server {

class CertManager {
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
      const ClientHello& chlo) const = 0;

  /**
   * Return a certificate with the a primary identity exactly matching identity.
   * Will return nullptr if no matching cert is found.
   */
  virtual std::shared_ptr<SelfCert> getCert(
      const std::string& identity) const = 0;
};
} // namespace server
} // namespace fizz
