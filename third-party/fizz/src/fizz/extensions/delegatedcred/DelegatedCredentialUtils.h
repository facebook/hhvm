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
#include <fizz/protocol/clock/Clock.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
namespace extensions {

class DelegatedCredentialUtils {
 public:
  /**
   * Checks that the cert passed in has the extensions needed for delegated
   * credentials. Returns an error if it doesn't have them.
   */
  static Status checkExtensions(
      Error& err,
      const folly::ssl::X509UniquePtr& cert);

  /**
   * Returns whether or not the delegated credential extension is present on
   * the certificate passed in.
   */
  static Status hasDelegatedExtension(
      bool& ret,
      Error& err,
      const folly::ssl::X509UniquePtr& cert);

  static std::chrono::system_clock::time_point getCredentialExpiresTime(
      const folly::ssl::X509UniquePtr& parentCert,
      const DelegatedCredential& credential);

  static Status checkCredentialTimeValidity(
      Error& err,
      const folly::ssl::X509UniquePtr& parentCert,
      const DelegatedCredential& credential,
      const std::shared_ptr<Clock>& clock);

  /**
   * Constructs the buffer used for verifying the signature on the
   * delegated credential.
   */
  static Status prepareSignatureBuffer(
      Buf& ret,
      Error& err,
      const DelegatedCredential& cred,
      Buf certData);

  /*
   * Generates a delegated credential for a given cert, private key, and
   * delegated private key.
   */
  static Status generateCredential(
      DelegatedCredential& ret,
      Error& err,
      std::shared_ptr<SelfCert> cert,
      const folly::ssl::EvpPkeyUniquePtr& certKey,
      const folly::ssl::EvpPkeyUniquePtr& credKey,
      SignatureScheme signScheme,
      SignatureScheme verifyScheme,
      CertificateVerifyContext verifyContext,
      std::chrono::seconds validSeconds);
};
} // namespace extensions
} // namespace fizz
