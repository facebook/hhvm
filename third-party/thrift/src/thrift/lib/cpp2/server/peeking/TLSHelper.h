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

#include <vector>

#include <folly/io/IOBuf.h>

namespace apache::thrift {

constexpr uint8_t kTLSPeekBytes = 9;

class TLSHelper {
 public:
  enum class Alert : uint8_t {
    UNEXPECTED_MESSAGE = 10,
  };

  /**
   * Checks whether or not the peeked bytes look like TLS bytes and not
   * thrift bytes.
   */
  static bool looksLikeTLS(const std::vector<uint8_t>& bytes);

  /**
   * Returns an alert message corresponding to an unexpected SSL message.
   * This is meant to deal with the fact that openssl does not provide
   * a method to serialize plaintext, and only serializes alerts that can
   * be sent over plaintext.
   */
  static std::unique_ptr<folly::IOBuf> getPlaintextAlert(
      uint8_t major, uint8_t minor, Alert alert);
};
} // namespace apache::thrift
