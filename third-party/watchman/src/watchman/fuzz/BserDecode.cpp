/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/bser.h"

extern "C" int LLVMFuzzerTestOneInput(void const* data, size_t size) {
  auto* d = reinterpret_cast<const char*>(data);
  try {
    bunser(d, d + size);
  } catch (const BserParseError&) {
    // Caught parse errors are okay.
  }
  return 0;
}
