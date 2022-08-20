/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>

#include <folly/portability/OpenSSL.h>
#include <openssl/evp.h>

namespace fizz {

struct ChaCha20Poly1305 {
  static const EVP_CIPHER* Cipher() {
#if FOLLY_OPENSSL_HAS_CHACHA
    return EVP_chacha20_poly1305();
#else
    throw std::runtime_error(
        "chacha20-poly1305 support requires OpenSSL 1.1.0");
#endif // FOLLY_OPENSSL_HAS_CHACHA
  }

  static const size_t kKeyLength{32};
  static const size_t kIVLength{12};
  static const size_t kTagLength{16};
  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

} // namespace fizz
