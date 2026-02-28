/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/LogConfig.h"
#include "watchman/Logging.h"

namespace watchman::logging {

int log_level = LogLevel::ERR;
std::string log_name;

} // namespace watchman::logging
