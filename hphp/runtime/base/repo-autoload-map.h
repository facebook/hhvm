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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * AutoloadMap using the info from the repo
 */

struct RepoAutoloadMap final : AutoloadMap {

  template <typename Compare>
  using Map = hphp_fast_map<
    const StringData*,
    int64_t,
    string_data_hash,
    Compare
  >;

  using CaseInsensitiveMap = Map<string_data_isame>;
  using CaseSensitiveMap = Map<string_data_same>;

  explicit RepoAutoloadMap(
      CaseInsensitiveMap types,
      CaseInsensitiveMap functions,
      CaseSensitiveMap constants,
      CaseInsensitiveMap typeAliases);

  Optional<String> getTypeFile(const String& typeName) override;
  Optional<String> getFunctionFile(const String& functionName) override;
  Optional<String> getConstantFile(const String& constantName) override;
  Optional<String> getTypeAliasFile(const String& typeAliasName) override;

  Array getFileTypes(const String& path) override;
  Array getFileFunctions(const String& path) override;
  Array getFileConstants(const String& path) override;
  Array getFileTypeAliases(const String& path) override;

  bool canHandleFailure() const override {
    return false;
  }

  void ensureUpdated() override {}

  bool isNative() const noexcept override {
    return true;
  }

  AutoloadMap::Result handleFailure(KindOf kind, const String& className,
      const Variant& err) const override;

  Array getAllFiles() const override;

private:
  CaseInsensitiveMap m_types;
  CaseInsensitiveMap m_functions;
  CaseSensitiveMap m_constants;
  CaseInsensitiveMap m_typeAliases;
};

} // HPHP
