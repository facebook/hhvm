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
#include <folly/Range.h>

namespace fizz::openssl {
template <class T>
std::unique_ptr<::fizz::Hasher> makeHasher() {
  return std::make_unique<::fizz::openssl::Sha>(Properties<T>::HashEngine());
}

} // namespace fizz::openssl
