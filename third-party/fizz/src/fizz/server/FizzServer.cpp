/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/FizzServer.h>

namespace fizz {
namespace server {

bool looksLikeV2ClientHello(const folly::IOBufQueue& queue) {
  if (queue.empty()) {
    return false;
  }
  folly::io::Cursor cursor(queue.front());
  if (!cursor.canAdvance(3)) {
    return false;
  }
  uint8_t byte1 = cursor.read<uint8_t>();
  cursor.skip(1);
  uint8_t byte3 = cursor.read<uint8_t>();
  if (byte1 & 0x80 && byte3 == 0x01) {
    return true;
  }
  return false;
}
} // namespace server
} // namespace fizz
