/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/Sha.h>
#include <fizz/backend/openssl/crypto/Sha256.h>
#include <fizz/backend/openssl/crypto/Sha384.h>
#include <fizz/backend/openssl/crypto/Sha512.h>
#include <folly/Range.h>

namespace fizz::openssl {
template <typename T>
struct HasherType {
  static constexpr size_t HashLen = T::HashLen;
  static constexpr size_t BlockSize = T::BlockSize;
  static constexpr folly::StringPiece BlankHash = T::BlankHash;
  static constexpr auto HashEngine = Properties<T>::HashEngine;
};

template <class T>
using Hasher = Sha<HasherType<T>>;

} // namespace fizz::openssl
