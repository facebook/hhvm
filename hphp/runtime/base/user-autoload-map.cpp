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

#include "hphp/runtime/base/user-autoload-map.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/util/assertions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_class("class"),
  s_function("function"),
  s_constant("constant"),
  s_type("type"),
  s_failure("failure");

const StaticString& getStringRepr(AutoloadMap::KindOf kind) {
  switch (kind) {
    case AutoloadMap::KindOf::Type:
      return s_class;
    case AutoloadMap::KindOf::Function:
      return s_function;
    case AutoloadMap::KindOf::Constant:
      return s_constant;
  }
  not_reached();
}

Array getSubMapFromMap(const Array& map, const StaticString& key) {
  auto const& subMap = map[key];
  if (!subMap.isArray() && !subMap.isDict()) {
    return {};
  }
  return Array{subMap.toDict()};
}

//////////////////////////////////////////////////////////////////////

UserAutoloadMap UserAutoloadMap::fromFullMap(const Array& fullMap,
                                             String root) {

  auto typeFile = getSubMapFromMap(fullMap, s_class);
  auto functionFile = getSubMapFromMap(fullMap, s_function);
  auto constantFile = getSubMapFromMap(fullMap, s_constant);
  auto typeAliasFile = getSubMapFromMap(fullMap, s_type);
  auto failFunc = fullMap[s_failure];

  // Merge types and type aliases into the same table.
  // Note that one, both, or neither array may actually exist here.
  typeFile = [&] {
    if (typeAliasFile.get() == nullptr) {
      return std::move(typeFile);
    }
    if (typeFile.get() == nullptr) {
      return std::move(typeAliasFile);
    }
    return typeFile.merge(typeAliasFile);
  }();

  return {std::move(root),
      std::move(typeFile),
      std::move(functionFile),
      std::move(constantFile),
      std::move(failFunc)};
}

folly::Optional<String> UserAutoloadMap::getTypeFile(
  const String& typeName) {
  return getFileFromMap(m_typeFile, typeName);
}

folly::Optional<String> UserAutoloadMap::getFunctionFile(
  const String& funcName) {
  return getFileFromMap(m_functionFile, funcName);
}

folly::Optional<String> UserAutoloadMap::getConstantFile(
  const String& constName) {
  return getFileFromMap(m_constantFile, constName);
}

Array UserAutoloadMap::getFileTypes(const String& path) {
  SystemLib::throwInvalidOperationExceptionObject(
    "User Autoload Map does not support getFileTypes"
  );
}

Array UserAutoloadMap::getFileFunctions(const String& path) {
  SystemLib::throwInvalidOperationExceptionObject(
    "User Autoload Map does not support getFileFunctions"
  );
}

Array UserAutoloadMap::getFileConstants(const String& path) {
  SystemLib::throwInvalidOperationExceptionObject(
    "User Autoload Map does not support getFileConstants"
  );
}

AutoloadMap::Result UserAutoloadMap::handleFailure(
  KindOf kind, const String& className, const Variant& err) const {
  if (m_failFunc.isNull()) {
    return AutoloadMap::Result::Failure;
  }

  // can throw, otherwise
  //  - true means the map was updated. try again
  //  - false means we should stop applying autoloaders (only affects classes)
  //  - anything else means keep going
  Variant action = vm_call_user_func(
    m_failFunc, make_vec_array(getStringRepr(kind), className, err));
  auto const& actionCell = action.asTypedValue();
  if (actionCell->m_type == KindOfBoolean) {
    return actionCell->m_data.num
      ? AutoloadMap::Result::RetryAutoloading
      : AutoloadMap::Result::StopAutoloading;
  }
  return AutoloadMap::Result::ContinueAutoloading;
}

folly::Optional<String> UserAutoloadMap::getFileFromMap(
    const Array& map, const String& key) const {
  auto const& filePath = map[key];
  if (!filePath.isString()) {
    return {};
  }
  auto path = filePath.toString();
  if (!path.empty() && !m_root.empty() && path.get()->data()[0] != '/') {
    path = m_root + std::move(path);
  }
  return {std::move(path)};
}

//////////////////////////////////////////////////////////////////////

} // HPHP
