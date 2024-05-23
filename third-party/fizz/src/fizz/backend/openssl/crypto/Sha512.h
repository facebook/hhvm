/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/Properties.h>
#include <fizz/backend/openssl/crypto/Sha.h>
#include <fizz/crypto/Crypto.h>
#include <openssl/evp.h>

namespace fizz {
namespace openssl {

template <>
struct Properties<fizz::Sha512> {
  static constexpr auto HashEngine = EVP_sha512;
};

} // namespace openssl
} // namespace fizz
