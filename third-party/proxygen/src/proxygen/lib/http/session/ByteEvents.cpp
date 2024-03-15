/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/ByteEvents.h>

#include <folly/Conv.h>

#include <ostream>
#include <proxygen/lib/utils/Time.h>
#include <string>

namespace proxygen {

const char* const kTypeStrings[] = {
    "FIRST_BYTE",
    "LAST_BYTE",
    "PING_REPLY_SENT",
    "FIRST_HEADER_BYTE",
    "TRACKED_BYTE",
    "SECOND_TO_LAST_PACKET",
};

std::ostream& operator<<(std::ostream& os, const ByteEvent& be) {
  os << folly::to<std::string>(
      "(", kTypeStrings[be.eventType_], ", ", be.byteOffset_, ")");
  return os;
}

int64_t PingByteEvent::getLatency() {
  return millisecondsSince(pingRequestReceivedTime_).count();
}

} // namespace proxygen
