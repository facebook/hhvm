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

#include <filesystem>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <folly/Function.h>
#include <folly/json/dynamic.h>

#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/util/optional.h"

namespace HPHP {
namespace Facts {

/**
 * Holds prepared statements to interact with the autoload SQLite DB.
 *
 * Instantiated as a thread-local in `t_adb`.
 */
struct AutoloadDB {
 protected:
  AutoloadDB() = default;
  AutoloadDB(const AutoloadDB&) = default;
  AutoloadDB(AutoloadDB&&) noexcept = default;
  AutoloadDB& operator=(const AutoloadDB&) = default;
  AutoloadDB& operator=(AutoloadDB&&) noexcept = default;
  virtual ~AutoloadDB() = default;

 public:
  template <typename T>
  class MultiResult;

  /**
   * Function which returns a reference to an AutoloadDB.
   */
  using Opener = folly::Function<std::shared_ptr<AutoloadDB>() const>;

  struct KindAndFlags {
    TypeKind m_kind;
    TypeFlagMask m_flags;
  };

  struct SymbolPath {
    std::string m_symbol;
    std::filesystem::path m_path;
  };

  struct MethodPath {
    std::string m_type;
    std::string m_method;
    std::filesystem::path m_path;
  };

  struct PathAndHash {
    std::filesystem::path m_path;
    std::string m_hash;
  };

  struct PathAndAttrVal {
    std::filesystem::path m_path;
    HPHP::Optional<folly::dynamic> m_AttrVal;
  };

  /**
   * True iff you cannot write to this DB.
   */
  virtual bool isReadOnly() const = 0;

  /**
   * Actually save all changes you've made.
   *
   * Changes will not persist unless you call this function before destroying
   * the AutoloadDB!
   */
  virtual void commit() = 0;

  virtual void insertPath(const std::filesystem::path& path) = 0;
  virtual void erasePath(const std::filesystem::path& path) = 0;

  // Hashes
  virtual void insertSha1Hex(
      const std::filesystem::path& path,
      const Optional<std::string>& sha1hex) = 0;
  virtual std::string getSha1Hex(const std::filesystem::path& path) = 0;

  // Types
  virtual void insertType(
      std::string_view type,
      const std::filesystem::path& path,
      TypeKind kind,
      TypeFlagMask flags) = 0;
  virtual std::vector<std::filesystem::path> getTypePath(
      std::string_view type) = 0;
  virtual std::vector<std::string> getPathTypes(
      const std::filesystem::path& path) = 0;

  /**
   * Return the kind of the given type defined at the given path.
   *
   * If the type is not defined in the given path, return TypeKind::Unknown.
   */
  virtual KindAndFlags getKindAndFlags(
      std::string_view type,
      const std::filesystem::path& path) = 0;

  // Inheritance
  virtual void insertBaseType(
      const std::filesystem::path& path,
      std::string_view derivedType,
      DeriveKind kind,
      std::string_view baseType) = 0;
  virtual std::vector<std::string> getBaseTypes(
      const std::filesystem::path& path,
      std::string_view derivedType,
      DeriveKind kind) = 0;

  /**
   * Return all types extending the given baseType, along with the paths which
   * claim the derived type extends this baseType.
   *
   * Returns [(pathWhereDerivedTypeExtendsBaseType, derivedType)]
   */
  virtual std::vector<SymbolPath> getDerivedTypes(
      std::string_view baseType,
      DeriveKind kind) = 0;

  // Attributes

  virtual void insertTypeAttribute(
      const std::filesystem::path& path,
      std::string_view type,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) = 0;

  virtual void insertMethodAttribute(
      const std::filesystem::path& path,
      std::string_view type,
      std::string_view method,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) = 0;

  virtual void insertFileAttribute(
      const std::filesystem::path& path,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) = 0;

  virtual std::vector<std::string> getAttributesOfType(
      std::string_view type,
      const std::filesystem::path& path) = 0;

  virtual std::vector<std::string> getAttributesOfMethod(
      std::string_view type,
      std::string_view method,
      const std::filesystem::path& path) = 0;

  virtual std::vector<std::string> getAttributesOfFile(
      const std::filesystem::path& path) = 0;

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
      std::string_view path,
      std::string_view attributeName) = 0;

  virtual std::vector<SymbolPath> getTypesWithAttribute(
      std::string_view attributeName) = 0;
  virtual std::vector<SymbolPath> getTypeAliasesWithAttribute(
      std::string_view attributeName) = 0;

  virtual std::vector<MethodPath> getPathMethods(std::string_view path) = 0;

  virtual std::vector<MethodPath> getMethodsWithAttribute(
      std::string_view attributeName) = 0;

  virtual std::vector<std::filesystem::path> getFilesWithAttribute(
      std::string_view attributeName) = 0;
  virtual std::vector<std::filesystem::path> getFilesWithAttributeAndAnyValue(
      std::string_view attributeName,
      const folly::dynamic& attributeValue) = 0;
  virtual std::vector<PathAndAttrVal> getFilesAndAttrValsWithAttribute(
      const std::string_view attribute_name) = 0;

  // Functions
  virtual void insertFunction(
      std::string_view function,
      const std::filesystem::path& path) = 0;
  virtual std::vector<std::pair<std::filesystem::path, std::string>>
  getFunctionPath(std::string_view function) = 0;
  virtual std::vector<std::string> getPathFunctions(
      const std::filesystem::path& path) = 0;

  // Constants
  virtual void insertConstant(
      std::string_view constant,
      const std::filesystem::path& path) = 0;
  virtual std::vector<std::filesystem::path> getConstantPath(
      std::string_view constant) = 0;
  virtual std::vector<std::string> getPathConstants(
      const std::filesystem::path& path) = 0;

  // Modules
  virtual void insertModule(
      std::string_view module,
      const std::filesystem::path& path) = 0;
  virtual std::vector<std::filesystem::path> getModulePath(
      std::string_view module) = 0;
  virtual std::vector<std::string> getPathModules(
      const std::filesystem::path& path) = 0;

  /**
   * Return a list of all paths defined in the given root, as absolute paths.
   *
   * Paths come paired with the last known SHA1 hash of the file.
   *
   * Returns results in the form of a lazy generator.
   */
  virtual MultiResult<PathAndHash> getAllPathsAndHashes() = 0;

  virtual void insertClock(const Clock& clock) = 0;
  virtual Clock getClock() = 0;

  /**
   * This should be run once, after initially building a DB. It may take a
   * while.
   *
   * If this DB is backed by SQLite, this method runs the ANALYZE command to
   * record stats about the DB's tables and indices. This allows SQLite to
   * create much better query plans.
   *
   * https://sqlite.org/lang_analyze.html
   */
  virtual void runPostBuildOptimizations() = 0;

  /**
   * Lazy generator to return results from the DB.
   */
  template <typename T>
  class MultiResult {
    /**
     * Advance the internal iterator by one step, then check if we're at
     * the end. If we are at the end, return `std::nullopt`. If we
     * aren't at the end, return the value pointed to by the iterator.
     */
    using RowFn = folly::Function<Optional<T>()>;

    class Iterator {
      using iterator_category = std::forward_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = T;
      using pointer = value_type*;
      using reference = value_type&;

     public:
      Iterator(RowFn& rowFn, bool done) : m_rowFn{rowFn}, m_done{done} {
        if (!m_done) {
          m_current = m_rowFn();
          if (!m_current) {
            m_done = true;
          }
        }
      }

      Iterator& operator++() noexcept {
        if (m_done) {
          return *this;
        }
        m_current = m_rowFn();
        if (!m_current) {
          m_done = true;
        }
        return *this;
      }

      value_type operator*() {
        assertx(!m_done);
        if (!m_current) {
          m_current = m_rowFn();
        }
        return *m_current;
      }

      explicit operator bool() const {
        return !m_done;
      }

      friend bool operator==(const Iterator& a, const Iterator& b) noexcept {
        return a.m_done == b.m_done;
      }
      friend bool operator!=(const Iterator& a, const Iterator& b) noexcept {
        return !(a == b);
      }

     private:
      RowFn& m_rowFn;
      Optional<T> m_current;
      bool m_done{false};
    };

   public:
    explicit MultiResult(RowFn rowFn) : m_rowFn{std::move(rowFn)} {}

    Iterator begin() {
      return {m_rowFn, false};
    }
    Iterator end() {
      return {m_rowFn, true};
    }

   private:
    RowFn m_rowFn;
  };
};

} // namespace Facts
} // namespace HPHP
