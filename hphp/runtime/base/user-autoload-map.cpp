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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/user-autoload-map.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/string/ext_string.h"
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
    case AutoloadMap::KindOf::TypeAlias:
      return s_type;
  }
  not_reached();
}

Array getSubMapFromMap(const Array& map, const StaticString& key) {
  auto const& subMap = map[key];
  if (!subMap.isArray() && !subMap.isDict()) {
    return Array::CreateDict();
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

  // We construct absolute paths with simple concatenation, so make
  // sure the root passed to us has a trailing slash.
  if (!root.empty() && root[root.size() - 1] != '/') {
    root += "/";
  }

  return {std::move(root),
      std::move(typeFile),
      std::move(functionFile),
      std::move(constantFile),
      std::move(typeAliasFile),
      std::move(failFunc)};
}

Array UserAutoloadMap::getAllFiles() const {
  auto maxSize = std::max({
    m_typeFile.size(), m_functionFile.size(), m_constantFile.size(),
    m_typeAliasFile.size()});
  if (UNLIKELY(maxSize < 0)) maxSize = 0;
  auto ret = Array::CreateKeyset();
  auto addToRet = [&](const Array& src) {
    IterateV(src.get(), [&](const TypedValue& v) {
      switch (type(v)) {
        case DataType::String:
        case DataType::PersistentString:
        {
          StringData* path = val(v).pstr;
          if (path->empty()) {
            return;
          }
          if (path->data()[0] == '/') {
            ret.append(String{path});
          } else {
            ret.append(m_root + String{path});
          }
          return;
        }
        default:
          break;
      }
    });
  };

  addToRet(m_typeFile);
  addToRet(m_functionFile);
  addToRet(m_constantFile);
  addToRet(m_typeAliasFile);

  return ret.toVec();
}

Optional<String> UserAutoloadMap::getTypeFile(
  const String& typeName) {
  return getFileFromMap(m_typeFile, HHVM_FN(strtolower)(typeName));
}

Optional<String> UserAutoloadMap::getFunctionFile(
  const String& funcName) {
  return getFileFromMap(m_functionFile, HHVM_FN(strtolower)(funcName));
}

Optional<String> UserAutoloadMap::getConstantFile(
  const String& constName) {
  return getFileFromMap(m_constantFile, constName);
}

Optional<String> UserAutoloadMap::getTypeAliasFile(
  const String& typeAliasName) {
  return getFileFromMap(m_typeAliasFile, HHVM_FN(strtolower)(typeAliasName));
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

Array UserAutoloadMap::getFileTypeAliases(const String& path) {
  SystemLib::throwInvalidOperationExceptionObject(
    "User Autoload Map does not support getFileTypeAliases"
  );
}

AutoloadMap::Result UserAutoloadMap::handleFailure(
  KindOf kind, const String& className, const Variant& err) const {
  if (m_failFunc.isNull()) {
    return AutoloadMap::Result::Failure;
  }

  // can throw, otherwise
  //  - true means the map was updated. try again
  //  - anything else means we failed
  Variant action = vm_call_user_func(
    m_failFunc, make_vec_array(getStringRepr(kind), className, err));
  auto const& actionCell = action.asTypedValue();
  if (actionCell->m_type == KindOfBoolean && actionCell->m_data.num) {
    return AutoloadMap::Result::RetryAutoloading;
  }
  return AutoloadMap::Result::StopAutoloading;
}

Optional<String> UserAutoloadMap::getFileFromMap(
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
