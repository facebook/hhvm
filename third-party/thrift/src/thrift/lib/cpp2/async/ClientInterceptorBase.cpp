/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/async/ClientInterceptorBase.h>

#include <folly/lang/SafeAssert.h>

#include <fmt/core.h>

namespace apache::thrift {

namespace {
std::string makeExceptionDescription(
    ClientInterceptorException::CallbackKind callbackKind,
    const std::vector<ClientInterceptorException::SingleExceptionInfo>&
        causes) {
  FOLLY_SAFE_CHECK(
      !causes.empty(), "no exceptions should be thrown without cause");
  std::string message = fmt::format(
      "ClientInterceptor::{} threw exceptions:\n[{}] {}\n",
      callbackKind == ClientInterceptorException::CallbackKind::ON_REQUEST
          ? "onRequest"
          : "onResponse",
      causes[0].sourceInterceptorName,
      folly::exceptionStr(causes[0].cause));
  for (std::size_t i = 1; i < causes.size(); ++i) {
    message += fmt::format(
        "[{}] {}\n",
        causes[i].sourceInterceptorName,
        folly::exceptionStr(causes[i].cause));
  }
  return message;
}
} // namespace

ClientInterceptorException::ClientInterceptorException(
    CallbackKind callbackKind, std::vector<SingleExceptionInfo> causes)
    : std::runtime_error(makeExceptionDescription(callbackKind, causes)),
      causes_(std::move(causes)) {}

} // namespace apache::thrift
