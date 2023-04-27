/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/delegatedcred/Types.h>
#include <fizz/protocol/Certificate.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
namespace extensions {

class DelegatedCredentialUtils {
 public:
  /**
   * Checks that the cert passed in has the extensions needed for delegated
   * credentials. Throws an exception if it doesn't have them.
   */
  static void checkExtensions(const folly::ssl::X509UniquePtr& cert);

  /**
   * Returns whether or not the delegated credential extension is present on
   * the certificate passed in.
   */
  static bool hasDelegatedExtension(const folly::ssl::X509UniquePtr& cert);

  /**
   * Constructs the buffer used for verifying the signature on the
   * delegated credential.
   */
  static Buf prepareSignatureBuffer(
      const DelegatedCredential& cred,
      Buf certData);

  /*
   * Generates a delegated credential for a given cert, private key, and
   * delegated private key.
   */
  static DelegatedCredential generateCredential(
      std::shared_ptr<SelfCert> cert,
      const folly::ssl::EvpPkeyUniquePtr& certKey,
      const folly::ssl::EvpPkeyUniquePtr& credKey,
      SignatureScheme signScheme,
      SignatureScheme verifyScheme,
      std::chrono::seconds validSeconds);
};
} // namespace extensions
} // namespace fizz
