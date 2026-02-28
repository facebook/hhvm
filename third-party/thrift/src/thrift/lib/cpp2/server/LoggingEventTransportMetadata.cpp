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

#include <thrift/lib/cpp2/server/LoggingEventTransportMetadata.h>

namespace apache::thrift {

namespace {

constexpr std::string_view kIdVerificationError{"id_verification_error"};

} // namespace

void logTransportMetadata(
    const ConnectionLoggingContext& context,
    std::optional<folly::F14NodeMap<std::string, std::string>>
        transportMetadata) {
  folly::dynamic metadata = folly::dynamic::object;
  if (transportMetadata) {
    bool hasIdVerificationError{false};
    for (auto& [key, value] : *transportMetadata) {
      metadata[key] = std::move(value);
      if (key == kIdVerificationError) {
        hasIdVerificationError = true;
      }
    }
    if (hasIdVerificationError) {
      THRIFT_CONNECTION_EVENT(transport_verification_failure).log(context, [&] {
        return std::move(metadata);
      });
    } else {
      THRIFT_CONNECTION_EVENT(transport.metadata).log(context, [&] {
        return std::move(metadata);
      });
    }
  } else {
    THRIFT_CONNECTION_EVENT(no_transport_verification).log(context, [&] {
      return std::move(metadata);
    });
  }
}

} // namespace apache::thrift
