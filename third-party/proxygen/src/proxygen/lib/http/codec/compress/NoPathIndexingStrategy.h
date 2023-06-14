/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/HeaderIndexingStrategy.h>

namespace proxygen {

class NoPathIndexingStrategy : public HeaderIndexingStrategy {
 public:
  static const NoPathIndexingStrategy* getInstance();

  NoPathIndexingStrategy() : HeaderIndexingStrategy() {
  }

  // For compression simulations we do not want to index :path headers
  bool indexHeader(const HPACKHeaderName& name,
                   folly::StringPiece value,
                   bool) const override {
    if (name.getHeaderCode() == HTTP_HEADER_COLON_PATH) {
      return false;
    } else {
      return HeaderIndexingStrategy::indexHeader(name, value);
    }
  }
};

} // namespace proxygen
