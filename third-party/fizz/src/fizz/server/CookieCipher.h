/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Factory.h>
#include <fizz/protocol/ech/Types.h>
#include <fizz/record/Types.h>

namespace fizz {
namespace server {

struct CookieState {
  ProtocolVersion version;
  CipherSuite cipher;
  folly::Optional<NamedGroup> group;

  Buf chloHash;

  Buf appToken;

  folly::Optional<ech::ECHCipherSuite> echCipherSuite;
  Buf echConfigId;
  Buf echEnc;
};

/**
 * Interface for decrypting prior state information from a cookie. These are
 * never sent through the state machine (they are only useful for applications
 * that require a stateless reset), hence the lack of an encrypt method.
 */
class CookieCipher {
 public:
  virtual ~CookieCipher() = default;

  virtual folly::Optional<CookieState> decrypt(Buf) const = 0;
};

/**
 * Build a stateless HelloRetryRequest. This is deterministic and will be used
 * to reconstruct the handshake transcript when receiving the second
 * ClientHello.
 */
Buf getStatelessHelloRetryRequest(
    ProtocolVersion version,
    CipherSuite cipher,
    folly::Optional<NamedGroup> group,
    Buf cookie);

/**
 * Negotiate and compute the CookieState to use in response to a ClientHello.
 * This must match the logic inside of the server state machine.
 */
CookieState getCookieState(
    const Factory& factory,
    const std::vector<ProtocolVersion>& supportedVersions,
    const std::vector<std::vector<CipherSuite>>& supportedCiphers,
    const std::vector<NamedGroup>& supportedGroups,
    const ClientHello& chlo,
    Buf appToken);
} // namespace server
} // namespace fizz
