/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/GroupLookup.h"
#include <folly/String.h>
#include "watchman/Logging.h"

#ifndef _WIN32
#include <errno.h>
#include <grp.h>
#include <string.h>
#include <sys/types.h>
#endif

#ifndef _WIN32
using namespace watchman;

const struct group* w_get_group(const char* group_name) {
  // This explicit errno statement is necessary to distinguish between the
  // group not existing and an error.
  errno = 0;
  struct group* group = getgrnam(group_name);
  if (!group) {
    if (errno == 0) {
      logf(ERR, "group '{}' does not exist\n", group_name);
    } else {
      logf(
          ERR,
          "getting gid for '{}' failed: {}\n",
          group_name,
          folly::errnoStr(errno));
    }
    return nullptr;
  }
  return group;
}
#endif // ndef _WIN32
