/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/crypto/Sha.h>

namespace fizz {
namespace openssl {

Sha::Sha(const EVP_MD* md) {
  digest_.hash_init(md);
}
void Sha::hash_update(folly::ByteRange data) {
  digest_.hash_update(data);
}
void Sha::hash_final(folly::MutableByteRange out) {
  digest_.hash_final(out);
}
std::unique_ptr<fizz::Hasher> Sha::clone() const {
  return std::make_unique<Sha>(*this);
}

size_t Sha::getHashLen() const {
  return digest_.hash_size();
}

size_t Sha::getBlockSize() const {
  return digest_.block_size();
}

} // namespace openssl
} // namespace fizz
