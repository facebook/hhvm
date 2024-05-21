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
#include <folly/portability/OpenSSL.h>
#include <openssl/evp.h>
#include <stdexcept>

namespace fizz {
namespace openssl {

template <>
struct Properties<fizz::AESOCB128> {
  static const EVP_CIPHER* Cipher() {
#if !defined(OPENSSL_NO_OCB)
    return EVP_aes_128_ocb();
#else
    throw std::runtime_error(
        "aes-ocb support requires OpenSSL 1.1.0 with ocb enabled");
#endif
  }

  static const bool kOperatesInBlocks{true};
  static const bool kRequiresPresetTagLen{true};
};

} // namespace openssl
} // namespace fizz
