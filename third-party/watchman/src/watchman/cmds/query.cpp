/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/Query.h"
#include "watchman/Client.h"
#include "watchman/ClientContext.h"
#include "watchman/ProcessUtil.h"
#include "watchman/query/eval.h"
#include "watchman/query/parse.h"
#include "watchman/saved_state/SavedStateFactory.h"
#include "watchman/watchman_cmd.h"

using namespace watchman;

/* query /root {query} */
static UntypedResponse cmd_query(Client* client, const json_ref& args) {
  if (json_array_size(args) != 3) {
    throw ErrorResponse("wrong number of arguments for 'query', expected 3");
  }

  auto root = resolveRoot(client, args);

  const auto& query_spec = args.at(2);
  auto query = parseQuery(root, query_spec);
  auto clientPid = client->stm ? client->stm->getPeerProcessID() : 0;
  query->clientInfo.clientPid = clientPid;
  query->clientInfo.clientInfo = clientPid
      ? std::make_optional(lookupProcessInfo(clientPid))
      : std::nullopt;

  if (client->client_mode) {
    query->sync_timeout = std::chrono::milliseconds(0);
  }

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
    "query",
    cmd_query,
    CMD_DAEMON | CMD_CLIENT | CMD_ALLOW_ANY_USER,
    w_cmd_realpath_root);

/* vim:ts=2:sw=2:et:
 */
