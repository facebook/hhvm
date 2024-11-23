/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <array>

namespace fizz {

// AEAD Ciphers
struct AESGCM128 {
  static constexpr size_t kKeyLength{16};
  static constexpr size_t kIVLength{12};
  static constexpr size_t kTagLength{16};
};

struct AESGCM256 {
  static constexpr size_t kKeyLength{32};
  static constexpr size_t kIVLength{12};
  static constexpr size_t kTagLength{16};
};

struct AESOCB128 {
  static constexpr size_t kKeyLength{16};
  static constexpr size_t kIVLength{12};
  static constexpr size_t kTagLength{16};
};

struct ChaCha20Poly1305 {
  static constexpr size_t kKeyLength{32};
  static constexpr size_t kIVLength{12};
  static constexpr size_t kTagLength{16};
};

struct AEGIS128L {
  static constexpr size_t kKeyLength{16};
  static constexpr size_t kIVLength{16};
  static constexpr size_t kTagLength{16};
};

struct AEGIS256 {
  static constexpr size_t kKeyLength{32};
  static constexpr size_t kIVLength{32};
  static constexpr size_t kTagLength{16};
};

enum class HashFunction { Sha256, Sha384, Sha512 };
inline folly::StringPiece toString(HashFunction hash) {
  switch (hash) {
    case HashFunction::Sha256:
      return "Sha256";
    case HashFunction::Sha384:
      return "Sha384";
    case HashFunction::Sha512:
      return "Sha512";
  }
  return "Invalid HashFunction";
}

// Hashing Algorithms
// Please update HASH_MAX_BLOCK_SIZE if necessary if adding support for more
// hashes.
struct Sha256 {
  static constexpr HashFunction HashId = HashFunction::Sha256;
  static constexpr size_t HashLen = 32;
  static constexpr size_t BlockSize = 64;
  static constexpr folly::StringPiece BlankHash{
      "\xe3\xb0\xc4\x42\x98\xfc\x1c\x14\x9a\xfb\xf4\xc8\x99\x6f\xb9\x24\x27\xae\x41\xe4\x64\x9b\x93\x4c\xa4\x95\x99\x1b\x78\x52\xb8\x55"};
};

struct Sha384 {
  static constexpr HashFunction HashId = HashFunction::Sha384;
  static constexpr size_t HashLen = 48;
  static constexpr size_t BlockSize = 128;
  static constexpr folly::StringPiece BlankHash{
      "\x38\xb0\x60\xa7\x51\xac\x96\x38\x4c\xd9\x32\x7e\xb1\xb1\xe3\x6a\x21\xfd\xb7\x11\x14\xbe\x07\x43\x4c\x0c\xc7\xbf\x63\xf6\xe1\xda\x27\x4e\xde\xbf\xe7\x6f\x65\xfb\xd5\x1a\xd2\xf1\x48\x98\xb9\x5b"};
};

struct Sha512 {
  static constexpr HashFunction HashId = HashFunction::Sha512;
  static constexpr size_t HashLen = 64;
  static constexpr size_t BlockSize = 128;
  static constexpr folly::StringPiece BlankHash{
      "\xcf\x83\xe1\x35\x7e\xef\xb8\xbd\xf1\x54\x28\x50\xd6\x6d\x80\x07\xd6\x20\xe4\x05\x0b\x57\x15\xdc\x83\xf4\xa9\x21\xd3\x6c\xe9\xce\x47\xd0\xd1\x3c\x5d\x85\xf2\xb0\xff\x83\x18\xd2\x87\x7e\xec\x2f\x63\xb9\x31\xbd\x47\x41\x7a\x81\xa5\x38\x32\x7a\xf9\x27\xda\x3e"};
};

inline folly::ByteRange getBlankHash(HashFunction hash) {
  switch (hash) {
    case HashFunction::Sha256:
      return Sha256::BlankHash;
    case HashFunction::Sha384:
      return Sha384::BlankHash;
    case HashFunction::Sha512:
      return Sha512::BlankHash;
  }
  throw std::runtime_error("invalid hash");
}

constexpr size_t kHashMaxBlockSize =
    128; // The largest block size of the hashes that we support. Please keep
         // this definition in close proximity to the tag types.

// Elliptic Curves
struct P256 {
  /**
   * See RFC8446 Section 4.2.8.2
   */
  static constexpr int coordinateLength = 32;
  static constexpr int keyShareLength = coordinateLength * 2 + 1;
};

struct P384 {
  static constexpr int coordinateLength = 48;
  static constexpr int keyShareLength = coordinateLength * 2 + 1;
};

struct P521 {
  static constexpr int coordinateLength = 66;
  static constexpr int keyShareLength = coordinateLength * 2 + 1;
};

struct X25519 {};

// KEMs
struct Kyber512 {};
struct Kyber768 {};
struct MLKEM512 {};
struct MLKEM768 {};

constexpr size_t kEcMaxPublicKeyshareLength =
    256; // The max possible size of a buffer storing an EC keyshare. Please
         // keep this definition in close proximity to the EC Curve tag types.

constexpr size_t kEcMaxSharedSecretLength =
    256; // The max possible size of a buffer storing an EC shared secret.
         // Please keep this definition in close proximity to the tag types.

enum class KeyType { RSA, P256, P384, P521, ED25519 };

using HashFunc = void (*)(const folly::IOBuf&, folly::MutableByteRange);
using HmacFunc =
    void (*)(folly::ByteRange, const folly::IOBuf&, folly::MutableByteRange);
} // namespace fizz
