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
 * Metadata about where the DB should be and what permissions it should have.
 */
struct DBData {

  static DBData readOnly(folly::fs::path path) {
    return DBData{
        std::move(path),
        SQLite::OpenMode::ReadOnly,
        static_cast<::gid_t>(-1),
        0};
  }

  static DBData readWrite(folly::fs::path path, ::gid_t gid, ::mode_t perms) {
    return DBData{std::move(path), SQLite::OpenMode::ReadWrite, gid, perms};
  }

  bool operator==(const DBData& rhs) const;

  /**
   * Render the DBData as a string
   */
  std::string toString() const;

  /**
   * Hash the DBData into an int
   */
  size_t hash() const;

  folly::fs::path m_path;
  SQLite::OpenMode m_rwMode;
  ::gid_t m_gid;
  ::mode_t m_perms;

private:
  DBData() = delete;
  DBData(
      folly::fs::path path,
      SQLite::OpenMode rwMode,
      ::gid_t gid,
      ::mode_t perms);
};

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

  /**
   * Actually save all changes you've made.
   *
   * Changes will not persist unless you call this function before destroying
   * the AutoloadDB!
   */
  virtual void commit() = 0;

  virtual void insertPath(const folly::fs::path& path) = 0;
  virtual void erasePath(const folly::fs::path& path) = 0;

  // Hashes
  virtual void insertSha1Hex(
      const folly::fs::path& path, const Optional<std::string>& sha1hex) = 0;
  virtual std::string getSha1Hex(const folly::fs::path& path) = 0;

  // Types
  virtual void insertType(
      std::string_view type,
      const folly::fs::path& path,
      TypeKind kind,
      TypeFlagMask flags) = 0;
  virtual std::vector<folly::fs::path> getTypePath(std::string_view type) = 0;
  virtual std::vector<std::string>
  getPathTypes(const folly::fs::path& path) = 0;

  /**
   * Return the kind of the given type defined at the given path.
   *
   * If the type is not defined in the given path, return TypeKind::Unknown.
   */
  virtual std::pair<TypeKind, TypeFlagMask>
  getKindAndFlags(std::string_view type, const folly::fs::path& path) = 0;

  // Inheritance
  virtual void insertBaseType(
      const folly::fs::path& path,
      std::string_view derivedType,
      DeriveKind kind,
      std::string_view baseType) = 0;
  virtual std::vector<std::string> getBaseTypes(
      const folly::fs::path& path,
      std::string_view derivedType,
      DeriveKind kind) = 0;

  /**
   * Return all types extending the given baseType, along with the paths which
   * claim the derived type extends this baseType.
   *
   * Returns [(pathWhereDerivedTypeExtendsBaseType, derivedType)]
   */
  virtual std::vector<std::pair<folly::fs::path, std::string>>
  getDerivedTypes(std::string_view baseType, DeriveKind kind) = 0;

  // (type, path, kind, flags)
  using DerivedTypeInfo = std::tuple<
      const std::string_view,
      const std::string_view,
      TypeKind,
      TypeFlagMask>;
  virtual RowIter<DerivedTypeInfo> getTransitiveDerivedTypes(

      std::string_view baseType,
      TypeKindMask kinds = kTypeKindAll,
      DeriveKindMask deriveKinds = kDeriveKindAll) = 0;

  // Attributes

  virtual void insertTypeAttribute(
      const folly::fs::path& path,
      std::string_view type,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) = 0;

  virtual void insertMethodAttribute(
      const folly::fs::path& path,
      std::string_view type,
      std::string_view method,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) = 0;

  virtual void insertFileAttribute(
      const folly::fs::path& path,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) = 0;

  /**
   * Record stats about the DB's tables and indices:
   * https://sqlite.org/lang_analyze.html
   */
  virtual void analyze() = 0;

  virtual std::vector<std::string>
  getAttributesOfType(std::string_view type, const folly::fs::path& path) = 0;

  virtual std::vector<std::string> getAttributesOfMethod(
      std::string_view type,
      std::string_view method,
      const folly::fs::path& path) = 0;

  virtual std::vector<std::string>
  getAttributesOfFile(const folly::fs::path& path) = 0;

  virtual std::vector<folly::dynamic> getTypeAttributeArgs(
      std::string_view type,
      std::string_view path,
      std::string_view attributeName) = 0;

  virtual std::vector<folly::dynamic> getTypeAliasAttributeArgs(
      std::string_view typeAlias,
      std::string_view path,
      std::string_view attributeName) = 0;

  virtual std::vector<folly::dynamic> getMethodAttributeArgs(
      std::string_view type,
      std::string_view method,
      std::string_view path,
      std::string_view attributeName) = 0;

  virtual std::vector<folly::dynamic> getFileAttributeArgs(
      std::string_view path, std::string_view attributeName) = 0;

  struct TypeDeclaration {
    std::string m_type;
    folly::fs::path m_path;
  };
  virtual std::vector<TypeDeclaration>
  getTypesWithAttribute(std::string_view attributeName) = 0;
  virtual std::vector<TypeDeclaration>
  getTypeAliasesWithAttribute(std::string_view attributeName) = 0;

  struct MethodDeclaration {
    std::string m_type;
    std::string m_method;
    folly::fs::path m_path;
  };

  virtual std::vector<MethodDeclaration>
  getPathMethods(std::string_view path) = 0;

  virtual std::vector<MethodDeclaration>
  getMethodsWithAttribute(std::string_view attributeName) = 0;

  virtual std::vector<folly::fs::path>
  getFilesWithAttribute(std::string_view attributeName) = 0;

  // Functions
  virtual void
  insertFunction(std::string_view function, const folly::fs::path& path) = 0;
  virtual std::vector<folly::fs::path>
  getFunctionPath(std::string_view function) = 0;
  virtual std::vector<std::string>
  getPathFunctions(const folly::fs::path& path) = 0;

  // Constants
  virtual void
  insertConstant(std::string_view constant, const folly::fs::path& path) = 0;
  virtual std::vector<folly::fs::path>
  getConstantPath(std::string_view constant) = 0;
  virtual std::vector<std::string>
  getPathConstants(const folly::fs::path& path) = 0;

  /**
   * Return a list of all paths defined in the given root, as absolute paths.
   *
   * Paths come paired with the last known SHA1 hash of the file.
   *
   * Returns results in the form of a lazy generator.
   */
  using PathAndHash = std::pair<folly::fs::path, const std::string_view>;
  virtual RowIter<PathAndHash> getAllPathsAndHashes() = 0;

  /**
   * Return a list of all symbols and paths defined in the given root.
   *
   * Returns results in the form of a lazy generator.
   */
  using SymbolPath = std::pair<const std::string_view, folly::fs::path>;
  virtual RowIter<SymbolPath> getAllTypePaths() = 0;
  virtual RowIter<SymbolPath> getAllFunctionPaths() = 0;
  virtual RowIter<SymbolPath> getAllConstantPaths() = 0;

  virtual void insertClock(const Clock& clock) = 0;
  virtual Clock getClock() = 0;

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

AutoloadDB& getDB(const DBData& dbData);

} // namespace Facts
} // namespace HPHP
