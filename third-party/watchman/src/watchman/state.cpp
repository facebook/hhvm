/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/state.h"
#include <folly/String.h>
#include <folly/Synchronized.h>
#include "watchman/Errors.h"
#include "watchman/Logging.h"
#include "watchman/Options.h"
#include "watchman/PDU.h"
#include "watchman/QueryableView.h"
#include "watchman/Shutdown.h"
#include "watchman/TriggerCommand.h"
#include "watchman/root/Root.h"
#include "watchman/root/resolve.h"
#include "watchman/root/watchlist.h"
#include "watchman/saved_state/SavedStateFactory.h"
#include "watchman/watchman_stream.h"

using namespace watchman;

/** The state saving thread is responsible for writing out the
 * persistent information about the users watches.
 * It runs in its own thread so that we avoid the possibility
 * of self deadlock if various threads were to immediately
 * save the state when things are changing.
 *
 * This uses a simple condition variable to wait for and be
 * notified of state changes.
 */

namespace {
struct state {
  bool needsSave{false};
};
folly::Synchronized<state, std::mutex> saveState;
std::condition_variable stateCond;
std::thread state_saver_thread;
} // namespace

static bool do_state_save();

static void state_saver() noexcept {
  bool do_save;

  w_set_thread_name("statesaver");

  while (!w_is_stopping()) {
    {
      auto state = saveState.lock();
      if (!state->needsSave) {
        stateCond.wait(state.as_lock());
      }
      do_save = state->needsSave;
      state->needsSave = false;
    }

    if (do_save) {
      do_state_save();
    }
  }
}

void w_state_shutdown() {
  if (flags.dont_save_state) {
    return;
  }

  stateCond.notify_one();
  state_saver_thread.join();
}

bool w_state_load() {
  if (flags.dont_save_state) {
    return true;
  }

  state_saver_thread = std::thread(state_saver);

  std::optional<json_ref> state;
  try {
    state = json_load_file(flags.watchman_state_file.c_str(), 0);
  } catch (const std::system_error& exc) {
    if (exc.code() == watchman::error_code::no_such_file_or_directory) {
      // No need to alarm anyone if we've never written a state file
      return false;
    }
    logf(
        ERR,
        "failed to load json from {}: {}\n",
        flags.watchman_state_file,
        folly::exceptionStr(exc).toStdString());
    return false;
  } catch (const std::exception& exc) {
    logf(
        ERR,
        "failed to parse json from {}: {}\n",
        flags.watchman_state_file,
        folly::exceptionStr(exc).toStdString());
    return false;
  }

  if (!w_root_load_state(state.value())) {
    return false;
  }

  return true;
}

static bool do_state_save() {
  PduBuffer buffer;

  auto state = json_object();

  auto file = w_stm_open(
      flags.watchman_state_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0600);
  if (!file) {
    log(ERR,
        "save_state: unable to open ",
        flags.watchman_state_file,
        " for write: ",
        folly::errnoStr(errno),
        "\n");
    return false;
  }

  state.set("version", typed_string_to_json(PACKAGE_VERSION, W_STRING_UNICODE));

  /* now ask the different subsystems to fill out the state */
  if (!w_root_save_state(state)) {
    return false;
  }

  /* we've prepared what we're going to save, so write it out */
  buffer.jsonEncodeToStream(state, file.get(), JSON_INDENT(4));
  return true;
}

/** Arranges for the state to be saved.
 * Does not immediately save the state. */
void w_state_save() {
  if (flags.dont_save_state) {
    return;
  }

  saveState.lock()->needsSave = true;
  stateCond.notify_one();
}

bool w_root_save_state(json_ref& state) {
  bool result = true;

  std::vector<json_ref> watched_dirs;

  logf(DBG, "saving state\n");

  {
    auto map = watched_roots.rlock();
    for (const auto& it : *map) {
      auto root = it.second;

      auto obj = json_object();

      json_object_set_new(obj, "path", w_string_to_json(root->root_path));

      auto triggers = root->triggerListToJson();
      json_object_set_new(obj, "triggers", std::move(triggers));

      watched_dirs.push_back(std::move(obj));
    }
  }

  json_object_set_new(state, "watched", json_array(std::move(watched_dirs)));

  return result;
}

bool w_root_load_state(const json_ref& state) {
  size_t i;

  auto watched = state.get_optional("watched");
  if (!watched) {
    return true;
  }

  if (!watched->isArray()) {
    return false;
  }

  for (i = 0; i < json_array_size(*watched); i++) {
    const auto& obj = watched->at(i);
    bool created = false;
    size_t j;

    auto triggers = obj.get("triggers");
    auto path = json_object_get(obj, "path");
    const char* filename = path ? json_string_value(*path) : nullptr;

    std::shared_ptr<Root> root;
    try {
      root = root_resolve(filename, true, &created);
    } catch (const std::exception&) {
      continue;
    }

    {
      auto wlock = root->triggers.wlock();
      auto& map = *wlock;

      /* re-create the trigger configuration */
      for (j = 0; j < json_array_size(triggers); j++) {
        const auto& tobj = triggers.at(j);

        // Legacy rules format
        auto rarray = tobj.get_optional("rules");
        if (rarray) {
          continue;
        }

        try {
          auto cmd = std::make_unique<TriggerCommand>(getInterface, root, tobj);
          cmd->start(root);
          auto& mapEntry = map[cmd->triggername];
          mapEntry = std::move(cmd);
        } catch (const std::exception& exc) {
          watchman::log(
              watchman::ERR,
              "loading trigger for ",
              root->root_path,
              ": ",
              exc.what(),
              "\n");
        }
      }
    }

    if (created) {
      try {
        root->view()->startThreads(root);
      } catch (const std::exception& e) {
        watchman::log(
            watchman::ERR,
            "root_start(",
            root->root_path,
            ") failed: ",
            e.what(),
            "\n");
        root->cancel(
            fmt::format("Error starting threads for root: {}", e.what()));
      }
    }
  }

  return true;
}

/* vim:ts=2:sw=2:et:
 */
