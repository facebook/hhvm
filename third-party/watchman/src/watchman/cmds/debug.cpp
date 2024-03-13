/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <unordered_map>

#include <fmt/chrono.h>

#include <folly/String.h>
#include <folly/chrono/Conv.h>
#include <folly/system/Shell.h>

#include "watchman/Client.h"
#include "watchman/InMemoryView.h"
#include "watchman/LRUCache.h"
#include "watchman/Logging.h"
#include "watchman/Poison.h"
#include "watchman/QueryableView.h"
#include "watchman/root/Root.h"
#include "watchman/watchman_cmd.h"

namespace watchman {
namespace {

static UntypedResponse cmd_debug_recrawl(Client* client, const json_ref& args) {
  /* resolve the root */
  if (json_array_size(args) != 2) {
    throw ErrorResponse("wrong number of arguments for 'debug-recrawl'");
  }

  auto root = resolveRoot(client, args);

  UntypedResponse resp;

  root->scheduleRecrawl("debug-recrawl");

  resp.set("recrawl", json_true());
  return resp;
}
W_CMD_REG("debug-recrawl", cmd_debug_recrawl, CMD_DAEMON, w_cmd_realpath_root);

static UntypedResponse cmd_debug_show_cursors(
    Client* client,
    const json_ref& args) {
  /* resolve the root */
  if (json_array_size(args) != 2) {
    throw ErrorResponse("wrong number of arguments for 'debug-show-cursors'");
  }

  auto root = resolveRoot(client, args);

  UntypedResponse resp;

  auto map = root->inner.cursors.rlock();
  std::unordered_map<w_string, json_ref> cursors;
  cursors.reserve(map->size());
  for (const auto& it : *map) {
    const auto& name = it.first;
    const auto& ticks = it.second;
    cursors.insert_or_assign(name, json_integer(ticks));
  }

  resp.set("cursors", json_object(std::move(cursors)));
  return resp;
}
W_CMD_REG(
    "debug-show-cursors",
    cmd_debug_show_cursors,
    CMD_DAEMON,
    w_cmd_realpath_root);

/* debug-ageout */
static UntypedResponse cmd_debug_ageout(Client* client, const json_ref& args) {
  /* resolve the root */
  if (json_array_size(args) != 3) {
    throw ErrorResponse("wrong number of arguments for 'debug-ageout'");
  }

  auto root = resolveRoot(client, args);

  std::chrono::seconds min_age(args.array()[2].asInt());

  UntypedResponse resp;
  root->performAgeOut(min_age);

  resp.set("ageout", json_true());
  return resp;
}
W_CMD_REG("debug-ageout", cmd_debug_ageout, CMD_DAEMON, w_cmd_realpath_root);

static UntypedResponse cmd_debug_poison(Client* client, const json_ref& args) {
  auto root = resolveRoot(client, args);

  auto now = std::chrono::system_clock::now();

  set_poison_state(
      root->root_path,
      now,
      "debug-poison",
      std::error_code(ENOMEM, std::generic_category()));

  UntypedResponse resp;
  resp.set(
      "poison",
      typed_string_to_json(poisoned_reason.rlock()->c_str(), W_STRING_UNICODE));
  return resp;
}
W_CMD_REG("debug-poison", cmd_debug_poison, CMD_DAEMON, w_cmd_realpath_root);

static UntypedResponse cmd_debug_drop_privs(Client* client, const json_ref&) {
  client->client_is_owner = false;

  UntypedResponse resp;
  resp.set("owner", json_boolean(client->client_is_owner));
  return resp;
}
W_CMD_REG("debug-drop-privs", cmd_debug_drop_privs, CMD_DAEMON, nullptr);

struct DebugSetParallelCrawlCommand
    : TypedCommand<DebugSetParallelCrawlCommand> {
  static constexpr std::string_view name = "debug-set-parallel-crawl";
  static constexpr CommandFlags flags = CMD_DAEMON;

  using Request = serde::Array<2, w_string, bool>;

  struct Response : BaseResponse {
    bool enabled;

    template <typename X>
    void map(X& x) {
      BaseResponse::map(x);
      x("enable_parallel_crawl", enabled);
    }
  };

  static Response handle(Client* client, const Request& req) {
    Response res;
    res.version = w_string{PACKAGE_VERSION, W_STRING_UNICODE};
    bool create = true;
    auto root = resolveRootByName(client, std::get<0>(req).c_str(), create);
    bool enabled = std::get<1>(req);
    root->enable_parallel_crawl.store(enabled, std::memory_order_release);
    res.enabled = enabled;
    return res;
  }
};
WATCHMAN_COMMAND(debug_set_parallel_command, DebugSetParallelCrawlCommand);

static UntypedResponse cmd_debug_set_subscriptions_paused(
    Client* clientbase,
    const json_ref& args) {
  auto client = (UserClient*)clientbase;

  const auto& paused = args.at(1);
  auto& paused_map = paused.object();
  for (auto& it : paused_map) {
    auto sub_iter = client->subscriptions.find(it.first);
    if (sub_iter == client->subscriptions.end()) {
      throw ErrorResponse(
          "this client does not have a subscription named '{}'", it.first);
    }
    if (!it.second.isBool()) {
      throw ErrorResponse(
          "new value for subscription '{}' not a boolean", it.first);
    }
  }

  auto states = json_object();

  for (auto& it : paused_map) {
    auto sub_iter = client->subscriptions.find(it.first);
    bool old_paused = sub_iter->second->debug_paused;
    bool new_paused = it.second.asBool();
    sub_iter->second->debug_paused = new_paused;
    states.set(
        it.first,
        json_object({{"old", json_boolean(old_paused)}, {"new", it.second}}));
  }

  UntypedResponse resp;
  resp.set("paused", std::move(states));
  return resp;
}
W_CMD_REG(
    "debug-set-subscriptions-paused",
    cmd_debug_set_subscriptions_paused,
    CMD_DAEMON,
    nullptr);

static json_ref getDebugSubscriptionInfo(Root* root) {
  std::vector<json_ref> subscriptions;
  for (const auto& user_client : UserClient::getAllClients()) {
    for (const auto& sub : user_client->subscriptions) {
      if (root == sub.second->root.get()) {
        std::vector<json_ref> last_responses;
        for (auto& response : sub.second->lastResponses) {
          char timebuf[64];
          last_responses.push_back(json_object({
              {"written_time",
               typed_string_to_json(Log::timeString(
                   timebuf,
                   std::size(timebuf),
                   folly::to<timeval>(response.written)))},
              {"response", response.response},
          }));
        }

        subscriptions.push_back(json_object({
            {"name", w_string_to_json(sub.first)},
            {"client_id", json_integer(user_client->unique_id)},
            {"last_responses", json_array(std::move(last_responses))},
        }));
      }
    }
  }
  return json_array(std::move(subscriptions));
}

static UntypedResponse cmd_debug_get_subscriptions(
    Client* clientbase,
    const json_ref& args) {
  auto client = (UserClient*)clientbase;

  auto root = resolveRoot(client, args);

  UntypedResponse resp;
  auto debug_info = root->unilateralResponses->getDebugInfo();
  // copy over all the key-value pairs from debug_info
  resp.insert(debug_info.object().begin(), debug_info.object().end());

  auto subscriptions = getDebugSubscriptionInfo(root.get());
  resp.emplace("subscriptions", subscriptions);

  return resp;
}
W_CMD_REG(
    "debug-get-subscriptions",
    cmd_debug_get_subscriptions,
    CMD_DAEMON,
    w_cmd_realpath_root);

static UntypedResponse cmd_debug_get_asserted_states(
    Client* clientbase,
    const json_ref& args) {
  auto client = (UserClient*)clientbase;

  auto root = resolveRoot(client, args);
  UntypedResponse response;

  // copy over all the key-value pairs to stateSet and release lock
  auto states = root->assertedStates.rlock()->debugStates();
  response.set(
      {{"root", w_string_to_json(root->root_path)},
       {"states", std::move(states)}});
  return response;
}
W_CMD_REG(
    "debug-get-asserted-states",
    cmd_debug_get_asserted_states,
    CMD_DAEMON,
    w_cmd_realpath_root);

namespace {

std::string shellQuoteCommand(std::string_view command) {
  std::vector<std::string> argv;
  folly::split('\0', command, argv);

  // Every argument in a command line is null-terminated. Remove the last, empty
  // argument.
  if (argv.size() && argv.back().empty()) {
    argv.pop_back();
  }

  for (auto& arg : argv) {
    // TODO: shellQuote is not particularly good. It always brackets with ' and
    // does not handle non-printable characters. We should write our own.
    arg = folly::shellQuote(arg);
  }
  return folly::join(' ', argv);
}

} // namespace

struct DebugStatusCommand : PrettyCommand<DebugStatusCommand> {
  static constexpr std::string_view name = "debug-status";

  static constexpr CommandFlags flags = CMD_DAEMON | CMD_ALLOW_ANY_USER;

  using Request = serde::Array<0>;

  struct Response : BaseResponse {
    std::vector<RootDebugStatus> roots;
    std::vector<ClientDebugStatus> clients;

    template <typename X>
    void map(X& x) {
      BaseResponse::map(x);
      x("roots", roots);
      x("clients", clients);
    }
  };

  static Response handle(Client*, const Request&) {
    Response res;
    res.version = w_string{PACKAGE_VERSION, W_STRING_UNICODE};
    res.roots = Root::getStatusForAllRoots();
    res.clients = UserClient::getStatusForAllClients();
    return res;
  }

  static void printResult(const Response& response) {
    fmt::print("ROOTS\n-----\n");
    for (auto& root : response.roots) {
      fmt::print("{}\n", root.path);
      if (root.cancelled) {
        fmt::print("  - cancelled: true\n");
      }
      fmt::print("  - fstype: {}\n", root.fstype);
      if (!root.watcher.empty()) {
        fmt::print("  - watcher: {}\n", root.watcher);
      }
      fmt::print("  - uptime: {} s\n", root.uptime);
      fmt::print("  - crawl_status: {}\n", root.crawl_status);
      fmt::print("  - done_initial: {}\n", root.done_initial);
      fmt::print("\n");
    }

    fmt::print("CLIENTS\n-------\n");
    for (auto& client : response.clients) {
      if (client.peer) {
        fmt::print(
            "{}: {}\n", client.peer->pid, shellQuoteCommand(client.peer->name));
      } else {
        fmt::print("unknown peer\n");
      }
      if (client.since) {
        fmt::print(
            "  - since: {}\n",
            std::chrono::system_clock::from_time_t(client.since.value()));
      }
      fmt::print("  - state: {}\n", client.state);
      fmt::print("\n");
    }
    // fmt does not flush, so when the stream is not line buffered the stream
    // needs to be manually flushed (or else nothing is written to stdout).
    // eventually this can be fmt::flush instead:
    // https://github.com/vgc/vgc/issues/519
    // TODO(T136788014): why doesn't macOS do this for us.
    fflush(stdout);
  }
};
WATCHMAN_COMMAND(debug_status, DebugStatusCommand);

struct DebugRootStatusCommand : TypedCommand<DebugRootStatusCommand> {
  static constexpr std::string_view name = "debug-root-status";
  static constexpr CommandFlags flags = CMD_DAEMON;

  using Request = serde::Array<1, w_string>;

  struct Response : BaseResponse {
    RootDebugStatus status;

    template <typename X>
    void map(X& x) {
      BaseResponse::map(x);
      x("root_status", status);
    }
  };

  static Response handle(Client* client, const Request& req) {
    Response res;
    res.version = w_string{PACKAGE_VERSION, W_STRING_UNICODE};
    auto root = resolveRootByName(client, std::get<0>(req).c_str());
    res.status = root->getStatus();
    return res;
  }
};
WATCHMAN_COMMAND(debug_root_status, DebugRootStatusCommand);

static UntypedResponse cmd_debug_watcher_info(
    Client* clientbase,
    const json_ref& args) {
  auto* client = static_cast<UserClient*>(clientbase);

  auto root = resolveRoot(client, args);
  UntypedResponse response;
  response.set("watcher-debug-info", root->view()->getWatcherDebugInfo());
  return response;
}
W_CMD_REG("debug-watcher-info", cmd_debug_watcher_info, CMD_DAEMON, nullptr);

static UntypedResponse cmd_debug_watcher_info_clear(
    Client* clientbase,
    const json_ref& args) {
  auto* client = static_cast<UserClient*>(clientbase);

  auto root = resolveRoot(client, args);
  UntypedResponse response;
  root->view()->clearWatcherDebugInfo();
  return response;
}
W_CMD_REG(
    "debug-watcher-info-clear",
    cmd_debug_watcher_info_clear,
    CMD_DAEMON,
    nullptr);

void addCacheStats(UntypedResponse& resp, const CacheStats& stats) {
  resp.set(
      {{"cacheHit", json_integer(stats.cacheHit)},
       {"cacheShare", json_integer(stats.cacheShare)},
       {"cacheMiss", json_integer(stats.cacheMiss)},
       {"cacheEvict", json_integer(stats.cacheEvict)},
       {"cacheStore", json_integer(stats.cacheStore)},
       {"cacheLoad", json_integer(stats.cacheLoad)},
       {"cacheErase", json_integer(stats.cacheErase)},
       {"clearCount", json_integer(stats.clearCount)},
       {"size", json_integer(stats.size)}});
}

UntypedResponse debugContentHashCache(Client* client, const json_ref& args) {
  /* resolve the root */
  if (json_array_size(args) != 2) {
    throw ErrorResponse("wrong number of arguments for 'debug-contenthash'");
  }

  auto root = resolveRoot(client, args);

  auto view = std::dynamic_pointer_cast<InMemoryView>(root->view());
  if (!view) {
    throw ErrorResponse("root is not an InMemoryView watcher");
  }

  auto stats = view->debugAccessCaches().contentHashCache.stats();
  UntypedResponse resp;
  addCacheStats(resp, stats);
  return resp;
}
W_CMD_REG(
    "debug-contenthash",
    debugContentHashCache,
    CMD_DAEMON,
    w_cmd_realpath_root);

UntypedResponse debugSymlinkTargetCache(Client* client, const json_ref& args) {
  /* resolve the root */
  if (json_array_size(args) != 2) {
    throw ErrorResponse(
        "wrong number of arguments for 'debug-symlink-target-cache'");
  }

  auto root = resolveRoot(client, args);

  auto view = std::dynamic_pointer_cast<InMemoryView>(root->view());
  if (!view) {
    throw ErrorResponse("root is not an InMemoryView watcher");
  }

  auto stats = view->debugAccessCaches().symlinkTargetCache.stats();
  UntypedResponse resp;
  addCacheStats(resp, stats);
  return resp;
}
W_CMD_REG(
    "debug-symlink-target-cache",
    debugSymlinkTargetCache,
    CMD_DAEMON,
    w_cmd_realpath_root);

} // namespace
} // namespace watchman
