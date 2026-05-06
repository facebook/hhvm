/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/ssl/FizzHandshakeErrorType.h>

#include <fizz/record/Alerts.h>
#include <fizz/util/Exceptions.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>

#include <array>

namespace {

bool isReceivedCertAlert(std::string_view msg) {
  static constexpr std::string_view kReceivedAlert = "received alert";
  static constexpr std::array<std::string_view, 4> kCertAlerts = {{
      "unknown_ca",
      "bad_certificate",
      "certificate_unknown",
      "certificate_expired",
  }};
  auto pos = msg.find(kReceivedAlert);
  if (pos == std::string_view::npos) {
    return false;
  }
  for (auto alert : kCertAlerts) {
    if (msg.find(alert, pos) != std::string_view::npos) {
      return true;
    }
  }
  return false;
}

} // namespace

namespace proxygen {

FizzHandshakeErrorType fromException(const folly::exception_wrapper& ex) {
  auto errorType = FizzHandshakeErrorType::Unclassified;
  ex.with_exception([&](const wangle::FizzHandshakeException& e) {
    auto& orig = e.getOriginalException();
    if (orig.is_compatible_with<fizz::FizzVerificationException>()) {
      errorType = FizzHandshakeErrorType::ServerRejectsClientCert;
    } else if (orig.is_compatible_with<fizz::FizzException>()) {
      orig.with_exception([&](const fizz::FizzException& fizzEx) {
        if (fizzEx.getAlert() == fizz::AlertDescription::certificate_required) {
          errorType = FizzHandshakeErrorType::ServerRejectsClientCert;
        } else if (isReceivedCertAlert(fizzEx.what())) {
          errorType = FizzHandshakeErrorType::ClientRejectsServerCert;
        } else {
          errorType = FizzHandshakeErrorType::Protocol;
        }
      });
    }
  });
  return errorType;
}

} // namespace proxygen
