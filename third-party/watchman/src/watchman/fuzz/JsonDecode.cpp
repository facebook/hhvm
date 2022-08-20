/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/thirdparty/jansson/jansson.h"

extern "C" int LLVMFuzzerTestOneInput(void const* data, size_t size) {
  json_error_t err{};
  auto* d = reinterpret_cast<const char*>(data);
  try {
    json_loadb(d, size, JSON_DECODE_ANY, &err);
  } catch (std::exception&) {
    // Catchable exceptions are okay.
  }
  return 0;
}
