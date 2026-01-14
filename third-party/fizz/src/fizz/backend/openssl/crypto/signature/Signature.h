/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/OpenSSL.h>
#include <fizz/record/Types.h>
#include <folly/Range.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
namespace openssl {

enum class KeyType { RSA, P256, P384, P521, ED25519 };

/**
 * Signature implementation using OpenSSL.
 */
template <KeyType T>
class OpenSSLSignature {
 public:
  void setKey(folly::ssl::EvpPkeyUniquePtr pkey);
  /**
   * Returns the EVP_PKEY private key as a unique pointer.
   */
  folly::ssl::EvpPkeyUniquePtr getKey() const;

  /**
   * Returns a signature of data.
   *
   * Only valid for SignatureSchemes that are compatible with KeyType.
   *
   * setKey() must be called before with a private key.
   */
  template <SignatureScheme Scheme>
  std::unique_ptr<folly::IOBuf> sign(folly::ByteRange data) const;

  /**
   * Verifies that signature is a valid signature over data. Throws if it's not.
   *
   * Only valid for SignatureSchemes that are compatible with KeyType.
   *
   * setKey() must be called before.
   */
  template <SignatureScheme Scheme>
  void verify(folly::ByteRange data, folly::ByteRange signature) const;

 private:
  folly::ssl::EvpPkeyUniquePtr pkey_;
};

} // namespace openssl
} // namespace fizz

#include <fizz/backend/openssl/crypto/signature/Signature-inl.h>
