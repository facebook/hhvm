/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace proxygen {

#define SET_PROXYGEN_ERROR_IF(errorPtr, error) \
  do {                                         \
    if (errorPtr) {                            \
      *errorPtr = error;                       \
    }                                          \
  } while (false)

// clang-format off
// Max must be the last one.
#define PROXYGEN_ERROR_GEN(x)                   \
    x(None),                                    \
    x(Message),                                 \
    x(Connect),                                 \
    x(ConnectTimeout),                          \
    x(Read),                                    \
    x(Write),                                   \
    x(Timeout),                                 \
    x(Handshake),                               \
    x(NoServer),                                \
    x(MaxRedirects),                            \
    x(InvalidRedirect),                         \
    x(ResponseAction),                          \
    x(MaxConnects),                             \
    x(Dropped),                                 \
    x(Connection),                              \
    x(ConnectionReset),                         \
    x(ParseHeader),                             \
    x(ParseBody),                               \
    x(EOF),                                     \
    x(ClientRenegotiation),                     \
    x(Unknown),                                 \
    x(BadDecompress),                           \
    x(SSL),                                     \
    x(StreamAbort),                             \
    x(StreamUnacknowledged),                    \
    x(WriteTimeout),                            \
    x(AddressPrivate),                          \
    x(AddressFamilyNotSupported),               \
    x(DNSResolutionErr),                        \
    x(DNSNoResults),                            \
    x(MalformedInput),                          \
    x(UnsupportedExpectation),                  \
    x(MethodNotSupported),                      \
    x(UnsupportedScheme),                       \
    x(Shutdown),                                \
    x(IngressStateTransition),                  \
    x(ClientSilent),                            \
    x(Canceled),                                \
    x(ParseResponse),                           \
    x(ConnRefused),                             \
    x(DNSOtherServer),                          \
    x(DNSOtherClient),                          \
    x(DNSOtherCancelled),                       \
    x(DNSshutdown),                             \
    x(DNSgetaddrinfo),                          \
    x(DNSthreadpool),                           \
    x(DNSunimplemented),                        \
    x(Network),                                 \
    x(Configuration),                           \
    x(EarlyDataRejected),                       \
    x(EarlyDataFailed),                         \
    x(AuthRequired),                            \
    x(Unauthorized),                            \
    x(EgressEOMSeenOnParentStream),             \
    x(TransportIsDraining),                     \
    x(ParentStreamNotExist),                    \
    x(CreatingStream),                          \
    x(PushNotSupported),                        \
    x(MaxConcurrentOutgoingStreamLimitReached), \
    x(BadSocket),                               \
    x(DuplicatedStreamId),                      \
    x(ClientTransactionGone),                   \
    x(NetworkSwitch),                           \
    x(Forbidden),                               \
    x(InternalError),                           \
    x(Max)
// clang-format on

// Increase this if you add more error types and Max exceeds 127
#define PROXYGEN_ERROR_BITSIZE 7

#define PROXYGEN_ERROR_ENUM(error) kError##error

enum ProxygenError { PROXYGEN_ERROR_GEN(PROXYGEN_ERROR_ENUM) };

#undef PROXYGEN_ERROR_ENUM

extern const char* getErrorString(ProxygenError error);

extern const char* getErrorStringByIndex(int i);

} // namespace proxygen
