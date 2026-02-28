/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

namespace watchman {

inline constexpr uint32_t kIoBufSize = 1024 * 1024;
inline constexpr uint32_t kBatchLimit = 16 * 1024;

} // namespace watchman

#define WATCHMAN_IO_BUF_SIZE (::watchman::kIoBufSize)
#define WATCHMAN_BATCH_LIMIT (::watchman::kBatchLimit)
