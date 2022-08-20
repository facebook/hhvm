/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <folly/Range.h>

namespace facebook {
namespace memcache {

template <class F>
typename std::result_of<F(folly::StringPiece)>::type hashWithSalt(
    const folly::StringPiece key,
    const folly::StringPiece salt,
    F&& hashFunc) {
  constexpr size_t kMaxKeySaltSize = 512;
  auto keySaltSize = key.size() + salt.size();
  auto copyAndHash = [&](char* buff) {
    memcpy(buff, key.data(), key.size());
    memcpy(buff + key.size(), salt.data(), salt.size());
    return hashFunc({buff, buff + keySaltSize});
  };
  if (keySaltSize <= kMaxKeySaltSize) {
    char c[kMaxKeySaltSize];
    return copyAndHash(c);
  } else {
    std::vector<char> vc(keySaltSize);
    return copyAndHash(vc.data());
  }
}

} // namespace memcache
} // namespace facebook
