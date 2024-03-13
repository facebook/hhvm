/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/core.h>
#include <folly/String.h>
#include <folly/memory/Malloc.h>
#include "watchman/Client.h"
#include "watchman/watchman_cmd.h"

using namespace watchman;

#if defined(FOLLY_USE_JEMALLOC)

// This command is present to manually trigger a  heap profile dump when
// jemalloc is in use.
static UntypedResponse cmd_debug_prof_dump(Client*, const json_ref&) {
  if (!folly::usingJEMalloc()) {
    throw std::runtime_error("jemalloc is not in use");
  }

  auto result = mallctl("prof.dump", nullptr, nullptr, nullptr, 0);
  UntypedResponse resp;
  resp.set(
      "prof.dump",
      w_string_to_json(
          fmt::format("mallctl prof.dump returned: {}", folly::errnoStr(result))
              .c_str()));
  return resp;
}
W_CMD_REG("debug-prof-dump", cmd_debug_prof_dump, CMD_DAEMON, nullptr);

#endif
