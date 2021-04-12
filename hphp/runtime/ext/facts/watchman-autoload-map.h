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

#include <memory>
#include <optional>

#include <folly/dynamic.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/futures/Future.h>
#include <folly/futures/FutureSplitter.h>

#include <watchman/cppclient/WatchmanClient.h>

#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/facts/path-and-hash.h"
#include "hphp/runtime/ext/facts/symbol-map.h"
#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/runtime/ext/facts/watchman.h"

namespace HPHP {
namespace Facts {

/**
 * Internal representation of filters used to query for subtypes
 */
struct InheritanceFilterData;
struct KindFilterData;
struct AttributeFilterData;

/**
 * The actual AutoloadMap. Stores one SymbolMap for each kind of symbol.
 */
struct WatchmanAutoloadMap final
    : public FactsStore,
      public std::enable_shared_from_this<WatchmanAutoloadMap> {

  /**
   * Create in dynamic mode, learning about changed files from Watchman and
   * updating the DB at `dbPath` accordingly.
   */
  WatchmanAutoloadMap(
      folly::fs::path root,
      folly::fs::path dbPath,
      folly::dynamic queryExpr,
      Watchman& watchmanClient);

  /**
   * Create in static mode, where we trust the DB at `dbPath` and never modify
   * it.
   */
  WatchmanAutoloadMap(folly::fs::path root, folly::fs::path dbPath);

  ~WatchmanAutoloadMap() override;
  WatchmanAutoloadMap(const WatchmanAutoloadMap&) = delete;
  WatchmanAutoloadMap(WatchmanAutoloadMap&&) noexcept = delete;
  WatchmanAutoloadMap& operator=(const WatchmanAutoloadMap&) = delete;
  WatchmanAutoloadMap& operator=(WatchmanAutoloadMap&&) noexcept = delete;

  /**
   * This AutoloadMap is capable of building itself by invoking a Watchman
   * query and running HackC to parse changed files.
   */
  bool isNative() const noexcept override {
    return true;
  }

  /**
   * Guarantee the map is at least as up-to-date as the codebase was
   * when update() was called.
   *
   * throws SQLiteExc or WatchmanAutoloadMapExc.
   */
  void ensureUpdated() override;

  Array getAllFiles() const override;

  Variant getTypeName(const String& type) override;
  Variant getKind(const String& type) override;
  bool isTypeAbstract(const String& type) override;
  bool isTypeFinal(const String& type) override;

  std::optional<String> getTypeFile(const String& type) override;

  std::optional<String> getFunctionFile(const String& function) override;

  std::optional<String> getConstantFile(const String& constant) override;

  std::optional<String> getTypeAliasFile(const String& typeAlias) override;

  Array getAllPaths() const;

  Array getFileTypes(const String& path) override;
  Array getFileFunctions(const String& path) override;
  Array getFileConstants(const String& path) override;
  Array getFileTypeAliases(const String& path) override;

  Array
  getBaseTypes(const String& derivedType, const Variant& filters) override;

  Array
  getDerivedTypes(const String& baseType, const Variant& filters) override;

  Array getTransitiveDerivedTypes(
      const String& baseType, const Variant& filters) override;

  Array getTypesWithAttribute(const String& attr) override;
  Array getTypeAliasesWithAttribute(const String& attr) override;
  Array getTypeAttributes(const String& type) override;
  Array getTypeAttrArgs(const String& type, const String& attr) override;

  Array getAllTypes() override;
  Array getAllFunctions() override;
  Array getAllConstants() override;
  Array getAllTypeAliases() override;

  bool canHandleFailure() const override {
    return false;
  }

  /**
   * Update whenever a file in the filesystem changes.
   */
  void subscribe();

  AutoloadMap::Result handleFailure(
      KindOf UNUSED kind,
      const String& UNUSED className,
      const Variant& UNUSED err) const override {
    return AutoloadMap::Result::Failure;
  }

private:
  /**
   * Query Watchman to see changed files, update our internal data structures,
   * and resolve the returned future once the map is at least as up-to-date as
   * the time update() was called.
   */
  folly::Future<folly::Unit> update();
  folly::Future<folly::Unit> updateImpl();

  /**
   * Calculate the paths which have changed since the last Watchman
   * update when Watchman gives us the full state of the world.
   */
  std::pair<std::vector<PathAndHash>, std::vector<folly::fs::path>>
  getFreshDelta(const folly::dynamic& result) const;

  /**
   * Calculate the paths which have changed since the last Watchman
   * update when Watchman gives us an incremental update.
   */
  std::pair<std::vector<PathAndHash>, std::vector<folly::fs::path>>
  getIncrementalDelta(const folly::dynamic& result) const;

  Array
  getBaseTypes(const String& derivedType, const InheritanceFilterData& filters);
  Array
  getDerivedTypes(const String& baseType, const InheritanceFilterData& filters);
  Array getTransitiveDerivedTypes(
      const String& baseType, const InheritanceFilterData& filters);

  std::mutex m_mutex;
  folly::CPUThreadPoolExecutor m_updateExec;

  std::atomic<bool> m_closing{false};
  folly::fs::path m_root;
  SymbolMap<StringData> m_map;

  /**
   * Updates this AutoloadMap using Watchman to track changed files.
   *
   * If this is `std::nullopt`, then we will treat the AutoloadMap as static.
   */
  struct WatchmanData {
    folly::dynamic m_queryExpr;
    Watchman& m_watchmanClient;
    folly::Future<watchman::SubscriptionPtr> m_subscribeFuture{nullptr};
    folly::FutureSplitter<folly::Unit> m_updateFuture{folly::makeFuture()};
  };
  std::optional<WatchmanData> m_watchmanData;

  template <SymKind k, typename TLambda>
  Array getFileSymbols(const String& path, TLambda lambda);

  template <SymKind k, class T>
  std::optional<String> getSymbolFile(const String& symbol, T lambda);

  /**
   * Filter the given `types` down to only those of the given `kinds`, such as
   * class, interface, enum, or trait.
   *
   * The Hack type of `kinds` is `keyset<HH\Facts\TypeKind>`.
   */
  std::vector<Symbol<StringData, SymKind::Type>> filterByKind(
      std::vector<Symbol<StringData, SymKind::Type>> types,
      const KindFilterData& kinds);

  /**
   * Filter the given `types` down to only those with the given attributes.
   */
  template <typename T>
  std::vector<T>
  filterByAttribute(std::vector<T> types, const AttributeFilterData& filter);
  template <typename T, typename TypeGetFn>
  std::vector<T> filterByAttribute(
      std::vector<T> types,
      const AttributeFilterData& filter,
      TypeGetFn typeGetFn);
};

} // namespace Facts
} // namespace HPHP
