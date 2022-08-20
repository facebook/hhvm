/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include "watchman/fs/FileDescriptor.h"
#include "watchman/watchman_string.h"

namespace watchman {

/** Returns CaseSensitive or CaseInSensitive depending on the
 * case sensitivity of the input path. */
CaseSensitivity getCaseSensitivityForPath(const char* path);

} // namespace watchman

// Returns the name of the filesystem for the specified path
w_string w_fstype(const char* path);
w_string find_fstype_in_linux_proc_mounts(
    std::string_view path,
    std::string_view procMountsData);

inline bool is_edenfs_fs_type(w_string_piece fs_type) {
  return fs_type == "edenfs" || fs_type.startsWith("edenfs:");
}
