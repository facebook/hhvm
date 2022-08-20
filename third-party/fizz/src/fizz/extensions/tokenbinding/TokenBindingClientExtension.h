/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/ClientExtensions.h>
#include <fizz/extensions/tokenbinding/TokenBindingContext.h>
#include <fizz/extensions/tokenbinding/Types.h>
#include <folly/Optional.h>

namespace fizz {
namespace extensions {

class TokenBindingClientExtension : public ClientExtensions {
 public:
  explicit TokenBindingClientExtension(
      const std::shared_ptr<TokenBindingContext>& context)
      : context_(context) {}

  std::vector<Extension> getClientHelloExtensions() const override;

  void onEncryptedExtensions(const std::vector<Extension>& extensions) override;

  const auto& getVersion() {
    return negotiatedVersion_;
  }

  const auto& getNegotiatedKeyParam() {
    return negotiatedKeyParam_;
  }

 private:
  folly::Optional<TokenBindingProtocolVersion> negotiatedVersion_;
  folly::Optional<TokenBindingKeyParameters> negotiatedKeyParam_;
  std::shared_ptr<TokenBindingContext> context_;
};
} // namespace extensions
} // namespace fizz
