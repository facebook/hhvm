/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/OpenSSL.h>
#include <openssl/evp.h>
#include <stdexcept>

namespace fizz {

struct AESOCB128 {
  static const EVP_CIPHER* Cipher() {
#if FOLLY_OPENSSL_IS_110 && !defined(OPENSSL_NO_OCB)
    return EVP_aes_128_ocb();
#else
    throw std::runtime_error(
        "aes-ocb support requires OpenSSL 1.1.0 with ocb enabled");
#endif
  }

  static const size_t kKeyLength{16};
  static const size_t kIVLength{12};
  static const size_t kTagLength{16};
  static const bool kOperatesInBlocks{true};
  static const bool kRequiresPresetTagLen{true};
};

} // namespace fizz
