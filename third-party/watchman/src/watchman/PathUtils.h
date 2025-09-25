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
 * Compute file name for watchman artifacts. If a directory is created as a side
 * effect of this function, the ownership of the directory is verified.
 *
 * @param str The directory for the target filename. If this string is empty,
 * the state directory will be computed and populated. Otherwise, the only logic
 * this function performs is checking if the path is absolute (if requested)
 * @param user The username for the state directory (only used if str is empty)
 * @param suffix The suffix to append to the state directory path (only used if
 * str is empty)
 * @param what Description of what file being computed (for error messages)
 * @param require_absolute Whether the path must be absolute (default: true)
 */
void compute_file_name(
    std::string& str,
    const std::string& user,
    const char* suffix,
    const char* what,
    bool require_absolute = true);
} // namespace watchman
