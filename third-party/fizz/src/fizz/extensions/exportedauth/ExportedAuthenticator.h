/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/AsyncFizzBase.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/Exporter.h>
#include <fizz/protocol/Protocol.h>
#include <fizz/record/Extensions.h>
#include <fizz/record/RecordLayer.h>
#include <fizz/record/Types.h>

namespace fizz {

enum class Direction : uint8_t {
  UPSTREAM, // toward the server
  DOWNSTREAM // toward the client
};

/**
 * Public facing interface for Exported Authenticators
 * (draft-ietf-tls-exported-authenticator) which enable application layer
 * protocols to request or export "authenticators" that can convey proof of
 * additionally identities after the TLS session is established.
 */
class ExportedAuthenticator {
 public:
  /**
   * "request" API
   *
   * Returns an opaque string that should be transmitted by the
   * application over a secure channel to request an authenticator.
   *
   * |certificateRequestContext| is an arbitrary sequence of bytes
   * that should be used to prevent replays.
   */
  static Buf getAuthenticatorRequest(
      Buf certificateRequestContext,
      std::vector<fizz::Extension> extensions);

  /**
   * "authenticate" API
   *
   * Constructs an authenticator in response to the authenticator
   * request given in |authenticatorRequest|, conveying the identity
   * in |cert|.
   */
  static Buf getAuthenticator(
      const fizz::AsyncFizzBase& transport,
      Direction dir,
      const SelfCert& cert,
      Buf authenticatorRequest);

  static Buf makeAuthenticator(
      std::unique_ptr<KeyDerivation>& kderiver,
      std::vector<SignatureScheme> supportedSchemes,
      const SelfCert& cert,
      Buf authenticatorRequest,
      Buf handshakeContext,
      Buf finishedMacKey,
      CertificateVerifyContext context);

  /**
   * "get context" API
   *
   * Returns the certificate_request_context given an authenticator
   */
  static Buf getAuthenticatorContext(Buf authenticator);

  /**
   * "validate" API
   *
   * Returns the certificate chain and extensions. If the authenticator was
   * empty, the certificate chain will contain no certificates.
   **/
  static folly::Optional<std::vector<CertificateEntry>> validateAuthenticator(
      const fizz::AsyncFizzBase& transport,
      Direction dir,
      Buf authenticatorRequest,
      Buf authenticator);

  static folly::Optional<std::vector<CertificateEntry>> validate(
      std::unique_ptr<KeyDerivation>& kderiver,
      Buf authenticatorRequest,
      Buf authenticator,
      Buf handshakeContext,
      Buf finishedMacKey,
      CertificateVerifyContext context);
};

} // namespace fizz
