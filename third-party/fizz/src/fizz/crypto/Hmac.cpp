/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/Hmac.h>
#include <fizz/util/Logging.h>

namespace fizz {

constexpr uint8_t HMAC_OPAD = 0x5c;
constexpr uint8_t HMAC_IPAD = 0x36;

// Reference: RFC 2104
// H(K XOR opad, H(K XOR ipad, text))
Status hmac(
    Error& err,
    const HasherFactoryWithMetadata* makeHasher,
    folly::ByteRange key,
    const folly::IOBuf& in,
    folly::MutableByteRange out) {
  std::unique_ptr<Hasher> inner;
  FIZZ_RETURN_ON_ERROR(makeHasher->make(inner, err));
  std::unique_ptr<Hasher> outer;
  FIZZ_RETURN_ON_ERROR(makeHasher->make(outer, err));

  size_t L = outer->getHashLen();
  FIZZ_CHECK_GE(out.size(), L);

  size_t B = outer->getBlockSize();
  FIZZ_CHECK_GT(B, 0UL);

  std::array<uint8_t, fizz::kHashMaxBlockSize> finalKey;

  // K must be of length B (blocksize) so we initialize with zeros up to
  // length B. When key is hashed or copied into finalKey, it will have padding
  // up to B
  memset(finalKey.data(), 0, B);

  // first hash the key if it's length is greater than B
  if (key.size() > B) {
    std::unique_ptr<Hasher> keyHasher;
    FIZZ_RETURN_ON_ERROR(makeHasher->make(keyHasher, err));
    FIZZ_RETURN_ON_ERROR(
        keyHasher->hash_update(err, folly::ByteRange(key.data(), key.size())));
    FIZZ_RETURN_ON_ERROR(keyHasher->hash_final(
        err, folly::MutableByteRange(finalKey.data(), L)));
  } else {
    memcpy(finalKey.data(), key.data(), key.size());
  }

  std::array<uint8_t, fizz::kHashMaxBlockSize> buf;

  // K XOR ipad
  memset(buf.data(), HMAC_IPAD, B);
  for (size_t i = 0; i < B; i++) {
    buf[i] ^= finalKey[i];
  }
  FIZZ_RETURN_ON_ERROR(
      inner->hash_update(err, folly::ByteRange(buf.data(), B)));

  // K XOR opad
  memset(buf.data(), HMAC_OPAD, B);
  for (size_t i = 0; i < B; i++) {
    buf[i] ^= finalKey[i];
  }
  FIZZ_RETURN_ON_ERROR(
      outer->hash_update(err, folly::ByteRange(buf.data(), B)));

  // H(K XOR ipad, text)
  FIZZ_RETURN_ON_ERROR(inner->hash_update(err, in));
  FIZZ_RETURN_ON_ERROR(
      inner->hash_final(err, folly::MutableByteRange(buf.data(), L)));

  // H(K XOR opad, H(K XOR ipad, text))
  FIZZ_RETURN_ON_ERROR(
      outer->hash_update(err, folly::ByteRange(buf.data(), L)));
  FIZZ_RETURN_ON_ERROR(outer->hash_final(err, out));
  return Status::Success;
}

} // namespace fizz
