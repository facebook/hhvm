/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Hasher.h>

namespace fizz {

void hash(
    HasherFactory makeHasher,
    const folly::IOBuf& in,
    folly::MutableByteRange out) {
  auto hasher = makeHasher();

  CHECK_GE(out.size(), hasher->getHashLen());

  hasher->hash_update(in);
  hasher->hash_final(out);
}

} // namespace fizz
