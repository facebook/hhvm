/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <folly/ScopeGuard.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/hash/Hash.h>
#include <folly/logging/xlog.h>

#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/symbol-map.h"
#include "hphp/util/assertions.h"

namespace fs = std::filesystem;

namespace HPHP {

struct StringData;

namespace Facts {

namespace {

/**
 * Get the PathToSymbolsMap corresponding to the given SymKind enum value
 */

// const
template <SymKind k>
typename std::
    enable_if<k == SymKind::Type, const PathToSymbolsMap<SymKind::Type>&>::type
    getPathSymMap(const typename SymbolMap::Data& data) {
  return data.m_typePath;
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Function,
    const PathToSymbolsMap<SymKind::Function>&>::type
getPathSymMap(const typename SymbolMap::Data& data) {
  return data.m_functionPath;
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Constant,
    const PathToSymbolsMap<SymKind::Constant>&>::type
getPathSymMap(const typename SymbolMap::Data& data) {
  return data.m_constantPath;
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Module,
    const PathToSymbolsMap<SymKind::Module>&>::type
getPathSymMap(const typename SymbolMap::Data& data) {
  return data.m_modulePath;
}

// non-const
template <SymKind k>
typename std::enable_if<k == SymKind::Type, PathToSymbolsMap<SymKind::Type>&>::
    type
    getPathSymMap(typename SymbolMap::Data& data) {
  return data.m_typePath;
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Function,
    PathToSymbolsMap<SymKind::Function>&>::type
getPathSymMap(typename SymbolMap::Data& data) {
  return data.m_functionPath;
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Constant,
    PathToSymbolsMap<SymKind::Constant>&>::type
getPathSymMap(typename SymbolMap::Data& data) {
  return data.m_constantPath;
}
template <SymKind k>
typename std::
    enable_if<k == SymKind::Module, PathToSymbolsMap<SymKind::Module>&>::type
    getPathSymMap(typename SymbolMap::Data& data) {
  return data.m_modulePath;
}

/**
 * Get all symbols of a given kind from the DB.
 */
template <SymKind k>
typename std::enable_if<
    k == SymKind::Type,
    AutoloadDB::MultiResult<AutoloadDB::SymbolPath>>::type
getAllSymbolsFromDB(AutoloadDB& db) {
  return db.getAllTypePaths();
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Function,
    AutoloadDB::MultiResult<AutoloadDB::SymbolPath>>::type
getAllSymbolsFromDB(AutoloadDB& db) {
  return db.getAllFunctionPaths();
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Constant,
    AutoloadDB::MultiResult<AutoloadDB::SymbolPath>>::type
getAllSymbolsFromDB(AutoloadDB& db) {
  return db.getAllConstantPaths();
}
template <SymKind k>
typename std::enable_if<
    k == SymKind::Module,
    AutoloadDB::MultiResult<AutoloadDB::SymbolPath>>::type
getAllSymbolsFromDB(AutoloadDB& db) {
  return db.getAllModulePaths();
}

} // namespace

AutoloadDBVault::AutoloadDBVault(AutoloadDB::Handle dbHandle)
    : m_dbHandle{std::move(dbHandle)} {}

std::shared_ptr<AutoloadDB> AutoloadDBVault::get() const {
  return m_dbs.withULockPtr([this](auto ulock) {
    auto it = ulock->find(std::this_thread::get_id());
    if (it != ulock->end()) {
      return it->second;
    }
    auto wlock = ulock.moveFromUpgradeToWrite();
    return wlock->insert({std::this_thread::get_id(), m_dbHandle()})
        .first->second;
  });
}

SymbolMap::SymbolMap(
    fs::path root,
    AutoloadDB::Handle dbHandle,
    hphp_hash_set<std::string> indexedMethodAttrs)
    : m_exec{std::make_shared<folly::CPUThreadPoolExecutor>(
          1,
          std::make_shared<folly::NamedThreadFactory>("Autoload DB update"))},
      m_root{std::move(root)},
      m_dbVault{std::move(dbHandle)},
      m_indexedMethodAttrs{std::move(indexedMethodAttrs)} {
  assertx(m_root.is_absolute());
}

SymbolMap::~SymbolMap() {
  try {
    waitForDBUpdate();
  } catch (...) {
    // Swallow the exception so we don't crash the program
  }
}

Optional<Symbol<SymKind::Type>> SymbolMap::getTypeName(
    const StringData& typeName) {
  Symbol<SymKind::Type> type{typeName};
  auto path = getSymbolPath(type);
  if (path == nullptr) {
    return {};
  }
  auto pathTypes = getPathSymbols<SymKind::Type>(path);
  auto const it = std::find(pathTypes.begin(), pathTypes.end(), type);
  if (it == pathTypes.end()) {
    return {};
  }
  return *it;
}

Path SymbolMap::getTypeOrTypeAliasFile(Symbol<SymKind::Type> type) {
  return getSymbolPath(type);
}

Path SymbolMap::getTypeOrTypeAliasFile(const StringData& type) {
  return getTypeOrTypeAliasFile(Symbol<SymKind::Type>{type});
}

Path SymbolMap::getTypeFile(Symbol<SymKind::Type> type) {
  auto path = getSymbolPath(type);
  auto [kind, _] = getKindAndFlags(type, path);
  if (kind == TypeKind::TypeAlias) {
    return Path{nullptr};
  }
  return path;
}

Path SymbolMap::getTypeFile(const StringData& type) {
  return getTypeFile(Symbol<SymKind::Type>{type});
}

Path SymbolMap::getFunctionFile(Symbol<SymKind::Function> function) {
  return getSymbolPath(function);
}

Path SymbolMap::getFunctionFile(const StringData& function) {
  return getFunctionFile(Symbol<SymKind::Function>{function});
}

Path SymbolMap::getConstantFile(Symbol<SymKind::Constant> constant) {
  return getSymbolPath(constant);
}

Path SymbolMap::getConstantFile(const StringData& constant) {
  return getConstantFile(Symbol<SymKind::Constant>{constant});
}

Path SymbolMap::getTypeAliasFile(Symbol<SymKind::Type> typeAlias) {
  auto path = getSymbolPath(typeAlias);
  auto [kind, _] = getKindAndFlags(typeAlias, path);
  if (kind != TypeKind::TypeAlias) {
    return Path{nullptr};
  }
  return path;
}

Path SymbolMap::getTypeAliasFile(const StringData& typeAlias) {
  return getTypeAliasFile(Symbol<SymKind::Type>{typeAlias});
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getFileTypes(Path path) {
  auto const& symbols = getPathSymbols<SymKind::Type>(path);
  std::vector<Symbol<SymKind::Type>> symbolVec;
  symbolVec.reserve(symbols.size());
  std::copy_if(
      symbols.begin(),
      symbols.end(),
      std::back_inserter(symbolVec),
      [&](auto const& sym) {
        auto [kind, _] = getKindAndFlags(sym, path);
        return kind != TypeKind::TypeAlias;
      });
  return symbolVec;
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getFileTypes(
    const fs::path& path) {
  return getFileTypes(Path{path});
}

std::vector<Symbol<SymKind::Function>> SymbolMap::getFileFunctions(Path path) {
  auto const& symbols = getPathSymbols<SymKind::Function>(path);
  std::vector<Symbol<SymKind::Function>> symbolVec;
  symbolVec.reserve(symbols.size());
  std::copy(symbols.begin(), symbols.end(), std::back_inserter(symbolVec));
  return symbolVec;
}

std::vector<Symbol<SymKind::Function>> SymbolMap::getFileFunctions(
    const fs::path& path) {
  return getFileFunctions(Path{path});
}

std::vector<Symbol<SymKind::Constant>> SymbolMap::getFileConstants(Path path) {
  auto const& symbols = getPathSymbols<SymKind::Constant>(path);
  std::vector<Symbol<SymKind::Constant>> symbolVec;
  symbolVec.reserve(symbols.size());
  std::copy(symbols.begin(), symbols.end(), std::back_inserter(symbolVec));
  return symbolVec;
}

std::vector<Symbol<SymKind::Constant>> SymbolMap::getFileConstants(
    const fs::path& path) {
  return getFileConstants(Path{path});
}

std::vector<Symbol<SymKind::Module>> SymbolMap::getFileModules(Path path) {
  auto const& symbols = getPathSymbols<SymKind::Module>(path);
  std::vector<Symbol<SymKind::Module>> symbolVec;
  symbolVec.reserve(symbols.size());
  std::copy(symbols.begin(), symbols.end(), std::back_inserter(symbolVec));
  return symbolVec;
}

std::vector<Symbol<SymKind::Module>> SymbolMap::getFileModules(
    const fs::path& path) {
  return getFileModules(Path{path});
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getFileTypeAliases(Path path) {
  auto const& symbols = getPathSymbols<SymKind::Type>(path);
  std::vector<Symbol<SymKind::Type>> symbolVec;
  symbolVec.reserve(symbols.size());
  std::copy_if(
      symbols.begin(),
      symbols.end(),
      std::back_inserter(symbolVec),
      [&](auto const& sym) {
        auto [kind, _] = getKindAndFlags(sym, path);
        return kind == TypeKind::TypeAlias;
      });
  return symbolVec;
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getFileTypeAliases(
    const fs::path& path) {
  return getFileTypeAliases(Path{path});
}

std::vector<std::pair<Symbol<SymKind::Type>, Path>> SymbolMap::getAllTypes() {
  auto results = getAllSymbols<SymKind::Type>();
  // Filter type aliases out
  results.erase(
      std::remove_if(
          results.begin(),
          results.end(),
          [this](auto const& symbolPath) -> bool {
            auto const& [symbol, path] = symbolPath;
            auto [kind, _] = getKindAndFlags(symbol, path);
            return kind == TypeKind::TypeAlias;
          }),
      results.end());
  return results;
}

std::vector<std::pair<Symbol<SymKind::Function>, Path>>
SymbolMap::getAllFunctions() {
  return getAllSymbols<SymKind::Function>();
}

std::vector<std::pair<Symbol<SymKind::Constant>, Path>>
SymbolMap::getAllConstants() {
  return getAllSymbols<SymKind::Constant>();
}

std::vector<std::pair<Symbol<SymKind::Module>, Path>>
SymbolMap::getAllModules() {
  return getAllSymbols<SymKind::Module>();
}

std::vector<std::pair<Symbol<SymKind::Type>, Path>>
SymbolMap::getAllTypeAliases() {
  auto results = getAllSymbols<SymKind::Type>();
  // Filter down to type aliases
  results.erase(
      std::remove_if(
          results.begin(),
          results.end(),
          [this](auto const& symbolPath) -> bool {
            auto const& [symbol, path] = symbolPath;
            auto [kind, _] = getKindAndFlags(symbol, path);
            return kind != TypeKind::TypeAlias;
          }),
      results.end());
  return results;
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getBaseTypes(
    Symbol<SymKind::Type> derivedType,
    DeriveKind kind) {
  auto derivedTypePath = getSymbolPath(derivedType);
  if (derivedTypePath == nullptr) {
    return {};
  }

  using Types = typename InheritanceInfo::Types;
  using TypeVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [](const Types& baseTypes) -> TypeVec {
    TypeVec baseTypeVec;
    baseTypeVec.reserve(baseTypes.size());
    for (auto const& [type, _] : baseTypes) {
      baseTypeVec.push_back(type);
    }
    return baseTypeVec;
  };

  return readOrUpdate<TypeVec>(
      [&](const Data& data) -> Optional<TypeVec> {
        auto baseTypes = data.m_inheritanceInfo.getBaseTypes(
            derivedType, derivedTypePath, kind);
        if (!baseTypes) {
          return std::nullopt;
        }
        return makeVec(*baseTypes);
      },
      [&](std::shared_ptr<AutoloadDB> db) -> std::vector<SubtypeQuery> {
        auto const symbolStrs = db->getBaseTypes(
            fs::path{std::string{derivedTypePath.slice()}},
            derivedType.slice(),
            kind);
        std::vector<SubtypeQuery> symbols;
        symbols.reserve(symbolStrs.size());
        for (auto const& symbolStr : symbolStrs) {
          symbols.push_back(
              {.m_type = Symbol<SymKind::Type>{symbolStr}, .m_kind = kind});
        }
        return symbols;
      },
      [&](Data& data, std::vector<SubtypeQuery> edgesFromDB) -> TypeVec {
        return makeVec(data.m_inheritanceInfo.getBaseTypes(
            derivedType, derivedTypePath, kind, std::move(edgesFromDB)));
      });
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getBaseTypes(
    const StringData& derivedType,
    DeriveKind kind) {
  return getBaseTypes(Symbol<SymKind::Type>{derivedType}, kind);
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getDerivedTypes(
    Symbol<SymKind::Type> baseType,
    DeriveKind kind) {
  // Return empty results if the given type is undefined
  if (getSymbolPath(baseType) == nullptr) {
    return {};
  }

  using TypeDefs = typename InheritanceInfo::TypeDefs;
  using TypeVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [&](const TypeDefs& subtypeDefs) -> TypeVec {
    TypeVec subtypes;
    subtypes.reserve(subtypeDefs.size());
    for (auto const& subTypeDef : subtypeDefs) {
      assertx(subTypeDef.m_kind == kind);
      subtypes.push_back(subTypeDef.m_type);
    }
    return subtypes;
  };
  auto subtypes = readOrUpdate<TypeVec>(
      [&](const Data& data) -> Optional<TypeVec> {
        auto derivedTypes =
            data.m_inheritanceInfo.getDerivedTypes(baseType, kind);
        if (!derivedTypes) {
          return std::nullopt;
        }
        return makeVec(*derivedTypes);
      },
      [&](std::shared_ptr<AutoloadDB> db) -> std::vector<EdgeToSupertype> {
        auto const typeDefStrs = db->getDerivedTypes(baseType.slice(), kind);
        std::vector<EdgeToSupertype> typeDefs;
        typeDefs.reserve(typeDefStrs.size());
        for (auto const& typeDefStr : typeDefStrs) {
          typeDefs.push_back(EdgeToSupertype{
              .m_type = Symbol<SymKind::Type>{typeDefStr.m_symbol},
              .m_kind = kind,
              .m_path = Path{typeDefStr.m_path}});
        }
        return typeDefs;
      },
      [&](Data& data, std::vector<EdgeToSupertype> edgesFromDB) -> TypeVec {
        return makeVec(data.m_inheritanceInfo.getDerivedTypes(
            baseType, kind, std::move(edgesFromDB)));
      });
  return subtypes;
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getDerivedTypes(
    const StringData& baseType,
    DeriveKind kind) {
  return getDerivedTypes(Symbol<SymKind::Type>{baseType}, kind);
}

std::vector<typename SymbolMap::DerivedTypeInfo>
SymbolMap::getTransitiveDerivedTypes(
    Symbol<SymKind::Type> baseType,
    TypeKindMask kinds,
    DeriveKindMask deriveKinds) {
  std::vector<typename SymbolMap::DerivedTypeInfo> results;

  hphp_hash_set<Symbol<SymKind::Type>> seen;
  std::vector<Symbol<SymKind::Type>> stack;
  stack.push_back(baseType);
  while (!stack.empty()) {
    auto type = stack.back();
    stack.pop_back();

    for (auto deriveKind :
         {DeriveKind::Extends,
          DeriveKind::RequireExtends,
          DeriveKind::RequireImplements,
          DeriveKind::RequireClass}) {
      if (deriveKinds & static_cast<int>(deriveKind)) {
        for (auto subtype : getDerivedTypes(type, deriveKind)) {
          if (seen.count(subtype) > 0) {
            continue;
          }
          seen.insert(subtype);

          auto subtypePath = getSymbolPath(subtype);
          auto [subtypeKind, subtypeFlags] =
              getKindAndFlags(subtype, subtypePath);
          if (kinds & static_cast<int>(subtypeKind)) {
            stack.push_back(subtype);
            results.push_back(
                {subtype, subtypePath, subtypeKind, subtypeFlags});
          }
        }
      }
    }
  }

  return results;
}

std::vector<typename SymbolMap::DerivedTypeInfo>
SymbolMap::getTransitiveDerivedTypes(
    const StringData& baseType,
    TypeKindMask kinds,
    DeriveKindMask deriveKinds) {
  return getTransitiveDerivedTypes(
      Symbol<SymKind::Type>{baseType}, kinds, deriveKinds);
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getAttributesOfType(
    Symbol<SymKind::Type> type) {
  auto path = getSymbolPath(type);
  if (path == nullptr) {
    return {};
  }
  using AttrVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [&](auto const& attrs) -> AttrVec {
    AttrVec attrVec;
    attrVec.reserve(attrs.size());
    for (auto const& attr : attrs) {
      attrVec.emplace_back(attr);
    }
    return attrVec;
  };
  return readOrUpdate<AttrVec>(
      [&](const Data& data) -> Optional<AttrVec> {
        auto attrs = data.m_typeAttrs.getAttributes({type, path});
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db)
          -> std::vector<Symbol<SymKind::Type>> {
        // If `type` is actually a type alias in `path`, and we don't do this
        // check, we'll pollute our in-memory map
        auto [kind, _] = db->getKindAndFlags(type.slice(), path.native());
        if (kind == TypeKind::TypeAlias) {
          return {};
        }
        auto const attrStrs =
            db->getAttributesOfType(type.slice(), path.native());
        std::vector<Symbol<SymKind::Type>> attrs;
        attrs.reserve(attrStrs.size());
        for (auto const& attrStr : attrStrs) {
          attrs.emplace_back(attrStr);
        }
        return attrs;
      },
      [&](Data& data,
          std::vector<Symbol<SymKind::Type>> attrsFromDB) -> AttrVec {
        return makeVec(data.m_typeAttrs.getAttributes(
            {type, path}, std::move(attrsFromDB)));
      });
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getAttributesOfType(
    const StringData& type) {
  return getAttributesOfType(Symbol<SymKind::Type>{type});
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getAttributesOfTypeAlias(
    Symbol<SymKind::Type> typeAlias) {
  auto path = getSymbolPath(typeAlias);
  if (path == nullptr) {
    return {};
  }
  using AttrVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [&](auto const& attrs) -> AttrVec {
    AttrVec attrVec;
    attrVec.reserve(attrs.size());
    for (auto const& attr : attrs) {
      attrVec.emplace_back(attr);
    }
    return attrVec;
  };
  return readOrUpdate<AttrVec>(
      [&](const Data& data) -> Optional<AttrVec> {
        auto attrs = data.m_typeAliasAttrs.getAttributes({typeAlias, path});
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db)
          -> std::vector<Symbol<SymKind::Type>> {
        // If `typeAlias` is actually a type in `path`, and we don't do this
        // check, we'll pollute our in-memory map
        auto [kind, _] = db->getKindAndFlags(typeAlias.slice(), path.native());
        if (kind != TypeKind::TypeAlias) {
          return {};
        }
        auto const attrStrs =
            db->getAttributesOfType(typeAlias.slice(), path.native());
        std::vector<Symbol<SymKind::Type>> attrs;
        attrs.reserve(attrStrs.size());
        for (auto const& attrStr : attrStrs) {
          attrs.emplace_back(attrStr);
        }
        return attrs;
      },
      [&](Data& data,
          std::vector<Symbol<SymKind::Type>> attrsFromDB) -> AttrVec {
        return makeVec(data.m_typeAliasAttrs.getAttributes(
            {typeAlias, path}, std::move(attrsFromDB)));
      });
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getAttributesOfTypeAlias(
    const StringData& typeAlias) {
  return getAttributesOfTypeAlias(Symbol<SymKind::Type>{typeAlias});
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getTypesWithAttribute(
    Symbol<SymKind::Type> attr) {
  using TypeVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [&](auto const& typeDefs) -> TypeVec {
    TypeVec typeVec;
    typeVec.reserve(typeDefs.size());
    for (auto const& [type, typePath] : typeDefs) {
      typeVec.emplace_back(type);
    }
    return typeVec;
  };
  auto types = readOrUpdate<TypeVec>(
      [&](const Data& data) -> Optional<TypeVec> {
        auto attrs = data.m_typeAttrs.getKeysWithAttribute(attr);
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db) -> std::vector<TypeDecl> {
        auto const typeDefStrs = db->getTypesWithAttribute(attr.slice());
        std::vector<TypeDecl> typeDefs;
        typeDefs.reserve(typeDefStrs.size());
        for (auto const& [type, path] : typeDefStrs) {
          typeDefs.push_back({Symbol<SymKind::Type>{type}, Path{path}});
        }
        return typeDefs;
      },
      [&](Data& data, std::vector<TypeDecl> typesFromDB) -> TypeVec {
        return makeVec(data.m_typeAttrs.getKeysWithAttribute(
            attr, std::move(typesFromDB)));
      });
  return types;
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getTypesWithAttribute(
    const StringData& attr) {
  return getTypesWithAttribute(Symbol<SymKind::Type>{attr});
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getTypeAliasesWithAttribute(
    Symbol<SymKind::Type> attr) {
  using TypeAliasVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [&](auto const& typeAliasDefs) -> TypeAliasVec {
    TypeAliasVec typeAliasVec;
    typeAliasVec.reserve(typeAliasDefs.size());
    for (auto const& [typeAlias, typeAliasPath] : typeAliasDefs) {
      typeAliasVec.emplace_back(typeAlias);
    }
    return typeAliasVec;
  };
  auto typeAliases = readOrUpdate<TypeAliasVec>(
      [&](const Data& data) -> Optional<TypeAliasVec> {
        auto attrs = data.m_typeAliasAttrs.getKeysWithAttribute(attr);
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db) -> std::vector<TypeDecl> {
        auto const typeAliasDefStrs =
            db->getTypeAliasesWithAttribute(attr.slice());
        std::vector<TypeDecl> typeAliasDefs;
        typeAliasDefs.reserve(typeAliasDefStrs.size());
        for (auto const& [type, path] : typeAliasDefStrs) {
          typeAliasDefs.push_back({Symbol<SymKind::Type>{type}, Path{path}});
        }
        return typeAliasDefs;
      },
      [&](Data& data, std::vector<TypeDecl> typeAliasesFromDB) -> TypeAliasVec {
        return makeVec(data.m_typeAliasAttrs.getKeysWithAttribute(
            attr, std::move(typeAliasesFromDB)));
      });
  return typeAliases;
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getTypeAliasesWithAttribute(
    const StringData& attr) {
  return getTypeAliasesWithAttribute(Symbol<SymKind::Type>{attr});
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getAttributesOfMethod(
    Symbol<SymKind::Type> type,
    Symbol<SymKind::Function> method) {
  auto path = getSymbolPath(type);
  if (path == nullptr) {
    return {};
  }
  using AttrVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [&](auto const& attrs) -> AttrVec {
    AttrVec attrVec;
    attrVec.reserve(attrs.size());
    for (auto const& attr : attrs) {
      attrVec.emplace_back(attr);
    }
    return attrVec;
  };
  return readOrUpdate<AttrVec>(
      [&](const Data& data) -> Optional<AttrVec> {
        auto attrs = data.m_methodAttrs.getAttributes({{type, path}, method});
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db)
          -> std::vector<Symbol<SymKind::Type>> {
        auto const attrStrs = db->getAttributesOfMethod(
            type.slice(), method.slice(), fs::path{std::string{path.slice()}});
        std::vector<Symbol<SymKind::Type>> attrs;
        attrs.reserve(attrStrs.size());
        for (auto const& attrStr : attrStrs) {
          attrs.emplace_back(attrStr);
        }
        return attrs;
      },
      [&](Data& data,
          std::vector<Symbol<SymKind::Type>> attrsFromDB) -> AttrVec {
        return makeVec(data.m_methodAttrs.getAttributes(
            {{type, path}, method}, std::move(attrsFromDB)));
      });
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getAttributesOfMethod(
    const StringData& type,
    const StringData& method) {
  return getAttributesOfMethod(
      Symbol<SymKind::Type>{type}, Symbol<SymKind::Function>{method});
}

std::vector<MethodDecl> SymbolMap::getMethodsWithAttribute(
    Symbol<SymKind::Type> attr) {
  using MethodVec = std::vector<MethodDecl>;
  auto makeVec = [](auto&& methods) -> MethodVec {
    MethodVec methodVec;
    methodVec.reserve(methods.size());
    for (auto&& method : std::move(methods)) {
      methodVec.push_back(std::move(method));
    }
    return methodVec;
  };
  auto methods = readOrUpdate<MethodVec>(
      [&](const Data& data) -> Optional<MethodVec> {
        auto attrs = data.m_methodAttrs.getKeysWithAttribute(attr);
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db) -> MethodVec {
        auto const dbMethodDecls = db->getMethodsWithAttribute(attr.slice());
        MethodVec methodDecls;
        methodDecls.reserve(dbMethodDecls.size());
        for (auto const& [type, method, path] : dbMethodDecls) {
          methodDecls.push_back(MethodDecl{
              .m_type =
                  TypeDecl{
                      .m_name = Symbol<SymKind::Type>{type},
                      .m_path = Path{path}},
              .m_method = Symbol<SymKind::Function>{method}});
        }
        return methodDecls;
      },
      [&](Data& data, MethodVec methodsFromDB) -> MethodVec {
        return makeVec(data.m_methodAttrs.getKeysWithAttribute(
            attr, std::move(methodsFromDB)));
      });
  return methods;
}

std::vector<MethodDecl> SymbolMap::getMethodsWithAttribute(
    const StringData& attr) {
  return getMethodsWithAttribute(Symbol<SymKind::Type>{attr});
}

std::vector<Symbol<SymKind::Type>> SymbolMap::getAttributesOfFile(Path path) {
  if (path == nullptr) {
    return {};
  }
  using AttrVec = std::vector<Symbol<SymKind::Type>>;
  auto makeVec = [&](auto const& attrs) -> AttrVec {
    AttrVec attrVec;
    attrVec.reserve(attrs.size());
    for (auto const& attr : attrs) {
      attrVec.emplace_back(attr);
    }
    return attrVec;
  };
  return readOrUpdate<AttrVec>(
      [&](const Data& data) -> Optional<AttrVec> {
        auto attrs = data.m_fileAttrs.getAttributes({path});
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db)
          -> std::vector<Symbol<SymKind::Type>> {
        auto const attrStrs = db->getAttributesOfFile(path.native());
        std::vector<Symbol<SymKind::Type>> attrs;
        attrs.reserve(attrStrs.size());
        for (auto const& attrStr : attrStrs) {
          attrs.emplace_back(attrStr);
        }
        return attrs;
      },
      [&](Data& data,
          std::vector<Symbol<SymKind::Type>> attrsFromDB) -> AttrVec {
        return makeVec(
            data.m_fileAttrs.getAttributes({path}, std::move(attrsFromDB)));
      });
}

std::vector<Path> SymbolMap::getFilesWithAttribute(Symbol<SymKind::Type> attr) {
  using PathVec = std::vector<Path>;
  auto makeVec = [](auto&& paths) -> PathVec {
    PathVec pathVec;
    pathVec.reserve(paths.size());
    for (auto&& path : std::move(paths)) {
      pathVec.push_back(std::move(path));
    }
    return pathVec;
  };
  auto paths = readOrUpdate<PathVec>(
      [&](const Data& data) -> Optional<PathVec> {
        auto attrs = data.m_fileAttrs.getKeysWithAttribute(attr);
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](std::shared_ptr<AutoloadDB> db) -> PathVec {
        auto dbPathDecls = db->getFilesWithAttribute(attr.slice());
        PathVec pathDecls;
        pathDecls.reserve(dbPathDecls.size());
        for (auto const& path : dbPathDecls) {
          pathDecls.push_back(Path{path});
        }
        return pathDecls;
      },
      [&](Data& data, PathVec pathsFromDB) -> PathVec {
        return makeVec(data.m_fileAttrs.getKeysWithAttribute(
            attr, std::move(pathsFromDB)));
      });
  return paths;
}

std::vector<Path> SymbolMap::getFilesWithAttribute(const StringData& attr) {
  return getFilesWithAttribute(Symbol<SymKind::Type>{attr});
}

std::vector<folly::dynamic> SymbolMap::getTypeAttributeArgs(
    Symbol<SymKind::Type> type,
    Symbol<SymKind::Type> attr) {
  auto path = getSymbolPath(type);
  if (path == nullptr) {
    return {};
  }
  using ArgVec = std::vector<folly::dynamic>;
  return readOrUpdate<ArgVec>(
      [&](const Data& data) -> Optional<ArgVec> {
        auto const* args =
            data.m_typeAttrs.getAttributeArgs({type, path}, attr);
        if (!args) {
          return std::nullopt;
        }
        return *args;
      },
      [&](std::shared_ptr<AutoloadDB> db) -> ArgVec {
        return db->getTypeAttributeArgs(
            type.slice(), path.slice(), attr.slice());
      },
      [&](Data& data, std::vector<folly::dynamic> argsFromDB) -> ArgVec {
        return data.m_typeAttrs.getAttributeArgs(
            {type, path}, attr, std::move(argsFromDB));
      });
}

std::vector<folly::dynamic> SymbolMap::getTypeAttributeArgs(
    const StringData& type,
    const StringData& attribute) {
  return getTypeAttributeArgs(
      Symbol<SymKind::Type>{type}, Symbol<SymKind::Type>{attribute});
}

std::vector<folly::dynamic> SymbolMap::getTypeAliasAttributeArgs(
    Symbol<SymKind::Type> typeAlias,
    Symbol<SymKind::Type> attr) {
  auto path = getSymbolPath(typeAlias);
  if (path == nullptr) {
    return {};
  }
  using ArgVec = std::vector<folly::dynamic>;
  return readOrUpdate<ArgVec>(
      [&](const Data& data) -> Optional<ArgVec> {
        auto const* args =
            data.m_typeAliasAttrs.getAttributeArgs({typeAlias, path}, attr);
        if (!args) {
          return std::nullopt;
        }
        return *args;
      },
      [&](std::shared_ptr<AutoloadDB> db) -> ArgVec {
        return db->getTypeAliasAttributeArgs(
            typeAlias.slice(), path.slice(), attr.slice());
      },
      [&](Data& data, std::vector<folly::dynamic> argsFromDB) -> ArgVec {
        return data.m_typeAliasAttrs.getAttributeArgs(
            {typeAlias, path}, attr, std::move(argsFromDB));
      });
}

std::vector<folly::dynamic> SymbolMap::getTypeAliasAttributeArgs(
    const StringData& typeAlias,
    const StringData& attribute) {
  return getTypeAliasAttributeArgs(
      Symbol<SymKind::Type>{typeAlias}, Symbol<SymKind::Type>{attribute});
}

std::vector<folly::dynamic> SymbolMap::getMethodAttributeArgs(
    Symbol<SymKind::Type> type,
    Symbol<SymKind::Function> method,
    Symbol<SymKind::Type> attr) {
  auto path = getSymbolPath(type);
  if (path == nullptr) {
    return {};
  }
  using ArgVec = std::vector<folly::dynamic>;
  return readOrUpdate<ArgVec>(
      [&](const Data& data) -> Optional<ArgVec> {
        auto const* args =
            data.m_methodAttrs.getAttributeArgs({{type, path}, method}, attr);
        if (!args) {
          return std::nullopt;
        }
        return *args;
      },
      [&](std::shared_ptr<AutoloadDB> db) -> ArgVec {
        return db->getMethodAttributeArgs(
            type.slice(), method.slice(), path.slice(), attr.slice());
      },
      [&](Data& data, std::vector<folly::dynamic> argsFromDB) -> ArgVec {
        return data.m_methodAttrs.getAttributeArgs(
            {{type, path}, method}, attr, std::move(argsFromDB));
      });
}

std::vector<folly::dynamic> SymbolMap::getMethodAttributeArgs(
    const StringData& type,
    const StringData& method,
    const StringData& attribute) {
  return getMethodAttributeArgs(
      Symbol<SymKind::Type>{type},
      Symbol<SymKind::Function>{method},
      Symbol<SymKind::Type>{attribute});
}

std::vector<folly::dynamic> SymbolMap::getFileAttributeArgs(
    Path path,
    Symbol<SymKind::Type> attr) {
  if (path == nullptr) {
    return {};
  }
  using ArgVec = std::vector<folly::dynamic>;
  return readOrUpdate<ArgVec>(
      [&](const Data& data) -> Optional<ArgVec> {
        auto const* args = data.m_fileAttrs.getAttributeArgs({path}, attr);
        if (!args) {
          return std::nullopt;
        }
        return *args;
      },
      [&](std::shared_ptr<AutoloadDB> db) -> ArgVec {
        return db->getFileAttributeArgs(path.slice(), attr.slice());
      },
      [&](Data& data, std::vector<folly::dynamic> argsFromDB) -> ArgVec {
        return data.m_fileAttrs.getAttributeArgs(
            {path}, attr, std::move(argsFromDB));
      });
}

std::vector<folly::dynamic> SymbolMap::getFileAttributeArgs(
    Path path,
    const StringData& attribute) {
  return getFileAttributeArgs(path, Symbol<SymKind::Type>{attribute});
}

Path SymbolMap::getModuleFile(Symbol<SymKind::Module> module) {
  return getSymbolPath(module);
}

Path SymbolMap::getModuleFile(const StringData& module) {
  return getModuleFile(Symbol<SymKind::Module>{module});
}

TypeKind SymbolMap::getKind(Symbol<SymKind::Type> type) {
  return getKindAndFlags(type).first;
}

TypeKind SymbolMap::getKind(const StringData& type) {
  return getKind(Symbol<SymKind::Type>{type});
}

bool SymbolMap::isTypeAbstract(Symbol<SymKind::Type> type) {
  return getKindAndFlags(type).second &
      static_cast<TypeFlagMask>(TypeFlag::Abstract);
}

bool SymbolMap::isTypeAbstract(const StringData& type) {
  return isTypeAbstract(Symbol<SymKind::Type>{type});
}

bool SymbolMap::isTypeFinal(Symbol<SymKind::Type> type) {
  return getKindAndFlags(type).second &
      static_cast<TypeFlagMask>(TypeFlag::Final);
}

bool SymbolMap::isTypeFinal(const StringData& type) {
  return isTypeFinal(Symbol<SymKind::Type>{type});
}

std::pair<TypeKind, TypeFlagMask> SymbolMap::getKindAndFlags(
    Symbol<SymKind::Type> type) {
  return getKindAndFlags(type, getSymbolPath(type));
}

std::pair<TypeKind, TypeFlagMask> SymbolMap::getKindAndFlags(
    Symbol<SymKind::Type> type,
    Path path) {
  if (path == nullptr) {
    return {TypeKind::Unknown, static_cast<TypeFlagMask>(TypeFlag::Empty)};
  }
  return readOrUpdate<std::pair<TypeKind, TypeFlagMask>>(
      [&](const Data& data) -> Optional<std::pair<TypeKind, int>> {
        return data.m_typeKind.getKindAndFlags(type, path);
      },
      [&](std::shared_ptr<AutoloadDB> db) {
        return db->getKindAndFlags(
            type.slice(), fs::path{std::string{path.slice()}});
      },
      [&](Data& data, AutoloadDB::KindAndFlags kindAndFlags)
          -> std::pair<TypeKind, TypeFlagMask> {
        auto [kind, flags] = kindAndFlags;
        if (kind != TypeKind::Unknown) {
          data.m_typeKind.setKindAndFlags(type, path, kind, flags);
        }
        return {kind, flags};
      });
}

Optional<SHA1> SymbolMap::getSha1Hash(Path path) const {
  auto rlock = m_syncedData.rlock();
  auto const& sha1Hashes = rlock->m_sha1Hashes;
  auto const it = sha1Hashes.find(path);
  if (it == sha1Hashes.end()) {
    return {};
  }
  return {it->second};
}

void SymbolMap::update(
    const Clock& since,
    const Clock& clock,
    std::vector<fs::path> alteredPaths,
    std::vector<fs::path> deletedPaths,
    std::vector<FileFacts> alteredPathFacts) {
  ++m_updatesInFlight;
  SCOPE_EXIT {
    --m_updatesInFlight;
  };

#ifndef NDEBUG
  assertx(alteredPaths.size() == alteredPathFacts.size());
  for (auto const& path : alteredPaths) {
    assertx(path.is_relative());
  }
  for (auto const& path : deletedPaths) {
    assertx(path.is_relative());
  }
  for (auto const& facts : alteredPathFacts) {
    assertx(!facts.m_sha1hex.empty());
  }
#endif

  auto wlock = m_syncedData.wlock();

  auto db = getDB();

  // If we have no clock (meaning this is the map's first update since
  // it was constructed), fill our clock from the DB.
  if (wlock->m_clock.isInitial()) {
    wlock->m_clock = db->getClock();
  }

  if (since != wlock->m_clock) {
    throw UpdateExc{folly::sformat(
        "Cannot use information since {} to update a map currently at {}.",
        since,
        wlock->m_clock)};
  }

  // If since and clock are equal, no time has elapsed and therefore
  // no changes have occurred.
  if (since == clock) {
    assertx(alteredPaths.empty() && deletedPaths.empty());
  }

  // Write information about base and derived types
  for (auto i = 0; i < alteredPaths.size(); i++) {
    wlock->updatePath(
        Path{alteredPaths[i]}, alteredPathFacts[i], m_indexedMethodAttrs);
  }

  for (auto const& path : deletedPaths) {
    wlock->removePath(Path{path});
  }

  wlock->m_clock = clock;

  if (!db->isReadOnly()) {
    // Any individual DB update may fail spuriously, but we can't
    // drop updates on failure. So add a work item to the queue
    // and drain the queue as we complete work items successfully.
    wlock->m_updateDBWork.push(
        {since,
         wlock->m_clock,
         std::move(alteredPaths),
         std::move(deletedPaths),
         std::move(alteredPathFacts)});
    wlock->m_updateDBFuture = folly::splitFuture(
        wlock->m_updateDBFuture.getSemiFuture()
            .via(m_exec.get())
            .thenTry([this](folly::Try<folly::Unit>&&) {
              while (true) {
                auto maybeWork = m_syncedData.withRLock(
                    [](const Data& data)
                        -> Optional<HPHP::Facts::UpdateDBWorkItem> {
                      if (data.m_updateDBWork.empty()) {
                        return std::nullopt;
                      }
                      return data.m_updateDBWork.front();
                    });
                if (!maybeWork) {
                  break;
                }
                auto const& work = *maybeWork;
                updateDB(
                    work.m_since,
                    work.m_clock,
                    work.m_alteredPaths,
                    work.m_deletedPaths,
                    work.m_alteredPathFacts);
                auto _workItemToDestroy =
                    m_syncedData.withWLock([&work](Data& data) {
                      Optional<UpdateDBWorkItem> workItemToDestroy;
                      if (data.m_updateDBWork.empty()) {
                        return workItemToDestroy;
                      }
                      if (data.m_updateDBWork.front().m_since == work.m_since) {
                        workItemToDestroy =
                            std::move(data.m_updateDBWork.front());
                        data.m_updateDBWork.pop();
                      }
                      return workItemToDestroy;
                    });
              }
            })
            .thenError(folly::tag_t<SQLiteExc>{}, [](const SQLiteExc& e) {
              switch (e.code()) {
                // SQLITE_BUSY errors are spurious and may occur if multiple
                // processes or threads are writing to the same database at
                // the same time. In these cases it's correct to fail the
                // current update and try again later, and there's no reason
                // to slam stderr with a warning in this case.
                case SQLiteExc::Code::BUSY:
                  XLOG(DBG0)
                      << "Exception while updating autoload DB: " << e.what();
                  break;
                default:
                  XLOG(WARN)
                      << "Exception while updating autoload DB: " << e.what();
              }
            }));
  }
}

Clock SymbolMap::getClock() const noexcept {
  auto rlock = m_syncedData.rlock();
  return rlock->m_clock;
}

Clock SymbolMap::dbClock() const {
  return getDB()->getClock();
}

hphp_hash_set<Path> SymbolMap::getAllPaths() const {
  auto rlock = m_syncedData.rlock();

  hphp_hash_set<Path> allPaths;
  auto db = getDB();

  for (auto&& [path, _] : db->getAllPathsAndHashes()) {
    assertx(path.is_relative());
    allPaths.insert(Path{path});
  }
  for (auto const& [path, _] : rlock->m_sha1Hashes) {
    allPaths.insert(path);
  }
  for (auto const& [path, exists] : rlock->m_fileExistsMap) {
    if (exists) {
      assertx(allPaths.find(path) != allPaths.end());
    } else {
      allPaths.erase(path);
    }
  }
  return allPaths;
}

hphp_hash_map<Path, SHA1> SymbolMap::getAllPathsWithHashes() const {
  auto rlock = m_syncedData.rlock();
  auto db = getDB();

  hphp_hash_map<Path, SHA1> allPaths;
  for (auto&& [path, hash] : db->getAllPathsAndHashes()) {
    assertx(path.is_relative());
    allPaths.insert({Path{path}, SHA1{hash}});
  }
  for (auto const& [path, sha1] : rlock->m_sha1Hashes) {
    allPaths.insert_or_assign(path, sha1);
  }
  for (auto const& [path, exists] : rlock->m_fileExistsMap) {
    if (exists) {
      assertx(allPaths.find(path) != allPaths.end());
    } else {
      allPaths.erase(path);
    }
  }
  return allPaths;
}

void SymbolMap::updateDB(
    const Clock& since,
    const Clock& clock,
    const std::vector<fs::path>& alteredPaths,
    const std::vector<fs::path>& deletedPaths,
    const std::vector<FileFacts>& alteredPathFacts) const {
  assertx(alteredPaths.size() == alteredPathFacts.size());

  if (since == clock) {
    return;
  }

  auto db = getDB();

  // Only update the DB if its clock matches the clock we thought it had.
  //
  // If the clocks don't match, someone updated the DB in the meantime
  // and proceeding might overwrite newer data.
  auto const dbClock = db->getClock();
  if (dbClock != since) {
    throw UpdateExc{folly::sformat(
        "Told to update the DB with information from {}, but DB is currently "
        "at {}",
        since,
        dbClock)};
  }

  for (auto i = 0; i < alteredPaths.size(); ++i) {
    auto const& absPath = alteredPaths.at(i);
    auto const& pathFacts = alteredPathFacts.at(i);
    updateDBPath(*db, absPath, pathFacts);
  }

  for (auto const& path : deletedPaths) {
    db->erasePath(path);
  }

  // ANALYZE after initially building the DB
  if (since.isInitial()) {
    db->runPostBuildOptimizations();
  }

  db->insertClock(clock);
  db->commit();
}

void SymbolMap::updateDBPath(
    AutoloadDB& db,
    const fs::path& path,
    const FileFacts& facts) const {
  assertx(path.is_relative());

  // Bail out early if the hex in memory is identical to the hex in the DB
  if (facts.m_sha1hex == db.getSha1Hex(path)) {
    return;
  }

  // Refresh the path by deleting all its data
  db.erasePath(path);
  db.insertPath(path);
  db.insertSha1Hex(path, facts.m_sha1hex);

  for (auto const& type : facts.m_types) {
    db.insertType(type.m_name, path, type.m_kind, type.m_flags);
    for (auto const& baseType : type.m_baseTypes) {
      db.insertBaseType(path, type.m_name, DeriveKind::Extends, baseType);
    }
    for (auto const& baseType : type.m_requireExtends) {
      db.insertBaseType(
          path, type.m_name, DeriveKind::RequireExtends, baseType);
    }
    for (auto const& baseType : type.m_requireImplements) {
      db.insertBaseType(
          path, type.m_name, DeriveKind::RequireImplements, baseType);
    }
    for (auto const& baseType : type.m_requireClass) {
      db.insertBaseType(path, type.m_name, DeriveKind::RequireClass, baseType);
    }
    for (auto const& attribute : type.m_attributes) {
      if (attribute.m_args.empty()) {
        db.insertTypeAttribute(
            path, type.m_name, attribute.m_name, std::nullopt, nullptr);
      } else {
        for (auto i = 0; i < attribute.m_args.size(); ++i) {
          db.insertTypeAttribute(
              path, type.m_name, attribute.m_name, i, &attribute.m_args[i]);
        }
      }
    }
    for (auto const& methodDetails : type.m_methods) {
      for (auto const& attribute : methodDetails.m_attributes) {
        // If we have an allowlist of method attributes to index, then skip any
        // method attribute which isn't in that allowlist.
        if (!m_indexedMethodAttrs.empty() &&
            !m_indexedMethodAttrs.count(attribute.m_name)) {
          continue;
        }
        if (attribute.m_args.empty()) {
          db.insertMethodAttribute(
              path,
              type.m_name,
              methodDetails.m_name,
              attribute.m_name,
              std::nullopt,
              nullptr);
        } else {
          for (auto i = 0; i < attribute.m_args.size(); ++i) {
            db.insertMethodAttribute(
                path,
                type.m_name,
                methodDetails.m_name,
                attribute.m_name,
                i,
                &attribute.m_args[i]);
          }
        }
      }
    }
  }

  for (auto const& module : facts.m_modules) {
    db.insertModule(module.m_name, path);
  }

  for (auto const& function : facts.m_functions) {
    db.insertFunction(function, path);
  }

  for (auto const& constant : facts.m_constants) {
    db.insertConstant(constant, path);
  }

  for (auto const& attribute : facts.m_attributes) {
    if (attribute.m_args.empty()) {
      db.insertFileAttribute(path, attribute.m_name, std::nullopt, nullptr);
    } else {
      for (auto i = 0; i < attribute.m_args.size(); ++i) {
        db.insertFileAttribute(path, attribute.m_name, i, &attribute.m_args[i]);
      }
    }
  }
}

bool SymbolMap::isPathDeleted(Path path) const noexcept {
  auto rlock = m_syncedData.rlock();
  auto const& fileExistsMap = rlock->m_fileExistsMap;
  auto const it = fileExistsMap.find(path);
  return it != fileExistsMap.end() && !it->second;
}

template <SymKind k>
std::vector<std::pair<Symbol<k>, Path>> SymbolMap::getAllSymbols() {
  hphp_hash_map<Path, std::vector<Symbol<k>>> pathToSymbols;

  auto db = getDB();
  for (auto&& [symbol, path] : getAllSymbolsFromDB<k>(*db)) {
    pathToSymbols[Path{path}].push_back(Symbol<k>{symbol});
  }

  m_syncedData.withRLock([&pathToSymbols](const Data& data) {
    for (auto const& [path, _] : data.m_versions->m_versionMap) {
      auto symbols = getPathSymMap<k>(data).getPathSymbols(path);
      if (!symbols) {
        continue;
      }
      pathToSymbols.erase(path);
      for (auto symbol : *symbols) {
        pathToSymbols[path].push_back(symbol);
      }
    }
  });

  std::vector<std::pair<Symbol<k>, Path>> results;
  results.reserve(pathToSymbols.size());
  for (auto const& [path, symbols] : pathToSymbols) {
    for (auto const& symbol : symbols) {
      results.push_back({symbol, path});
    }
  }
  return results;
}

template <typename Ret, typename ReadFn, typename GetFromDBFn, typename WriteFn>
Ret SymbolMap::readOrUpdate(
    ReadFn readFn,
    GetFromDBFn getFromDBFn,
    WriteFn writeFn) {
  {
    // Try reading with only a reader lock.
    auto rlock = m_syncedData.rlock();
    auto readOnlyData = readFn(*rlock);
    if (readOnlyData) {
      return *readOnlyData;
    }
  }
  auto dataFromDB = [&]() { return getFromDBFn(getDB()); }();
  return m_syncedData.withULockPtr([&](auto ulock) {
    // Try reading once again, in case someone else wrote the value before we
    // took out the upgrade lock.
    auto readOnlyData = readFn(*ulock);
    if (readOnlyData) {
      return *readOnlyData;
    }

    // Now take out a write lock to get the canonical answer.
    auto wlock = ulock.moveFromUpgradeToWrite();
    return writeFn(*wlock, std::move(dataFromDB));
  });
}

template <SymKind k>
Path SymbolMap::getSymbolPath(Symbol<k> symbol) {
  auto symbolPath = [this](auto const& paths) -> Path {
    if (UNLIKELY(paths.empty())) {
      return Path{nullptr};
    } else {
      return *paths.begin();
    }
  };

  return readOrUpdate<Path>(
      [&](const Data& data) -> Optional<Path> {
        auto paths = getPathSymMap<k>(data).getSymbolPaths(symbol);
        if (paths) {
          return {symbolPath(*paths)};
        } else {
          return std::nullopt;
        }
      },
      [&](std::shared_ptr<AutoloadDB> db) {
        auto pathStrs = [&]() -> std::vector<fs::path> {
          switch (k) {
            case SymKind::Type:
              return db->getTypePath(symbol.slice());
            case SymKind::Function:
              return db->getFunctionPath(symbol.slice());
            case SymKind::Constant:
              return db->getConstantPath(symbol.slice());
            case SymKind::Module:
              return db->getModulePath(symbol.slice());
          }
        }();

        std::vector<Path> paths;
        paths.reserve(pathStrs.size());
        std::transform(
            pathStrs.begin(),
            pathStrs.end(),
            std::back_inserter(paths),
            [](auto const& path) { return Path{path}; });
        return paths;
      },
      [&](Data& data, std::vector<Path> pathsFromDB) -> Path {
        return symbolPath(getPathSymMap<k>(data).getSymbolPaths(
            symbol, std::move(pathsFromDB)));
      });
}

template <SymKind k>
typename PathToSymbolsMap<k>::PathSymbolMap::Values SymbolMap::getPathSymbols(
    Path path) {
  using Symbols = typename PathToSymbolsMap<k>::PathSymbolMap::Values;

  return readOrUpdate<Symbols>(
      [&](const Data& data) -> Optional<Symbols> {
        auto existsIt = data.m_fileExistsMap.find(path);
        if (existsIt != data.m_fileExistsMap.end() && !existsIt->second) {
          return Symbols{};
        }
        auto symbols = getPathSymMap<k>(data).getPathSymbols(path);
        if (symbols) {
          return *symbols;
        } else {
          return std::nullopt;
        }
      },
      [&](std::shared_ptr<AutoloadDB> db) {
        auto symbolStrs =
            [&](const fs::path& path) -> std::vector<std::string> {
          assertx(path.is_relative());
          switch (k) {
            case SymKind::Type:
              return db->getPathTypes(path);
            case SymKind::Function:
              return db->getPathFunctions(path);
            case SymKind::Constant:
              return db->getPathConstants(path);
            case SymKind::Module:
              return db->getPathModules(path);
          }
        }(fs::path{std::string{path.slice()}});

        Symbols symbols;
        symbols.reserve(symbolStrs.size());
        std::transform(
            symbolStrs.begin(),
            symbolStrs.end(),
            std::back_inserter(symbols),
            [](auto const& symbol) { return Symbol<k>{symbol}; });
        return symbols;
      },
      [&](Data& data, std::vector<Symbol<k>> symbolsFromDB) -> Symbols {
        return getPathSymMap<k>(data).getPathSymbols(
            path, std::move(symbolsFromDB));
      });
}

SymbolMap::Data::Data()
    : m_versions{std::make_shared<PathVersions>()},
      m_typePath{m_versions},
      m_functionPath{m_versions},
      m_constantPath{m_versions},
      m_modulePath{m_versions},
      m_methodPath{m_versions},
      m_inheritanceInfo{m_versions},
      m_typeAttrs{m_versions},
      m_typeAliasAttrs{m_versions},
      m_methodAttrs{m_versions},
      m_fileAttrs{m_versions} {}

void SymbolMap::Data::updatePath(
    Path path,
    FileFacts facts,
    const hphp_hash_set<std::string>& indexedMethodAttrs) {
  m_versions->bumpVersion(path);

  typename PathToSymbolsMap<SymKind::Type>::Symbols types;
  typename PathToMethodsMap::Methods methods;
  for (auto& type : facts.m_types) {
    always_assert(!type.m_name.empty());
    // ':' is a valid character in XHP classnames, but not Hack
    // classnames. We should have replaced ':' in the parser.
    always_assert(type.m_name.find(':') == -1);
    auto typeName = Symbol<SymKind::Type>{type.m_name};

    types.push_back(typeName);
    m_typeKind.setKindAndFlags(typeName, path, type.m_kind, type.m_flags);
    if (type.m_kind == TypeKind::TypeAlias) {
      m_typeAliasAttrs.setAttributes(
          {typeName, path}, std::move(type.m_attributes));
    } else {
      m_typeAttrs.setAttributes({typeName, path}, std::move(type.m_attributes));
    }
    m_inheritanceInfo.setBaseTypes(
        typeName, path, DeriveKind::Extends, std::move(type.m_baseTypes));
    m_inheritanceInfo.setBaseTypes(
        typeName,
        path,
        DeriveKind::RequireClass,
        std::move(type.m_requireClass));
    m_inheritanceInfo.setBaseTypes(
        typeName,
        path,
        DeriveKind::RequireExtends,
        std::move(type.m_requireExtends));
    m_inheritanceInfo.setBaseTypes(
        typeName,
        path,
        DeriveKind::RequireImplements,
        std::move(type.m_requireImplements));

    for (auto& [method, attributes] : type.m_methods) {
      // Remove method attributes not in the allowlist if the allowlist exists
      if (!indexedMethodAttrs.empty()) {
        attributes.erase(
            std::remove_if(
                attributes.begin(),
                attributes.end(),
                [&](const Attribute& attr) {
                  return !indexedMethodAttrs.count(attr.m_name);
                }),
            attributes.end());
      }

      MethodDecl methodDecl{
          .m_type = {.m_name = typeName, .m_path = path},
          .m_method = Symbol<SymKind::Function>{method}};

      m_methodAttrs.setAttributes(methodDecl, std::move(attributes));
      methods.push_back(methodDecl);
    }
  }

  typename PathToSymbolsMap<SymKind::Module>::Symbols modules;
  for (auto const& module : facts.m_modules) {
    always_assert(!module.m_name.empty());
    modules.push_back(Symbol<SymKind::Module>{module.m_name});
  }

  typename PathToSymbolsMap<SymKind::Function>::Symbols functions;
  for (auto const& function : facts.m_functions) {
    always_assert(!function.empty());
    functions.push_back(Symbol<SymKind::Function>{function});
  }

  typename PathToSymbolsMap<SymKind::Constant>::Symbols constants;
  for (auto const& constant : facts.m_constants) {
    always_assert(!constant.empty());
    constants.push_back(Symbol<SymKind::Constant>{constant});
  }

  m_fileAttrs.setAttributes({path}, facts.m_attributes);

  m_modulePath.replacePathSymbols(path, std::move(modules));
  m_typePath.replacePathSymbols(path, std::move(types));
  m_functionPath.replacePathSymbols(path, std::move(functions));
  m_constantPath.replacePathSymbols(path, std::move(constants));
  m_methodPath.replacePathMethods(path, std::move(methods));
  m_sha1Hashes.insert_or_assign(path, SHA1{facts.m_sha1hex});

  m_fileExistsMap.insert_or_assign(path, true);
}

void SymbolMap::Data::removePath(Path path) {
  m_versions->bumpVersion(path);
  m_fileExistsMap.insert_or_assign(path, false);
}

void SymbolMap::waitForDBUpdate() {
  auto updateDBFuture = m_syncedData.withWLock(
      [](Data& data) { return data.m_updateDBFuture.getFuture(); });
  if (updateDBFuture.valid()) {
    updateDBFuture.wait();
  }
  // Refresh the DB transaction
  try {
    getDB()->commit();
  } catch (const std::runtime_error& e) {
    XLOG(ERR) << e.what();
  }
}

std::shared_ptr<AutoloadDB> SymbolMap::getDB() const {
  return m_dbVault.get();
}

} // namespace Facts
} // namespace HPHP
