/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Hasher.h>
#include <fizz/util/Logging.h>

namespace fizz {

Status hash(
    Error& err,
    const HasherFactoryWithMetadata* makeHasher,
    const folly::IOBuf& in,
    folly::MutableByteRange out) {
  std::unique_ptr<Hasher> hasher;
  FIZZ_RETURN_ON_ERROR(makeHasher->make(hasher, err));

  FIZZ_CHECK_GE(out.size(), hasher->getHashLen());

  FIZZ_RETURN_ON_ERROR(hasher->hash_update(err, in));
  FIZZ_RETURN_ON_ERROR(hasher->hash_final(err, out));
  return Status::Success;
}

} // namespace fizz
