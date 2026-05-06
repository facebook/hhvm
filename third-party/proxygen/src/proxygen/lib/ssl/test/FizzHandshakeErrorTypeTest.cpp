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
#include <folly/ExceptionWrapper.h>
#include <folly/portability/GTest.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>

using namespace proxygen;

namespace {

folly::exception_wrapper makeFizzHandshakeError(
    folly::exception_wrapper inner) {
  return folly::make_exception_wrapper<wangle::FizzHandshakeException>(
      wangle::SSLErrorEnum::NO_ERROR,
      std::chrono::milliseconds(0),
      1,
      std::move(inner));
}

} // namespace

struct ErrorClassificationParam {
  std::string name;
  folly::exception_wrapper exception;
  FizzHandshakeErrorType expectedType;
};

class FizzHandshakeErrorTypeTest
    : public ::testing::TestWithParam<ErrorClassificationParam> {};

std::vector<ErrorClassificationParam> getTestParams() {
  return {
      {.name = "VerificationException",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<fizz::FizzVerificationException>(
               "client certificate failure: bad_certificate",
               fizz::AlertDescription::bad_certificate)),
       .expectedType = FizzHandshakeErrorType::ServerRejectsClientCert},

      {.name = "ReceivedAlert_UnknownCA",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<fizz::FizzException>(
               "received alert: unknown_ca",
               fizz::AlertDescription::unknown_ca)),
       .expectedType = FizzHandshakeErrorType::ClientRejectsServerCert},

      {.name = "ReceivedAlert_BadCertificate",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<fizz::FizzException>(
               "received alert: bad_certificate",
               fizz::AlertDescription::bad_certificate)),
       .expectedType = FizzHandshakeErrorType::ClientRejectsServerCert},

      {.name = "ReceivedAlert_CertificateUnknown",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<fizz::FizzException>(
               "received alert: certificate_unknown",
               fizz::AlertDescription::certificate_unknown)),
       .expectedType = FizzHandshakeErrorType::ClientRejectsServerCert},

      {.name = "ReceivedAlert_CertificateExpired",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<fizz::FizzException>(
               "received alert: certificate_expired",
               fizz::AlertDescription::certificate_expired)),
       .expectedType = FizzHandshakeErrorType::ClientRejectsServerCert},

      {.name = "CertRequestedButNoneReceived",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<fizz::FizzException>(
               "certificate requested but none received",
               fizz::AlertDescription::certificate_required)),
       .expectedType = FizzHandshakeErrorType::ServerRejectsClientCert},

      {.name = "OtherFizzException",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<fizz::FizzException>(
               "unexpected message",
               fizz::AlertDescription::unexpected_message)),
       .expectedType = FizzHandshakeErrorType::Protocol},

      {.name = "NonFizzHandshakeException",
       .exception =
           folly::make_exception_wrapper<std::runtime_error>("some error"),
       .expectedType = FizzHandshakeErrorType::Unclassified},

      {.name = "NonFizzOriginalException",
       .exception = makeFizzHandshakeError(
           folly::make_exception_wrapper<std::runtime_error>(
               "transport error")),
       .expectedType = FizzHandshakeErrorType::Unclassified},
  };
}

TEST_P(FizzHandshakeErrorTypeTest, Classification) {
  const auto& param = GetParam();
  EXPECT_EQ(fromException(param.exception), param.expectedType);
}

INSTANTIATE_TEST_SUITE_P(
    FromException,
    FizzHandshakeErrorTypeTest,
    ::testing::ValuesIn(getTestParams()),
    [](const ::testing::TestParamInfo<ErrorClassificationParam>& info) {
      return info.param.name;
    });
