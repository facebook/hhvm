/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

namespace facebook::common::mysql_client {

using Duration = std::chrono::duration<uint64_t, std::micro>;
using Timepoint = std::chrono::time_point<std::chrono::steady_clock>;

} // namespace facebook::common::mysql_client
