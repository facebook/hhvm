/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/compress/HPACKHeader.h>

namespace proxygen {

class HeaderIndexingStrategy {
 public:
  static const HeaderIndexingStrategy* getDefaultInstance();

  // Explicitly defined constructor/destructor
  // Destructor is virtual so that a subclass can provide an implementation
  // and that it will be correctly called even when aliased by a
  // HPACKEnoderStrat* var
  HeaderIndexingStrategy() {
  }
  virtual ~HeaderIndexingStrategy() {
  }

  // Virtual method for subclasses to implement as they see fit
  // Returns a bool that indicates whether the specified header should be
  // indexed
  virtual bool indexHeader(const HPACKHeaderName& name,
                           folly::StringPiece value) const;
};

} // namespace proxygen
