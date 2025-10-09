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

#include <thrift/lib/cpp2/TrustedServerException.h>

#include <thrift/lib/cpp2/async/ResponseChannel.h>

namespace apache::thrift {

/* static */ TrustedServerException TrustedServerException::requestParsingError(
    const char* message) {
  return TrustedServerException{
      TApplicationException::PROTOCOL_ERROR, message, kRequestParsingErrorCode};
}

/* static */ TrustedServerException
TrustedServerException::unimplementedMethodError(const char* message) {
  return TrustedServerException{
      TApplicationException::UNKNOWN_METHOD,
      message,
      kUnimplementedMethodErrorCode};
}

/* static */ TrustedServerException TrustedServerException::appOverloadError(
    const std::string& message) {
  return TrustedServerException{
      TApplicationException::LOADSHEDDING, message, kAppOverloadedErrorCode};
}

/* static */ TrustedServerException TrustedServerException::badInteractionState(
    const std::string& message) {
  return TrustedServerException{
      TApplicationException::UNKNOWN, message, kUnknownErrorCode};
}

} // namespace apache::thrift
