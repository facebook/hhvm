/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/Properties.h>
#include <fizz/crypto/Crypto.h>
#include <openssl/evp.h>

namespace fizz {
namespace openssl {

template <>
struct Properties<fizz::P256> {
  static constexpr int curveNid{NID_X9_62_prime256v1};
};

template <>
struct Properties<fizz::P384> {
  static constexpr int curveNid{NID_secp384r1};
};

template <>
struct Properties<fizz::P521> {
  static constexpr int curveNid{NID_secp521r1};
};

} // namespace openssl
} // namespace fizz
