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

#include <folly/Optional.h>

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
    ContinueAutoloading,
    RetryAutoloading
  };

  enum class KindOf {
    Type,
    Function,
    Constant,
    TypeAlias,
  };

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
  folly::Optional<String> getFile(KindOf kind,
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
  virtual folly::Optional<String> getTypeFile(
      const String& typeName) = 0;
  virtual folly::Optional<String> getFunctionFile(
      const String& functionName) = 0;
  virtual folly::Optional<String> getConstantFile(
      const String& constantName) = 0;
  virtual folly::Optional<String> getTypeAliasFile(
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

} // namespace HPHP
