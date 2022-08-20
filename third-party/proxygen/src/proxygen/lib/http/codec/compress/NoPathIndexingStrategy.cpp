/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/NoPathIndexingStrategy.h>

namespace proxygen {

const NoPathIndexingStrategy* NoPathIndexingStrategy::getInstance() {
  static const NoPathIndexingStrategy* instance = new NoPathIndexingStrategy();
  return instance;
}

} // namespace proxygen
