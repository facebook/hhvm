/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <folly/Range.h>

namespace fizz {

enum class HashFunction { Sha256, Sha384 };

enum class PskType {
  NotSupported,
  NotAttempted,
  Rejected,
  External,
  Resumption
};

/**
 * Encryption level for the TLS layer.
 */
enum class EncryptionLevel { Plaintext, Handshake, EarlyData, AppTraffic };

enum class KeyExchangeType { None, OneRtt, HelloRetryRequest };

enum class EarlyDataType { NotAttempted, Attempted, Rejected, Accepted };

HashFunction getHashFunction(CipherSuite cipher);
size_t getHashSize(HashFunction hash);

folly::StringPiece toString(HashFunction hash);
folly::StringPiece toString(PskType pskType);
folly::StringPiece toString(KeyExchangeType kexType);
folly::StringPiece toString(EarlyDataType earlyDataType);
} // namespace fizz
