/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/evp.h>

#include <fizz/backend/openssl/Properties.h>
#include <fizz/crypto/Crypto.h>

namespace fizz {
namespace openssl {

template <>
struct Properties<fizz::AESGCM128> {
  static const EVP_CIPHER* Cipher() {
    return EVP_aes_128_gcm();
  }

  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

} // namespace openssl
} // namespace fizz
