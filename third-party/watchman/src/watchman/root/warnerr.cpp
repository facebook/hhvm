/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/root/warnerr.h"
#include "watchman/Errors.h"
#include "watchman/Logging.h"
#include "watchman/Poison.h"
#include "watchman/root/Root.h"

namespace watchman {

void handle_open_errno(
    Root& root,
    w_string_piece dirPath,
    std::chrono::system_clock::time_point now,
    const char* syscall,
    const std::error_code& err) {
  bool log_warning = true;

  if (err == error_code::no_such_file_or_directory ||
      err == error_code::not_a_directory ||
      err == error_code::too_many_symbolic_link_levels) {
    log_warning = false;
  } else if (err == error_code::permission_denied) {
    log_warning = true;
  } else if (err == error_code::system_limits_exceeded) {
    set_poison_state(dirPath, now, syscall, err);
    if (!root.failure_reason) {
      root.failure_reason = w_string::build(*poisoned_reason.rlock());
    }
    return;
  } else {
    log_warning = true;
  }

  if (dirPath == root.root_path) {
    auto warn = w_string::build(
        syscall,
        "(",
        dirPath,
        ") -> ",
        err.message(),
        ". Root is inaccessible; cancelling watch\n");
    log(ERR, warn);
    if (!root.failure_reason) {
      root.failure_reason = warn;
    }
    root.cancel("root inaccessible");
    return;
  }

  auto warn = w_string::build(
      syscall,
      "(",
      dirPath,
      ") -> ",
      err.message(),
      ". Marking this portion of the tree deleted");

  log(err == error_code::no_such_file_or_directory ? DBG : ERR, warn, "\n");
  if (log_warning) {
    root.recrawlInfo.wlock()->warning = warn;
  }
}

} // namespace watchman
