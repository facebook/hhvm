/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "eden/common/utils/ProcessInfoCache.h"

namespace watchman {
/**
 * A struct containing information about the client that is triggering some
 * action in watchman. Currently used for telemetry.
 */
struct ClientContext {
  pid_t clientPid;
  std::optional<facebook::eden::ProcessInfoHandle> clientInfo;
};

} // namespace watchman
