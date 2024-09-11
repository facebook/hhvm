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

/* since /root <timestamp> [patterns] */
static UntypedResponse cmd_since(Client* client, const json_ref& args) {
  const char* clockspec;

  /* resolve the root */
  if (json_array_size(args) < 3) {
    throw ErrorResponse("not enough arguments for 'since'");
  }
  auto& arr = args.array();

  auto root = resolveRoot(client, args);

  auto clock_ele = arr[2];
  clockspec = json_string_value(clock_ele);
  if (!clockspec) {
    throw ErrorResponse("expected argument 2 to be a valid clockspec");
  }

  auto query = parseQueryLegacy(root, args, 3, nullptr, clockspec, nullptr);
  auto clientPid = client->stm ? client->stm->getPeerProcessID() : 0;
  query->clientInfo.clientPid = clientPid;
  query->clientInfo.clientInfo = clientPid
      ? std::make_optional(lookupProcessInfo(clientPid))
      : std::nullopt;

  auto res = w_query_execute(query.get(), root, nullptr, getInterface);
  UntypedResponse response;
  response.set(
      {{"is_fresh_instance", json_boolean(res.isFreshInstance)},
       {"clock", res.clockAtStartOfQuery.toJson()},
       {"files", std::move(res.resultsArray).toJson()},
       {"debug", res.debugInfo.render()}});
  if (res.savedStateInfo) {
    response.set("saved-state-info", std::move(*res.savedStateInfo));
  }

  add_root_warnings_to_response(response, root);
  return response;
}
W_CMD_REG(
    "since",
    cmd_since,
    CMD_DAEMON | CMD_ALLOW_ANY_USER,
    w_cmd_realpath_root);

/* vim:ts=2:sw=2:et:
 */
