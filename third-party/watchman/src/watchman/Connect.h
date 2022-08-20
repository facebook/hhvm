/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "watchman/Result.h"

namespace watchman {
class Stream;
}

/**
 * Connect to a running Watchman instance via unix socket or a named pipe.
 *
 * Returns a connected stream, or an errno upon error.
 */
watchman::ResultErrno<std::unique_ptr<watchman::Stream>> w_stm_connect(
    int timeoutms);
