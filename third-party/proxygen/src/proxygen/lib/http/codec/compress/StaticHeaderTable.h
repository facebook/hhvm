/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/HTTPCommonHeaders.h>
#include <proxygen/lib/http/codec/compress/HeaderTable.h>

namespace proxygen {

class StaticHeaderTable : public HeaderTable {

 public:
  explicit StaticHeaderTable(const char* entries[][2], int size);

  static const StaticHeaderTable& get();

  static bool isHeaderCodeInTableWithNonEmptyValue(HTTPHeaderCode headerCode);
};

} // namespace proxygen
