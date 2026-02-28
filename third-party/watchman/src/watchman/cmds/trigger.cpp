/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Client.h"
#include "watchman/CommandRegistry.h"
#include "watchman/TriggerCommand.h"
#include "watchman/query/Query.h"
#include "watchman/query/parse.h"
#include "watchman/root/Root.h"
#include "watchman/saved_state/SavedStateFactory.h"
#include "watchman/state.h"
#include "watchman/watchman_cmd.h"

#include <memory>

using namespace watchman;

/* trigger-del /root triggername
 * Delete a trigger from a root
 */
static UntypedResponse cmd_trigger_delete(
    Client* client,
    const json_ref& args) {
  w_string tname;
  bool res;

  auto root = resolveRoot(client, args);

  if (json_array_size(args) != 3) {
    throw ErrorResponse("wrong number of arguments");
  }
  auto jname = args.at(2);
  if (!jname.isString()) {
    throw ErrorResponse("expected 2nd parameter to be trigger name");
  }
  tname = json_to_w_string(jname);

  std::unique_ptr<TriggerCommand> cmd;

  {
    auto map = root->triggers.wlock();
    auto it = map->find(tname);
    if (it == map->end()) {
      res = false;
    } else {
      std::swap(cmd, it->second);
      map->erase(it);
      res = true;
    }
  }

  if (cmd) {
    // Stop the thread
    cmd->stop();
  }

  if (res) {
    w_state_save();
  }

  UntypedResponse resp;
  resp.set({{"deleted", json_boolean(res)}, {"trigger", json_ref(jname)}});
  return resp;
}
W_CMD_REG("trigger-del", cmd_trigger_delete, CMD_DAEMON, w_cmd_realpath_root);

/* trigger-list /root
 * Displays a list of registered triggers for a given root
 */
static UntypedResponse cmd_trigger_list(Client* client, const json_ref& args) {
  auto root = resolveRoot(client, args);

  UntypedResponse resp;
  auto arr = root->triggerListToJson();

  resp.set("triggers", std::move(arr));
  return resp;
}
W_CMD_REG("trigger-list", cmd_trigger_list, CMD_DAEMON, w_cmd_realpath_root);

static json_ref build_legacy_trigger(
    const std::shared_ptr<Root>& root,
    const json_ref& args) {
  uint32_t next_arg = 0;
  uint32_t i;
  size_t n;

  auto trig = json_object(
      {{"name", args.at(2)},
       {"append_files", json_true()},
       {"stdin",
        json_array(
            {typed_string_to_json("name"),
             typed_string_to_json("exists"),
             typed_string_to_json("new"),
             typed_string_to_json("size"),
             typed_string_to_json("mode")})}});

  json_ref expr = json_null();
  auto query = parseQueryLegacy(root, args, 3, &next_arg, nullptr, &expr);
  query->request_id = w_string::build("trigger ", json_to_w_string(args.at(2)));

  json_object_set(
      trig,
      "expression",
      expr.get_optional("expression").value_or(json_null()));

  if (next_arg >= args.array().size()) {
    throw ErrorResponse("no command was specified");
  }

  n = json_array_size(args) - next_arg;
  std::vector<json_ref> command;
  command.reserve(n);
  for (i = 0; i < n; i++) {
    auto ele = args.at(i + next_arg);
    if (!ele.isString()) {
      throw ErrorResponse("expected argument {} to be a string", i);
    }
    command.push_back(std::move(ele));
  }
  json_object_set_new(trig, "command", json_array(std::move(command)));

  return trig;
}

/* trigger /root triggername [watch patterns] -- cmd to run
 * Sets up a trigger so that we can execute a command when a change
 * is detected */
static UntypedResponse cmd_trigger(Client* client, const json_ref& args) {
  bool need_save = true;
  std::unique_ptr<TriggerCommand> cmd;

  auto root = resolveRoot(client, args);

  if (json_array_size(args) < 3) {
    throw ErrorResponse("not enough arguments");
  }

  json_ref trig = args.at(2);
  if (trig.isString()) {
    trig = build_legacy_trigger(root, args);
  }

  cmd = std::make_unique<TriggerCommand>(getInterface, root, trig);

  UntypedResponse resp;
  resp.set("triggerid", w_string_to_json(cmd->triggername));

  {
    auto wlock = root->triggers.wlock();
    auto& map = *wlock;
    auto& old = map[cmd->triggername];

    if (old && json_equal(cmd->definition, old->definition)) {
      // Same definition: we don't and shouldn't touch things, so that we
      // preserve the associated trigger clock and don't cause the trigger
      // to re-run immediately
      resp.set(
          "disposition",
          typed_string_to_json("already_defined", W_STRING_UNICODE));
      need_save = false;
    } else {
      resp.set(
          "disposition",
          typed_string_to_json(old ? "replaced" : "created", W_STRING_UNICODE));
      if (old) {
        // If we're replacing an old definition, be sure to stop the old
        // one before we destroy it, and before we start the new one.
        old->stop();
      }
      // Start the new trigger thread
      cmd->start(root);
      old = std::move(cmd);

      need_save = true;
    }
  }

  if (need_save) {
    w_state_save();
  }

  return resp;
}
W_CMD_REG("trigger", cmd_trigger, CMD_DAEMON, w_cmd_realpath_root);

/* vim:ts=2:sw=2:et:
 */
