/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/ScopeGuard.h>
#include "watchman/Client.h"
#include "watchman/Errors.h"
#include "watchman/Logging.h"
#include "watchman/Shutdown.h"
#include "watchman/root/Root.h"
#include "watchman/root/resolve.h"
#include "watchman/watchman_cmd.h"

// Functions relating to the per-user service

using namespace watchman;

static UntypedResponse cmd_shutdown(Client*, const json_ref&) {
  logf(ERR, "shutdown-server was requested, exiting!\n");
  w_request_shutdown();

  UntypedResponse resp;
  resp.set("shutdown-server", json_true());
  return resp;
}
W_CMD_REG(
    "shutdown-server",
    cmd_shutdown,
    CMD_DAEMON | CMD_POISON_IMMUNE,
    nullptr);

void add_root_warnings_to_response(
    UntypedResponse& response,
    const std::shared_ptr<Root>& root) {
  auto info = root->recrawlInfo.rlock();

  if (!info->warning) {
    return;
  }

  response.set(
      "warning",
      w_string_to_json(w_string::build(
          info->warning.value(),
          "\n",
          "To clear this warning, run:\n"
          "`watchman watch-del '",
          root->root_path,
          "' ; watchman watch-project '",
          root->root_path,
          "'`\n")));
}

std::shared_ptr<Root>
doResolveOrCreateRoot(Client* client, const json_ref& args, bool create) {
  // Assume root is first element
  size_t root_index = 1;
  if (args.array().size() <= root_index) {
    throw RootResolveError("wrong number of arguments");
  }
  const auto& ele = args.at(root_index);

  const char* root_name = json_string_value(ele);
  if (!root_name) {
    RootResolveError::throwf(
        "invalid value for argument {}, expected a string naming the root dir",
        root_index);
  }
  return resolveRootByName(client, root_name, create);
}

std::shared_ptr<Root>
resolveRootByName(Client* client, const char* rootName, bool create) {
  try {
    std::shared_ptr<Root> root;
    if (client->client_mode) {
      root = w_root_resolve_for_client_mode(rootName);
    } else {
      if (!client->client_is_owner) {
        // Only the owner is allowed to create watches
        create = false;
      }
      root = w_root_resolve(rootName, create);
    }

    if (client->perf_sample) {
      root->addPerfSampleMetadata(*client->perf_sample);
    }
    return root;

  } catch (const std::exception& exc) {
    RootResolveError::throwf(
        "unable to resolve root {}: {}{}",
        rootName,
        exc.what(),
        client->client_is_owner
            ? ""
            : " (this may be because you are not the process owner)");
  }
}

std::shared_ptr<Root> resolveRoot(Client* client, const json_ref& args) {
  return doResolveOrCreateRoot(client, args, false);
}

std::shared_ptr<Root> resolveOrCreateRoot(
    Client* client,
    const json_ref& args) {
  return doResolveOrCreateRoot(client, args, true);
}

/* vim:ts=2:sw=2:et:
 */
