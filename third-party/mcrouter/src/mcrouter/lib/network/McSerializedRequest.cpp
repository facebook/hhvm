/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/network/McSerializedRequest.h"

namespace facebook {
namespace memcache {

size_t McSerializedRequest::getBodySize() {
  switch (protocol_) {
    case mc_ascii_protocol:
      return asciiRequest_.getSize();
    case mc_caret_protocol:
      return caretRequest_.getSizeNoHeader();
    default:
      // Unreachable, see constructor.
      return 0;
  }
}

McSerializedRequest::~McSerializedRequest() {
  switch (protocol_) {
    case mc_ascii_protocol:
      asciiRequest_.~AsciiSerializedRequest();
      break;
    case mc_caret_protocol:
      caretRequest_.~CaretSerializedMessage();
      break;
    default:
      break;
  }
}
} // namespace memcache
} // namespace facebook
