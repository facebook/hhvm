/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace facebook::memcache::cycles {

/**
 * Returns the number of cpu cycles since power-on. This clock doesn't involve
 * a system call. This clock does not measure context switches.
 * Thread-safe.
 * NOTE: Not all cpu and operating systems guarantee that this clock is
 * synchronized and increments constantly across all cpu cores.
 * This clock has no serializing instruction, which means that for some cpu
 * implementations this clock might be inaccurate for measuring a really small
 * amount of instructions due to out-of-order execution.
 */
uint64_t getCpuCycles() noexcept;

} // namespace facebook::memcache::cycles
