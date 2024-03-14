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

#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <folly/Executor.h>
#include <folly/SharedMutex.h>
#include <folly/Synchronized.h>
#include <folly/futures/Future.h>
#include <folly/futures/FutureSplitter.h>
#include "hphp/runtime/ext/facts/attribute-map.h"
#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/inheritance-info.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/path-symbols-map.h"
#include "hphp/runtime/ext/facts/path-versions.h"
#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/sha1.h"
#include "hphp/util/sqlite-wrapper.h"

namespace HPHP {

struct StringData;

namespace Facts {

struct UpdateDBWorkItem {
  Clock m_since;
  Clock m_clock;
  std::vector<std::filesystem::path> m_alteredPaths;
  std::vector<std::filesystem::path> m_deletedPaths;
  std::vector<FileFacts> m_alteredPathFacts;
};

/**
 * Stores a map from thread to AutoloadDB.
 */
struct AutoloadDBVault {
  explicit AutoloadDBVault(AutoloadDB::Opener);

  /**
   * Get the AutoloadDB associated with the thread that calls this method.
   *
   * Logically this is `const`, but it creates an AutoloadDB on first access.
   */
  std::shared_ptr<AutoloadDB> get() const;

 private:
  AutoloadDB::Opener m_dbOpener;
  // Holds one AutoloadDB per thread. Creates an AutoloadDB on the first access.
  mutable folly::Synchronized<
      hphp_hash_map<std::thread::id, std::shared_ptr<AutoloadDB>>>
      m_dbs;
};

/**
 * Stores and updates one PathToSymbolsMap for each kind of symbol.
 *
 * Multiple readers can run concurrently with one writer, but only one
 * call to SymbolMap::update should happen at any given time.
 *
 * Queries the SQLite AutoloadDB when the DB has information
 * we don't, and updates the AutoloadDB when we have
 * information the DB doesn't have.
 */
struct SymbolMap {
  explicit SymbolMap(
      std::filesystem::path root,
      AutoloadDB::Opener dbOpener,
      hphp_vector_set<Symbol<SymKind::Type>> indexedMethodAttributes,
      bool enableBlockingDbWait,
      std::chrono::milliseconds blockingDbwWaitTimeout);
  SymbolMap() = delete;
  SymbolMap(const SymbolMap&) = delete;
  SymbolMap(SymbolMap&&) noexcept = delete;
  SymbolMap& operator=(const SymbolMap&) = delete;
  SymbolMap& operator=(SymbolMap&&) noexcept = delete;
  ~SymbolMap();

  /**
   * Resolve a type's name to its canonical, correctly-capitalized name.
   *
   * Return nullptr if the type is not defined, or if the type is defined in
   * more than one file.
   */
  Optional<Symbol<SymKind::Type>> getTypeName(const StringData& type);

  /**
   * Return the one and only definition for the given symbol.
   *
   * If the symbol is defined in no files, or in more than one file,
   * return nullptr.
   *
   * These methods may fill the map with information from the SQLite
   * DB, and as such may throw SQLite exceptions.
   */
  Path getTypeOrTypeAliasFile(Symbol<SymKind::Type> type);
  Path getTypeOrTypeAliasFile(const StringData& type);
  Path getTypeFile(Symbol<SymKind::Type> type);
  Path getTypeFile(const StringData& type);
  Path getFunctionFile(Symbol<SymKind::Function> function);
  Path getFunctionFile(const StringData& function);
  Path getConstantFile(Symbol<SymKind::Constant> constant);
  Path getConstantFile(const StringData& constant);
  Path getModuleFile(Symbol<SymKind::Module> module);
  Path getModuleFile(const StringData& module);
  Path getTypeAliasFile(Symbol<SymKind::Type> typeAlias);
  Path getTypeAliasFile(const StringData& typeAlias);

  /**
   * Return all symbols of a given kind declared in the given path.
   *
   * These methods may fill the map with information from the SQLite
   * DB, and as such may throw SQLite exceptions.
   */
  std::vector<Symbol<SymKind::Type>> getFileTypes(Path path);
  std::vector<Symbol<SymKind::Type>> getFileTypes(const std::filesystem::path&);

  std::vector<Symbol<SymKind::Function>> getFileFunctions(Path path);
  std::vector<Symbol<SymKind::Function>> getFileFunctions(
      const std::filesystem::path& path);

  std::vector<Symbol<SymKind::Constant>> getFileConstants(Path path);
  std::vector<Symbol<SymKind::Constant>> getFileConstants(
      const std::filesystem::path& path);

  std::vector<Symbol<SymKind::Module>> getFileModules(Path path);
  std::vector<Symbol<SymKind::Module>> getFileModules(
      const std::filesystem::path& path);

  std::vector<Symbol<SymKind::Type>> getFileTypeAliases(Path path);
  std::vector<Symbol<SymKind::Type>> getFileTypeAliases(
      const std::filesystem::path& path);

  /**
   * Return inheritance data about the given type
   */
  std::vector<Symbol<SymKind::Type>> getBaseTypes(
      Symbol<SymKind::Type> derivedType,
      DeriveKind kind);
  std::vector<Symbol<SymKind::Type>> getBaseTypes(
      const StringData& derivedType,
      DeriveKind kind);

  std::vector<Symbol<SymKind::Type>> getDerivedTypes(
      Symbol<SymKind::Type> baseType,
      DeriveKind kind);
  std::vector<Symbol<SymKind::Type>> getDerivedTypes(
      const StringData& baseType,
      DeriveKind kind);

  /**
   * Return the attributes of a type
   */
  std::vector<Symbol<SymKind::Type>> getAttributesOfType(
      Symbol<SymKind::Type> type);
  std::vector<Symbol<SymKind::Type>> getAttributesOfType(
      const StringData& type);

  /**
   * Return the attributes decorating a type alias
   */
  std::vector<Symbol<SymKind::Type>> getAttributesOfTypeAlias(
      Symbol<SymKind::Type> typeAlias);
  std::vector<Symbol<SymKind::Type>> getAttributesOfTypeAlias(
      const StringData& typeAlias);

  /**
   * Return the types decorated with a given attribute
   */
  std::vector<Symbol<SymKind::Type>> getTypesWithAttribute(
      Symbol<SymKind::Type> attr);
  std::vector<Symbol<SymKind::Type>> getTypesWithAttribute(
      const StringData& attr);

  /**
   * Return the type aliases decorated with a given attribute
   */
  std::vector<Symbol<SymKind::Type>> getTypeAliasesWithAttribute(
      Symbol<SymKind::Type> attr);
  std::vector<Symbol<SymKind::Type>> getTypeAliasesWithAttribute(
      const StringData& attr);

  /**
   * Return the attributes of a method
   */
  std::vector<Symbol<SymKind::Type>> getAttributesOfMethod(
      Symbol<SymKind::Type> type,
      Symbol<SymKind::Method> method);
  std::vector<Symbol<SymKind::Type>> getAttributesOfMethod(
      const StringData& type,
      const StringData& method);

  /**
   * Return the methods with a given attribute
   */
  std::vector<MethodDecl> getMethodsWithAttribute(Symbol<SymKind::Type> attr);
  std::vector<MethodDecl> getMethodsWithAttribute(const StringData& attr);

  /**
   * Return the attributes of a file
   */
  std::vector<Symbol<SymKind::Type>> getAttributesOfFile(Path path);

  /**
   * Return the files with a given attribute
   */
  std::vector<Path> getFilesWithAttribute(Symbol<SymKind::Type> attr);
  std::vector<Path> getFilesWithAttribute(const StringData& attr);

  /**
   * Return the files with a given attribute and value
   */
  std::vector<Path> getFilesWithAttributeAndAnyValue(
      Symbol<SymKind::Type> attr,
      const folly::dynamic& value);
  std::vector<Path> getFilesWithAttributeAndAnyValue(
      const StringData& attr,
      const folly::dynamic& value);

  std::vector<FileAttrVal> getFilesAndAttrValsWithAttribute(
      Symbol<SymKind::Type> attr);

  std::vector<FileAttrVal> getFilesAndAttrValsWithAttribute(
      const StringData& attr);

  /**
   * Return the argument at the given position of a given type with a given
   * attribute.
   *
   * So if a type were defined with an attribute like this:
   *
   *     <<Oncalls('hhvm')>>
   *     class Foo {}
   *
   * You'd expect to be able to extract that "hhvm" argument this way:
   *
   *     getAttributeArg("Foo", "Oncalls").at(0) == "hhvm"
   *
   * If this function returns an empty vector, it could be because:
   *
   * - The type is not defined.
   * - The type is defined in more than one file, violating the One Definition
   *   Rule.
   * - The type doesn't have the given attribute.
   * - The attribute doesn't have any arguments.
   *
   * You can check that the type is defined with `getTypeFile()`, and you can
   * check that the type has the given attribute with `getAttributesOfType()`.
   */
  std::vector<folly::dynamic> getTypeAttributeArgs(
      Symbol<SymKind::Type> type,
      Symbol<SymKind::Type> attribute);
  std::vector<folly::dynamic> getTypeAttributeArgs(
      const StringData& type,
      const StringData& attribute);

  std::vector<folly::dynamic> getTypeAliasAttributeArgs(
      Symbol<SymKind::Type> type,
      Symbol<SymKind::Type> attribute);
  std::vector<folly::dynamic> getTypeAliasAttributeArgs(
      const StringData& type,
      const StringData& attribute);

  std::vector<folly::dynamic> getMethodAttributeArgs(
      Symbol<SymKind::Type> type,
      Symbol<SymKind::Method> method,
      Symbol<SymKind::Type> attribute);
  std::vector<folly::dynamic> getMethodAttributeArgs(
      const StringData& type,
      const StringData& method,
      const StringData& attribute);

  std::vector<folly::dynamic> getFileAttributeArgs(
      Path path,
      Symbol<SymKind::Type> attribute);
  std::vector<folly::dynamic> getFileAttributeArgs(
      Path path,
      const StringData& attribute);

  /**
   * Return whether the given type is, for example, a class or interface.
   *
   * Return `TypeKind::Unknown` if the given type does not have a unique
   * definition or is not an autoloadable type at all.
   */
  TypeKind getKind(Symbol<SymKind::Type> type);
  TypeKind getKind(const StringData& type);

  bool isTypeAbstract(Symbol<SymKind::Type> type);
  bool isTypeAbstract(const StringData& type);

  bool isTypeFinal(Symbol<SymKind::Type> type);
  bool isTypeFinal(const StringData& type);

  bool isAttrIndexed(const StringData& attr) const;
  std::string debugIndexedAttrs() const;

  /**
   * Return a hash representing the given path's last-known checksum.
   */
  Optional<SHA1> getSha1Hash(Path path) const;

  /**
   * For each file, update the SymbolMap with the given file facts.
   *
   * On success set m_clock to the given clock, and schedule a thread
   * to update the DB with the new information.
   *
   * If the `since` token is nonempty and does not correspond to
   * either our in-memory clock or the clock in the DB, throw an
   * UpdateExc and do not perform any writes. The caller should
   * initiate a fresh Watchman query with a `since` token
   * corresponding to the clock in the map.
   *
   * since: An opaque token originating from Watchman representing the
   * beginning of this update's time interval. `since` should either
   * be empty (if we're initializing the map from scratch) or should
   * be the timestamp currently in either the map or the DB (if we're
   * incrementally updating the map).
   *
   * clock: An opaque token originating from Watchman representing the
   * end of this update's time interval. After updating, this clock
   * will be stored in the SymbolMap and DB.
   *
   * alteredPaths: A list of all paths which have changed between the
   * last two queries to Watchman (represented by `since` and
   * `clock`). This vector must have the same number of elements as
   * `facts`.
   *
   * deletedPaths: A list of all paths which have been deleted between
   * the last two queries to Watchman (represented by `since` and
   * `clock`).
   *
   * alteredPathFacts: A list of all symbols found in the
   * `alteredPaths`. Must be the same size as `alteredPaths`, and
   * elements must be in the same order.
   */
  void update(
      const Clock& since,
      const Clock& clock,
      std::vector<std::filesystem::path> alteredPaths,
      std::vector<std::filesystem::path> deletedPaths,
      std::vector<FileFacts> alteredPathFacts); // throws(SQLiteExc)

  /**
   * Return an opaque token representing how up to date this map is.
   *
   * This token originated from Watchman.
   */
  Clock getClock() const noexcept;

  /**
   * Return an opaque token representing how up to date the SQLite DB is.
   *
   * This token originated from Watchman.
   */
  Clock dbClock() const; // throws(SQLiteExc)

  /**
   * Return the one and only path where `symbol` is defined.
   *
   * If `symbol` is not defined, return nullptr.
   *
   * If `symbol` is defined in more than one path, return either one of the
   * paths that defines `symbol`, or return `nullptr`. In Hack, it's a bug to
   * define the same symbol in multiple paths, so we leave this behavior
   * flexible.
   *
   * Query the DB if there is no data about `symbol` in the given
   * symbolMap, and add any information found to the corresponding symbolMap if
   * so.
   */
  template <SymKind k>
  Path getSymbolPath(Symbol<k> symbol); // throws(SQLiteExc)

  /**
   * Return all the symbols of the kind corresponding to symbolMap
   * defined in the given path.
   *
   * Query the DB if there is no data about `path` in the given
   * symbolMap, and add any information found to the corresponding symbolMap if
   * so.
   */
  template <SymKind k>
  typename PathToSymbolsMap<k>::PathSymbolMap::Values getPathSymbols(Path path);

  void waitForDBUpdate();
  void waitForDBUpdate(std::chrono::milliseconds timeoutMs);

  /**
   * Return a map from path to hash for every path we know about.
   */
  hphp_hash_map<Path, SHA1> getAllPathsWithHashes() const;

  std::shared_ptr<folly::Executor> m_exec;

  struct Data {
    Data();

    /**
     * A Watchman clock representing how up-to-date this map is.
     *
     * If its `m_clock` string is empty, then this map has not yet updated.
     */
    Clock m_clock;

    /**
     * Version numbers which get bumped each time a path changes. We filter out
     * the facts in our data structures which have version numbers older than
     * the ones in this map.
     */
    std::shared_ptr<PathVersions> m_versions;

    /**
     * Maps between symbols and the paths defining them.
     */
    PathToSymbolsMap<SymKind::Type> m_typePath;
    PathToSymbolsMap<SymKind::Function> m_functionPath;
    PathToSymbolsMap<SymKind::Constant> m_constantPath;
    PathToSymbolsMap<SymKind::Module> m_modulePath;

    /**
     * Future chain and queue holding the work that needs to be done before the
     * DB is considered up to date.
     */
    std::queue<UpdateDBWorkItem> m_updateDBWork;
    folly::FutureSplitter<folly::Unit> m_updateDBFuture{folly::makeFuture()};

    struct TypeInfo {
      using KindAndFlags = std::pair<TypeKind, int>;

      void setKindAndFlags(
          Symbol<SymKind::Type> type,
          Path path,
          TypeKind kind,
          int flags) {
        auto& defs = m_map[type];
        for (auto& [existingPath, existingInfo] : defs) {
          if (existingPath == path) {
            existingInfo = {kind, flags};
            return;
          }
        }
        defs.push_back({path, {kind, flags}});
      }

      Optional<std::pair<TypeKind, int>> getKindAndFlags(
          Symbol<SymKind::Type> type,
          Path path) const {
        auto const it = m_map.find(type);
        if (it == m_map.end()) {
          return std::nullopt;
        }
        for (auto& [existingPath, info] : it->second) {
          if (existingPath == path) {
            return info;
          }
        }
        return std::nullopt;
      }

      // {type: (path, (kind, flags))}
      hphp_hash_map<
          Symbol<SymKind::Type>,
          std::vector<std::pair<Path, KindAndFlags>>>
          m_map;
    } m_typeKind;

    /**
     * True if the file exists, false if the file is deleted.
     */
    hphp_hash_map<Path, bool> m_fileExistsMap;

    /**
     * Maps between types and their subtypes/supertypes.
     */
    InheritanceInfo m_inheritanceInfo;

    /**
     * Maps between types and the attributes that decorate them.
     */
    AttributeMap<TypeDecl> m_typeAttrs;

    /**
     * Maps between type aliases and the attributes that decorate them.
     */
    AttributeMap<TypeDecl> m_typeAliasAttrs;

    /**
     * Maps between methods and the attributes that decorate them.
     */
    AttributeMap<MethodDecl> m_methodAttrs;

    /**
     * Maps between files and the attributes that decorate them.
     */
    AttributeMap<Path> m_fileAttrs;

    /**
     * 40-byte hex strings representing the last-known SHA1 checksums of
     * each file we've seen
     */
    hphp_hash_map<Path, SHA1> m_sha1Hashes;

    /**
     * Parse the given path and store all its data in the map.
     */
    void updatePath(
        Path path,
        FileFacts facts,
        const hphp_vector_set<Symbol<SymKind::Type>>& indexedMethodAttrs);

    /**
     * Remove the given path from the map, along with all data associated with
     * the path.
     */
    void removePath(Path path);
  };

 private:
  void waitForDBUpdateImpl(HPHP::Optional<std::chrono::milliseconds> timeoutMs);
  /**
   * Update the DB on the time interval beginning at `since` and
   * ending at `clock`.
   *
   * We throw an UpdateExc if the `since` token does not match the clock
   * in the DB, and we don't catch SQLiteExc from the underlying SQLite layer.
   */
  void updateDB(
      const Clock& since,
      const Clock& clock,
      const std::vector<std::filesystem::path>& alteredPaths,
      const std::vector<std::filesystem::path>& deletedPaths,
      const std::vector<FileFacts>& alteredPathFacts) const; // throws

  /**
   * Replace all facts in the DB with in-memory facts about the given path.
   */
  void updateDBPath(
      AutoloadDB& db,
      const std::filesystem::path& path,
      const FileFacts& facts) const;

  /**
   * Mark `derivedType` as inheriting from each of the `baseTypes`.
   */
  void setBaseTypes(
      Path path,
      Symbol<SymKind::Type> derivedType,
      rust::Vec<rust::String> baseTypes);

  /**
   * Load information from the DB about who the given `derivedType` inherits.
   */
  void loadBaseTypesFromDB(
      AutoloadDB& db,
      Path path,
      Symbol<SymKind::Type> derivedType);

  /**
   * Helper function to read from and write to m_synchronizedData.
   *
   * readFn: ((const Data&) -> Optional<Ret>)
   * GetFromDBFn: ((AutoloadDB&) -> DataFromDB)
   * writeFn: ((Data&, DataFromDB) -> Ret)
   */
  template <
      typename Ret,
      typename ReadFn,
      typename GetFromDBFn,
      typename WriteFn>
  Ret readOrUpdate(ReadFn readFn, GetFromDBFn getFromDBFn, WriteFn writeFn);

  /**
   * Return a thread-local connection to the DB associated with this map.
   */
  std::shared_ptr<AutoloadDB> getDB() const;

  /**
   * Return the type's kind (class/interface/enum) along with an
   * abstract/final bitmask.
   */
  std::pair<TypeKind, TypeFlagMask> getKindAndFlags(Symbol<SymKind::Type> type);
  std::pair<TypeKind, TypeFlagMask> getKindAndFlags(
      Symbol<SymKind::Type> type,
      Path path);

  std::atomic<bool> m_useDB = false;
  // Used to prioritize updates over caching. Pending updates increment this
  // count, while caching only occurs if this count is at 0.
  std::atomic<size_t> m_updatesInFlight = 0;

  folly::Synchronized<Data, folly::SharedMutexWritePriority> m_syncedData;

  const std::filesystem::path m_root;
  const std::string m_schemaHash;
  AutoloadDBVault m_dbVault;
  const hphp_vector_set<Symbol<SymKind::Type>> m_indexedMethodAttrs;
  const bool m_enableBlockingDbWait;
  const std::chrono::milliseconds m_blockingDbWaitTimeout;
};

} // namespace Facts
} // namespace HPHP
