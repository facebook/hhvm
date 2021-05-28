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

#include <chrono>
#include <functional>
#include <iomanip>
#include <mutex>
#include <optional>
#include <pwd.h>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/types.h>

#include <folly/Hash.h>
#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/io/async/EventBaseThread.h>
#include <folly/json.h>

#include <watchman/cppclient/WatchmanClient.h>

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/ext_facts.h"
#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/runtime/ext/facts/watchman-autoload-map.h"
#include "hphp/runtime/ext/facts/watchman.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/assertions.h"
#include "hphp/util/build-info.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash.h"
#include "hphp/util/user-info.h"
#include "hphp/zend/zend-string.h"

TRACE_SET_MOD(facts);

namespace HPHP {
namespace Facts {
namespace {

constexpr std::string_view kEUIDPlaceholder = "%{euid}";
constexpr std::string_view kSchemaPlaceholder = "%{schema}";
constexpr std::chrono::seconds kDefaultExpirationTime{30 * 60};

struct RepoOptionsParseExc : public std::runtime_error {
  explicit RepoOptionsParseExc(std::string msg)
      : std::runtime_error{std::move(msg)} {
  }
};

/**
 * Get the directory containing the given RepoOptions file. We define this to
 * be the root of the repository we're autoloading.
 */
folly::fs::path getRepoRoot(const RepoOptions& options) {
  return folly::fs::canonical(folly::fs::path{options.path()}.parent_path());
}

folly::fs::path getDBPath(const RepoOptions& repoOptions) {
  always_assert(!RuntimeOption::AutoloadDBPath.empty());
  std::string pathTemplate{RuntimeOption::AutoloadDBPath};

  {
    size_t idx = pathTemplate.find(kEUIDPlaceholder);
    if (idx != std::string::npos) {
      pathTemplate.replace(
          idx, kEUIDPlaceholder.size(), folly::to<std::string>(geteuid()));
    }
  }

  auto root = getRepoRoot(repoOptions);

  {
    size_t idx = pathTemplate.find(kSchemaPlaceholder);
    if (idx != std::string::npos) {
      pathTemplate.replace(
          idx,
          kSchemaPlaceholder.size(),
          mangleUnitSha1(root.native(), repoOptions.path(), repoOptions));
    }
  }

  folly::fs::path dbPath = pathTemplate;
  if (dbPath.is_relative()) {
    dbPath = root / dbPath;
  }

  return folly::fs::system_complete(dbPath);
}

::gid_t getGroup() {
  // Resolve the group to a unix gid
  if (RuntimeOption::AutoloadDBGroup.empty()) {
    return -1;
  }
  try {
    GroupInfo grp{RuntimeOption::AutoloadDBGroup.c_str()};
    return grp.gr->gr_gid;
  } catch (const Exception& e) {
    Logger::Warning(folly::sformat(
        "Can't resolve {} to a gid: {}",
        RuntimeOption::AutoloadDBGroup,
        e.what()));
    return -1;
  }
}

::mode_t getDBPerms() {
  try {
    ::mode_t res = std::stoi(RuntimeOption::AutoloadDBPerms, 0, 8);
    FTRACE(3, "Converted {} to {}\n", RuntimeOption::AutoloadDBPerms, res);
    return res;
  } catch (const std::exception& e) {
    Logger::Warning(folly::sformat(
        "Error running std::stoi on \"Autoload.DB.Perms\": {}", e.what()));
    return 0644;
  }
}

DBData getDBData(
    const folly::fs::path& root,
    const folly::dynamic& queryExpr,
    const RepoOptions& repoOptions) {
  assertx(root.is_absolute());

  auto trustedDBPath = [&]() -> folly::fs::path {
    folly::fs::path trusted{repoOptions.trustedDBPath()};
    if (trusted.empty()) {
      return trusted;
    }
    // If the trustedDBPath is relative, make sure we resolve it relative
    // to the repo root rather than the current working directory
    if (trusted.is_relative()) {
      trusted = root / trusted;
    }
    try {
      return folly::fs::canonical(trusted);
    } catch (const folly::fs::filesystem_error& e) {
      throw RepoOptionsParseExc{folly::sformat(
          "Error resolving Autoload.TrustedDBPath = {}: {}",
          trusted.native().c_str(),
          e.what())};
    }
  }();

  if (trustedDBPath.empty()) {
    ::gid_t gid = getGroup();
    return DBData::readWrite(getDBPath(repoOptions), gid, getDBPerms());
  } else {
    return DBData::readOnly(std::move(trustedDBPath));
  }
}

// Convenience wrapper for std::string_view
inline strhash_t hash_string_view_cs(std::string_view s) {
  return hash_string_cs(s.data(), s.size());
}

/**
 * List of options making a WatchmanAutoloadMap unique
 */
struct WatchmanAutoloadMapKey {

  static WatchmanAutoloadMapKey get(const RepoOptions& repoOptions) {
    auto root = getRepoRoot(repoOptions);

    auto queryExpr = [&]() -> folly::dynamic {
      auto queryStr = repoOptions.autoloadQuery();
      if (queryStr.empty()) {
        return {};
      }
      try {
        return folly::parseJson(queryStr);
      } catch (const folly::json::parse_error& e) {
        throw RepoOptionsParseExc{folly::sformat(
            "Error JSON-parsing Autoload.Query = \"{}\": {}",
            queryStr.c_str(),
            e.what())};
      }
    }();

    auto dbData = getDBData(root, queryExpr, repoOptions);

    return WatchmanAutoloadMapKey{
        .m_root = std::move(root),
        .m_queryExpr = std::move(queryExpr),
        .m_dbData = std::move(dbData)};
  }

  bool operator==(const WatchmanAutoloadMapKey& rhs) const noexcept {
    return m_root == rhs.m_root && m_queryExpr == rhs.m_queryExpr &&
           m_dbData == rhs.m_dbData;
  }

  std::string toString() const {
    return folly::sformat(
        "WatchmanAutoloadMapKey({}, {}, {})",
        m_root.native(),
        folly::toJson(m_queryExpr),
        m_dbData.toString());
  }

  strhash_t hash() const noexcept {
    return folly::hash::hash_combine(
        hash_string_view_cs(m_root.native()),
        m_queryExpr.hash(),
        m_dbData.hash());
  }

  /**
   * A repo is autoloadable if we can either:
   *
   * 1. Use Watchman to track the files and create our own database
   * 2. Read an existing database file somewhere
   */
  bool isAutoloadableRepo() const {
    return m_queryExpr.isObject() ||
           m_dbData.m_rwMode == SQLite::OpenMode::ReadOnly;
  }

  folly::fs::path m_root;
  folly::dynamic m_queryExpr;
  DBData m_dbData;
};

} // namespace
} // namespace Facts
} // namespace HPHP

namespace std {
template <> struct hash<HPHP::Facts::WatchmanAutoloadMapKey> {
  size_t operator()(const HPHP::Facts::WatchmanAutoloadMapKey& k) const {
    return static_cast<size_t>(k.hash());
  }
};
} // namespace std

namespace HPHP {
namespace Facts {
namespace {

/**
 * Sent to AutoloadHandler so AutoloadHandler can create
 * WatchmanAutoloadMaps across the open-source / FB-only boundary.
 */
struct WatchmanAutoloadMapFactory final : public FactsFactory {

  WatchmanAutoloadMapFactory() = default;
  WatchmanAutoloadMapFactory(const WatchmanAutoloadMapFactory&) = delete;
  WatchmanAutoloadMapFactory(WatchmanAutoloadMapFactory&&) = delete;
  WatchmanAutoloadMapFactory&
  operator=(const WatchmanAutoloadMapFactory&) = delete;
  WatchmanAutoloadMapFactory& operator=(WatchmanAutoloadMapFactory&&) = delete;
  ~WatchmanAutoloadMapFactory() override = default;

  FactsStore* getForOptions(const RepoOptions& options) override;

  /**
   * Delete AutoloadMaps which haven't been accessed in the last
   * `expirationTime` seconds.
   */
  void garbageCollectUnusedAutoloadMaps(std::chrono::seconds expirationTime);

private:
  std::mutex m_mutex;

  /**
   * Map from root to AutoloadMap
   */
  hphp_hash_map<WatchmanAutoloadMapKey, std::shared_ptr<FactsStore>> m_maps;

  /**
   * Map from root to time we last accessed the AutoloadMap
   */
  hphp_hash_map<
      WatchmanAutoloadMapKey,
      std::chrono::time_point<std::chrono::steady_clock>>
      m_lastUsed;
};

struct Facts final : Extension {
  Facts() : Extension("facts", "1.0") {
  }

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {

    // Create all resources at a deterministic time to avoid SIOF
    m_data = FactsData{};

    // An AutoloadMap may be freed after this many seconds since its last use
    m_data->m_expirationTime = std::chrono::seconds{Config::GetInt64(
        ini,
        config,
        "Autoload.MapIdleGCTimeSeconds",
        kDefaultExpirationTime.count())};
    if (m_data->m_expirationTime != kDefaultExpirationTime) {
      FTRACE(
          3,
          "Autoload.MapIdleGCTimeSeconds = {}\n",
          m_data->m_expirationTime.count());
    }

    auto watchmanSocket =
        Config::GetString(ini, config, "watchman.socket.default");
    if (!watchmanSocket.empty()) {
      FTRACE(3, "watchman.socket.default = {}\n", watchmanSocket);
      m_data->m_watchmanDefaultSocket = std::move(watchmanSocket);
    }

    auto watchmanRootSocket =
        Config::GetString(ini, config, "watchman.socket.root");
    if (!watchmanRootSocket.empty()) {
      FTRACE(3, "watchman.socket.root = {}\n", watchmanRootSocket);
      m_data->m_watchmanRootSocket = std::move(watchmanRootSocket);
    }

    auto excluded = Config::GetStrVector(ini, config, "Autoload.ExcludedRepos");
    for (auto const& repo : excluded) {
      try {
        m_data->m_excludedRepos.insert(folly::fs::canonical(repo).native());
      } catch (const folly::fs::filesystem_error& e) {
        Logger::Info(
            "Could not disable native autoloader for %s: %s\n",
            repo.c_str(),
            e.what());
      }
    }
  }

  void moduleInit() override {
    HHVM_NAMED_FE(HH\\Facts\\enabled, HHVM_FN(facts_enabled));
    HHVM_NAMED_FE(HH\\Facts\\db_path, HHVM_FN(facts_db_path));
    HHVM_NAMED_FE(HH\\Facts\\type_to_path, HHVM_FN(facts_type_to_path));
    HHVM_NAMED_FE(HH\\Facts\\function_to_path, HHVM_FN(facts_function_to_path));
    HHVM_NAMED_FE(HH\\Facts\\constant_to_path, HHVM_FN(facts_constant_to_path));
    HHVM_NAMED_FE(
        HH\\Facts\\type_alias_to_path, HHVM_FN(facts_type_alias_to_path));

    HHVM_NAMED_FE(HH\\Facts\\path_to_types, HHVM_FN(facts_path_to_types));
    HHVM_NAMED_FE(
        HH\\Facts\\path_to_functions, HHVM_FN(facts_path_to_functions));
    HHVM_NAMED_FE(
        HH\\Facts\\path_to_constants, HHVM_FN(facts_path_to_constants));
    HHVM_NAMED_FE(
        HH\\Facts\\path_to_type_aliases, HHVM_FN(facts_path_to_type_aliases));
    HHVM_NAMED_FE(HH\\Facts\\type_name, HHVM_FN(facts_type_name));
    HHVM_NAMED_FE(HH\\Facts\\kind, HHVM_FN(facts_kind));
    HHVM_NAMED_FE(HH\\Facts\\is_abstract, HHVM_FN(facts_is_abstract));
    HHVM_NAMED_FE(HH\\Facts\\is_final, HHVM_FN(facts_is_final));
    HHVM_NAMED_FE(HH\\Facts\\subtypes, HHVM_FN(facts_subtypes));
    HHVM_NAMED_FE(
        HH\\Facts\\transitive_subtypes, HHVM_FN(facts_transitive_subtypes));
    HHVM_NAMED_FE(HH\\Facts\\supertypes, HHVM_FN(facts_supertypes));
    HHVM_NAMED_FE(
        HH\\Facts\\types_with_attribute, HHVM_FN(facts_types_with_attribute));
    HHVM_NAMED_FE(
        HH\\Facts\\type_aliases_with_attribute,
        HHVM_FN(facts_type_aliases_with_attribute));
    HHVM_NAMED_FE(
        HH\\Facts\\methods_with_attribute,
        HHVM_FN(facts_methods_with_attribute));
    HHVM_NAMED_FE(HH\\Facts\\type_attributes, HHVM_FN(facts_type_attributes));
    HHVM_NAMED_FE(
        HH\\Facts\\type_alias_attributes, HHVM_FN(facts_type_alias_attributes));
    HHVM_NAMED_FE(
        HH\\Facts\\method_attributes, HHVM_FN(facts_method_attributes));
    HHVM_NAMED_FE(
        HH\\Facts\\type_attribute_parameters,
        HHVM_FN(facts_type_attribute_parameters));
    HHVM_NAMED_FE(
        HH\\Facts\\type_alias_attribute_parameters,
        HHVM_FN(facts_type_alias_attribute_parameters));
    HHVM_NAMED_FE(
        HH\\Facts\\method_attribute_parameters,
        HHVM_FN(facts_method_attribute_parameters));
    HHVM_NAMED_FE(HH\\Facts\\all_types, HHVM_FN(facts_all_types));
    HHVM_NAMED_FE(HH\\Facts\\all_functions, HHVM_FN(facts_all_functions));
    HHVM_NAMED_FE(HH\\Facts\\all_constants, HHVM_FN(facts_all_constants));
    HHVM_NAMED_FE(HH\\Facts\\all_type_aliases, HHVM_FN(facts_all_type_aliases));
    loadSystemlib();

    if (!RuntimeOption::AutoloadEnabled) {
      FTRACE(
          1,
          "Autoload.Enabled is not true, not enabling native "
          "autoloader.\n");
      return;
    }

    if (RuntimeOption::AutoloadDBPath.empty()) {
      FTRACE(
          1, "Autoload.DB.Path was empty, not enabling native autoloader.\n");
      return;
    }

    if (!m_data->m_watchmanDefaultSocket) {
      FTRACE(2, "watchman.socket.default was not provided.\n");
    }

    if (!m_data->m_watchmanRootSocket) {
      FTRACE(2, "watchman.socket.root was not provided.\n");
    }

    m_data->m_mapFactory = std::make_unique<WatchmanAutoloadMapFactory>();
    FactsFactory::setInstance(m_data->m_mapFactory.get());
  }

  void moduleShutdown() override {
    // Destroy all resources at a deterministic time to avoid SDOF
    FactsFactory::setInstance(nullptr);
    m_data = {};
  }

  std::chrono::seconds getExpirationTime() const {
    return m_data->m_expirationTime;
  }

  const std::optional<std::string>& getWatchmanDefaultSocket() const {
    return m_data->m_watchmanDefaultSocket;
  }

  const std::optional<std::string>& getWatchmanRootSocket() const {
    return m_data->m_watchmanRootSocket;
  }

  Watchman& getWatchmanClient(const folly::fs::path& root) {
    auto it = m_data->m_watchmanClients.find(root.native());
    if (it != m_data->m_watchmanClients.end()) {
      return *it->second;
    }
    auto watchmanSocket = getPerUserWatchmanSocket();
    if (!watchmanSocket) {
      FTRACE(3, "No per-user watchman socket found.\n");
      watchmanSocket = getWatchmanDefaultSocket();
    }

    return *m_data->m_watchmanClients
                .insert(root.native(), Watchman::get(root, watchmanSocket))
                .first->second;
  }

private:
  // Add new members to this struct instead of the top level so we can be sure
  // your new member is destroyed at the right time.
  struct FactsData {
    std::chrono::seconds m_expirationTime{30 * 60};
    std::optional<std::string> m_watchmanDefaultSocket;
    std::optional<std::string> m_watchmanRootSocket;
    hphp_hash_set<std::string> m_excludedRepos;
    folly::ConcurrentHashMap<std::string, std::shared_ptr<Watchman>>
        m_watchmanClients{};
    std::unique_ptr<WatchmanAutoloadMapFactory> m_mapFactory;
  };
  std::optional<FactsData> m_data;

  /**
   * Discover who owns the given repo and return the Watchman socket
   * corresponding to that user.
   */
  std::optional<std::string> getPerUserWatchmanSocket() {
    IniSetting::Map ini = IniSetting::Map::object;
    Hdf config;
    auto* repoOptions = g_context->getRepoOptionsForRequest();
    if (!repoOptions) {
      return {};
    }

    // Figure out who owns the repo we're trying to run code in
    auto repoRoot = getRepoRoot(*repoOptions);
    int repoRootFD = ::open(repoRoot.native().c_str(), O_DIRECTORY | O_RDONLY);
    if (repoRootFD == -1) {
      return {};
    }
    SCOPE_EXIT {
      ::close(repoRootFD);
    };
    struct ::stat hstat {};
    if (::fstat(repoRootFD, &hstat) != 0) {
      return {};
    }

    // The repo is owned by root, so use a special root socket
    if (hstat.st_uid == 0) {
      auto const& rootSock = getWatchmanRootSocket();
      FTRACE(
          3,
          "{} is owned by root, looking for socket at {}\n",
          repoRoot.native(),
          rootSock ? *rootSock : "<none>");
      return rootSock;
    }

    // Find the `watchman.socket` setting in the repo owner's
    // SandboxConfFile (usually a .hphp file somewhere in their home
    // directory).
    UserInfo info{hstat.st_uid};
    auto user = std::string{info.pw->pw_name};
    auto homePath = RuntimeOption::GetHomePath(user);
    if (!homePath) {
      return {};
    }
    auto confFileName = (*homePath) / RuntimeOption::SandboxConfFile;
    bool success =
        RuntimeOption::ReadPerUserSettings(confFileName, ini, config);
    if (!success) {
      return {};
    }
    auto sock = Config::GetString(ini, config, "watchman.socket.default");
    if (sock.empty()) {
      return {};
    }
    return {std::move(sock)};
  }

} s_ext;

FactsStore*
WatchmanAutoloadMapFactory::getForOptions(const RepoOptions& options) {

  auto mapKey = [&]() -> std::optional<WatchmanAutoloadMapKey> {
    try {
      auto mk = WatchmanAutoloadMapKey::get(options);
      if (!mk.isAutoloadableRepo()) {
        return std::nullopt;
      }
      return {std::move(mk)};
    } catch (const RepoOptionsParseExc& e) {
      Logger::Warning("%s\n", e.what());
      return std::nullopt;
    }
  }();

  if (!mapKey) {
    return nullptr;
  }

  std::unique_lock g{m_mutex};

  // Mark the fact that we've accessed the map
  m_lastUsed.insert_or_assign(*mapKey, std::chrono::steady_clock::now());

  // Try to return a corresponding WatchmanAutoloadMap
  auto const it = m_maps.find(*mapKey);
  if (it != m_maps.end()) {
    return it->second.get();
  }

  // We're creating a new map. This is a good sign that an existing map may
  // be defunct, so schedule a cleanup job to check.
  Treadmill::enqueue(
      [this] { garbageCollectUnusedAutoloadMaps(s_ext.getExpirationTime()); });

  if (mapKey->m_dbData.m_rwMode == SQLite::OpenMode::ReadOnly) {
    FTRACE(
        3,
        "Loading {} from trusted Autoload DB at {}\n",
        mapKey->m_root.native(),
        mapKey->m_dbData.m_path.native());
    return m_maps
        .insert(
            {*mapKey,
             std::make_shared<WatchmanAutoloadMap>(
                 mapKey->m_root, mapKey->m_dbData)})
        .first->second.get();
  }

  assertx(mapKey->m_queryExpr.isObject());
  auto map = std::make_shared<WatchmanAutoloadMap>(
      mapKey->m_root,
      mapKey->m_dbData,
      mapKey->m_queryExpr,
      s_ext.getWatchmanClient(mapKey->m_root));

  if (RuntimeOption::ServerExecutionMode()) {
    map->subscribe();
  }

  return m_maps.insert({*mapKey, std::move(map)}).first->second.get();
}

void WatchmanAutoloadMapFactory::garbageCollectUnusedAutoloadMaps(
    std::chrono::seconds expirationTime) {
  auto mapsToRemove = [&]() -> std::vector<std::shared_ptr<FactsStore>> {
    std::unique_lock g{m_mutex};

    // If a map was last used before this time, remove it
    auto deadline = std::chrono::steady_clock::now() - expirationTime;

    std::vector<WatchmanAutoloadMapKey> keysToRemove;
    for (auto const& [mapKey, _] : m_maps) {
      auto lastUsedIt = m_lastUsed.find(mapKey);
      if (lastUsedIt == m_lastUsed.end() || lastUsedIt->second < deadline) {
        keysToRemove.push_back(mapKey);
      }
    }

    std::vector<std::shared_ptr<FactsStore>> maps;
    maps.reserve(keysToRemove.size());
    for (auto const& mapKey : keysToRemove) {
      FTRACE(2, "Evicting WatchmanAutoloadMap: {}\n", mapKey.toString());
      auto it = m_maps.find(mapKey);
      if (it != m_maps.end()) {
        maps.push_back(std::move(it->second));
        m_maps.erase(it);
      }
      m_lastUsed.erase(mapKey);
    }
    return maps;
  }();

  // Final references to shared_ptr<Facts> fall out of scope
  // while `m_mutex` lock is not held
}

FactsStore& getFactsOrThrow() {
  auto* facts = AutoloadHandler::s_instance->getFacts();
  if (facts == nullptr) {
    SystemLib::throwInvalidOperationExceptionObject(
        "Native Facts is not enabled. Call HH\\Facts\\enabled() to determine "
        "if native Facts is enabled for the current request.");
  }
  return *facts;
}

} // namespace
} // namespace Facts

bool HHVM_FUNCTION(facts_enabled) {
  return AutoloadHandler::s_instance->getFacts() != nullptr;
}

Variant HHVM_FUNCTION(facts_db_path, const String& rootStr) {
  // Turn rootStr into an absolute path.
  auto root = [&]() -> std::optional<folly::fs::path> {
    folly::fs::path maybeRoot{rootStr.get()->slice()};
    if (maybeRoot.is_absolute()) {
      return maybeRoot;
    }
    // The given root is a relative path, so find the directory where the
    // current request's `.hhvmconfig.hdf` file lives and resolve relative to
    // that.
    auto requestOptions = g_context->getRepoOptionsForRequest();
    if (!requestOptions) {
      return std::nullopt;
    }
    return folly::fs::path{requestOptions->path()}.parent_path() / maybeRoot;
  }();
  if (!root) {
    FTRACE(2, "Error resolving {}\n", rootStr.slice());
    return Variant{Variant::NullInit{}};
  }
  assertx(root->is_absolute());

  auto optionPath = *root / ".hhvmconfig.hdf";
  FTRACE(3, "Got options at {}\n", optionPath.native());
  auto const& repoOptions = RepoOptions::forFile(optionPath.native().c_str());

  try {
    return Variant{Facts::WatchmanAutoloadMapKey::get(repoOptions)
                       .m_dbData.m_path.native()};
  } catch (const Facts::RepoOptionsParseExc& e) {
    throw_invalid_operation_exception(makeStaticString(e.what()));
  }
}

Variant HHVM_FUNCTION(facts_type_to_path, const String& typeName) {
  auto path = Facts::getFactsOrThrow().getTypeFile(typeName);
  if (!path) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{*path};
  }
}

Variant HHVM_FUNCTION(facts_function_to_path, const String& functionName) {
  auto path = Facts::getFactsOrThrow().getFunctionFile(functionName);
  if (!path) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{*path};
  }
}

Variant HHVM_FUNCTION(facts_constant_to_path, const String& constantName) {
  auto path = Facts::getFactsOrThrow().getConstantFile(constantName);
  if (!path) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{*path};
  }
}

Variant HHVM_FUNCTION(facts_type_alias_to_path, const String& typeAliasName) {
  auto path = Facts::getFactsOrThrow().getTypeAliasFile(typeAliasName);
  if (!path) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{*path};
  }
}

Array HHVM_FUNCTION(facts_path_to_types, const String& path) {
  return Facts::getFactsOrThrow().getFileTypes(path);
}

Array HHVM_FUNCTION(facts_path_to_functions, const String& path) {
  return Facts::getFactsOrThrow().getFileFunctions(path);
}

Array HHVM_FUNCTION(facts_path_to_constants, const String& path) {
  return Facts::getFactsOrThrow().getFileConstants(path);
}

Array HHVM_FUNCTION(facts_path_to_type_aliases, const String& path) {
  return Facts::getFactsOrThrow().getFileTypeAliases(path);
}

Variant HHVM_FUNCTION(facts_type_name, const String& type) {
  return Facts::getFactsOrThrow().getTypeName(type);
}

Variant HHVM_FUNCTION(facts_kind, const String& type) {
  return Facts::getFactsOrThrow().getKind(type);
}

bool HHVM_FUNCTION(facts_is_abstract, const String& type) {
  return Facts::getFactsOrThrow().isTypeAbstract(type);
}

bool HHVM_FUNCTION(facts_is_final, const String& type) {
  return Facts::getFactsOrThrow().isTypeFinal(type);
}

Array HHVM_FUNCTION(
    facts_subtypes, const String& baseType, const Variant& filters) {
  return Facts::getFactsOrThrow().getDerivedTypes(baseType, filters);
}

Array HHVM_FUNCTION(
    facts_transitive_subtypes, const String& baseType, const Variant& filters) {
  return Facts::getFactsOrThrow().getTransitiveDerivedTypes(baseType, filters);
}

Array HHVM_FUNCTION(
    facts_supertypes, const String& derivedType, const Variant& filters) {
  return Facts::getFactsOrThrow().getBaseTypes(derivedType, filters);
}

Array HHVM_FUNCTION(facts_types_with_attribute, const String& attr) {
  return Facts::getFactsOrThrow().getTypesWithAttribute(attr);
}

Array HHVM_FUNCTION(facts_type_aliases_with_attribute, const String& attr) {
  return Facts::getFactsOrThrow().getTypeAliasesWithAttribute(attr);
}

Array HHVM_FUNCTION(facts_methods_with_attribute, const String& attr) {
  return Facts::getFactsOrThrow().getMethodsWithAttribute(attr);
}

Array HHVM_FUNCTION(facts_type_attributes, const String& type) {
  return Facts::getFactsOrThrow().getTypeAttributes(type);
}

Array HHVM_FUNCTION(facts_type_alias_attributes, const String& typeAlias) {
  return Facts::getFactsOrThrow().getTypeAttributes(typeAlias);
}

Array HHVM_FUNCTION(
    facts_method_attributes, const String& type, const String& method) {
  return Facts::getFactsOrThrow().getMethodAttributes(type, method);
}

Array HHVM_FUNCTION(
    facts_type_attribute_parameters, const String& type, const String& attr) {
  return Facts::getFactsOrThrow().getTypeAttrArgs(type, attr);
}

Array HHVM_FUNCTION(
    facts_type_alias_attribute_parameters,
    const String& type,
    const String& attr) {
  return Facts::getFactsOrThrow().getTypeAttrArgs(type, attr);
}

Array HHVM_FUNCTION(
    facts_method_attribute_parameters,
    const String& type,
    const String& method,
    const String& attr) {
  return Facts::getFactsOrThrow().getMethodAttrArgs(type, method, attr);
}

Array HHVM_FUNCTION(facts_all_types) {
  return Facts::getFactsOrThrow().getAllTypes();
}
Array HHVM_FUNCTION(facts_all_functions) {
  return Facts::getFactsOrThrow().getAllFunctions();
}
Array HHVM_FUNCTION(facts_all_constants) {
  return Facts::getFactsOrThrow().getAllConstants();
}
Array HHVM_FUNCTION(facts_all_type_aliases) {
  return Facts::getFactsOrThrow().getAllTypeAliases();
}

} // namespace HPHP
