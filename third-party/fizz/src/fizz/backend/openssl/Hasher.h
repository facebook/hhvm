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
Status makeHasher(std::unique_ptr<::fizz::Hasher>& ret, Error& /*err*/) {
  ret = std::make_unique<::fizz::openssl::Sha>(Properties<T>::HashEngine());
  return Status::Success;
}

template <class T>
inline constexpr auto hasherImpl =
    HasherFactoryWithMetadata::bind<T>(makeHasher<T>);

template <class T>
const HasherFactoryWithMetadata* hasherFactory() {
  return &hasherImpl<T>;
}

} // namespace fizz::openssl
