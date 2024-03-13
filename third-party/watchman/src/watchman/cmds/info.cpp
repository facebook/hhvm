/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <map>
#include <optional>
#include <set>

#include <folly/String.h>

#include "watchman/Client.h"
#include "watchman/CommandRegistry.h"
#include "watchman/Logging.h"
#include "watchman/Serde.h"
#include "watchman/root/Root.h"
#include "watchman/root/resolve.h"
#include "watchman/sockname.h"
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/watchman_cmd.h"
#include "watchman/watchman_stream.h"

using namespace watchman;

namespace {

class VersionCommand : public PrettyCommand<VersionCommand> {
 public:
  static constexpr std::string_view name = "version";

  static constexpr CommandFlags flags =
      CMD_DAEMON | CMD_CLIENT | CMD_ALLOW_ANY_USER;

  struct RequestOptions : serde::Object {
    std::vector<w_string> required;
    std::vector<w_string> optional;

    template <typename X>
    void map(X& x) {
      x.skip_if_default("required", required);
      x.skip_if_default("optional", optional);
    }
  };

  using Request = serde::Array<0, RequestOptions>;

  struct Response : BaseResponse {
    std::optional<w_string> buildinfo;
    std::map<w_string, bool> capabilities;
    std::optional<w_string> error;

    template <typename X>
    void map(X& x) {
      BaseResponse::map(x);
      x.skip_if_default("buildinfo", buildinfo);
      x.skip_if_default("capabilities", capabilities);
      x.skip_if_default("error", error);
    }
  };

  static Response handle(Client*, const Request& request) {
    Response response;

#ifdef WATCHMAN_BUILD_INFO
    response.buildinfo = w_string{WATCHMAN_BUILD_INFO, W_STRING_UNICODE};
#endif

    auto& options = std::get<0>(request);

    if (!options.required.empty() || !options.optional.empty()) {
      for (const auto& capname : options.optional) {
        response.capabilities[capname] = capability_supported(capname.view());
      }

      std::set<w_string> missing;

      for (const auto& capname : options.required) {
        bool have = capability_supported(capname.view());
        response.capabilities[capname] = have;
        if (!have) {
          missing.insert(capname);
        }
      }

      if (!missing.empty()) {
        response.error = w_string::build(
            "client required capabilities [",
            fmt::join(missing, ", "),
            "] not supported by this server");
      }
    }

    return response;
  }

  static void printResult(const Response& response) {
    if (response.error) {
      fmt::print("error: {}\n", response.error.value());
    }
    fmt::print("version: {}\n", response.version);
    if (response.buildinfo) {
      fmt::print("buildinfo: {}\n", response.buildinfo.value());
    }
    // fmt does not flush, so when the stream is not line buffered the stream
    // needs to be manually flushed (or else nothing is written to stdout).
    // eventually this can be fmt::flush instead:
    // https://github.com/vgc/vgc/issues/519
    // TODO(T136788014): why doesn't macOS do this for us.
    fflush(stdout);
  }
};

WATCHMAN_COMMAND(version, VersionCommand);

/* list-capabilities */
static UntypedResponse cmd_list_capabilities(Client*, const json_ref&) {
  UntypedResponse resp;
  resp.set("capabilities", capability_get_list());
  return resp;
}
W_CMD_REG(
    "list-capabilities",
    cmd_list_capabilities,
    CMD_DAEMON | CMD_CLIENT | CMD_ALLOW_ANY_USER,
    nullptr);

/* get-sockname */
static UntypedResponse cmd_get_sockname(Client*, const json_ref&) {
  UntypedResponse resp;

  // For legacy reasons we report the unix domain socket as sockname on
  // unix but the named pipe path on windows
  resp.set(
      "sockname",
      w_string_to_json(w_string(get_sock_name_legacy(), W_STRING_BYTE)));
  if (!disable_unix_socket) {
    resp.set(
        "unix_domain", w_string_to_json(w_string::build(get_unix_sock_name())));
  }

#ifdef _WIN32
  if (!disable_named_pipe) {
    resp.set(
        "named_pipe",
        w_string_to_json(w_string::build(get_named_pipe_sock_path())));
  }
#endif

  return resp;
}
W_CMD_REG(
    "get-sockname",
    cmd_get_sockname,
    CMD_DAEMON | CMD_CLIENT | CMD_ALLOW_ANY_USER,
    nullptr);

static UntypedResponse cmd_get_config(Client* client, const json_ref& args) {
  if (json_array_size(args) != 2) {
    throw ErrorResponse("wrong number of arguments for 'get-config'");
  }

  auto root = resolveRoot(client, args);

  UntypedResponse resp;

  std::optional<json_ref> config = root->config_file;
  if (!config) {
    config = json_object();
  }

  resp.set("config", std::move(*config));
  return resp;
}
W_CMD_REG("get-config", cmd_get_config, CMD_DAEMON, w_cmd_realpath_root);

} // namespace

/* vim:ts=2:sw=2:et:
 */
