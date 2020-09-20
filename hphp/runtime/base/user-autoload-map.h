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

#include <optional>
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
struct UserAutoloadMap : AutoloadMap {

  String m_root;
  Array m_typeFile;
  Array m_functionFile;
  Array m_constantFile;
  Array m_typeAliasFile;
  Variant m_failFunc;

  UserAutoloadMap(String root,
                  Array typeFile,
                  Array functionFile,
                  Array constantFile,
                  Array typeAliasFile,
                  Variant failFunc)
      : m_root{std::move(root)},
        m_typeFile{std::move(typeFile)},
        m_functionFile{std::move(functionFile)},
        m_constantFile{std::move(constantFile)},
        m_typeAliasFile{std::move(typeAliasFile)},
        m_failFunc(std::move(failFunc)) {}

  ~UserAutoloadMap() override {
    m_root.detach();
    m_typeFile.detach();
    m_functionFile.detach();
    m_constantFile.detach();
    m_typeAliasFile.detach();
    m_failFunc.setNull();
  }

  static UserAutoloadMap fromFullMap(const Array& fullMap,
                                     String root);

  /**
   * This map is not native because it gets data when userspace calls the
   * builtin function `autoload_set_paths()`.
   */
  virtual bool isNative() const noexcept override {
    return false;
  }

  virtual Array getAllFiles() const override;

  virtual std::optional<String> getTypeFile(
      const String& typeName) override;
  virtual std::optional<String> getFunctionFile(
      const String& functionName) override;
  virtual std::optional<String> getConstantFile(
      const String& constantName) override;
  virtual std::optional<String> getTypeAliasFile(
      const String& typeAliasName) override;

  virtual Array getFileTypes(const String& path) override;
  virtual Array getFileFunctions(const String& path) override;
  virtual Array getFileConstants(const String& path) override;
  virtual Array getFileTypeAliases(const String& path) override;

  virtual bool canHandleFailure() const override {
    return !m_failFunc.isNull();
  }
  virtual AutoloadMap::Result handleFailure(KindOf kind,
                                            const String& className,
                                            const Variant& err) const override;

 private:
  std::optional<String> getFileFromMap(const Array& map,
                                         const String& key) const;
};

//////////////////////////////////////////////////////////////////////
}
