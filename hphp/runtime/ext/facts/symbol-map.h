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
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <folly/Executor.h>
#include <folly/SharedMutex.h>
#include <folly/Synchronized.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/futures/Future.h>
#include <folly/futures/FutureSplitter.h>

#include "hphp/runtime/ext/facts/attribute-map.h"
#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/inheritance-info.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/path-methods-map.h"
#include "hphp/runtime/ext/facts/path-symbols-map.h"
#include "hphp/runtime/ext/facts/string-ptr.h"
#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/sha1.h"
#include "hphp/util/sqlite-wrapper.h"

namespace HPHP {
namespace Facts {

struct UpdateDBWorkItem {
  std::string m_since;
  std::string m_clock;
  std::vector<folly::fs::path> m_alteredPaths;
  std::vector<folly::fs::path> m_deletedPaths;
  std::vector<FileFacts> m_alteredPathFacts;
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
template <typename S> struct SymbolMap {

  explicit SymbolMap(
      folly::fs::path root,
      DBData dbData,
      SQLite::OpenMode dbMode = SQLite::OpenMode::ReadWrite);
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
  Optional<Symbol<S, SymKind::Type>> getTypeName(const S& type);

  /**
   * Return the one and only definition for the given symbol.
   *
   * If the symbol is defined in no files, or in more than one file,
   * return nullptr.
   *
   * These methods may fill the map with information from the SQLite
   * DB, and as such may throw SQLite exceptions.
   */
  Path<S> getTypeFile(Symbol<S, SymKind::Type> type);
  Path<S> getTypeFile(const S& type);
  Path<S> getFunctionFile(Symbol<S, SymKind::Function> function);
  Path<S> getFunctionFile(const S& function);
  Path<S> getConstantFile(Symbol<S, SymKind::Constant> constant);
  Path<S> getConstantFile(const S& constant);
  Path<S> getTypeAliasFile(Symbol<S, SymKind::Type> typeAlias);
  Path<S> getTypeAliasFile(const S& typeAlias);

  /**
   * Return all symbols in the repo, along with the relative path defining
   * them. For large repos, this will be slow.
   *
   * Results are returned in an unspecified order. If a symbol is defined in
   * more than one path, the symbol will appear multiple times in the returned
   * vector, with each path defining it.
   */
  std::vector<std::pair<Symbol<S, SymKind::Type>, Path<S>>> getAllTypes();
  std::vector<std::pair<Symbol<S, SymKind::Function>, Path<S>>>
  getAllFunctions();
  std::vector<std::pair<Symbol<S, SymKind::Constant>, Path<S>>>
  getAllConstants();
  std::vector<std::pair<Symbol<S, SymKind::Type>, Path<S>>> getAllTypeAliases();

  /**
   * Return all symbols of a given kind declared in the given path.
   *
   * These methods may fill the map with information from the SQLite
   * DB, and as such may throw SQLite exceptions.
   */
  std::vector<Symbol<S, SymKind::Type>> getFileTypes(Path<S> path);
  std::vector<Symbol<S, SymKind::Type>>
  getFileTypes(const folly::fs::path& path);

  std::vector<Symbol<S, SymKind::Function>> getFileFunctions(Path<S> path);
  std::vector<Symbol<S, SymKind::Function>>
  getFileFunctions(const folly::fs::path& path);

  std::vector<Symbol<S, SymKind::Constant>> getFileConstants(Path<S> path);
  std::vector<Symbol<S, SymKind::Constant>>
  getFileConstants(const folly::fs::path& path);

  std::vector<Symbol<S, SymKind::Type>> getFileTypeAliases(Path<S> path);
  std::vector<Symbol<S, SymKind::Type>>
  getFileTypeAliases(const folly::fs::path& path);

  /**
   * Return inheritance data about the given type
   */
  std::vector<Symbol<S, SymKind::Type>>
  getBaseTypes(Symbol<S, SymKind::Type> derivedType, DeriveKind kind);
  std::vector<Symbol<S, SymKind::Type>>
  getBaseTypes(const S& derivedType, DeriveKind kind);

  std::vector<Symbol<S, SymKind::Type>>
  getDerivedTypes(Symbol<S, SymKind::Type> baseType, DeriveKind kind);
  std::vector<Symbol<S, SymKind::Type>>
  getDerivedTypes(const S& baseType, DeriveKind kind);

  /**
   * Return all types which transitively extend, implement, or use the given
   * base type.
   *
   * `kinds` is a bitmask dictating whether we should follow classes,
   * interfaces, enums, or traits. If one of these kinds is missing, we don't
   * include anything of that kind, or any of their subtypes.
   *
   * `deriveKinds` is a bitmask dictating whether we should follow `extends` or
   * `require extends` relationships.
   */
  using DerivedTypeInfo =
      std::tuple<Symbol<S, SymKind::Type>, Path<S>, TypeKind, TypeFlagMask>;
  std::vector<DerivedTypeInfo> getTransitiveDerivedTypes(
      Symbol<S, SymKind::Type> baseType,
      TypeKindMask kinds = kTypeKindAll,
      DeriveKindMask deriveKinds = kDeriveKindAll);
  std::vector<DerivedTypeInfo> getTransitiveDerivedTypes(
      const S& baseType,
      TypeKindMask kinds = kTypeKindAll,
      DeriveKindMask deriveKinds = kDeriveKindAll);

  /**
   * Return the attributes of a type
   */
  std::vector<Symbol<S, SymKind::Type>>
  getAttributesOfType(Symbol<S, SymKind::Type> type);
  std::vector<Symbol<S, SymKind::Type>> getAttributesOfType(const S& type);

  /**
   * Return the types and type aliases with a given attribute
   */
  std::vector<Symbol<S, SymKind::Type>>
  getTypesAndTypeAliasesWithAttribute(Symbol<S, SymKind::Type> attr);
  std::vector<Symbol<S, SymKind::Type>>
  getTypesAndTypeAliasesWithAttribute(const S& attr);

  /**
   * Return the attributes of a method
   */
  std::vector<Symbol<S, SymKind::Type>> getAttributesOfMethod(
      Symbol<S, SymKind::Type> type, Symbol<S, SymKind::Function> method);
  std::vector<Symbol<S, SymKind::Type>>
  getAttributesOfMethod(const S& type, const S& method);

  /**
   * Return the methods with a given attribute
   */
  std::vector<MethodDecl<S>>
  getMethodsWithAttribute(Symbol<S, SymKind::Type> attr);
  std::vector<MethodDecl<S>> getMethodsWithAttribute(const S& attr);

  /**
   * Return the attributes of a file
   */
  std::vector<Symbol<S, SymKind::Type>> getAttributesOfFile(Path<S> path);

  /**
   * Return the files with a given attribute
   */
  std::vector<Path<S>>
  getFilesWithAttribute(Symbol<S, SymKind::Type> attr);
  std::vector<Path<S>> getFilesWithAttribute(const S& attr);

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
      Symbol<S, SymKind::Type> type, Symbol<S, SymKind::Type> attribute);
  std::vector<folly::dynamic>
  getTypeAttributeArgs(const S& type, const S& attribute);

  std::vector<folly::dynamic> getMethodAttributeArgs(
      Symbol<S, SymKind::Type> type,
      Symbol<S, SymKind::Function> method,
      Symbol<S, SymKind::Type> attribute);
  std::vector<folly::dynamic>
  getMethodAttributeArgs(const S& type, const S& method, const S& attribute);

  std::vector<folly::dynamic> getFileAttributeArgs(
      Path<S> path,
      Symbol<S, SymKind::Type> attribute);
  std::vector<folly::dynamic> getFileAttributeArgs(Path<S> path, const S& attribute);

  /**
   * Return whether the given type is, for example, a class or interface.
   *
   * Return `TypeKind::Unknown` if the given type does not have a unique
   * definition or is not an autoloadable type at all.
   */
  TypeKind getKind(Symbol<S, SymKind::Type> type);
  TypeKind getKind(const S& type);

  bool isTypeAbstract(Symbol<S, SymKind::Type> type);
  bool isTypeAbstract(const S& type);

  bool isTypeFinal(Symbol<S, SymKind::Type> type);
  bool isTypeFinal(const S& type);

  /**
   * Return a hash representing the given path's last-known checksum.
   */
  Optional<SHA1> getSha1Hash(Path<S> path) const;

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
      std::string_view since,
      std::string_view clock,
      std::vector<folly::fs::path> alteredPaths,
      std::vector<folly::fs::path> deletedPaths,
      std::vector<FileFacts> alteredPathFacts); // throws(SQLiteExc)

  /**
   * Return an opaque token representing how up to date this map is.
   *
   * This token originated from Watchman.
   */
  std::string getClock() const noexcept;

  /**
   * Return an opaque token representing how up to date the SQLite DB is.
   *
   * This token originated from Watchman.
   */
  std::string dbClock() const; // throws(SQLiteExc)

  /**
   * Return the one and only path where `symbol` is defined.
   *
   * If `symbol` is not defined, or if `symbol` is defined in more
   * than one path, return nullptr.
   *
   * Query the DB if there is no data about `symbol` in the given
   * symbolMap, and add any information found to the corresponding symbolMap if
   * so.
   */
  template <SymKind k>
  Path<S> getOnlyPath(Symbol<S, k> symbol); // throws(SQLiteExc)

  /**
   * Return all the symbols of the kind corresponding to symbolMap
   * defined in the given path.
   *
   * Query the DB if there is no data about `path` in the given
   * symbolMap, and add any information found to the corresponding symbolMap if
   * so.
   */
  template <SymKind k>
  const typename PathToSymbolsMap<S, k>::PathSymbolMap::ValuesSet&
  getPathSymbols(Path<S> path);

  void waitForDBUpdate();

  /**
   * Return every path we know about.
   */
  hphp_hash_set<Path<S>> getAllPaths() const;

  /**
   * Return a map from path to hash for every path we know about.
   */
  hphp_hash_map<Path<S>, SHA1> getAllPathsWithHashes() const;

  std::shared_ptr<folly::Executor> m_exec;

  struct Data {
    /**
     * A Watchman clock representing how up-to-date this map is.
     *
     * If this string is empty, then this map has not yet updated.
     */
    std::string m_clock;

    /**
     * Maps between symbols and the paths defining them.
     */
    PathToSymbolsMap<S, SymKind::Type> m_typePath;
    PathToSymbolsMap<S, SymKind::Function> m_functionPath;
    PathToSymbolsMap<S, SymKind::Constant> m_constantPath;
    PathToMethodsMap<S> m_methodPath;

    /**
     * Future chain and queue holding the work that needs to be done before the
     * DB is considered up to date.
     */
    std::queue<UpdateDBWorkItem> m_updateDBWork;
    folly::FutureSplitter<folly::Unit> m_updateDBFuture{folly::makeFuture()};

    struct TypeInfo {
      using KindAndFlags = std::pair<TypeKind, int>;

      void setKindAndFlags(
          Symbol<S, SymKind::Type> type,
          Path<S> path,
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

      Optional<std::pair<TypeKind, int>>
      getKindAndFlags(Symbol<S, SymKind::Type> type, Path<S> path) const {
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
          Symbol<S, SymKind::Type>,
          std::vector<std::pair<Path<S>, KindAndFlags>>>
          m_map;
    } m_typeKind;

    /**
     * True if the file exists, false if the file is deleted.
     */
    hphp_hash_map<Path<S>, bool> m_fileExistsMap;

    /**
     * Maps between types and their subtypes/supertypes.
     */
    InheritanceInfo<S> m_inheritanceInfo;

    /**
     * Maps between types and the attributes that decorate them.
     */
    AttributeMap<S, TypeDecl<S>> m_typeAttrs;

    /**
     * Maps between methods and the attributes that decorate them.
     */
    AttributeMap<S, MethodDecl<S>> m_methodAttrs;

    /**
     * Maps between files and the attributes that decorate them.
     */
    AttributeMap<S, Path<S>> m_fileAttrs;

    /**
     * 40-byte hex strings representing the last-known SHA1 checksums of
     * each file we've seen
     */
    hphp_hash_map<Path<S>, SHA1> m_sha1Hashes;

    /**
     * Parse the given path and store all its data in the map.
     */
    void updatePath(Path<S> path, FileFacts facts);

    /**
     * Remove the given path from the map, along with all data associated with
     * the path.
     */
    void removePath(AutoloadDB& db, SQLiteTxn& txn, Path<S> path);
  };

private:
  /**
   * Update the DB on the time interval beginning at `since` and
   * ending at `clock`.
   *
   * We throw an UpdateExc if the `since` token does not match the clock
   * in the DB, and we don't catch SQLiteExc from the underlying SQLite layer.
   */
  void updateDB(
      std::string_view since,
      std::string_view clock,
      const std::vector<folly::fs::path>& alteredPaths,
      const std::vector<folly::fs::path>& deletedPaths,
      const std::vector<FileFacts>& alteredPathFacts) const; // throws

  /**
   * Replace all facts in the DB with in-memory facts about the given path.
   */
  void updateDBPath(
      AutoloadDB& db,
      SQLiteTxn& txn,
      const folly::fs::path& path,
      const FileFacts& facts) const;

  /**
   * True iff the given path is known to be deleted.
   */
  bool isPathDeleted(Path<S> path) const noexcept;

  /**
   * Mark `derivedType` as inheriting from each of the `baseTypes`.
   */
  void setBaseTypes(
      Path<S> path,
      Symbol<S, SymKind::Type> derivedType,
      std::vector<std::string> baseTypes);

  /**
   * Load information from the DB about who the given `derivedType` inherits.
   */
  void loadBaseTypesFromDB(
      AutoloadDB& db,
      SQLiteTxn& txn,
      Path<S> path,
      Symbol<S, SymKind::Type> derivedType);

  /**
   * Helper function to read from and write to m_synchronizedData.
   *
   * readFn: ((const Data&) -> Optional<Ret>)
   * GetFromDBFn: ((AutoloadDB&, SQLiteTxn&) -> DataFromDB)
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
  AutoloadDB& getDB() const;

  /**
   * Return the type's kind (class/interface/enum) along with an
   * abstract/final bitmask.
   */
  std::pair<TypeKind, TypeFlagMask>
  getKindAndFlags(Symbol<S, SymKind::Type> type);
  std::pair<TypeKind, TypeFlagMask>
  getKindAndFlags(Symbol<S, SymKind::Type> type, Path<S> path);

  std::atomic<bool> m_useDB = false;
  // Used to prioritize updates over caching. Pending updates increment this
  // count, while caching only occurs if this count is at 0.
  std::atomic<size_t> m_updatesInFlight = 0;

  folly::Synchronized<Data, folly::SharedMutexWritePriority> m_syncedData;

  const folly::fs::path m_root;
  const std::string m_schemaHash;
  const DBData m_dbData;
  const SQLite::OpenMode m_dbMode{SQLite::OpenMode::ReadWrite};
};

} // namespace Facts
} // namespace HPHP
