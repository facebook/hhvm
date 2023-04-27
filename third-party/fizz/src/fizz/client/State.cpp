/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/State.h>

namespace fizz {
namespace client {

folly::StringPiece toString(StateEnum state) {
  switch (state) {
    case StateEnum::Uninitialized:
      return "Uninitialized";
    case StateEnum::ExpectingServerHello:
      return "ExpectingServerHello";
    case StateEnum::ExpectingEncryptedExtensions:
      return "ExpectingEncryptedExtensions";
    case StateEnum::ExpectingCertificate:
      return "ExpectingCertificate";
    case StateEnum::ExpectingCertificateVerify:
      return "ExpectingCertificateVerify";
    case StateEnum::ExpectingFinished:
      return "ExpectingFinished";
    case StateEnum::Established:
      return "Established";
    case StateEnum::ExpectingCloseNotify:
      return "ExpectingCloseNotify";
    case StateEnum::Closed:
      return "Closed";
    case StateEnum::Error:
      return "Error";
    case StateEnum::NUM_STATES:
      return "Invalid state NUM_STATES";
  }
  return "Invalid state";
}

folly::StringPiece toString(ClientAuthType auth) {
  switch (auth) {
    case ClientAuthType::NotRequested:
      return "NotRequested";
    case ClientAuthType::Sent:
      return "Sent";
    case ClientAuthType::RequestedNoMatch:
      return "RequestedNoMatch";
    case ClientAuthType::Stored:
      return "Stored";
  }
  return "Invalid client auth type";
}

folly::StringPiece toString(ECHStatus status) {
  switch (status) {
    case ECHStatus::Requested:
      return "Requested";
    case ECHStatus::Accepted:
      return "Accepted";
    case ECHStatus::Rejected:
      return "Rejected";
  }
  return "Invalid ECH Status";
}
} // namespace client
} // namespace fizz
