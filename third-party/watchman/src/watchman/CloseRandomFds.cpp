/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/CloseRandomFds.h"

#include "watchman/watchman_system.h"

#include <folly/portability/Dirent.h>
#include <folly/portability/Fcntl.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/Unistd.h>

#include <climits>
#include <cstdlib>
#include <vector>

#ifndef _WIN32
#include "watchman/WatchmanConfig.h"
#endif

namespace watchman {

bool shouldCloseInheritedFd(int fd) {
#ifdef _WIN32
  (void)fd;
  return false;
#else
  if (fd <= STDERR_FILENO) {
    return false;
  }
  const int flags = fcntl(fd, F_GETFD);
  return flags != -1 && (flags & FD_CLOEXEC) == 0;
#endif
}

void close_random_fds() {
#ifdef __linux__
  // Visit only the descriptors that are actually open: the fd limit can be
  // enormous in containers, so iterating the whole range is prohibitive.
  if (DIR* dir = opendir("/proc/self/fd")) {
    const int dir_fd = dirfd(dir);
    // Collect before closing: mutating the fd table while iterating
    // /proc/self/fd can cause readdir() to skip entries.
    std::vector<int> to_close;
    while (struct dirent* ent = readdir(dir)) {
      const int fd = atoi(ent->d_name);
      if (fd != dir_fd && shouldCloseInheritedFd(fd)) {
        to_close.push_back(fd);
      }
    }
    closedir(dir);
    for (const int fd : to_close) {
      folly::fileops::close(fd);
    }
    return;
  }
  // If /proc is unavailable, fall through to the bounded scan below.
#endif

#ifndef _WIN32
  struct rlimit limit{};
  long open_max = 0;

  // Deduce the upper bound for number of descriptors
  limit.rlim_cur = 0;
#ifdef RLIMIT_NOFILE
  if (getrlimit(RLIMIT_NOFILE, &limit) != 0) {
    limit.rlim_cur = 0;
  }
#elif defined(RLIM_OFILE)
  if (getrlimit(RLIMIT_OFILE, &limit) != 0) {
    limit.rlim_cur = 0;
  }
#endif
#ifdef _SC_OPEN_MAX
  open_max = sysconf(_SC_OPEN_MAX);
#endif
  if (open_max <= 0) {
    open_max = 36; /* POSIX_OPEN_MAX (20) + some padding */
  }
  if (limit.rlim_cur == RLIM_INFINITY || limit.rlim_cur > INT_MAX) {
    // "no limit", which seems unlikely
    limit.rlim_cur = INT_MAX;
  }
  // Take the larger of the two values we compute
  if (limit.rlim_cur > (rlim_t)open_max) {
    open_max = limit.rlim_cur;
  }
  // Closing too many fds can be too slow. Limit the `open_max` to avoid slow
  // startup.
  const auto reasonable_fd_max =
      Configuration().getInt("reasonable_fd_max", 2500000);
  if (open_max > reasonable_fd_max) {
    open_max = reasonable_fd_max;
  }
  for (int fd = static_cast<int>(open_max); fd > STDERR_FILENO; --fd) {
    if (shouldCloseInheritedFd(fd)) {
      folly::fileops::close(fd);
    }
  }
#endif
}

} // namespace watchman
