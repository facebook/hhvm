/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/HTTPConstants.h>

namespace proxygen {

#define CONNECTION_CLOSE_REASON_STRING(e, r) r,
const char* connectionCloseStrings[] = {
    CONNECTION_CLOSE_REASON_GEN(CONNECTION_CLOSE_REASON_STRING)};
#undef CONNECTION_CLOSE_REASON_STRING

const char* getConnectionCloseReasonStringByIndex(unsigned int index) {
  if (index >= (unsigned int)ConnectionCloseReason::kMAX_REASON) {
    index = (unsigned int)ConnectionCloseReason::kMAX_REASON - 1;
  }

  return connectionCloseStrings[index];
}

const char* getConnectionCloseReasonString(ConnectionCloseReason r) {
  return connectionCloseStrings[(unsigned int)r];
}

} // namespace proxygen
