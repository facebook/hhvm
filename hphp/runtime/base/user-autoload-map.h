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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * AutoloadMap mostly implemented in userspace Hack code.
 */
struct UserAutoloadMap final : AutoloadMap {

  String m_root;
  Array m_typeFile;
  Array m_functionFile;
  Array m_constantFile;
  Array m_typeAliasFile;
  Array m_moduleFile;
  Variant m_failFunc;

  UserAutoloadMap(String root,
                  Array typeFile,
                  Array functionFile,
                  Array constantFile,
                  Array typeAliasFile,
                  Array moduleFile,
                  Variant failFunc)
      : m_root{std::move(root)},
        m_typeFile{std::move(typeFile)},
        m_functionFile{std::move(functionFile)},
        m_constantFile{std::move(constantFile)},
        m_typeAliasFile{std::move(typeAliasFile)},
        m_moduleFile{std::move(moduleFile)},
        m_failFunc(std::move(failFunc)) {}

  UserAutoloadMap(const UserAutoloadMap&) = default;
  UserAutoloadMap(UserAutoloadMap&&) noexcept = default;
  UserAutoloadMap& operator=(const UserAutoloadMap&) = default;
  UserAutoloadMap& operator=(UserAutoloadMap&&) = default;
  ~UserAutoloadMap() override {
    m_root.detach();
    m_typeFile.detach();
    m_functionFile.detach();
    m_constantFile.detach();
    m_typeAliasFile.detach();
    m_moduleFile.detach();
    m_failFunc.setNull();
  }

  static UserAutoloadMap fromFullMap(const Array& fullMap,
                                     String root);

  void ensureUpdated() override {}

  /**
   * This map is not native because it calls into userspace hack.
   * It gets data via userspace calls to the builtin `autoload_set_paths()`.
   */
  bool isNative() const noexcept override {
    return false;
  }

  Array getAllFiles() const override;

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

  bool canHandleFailure() const override {
    return !m_failFunc.isNull();
  }
  AutoloadMap::Result handleFailure(KindOf kind,
                                    const String& className,
                                    const Variant& err) const override;

 private:
  Optional<AutoloadMap::FileResult> getFileFromMap(const Array& map, const String& key) const;
  Optional<std::filesystem::path> getPathFromMap(const Array&, const String&) const;
};

//////////////////////////////////////////////////////////////////////
}
