/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/tokenbinding/TokenBindingContext.h>
#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/server/Negotiator.h>
#include <fizz/server/ServerExtensions.h>

namespace fizz {
namespace extensions {

class TokenBindingServerExtension : public ServerExtensions {
 public:
  explicit TokenBindingServerExtension(
      const std::shared_ptr<TokenBindingContext>& tokenBindingContext)
      : tokenBindingContext_(tokenBindingContext) {}

  std::vector<Extension> getExtensions(const ClientHello& chlo) override {
    std::vector<Extension> serverExtensions;
    auto params = getExtension<TokenBindingParameters>(chlo.extensions);
    if (params) {
      auto negotiatedVersion = negotiateVersion(params->version);
      auto negotiatedKeyParam = server::negotiate(
          tokenBindingContext_->getKeyParams(), params->key_parameters_list);
      if (negotiatedKeyParam && negotiatedVersion) {
        TokenBindingParameters negotiatedParams;
        negotiatedParams.version = *negotiatedVersion;
        negotiatedParams.key_parameters_list.push_back(*negotiatedKeyParam);
        serverExtensions.push_back(encodeExtension(negotiatedParams));
        negotiatedKeyParam_ = std::move(negotiatedKeyParam);
      }
    }
    return serverExtensions;
  }
  const auto& getNegotiatedKeyParam() {
    return negotiatedKeyParam_;
  }

 private:
  std::shared_ptr<TokenBindingContext> tokenBindingContext_;
  folly::Optional<TokenBindingKeyParameters> negotiatedKeyParam_;

  /**
   * For TokenBinding, the server can negotiate any version below the clients
   * preferred version, inclusive.
   */
  folly::Optional<TokenBindingProtocolVersion> negotiateVersion(
      const TokenBindingProtocolVersion& clientPref) {
    for (const auto& pref : tokenBindingContext_->getSupportedVersions()) {
      if (pref <= clientPref) {
        return pref;
      }
    }
    return folly::none;
  }
};
} // namespace extensions
} // namespace fizz
