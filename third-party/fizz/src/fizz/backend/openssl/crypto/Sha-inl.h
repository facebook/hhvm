/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Hmac.h>

namespace fizz {
namespace openssl {

template <typename T>
void Sha<T>::hash_init() {
  digest_.hash_init(T::HashEngine());
}

template <typename T>
void Sha<T>::hash_update(folly::ByteRange data) {
  digest_.hash_update(data);
}

template <typename T>
void Sha<T>::hash_final(folly::MutableByteRange out) {
  digest_.hash_final(out);
}

template <typename T>
std::unique_ptr<fizz::Hasher> Sha<T>::clone() const {
  return std::make_unique<Sha<T>>(*this);
}

template <typename T>
size_t Sha<T>::getHashLen() const {
  return T::HashLen;
}

template <typename T>
inline size_t Sha<T>::getBlockSize() const {
  return T::BlockSize;
}

template <typename T>
void Sha<T>::hmac(
    folly::ByteRange key,
    const folly::IOBuf& in,
    folly::MutableByteRange out) {
  fizz::hmac(
      []() -> std::unique_ptr<fizz::Hasher> {
        return std::make_unique<Sha<T>>();
      },
      key,
      in,
      out);
}

template <typename T>
void Sha<T>::hash(const folly::IOBuf& in, folly::MutableByteRange out) {
  fizz::hash(
      []() -> std::unique_ptr<fizz::Hasher> {
        return std::make_unique<Sha<T>>();
      },
      in,
      out);
}
} // namespace openssl
} // namespace fizz
