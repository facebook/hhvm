/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

namespace fizz {

enum class Event {
  ClientHello,
  ServerHello,
  EndOfEarlyData,
  HelloRetryRequest,
  EncryptedExtensions,
  CertificateRequest,
  Certificate,
  CompressedCertificate,
  CertificateVerify,
  Finished,
  NewSessionTicket,
  KeyUpdate,
  Alert,
  Accept,
  Connect,
  AppData,
  EarlyAppWrite,
  AppWrite,
  AppClose,
  WriteNewSessionTicket,
  CloseNotify,
  KeyUpdateInitiation,
  NUM_EVENTS
};

template <Event e>
struct EventType {
  static constexpr Event event = e;
};

folly::StringPiece toString(Event event);
} // namespace fizz
