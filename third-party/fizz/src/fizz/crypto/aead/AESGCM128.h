/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <openssl/evp.h>

namespace fizz {

struct AESGCM128 {
  static constexpr auto Cipher = EVP_aes_128_gcm;

  static const size_t kKeyLength{16};
  static const size_t kIVLength{12};
  static const size_t kTagLength{16};
  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

} // namespace fizz
