/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/delegatedcred/Types.h>
#include <fizz/protocol/OpenSSLPeerCertImpl.h>

namespace fizz {
namespace extensions {

template <KeyType T>
class PeerDelegatedCredential : public OpenSSLPeerCertImpl<T> {
 public:
  explicit PeerDelegatedCredential(
      folly::ssl::X509UniquePtr cert,
      folly::ssl::EvpPkeyUniquePtr pubKey,
      DelegatedCredential credential);

  ~PeerDelegatedCredential() override = default;

  void verify(
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned,
      folly::ByteRange signature) const override;

  SignatureScheme getExpectedScheme() const;

 private:
  DelegatedCredential credential_;
};
} // namespace extensions
} // namespace fizz

#include <fizz/extensions/delegatedcred/PeerDelegatedCredential-inl.h>
