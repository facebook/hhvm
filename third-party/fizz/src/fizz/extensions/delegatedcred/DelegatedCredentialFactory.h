/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/extensions/delegatedcred/Types.h>
#include <fizz/protocol/OpenSSLFactory.h>

namespace fizz {
namespace extensions {

/**
 * This class allows delegated credentials to be parsed when sent by the server.
 */
class DelegatedCredentialFactory : public OpenSSLFactory {
 public:
  ~DelegatedCredentialFactory() override = default;

  std::shared_ptr<PeerCert> makePeerCert(CertificateEntry entry, bool leaf)
      const override;

  static std::shared_ptr<PeerCert> makePeerCertStatic(
      CertificateEntry entry,
      bool leaf);
};
} // namespace extensions
} // namespace fizz
