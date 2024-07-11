/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Hasher.h>

namespace fizz {

constexpr uint8_t HMAC_OPAD = 0x5c;
constexpr uint8_t HMAC_IPAD = 0x36;

/**
 * Puts `HMAC(key, in)` into `out`
 * `out` must be at least of size HashLen.
 */
void hmac(
    HasherFactory makeHasher,
    folly::ByteRange key,
    const folly::IOBuf& in,
    folly::MutableByteRange out);
} // namespace fizz
