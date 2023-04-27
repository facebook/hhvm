/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Sha.h>
#include <openssl/evp.h>
#include <array>

namespace fizz {

class Sha256 : public Sha<Sha256> {
 public:
  static constexpr size_t HashLen = 32;

  static constexpr auto HashEngine = EVP_sha256;

  static constexpr folly::StringPiece BlankHash{
      "\xe3\xb0\xc4\x42\x98\xfc\x1c\x14\x9a\xfb\xf4\xc8\x99\x6f\xb9\x24\x27\xae\x41\xe4\x64\x9b\x93\x4c\xa4\x95\x99\x1b\x78\x52\xb8\x55"};
};
} // namespace fizz
