/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/fbi/queue.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

// Forward declaration.
struct awriter_entry_t;

struct awriter_callbacks_t {
  void (*completed)(awriter_entry_t*, int);
  int (*perform_write)(awriter_entry_t*);
};

struct awriter_entry_t {
  TAILQ_ENTRY(awriter_entry_t) links;
  void* context;
  const awriter_callbacks_t* callbacks;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
