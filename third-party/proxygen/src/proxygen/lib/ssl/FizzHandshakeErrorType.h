/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <folly/ExceptionWrapper.h>

namespace proxygen {

enum class FizzHandshakeErrorType : uint8_t {
  // No exception.
  None,

  // A TLS protocol level exception.
  Protocol,

  // Handshake failing due to the server rejecting a client certificate.
  // This is categorized separately since we, the server, largely cannot
  // control what kind of certificate the client sends us.
  ServerRejectsClientCert,

  // Handshake failing due to the client rejecting our server certificate.
  // This is categorized separately since we, the server, cannot control
  // what trust stores the client decides to configure themselves with.
  ClientRejectsServerCert,

  // Exception that does not match any of the above categories.
  // Not all exceptions need classification — this captures the
  // remaining non-cert protocol and transport errors.
  Unclassified,
};

FizzHandshakeErrorType fromException(const folly::exception_wrapper& ex);

} // namespace proxygen
