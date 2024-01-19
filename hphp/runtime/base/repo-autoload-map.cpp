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
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/util/assertions.h"

TRACE_SET_MOD(repo_autoload);

namespace fs = std::filesystem;

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

using UnitToPathMap = tbb::concurrent_hash_map<int64_t, const StringData*>;

namespace {
  // Holds interned absolute paths for symbols we have already looked up.
  UnitToPathMap unitToPathMap;
}

RepoAutoloadMap::RepoAutoloadMap(
    HashMapTypeIndex types,
    HashMapFuncIndex functions,
    CaseSensitiveHashMapIndex constants,
    HashMapTypeIndex typeAliases,
    CaseSensitiveHashMapIndex modules)
  : m_types{std::move(types)},
    m_functions{std::move(functions)},
    m_constants{std::move(constants)},
    m_typeAliases{std::move(typeAliases)},
    m_modules{std::move(modules)} {
  FTRACE(2, "Types: {} ({}, {}) ({}, {})\n",
          m_types.size, m_types.indexBounds.offset, m_types.indexBounds.size,
          m_types.dataBounds.offset, m_types.dataBounds.size);
  FTRACE(2, "Functions: {} ({}, {}) ({}, {})\n",
          m_functions.size, m_functions.indexBounds.offset,
          m_functions.indexBounds.size, m_functions.dataBounds.offset,
          m_functions.dataBounds.size);
  FTRACE(2, "Constants: {} ({}, {}) ({}, {})\n",
          m_constants.size, m_constants.indexBounds.offset,
          m_constants.indexBounds.size, m_constants.dataBounds.offset,
          m_constants.dataBounds.size);
  FTRACE(2, "TypeAliases: {} ({}, {}) ({}, {})\n",
          m_typeAliases.size, m_typeAliases.indexBounds.offset,
          m_typeAliases.indexBounds.size, m_typeAliases.dataBounds.offset,
          m_typeAliases.dataBounds.size);
  FTRACE(2, "Modules: {} ({}, {}) ({}, {})\n",
          m_modules.size, m_modules.indexBounds.offset,
          m_modules.indexBounds.size, m_modules.dataBounds.offset,
          m_modules.dataBounds.size);
}

template <typename KeyCompare>
static Optional<AutoloadMap::FileResult> getPathFromSymbol(
    const Blob::HashMapIndex<KeyCompare>& map,
    const String& name) {
  auto info = RepoFile::findUnitInfo(map, name.get());
  if (!info) {
    FTRACE(1, "Fail autoload {}\n", name.data());
    return {};
  }

  auto path = info->path;
  FTRACE(1, "Success autoload {} {}\n", name.data(), path->data());
  return {AutoloadMap::FileResult(StrNR(path), info)};
}

template <typename KeyCompare>
static Optional<fs::path> getPathFromSymbol(
    const Blob::HashMapIndex<KeyCompare>& map,
    std::string_view name) {
  auto fileRes = getPathFromSymbol(map, StrNR(makeStaticString(name)));
  if (!fileRes) return {};
  return fileRes->m_path.toCppString();
}

static Array getSymbolFromPath(const String& path, RepoSymbolType type) {
  auto symbols = RepoFile::findUnitSymbols(path.get());
  auto ret = Array::CreateVec();
  if (symbols) {
    for (auto& symbol : *symbols) {
      if (symbol.second == type) {
        ret.append(StrNR(symbol.first).asString());
      }
    }
  }
  return ret;
}

Optional<AutoloadMap::FileResult>
RepoAutoloadMap::getTypeOrTypeAliasFile(const String& typeName) {
  auto typeFile = getTypeFile(typeName);
  if (typeFile) {
    return typeFile;
  }
  return getTypeAliasFile(typeName);
}

Optional<AutoloadMap::FileResult>
RepoAutoloadMap::getTypeFile(const String& typeName) {
  return getPathFromSymbol(m_types, typeName);
}

Optional<AutoloadMap::FileResult>
RepoAutoloadMap::getFunctionFile(const String& funcName) {
  return getPathFromSymbol(m_functions, funcName);
}

Optional<AutoloadMap::FileResult>
RepoAutoloadMap::getConstantFile(const String& constName) {
  return getPathFromSymbol(m_constants, constName);
}

Optional<AutoloadMap::FileResult>
RepoAutoloadMap::getTypeAliasFile(const String& typeAliasName) {
  return getPathFromSymbol(m_typeAliases, typeAliasName);
}

Optional<AutoloadMap::FileResult>
RepoAutoloadMap::getModuleFile(const String& moduleName) {
  return getPathFromSymbol(m_modules, moduleName);
}

Optional<fs::path>
RepoAutoloadMap::getTypeOrTypeAliasFile(std::string_view typeName) {
  auto typeFile = getTypeFile(typeName);
  if (typeFile) {
    return typeFile;
  }
  return getTypeAliasFile(typeName);
}

Optional<fs::path>
RepoAutoloadMap::getTypeFile(std::string_view typeName) {
  return getPathFromSymbol(m_types, typeName);
}

Optional<fs::path>
RepoAutoloadMap::getFunctionFile(std::string_view funcName) {
  return getPathFromSymbol(m_functions, funcName);
}

Optional<fs::path>
RepoAutoloadMap::getConstantFile(std::string_view constName) {
  return getPathFromSymbol(m_constants, constName);
}

Optional<fs::path>
RepoAutoloadMap::getTypeAliasFile(std::string_view typeAliasName) {
  return getPathFromSymbol(m_typeAliases, typeAliasName);
}

Optional<fs::path>
RepoAutoloadMap::getModuleFile(std::string_view moduleName) {
  return getPathFromSymbol(m_modules, moduleName);
}

Array RepoAutoloadMap::getFileTypes(const String& path) {
  return getSymbolFromPath(path, RepoSymbolType::TYPE);
}

Array RepoAutoloadMap::getFileFunctions(const String& path) {
  return getSymbolFromPath(path, RepoSymbolType::FUNC);
}

Array RepoAutoloadMap::getFileConstants(const String& path) {
  return getSymbolFromPath(path, RepoSymbolType::CONSTANT);
}

Array RepoAutoloadMap::getFileTypeAliases(const String& path) {
  return getSymbolFromPath(path, RepoSymbolType::TYPE_ALIAS);
}

Array RepoAutoloadMap::getFileModules(const String& path) {
  return getSymbolFromPath(path, RepoSymbolType::MODULE);
}

////////////////////////////////////////////////////////////////////////////////

} // HPHP
