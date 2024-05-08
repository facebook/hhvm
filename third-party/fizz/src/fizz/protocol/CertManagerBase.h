/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include <fizz/protocol/Certificate.h>

namespace fizz {
class CertManagerBase {
 public:
  enum class MatchType { Direct, Default };
  struct CertMatchStruct {
    std::shared_ptr<SelfCert> cert;
    SignatureScheme scheme;
    MatchType type;
  };
  using CertMatch = folly::Optional<CertMatchStruct>;

  virtual ~CertManagerBase() = default;

  /**
   * Select a cert given a client supplied SNI value, server
   * supportedSigSchemes, client peerSigSchemes, and client peerExtensions
   *
   * Will ignore peerSigSchemes if no matching certificate is found.
   */
  virtual CertMatch getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& supportedSigSchemes,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const std::vector<Extension>& peerExtensions) const = 0;

 protected:
  using SigSchemeMap = std::map<SignatureScheme, std::shared_ptr<SelfCert>>;
};
} // namespace fizz
