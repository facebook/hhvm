/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace fizz {

template <typename T>
void Sha<T>::hash_init() {
  digest_.hash_init(T::HashEngine());
}

template <typename T>
void Sha<T>::hash_update(folly::ByteRange data) {
  digest_.hash_update(data);
}

template <typename T>
void Sha<T>::hash_update(const folly::IOBuf& data) {
  digest_.hash_update(data);
}

template <typename T>
void Sha<T>::hash_final(folly::MutableByteRange out) {
  digest_.hash_final(out);
}

template <typename T>
void Sha<T>::hmac(
    folly::ByteRange key,
    const folly::IOBuf& in,
    folly::MutableByteRange out) {
  CHECK_GE(out.size(), T::HashLen);
  folly::ssl::OpenSSLHash::hmac(out, T::HashEngine(), key, in);
}

template <typename T>
void Sha<T>::hash(const folly::IOBuf& in, folly::MutableByteRange out) {
  CHECK_GE(out.size(), T::HashLen);
  folly::ssl::OpenSSLHash::hash(out, T::HashEngine(), in);
}
} // namespace fizz
