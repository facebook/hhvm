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

#include <vector>

#include "hphp/runtime/base/type-string.h"
#include "hphp/util/assertions.h"

namespace folly {
  struct dynamic;
}

namespace HPHP {

struct Variant;

struct AutoloadMap {

  enum class Result {
    Failure,
    Success,
    StopAutoloading,
    RetryAutoloading
  };

  enum class KindOf {
    Type,
    Function,
    Constant,
    TypeAlias,
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
  virtual void ensureUpdated() {
  }

  /**
   * True iff this AutoloadMap knows which files contain which symbols without
   * needing to query userland Hack code. If we're using a native AutoloadMap,
   * we'll be able to use any symbol when the very first line of Hack code is
   * run.
   */
  virtual bool isNative() const noexcept = 0;

  /**
   * Given the name of a type and the kind of type we're looking for,
   * return the absolute path of the file defining that type.
   *
   * Return None if the file is defined in zero places or more than
   * one place.
   */
  Optional<String> getFile(KindOf kind,
                                  const String& typeName) {
    switch (kind) {
      case AutoloadMap::KindOf::Type:
        return getTypeFile(typeName);
      case AutoloadMap::KindOf::Function:
        return getFunctionFile(typeName);
      case AutoloadMap::KindOf::Constant:
        return getConstantFile(typeName);
      case AutoloadMap::KindOf::TypeAlias:
        return getTypeAliasFile(typeName);
    }
    not_reached();
  }

  Array getSymbols(KindOf kind,
                   const String& path) {
    switch (kind) {
      case AutoloadMap::KindOf::Type:
        return getFileTypes(path);
      case AutoloadMap::KindOf::Function:
        return getFileFunctions(path);
      case AutoloadMap::KindOf::Constant:
        return getFileConstants(path);
      case AutoloadMap::KindOf::TypeAlias:
        return getFileTypeAliases(path);
    }
    not_reached();
  }

  /**
   * Get all filepaths known to the autoloader at this point in time.
   *
   * Each path must be an absolute path with all symlinks dereferenced.
   */
  virtual Array getAllFiles() const = 0;

  /**
   * Map symbols to files
   */
  virtual Optional<String> getTypeFile(
      const String& typeName) = 0;
  virtual Optional<String> getFunctionFile(
      const String& functionName) = 0;
  virtual Optional<String> getConstantFile(
      const String& constantName) = 0;
  virtual Optional<String> getTypeAliasFile(
      const String& typeAliasName) = 0;

  /**
   * Map path to symbols
   */
  virtual Array getFileTypes(const String& path) = 0;
  virtual Array getFileFunctions(const String& path) = 0;
  virtual Array getFileConstants(const String& path) = 0;
  virtual Array getFileTypeAliases(const String& path) = 0;

  virtual bool canHandleFailure() const = 0;
  virtual Result handleFailure(KindOf kind, const String& className,
                               const Variant& err) const = 0;
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
  ~FactsStore() override = default;

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
   * Return all types in the repo which transitively extend the given type.
   */
  virtual Array getTransitiveDerivedTypes(
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
   * Return all attributes decorating the given type.
   */
  virtual Array getTypeAttributes(const String& type) = 0;

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

  /**
   * Return all symbols defined in the repo, as a dict mapping each symbol
   * name to the path where the symbol lives in the repo.
   *
   * If a symbol is defined in more than one path, one of the paths defining the
   * symbol will be chosen in an unspecified manner.
   */
  virtual Array getAllTypes() = 0;
  virtual Array getAllFunctions() = 0;
  virtual Array getAllConstants() = 0;
  virtual Array getAllTypeAliases() = 0;
};

} // namespace HPHP
