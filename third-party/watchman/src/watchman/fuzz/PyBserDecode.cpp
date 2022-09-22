/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/python/pywatchman/bser.h"

extern "C" int LLVMFuzzerTestOneInput(void const* data, size_t size) {
  // libfuzzer is not happy if Py_Initialize initializes signal handlers.
  Py_InitializeEx(0);

  auto* d = reinterpret_cast<const char*>(data);
  unser_ctx_t ctx{};
  ctx.is_mutable = false;
  ctx.value_encoding = nullptr;
  ctx.value_errors = nullptr;
  ctx.bser_version = 1;
  ctx.bser_capabilities = 0;

  PyObject* parsed = bser_loads_recursive(&d, d + size, &ctx);
  if (!parsed) {
    // Raised parse errors are okay.
    PyErr_Clear();
    return 0;
  }
  Py_DECREF(parsed);
  return 0;
}
