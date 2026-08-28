/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>

#include "proxygen/lib/http/coro/client/HTTPCoroConnector.h"
#include "proxygen/lib/http/coro/client/ProxygenCertVerifier.h"

namespace proxygen::coro {

std::optional<ValidationPolicy> getValidationPolicy(
    HTTPCoroConnector::IdentityValidation identityValidation);

} // namespace proxygen::coro
