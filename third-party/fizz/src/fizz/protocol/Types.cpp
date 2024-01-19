/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/Types.h>

namespace fizz {

HashFunction getHashFunction(CipherSuite cipher) {
  switch (cipher) {
    case CipherSuite::TLS_AES_128_GCM_SHA256:
    case CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL:
    case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
    case CipherSuite::TLS_AEGIS_128L_SHA256:
      return HashFunction::Sha256;
    case CipherSuite::TLS_AES_256_GCM_SHA384:
      return HashFunction::Sha384;
    case CipherSuite::TLS_AEGIS_256_SHA512:
      return HashFunction::Sha512;
  }
  throw std::runtime_error("unknown cipher suite");
}

size_t getHashSize(HashFunction hash) {
  switch (hash) {
    case HashFunction::Sha256:
      return 32;
    case HashFunction::Sha384:
      return 48;
    case HashFunction::Sha512:
      return 64;
  }
  throw std::runtime_error("unknown hash function");
}

folly::StringPiece toString(HashFunction hash) {
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

folly::StringPiece toString(PskType pskType) {
  switch (pskType) {
    case PskType::NotSupported:
      return "NotSupported";
    case PskType::NotAttempted:
      return "NotAttempted";
    case PskType::Rejected:
      return "Rejected";
    case PskType::External:
      return "External";
    case PskType::Resumption:
      return "Resumption";
  }
  return "Invalid PskType";
}

folly::StringPiece toString(KeyExchangeType kexType) {
  switch (kexType) {
    case KeyExchangeType::None:
      return "None";
    case KeyExchangeType::OneRtt:
      return "OneRtt";
    case KeyExchangeType::HelloRetryRequest:
      return "HelloRetryRequest";
  }
  return "Invalid kex type";
}

folly::StringPiece toString(EarlyDataType earlyDataType) {
  switch (earlyDataType) {
    case EarlyDataType::NotAttempted:
      return "NotAttempted";
    case EarlyDataType::Attempted:
      return "Attempted";
    case EarlyDataType::Rejected:
      return "Rejected";
    case EarlyDataType::Accepted:
      return "Accepted";
  }
  return "Invalid EarlyDataType";
}
} // namespace fizz
