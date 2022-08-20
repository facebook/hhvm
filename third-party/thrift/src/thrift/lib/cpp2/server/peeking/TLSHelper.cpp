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

#include <thrift/lib/cpp2/server/peeking/TLSHelper.h>

#include <folly/io/Cursor.h>

static constexpr size_t kAlertRecordLength = 7;
static constexpr uint8_t kAlertRecordType = 21;
static constexpr uint16_t kAlertFragmentLength = 2;
static constexpr uint8_t kAlertFatalType = 2;

namespace apache {
namespace thrift {

bool TLSHelper::looksLikeTLS(const std::vector<uint8_t>& bytes) {
  CHECK_GE(bytes.size(), kTLSPeekBytes);
  // TLS starts as
  // 0: 0x16 - handshake protocol magic
  // 1: 0x03 - SSL version major
  // 2: 0x00 to 0x03 - SSL version minor (SSLv3 or TLS1.0 through TLS1.3)
  // 3-4: length (2 bytes)
  // 5: 0x01 - handshake type (ClientHello)
  // 6-8: handshake len (3 bytes), equals value from offset 3-4 minus 4

  // Framed binary starts as
  // 0-3: frame len
  // 4: 0x80 - binary magic
  // 5: 0x01 - protocol version
  // 6-7: various
  // 8-11: method name len

  // Other Thrift transports/protocols can't conflict because they don't have
  // 16-03-01 at offsets 0-1-5.

  // Definitely not TLS
  if (bytes[0] != 0x16 || bytes[1] != 0x03 || bytes[5] != 0x01) {
    return false;
  }

  // This is most likely TLS, but could be framed binary, which has 80-01
  // at offsets 4-5.
  if (bytes[4] == 0x80 && bytes[8] != 0x7c) {
    // Binary will have the method name length at offsets 8-11, which must be
    // smaller than the frame length at 0-3, so byte 8 is <=  byte 0,
    // which is 0x16.
    // However, for TLS, bytes 6-8 (24 bits) are the length of the
    // handshake protocol and this value is 4 less than the record-layer
    // length at offset 3-4 (16 bits), so byte 8 equals 0x7c (0x80 - 4),
    // which is not smaller than 0x16
    return false;
  }

  return true;
}

std::unique_ptr<folly::IOBuf> TLSHelper::getPlaintextAlert(
    uint8_t major, uint8_t minor, Alert alert) {
  auto alertBuf = folly::IOBuf::create(kAlertRecordLength);
  folly::io::Appender appender(alertBuf.get(), 0);
  appender.write<uint8_t>(kAlertRecordType);
  appender.write<uint8_t>(major);
  appender.write<uint8_t>(minor);
  appender.writeBE<uint16_t>(kAlertFragmentLength);
  appender.write<uint8_t>(kAlertFatalType);
  appender.write<uint8_t>(static_cast<uint8_t>(alert));
  return alertBuf;
}
} // namespace thrift
} // namespace apache
