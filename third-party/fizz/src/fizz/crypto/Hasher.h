/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBuf.h>

namespace fizz {

/**
 * `Hasher` implements a message digest.
 *
 *  An instance of `Hasher` should be initialized with `hash_init()`. This may
 *  only be invoked once per Hasher instance.
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

  virtual void hash_init() = 0;

  virtual void hash_update(folly::ByteRange data) = 0;

  void hash_update(const folly::IOBuf& data) {
    for (auto chunk : data) {
      hash_update(chunk);
    }
  }

  virtual void hash_final(folly::MutableByteRange out) = 0;

  virtual std::unique_ptr<Hasher> clone() const = 0;

  virtual size_t getHashLen() const = 0;

  virtual size_t getBlockSize() const = 0;
};

} // namespace fizz
