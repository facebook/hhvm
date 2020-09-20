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

#include "hphp/runtime/base/repo-autoload-map.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/util/assertions.h"

TRACE_SET_MOD(repo_autoload);

namespace HPHP {

//////////////////////////////////////////////////////////////////////

using UnitToPathMap = tbb::concurrent_hash_map<int64_t, const StringData*>;

namespace {
  UnitToPathMap unitToPathMap;
}

RepoAutoloadMap::RepoAutoloadMap(
      CaseInsensitiveMap types,
      CaseInsensitiveMap functions,
      CaseSensitiveMap constants,
      CaseInsensitiveMap typeAliases)
  : m_types{std::move(types)},
    m_functions{std::move(functions)},
    m_constants{std::move(constants)},
    m_typeAliases{std::move(typeAliases)} {
  if (Trace::moduleEnabled(Trace::repo_autoload, 2)) {
    FTRACE(2, "Type:\n");
    for (auto const DEBUG_ONLY& s : types) {
      FTRACE(2, "{} => {}\n", s.first->data(), s.second);
    }
    FTRACE(2, "Functions:\n");
    for (auto const DEBUG_ONLY& s : functions) {
      FTRACE(2, "{} => {}\n", s.first->data(), s.second);
    }
    FTRACE(2, "Constants:\n");
    for (auto const DEBUG_ONLY& s : constants) {
      FTRACE(2, "{} => {}\n", s.first->data(), s.second);
    }
    FTRACE(2, "TypeAliases:\n");
    for (auto const DEBUG_ONLY& s : typeAliases) {
      FTRACE(2, "{} => {}\n", s.first->data(), s.second);
    }
  }
}

template <typename Compare>
static std::optional<String> getPathFromSymbol(
    const RepoAutoloadMap::Map<Compare>& map,
    const String& name) {
  auto search = map.find(name.get());
  if (search == map.end()) {
    FTRACE(1, "Fail autoload {}\n", name.data());
    return {};
  }
  auto unitSn = search->second;

  UnitToPathMap::const_accessor acc;
  if (unitToPathMap.find(acc, unitSn)) {
    FTRACE(1, "Success autoload (cache) {} {}\n", name.data(), acc->second->data());
    return {StrNR(acc->second).asString()};
  }

  std::optional<String> path;
  try {
    path = Repo::get().findPath(unitSn, RuntimeOption::SourceRoot);
  } catch (RepoExc& re) {
    FTRACE(1, "Failed loading because of repo error {} {}\n", name.data(), re.msg());
    return {};
  }

  always_assert(path.has_value());
  auto spath = makeStaticString(path->get());
  unitToPathMap.insert(std::make_pair(unitSn, spath));

  FTRACE(1, "Success autoload {} {}\n", name.data(), spath->data());
  return {StrNR(spath).asString()};
}

template <typename Compare>
Array getSymbolFromPath(
    const RepoAutoloadMap::Map<Compare>& map,
    const String& path) {
  auto ret = Array::CreateVec();
  int64_t unitSn;
  auto res = Repo::get().findUnit(path.c_str(), RuntimeOption::SourceRoot, unitSn);
  if (res == RepoStatus::success) {
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (it->second == unitSn) {
        ret.append(StrNR(it->first).asString());
      }
    }
  }
  return ret;
}

std::optional<String> RepoAutoloadMap::getTypeFile(
  const String& typeName) {
  return getPathFromSymbol(m_types, typeName);
}

std::optional<String> RepoAutoloadMap::getFunctionFile(
  const String& funcName) {
  return getPathFromSymbol(m_functions, funcName);
}

std::optional<String> RepoAutoloadMap::getConstantFile(
  const String& constName) {
  return getPathFromSymbol(m_constants, constName);
}

std::optional<String> RepoAutoloadMap::getTypeAliasFile(
  const String& typeAliasName) {
  return getPathFromSymbol(m_typeAliases, typeAliasName);
}

Array RepoAutoloadMap::getFileTypes(const String& path) {
  return getSymbolFromPath(m_types, path);
}

Array RepoAutoloadMap::getFileFunctions(const String& path) {
  return getSymbolFromPath(m_functions, path);
}

Array RepoAutoloadMap::getFileConstants(const String& path) {
  return getSymbolFromPath(m_constants, path);
}

Array RepoAutoloadMap::getFileTypeAliases(const String& path) {
  return getSymbolFromPath(m_typeAliases, path);
}

AutoloadMap::Result RepoAutoloadMap::handleFailure(
  KindOf kind, const String& className, const Variant& err) const {
  return AutoloadMap::Result::Failure;
}

Array RepoAutoloadMap::getAllFiles() const {
  SystemLib::throwInvalidOperationExceptionObject(
    "Repo Autoload Map does not support getAllFiles"
  );
}

//////////////////////////////////////////////////////////////////////

} // HPHP
