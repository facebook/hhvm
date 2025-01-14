/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include "fizz/client/FizzClientContext.h"

#include <fizz/protocol/DefaultFactory.h>

namespace fizz {
namespace client {

FizzClientContext::FizzClientContext()
    : factory_(std::make_shared<DefaultFactory>()),
      clock_(std::make_shared<SystemClock>()) {}

void FizzClientContext::validate() const {
  // TODO: check supported sig schemes
  for (auto& c : supportedCiphers_) {
    if (!FIZZ_CONTEXT_VALIDATION_SHOULD_CHECK_CIPHER(c)) {
      continue;
    }
    // will throw if factory doesn't support this cipher
    factory_->makeAead(c);
  }

  for (auto& g : supportedGroups_) {
    // will throw if factory doesn't support this named group
    factory_->makeKeyExchange(g, KeyExchangeRole::Client);
  }

  for (auto& share : defaultShares_) {
    if (std::find(supportedGroups_.begin(), supportedGroups_.end(), share) ==
        supportedGroups_.end()) {
      throw std::runtime_error("unsupported named group in default shares");
    }
  }
}

} // namespace client
} // namespace fizz
