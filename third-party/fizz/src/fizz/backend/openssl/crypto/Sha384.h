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
#include <array>

namespace fizz {
namespace openssl {

class Sha384 : public Sha<Sha384> {
 public:
  static constexpr size_t HashLen = 48;

  static constexpr auto HashEngine = EVP_sha384;

  static constexpr folly::StringPiece BlankHash{
      "\x38\xb0\x60\xa7\x51\xac\x96\x38\x4c\xd9\x32\x7e\xb1\xb1\xe3\x6a\x21\xfd\xb7\x11\x14\xbe\x07\x43\x4c\x0c\xc7\xbf\x63\xf6\xe1\xda\x27\x4e\xde\xbf\xe7\x6f\x65\xfb\xd5\x1a\xd2\xf1\x48\x98\xb9\x5b"};
};

template <>
struct Properties<fizz::Sha384> {
  static constexpr auto HashEngine = EVP_sha384;
  // TODO: include Sha<Sha384> hasher as part of this struct.
};

} // namespace openssl
} // namespace fizz
