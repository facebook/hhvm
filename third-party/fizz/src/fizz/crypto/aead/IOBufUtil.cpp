/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/IOBufUtil.h>

using namespace folly;

namespace fizz {

void trimBytes(IOBuf& buf, folly::MutableByteRange trimmed) {
  auto trim = trimmed.size();
  size_t currentTrim = trim;
  IOBuf* current = buf.prev();
  // Iterate using the buffer.
  do {
    size_t toTrim =
        std::min(currentTrim, static_cast<size_t>(current->length()));
    memcpy(
        trimmed.begin() + (currentTrim - toTrim),
        current->data() + (current->length() - toTrim),
        toTrim);
    current->trimEnd(toTrim);
    currentTrim -= toTrim;
    DCHECK(current != &buf || currentTrim == 0);
    current = current->prev();
  } while (currentTrim != 0);
}

void trimStart(IOBuf& buf, size_t toTrim) {
  // Trimming on the IOBufQueue is slow, it does a bunch of
  // pop operations, trimming manually is faster.
  auto currentBuffer = &buf;
  do {
    auto amtTrim = std::min(currentBuffer->length(), toTrim);
    currentBuffer->trimStart(amtTrim);
    toTrim -= amtTrim;
    currentBuffer = currentBuffer->next();
  } while (toTrim > 0 && currentBuffer != &buf);
}

void XOR(ByteRange first, MutableByteRange second) {
  CHECK_EQ(first.size(), second.size());
  for (size_t i = 0; i < first.size(); ++i) {
    second[i] ^= first[i];
  }
}
} // namespace fizz
