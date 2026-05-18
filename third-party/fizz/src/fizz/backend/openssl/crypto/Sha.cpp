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
Status Sha::hash_update(Error& /*err*/, folly::ByteRange data) {
  digest_.hash_update(data);
  return Status::Success;
}
Status Sha::hash_final(Error& /*err*/, folly::MutableByteRange out) {
  digest_.hash_final(out);
  return Status::Success;
}
Status Sha::clone(std::unique_ptr<fizz::Hasher>& ret, Error& /*err*/) const {
  ret = std::make_unique<Sha>(*this);
  return Status::Success;
}

size_t Sha::getHashLen() const {
  return digest_.hash_size();
}

size_t Sha::getBlockSize() const {
  return digest_.block_size();
}

} // namespace openssl
} // namespace fizz
