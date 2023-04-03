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

#include <pwd.h>
#include <sys/types.h>
#include <chrono>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

#include <folly/Conv.h>
#include <folly/Hash.h>
#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/io/async/EventBaseThread.h>
#include <folly/json.h>
#include <folly/logging/xlog.h>

#include <watchman/cppclient/WatchmanClient.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/watchman-connection.h"
#include "hphp/runtime/base/watchman.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/facts/fact-extractor.h"
#include "hphp/runtime/ext/facts/facts-store.h"
#include "hphp/runtime/ext/facts/logging.h"
#include "hphp/runtime/ext/facts/sqlite-autoload-db.h"
#include "hphp/runtime/ext/facts/sqlite-key.h"
#include "hphp/runtime/ext/facts/static-watcher.h"
#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/runtime/ext/facts/watchman-watcher.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash.h"
#include "hphp/util/logger.h"
#include "hphp/util/sqlite-wrapper.h"
#include "hphp/util/trace.h"
#include "hphp/util/user-info.h"
#include "hphp/zend/zend-string.h"

TRACE_SET_MOD(facts);

namespace fs = std::filesystem;

namespace HPHP {
namespace Facts {
namespace {

// SQLFacts version number representing the DB's schema.  This number is
// determined randomly, but should match the number in the SQL Facts
// implementation.  We use this when we make a change that invalidates
// the cache, such as adding a new table which would otherwise be
// unpopulated without a cache rebuild.
constexpr size_t kSchemaVersion = 1916337637;

constexpr std::string_view kEUIDPlaceholder = "%{euid}";
constexpr std::string_view kSchemaPlaceholder = "%{schema}";
constexpr std::chrono::seconds kDefaultExpirationTime{30 * 60};
constexpr int32_t kDefaultWatchmanRetries = 0;

struct RepoOptionsParseExc : public std::runtime_error {
  explicit RepoOptionsParseExc(std::string msg)
      : std::runtime_error{std::move(msg)} {}
};

/**
 * Get the directory containing the given RepoOptions file. We define this to
 * be the root of the repository we're autoloading.
 */
fs::path getRepoRoot(const RepoOptions& options) {
  return fs::canonical(fs::path{options.path()}.parent_path());
}

std::string getCacheBreakerSchemaHash(
    std::string_view root,
    const RepoOptions& opts) {
  std::string optsHash = opts.flags().cacheKeySha1().toString();
  XLOG(INFO) << "Native Facts DB cache breaker:"
             << "\n Version: " << kSchemaVersion << "\n Root: " << root
             << "\n RepoOpts hash: " << optsHash;
  std::string rootHash = string_sha1(root);
  optsHash.resize(10);
  rootHash.resize(10);
  return folly::to<std::string>(kSchemaVersion, '_', optsHash, '_', rootHash);
}

fs::path getDBPath(const RepoOptions& repoOptions) {
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
          getCacheBreakerSchemaHash(root.native(), repoOptions));
    }
  }

  fs::path dbPath = pathTemplate;
  if (dbPath.is_relative()) {
    dbPath = root / dbPath;
  }

  return fs::absolute(dbPath);
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
    XLOGF(
        WARN,
        "Can't resolve {} to a gid: {}",
        RuntimeOption::AutoloadDBGroup,
        e.what());
    return -1;
  }
}

::mode_t getDBPerms() {
  try {
    ::mode_t res = std::stoi(RuntimeOption::AutoloadDBPerms, 0, 8);
    XLOGF(DBG0, "Converted {} to {:04o}", RuntimeOption::AutoloadDBPerms, res);
    return res;
  } catch (const std::exception& e) {
    XLOG(WARN) << "Error running std::stoi on \"Autoload.DB.Perms\": "
               << e.what();
    return 0644;
  }
}

bool hasWatchedFileExtension(const std::filesystem::path& path) {
  auto ext = path.extension();
  return ext == ".php" || ext == ".hck" || ext == ".inc";
}

SQLiteKey getDBKey(
    const fs::path& root,
    const folly::dynamic& queryExpr,
    const RepoOptions& repoOptions) {
  assertx(root.is_absolute());

  auto trustedDBPath = [&]() -> fs::path {
    fs::path trusted{repoOptions.flags().trustedDBPath()};
    if (trusted.empty()) {
      return trusted;
    }
    // If the trustedDBPath is relative, make sure we resolve it relative
    // to the repo root rather than the current working directory
    if (trusted.is_relative()) {
      trusted = root / trusted;
    }
    try {
      return fs::canonical(trusted);
    } catch (const fs::filesystem_error& e) {
      throw RepoOptionsParseExc{folly::sformat(
          "Error resolving Autoload.TrustedDBPath = {}: {}",
          trusted.native().c_str(),
          e.what())};
    }
  }();

  // Autoload out of a read-only, "trusted" DB, like in /var/www
  if (!trustedDBPath.empty()) {
    return SQLiteKey::readOnly(std::move(trustedDBPath));
  }
  // Create a DB with the given permissions if none exists
  if (RuntimeOption::AutoloadDBCanCreate) {
    ::gid_t gid = getGroup();
    return SQLiteKey::readWriteCreate(
        getDBPath(repoOptions), gid, getDBPerms());
  }
  // Use an existing DB and throw if it doesn't exist
  return SQLiteKey::readWrite(getDBPath(repoOptions));
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
      auto queryStr = repoOptions.flags().autoloadQuery();
      if (queryStr.empty()) {
        return {};
      }
      try {
        return folly::parseJson(queryStr);
      } catch (const folly::json::parse_error& e) {
        throw RepoOptionsParseExc{folly::sformat(
            "Error JSON-parsing Autoload.Query = \"{}\": {}",
            queryStr,
            e.what())};
      }
    }();

    auto dbKey = getDBKey(root, queryExpr, repoOptions);

    return WatchmanAutoloadMapKey{
        .m_root = std::move(root),
        .m_queryExpr = std::move(queryExpr),
        .m_indexedMethodAttrs = repoOptions.flags().indexedMethodAttributes(),
        .m_dbKey = std::move(dbKey)};
  }

  bool operator==(const WatchmanAutoloadMapKey& rhs) const noexcept {
    return m_root == rhs.m_root && m_queryExpr == rhs.m_queryExpr &&
        m_indexedMethodAttrs == rhs.m_indexedMethodAttrs &&
        m_dbKey == rhs.m_dbKey;
  }

  std::string toString() const {
    std::string indexedMethodAttrString = "{";
    for (auto const& v : m_indexedMethodAttrs) {
      if (indexedMethodAttrString.size() > 1) {
        indexedMethodAttrString += ',';
      }
      indexedMethodAttrString += v;
    }
    indexedMethodAttrString += '}';

    return folly::sformat(
        "WatchmanAutoloadMapKey({}, {}, {}, {})",
        m_root.native(),
        folly::toJson(m_queryExpr),
        indexedMethodAttrString,
        m_dbKey.toString());
  }

  strhash_t hash() const noexcept {
    return folly::hash::hash_combine(
        hash_string_view_cs(m_root.native()),
        m_queryExpr.hash(),
        folly::hash::hash_range(
            m_indexedMethodAttrs.begin(), m_indexedMethodAttrs.end()),
        m_dbKey.hash());
  }

  fs::path m_root;
  folly::dynamic m_queryExpr;
  std::vector<std::string> m_indexedMethodAttrs;
  SQLiteKey m_dbKey;
};

// Code to coerce a FileFacts into a userspace shape.

#define X(str) const StaticString s_##str{#str};
// Attribute
X(name)
X(args)

// MethodDetails
X(attributes)

// TypeDetails
X(kind)
X(flags)
X(baseTypes)
X(requireClass)
X(requireExtends)
X(requireImplements)
X(methods)

// FileFacts
X(types)
X(functions)
X(constants)
X(modules)
X(sha1sum)
#undef X

template <typename T>
Array makeVec(const std::vector<T>& types) {
  VecInit ret{types.size()};
  for (auto const& type : types) {
    ret.append(type);
  }
  return ret.toArray();
}

Array makeVec(const std::vector<folly::dynamic>& args) {
  VecInit ret{args.size()};
  for (auto const& arg : args) {
    ret.append(Variant::fromDynamic(arg));
  }
  return ret.toArray();
}

Array makeShape(const Attribute& attr) {
  return make_dict_array(
      StrNR{s_name},
      String{std::string_view{attr.m_name}},
      StrNR{s_args},
      makeVec(attr.m_args));
}

Array makeVec(const std::vector<Attribute>& attrs) {
  VecInit ret{attrs.size()};
  for (auto const& attr : attrs) {
    ret.append(makeShape(attr));
  }
  return ret.toArray();
}

Array makeShape(const MethodDetails& method) {
  size_t retSize = 2;
  DictInit ret{retSize};
  ret.set(StrNR{s_name}, method.m_name);
  ret.set(StrNR{s_attributes}, makeVec(method.m_attributes));
  return ret.toArray();
}

Array makeVec(const std::vector<MethodDetails>& methods) {
  VecInit ret{methods.size()};
  for (auto const& method : methods) {
    ret.append(makeShape(method));
  }
  return ret.toArray();
}

Array makeShape(const TypeDetails& type) {
  size_t retSize = 9;
  DictInit ret{retSize};
  ret.set(StrNR{s_name}, type.m_name);
  ret.set(StrNR{s_kind}, String{toString(type.m_kind)});
  ret.set(StrNR{s_flags}, type.m_flags);
  ret.set(StrNR{s_baseTypes}, makeVec(type.m_baseTypes));
  ret.set(StrNR{s_attributes}, makeVec(type.m_attributes));
  ret.set(StrNR{s_requireClass}, makeVec(type.m_requireClass));
  ret.set(StrNR{s_requireExtends}, makeVec(type.m_requireExtends));
  ret.set(StrNR{s_requireImplements}, makeVec(type.m_requireImplements));
  ret.set(StrNR{s_methods}, makeVec(type.m_methods));
  return ret.toArray();
}

Array makeVec(const std::vector<TypeDetails>& types) {
  VecInit ret{types.size()};
  for (auto const& type : types) {
    ret.append(makeShape(type));
  }
  return ret.toArray();
}

Array makeShape(const ModuleDetails& modules) {
  size_t retSize = 1;
  DictInit ret{retSize};
  ret.set(StrNR{s_name}, modules.m_name);
  return ret.toArray();
}

Array makeVec(const std::vector<ModuleDetails>& modules) {
  VecInit ret{modules.size()};
  for (auto const& module : modules) {
    ret.append(makeShape(module));
  }
  return ret.toArray();
}

Array makeShape(const FileFacts& facts) {
  return make_dict_array(
      StrNR{s_types},
      makeVec(facts.m_types),

      StrNR{s_functions},
      makeVec(facts.m_functions),

      StrNR{s_constants},
      makeVec(facts.m_constants),

      StrNR{s_modules},
      makeVec(facts.m_modules),

      StrNR{s_attributes},
      makeVec(facts.m_attributes),

      StrNR{s_sha1sum},
      String{facts.m_sha1hex});
}

} // namespace
} // namespace Facts
} // namespace HPHP

namespace std {
template <>
struct hash<HPHP::Facts::WatchmanAutoloadMapKey> {
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
  WatchmanAutoloadMapFactory& operator=(const WatchmanAutoloadMapFactory&) =
      delete;
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
  Facts() : Extension("facts", "1.0") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    if (!RuntimeOption::AutoloadEnabled) {
      return;
    }

    // Why are we using TRACE/Logger in moduleLoad instead of XLOG?  Because of
    // the HHVM startup process and where moduleLoad happens within it, we can't
    // initialize any async handlers until moduleInit() otherwise HHVM
    // with throw an exception for having created a thread before it was ready.

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

    if (!RO::WatchmanDefaultSocket.empty()) {
      FTRACE(3, "watchman.socket.default = {}\n", RO::WatchmanDefaultSocket);
    }

    if (!RO::WatchmanRootSocket.empty()) {
      FTRACE(3, "watchman.socket.root = {}\n", RO::WatchmanRootSocket);
    }

    m_data->m_watchmanWatcherOpts = WatchmanWatcherOpts{
        .m_retries = Config::GetInt32(
            ini, config, "Autoload.WatchmanRetries", kDefaultWatchmanRetries)};

    for (auto const& repo : RuntimeOption::AutoloadExcludedRepos) {
      try {
        m_data->m_excludedRepos.insert(fs::canonical(repo).native());
      } catch (const fs::filesystem_error& e) {
        Logger::Info(
            "Could not disable native autoloader for %s: %s\n",
            repo.c_str(),
            e.what());
      }
    }
  }

  void moduleInit() override;

  void moduleShutdown() override {
    // Destroy all resources at a deterministic time to avoid SDOF
    FactsFactory::setInstance(nullptr);
    m_data = {};
  }

  std::chrono::seconds getExpirationTime() const {
    return m_data->m_expirationTime;
  }

  const WatchmanWatcherOpts& getWatchmanWatcherOpts() const {
    return m_data->m_watchmanWatcherOpts;
  }

 private:
  // Add new members to this struct instead of the top level so we can be sure
  // your new member is destroyed at the right time.
  struct FactsData {
    std::chrono::seconds m_expirationTime{30 * 60};
    hphp_hash_set<std::string> m_excludedRepos;
    std::unique_ptr<WatchmanAutoloadMapFactory> m_mapFactory;
    WatchmanWatcherOpts m_watchmanWatcherOpts;
  };
  Optional<FactsData> m_data;
} s_ext;

std::shared_ptr<Watcher> make_watcher(const WatchmanAutoloadMapKey& mapKey) {
  if (mapKey.m_queryExpr.isObject()) {
    // Pass the query expression to Watchman to watch the directory
    return make_watchman_watcher(
        mapKey.m_queryExpr,
        get_watchman_client(mapKey.m_root),
        s_ext.getWatchmanWatcherOpts());
  } else {
    XLOG(INFO) << "Crawling " << mapKey.m_root << " ...";
    // Crawl the filesystem now to build a list of files for the static watcher.
    // We won't refresh this list of files.
    std::vector<std::filesystem::path> staticFiles;
    for (auto const& entry :
         std::filesystem::recursive_directory_iterator{mapKey.m_root}) {
      if (entry.is_regular_file() && hasWatchedFileExtension(entry)) {
        staticFiles.push_back(
            std::filesystem::relative(entry.path(), mapKey.m_root));
      }
    }
    if (staticFiles.size() > 100'000) {
      XLOG(WARN)
          << "Found " << staticFiles.size() << " files in " << mapKey.m_root
          << " . Consider installing Watchman and setting Autoload.Query in "
             "your repo's .hhvmconfig.hdf file.";
    }
    return make_static_watcher(staticFiles);
  }
}

FactsStore* WatchmanAutoloadMapFactory::getForOptions(
    const RepoOptions& options) {
  auto mapKey = [&]() -> Optional<WatchmanAutoloadMapKey> {
    try {
      auto mk = WatchmanAutoloadMapKey::get(options);
      return {std::move(mk)};
    } catch (const RepoOptionsParseExc& e) {
      XLOG(ERR) << e.what();
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

  AutoloadDB::Handle dbHandle =
      [dbKey = mapKey->m_dbKey]() -> std::shared_ptr<AutoloadDB> {
    return SQLiteAutoloadDB::get(dbKey);
  };

  if (mapKey->m_dbKey.m_writable == SQLite::OpenMode::ReadOnly) {
    XLOGF(
        DBG0,
        "Loading {} from trusted Autoload DB at {}",
        mapKey->m_root.native(),
        mapKey->m_dbKey.m_path.native());
    return m_maps
        .insert(
            {*mapKey,
             make_trusted_facts(
                 mapKey->m_root,
                 std::move(dbHandle),
                 mapKey->m_indexedMethodAttrs)})
        .first->second.get();
  }

  Optional<std::filesystem::path> updateSuppressionPath;
  if (!RuntimeOption::AutoloadUpdateSuppressionPath.empty()) {
    updateSuppressionPath = {
        std::filesystem::path{RuntimeOption::AutoloadUpdateSuppressionPath}};
  }
  return m_maps
      .insert(
          {*mapKey,
           make_watcher_facts(
               mapKey->m_root,
               std::move(dbHandle),
               make_watcher(*mapKey),
               RuntimeOption::ServerExecutionMode(),
               std::move(updateSuppressionPath),
               mapKey->m_indexedMethodAttrs)})
      .first->second.get();
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
      XLOG(INFO) << "Evicting WatchmanAutoloadMap: " << mapKey.toString();
      auto it = m_maps.find(mapKey);
      if (it != m_maps.end()) {
        maps.push_back(std::move(it->second));
        m_maps.erase(it);
      }
      m_lastUsed.erase(mapKey);
    }
    return maps;
  }();

  // Final references to shared_ptr<Facts> fall out of scope on the Treadmill
  Treadmill::enqueue([_destroyed = std::move(mapsToRemove)]() {});
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

void HHVM_FUNCTION(facts_sync) {
  Facts::getFactsOrThrow().ensureUpdated();
}

Variant HHVM_FUNCTION(facts_db_path, const String& rootStr) {
  // Turn rootStr into an absolute path.
  auto root = [&]() -> Optional<fs::path> {
    fs::path maybeRoot{rootStr.toCppString()};
    if (maybeRoot.is_absolute()) {
      return maybeRoot;
    }
    // The given root is a relative path, so find the directory where the
    // current request's `.hhvmconfig.hdf` file lives and resolve relative to
    // that.
    auto requestOptions = g_context->getRepoOptionsForRequest();
    if (!requestOptions || requestOptions->path().empty()) {
      return std::nullopt;
    }
    return fs::path{requestOptions->path()}.parent_path() / maybeRoot;
  }();
  if (!root) {
    XLOG(ERR) << "Error resolving " << rootStr.slice();
    return Variant{Variant::NullInit{}};
  }
  assertx(root->is_absolute());

  auto optionPath = *root / ".hhvmconfig.hdf";
  XLOG(DBG0) << "Got options at " << optionPath.native();
  auto const& repoOptions = RepoOptions::forFile(optionPath.native().c_str());

  try {
    return Variant{Facts::WatchmanAutoloadMapKey::get(repoOptions)
                       .m_dbKey.m_path.native()};
  } catch (const Facts::RepoOptionsParseExc& e) {
    throw_invalid_operation_exception(makeStaticString(e.what()));
  }
}

int64_t HHVM_FUNCTION(facts_schema_version) {
  return Facts::kSchemaVersion;
}

Variant HHVM_FUNCTION(facts_type_to_path, const String& typeName) {
  auto fileRes = Facts::getFactsOrThrow().getTypeFile(typeName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->path};
  }
}

Variant HHVM_FUNCTION(
    facts_type_or_type_alias_to_path,
    const String& typeName) {
  auto fileRes = Facts::getFactsOrThrow().getTypeOrTypeAliasFile(typeName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->path};
  }
}

Variant HHVM_FUNCTION(facts_function_to_path, const String& functionName) {
  auto fileRes = Facts::getFactsOrThrow().getFunctionFile(functionName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->path};
  }
}

Variant HHVM_FUNCTION(facts_constant_to_path, const String& constantName) {
  auto fileRes = Facts::getFactsOrThrow().getConstantFile(constantName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->path};
  }
}

Variant HHVM_FUNCTION(facts_module_to_path, const String& moduleName) {
  auto fileRes = Facts::getFactsOrThrow().getModuleFile(moduleName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->path};
  }
}

Variant HHVM_FUNCTION(facts_type_alias_to_path, const String& typeAliasName) {
  auto fileRes = Facts::getFactsOrThrow().getTypeAliasFile(typeAliasName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->path};
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

Array HHVM_FUNCTION(facts_path_to_modules, const String& path) {
  return Facts::getFactsOrThrow().getFileModules(path);
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
    facts_subtypes,
    const String& baseType,
    const Variant& filters) {
  return Facts::getFactsOrThrow().getDerivedTypes(baseType, filters);
}

Array HHVM_FUNCTION(
    facts_transitive_subtypes,
    const String& baseType,
    const Variant& filters) {
  return Facts::getFactsOrThrow().getTransitiveDerivedTypes(baseType, filters);
}

Array HHVM_FUNCTION(
    facts_supertypes,
    const String& derivedType,
    const Variant& filters) {
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

Array HHVM_FUNCTION(facts_files_with_attribute, const String& attr) {
  return Facts::getFactsOrThrow().getFilesWithAttribute(attr);
}

Array HHVM_FUNCTION(facts_type_attributes, const String& type) {
  return Facts::getFactsOrThrow().getTypeAttributes(type);
}

Array HHVM_FUNCTION(facts_type_alias_attributes, const String& typeAlias) {
  return Facts::getFactsOrThrow().getTypeAliasAttributes(typeAlias);
}

Array HHVM_FUNCTION(
    facts_method_attributes,
    const String& type,
    const String& method) {
  return Facts::getFactsOrThrow().getMethodAttributes(type, method);
}

Array HHVM_FUNCTION(facts_file_attributes, const String& file) {
  return Facts::getFactsOrThrow().getFileAttributes(file);
}

Array HHVM_FUNCTION(
    facts_type_attribute_parameters,
    const String& type,
    const String& attr) {
  return Facts::getFactsOrThrow().getTypeAttrArgs(type, attr);
}

Array HHVM_FUNCTION(
    facts_type_alias_attribute_parameters,
    const String& type,
    const String& attr) {
  return Facts::getFactsOrThrow().getTypeAliasAttrArgs(type, attr);
}

Array HHVM_FUNCTION(
    facts_method_attribute_parameters,
    const String& type,
    const String& method,
    const String& attr) {
  return Facts::getFactsOrThrow().getMethodAttrArgs(type, method, attr);
}

Array HHVM_FUNCTION(
    facts_file_attribute_parameters,
    const String& file,
    const String& attr) {
  return Facts::getFactsOrThrow().getFileAttrArgs(file, attr);
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
Array HHVM_FUNCTION(facts_all_modules) {
  return Facts::getFactsOrThrow().getAllModules();
}

Array HHVM_FUNCTION(
    facts_extract,
    const Array& alteredPathsAndHashesArr,
    const Variant& maybeRoot) {
  // Get the root of the repository
  auto root = [&]() -> fs::path {
    if (maybeRoot.isString()) {
      return {std::string{maybeRoot.toStrNR().get()->slice()}};
    }
    auto* repoOptions = g_context->getRepoOptionsForRequest();
    if (!repoOptions || repoOptions->path().empty()) {
      SystemLib::throwInvalidOperationExceptionObject(
          "Could not find a root directory for the current request");
    }
    return Facts::getRepoRoot(*repoOptions);
  }();

  // Coerce the given vec<(string, string)> into C++ paths and hashes
  std::vector<Facts::PathAndOptionalHash> alteredPathsAndHashes;
  alteredPathsAndHashes.reserve(alteredPathsAndHashesArr.size());
  std::vector<String> alteredPathStrs;
  alteredPathStrs.reserve(alteredPathsAndHashesArr.size());
  IterateV(alteredPathsAndHashesArr.get(), [&](TypedValue v) {
    Facts::PathAndOptionalHash pathAndHash;
    if (UNLIKELY(!tvIsArrayLike(v))) {
      SystemLib::throwTypeAssertionExceptionObject(
          "HH\\Facts\\extract expects vec<(string, ?string)>");
    }
    size_t i = 0;
    IterateV(v.m_data.parr, [&](TypedValue pathOrHash) {
      if (i == 0) {
        if (UNLIKELY(!tvIsString(pathOrHash))) {
          SystemLib::throwTypeAssertionExceptionObject(
              "HH\\Facts\\extract expects vec<(string, ?string)>");
        }
        alteredPathStrs.push_back(String::attach(pathOrHash.m_data.pstr));
        pathAndHash.m_path = {std::string{pathOrHash.m_data.pstr->slice()}};
      } else if (i == 1) {
        if (tvIsString(pathOrHash)) {
          pathAndHash.m_hash = {std::string{pathOrHash.m_data.pstr->slice()}};
        } else if (!tvIsNull(pathOrHash)) {
          SystemLib::throwTypeAssertionExceptionObject(
              "HH\\Facts\\extract expects vec<(string, ?string)>");
        }
      }
      ++i;
    });
    alteredPathsAndHashes.push_back(std::move(pathAndHash));
  });

  // Extract facts
  auto alteredPathFacts = Facts::facts_from_paths(root, alteredPathsAndHashes);

  // Convert extracted Facts to HHVM userspace objects
  DictInit factsArr{alteredPathsAndHashes.size()};
  for (int64_t i = 0; i < alteredPathFacts.size(); ++i) {
    auto const& facts = alteredPathFacts[i];
    if (LIKELY(facts.hasValue())) {
      factsArr.set(
          std::move(alteredPathStrs[i]), tvReturn(Facts::makeShape(*facts)));
    } else {
      // Set the path's facts to null on failure. This is likely a parse error.
      factsArr.set(std::move(alteredPathStrs[i]), Variant{});
    }
  }
  return factsArr.toArray();
}

namespace Facts {

void Facts::moduleInit() {
  // This, unfortunately, cannot be done in moduleLoad() due to the fact
  // that certain async loggers may create a new thread.  HHVM will throw
  // if any threads have been created during the moduleLoad() step.
  try {
    enableFactsLogging(
        RuntimeOption::ServerUser,
        RuntimeOption::AutoloadLogging,
        RuntimeOption::AutoloadLoggingAllowPropagation);
  } catch (std::exception& e) {
    Logger::FError(
        "Caught exception ({}) while setting up logging with settings: {}",
        e.what(),
        RuntimeOption::AutoloadLogging);
  }

  HHVM_NAMED_FE(HH\\Facts\\enabled, HHVM_FN(facts_enabled));
  HHVM_NAMED_FE(HH\\Facts\\db_path, HHVM_FN(facts_db_path));
  HHVM_NAMED_FE(HH\\Facts\\schema_version, HHVM_FN(facts_schema_version));
  HHVM_NAMED_FE(HH\\Facts\\sync, HHVM_FN(facts_sync));
  HHVM_NAMED_FE(HH\\Facts\\type_to_path, HHVM_FN(facts_type_to_path));
  HHVM_NAMED_FE(
      HH\\Facts\\type_or_type_alias_to_path,
      HHVM_FN(facts_type_or_type_alias_to_path));
  HHVM_NAMED_FE(HH\\Facts\\function_to_path, HHVM_FN(facts_function_to_path));
  HHVM_NAMED_FE(HH\\Facts\\constant_to_path, HHVM_FN(facts_constant_to_path));
  HHVM_NAMED_FE(HH\\Facts\\module_to_path, HHVM_FN(facts_module_to_path));
  HHVM_NAMED_FE(
      HH\\Facts\\type_alias_to_path, HHVM_FN(facts_type_alias_to_path));

  HHVM_NAMED_FE(HH\\Facts\\path_to_types, HHVM_FN(facts_path_to_types));
  HHVM_NAMED_FE(HH\\Facts\\path_to_functions, HHVM_FN(facts_path_to_functions));
  HHVM_NAMED_FE(HH\\Facts\\path_to_constants, HHVM_FN(facts_path_to_constants));
  HHVM_NAMED_FE(
      HH\\Facts\\path_to_type_aliases, HHVM_FN(facts_path_to_type_aliases));
  HHVM_NAMED_FE(HH\\Facts\\path_to_modules, HHVM_FN(facts_path_to_modules));
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
      HH\\Facts\\methods_with_attribute, HHVM_FN(facts_methods_with_attribute));
  HHVM_NAMED_FE(
      HH\\Facts\\files_with_attribute, HHVM_FN(facts_files_with_attribute));
  HHVM_NAMED_FE(HH\\Facts\\type_attributes, HHVM_FN(facts_type_attributes));
  HHVM_NAMED_FE(
      HH\\Facts\\type_alias_attributes, HHVM_FN(facts_type_alias_attributes));
  HHVM_NAMED_FE(HH\\Facts\\method_attributes, HHVM_FN(facts_method_attributes));
  HHVM_NAMED_FE(HH\\Facts\\file_attributes, HHVM_FN(facts_file_attributes));
  HHVM_NAMED_FE(
      HH\\Facts\\type_attribute_parameters,
      HHVM_FN(facts_type_attribute_parameters));
  HHVM_NAMED_FE(
      HH\\Facts\\type_alias_attribute_parameters,
      HHVM_FN(facts_type_alias_attribute_parameters));
  HHVM_NAMED_FE(
      HH\\Facts\\method_attribute_parameters,
      HHVM_FN(facts_method_attribute_parameters));
  HHVM_NAMED_FE(
      HH\\Facts\\file_attribute_parameters,
      HHVM_FN(facts_file_attribute_parameters));
  HHVM_NAMED_FE(HH\\Facts\\all_types, HHVM_FN(facts_all_types));
  HHVM_NAMED_FE(HH\\Facts\\all_functions, HHVM_FN(facts_all_functions));
  HHVM_NAMED_FE(HH\\Facts\\all_constants, HHVM_FN(facts_all_constants));
  HHVM_NAMED_FE(HH\\Facts\\all_type_aliases, HHVM_FN(facts_all_type_aliases));
  HHVM_NAMED_FE(HH\\Facts\\all_modules, HHVM_FN(facts_all_modules));
  HHVM_NAMED_FE(HH\\Facts\\extract, HHVM_FN(facts_extract));
  loadSystemlib();

  if (!RuntimeOption::AutoloadEnabled) {
    XLOG(INFO)
        << "Autoload.Enabled is not true, not enabling native autoloader.";
    return;
  }

  if (RuntimeOption::AutoloadDBPath.empty()) {
    XLOG(ERR) << "Autoload.DB.Path was empty, not enabling native autoloader.";
    return;
  }

  if (RO::WatchmanDefaultSocket.empty()) {
    XLOG(INFO) << "watchman.socket.default was not provided.";
  }

  if (RO::WatchmanRootSocket.empty()) {
    XLOG(INFO) << "watchman.socket.root was not provided.";
  }

  m_data->m_mapFactory = std::make_unique<WatchmanAutoloadMapFactory>();
  FactsFactory::setInstance(m_data->m_mapFactory.get());
}

} // namespace Facts
} // namespace HPHP
