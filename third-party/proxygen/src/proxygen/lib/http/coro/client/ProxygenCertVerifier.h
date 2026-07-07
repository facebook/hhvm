/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertificateVerifier.h>
#include <folly/IPAddress.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace proxygen::coro {

// Validation policy for the identity check.
enum class ValidationPolicy : std::uint8_t {
  // A mismatch will fail the validation.
  Enforcing,
  // Validation will always run, but a mismatch will not fail the validation.
  Logging,
};

// Expected identity, either a hostname or an IP address, to match the peer
// certificate against.
struct ExpectedIdentity {
  // Construct an ExpectedIdentity from IP address.
  static ExpectedIdentity expectIP(folly::IPAddress ip);

  // Construct an ExpectedIdentity from DNS hostname.
  static ExpectedIdentity expectDNS(std::string hostname);

  // returns the hostname if the identity is a hostname, otherwise null
  [[nodiscard]] std::optional<std::string> getHostname() const;

  // returns the IP address if the identity is an IP address, otherwise null
  [[nodiscard]] std::optional<folly::IPAddress> getIp() const;

 private:
  explicit ExpectedIdentity(std::variant<std::string, folly::IPAddress> id)
      : identity(std::move(id)) {
  }

  std::variant<std::string, folly::IPAddress> identity;
};

/**
 * Creates a fizz::CertificateVerifier that validates the peer certificate chain
 * with `verifier` and, on top of that, verifies that the leaf certificate
 * matches `expectedIdentity`, handling a mismatch according to `policy`.
 *
 * `verifier` must not be null.
 */
std::shared_ptr<fizz::CertificateVerifier> makeVerifier(
    std::shared_ptr<const fizz::CertificateVerifier> verifier,
    ExpectedIdentity expectedIdentity,
    ValidationPolicy policy);

} // namespace proxygen::coro
