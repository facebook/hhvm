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
#include <folly/io/async/EventBaseThread.h>
#include <folly/json/json.h>
#include <folly/logging/xlog.h>

#include <watchman/cppclient/WatchmanClient.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/sandbox-events.h"
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
#include "hphp/util/configs/autoload.h" // @manual=//hphp/util/configs:autoload
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

constexpr std::chrono::seconds kDefaultIdleSec{30 * 60};
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
  return options.dir();
}

::gid_t getGroup() {
  // Resolve the group to a unix gid
  if (Cfg::Autoload::DBGroup.empty()) {
    return -1;
  }
  try {
    GroupInfo grp{Cfg::Autoload::DBGroup.c_str()};
    return grp.gr->gr_gid;
  } catch (const Exception& e) {
    XLOGF(
        WARN,
        "Can't resolve {} to a gid: {}",
        Cfg::Autoload::DBGroup,
        e.what());
    return -1;
  }
}

::mode_t getDBPerms() {
  try {
    ::mode_t res = std::stoi(Cfg::Autoload::DBPerms, 0, 8);
    XLOGF(DBG0, "Converted {} to {:04o}", Cfg::Autoload::DBPerms, res);
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

SQLiteKey getDBKey(const fs::path& root, const RepoOptions& repoOptions) {
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
  auto const dbPath = repoOptions.autoloadDB();
  always_assert(!dbPath.empty());
  // Create a DB with the given permissions if none exists
  if (Cfg::Autoload::DBCanCreate) {
    ::gid_t gid = getGroup();
    return SQLiteKey::readWriteCreate(dbPath, gid, getDBPerms());
  }
  // Use an existing DB and throw if it doesn't exist
  return SQLiteKey::readWrite(dbPath);
}

/**
 * List of options making a SqliteAutoloadMap unique
 */
struct SqliteAutoloadMapKey {
  static SqliteAutoloadMapKey get(const RepoOptions& repoOptions) {
    auto root = getRepoRoot(repoOptions);

    auto queryExpr = [&]() -> folly::dynamic {
      auto const cached = repoOptions.flags().autoloadQueryObj();
      if (cached.isObject())
        return cached;

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

    auto dbKey = getDBKey(root, repoOptions);

    return SqliteAutoloadMapKey{
        .m_root = std::move(root),
        .m_queryExpr = std::move(queryExpr),
        .m_indexedMethodAttrs = repoOptions.flags().indexedMethodAttributes(),
        .m_dbKey = std::move(dbKey)};
  }

  bool operator==(const SqliteAutoloadMapKey& rhs) const noexcept {
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
        "SqliteAutoloadMapKey({}, {}, {}, {})",
        m_root.native(),
        folly::toJson(m_queryExpr),
        indexedMethodAttrString,
        m_dbKey.toString());
  }

  strhash_t hash() const noexcept {
    return folly::hash::hash_combine(
        hash_string_cs(m_root.native()),
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

} // namespace
} // namespace Facts
} // namespace HPHP

namespace std {
template <>
struct hash<HPHP::Facts::SqliteAutoloadMapKey> {
  size_t operator()(const HPHP::Facts::SqliteAutoloadMapKey& k) const {
    return static_cast<size_t>(k.hash());
  }
};
} // namespace std

namespace HPHP {
namespace Facts {
namespace {

/**
 * Sent to AutoloadHandler so AutoloadHandler can create
 * SqliteAutoloadMaps across the open-source / FB-only boundary.
 */
struct SqliteAutoloadMapFactory final : public FactsFactory {
  SqliteAutoloadMapFactory() = default;
  SqliteAutoloadMapFactory(const SqliteAutoloadMapFactory&) = delete;
  SqliteAutoloadMapFactory(SqliteAutoloadMapFactory&&) = delete;
  SqliteAutoloadMapFactory& operator=(const SqliteAutoloadMapFactory&) = delete;
  SqliteAutoloadMapFactory& operator=(SqliteAutoloadMapFactory&&) = delete;
  ~SqliteAutoloadMapFactory() override = default;

  FactsStore* getForOptions(const RepoOptions& options) override;

  /**
   * Delete AutoloadMaps which haven't been accessed in the last
   * `idleSec` seconds.
   */
  void garbageCollectUnusedAutoloadMaps(std::chrono::seconds idleSec);

 private:
  std::mutex m_mutex;

  /**
   * Map from root to AutoloadMap
   */
  hphp_hash_map<SqliteAutoloadMapKey, std::shared_ptr<FactsStore>> m_stores;

  /**
   * Map from root to time we last accessed the AutoloadMap
   */
  hphp_hash_map<
      SqliteAutoloadMapKey,
      std::chrono::time_point<std::chrono::steady_clock>>
      m_lastUsed;
};

struct FactsExtension final : Extension {
  FactsExtension() : Extension("facts", "1.0", "hphp_hphpi") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    // Why are we using TRACE/Logger in moduleLoad instead of XLOG?  Because of
    // the HHVM startup process and where moduleLoad happens within it, we can't
    // initialize any async handlers until moduleInit() otherwise HHVM
    // with throw an exception for having created a thread before it was ready.

    // Create all resources at a deterministic time to avoid SIOF
    m_data = FactsData{};

    // An AutoloadMap may be freed after this many seconds since its last use
    m_data->m_idleSec = std::chrono::seconds{Config::GetInt64(
        ini, config, "Autoload.MapIdleGCTimeSeconds", kDefaultIdleSec.count())};
    if (m_data->m_idleSec != kDefaultIdleSec) {
      FTRACE(
          3, "Autoload.MapIdleGCTimeSeconds = {}\n", m_data->m_idleSec.count());
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
  }

  void moduleInit() override;
  void moduleRegisterNative() override;

  void moduleShutdown() override {
    // Destroy all resources at a deterministic time to avoid SDOF
    FactsFactory::setInstance(nullptr);
    m_data = {};
  }

  std::chrono::seconds getExpirationTime() const {
    return m_data->m_idleSec;
  }

  const WatchmanWatcherOpts& getWatchmanWatcherOpts() const {
    return m_data->m_watchmanWatcherOpts;
  }

 private:
  // Add new members to this struct instead of the top level so we can be sure
  // your new member is destroyed at the right time.
  struct FactsData {
    std::chrono::seconds m_idleSec{kDefaultIdleSec};
    std::unique_ptr<SqliteAutoloadMapFactory> m_factory;
    WatchmanWatcherOpts m_watchmanWatcherOpts;
  };
  Optional<FactsData> m_data;
} s_ext;

std::shared_ptr<Watcher> make_watcher(const SqliteAutoloadMapKey& mapKey) {
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

FactsStore* SqliteAutoloadMapFactory::getForOptions(
    const RepoOptions& options) {
  auto mapKey = [&]() -> Optional<SqliteAutoloadMapKey> {
    try {
      auto mk = SqliteAutoloadMapKey::get(options);
      return {std::move(mk)};
    } catch (const RepoOptionsParseExc& e) {
      XLOG(ERR) << e.what();
      rareSboxEvent("ext_facts", "RepoOptionsParseExc", e.what());
      return std::nullopt;
    }
  }();

  if (!mapKey) {
    return nullptr;
  }

  std::unique_lock g{m_mutex};

  // Mark the fact that we've accessed the map
  m_lastUsed.insert_or_assign(*mapKey, std::chrono::steady_clock::now());

  // Try to return a corresponding SqliteAutoloadMap
  auto const it = m_stores.find(*mapKey);
  if (it != m_stores.end()) {
    return it->second.get();
  }

  // We're creating a new map. This is a good sign that an existing map may
  // be defunct, so schedule a cleanup job to check.
  Treadmill::enqueue(
      [this] { garbageCollectUnusedAutoloadMaps(s_ext.getExpirationTime()); });

  AutoloadDB::Opener dbOpener =
      [dbKey = mapKey->m_dbKey]() -> std::shared_ptr<AutoloadDB> {
    return SQLiteAutoloadDB::get(dbKey);
  };

  if (mapKey->m_dbKey.m_mode == SQLite::OpenMode::ReadOnly) {
    rareSboxEvent(
        "ext_facts", "getForOptions opening trusted store", mapKey->toString());
    XLOGF(
        DBG0,
        "Loading {} from trusted Autoload DB at {}",
        mapKey->m_root.native(),
        mapKey->m_dbKey.m_path.native());
    return m_stores
        .insert(
            {*mapKey,
             make_trusted_facts(
                 mapKey->m_root,
                 std::move(dbOpener),
                 mapKey->m_indexedMethodAttrs)})
        .first->second.get();
  }

  Optional<std::filesystem::path> updateSuppressionPath;
  if (!Cfg::Autoload::UpdateSuppressionPath.empty()) {
    updateSuppressionPath = {
        std::filesystem::path{Cfg::Autoload::UpdateSuppressionPath}};
  }

  // Prefetch a FactsDB if we don't have one, while guarded by m_mutex
  prefetchDb(mapKey->m_root, mapKey->m_dbKey);

  rareSboxEvent(
      "ext_facts", "getForOptions opening mutable store", mapKey->toString());
  return m_stores
      .insert(
          {*mapKey,
           make_watcher_facts(
               mapKey->m_root,
               std::move(dbOpener),
               make_watcher(*mapKey),
               RuntimeOption::ServerExecutionMode(),
               std::move(updateSuppressionPath),
               mapKey->m_indexedMethodAttrs)})
      .first->second.get();
}

void SqliteAutoloadMapFactory::garbageCollectUnusedAutoloadMaps(
    std::chrono::seconds idleSec) {
  auto mapsToRemove = [&]() -> std::vector<std::shared_ptr<FactsStore>> {
    std::unique_lock g{m_mutex};

    // If a map was last used before this time, remove it
    auto deadline = std::chrono::steady_clock::now() - idleSec;

    std::vector<SqliteAutoloadMapKey> keysToRemove;
    for (auto const& [mapKey, _] : m_stores) {
      auto lastUsedIt = m_lastUsed.find(mapKey);
      if (lastUsedIt == m_lastUsed.end() || lastUsedIt->second < deadline) {
        keysToRemove.push_back(mapKey);
      }
    }

    std::vector<std::shared_ptr<FactsStore>> maps;
    maps.reserve(keysToRemove.size());
    for (auto const& mapKey : keysToRemove) {
      XLOG(INFO) << "Evicting SqliteAutoloadMap: " << mapKey.toString();
      auto it = m_stores.find(mapKey);
      if (it != m_stores.end()) {
        rareSboxEvent("ext_facts", __func__, mapKey.toString());
        maps.push_back(std::move(it->second));
        m_stores.erase(it);
      } else {
        rareSboxEvent("ext_facts", "evict unknown mapkey", mapKey.toString());
      }
      m_lastUsed.erase(mapKey);
    }
    return maps;
  }();

  for (auto& map : mapsToRemove) {
    // Join each map's update threads
    map->close();
  }

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

// Only bool, int, double, string, class, lazyclass, and Hack arrays are
// supported.
folly::dynamic dynamicFromVariant(const Variant& v) {
  if (v.isBoolean()) {
    return v.getBoolean();
  }
  if (v.isInteger()) {
    return v.getInt64();
  }
  if (v.isDouble()) {
    return v.getDouble();
  }
  if (v.isLazyClass()) {
    return v.toLazyClassVal().name()->toCppString();
  }
  if (v.isClass()) {
    return v.toClassVal()->name()->toCppString();
  }
  if (v.isString()) {
    return v.getStringData()->toCppString();
  }
  if (v.isDict()) {
    folly::dynamic ret = folly::dynamic::object;
    ret.resize(v.getArrayData()->size());
    IterateKV(v.getArrayData(), [&](TypedValue k, TypedValue v) {
      ret[dynamicFromVariant(Variant::wrap(k))] =
          dynamicFromVariant(Variant::wrap(v));
    });
    return ret;
  }
  if (v.isArray()) {
    folly::dynamic ret = folly::dynamic::array;
    ret.resize(v.getArrayData()->size());
    uint64_t i;
    IterateV(v.getArrayData(), [&](TypedValue v) {
      ret[i] = dynamicFromVariant(Variant::wrap(v));
      i++;
    });
    return ret;
  }
  return nullptr;
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
    return requestOptions->dir() / maybeRoot;
  }();
  if (!root) {
    XLOG(ERR) << "Error resolving " << rootStr.slice();
    rareSboxEvent("ext_facts", "facts_db_path bad root", rootStr.slice());
    return Variant{Variant::NullInit{}};
  }
  assertx(root->is_absolute());

  auto optionPath = *root / ".hhvmconfig.hdf";
  XLOG(DBG0) << "Got options at " << optionPath.native();
  auto const& repoOptions = RepoOptions::forFile(optionPath.native().c_str());

  try {
    return Variant{
        Facts::SqliteAutoloadMapKey::get(repoOptions).m_dbKey.m_path.native()};
  } catch (const Facts::RepoOptionsParseExc& e) {
    rareSboxEvent(
        "ext_facts",
        folly::sformat("facts_db_path {}", rootStr.slice()),
        e.what());
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
    return Variant{fileRes->m_path};
  }
}

Variant HHVM_FUNCTION(
    facts_type_or_type_alias_to_path,
    const String& typeName) {
  auto fileRes = Facts::getFactsOrThrow().getTypeOrTypeAliasFile(typeName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->m_path};
  }
}

Variant HHVM_FUNCTION(facts_function_to_path, const String& functionName) {
  auto fileRes = Facts::getFactsOrThrow().getFunctionFile(functionName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->m_path};
  }
}

Variant HHVM_FUNCTION(facts_constant_to_path, const String& constantName) {
  auto fileRes = Facts::getFactsOrThrow().getConstantFile(constantName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->m_path};
  }
}

Variant HHVM_FUNCTION(facts_module_to_path, const String& moduleName) {
  auto fileRes = Facts::getFactsOrThrow().getModuleFile(moduleName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->m_path};
  }
}

Variant HHVM_FUNCTION(facts_type_alias_to_path, const String& typeAliasName) {
  auto fileRes = Facts::getFactsOrThrow().getTypeAliasFile(typeAliasName);
  if (!fileRes) {
    return Variant{Variant::NullInit{}};
  } else {
    return Variant{fileRes->m_path};
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

Array HHVM_FUNCTION(
    facts_files_with_attribute_and_any_value,
    const String& attr,
    const Variant& value) {
  return Facts::getFactsOrThrow().getFilesWithAttributeAndAnyValue(
      attr, Facts::dynamicFromVariant(value));
}

Array HHVM_FUNCTION(facts_files_and_attr_args_with_attr, const String& attr) {
  return Facts::getFactsOrThrow().getFilesAndAttrValsWithAttribute(attr);
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

namespace Facts {

void FactsExtension::moduleInit() {
  // This, unfortunately, cannot be done in moduleLoad() due to the fact
  // that certain async loggers may create a new thread.  HHVM will throw
  // if any threads have been created during the moduleLoad() step.
  try {
    enableFactsLogging(
        Cfg::Server::User,
        Cfg::Autoload::Logging,
        Cfg::Autoload::AllowLoggingPropagation);
  } catch (std::exception& e) {
    Logger::FError(
        "Caught exception ({}) while setting up logging with settings: {}",
        e.what(),
        Cfg::Autoload::Logging);
  }

  if (Cfg::Autoload::DBPath.empty()) {
    XLOG(ERR) << "Autoload.DB.Path was empty, not enabling native autoloader.";
    return;
  }

  if (RO::WatchmanDefaultSocket.empty()) {
    XLOG(INFO) << "watchman.socket.default was not provided.";
  }

  if (RO::WatchmanRootSocket.empty()) {
    XLOG(INFO) << "watchman.socket.root was not provided.";
  }

  m_data->m_factory = std::make_unique<SqliteAutoloadMapFactory>();
  FactsFactory::setInstance(m_data->m_factory.get());
}

void FactsExtension::moduleRegisterNative() {
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
  HHVM_NAMED_FE(
      HH\\Facts\\files_with_attribute_and_any_value,
      HHVM_FN(facts_files_with_attribute_and_any_value));
  HHVM_NAMED_FE(
      HH\\Facts\\files_and_attr_args_with_attribute,
      HHVM_FN(facts_files_and_attr_args_with_attr));
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
}

} // namespace Facts
} // namespace HPHP
