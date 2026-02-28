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

#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>

#include <folly/Range.h>

namespace apache::thrift::rocket {

folly::StringPiece toString(ErrorCode ec) {
  switch (ec) {
    case ErrorCode::RESERVED:
      return "RESERVED";
    case ErrorCode::INVALID_SETUP:
      return "INVALID_SETUP";
    case ErrorCode::UNSUPPORTED_SETUP:
      return "UNSUPPORTED_SETUP";
    case ErrorCode::REJECTED_SETUP:
      return "REJECTED_SETUP";
    case ErrorCode::REJECTED_RESUME:
      return "REJECTED_RESUME";
    case ErrorCode::CONNECTION_ERROR:
      return "CONNECTION_ERROR";
    case ErrorCode::CONNECTION_CLOSE:
      return "CONNECTION_CLOSE";
    case ErrorCode::APPLICATION_ERROR:
      return "APPLICATION_ERROR";
    case ErrorCode::REJECTED:
      return "REJECTED";
    case ErrorCode::CANCELED:
      return "CANCELED";
    case ErrorCode::INVALID:
      return "INVALID";
    case ErrorCode::RESERVED_EXT:
      return "RESERVED_EXT";
  }
  return "UNKNOWN";
}

} // namespace apache::thrift::rocket
