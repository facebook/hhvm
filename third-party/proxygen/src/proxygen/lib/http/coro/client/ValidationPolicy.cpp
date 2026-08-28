/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/ValidationPolicy.h"

#include <stdexcept>

namespace proxygen::coro {

std::optional<ValidationPolicy> getValidationPolicy(
    HTTPCoroConnector::IdentityValidation identityValidation) {
  switch (identityValidation) {
    case HTTPCoroConnector::IdentityValidation::Disabled_INSECURE:
      return std::nullopt;
    case HTTPCoroConnector::IdentityValidation::Enforcing:
      return ValidationPolicy::Enforcing;
    case HTTPCoroConnector::IdentityValidation::Logging:
      return ValidationPolicy::Logging;
  }
  throw std::invalid_argument("unknown identity validation mode");
}

} // namespace proxygen::coro
