/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/Events.h>

namespace fizz {
folly::StringPiece toString(Event event) {
  switch (event) {
    case Event::ClientHello:
      return "ClientHello";
    case Event::ServerHello:
      return "ServerHello";
    case Event::EndOfEarlyData:
      return "EndOfEarlyData";
    case Event::HelloRetryRequest:
      return "HelloRetryRequest";
    case Event::EncryptedExtensions:
      return "EncryptedExtensions";
    case Event::CertificateRequest:
      return "CertificateRequest";
    case Event::Certificate:
      return "Certificate";
    case Event::CompressedCertificate:
      return "CompressedCertificate";
    case Event::CertificateVerify:
      return "CertificateVerify";
    case Event::Finished:
      return "Finished";
    case Event::NewSessionTicket:
      return "NewSessionTicket";
    case Event::KeyUpdate:
      return "KeyUpdate";
    case Event::Alert:
      return "Alert";
    case Event::Accept:
      return "Accept";
    case Event::Connect:
      return "Connect";
    case Event::AppData:
      return "AppData";
    case Event::EarlyAppWrite:
      return "EarlyAppWrite";
    case Event::AppWrite:
      return "AppWrite";
    case Event::AppClose:
      return "AppClose";
    case Event::WriteNewSessionTicket:
      return "WriteNewSessionTicket";
    case Event::CloseNotify:
      return "CloseNotify";
    case Event::NUM_EVENTS:
      return "Invalid event NUM_EVENTS";
    case Event::KeyUpdateInitiation:
      return "KeyUpdateInitiation";
  }
  return "Unknown event";
}
} // namespace fizz
