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

#pragma once

namespace apache::thrift {

/**
 * Abstract interface that exposes StopTLSv2 per-record encryption state to core
 * thrift/rocket request dispatch.
 */
class StopTLSEncryptionStateProvider {
 public:
  virtual ~StopTLSEncryptionStateProvider() = default;

  /// Whether StopTLSv2 was negotiated on this connection.
  virtual bool isStopTLSNegotiated() const = 0;

  /// Whether any plaintext record has been observed on this connection
  virtual bool hasObservedPlaintext() const = 0;
};

} // namespace apache::thrift
