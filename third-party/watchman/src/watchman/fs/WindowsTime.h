/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/SysTime.h>
#include <time.h>

namespace watchman {

#ifdef _WIN32

void FILETIME_to_timespec(const FILETIME* ft, timespec* ts);
void FILETIME_LARGE_INTEGER_to_timespec(LARGE_INTEGER ft, timespec* ts);

#endif

} // namespace watchman
