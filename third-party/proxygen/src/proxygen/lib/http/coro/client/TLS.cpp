/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/TLS.h"

namespace proxygen::coro {

const char* getFallbackCAPath() {
#ifdef PROXYGEN_FALLBACK_CA_PATH
  return PROXYGEN_FALLBACK_CA_PATH;
#else
  return nullptr;
#endif
}

} // namespace proxygen::coro
