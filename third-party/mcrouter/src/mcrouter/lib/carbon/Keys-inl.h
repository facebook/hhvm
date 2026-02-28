/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <iostream>
#include <utility>

namespace carbon {

template <class Storage>
Keys<Storage>::Keys(const Keys<Storage>& other) : key_(other.key_) {
  initStringPieces(other);
}

template <class Storage>
Keys<Storage>& Keys<Storage>::operator=(const Keys<Storage>& other) {
  if (this != &other) {
    key_ = other.key_;
    initStringPieces(other);
  }
  return *this;
}

template <class Storage>
Keys<Storage>::Keys(Keys<Storage>&& other) noexcept
    : key_(std::move(other.key_)) {
  initStringPieces(other);
}

template <class Storage>
Keys<Storage>& Keys<Storage>::operator=(Keys<Storage>&& other) noexcept {
  if (this != &other) {
    key_ = std::move(other.key_);
    initStringPieces(other);
  }
  return *this;
}

template <class Storage>
void Keys<Storage>::update() {
  if constexpr (std::is_same_v<Storage, folly::IOBuf>) {
    key_.coalesce();
  }

  const folly::StringPiece key = fullKey();
  keyWithoutRoute_ = key;
  routingPrefix_.reset(key.begin(), 0);
  if (!key.empty() && *key.begin() == '/') {
    size_t pos = 1;
    for (int i = 0; i < 2; ++i) {
      pos = key.find('/', pos);
      if (pos == std::string::npos) {
        break;
      }
      ++pos;
    }
    if (pos != std::string::npos) {
      keyWithoutRoute_.advance(pos);
      routingPrefix_.reset(key.begin(), pos);
    }
  }
  routingKey_ = keyWithoutRoute_;
  // Micro-optimization: We use find_first_of as a first pass while looking for
  // the hash stop because searching for a single character is faster than
  // looking for a substring. Most keys in practice don't have a hash stop or a
  // '|' character.
  size_t pos = keyWithoutRoute_.find_first_of('|');
  if (pos != std::string::npos) {
    pos = keyWithoutRoute_.find("|#|", pos);
    if (pos != std::string::npos) {
      routingKey_ = keyWithoutRoute_.subpiece(0, pos);
      afterRoutingKey_ = keyWithoutRoute_.subpiece(pos);
    }
  }
  routingKeyHash_ = 0;
}

} // namespace carbon
