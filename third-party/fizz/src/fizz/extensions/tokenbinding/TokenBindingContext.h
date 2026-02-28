/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/tokenbinding/Types.h>

namespace fizz {
namespace extensions {

class TokenBindingContext {
 public:
  ~TokenBindingContext() = default;

  /**
   * Set the supported Token Binding protocol versions. The input should be in
   * descending order.
   */
  void setSupportedVersions(std::vector<TokenBindingProtocolVersion> versions) {
    if (!std::is_sorted(
            versions.begin(),
            versions.end(),
            std::greater<TokenBindingProtocolVersion>())) {
      throw std::runtime_error("Protocol Versions are not in descending order");
    }
    // check if these are unique
    auto it = std::unique(versions.begin(), versions.end());
    if (it != versions.end()) {
      throw std::runtime_error("Protocol Versions contain duplicates");
    }
    supportedVersions_ = std::move(versions);
  }
  const auto& getSupportedVersions() const {
    return supportedVersions_;
  }

  /**
   * Set the supported key parameters in preference order.
   */
  void setSupportedKeyParameters(
      std::vector<TokenBindingKeyParameters> keyParams) {
    supportedKeyParams_ = std::move(keyParams);
  }
  const auto& getKeyParams() const {
    return supportedKeyParams_;
  }

 private:
  std::vector<TokenBindingProtocolVersion> supportedVersions_ = {
      TokenBindingProtocolVersion::token_binding_1_0};

  std::vector<TokenBindingKeyParameters> supportedKeyParams_ = {
      TokenBindingKeyParameters::ecdsap256};
};
} // namespace extensions
} // namespace fizz
