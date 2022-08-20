/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Synchronized.h>
#include <chrono>
#include <string>
#include "watchman/watchman_string.h"

namespace watchman {

/**
 * Some error conditions will put us into a non-recoverable state where we
 * can't guarantee that we will be operating correctly.  Rather than suffering
 * in silence and misleading our clients, we'll poison ourselves and advertise
 * that we have done so and provide some advice on how the user can cure us.
 */
extern folly::Synchronized<std::string> poisoned_reason;

void set_poison_state(
    w_string_piece dir,
    std::chrono::system_clock::time_point now,
    const char* syscall,
    const std::error_code& err);

} // namespace watchman
