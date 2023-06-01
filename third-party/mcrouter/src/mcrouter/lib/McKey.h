/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <ctype.h>

#include <folly/Range.h>
#include <folly/String.h>

#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/mc/protocol.h"

namespace facebook {
namespace memcache {

/**
 * Checks whether the given key is valid.
 * The key must satisfy:
 *   1) The length should be nonzero.
 *   2) The length should be at most MC_KEY_MAX_LEN.
 *   3) If a memcache key is being checked, as indicated by the template
 *      parameter, there should be no spaces or control characters.
 */
template <bool DoSpaceAndCtrlCheck>
mc_req_err_t isKeyValid(folly::StringPiece key) {
  if (key.empty()) {
    return mc_req_err_no_key;
  }

  if (key.size() > MC_KEY_MAX_LEN) {
    return mc_req_err_key_too_long;
  }

  if (DoSpaceAndCtrlCheck) {
    if (folly::hasSpaceOrCntrlSymbols(key)) {
      return mc_req_err_space_or_ctrl;
    }
  }

  return mc_req_err_valid;
}

namespace mcrouter {
namespace detail {
constexpr size_t numDigitsBase10(uint64_t n) {
  return n < 10 ? 1 : 1 + numDigitsBase10(n / 10);
}
} // namespace detail
} // namespace mcrouter

} // namespace memcache
} // namespace facebook
