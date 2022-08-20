/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/WindowsTime.h"
#include <stdint.h>
#include "watchman/watchman_time.h"

namespace watchman {

#ifdef _WIN32

namespace {

// 100's of nanoseconds since the FILETIME epoch
static constexpr uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

} // namespace

// FILETIME is expressed in 100's of nanoseconds
void FILETIME_LARGE_INTEGER_to_timespec(LARGE_INTEGER ft, timespec* ts) {
  static const uint32_t factor = WATCHMAN_NSEC_IN_SEC / 100;

  ft.QuadPart -= EPOCH;
  ts->tv_sec = ft.QuadPart / factor;
  ft.QuadPart -= ts->tv_sec * factor;
  ts->tv_nsec = ft.QuadPart * 100;
}

void FILETIME_to_timespec(const FILETIME* ft, timespec* ts) {
  LARGE_INTEGER li;

  li.HighPart = ft->dwHighDateTime;
  li.LowPart = ft->dwLowDateTime;

  FILETIME_LARGE_INTEGER_to_timespec(li, ts);
}

#endif

} // namespace watchman
