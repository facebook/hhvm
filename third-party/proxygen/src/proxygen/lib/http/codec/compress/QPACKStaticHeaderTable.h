/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/HTTPCommonHeaders.h>
#include <proxygen/lib/http/codec/compress/StaticHeaderTable.h>

namespace proxygen {

class QPACKStaticHeaderTable {

 public:
  static const StaticHeaderTable& get();

  // Not currently used
  static bool isHeaderCodeInTableWithNonEmptyValue(HTTPHeaderCode headerCode);
};

} // namespace proxygen
