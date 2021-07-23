/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/watchman-connection.h"

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/trace.h"
#include "hphp/util/user-info.h"

#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/Singleton.h>

TRACE_SET_MOD(watchman);

namespace HPHP {

namespace {

/**
 * Discover who owns the given repo and return the Watchman socket
 * corresponding to that user.
 */
Optional<std::string> find_user_socket(const folly::fs::path& repoRoot) {
  auto def = [] () -> Optional<std::string> {
    FTRACE(3, "No per-user watchman socket found.\n");
    if (RO::WatchmanDefaultSocket.empty()) return {};
    return RO::WatchmanDefaultSocket;
  };
  int repoRootFD = ::open(repoRoot.native().c_str(), O_DIRECTORY | O_RDONLY);
  if (repoRootFD == -1) return def();
  SCOPE_EXIT { ::close(repoRootFD); };

  struct ::stat hstat {};
  if (::fstat(repoRootFD, &hstat) != 0) return def();

  // The repo is owned by root, so use a special root socket
  if (hstat.st_uid == 0) {
    auto const& rootSock = RO::WatchmanRootSocket;
    FTRACE(
        3,
        "{} is owned by root, looking for socket at {}\n",
        repoRoot.native(),
        rootSock.empty() ? rootSock : "<none>");
    return rootSock.empty() ? def() : rootSock;
  }

  // Find the `watchman.socket` setting in the repo owner's
  // SandboxConfFile (usually a .hphp file somewhere in their home
  // directory).
  UserInfo info{hstat.st_uid};
  auto user = std::string{info.pw->pw_name};
  auto homePath = RO::GetHomePath(user);
  if (!homePath) return def();

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;

  auto confFileName = (*homePath) / RO::SandboxConfFile;
  if (!RO::ReadPerUserSettings(confFileName, ini, config)) return def();

  auto sock = Config::GetString(ini, config, "watchman.socket.default");
  return sock.empty() ? def() : sock;
}

using WatchmanCache = folly::ConcurrentHashMap<
  std::string,
  std::shared_ptr<Watchman>
>;

folly::Singleton<WatchmanCache> s_watchmanClients;

////////////////////////////////////////////////////////////////////////////////
}

Watchman& get_watchman_client(const folly::fs::path& root) {
  auto const cache = s_watchmanClients.try_get();
  always_assert(cache);

  auto it = cache->find(root.native());
  if (it != cache->end()) return *it->second;

  return *cache->insert(
    root.native(), Watchman::get(root, find_user_socket(root))
  ).first->second;
}

}
