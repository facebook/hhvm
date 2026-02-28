/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string_view>
#include "watchman/watchman_string.h"

namespace watchman {

inline constexpr std::string_view kCookiePrefix = ".watchman-cookie-";

/**
 * We need to guarantee that we never collapse a cookie notification
 * out of the pending list, because we absolutely must observe it coming
 * in via the kernel notification mechanism in order for synchronization
 * to be correct.
 * Since we don't have a Root available, we can't tell what the
 * precise cookie prefix is for the current pending list here, so
 * we do a substring match.  Not the most elegant thing in the world.
 */
inline bool isPossiblyACookie(w_string_piece path) {
  return path.contains(kCookiePrefix);
}

} // namespace watchman
