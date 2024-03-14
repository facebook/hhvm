/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <filesystem>
#include <string_view>
#include <utility>
#include <vector>

#include "hphp/runtime/base/type-string.h"

#include "hphp/util/assertions.h"

namespace folly {
  struct dynamic;
}

namespace HPHP {

struct Variant;
struct RepoUnitInfo;

struct AutoloadMap {
  /**
   * Keep enum values in sync with `HH\Facts\SymbolKind` in `ext_facts.php`
   */
  enum class KindOf : int {
    Type = 1,
    Function = 2,
    Constant = 3,
    Module = 4,
    TypeAlias = 5,
    TypeOrTypeAlias = 6,
  };

  struct FileResult {
    String m_path;
    const RepoUnitInfo* m_info;

    FileResult(String path, const RepoUnitInfo* info)
      : m_path(path), m_info(info) {}
    explicit FileResult(String path): m_path(path), m_info(nullptr) {}

    FileResult& operator=(String path) {
      m_path = path;
      m_info = nullptr;
      return *this;
    }
  };

  struct Holder {
    Holder() = default;
    Holder(AutoloadMap* map, std::function<void()>&& release)
      : m_map(map)
      , m_release(std::move(release))
    {}
    Holder(Holder&&) = default;
    Holder& operator=(Holder&&) = default;
    ~Holder() { if (m_release) m_release(); }

    operator bool() const { return !!m_map; }
    AutoloadMap* operator->() const { return m_map; }
    AutoloadMap* get() const { return m_map; }
  private:
    AutoloadMap* m_map{nullptr};
    std::function<void()> m_release{nullptr};
  };

  AutoloadMap() = default;
  AutoloadMap(const AutoloadMap&) = default;
  AutoloadMap(AutoloadMap&&) noexcept = default;
  AutoloadMap& operator=(const AutoloadMap&) = default;
  AutoloadMap& operator=(AutoloadMap&&) = default;
  virtual ~AutoloadMap() = default;

  /**
   * Block until the AutoloadMap is up-to-date.
   *
   * May throw an exception if updating failed.
   */
  virtual void ensureUpdated() = 0;

  /**
   * Returns a Holder object which wraps a native AutoloadMap in a manner which
   * is safe to be shared across threads. For non-native or non-shareable maps
   * returns an empty holder.
   */
  virtual Holder getNativeHolder() noexcept = 0;

  /**
   * Given a symbol and the kind we're looking for, return the absolute path
   * of the file defining that symbol.
   *
   * Return None if the file is defined in zero or more than one place.
   *
   * In general this is only safe to call on request threads because the
   * resulting string (absolute path) may be allocated on the request heap.
   */
  Optional<FileResult> getFile(KindOf kind, const String& symbol) {
    switch (kind) {
      case AutoloadMap::KindOf::TypeOrTypeAlias: return getTypeOrTypeAliasFile(symbol);
      case AutoloadMap::KindOf::Type: return getTypeFile(symbol);
      case AutoloadMap::KindOf::Function: return getFunctionFile(symbol);
      case AutoloadMap::KindOf::Constant: return getConstantFile(symbol);
      case AutoloadMap::KindOf::TypeAlias: return getTypeAliasFile(symbol);
      case AutoloadMap::KindOf::Module: return getModuleFile(symbol);
    }
    not_reached();
  }

  /**
   * This overload takes std::string_view and returns std::filesystem::path
   * to avoid request heap allocation.
   */
  Optional<std::filesystem::path> getFile(KindOf kind, std::string_view symbol) {
    switch (kind) {
      case AutoloadMap::KindOf::TypeOrTypeAlias: return getTypeOrTypeAliasFile(symbol);
      case AutoloadMap::KindOf::Type: return getTypeFile(symbol);
      case AutoloadMap::KindOf::Function: return getFunctionFile(symbol);
      case AutoloadMap::KindOf::Constant: return getConstantFile(symbol);
      case AutoloadMap::KindOf::TypeAlias: return getTypeAliasFile(symbol);
      case AutoloadMap::KindOf::Module: return getModuleFile(symbol);
    }
    not_reached();
  }

  /**
   * Map symbols to files
   */
  virtual Optional<FileResult> getTypeOrTypeAliasFile(const String& typeName) = 0;
  virtual Optional<FileResult> getTypeFile(const String& typeName) = 0;
  virtual Optional<FileResult> getFunctionFile(const String& functionName) = 0;
  virtual Optional<FileResult> getConstantFile(const String& constantName) = 0;
  virtual Optional<FileResult> getTypeAliasFile(const String& aliasName) = 0;
  virtual Optional<FileResult> getModuleFile(const String& moduleName) = 0;

  virtual Optional<std::filesystem::path> getTypeOrTypeAliasFile(std::string_view name) = 0;
  virtual Optional<std::filesystem::path> getTypeFile(std::string_view name) = 0;
  virtual Optional<std::filesystem::path> getFunctionFile(std::string_view name) = 0;
  virtual Optional<std::filesystem::path> getConstantFile(std::string_view name) = 0;
  virtual Optional<std::filesystem::path> getTypeAliasFile(std::string_view name) = 0;
  virtual Optional<std::filesystem::path> getModuleFile(std::string_view name) = 0;

  /**
   * Map path to symbols
   */
  virtual Array getFileTypes(const String& path) = 0;
  virtual Array getFileFunctions(const String& path) = 0;
  virtual Array getFileConstants(const String& path) = 0;
  virtual Array getFileTypeAliases(const String& path) = 0;
  virtual Array getFileModules(const String& path) = 0;
};

/**
 * An AutoloadMap which can also return data not directly related to
 * autoloading.
 */
struct FactsStore : public AutoloadMap {

  FactsStore() = default;
  FactsStore(const FactsStore&) = default;
  FactsStore(FactsStore&&) = default;
  FactsStore& operator=(const FactsStore&) = default;
  FactsStore& operator=(FactsStore&&) noexcept = default;
  virtual ~FactsStore() override = default;

  /**
   * Unsubscribe from any watchers, and finish any updates.
   *
   * Once you call this function, no new requests can use this FactsStore.
   * Calls to `ensureUpdated()` will throw exceptions.
   */
  virtual void close() = 0;

  /**
   * Return the correctly-capitalized name of `type`.
   *
   * Return `null` if `type` is not defined, or if it is defined in more than
   * one file.
   */
  virtual Variant getTypeName(const String& type) = 0;

  /**
   * Return whether the given type is a class, enum, interface, or trait.
   *
   * Return `null` if given none of the above.
   */
  virtual Variant getKind(const String& type) = 0;

  /**
   * Return true iff the type cannot be constructed.
   */
  virtual bool isTypeAbstract(const String& type) = 0;

  /**
   * Return true iff no other type can inherit from this type.
   */
  virtual bool isTypeFinal(const String& type) = 0;

  /**
   * Return all types in the repo which the given type extends.
   */
  virtual Array getBaseTypes(
      const String& derivedType, const Variant& filters) = 0;

  /**
   * Return all types in the repo which extend the given type.
   */
  virtual Array getDerivedTypes(
      const String& baseType, const Variant& filters) = 0;

  /**
   * Return all types decorated with the given attribute.
   */
  virtual Array getTypesWithAttribute(const String& attr) = 0;

  /**
   * Return all type aliases decorated with the given attribute.
   */
  virtual Array getTypeAliasesWithAttribute(const String& attr) = 0;

  /**
   * Return all methods decorated with the given attribute.
   */
  virtual Array getMethodsWithAttribute(const String& attr) = 0;

  /**
   * Return all files decorated with the given attribute.
   */
  virtual Array getFilesWithAttribute(const String& attr) = 0;

  /**
   * Return all files decorated with the given attribute and
   * the given value passed into that attribute.
   */
  virtual Array getFilesWithAttributeAndAnyValue(
      const String& attr, const folly::dynamic& value) = 0;


  /**
   * Return all files decorated with the given attribute including
   * the attribute value for that attribute.  If there is more than
   * one attr value for that attribute, the file will appear in the returned
   * list more than once.
   */
  virtual Array getFilesAndAttrValsWithAttribute(const String& attr) = 0;

  /**
   * Return all attributes decorating the given type.
   */
  virtual Array getTypeAttributes(const String& type) = 0;

  /**
   * Return all attributes decorating the given type alias.
   */
  virtual Array getTypeAliasAttributes(const String& typeAlias) = 0;

  /**
   * Return all attributes decorating the given method.
   */
  virtual Array getMethodAttributes(const String& type, const String& method) = 0;

  /**
   * Return all attributes decorating the given file.
   */
  virtual Array getFileAttributes(const String& file) = 0;

  /**
   * Return the arguments associated with the given type and attribute, as a
   * vec.
   *
   * If the given type does not have the given attribute, return an empty vec.
   */
  virtual Array getTypeAttrArgs(const String& type, const String& attr) = 0;

  /**
   * Return the arguments associated with the given type alias and attribute,
   * as a vec.
   *
   * If the given type alias does not have the given attribute, return an
   * empty vec.
   */
  virtual Array getTypeAliasAttrArgs(const String& type, const String& attr) = 0;

  /**
   * Return the arguments associated with the given method and attribute, as a
   * vec.
   *
   * If the given method does not have the given attribute, return an empty vec.
   */
  virtual Array getMethodAttrArgs(
      const String& type, const String& method, const String& attr) = 0;

  /**
   * Return the arguments associated with the given file and attribute, as a
   * vec.
   *
   * If the given file does not have the given attribute, return an empty vec.
   */
  virtual Array getFileAttrArgs(const String& file, const String& attr) = 0;
};

} // namespace HPHP
