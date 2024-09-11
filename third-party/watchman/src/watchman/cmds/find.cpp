/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Client.h"
#include "watchman/ClientContext.h"
#include "watchman/ProcessUtil.h"
#include "watchman/query/Query.h"
#include "watchman/query/eval.h"
#include "watchman/query/parse.h"
#include "watchman/saved_state/SavedStateFactory.h"
#include "watchman/watchman_cmd.h"

using namespace watchman;

/* find /root [patterns] */
static UntypedResponse cmd_find(Client* client, const json_ref& args) {
  /* resolve the root */
  if (json_array_size(args) < 2) {
    throw ErrorResponse("not enough arguments for 'find'");
  }

  auto root = resolveRoot(client, args);

  auto query = parseQueryLegacy(root, args, 2, nullptr, nullptr, nullptr);
  if (client->client_mode) {
    query->sync_timeout = std::chrono::milliseconds(0);
  }
  auto clientPid = client->stm ? client->stm->getPeerProcessID() : 0;
  query->clientInfo.clientPid = clientPid;
  query->clientInfo.clientInfo = clientPid
      ? std::make_optional(lookupProcessInfo(clientPid))
      : std::nullopt;

  auto res = w_query_execute(query.get(), root, nullptr, getInterface);
  UntypedResponse response;
  response.set(
      {{"clock", res.clockAtStartOfQuery.toJson()},
       {"files", std::move(res.resultsArray).toJson()}});

  return response;
}
W_CMD_REG(
    "find",
    cmd_find,
    CMD_DAEMON | CMD_ALLOW_ANY_USER,
    w_cmd_realpath_root);

/* vim:ts=2:sw=2:et:
 */
