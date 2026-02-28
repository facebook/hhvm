/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/PathUtils.h"

#include <fmt/core.h>
#include <folly/Exception.h>
#include <folly/String.h>
#include <folly/portability/Fcntl.h>
#include <folly/portability/Filesystem.h>
#include <folly/portability/SysStat.h>

#include "watchman/GroupLookup.h"
#include "watchman/Logging.h"
#include "watchman/UserDir.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/XattrUtils.h"
#include "watchman/fs/DirHandle.h"
#include "watchman/watchman_string.h"

#ifndef _WIN32
#include <folly/portability/Unistd.h>
#include <sys/stat.h>
#endif

namespace watchman {

void verify_dir_ownership(const std::string& state_dir) {
#ifndef _WIN32
  // verify ownership
  struct stat st{};
  int dir_fd;
  int ret = 0;
  uid_t euid = geteuid();
  // TODO: also allow a gid to be specified here
  const char* sock_group_name = cfg_get_string("sock_group", nullptr);
  const char* secondary_sock_group =
      cfg_get_string("secondary_sock_group", nullptr);
  // S_ISGID is set so that files inside this directory inherit the group
  // name
  mode_t dir_perms =
      cfg_get_perms(
          "sock_access", false /* write bits */, true /* execute bits */) |
      S_ISGID;

  auto dirp =
      openDir(state_dir.c_str(), false /* don't need strict symlink rules */);

  dir_fd = dirp->getFd();
  if (dir_fd == -1) {
    log(ERR, "dirfd(", state_dir, "): ", folly::errnoStr(errno), "\n");
    goto bail;
  }

  if (fstat(dir_fd, &st) != 0) {
    log(ERR, "fstat(", state_dir, "): ", folly::errnoStr(errno), "\n");
    ret = 1;
    goto bail;
  }
  if (euid != st.st_uid) {
    log(ERR,
        "the owner of ",
        state_dir,
        " is uid ",
        st.st_uid,
        " and doesn't match your euid ",
        euid,
        "\n");
    ret = 1;
    goto bail;
  }
  if (st.st_mode & 0022) {
    log(ERR,
        "the permissions on ",
        state_dir,
        " allow others to write to it. "
        "Verify that you own the contents and then fix its "
        "permissions by running `chmod 0700 '",
        state_dir,
        "'`\n");
    ret = 1;
    goto bail;
  }

  if (sock_group_name) {
    const struct group* sock_group = w_get_group(sock_group_name);
    if (!sock_group) {
      ret = 1;
      goto bail;
    }

    if (fchown(dir_fd, -1, sock_group->gr_gid) == -1) {
      log(ERR,
          "setting up group '",
          sock_group_name,
          "' failed: ",
          folly::errnoStr(errno),
          "\n");
      ret = 1;
      goto bail;
    }
  }

#ifdef __linux__
  // Allow setting an ACL on the state_dir, this method does not require the
  // owner to be a member of the target group.
  if (secondary_sock_group) {
    // TODO: pass mode_t dir_perms (from earlier in this function) to
    // setSecondaryGroupACL instead of using three booleans
    if (!watchman::setSecondaryGroupACL(
            state_dir.c_str(),
            secondary_sock_group,
            true /* read bits */,
            false /* write bits */,
            true /* execute bits */)) {
      ret = 1;
      goto bail;
    }
  }
#else
  (void)secondary_sock_group;
#endif

  // Depending on group and world accessibility, change permissions on the
  // directory. We can't leave the directory open and set permissions on the
  // socket because not all POSIX systems respect permissions on UNIX domain
  // sockets, but all POSIX systems respect permissions on the containing
  // directory.
  logf(DBG, "Setting permissions on state dir to {:o}\n", dir_perms);
  if (fchmod(dir_fd, dir_perms) == -1) {
    logf(
        ERR,
        "fchmod({}, {:o}): {}\n",
        state_dir,
        dir_perms,
        folly::errnoStr(errno));
    ret = 1;
    goto bail;
  }

bail:
  if (ret) {
    exit(ret);
  }
#else
  (void)state_dir;
#endif
}

void create_state_dir(const char* state_dir, std::error_code& ec) {
  ec.clear();
  // folly::fs::create_directories errors in the case where the state_dir is a
  // symlink, so explicitly check for the existence first and gate the creation
  // on this check.
  auto exists = folly::fs::exists(state_dir, ec);
  if (ec) {
    return;
  }

  if (!exists) {
    auto created = folly::fs::create_directories(state_dir, ec);

    // If create_directories() failed, it will set ec, so just return
    // so the caller can handle the error.
    if (ec) {
      return;
    }

    if (created) {
#ifndef _WIN32
      // Ignore the result here as it doesn't matter, we will check the actual
      // permissions in verify_dir_ownership
      chmod(state_dir, 0700);
#endif
    }
  }

  // Verify the permissions on both pre-existing and newly created directories
  verify_dir_ownership(state_dir);
}

void create_log_dir(const char* log_dir, std::error_code& ec) {
  ec.clear();
  // Handle symlinks: check existence first, same pattern as create_state_dir
  // (folly::fs::create_directories errors on symlinks)
  auto exists = folly::fs::exists(log_dir, ec);
  if (ec) {
    return;
  }

  if (!exists) {
    // Only create the leaf directory — the parent (log_dir config value)
    // must already exist (e.g. set up by the container runtime or system).
    // Using create_directory (not create_directories) so that a misconfigured
    // log_dir path fails clearly instead of silently creating a deep tree.
    folly::fs::create_directory(log_dir, ec);
    if (ec) {
      return;
    }

#ifndef _WIN32
    chmod(log_dir, 0755);
#endif
  }
}

void compute_file_name(
    std::string& str,
    const std::string& user,
    const char* suffix,
    const char* what,
    bool require_absolute) {
  bool str_computed = false;
  if (str.empty()) {
    str_computed = true;
    /* We'll put our various artifacts in a user specific dir
     * within the state dir location */
    auto state_dir = computeWatchmanStateDirectory(user);

    std::error_code ec;
    create_state_dir(state_dir.c_str(), ec);
    if (ec) {
      log(ERR,
          "while computing ",
          what,
          ": failed to create ",
          state_dir,
          ": ",
          ec.message(),
          "\n");
      exit(1);
    }

    str = fmt::format("{}/{}", state_dir, suffix);
  }
#ifndef _WIN32
  if (require_absolute && !w_string_piece(str).pathIsAbsolute()) {
    log(FATAL,
        what,
        " must be an absolute file path but ",
        str,
        " was",
        str_computed ? " computed." : " provided.",
        "\n");
  }
#else
  (void)require_absolute;
#endif
}

} // namespace watchman
