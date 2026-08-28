/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#ifdef __linux__
#include <chrono>
#include <cstdint>
#include <optional>
#include <thread>
#endif

namespace watchman {

#ifdef __linux__
enum class ExecutableChange {
  Unchanged,
  Changed,
  Unknown,
};

struct FileIdentity {
  uint64_t device = 0;
  uint64_t inode = 0;

  bool operator==(const FileIdentity&) const = default;
};

/**
 * Device and inode of whatever `path` resolves to, following symlinks and
 * skipping name checks so magic links such as `/proc/self/exe` are accepted.
 * Returns nullopt if the path cannot be inspected.
 */
std::optional<FileIdentity> getIdentityForPath(const char* path);

/**
 * Whether the file at `path` is still the one `running` identifies. `Unknown`
 * means the path could not be inspected; it is not evidence of a change.
 */
ExecutableChange checkExecutableChange(
    const FileIdentity& running,
    const char* path);
std::thread startBinaryChangeMonitorThread(std::chrono::seconds checkInterval);
#endif
void startSanityCheckThread();

} // namespace watchman
