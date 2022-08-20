/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

namespace facebook {
namespace memcache {

constexpr folly::StringPiece kMcSecurityTlsToPlaintextProto = "mc_tls_to_pt";

// an enum to determine which security protocol to use to connect to the
// endpoint.
enum class SecurityMech : uint8_t {
  NONE = 0,
  TLS,
  // A mechanism for exchanging identity via certificates via TLS handshake
  // and then falling back to plaintext over the wire.  This is not considered
  // a secure transport since it lacks confidentiality and integrity.
  TLS_TO_PLAINTEXT,
  // tls 1.3 w/ fizz
  TLS13_FIZZ,
  // TLS 1.2 handshake and attempt kTLS.  If KTLS is not available, it is same
  // as TLS
  KTLS12,
};

// packed struct for stats
struct SecurityTransportStats {
  SecurityTransportStats()
      : tfoSuccess(false),
        tfoAttempted(false),
        tfoFinished(false),
        sessionReuseSuccess(false),
        sessionReuseAttempted(false) {}

  bool tfoSuccess : 1;
  bool tfoAttempted : 1;
  bool tfoFinished : 1;
  bool sessionReuseSuccess : 1;
  bool sessionReuseAttempted : 1;
};

const char* securityMechToString(SecurityMech mech);
SecurityMech parseSecurityMech(folly::StringPiece s);

struct SecurityOptions {
  /**
   * Certificate paths for mutual auth or server cert verification.
   * If cert and key paths are empty, then no client cert is presented
   * If ca path is empty, then no server cert verification is attempted
   */
  std::string sslPemCertPath;
  std::string sslPemKeyPath;
  std::string sslPemCaPath;

  /**
   * enable ssl session caching
   */
  bool sessionCachingEnabled{false};

  /**
   * enable ssl handshake offload to a separate thread pool
   */
  bool sslHandshakeOffload{false};

  /**
   * Service identity of the destination service when SSL is used.
   */
  std::string sslServiceIdentity;

  /**
   * Client service identity must match server service identity
   */
  bool sslAuthorizationEnforce{false};

  /**
   * Whether TFO is enabled for SSL connections
   */
  bool tfoEnabledForSsl{false};

  /**
   * Client side to prefer AES-OCB cipher suite if supported.
   */
  bool tlsPreferOcbCipher{false};
};

} // namespace memcache
} // namespace facebook
