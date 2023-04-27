/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

namespace facebook {
namespace memcache {
namespace globals {

/**
 * @return lazy-initialized hostid.
 */
uint32_t hostid();

/**
 * FOR TEST PURPOSES ONLY
 *
 * Allows to override hostid for testing purposes, resets it on destruction.
 */
struct HostidMock {
  explicit HostidMock(uint32_t value);
  void reset();
  ~HostidMock() {
    reset();
  }
};
} // namespace globals
} // namespace memcache
} // namespace facebook
