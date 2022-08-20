/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/openssl/OpenSSL.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {

class OpenSSLKeyUtils {
 public:
  /**
   * Generates an new EVP_PKEY on the curve.
   * Throws an exception on error.
   *
   * This is a public interface to the namespaced private method below.
   */
  static folly::ssl::EvpPkeyUniquePtr generateECKeyPair(int curveNid);
};

namespace detail {

/**
 * Validates whether or not the EVP_PKEY belongs to the
 * curve. If not, this throws an exception.
 */
void validateECKey(const folly::ssl::EvpPkeyUniquePtr& key, int curveNid);

#if FIZZ_OPENSSL_HAS_ED25519
/**
 * Validates whether or not the EVP_PKEY belongs to the
 * Edwards curve (currently supports only Ed25519 & Ed448).
 * If not, this throws an exception.
 */
void validateEdKey(const folly::ssl::EvpPkeyUniquePtr& key, int curveNid);
#endif

/**
 * Generates an new EVP_PKEY on the curve.
 * Throws an exception on error.
 */
folly::ssl::EvpPkeyUniquePtr generateECKeyPair(int curveNid);

/**
 * Decodes a EC public key specified as a member of the curve
 * curveNid.
 */
folly::ssl::EvpPkeyUniquePtr decodeECPublicKey(
    folly::ByteRange range,
    int curveNid);

/**
 * Encodes the public key and returns a buffer.
 */

std::unique_ptr<folly::IOBuf> encodeECPublicKey(
    const folly::ssl::EvpPkeyUniquePtr& key);

std::unique_ptr<folly::IOBuf> encodeECPublicKey(
    const folly::ssl::EcKeyUniquePtr& ecKey);

/**
 * Generates a shared secred from a private key, key and the
 * peerKey public key.
 */
std::unique_ptr<folly::IOBuf> generateEvpSharedSecret(
    const folly::ssl::EvpPkeyUniquePtr& key,
    const folly::ssl::EvpPkeyUniquePtr& peerKey);

/**
 * Returns the current error in the thread queue as a string.
 */
std::string getOpenSSLError();
} // namespace detail
} // namespace fizz
