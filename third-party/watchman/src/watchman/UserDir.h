/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace watchman {

/**
 * Returns the username of the current user.
 */
std::string computeUserName();

/**
 * Returns a cached reference to the current user's temporary directory.
 */
const std::string& getTemporaryDirectory();

/**
 * Computes the Watchman state directory corresponding to the given user name.
 */
std::string computeWatchmanStateDirectory(const std::string& user);

} // namespace watchman
