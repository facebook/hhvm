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

#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <folly/dynamic.h>
#include <folly/experimental/io/FsUtil.h>

#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/sqlite-wrapper.h"
#include "hphp/util/thread-local.h"

namespace HPHP {
namespace Facts {

/**
 * Holds prepared statements to interact with the autoload SQLite DB.
 *
 * Instantiated as a thread-local in `t_adb`.
 */
struct AutoloadDB {

  template <typename TValue> class RowIter;

  AutoloadDB() = default;
  AutoloadDB(const AutoloadDB&) = default;
  AutoloadDB(AutoloadDB&&) noexcept = default;
  AutoloadDB& operator=(const AutoloadDB&) = default;
  AutoloadDB& operator=(AutoloadDB&&) noexcept = default;

  virtual ~AutoloadDB();

  virtual SQLiteTxn begin() = 0;

  virtual void insertPath(SQLiteTxn& txn, const folly::fs::path& path) = 0;
  virtual void erasePath(SQLiteTxn& txn, const folly::fs::path& path) = 0;

  // Hashes
  virtual void insertSha1Hex(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      const std::optional<std::string>& sha1hex) = 0;
  virtual std::string
  getSha1Hex(SQLiteTxn& txn, const folly::fs::path& path) = 0;

  // Types
  virtual void insertType(
      SQLiteTxn& txn,
      std::string_view type,
      const folly::fs::path& path,
      TypeKind kind,
      TypeFlagMask flags) = 0;
  virtual std::vector<folly::fs::path>
  getTypePath(SQLiteTxn& txn, std::string_view type) = 0;
  virtual std::vector<std::string>
  getPathTypes(SQLiteTxn& txn, const folly::fs::path& path) = 0;

  /**
   * Return the kind of the given type defined at the given path.
   *
   * If the type is not defined in the given path, return TypeKind::Unknown.
   */
  virtual std::pair<TypeKind, TypeFlagMask> getKindAndFlags(
      SQLiteTxn& txn, std::string_view type, const folly::fs::path& path) = 0;

  // Inheritance
  virtual void insertBaseType(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      std::string_view derivedType,
      DeriveKind kind,
      std::string_view baseType) = 0;
  virtual std::vector<std::string> getBaseTypes(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      std::string_view derivedType,
      DeriveKind kind) = 0;

  /**
   * Return all types extending the given baseType, along with the paths which
   * claim the derived type extends this baseType.
   *
   * Returns [(pathWhereDerivedTypeExtendsBaseType, derivedType)]
   */
  virtual std::vector<std::pair<folly::fs::path, std::string>> getDerivedTypes(
      SQLiteTxn& txn, std::string_view baseType, DeriveKind kind) = 0;

  // (type, path, kind, flags)
  using DerivedTypeInfo = std::tuple<
      const std::string_view,
      const std::string_view,
      TypeKind,
      TypeFlagMask>;
  virtual RowIter<DerivedTypeInfo> getTransitiveDerivedTypes(
      SQLiteTxn& txn,
      std::string_view baseType,
      TypeKindMask kinds = kTypeKindAll,
      DeriveKindMask deriveKinds = kDeriveKindAll) = 0;

  // Attributes
  virtual void insertTypeAttribute(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      std::string_view type,
      std::string_view attributeName,
      std::optional<int> attributePosition,
      const folly::dynamic* attributeValue) = 0;

  /**
   * Record stats about the DB's tables and indices:
   * https://sqlite.org/lang_analyze.html
   */
  virtual void analyze() = 0;

  virtual std::vector<std::string> getAttributesOfType(
      SQLiteTxn& txn, std::string_view type, const folly::fs::path& path) = 0;

  virtual std::vector<folly::dynamic> getAttributeArgs(
      SQLiteTxn& txn,
      std::string_view type,
      std::string_view path,
      std::string_view attributeName) = 0;

  virtual std::vector<std::pair<std::string, folly::fs::path>>
  getTypesWithAttribute(SQLiteTxn& txn, std::string_view attributeName) = 0;

  virtual std::string
  getTypeCorrectCase(SQLiteTxn& txn, std::string_view type) = 0;

  // Functions
  virtual void insertFunction(
      SQLiteTxn& txn,
      std::string_view function,
      const folly::fs::path& path) = 0;
  virtual std::vector<folly::fs::path>
  getFunctionPath(SQLiteTxn& txn, std::string_view function) = 0;
  virtual std::vector<std::string>
  getPathFunctions(SQLiteTxn& txn, const folly::fs::path& path) = 0;
  virtual std::string
  getFunctionCorrectCase(SQLiteTxn& txn, std::string_view function) = 0;

  // Constants
  virtual void insertConstant(
      SQLiteTxn& txn,
      std::string_view constant,
      const folly::fs::path& path) = 0;
  virtual std::vector<folly::fs::path>
  getConstantPath(SQLiteTxn& txn, std::string_view constant) = 0;
  virtual std::vector<std::string>
  getPathConstants(SQLiteTxn& txn, const folly::fs::path& path) = 0;

  /**
   * Return a list of all paths defined in the given root, as absolute paths.
   *
   * Paths come paired with the last known SHA1 hash of the file.
   *
   * Returns results in the form of a lazy generator.
   */
  using PathAndHash = std::pair<folly::fs::path, const std::string_view>;
  virtual RowIter<PathAndHash> getAllPathsAndHashes(SQLiteTxn& txn) = 0;

  /**
   * Return a list of all symbols and paths defined in the given root.
   *
   * Returns results in the form of a lazy generator.
   */
  using SymbolPath = std::pair<const std::string_view, folly::fs::path>;
  virtual RowIter<SymbolPath> getAllTypePaths(SQLiteTxn& txn) = 0;
  virtual RowIter<SymbolPath> getAllFunctionPaths(SQLiteTxn& txn) = 0;
  virtual RowIter<SymbolPath> getAllConstantPaths(SQLiteTxn& txn) = 0;

  virtual void insertClock(SQLiteTxn& txn, std::string_view clock) = 0;
  virtual std::string getClock(SQLiteTxn& txn) = 0;

  template <typename TValue> class RowIter {

  public:
    explicit RowIter(SQLiteQuery&& query, TValue (*fn)(SQLiteQuery&))
        : m_query{std::move(query)}, m_fn{fn} {
    }

    class iterator : std::iterator<std::input_iterator_tag, TValue> {
    public:
      iterator(RowIter& container, bool done)
          : m_container{container}, m_done{done} {
        ++(*this);
      }
      iterator& operator++() {
        if (m_done) {
          return *this;
        }
        m_container.m_query.step();
        if (m_container.m_query.done()) {
          m_done = true;
        }
        return *this;
      }
      TValue operator*() {
        return m_container.m_fn(m_container.m_query);
      }
      explicit operator bool() const {
        return !m_done && m_container.m_query.row();
      }
      bool operator==(const iterator& o) const {
        return m_done == o.m_done;
      }
      bool operator!=(const iterator& o) const {
        return !(*this == o);
      }

    private:
      RowIter& m_container;
      bool m_done;
    };

    friend class iterator;
    iterator begin() {
      return iterator{*this, false};
    }
    iterator end() {
      return iterator{*this, true};
    }

  private:
    SQLiteQuery m_query;
    TValue (*m_fn)(SQLiteQuery&);
  };
};

using AutoloadDBThreadLocal = hphp_hash_map<
    std::pair<std::string, SQLite::OpenMode>,
    std::unique_ptr<AutoloadDB>>;

extern THREAD_LOCAL(AutoloadDBThreadLocal, t_adb);

AutoloadDB& getDB(
    const folly::fs::path& dbPath,
    SQLite::OpenMode mode = SQLite::OpenMode::ReadWrite);

} // namespace Facts
} // namespace HPHP
