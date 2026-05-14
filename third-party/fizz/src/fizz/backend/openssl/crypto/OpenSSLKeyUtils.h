/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/OpenSSL.h>
#include <fizz/util/Status.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
namespace openssl {

class OpenSSLKeyUtils {
 public:
  /**
   * Generates an new EVP_PKEY on the curve.
   * Returns Status::Fail on error.
   *
   * This is a public interface to the namespaced private method below.
   */
  static Status generateECKeyPair(
      folly::ssl::EvpPkeyUniquePtr& ret,
      Error& err,
      int curveNid);
};

namespace detail {

/**
 * Validates whether or not the EVP_PKEY belongs to the
 * curve. Returns Status::Fail on error.
 */
Status validateECKey(
    Error& err,
    const folly::ssl::EvpPkeyUniquePtr& key,
    int curveNid);

/**
 * Validates whether or not the EVP_PKEY belongs to the
 * Edwards curve (currently supports only Ed25519 & Ed448).
 * Returns Status::Fail on error.
 */
Status validateEdKey(
    Error& err,
    const folly::ssl::EvpPkeyUniquePtr& key,
    int curveNid);

/**
 * Generates an new EVP_PKEY on the curve.
 * Returns Status::Fail on error.
 */
Status
generateECKeyPair(folly::ssl::EvpPkeyUniquePtr& ret, Error& err, int curveNid);

/**
 * Decodes a EC public key specified as a member of the curve
 * curveNid.
 */
Status decodeECPublicKey(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    folly::ByteRange range,
    int curveNid);

/**
 * Encodes the public key and returns a buffer.
 */

Status encodeECPublicKey(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const folly::ssl::EvpPkeyUniquePtr& key);

Status encodeECPublicKey(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const folly::ssl::EcKeyUniquePtr& ecKey);

/**
 * Generates a shared secred from a private key, key and the
 * peerKey public key.
 */
Status generateEvpSharedSecret(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const folly::ssl::EvpPkeyUniquePtr& key,
    const folly::ssl::EvpPkeyUniquePtr& peerKey);

/**
 * Returns the current error in the thread queue as a string.
 */
std::string getOpenSSLError();
} // namespace detail
} // namespace openssl
} // namespace fizz
