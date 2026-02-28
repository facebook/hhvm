/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

namespace fizz {

struct CryptoUtils {
  /**
   * Returns true if a and b are identical.
   *
   * Constant time in terms of the contents of a and b (but not in terms of
   * length).
   */
  static bool equal(folly::ByteRange a, folly::ByteRange b);

  /**
   * Overwrites the memory in range.
   */
  static void clean(folly::MutableByteRange range);

  /**
   * Initialize all required crypto libraries.
   */
  static void init();
};
} // namespace fizz
