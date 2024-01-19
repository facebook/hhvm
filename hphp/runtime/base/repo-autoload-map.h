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
#include <vector>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

#include "hphp/runtime/vm/repo-file.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * AutoloadMap using the info from the repo
 */

struct RepoAutoloadMap final : AutoloadMap {

  explicit RepoAutoloadMap(
    HashMapTypeIndex types,
    HashMapFuncIndex functions,
    CaseSensitiveHashMapIndex constants,
    HashMapTypeIndex typeAliases,
    CaseSensitiveHashMapIndex modules);

  Optional<AutoloadMap::FileResult> getTypeOrTypeAliasFile(const String& typeName) override;
  Optional<AutoloadMap::FileResult> getTypeFile(const String& typeName) override;
  Optional<AutoloadMap::FileResult> getFunctionFile(const String& functionName) override;
  Optional<AutoloadMap::FileResult> getConstantFile(const String& constantName) override;
  Optional<AutoloadMap::FileResult> getTypeAliasFile(const String& typeAliasName) override;
  Optional<AutoloadMap::FileResult> getModuleFile(const String& moduleName) override;

  Optional<std::filesystem::path> getTypeOrTypeAliasFile(std::string_view name) override;
  Optional<std::filesystem::path> getTypeFile(std::string_view name) override;
  Optional<std::filesystem::path> getFunctionFile(std::string_view name) override;
  Optional<std::filesystem::path> getConstantFile(std::string_view name) override;
  Optional<std::filesystem::path> getTypeAliasFile(std::string_view name) override;
  Optional<std::filesystem::path> getModuleFile(std::string_view name) override;

  Array getFileTypes(const String& path) override;
  Array getFileFunctions(const String& path) override;
  Array getFileConstants(const String& path) override;
  Array getFileTypeAliases(const String& path) override;
  Array getFileModules(const String& path) override;

  void ensureUpdated() override {}

  Holder getNativeHolder() noexcept override {
    return Holder{this, nullptr};
  }

private:
  HashMapTypeIndex m_types;
  HashMapFuncIndex m_functions;
  CaseSensitiveHashMapIndex m_constants;
  HashMapTypeIndex m_typeAliases;
  CaseSensitiveHashMapIndex m_modules;
};

} // HPHP
