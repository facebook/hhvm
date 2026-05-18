/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Crypto.h>
#include <folly/io/IOBuf.h>

namespace fizz {

/**
 * `Hasher` implements a message digest.
 *
 *  The message should be fed to the hasher with multiple calls to
 * `hash_update`.
 *
 *  When the entire message has been processed, `hash_final()` is used to write
 *  out the hash. After `hash_final()` has been invoked, no further calls to
 *  `hash_*` are allowed.
 */
class Hasher {
 public:
  virtual ~Hasher() = default;

  virtual Status hash_update(Error& err, folly::ByteRange data) = 0;

  Status hash_update(Error& err, const folly::IOBuf& data) {
    for (auto chunk : data) {
      FIZZ_RETURN_ON_ERROR(hash_update(err, chunk));
    }
    return Status::Success;
  }

  virtual Status hash_final(Error& err, folly::MutableByteRange out) = 0;

  virtual Status clone(std::unique_ptr<Hasher>& ret, Error& err) const = 0;

  virtual size_t getHashLen() const = 0;

  virtual inline size_t getBlockSize() const = 0;
};

/**
 * HasherFactoryWithMetadata describes a hashing algorithm. It binds an
 * implementation of a hashing algorithm along with properties of the hash.
 *
 * Each backend exposes a singleton instance of `HasherFactoryWithMetadata`
 * for each hashing algorithm the backend supports.
 */
struct HasherFactoryWithMetadata {
  using FactoryFn = std::unique_ptr<Hasher>();

  constexpr explicit HasherFactoryWithMetadata(
      FactoryFn* fn,
      fizz::HashFunction id,
      size_t hashlen,
      size_t blocksize,
      folly::StringPiece blankHash)
      : fn_(fn),
        id_(id),
        hashlen_(hashlen),
        blockSize_(blocksize),
        blankHash_(blankHash) {}

  template <class Hash>
  static constexpr HasherFactoryWithMetadata bind(FactoryFn* fn) {
    return HasherFactoryWithMetadata(
        fn, Hash::HashId, Hash::HashLen, Hash::BlockSize, Hash::BlankHash);
  }

  std::unique_ptr<Hasher> make() const {
    return fn_();
  }

  folly::ByteRange blankHash() const {
    return blankHash_;
  }

  HashFunction id() const {
    return id_;
  }

  size_t hashLength() const {
    return hashlen_;
  }

  size_t blockSize() const {
    return blockSize_;
  }

 private:
  FactoryFn* fn_;
  HashFunction id_;
  size_t hashlen_;
  size_t blockSize_;
  folly::StringPiece blankHash_;
};

/**
 * Puts `Hash(in)` into `out`.
 * `out` must be at least of size HashLen.
 */
Status hash(
    Error& err,
    const HasherFactoryWithMetadata* makeHasher,
    const folly::IOBuf& in,
    folly::MutableByteRange out);

} // namespace fizz
