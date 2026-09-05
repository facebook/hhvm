/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/crypto/Sha.h>

#include <stdexcept>

namespace fizz {
namespace openssl {

namespace {

void checkResult(int result) {
  if (result != 1) {
    folly::throw_exception<std::runtime_error>(
        "openssl crypto function failed");
  }
}

} // namespace

template <class T>
Sha<T>::Sha() {
  checkResult(detail::ShaTraits<T>::init(&ctx_));
}

template <class T>
Status Sha<T>::hash_update(Error& /*err*/, folly::ByteRange data) {
  checkResult(detail::ShaTraits<T>::update(&ctx_, data.data(), data.size()));
  return Status::Success;
}

template <class T>
Status Sha<T>::hash_final(Error& /*err*/, folly::MutableByteRange out) {
  if (out.size() != T::HashLen) {
    throw std::invalid_argument("output size does not match hash size");
  }
  checkResult(detail::ShaTraits<T>::finalize(out.data(), &ctx_));
  return Status::Success;
}

template <class T>
Status Sha<T>::clone(std::unique_ptr<fizz::Hasher>& ret, Error& /*err*/) const {
  ret = std::make_unique<Sha<T>>(*this);
  return Status::Success;
}

template <class T>
size_t Sha<T>::getHashLen() const {
  return T::HashLen;
}

template <class T>
size_t Sha<T>::getBlockSize() const {
  return T::BlockSize;
}

template class Sha<fizz::Sha256>;
template class Sha<fizz::Sha384>;
template class Sha<fizz::Sha512>;

} // namespace openssl
} // namespace fizz
