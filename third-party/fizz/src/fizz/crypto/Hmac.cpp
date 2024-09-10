/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/Hmac.h>

namespace fizz {

constexpr uint8_t HMAC_OPAD = 0x5c;
constexpr uint8_t HMAC_IPAD = 0x36;

// Reference: RFC 2104
// H(K XOR opad, H(K XOR ipad, text))
void hmac(
    HasherFactory makeHasher,
    folly::ByteRange key,
    const folly::IOBuf& in,
    folly::MutableByteRange out) {
  auto inner = makeHasher();
  auto outer = makeHasher();

  size_t L = outer->getHashLen();
  CHECK_GE(out.size(), L);

  size_t B = outer->getBlockSize();
  CHECK_GT(B, 0);

  std::array<uint8_t, fizz::kHashMaxBlockSize> finalKey;

  // K must be of length B (blocksize) so we initialize with zeros up to
  // length B. When key is hashed or copied into finalKey, it will have padding
  // up to B
  memset(finalKey.data(), 0, B);

  // first hash the key if it's length is greater than B
  if (key.size() > B) {
    auto keyHasher = makeHasher();
    keyHasher->hash_update(folly::ByteRange(key.data(), key.size()));
    keyHasher->hash_final(folly::MutableByteRange(finalKey.data(), L));
  } else {
    memcpy(finalKey.data(), key.data(), key.size());
  }

  std::array<uint8_t, fizz::kHashMaxBlockSize> buf;

  // K XOR ipad
  memset(buf.data(), HMAC_IPAD, B);
  for (size_t i = 0; i < B; i++) {
    buf[i] ^= finalKey[i];
  }
  inner->hash_update(folly::ByteRange(buf.data(), B));

  // K XOR opad
  memset(buf.data(), HMAC_OPAD, B);
  for (size_t i = 0; i < B; i++) {
    buf[i] ^= finalKey[i];
  }
  outer->hash_update(folly::ByteRange(buf.data(), B));

  // H(K XOR ipad, text)
  inner->hash_update(in);
  inner->hash_final(folly::MutableByteRange(buf.data(), L));

  // H(K XOR opad, H(K XOR ipad, text))
  outer->hash_update(folly::ByteRange(buf.data(), L));
  outer->hash_final(out);
}

} // namespace fizz
