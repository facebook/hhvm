/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HeaderIndexingStrategy.h>

namespace proxygen {

const HeaderIndexingStrategy* HeaderIndexingStrategy::getDefaultInstance() {
  static const HeaderIndexingStrategy* instance = new HeaderIndexingStrategy();
  return instance;
}

bool HeaderIndexingStrategy::indexHeader(const HPACKHeaderName& name,
                                         folly::StringPiece value,
                                         bool) const {
  // Handle all the cases where we want to return false in the switch statement
  // below; else let the code fall through and return true
  switch (name.getHeaderCode()) {
    case HTTP_HEADER_COLON_PATH:
      if (value.find('=') != std::string::npos) {
        return false;
      }
      if (value.find("jpg") != std::string::npos) {
        return false;
      }
      break;

    // The wrapped header should never be HTTP_HEADER_NONE but for completeness
    // the condition is included below
    case HTTP_HEADER_NONE:
    case HTTP_HEADER_CONTENT_LENGTH:
    case HTTP_HEADER_IF_MODIFIED_SINCE:
    case HTTP_HEADER_LAST_MODIFIED:
      return false;

    default:
      break;
  }

  return true;
}

} // namespace proxygen
