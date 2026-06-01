/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertificateVerifier.h>

namespace proxygen {

// This is an insecure certificate verifier and is not meant to be
// used in production. Using it in production would mean that this will
// leave everyone insecure.
class InsecureVerifierDangerousDoNotUseInProduction
    : public fizz::CertificateVerifier {
 public:
  ~InsecureVerifierDangerousDoNotUseInProduction() override = default;

  fizz::Status verify(std::shared_ptr<const fizz::Cert>& ret,
                      fizz::Error& /* err */,
                      const std::vector<std::shared_ptr<const fizz::PeerCert>>&
                          certs) const override {
    ret = certs.front();
    return fizz::Status::Success;
  }

  fizz::Status getCertificateRequestExtensions(
      std::vector<fizz::Extension>& /* ret */,
      fizz::Error& /* err */) const override {
    return fizz::Status::Success;
  }
};
} // namespace proxygen
