/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBuf.h>

namespace fizz {
namespace hpke {

using HpkeSuiteId = std::unique_ptr<folly::IOBuf>;

enum class KEMId : uint16_t {
  secp256r1 = 0x0010,
  secp384r1 = 0x0011,
  secp521r1 = 0x0012,
  x25519 = 0x0020,
  x448 = 0x0021,
};

enum class KDFId : uint16_t {
  Sha256 = 0x0001,
  Sha384 = 0x0002,
  Sha512 = 0x0003,
};

enum class AeadId : uint16_t {
  TLS_AES_128_GCM_SHA256 = 0x0001,
  TLS_AES_256_GCM_SHA384 = 0x0002,
  TLS_CHACHA20_POLY1305_SHA256 = 0x0003,
};

enum class Mode : uint8_t {
  Base = 0x00,
  Psk = 0x01,
  Auth = 0x02,
  AuthPsk = 0x03,
};

} // namespace hpke
} // namespace fizz
