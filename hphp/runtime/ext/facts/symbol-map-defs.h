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

#pragma once

#include <folly/ScopeGuard.h>
#include <folly/executors/CPUThreadPoolExecutor.h>

#include "hphp/runtime/ext/facts/exception.h"
#include "hphp/runtime/ext/facts/symbol-map.h"
#include "hphp/util/assertions.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

namespace HPHP {
namespace Facts {

namespace {

/**
 * Get the PathToSymbolsMap corresponding to the given SymKind enum value
 */

// const
template <typename S, SymKind k>
typename std::enable_if<
    k == SymKind::Type,
    const PathToSymbolsMap<S, SymKind::Type>&>::type
getPathSymMap(const typename SymbolMap<S>::Data& data) {
  return data.m_typePath;
}
template <typename S, SymKind k>
typename std::enable_if<
    k == SymKind::Function,
    const PathToSymbolsMap<S, SymKind::Function>&>::type
getPathSymMap(const typename SymbolMap<S>::Data& data) {
  return data.m_functionPath;
}
template <typename S, SymKind k>
typename std::enable_if<
    k == SymKind::Constant,
    const PathToSymbolsMap<S, SymKind::Constant>&>::type
getPathSymMap(const typename SymbolMap<S>::Data& data) {
  return data.m_constantPath;
}

// non-const
template <typename S, SymKind k>
typename std::
    enable_if<k == SymKind::Type, PathToSymbolsMap<S, SymKind::Type>&>::type
    getPathSymMap(typename SymbolMap<S>::Data& data) {
  return data.m_typePath;
}
template <typename S, SymKind k>
typename std::enable_if<
    k == SymKind::Function,
    PathToSymbolsMap<S, SymKind::Function>&>::type
getPathSymMap(typename SymbolMap<S>::Data& data) {
  return data.m_functionPath;
}
template <typename S, SymKind k>
typename std::enable_if<
    k == SymKind::Constant,
    PathToSymbolsMap<S, SymKind::Constant>&>::type
getPathSymMap(typename SymbolMap<S>::Data& data) {
  return data.m_constantPath;
}

} // namespace

template <typename S>
SymbolMap<S>::SymbolMap(
    folly::fs::path root, folly::fs::path dbPath, SQLite::OpenMode dbMode)
    : m_exec{std::make_shared<folly::CPUThreadPoolExecutor>(
          1, std::make_shared<folly::NamedThreadFactory>("Autoload DB update"))}
    , m_root{std::move(root)}
    , m_dbPath{std::move(dbPath)}
    , m_dbMode{dbMode} {
  assertx(m_root.is_absolute());
  assertx(m_dbPath.is_absolute());
}

template <typename S> SymbolMap<S>::~SymbolMap() {
  waitForDBUpdate();
}

template <typename S>
std::optional<Symbol<S, SymKind::Type>>
SymbolMap<S>::getTypeName(const S& typeName) {
  Symbol<S, SymKind::Type> type{typeName};
  auto path = getOnlyPath(type);
  if (path == nullptr) {
    return {};
  }
  auto const& pathTypes = getPathSymbols<SymKind::Type>(path);
  auto it = pathTypes.find(type);
  if (it == pathTypes.end()) {
    return {};
  }
  return *it;
}

template <typename S>
Path<S> SymbolMap<S>::getTypeFile(Symbol<S, SymKind::Type> type) {
  auto path = getOnlyPath(type);
  auto [kind, _] = getKindAndFlags(type, path);
  if (kind == TypeKind::TypeAlias) {
    return Path<S>{nullptr};
  }
  return path;
}

template <typename S> Path<S> SymbolMap<S>::getTypeFile(const S& type) {
  return getTypeFile(Symbol<S, SymKind::Type>{type});
}

template <typename S>
Path<S> SymbolMap<S>::getFunctionFile(Symbol<S, SymKind::Function> function) {
  return getOnlyPath(function);
}

template <typename S> Path<S> SymbolMap<S>::getFunctionFile(const S& function) {
  return getFunctionFile(Symbol<S, SymKind::Function>{function});
}

template <typename S>
Path<S> SymbolMap<S>::getConstantFile(Symbol<S, SymKind::Constant> constant) {
  return getOnlyPath(constant);
}

template <typename S> Path<S> SymbolMap<S>::getConstantFile(const S& constant) {
  return getConstantFile(Symbol<S, SymKind::Constant>{constant});
}

template <typename S>
Path<S> SymbolMap<S>::getTypeAliasFile(Symbol<S, SymKind::Type> typeAlias) {
  auto path = getOnlyPath(typeAlias);
  auto [kind, _] = getKindAndFlags(typeAlias, path);
  if (kind != TypeKind::TypeAlias) {
    return Path<S>{nullptr};
  }
  return path;
}

template <typename S>
Path<S> SymbolMap<S>::getTypeAliasFile(const S& typeAlias) {
  return getTypeAliasFile(Symbol<S, SymKind::Type>{typeAlias});
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>> SymbolMap<S>::getFileTypes(Path<S> path) {
  auto const& symbols = getPathSymbols<SymKind::Type>(path);
  std::vector<Symbol<S, SymKind::Type>> symbolVec;
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

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getFileTypes(const folly::fs::path& path) {
  return getFileTypes(Path<S>{path});
}

template <typename S>
std::vector<Symbol<S, SymKind::Function>>
SymbolMap<S>::getFileFunctions(Path<S> path) {
  auto const& symbols = getPathSymbols<SymKind::Function>(path);
  std::vector<Symbol<S, SymKind::Function>> symbolVec;
  symbolVec.reserve(symbols.size());
  std::copy(symbols.begin(), symbols.end(), std::back_inserter(symbolVec));
  return symbolVec;
}

template <typename S>
std::vector<Symbol<S, SymKind::Function>>
SymbolMap<S>::getFileFunctions(const folly::fs::path& path) {
  return getFileFunctions(Path<S>{path});
}

template <typename S>
std::vector<Symbol<S, SymKind::Constant>>
SymbolMap<S>::getFileConstants(Path<S> path) {
  auto const& symbols = getPathSymbols<SymKind::Constant>(path);
  std::vector<Symbol<S, SymKind::Constant>> symbolVec;
  symbolVec.reserve(symbols.size());
  std::copy(symbols.begin(), symbols.end(), std::back_inserter(symbolVec));
  return symbolVec;
}

template <typename S>
std::vector<Symbol<S, SymKind::Constant>>
SymbolMap<S>::getFileConstants(const folly::fs::path& path) {
  return getFileConstants(Path<S>{path});
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getFileTypeAliases(Path<S> path) {
  auto const& symbols = getPathSymbols<SymKind::Type>(path);
  std::vector<Symbol<S, SymKind::Type>> symbolVec;
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

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getFileTypeAliases(const folly::fs::path& path) {
  return getFileTypeAliases(Path<S>{path});
}

template <typename S>
std::vector<std::pair<Symbol<S, SymKind::Type>, Path<S>>>
SymbolMap<S>::getAllTypes() {
  waitForDBUpdate();
  auto& db = getDB();
  auto txn = db.begin();
  std::vector<std::pair<Symbol<S, SymKind::Type>, Path<S>>> results;
  for (auto&& [symbol, path] : db.getAllTypePaths(txn)) {
    auto typeName = Symbol<S, SymKind::Type>{symbol};
    auto [kind, _] = getKindAndFlags(typeName, Path<S>{path});
    if (kind != TypeKind::TypeAlias) {
      results.emplace_back(typeName, Path<S>{path});
    }
  }
  return results;
}

template <typename S>
std::vector<std::pair<Symbol<S, SymKind::Function>, Path<S>>>
SymbolMap<S>::getAllFunctions() {
  waitForDBUpdate();
  auto& db = getDB();
  auto txn = db.begin();
  std::vector<std::pair<Symbol<S, SymKind::Function>, Path<S>>> results;
  for (auto&& [symbol, path] : db.getAllFunctionPaths(txn)) {
    results.emplace_back(Symbol<S, SymKind::Function>{symbol}, Path<S>{path});
  }
  return results;
}

template <typename S>
std::vector<std::pair<Symbol<S, SymKind::Constant>, Path<S>>>
SymbolMap<S>::getAllConstants() {
  waitForDBUpdate();
  auto& db = getDB();
  auto txn = db.begin();
  std::vector<std::pair<Symbol<S, SymKind::Constant>, Path<S>>> results;
  for (auto&& [symbol, path] : db.getAllConstantPaths(txn)) {
    results.emplace_back(Symbol<S, SymKind::Constant>{symbol}, Path<S>{path});
  }
  return results;
}

template <typename S>
std::vector<std::pair<Symbol<S, SymKind::Type>, Path<S>>>
SymbolMap<S>::getAllTypeAliases() {
  waitForDBUpdate();
  auto& db = getDB();
  auto txn = db.begin();
  std::vector<std::pair<Symbol<S, SymKind::Type>, Path<S>>> results;
  for (auto&& [symbol, path] : db.getAllTypePaths(txn)) {
    auto typeName = Symbol<S, SymKind::Type>{symbol};
    auto [kind, _] = getKindAndFlags(typeName, Path<S>{path});
    if (kind == TypeKind::TypeAlias) {
      results.emplace_back(typeName, Path<S>{path});
    }
  }
  return results;
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>> SymbolMap<S>::getBaseTypes(
    Symbol<S, SymKind::Type> derivedType, DeriveKind kind) {
  auto derivedTypePath = getOnlyPath(derivedType);
  if (derivedTypePath == nullptr) {
    return {};
  }

  using TypeSet = typename InheritanceInfo<S>::TypeSet;
  using TypeVec = std::vector<Symbol<S, SymKind::Type>>;
  auto makeVec = [](const TypeSet& baseTypes) -> TypeVec {
    TypeVec baseTypeVec;
    baseTypeVec.reserve(baseTypes.size());
    for (auto const& [type, _] : baseTypes) {
      baseTypeVec.push_back(type);
    }
    return baseTypeVec;
  };

  return readOrUpdate<TypeVec>(
      [&](const Data& data) -> std::optional<TypeVec> {
        auto const* baseTypes = data.m_inheritanceInfo.getBaseTypes(
            derivedType, derivedTypePath, kind);
        if (!baseTypes) {
          return std::nullopt;
        }
        return makeVec(*baseTypes);
      },
      [&](AutoloadDB& db, SQLiteTxn& txn) -> std::vector<SubtypeQuery<S>> {
        auto const symbolStrs = db.getBaseTypes(
            txn,
            folly::fs::path{std::string{derivedTypePath.slice()}},
            derivedType.slice(),
            kind);
        std::vector<SubtypeQuery<S>> symbols;
        symbols.reserve(symbolStrs.size());
        for (auto const& symbolStr : symbolStrs) {
          symbols.push_back(
              {.m_type = Symbol<S, SymKind::Type>{symbolStr}, .m_kind = kind});
        }
        return symbols;
      },
      [&](Data& data, std::vector<SubtypeQuery<S>> edgesFromDB) -> TypeVec {
        return makeVec(data.m_inheritanceInfo.getBaseTypes(
            derivedType, derivedTypePath, kind, std::move(edgesFromDB)));
      });
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getBaseTypes(const S& derivedType, DeriveKind kind) {
  return getBaseTypes(Symbol<S, SymKind::Type>{derivedType}, kind);
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>> SymbolMap<S>::getDerivedTypes(
    Symbol<S, SymKind::Type> baseType, DeriveKind kind) {
  // Return empty results if the given type doesn't have a single definition
  if (getOnlyPath(baseType) == nullptr) {
    return {};
  }

  using TypeDefSet = typename InheritanceInfo<S>::TypeDefSet;
  using TypeVec = std::vector<Symbol<S, SymKind::Type>>;
  auto makeVec = [&](const TypeDefSet& subtypeDefs) -> TypeVec {
    TypeVec subtypes;
    subtypes.reserve(subtypeDefs.size());
    for (auto const& [type, defKind, _path] : subtypeDefs) {
      assertx(defKind == kind);
      subtypes.push_back(type);
    }
    return subtypes;
  };
  auto subtypes = readOrUpdate<TypeVec>(
      [&](const Data& data) -> std::optional<TypeVec> {
        auto const* derivedTypes =
            data.m_inheritanceInfo.getDerivedTypes(baseType, kind);
        if (!derivedTypes) {
          return std::nullopt;
        }
        return makeVec(*derivedTypes);
      },
      [&](AutoloadDB& db, SQLiteTxn& txn) -> std::vector<EdgeToSupertype<S>> {
        auto const typeDefStrs =
            db.getDerivedTypes(txn, baseType.slice(), kind);
        std::vector<EdgeToSupertype<S>> typeDefs;
        typeDefs.reserve(typeDefStrs.size());
        for (auto const& [pathStr, typeStr] : typeDefStrs) {
          typeDefs.push_back(EdgeToSupertype<S>{
              .m_type = Symbol<S, SymKind::Type>{typeStr},
              .m_kind = kind,
              .m_path = Path<S>{pathStr}});
        }
        return typeDefs;
      },
      [&](Data& data, std::vector<EdgeToSupertype<S>> edgesFromDB) -> TypeVec {
        return makeVec(data.m_inheritanceInfo.getDerivedTypes(
            baseType, kind, std::move(edgesFromDB)));
      });
  // Remove types that are duplicate-defined or missing
  subtypes.erase(
      std::remove_if(
          subtypes.begin(),
          subtypes.end(),
          [this](const Symbol<S, SymKind::Type>& subtype) {
            return getOnlyPath(subtype) == nullptr;
          }),
      subtypes.end());
  return subtypes;
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getDerivedTypes(const S& baseType, DeriveKind kind) {
  return getDerivedTypes(Symbol<S, SymKind::Type>{baseType}, kind);
}

template <typename S>
std::vector<typename SymbolMap<S>::DerivedTypeInfo>
SymbolMap<S>::getTransitiveDerivedTypes(
    Symbol<S, SymKind::Type> baseType,
    TypeKindMask kinds,
    DeriveKindMask deriveKinds) {
  // Wait for the DB to fully update, then make a single query to the now
  // up-to-date DB.
  waitForDBUpdate();
  auto& db = getDB();
  auto txn = db.begin();
  std::vector<DerivedTypeInfo> derivedTypes;
  for (auto const& [type, path, kind, flags] : db.getTransitiveDerivedTypes(
           txn, baseType.slice(), kinds, deriveKinds)) {
    derivedTypes.emplace_back(
        Symbol<S, SymKind::Type>{type}, Path<S>{path}, kind, flags);
  }
  return derivedTypes;
}

template <typename S>
std::vector<typename SymbolMap<S>::DerivedTypeInfo>
SymbolMap<S>::getTransitiveDerivedTypes(
    const S& baseType, TypeKindMask kinds, DeriveKindMask deriveKinds) {
  return getTransitiveDerivedTypes(
      Symbol<S, SymKind::Type>{baseType}, kinds, deriveKinds);
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getAttributesOfType(Symbol<S, SymKind::Type> type) {
  auto path = getOnlyPath(type);
  if (path == nullptr) {
    return {};
  }
  using AttrVec = std::vector<Symbol<S, SymKind::Type>>;
  auto makeVec = [&](auto const& attrs) -> AttrVec {
    AttrVec attrVec;
    attrVec.reserve(attrs.size());
    for (auto const& attr : attrs) {
      attrVec.emplace_back(attr);
    }
    return attrVec;
  };
  return readOrUpdate<AttrVec>(
      [&](const Data& data) -> std::optional<AttrVec> {
        auto const* attrs = data.m_typeAttrs.getAttributesForType(type, path);
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](AutoloadDB& db,
          SQLiteTxn& txn) -> std::vector<Symbol<S, SymKind::Type>> {
        auto const attrStrs = db.getAttributesOfType(
            txn, type.slice(), folly::fs::path{std::string{path.slice()}});
        std::vector<Symbol<S, SymKind::Type>> attrs;
        attrs.reserve(attrStrs.size());
        for (auto const& attrStr : attrStrs) {
          attrs.emplace_back(attrStr);
        }
        return attrs;
      },
      [&](Data& data,
          std::vector<Symbol<S, SymKind::Type>> attrsFromDB) -> AttrVec {
        return makeVec(data.m_typeAttrs.getAttributesForType(
            type, path, std::move(attrsFromDB)));
      });
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getAttributesOfType(const S& type) {
  return getAttributesOfType(Symbol<S, SymKind::Type>{type});
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getTypesAndTypeAliasesWithAttribute(
    Symbol<S, SymKind::Type> attr) {
  using TypeVec = std::vector<Symbol<S, SymKind::Type>>;
  auto makeVec = [&](auto const& typeDefs) -> TypeVec {
    TypeVec typeVec;
    typeVec.reserve(typeDefs.size());
    for (auto const& [type, typePath] : typeDefs) {
      typeVec.emplace_back(type);
    }
    return typeVec;
  };
  auto types = readOrUpdate<TypeVec>(
      [&](const Data& data) -> std::optional<TypeVec> {
        auto const* attrs = data.m_typeAttrs.getTypesWithAttribute(attr);
        if (!attrs) {
          return std::nullopt;
        }
        return makeVec(*attrs);
      },
      [&](AutoloadDB& db, SQLiteTxn& txn) -> std::vector<TypeAndPath<S>> {
        auto const typeDefStrs = db.getTypesWithAttribute(txn, attr.slice());
        std::vector<TypeAndPath<S>> typeDefs;
        typeDefs.reserve(typeDefStrs.size());
        for (auto const& [type, path] : typeDefStrs) {
          typeDefs.push_back({Symbol<S, SymKind::Type>{type}, Path<S>{path}});
        }
        return typeDefs;
      },
      [&](Data& data, std::vector<TypeAndPath<S>> typesFromDB) -> TypeVec {
        return makeVec(data.m_typeAttrs.getTypesWithAttribute(
            attr, std::move(typesFromDB)));
      });
  // Remove types that are duplicate-defined or missing
  types.erase(
      std::remove_if(
          types.begin(),
          types.end(),
          [this](const Symbol<S, SymKind::Type>& type) {
            return getOnlyPath(type) == nullptr;
          }),
      types.end());
  return types;
}

template <typename S>
std::vector<Symbol<S, SymKind::Type>>
SymbolMap<S>::getTypesAndTypeAliasesWithAttribute(const S& attr) {
  return getTypesAndTypeAliasesWithAttribute(Symbol<S, SymKind::Type>{attr});
}

template <typename S>
std::vector<folly::dynamic> SymbolMap<S>::getAttributeArgs(
    Symbol<S, SymKind::Type> type, Symbol<S, SymKind::Type> attr) {
  auto path = getOnlyPath(type);
  if (path == nullptr) {
    return {};
  }
  using ArgVec = std::vector<folly::dynamic>;
  return readOrUpdate<ArgVec>(
      [&](const Data& data) -> std::optional<ArgVec> {
        auto const* args = data.m_typeAttrs.getAttributeArgs(type, path, attr);
        if (!args) {
          return std::nullopt;
        }
        return *args;
      },
      [&](AutoloadDB& db, SQLiteTxn& txn) -> ArgVec {
        return db.getAttributeArgs(
            txn, type.slice(), path.slice(), attr.slice());
      },
      [&](Data& data, std::vector<folly::dynamic> argsFromDB) -> ArgVec {
        return data.m_typeAttrs.getAttributeArgs(
            type, path, attr, std::move(argsFromDB));
      });
}

template <typename S>
std::vector<folly::dynamic>
SymbolMap<S>::getAttributeArgs(const S& type, const S& attribute) {
  return getAttributeArgs(
      Symbol<S, SymKind::Type>{type}, Symbol<S, SymKind::Type>{attribute});
}

template <typename S>
TypeKind SymbolMap<S>::getKind(Symbol<S, SymKind::Type> type) {
  return getKindAndFlags(type).first;
}

template <typename S> TypeKind SymbolMap<S>::getKind(const S& type) {
  return getKind(Symbol<S, SymKind::Type>{type});
}

template <typename S>
bool SymbolMap<S>::isTypeAbstract(Symbol<S, SymKind::Type> type) {
  return getKindAndFlags(type).second &
         static_cast<TypeFlagMask>(TypeFlag::Abstract);
}
template <typename S> bool SymbolMap<S>::isTypeAbstract(const S& type) {
  return isTypeAbstract(Symbol<S, SymKind::Type>{type});
}

template <typename S>
bool SymbolMap<S>::isTypeFinal(Symbol<S, SymKind::Type> type) {
  return getKindAndFlags(type).second &
         static_cast<TypeFlagMask>(TypeFlag::Final);
}

template <typename S> bool SymbolMap<S>::isTypeFinal(const S& type) {
  return isTypeFinal(Symbol<S, SymKind::Type>{type});
}

template <typename S>
std::pair<TypeKind, TypeFlagMask>
SymbolMap<S>::getKindAndFlags(Symbol<S, SymKind::Type> type) {
  return getKindAndFlags(type, getOnlyPath(type));
}

template <typename S>
std::pair<TypeKind, TypeFlagMask>
SymbolMap<S>::getKindAndFlags(Symbol<S, SymKind::Type> type, Path<S> path) {
  if (path == nullptr) {
    return {TypeKind::Unknown, static_cast<TypeFlagMask>(TypeFlag::Empty)};
  }
  return readOrUpdate<std::pair<TypeKind, TypeFlagMask>>(
      [&](const Data& data) -> std::optional<std::pair<TypeKind, int>> {
        return data.m_typeKind.getKindAndFlags(type, path);
      },
      [&](AutoloadDB& db, SQLiteTxn& txn) -> std::pair<TypeKind, TypeFlagMask> {
        return db.getKindAndFlags(
            txn, type.slice(), folly::fs::path{std::string{path.slice()}});
      },
      [&](Data& data, std::pair<TypeKind, TypeFlagMask> kindAndFlags)
          -> std::pair<TypeKind, TypeFlagMask> {
        auto [kind, flags] = kindAndFlags;
        if (kind != TypeKind::Unknown) {
          data.m_typeKind.setKindAndFlags(type, path, kind, flags);
        }
        return {kind, flags};
      });
}

template <typename S>
std::optional<SHA1> SymbolMap<S>::getSha1Hash(Path<S> path) const {
  auto rlock = m_syncedData.rlock();
  auto const& sha1Hashes = rlock->m_sha1Hashes;
  auto const it = sha1Hashes.find(path);
  if (it == sha1Hashes.end()) {
    return {};
  }
  return {it->second};
}

template <typename S>
void SymbolMap<S>::update(
    const std::string_view since,
    const std::string_view clock,
    std::vector<folly::fs::path> alteredPaths,
    std::vector<folly::fs::path> deletedPaths,
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

  auto& db = getDB();
  auto txn = db.begin();

  // If we have no clock (meaning this is the map's first update since
  // it was constructed), fill our clock from the DB.
  if (wlock->m_clock.empty()) {
    wlock->m_clock = db.getClock(txn);
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
    wlock->removePath(db, txn, Path<S>{alteredPaths[i]});
    wlock->updatePath(Path<S>{alteredPaths[i]}, alteredPathFacts[i]);
  }

  for (auto const& path : deletedPaths) {
    wlock->removePath(db, txn, Path<S>{path});
  }

  wlock->m_clock = clock;

  if (m_dbMode == SQLite::OpenMode::ReadWrite) {
    // Any individual DB update may fail spuriously, but we can't
    // drop updates on failure. So add a work item to the queue
    // and drain the queue as we complete work items successfully.
    wlock->m_updateDBWork.push(
        {std::string{since},
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
                        -> std::optional<HPHP::Facts::UpdateDBWorkItem> {
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
                      std::optional<UpdateDBWorkItem> workItemToDestroy;
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
                  Logger::Info(
                      "Exception while updating autoload DB: %s", e.what());
                  break;
                default:
                  Logger::Warning(
                      "Exception while updating autoload DB: %s", e.what());
              }
            }));
  }
}

template <typename S> std::string SymbolMap<S>::getClock() const noexcept {
  auto rlock = m_syncedData.rlock();
  return rlock->m_clock;
}

template <typename S> std::string SymbolMap<S>::dbClock() const {
  auto& db = getDB();
  auto txn = db.begin();
  return db.getClock(txn);
}

template <typename S> hphp_hash_set<Path<S>> SymbolMap<S>::getAllPaths() const {
  auto rlock = m_syncedData.rlock();

  hphp_hash_set<Path<S>> allPaths;
  auto& db = getDB();
  auto txn = db.begin();

  for (auto&& [path, _] : db.getAllPathsAndHashes(txn)) {
    assertx(path.is_relative());
    allPaths.insert(Path<S>{path});
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

template <typename S>
hphp_hash_map<Path<S>, SHA1> SymbolMap<S>::getAllPathsWithHashes() const {
  auto rlock = m_syncedData.rlock();
  auto& db = getDB();
  auto txn = db.begin();

  hphp_hash_map<Path<S>, SHA1> allPaths;
  for (auto&& [path, hash] : db.getAllPathsAndHashes(txn)) {
    assertx(path.is_relative());
    allPaths.insert({Path<S>{path}, SHA1{hash}});
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

template <typename S>
void SymbolMap<S>::updateDB(
    const std::string_view since,
    const std::string_view clock,
    const std::vector<folly::fs::path>& alteredPaths,
    const std::vector<folly::fs::path>& deletedPaths,
    const std::vector<FileFacts>& alteredPathFacts) const {
  assertx(alteredPaths.size() == alteredPathFacts.size());

  if (since == clock) {
    return;
  }

  auto& db = getDB();
  auto txn = db.begin();

  // Only update the DB if its clock matches the clock we thought it had.
  //
  // If the clocks don't match, someone updated the DB in the meantime
  // and proceeding might overwrite newer data.
  auto const dbClock = db.getClock(txn);
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
    updateDBPath(db, txn, absPath, pathFacts);
  }

  for (auto const& path : deletedPaths) {
    db.erasePath(txn, path);
  }

  // ANALYZE after initially building the DB
  if (since.empty()) {
    try {
      auto DEBUG_ONLY t0 = std::chrono::steady_clock::now();
      FTRACE_MOD(
          Trace::facts,
          2,
          "Running ANALYZE on {}...\n",
          m_dbPath.native());
      db.analyze();
      auto DEBUG_ONLY tf = std::chrono::steady_clock::now();
      FTRACE_MOD(
          Trace::facts,
          2,
          "Finished ANALYZE on {} in {:.3} seconds.\n",
          m_dbPath.native(),
          static_cast<double>(
              std::chrono::duration_cast<std::chrono::milliseconds>(tf - t0)
                  .count()) /
              1000);
    } catch (const SQLiteExc& e) {
      FTRACE_MOD(
          Trace::facts,
          1,
          "Error while running ANALYZE on {}: {}\n",
          m_dbPath.native(),
          e.what());
    } catch (std::exception& e) {
      FTRACE_MOD(
          Trace::facts,
          1,
          "Error while running ANALYZE on {}: {}\n",
          m_dbPath.native(),
          e.what());
    }
  }

  db.insertClock(txn, clock);
  txn.commit();
}

template <typename S>
void SymbolMap<S>::updateDBPath(
    AutoloadDB& db,
    SQLiteTxn& txn,
    const folly::fs::path& path,
    const FileFacts& facts) const {
  assertx(path.is_relative());

  // Bail out early if the hex in memory is identical to the hex in the DB
  if (facts.m_sha1hex == db.getSha1Hex(txn, path)) {
    return;
  }

  // Refresh the path by deleting all its data
  db.erasePath(txn, path);
  db.insertPath(txn, path);
  db.insertSha1Hex(txn, path, facts.m_sha1hex);

  for (auto const& type : facts.m_types) {

    db.insertType(txn, type.m_name, path, type.m_kind, type.m_flags);
    for (auto const& baseType : type.m_baseTypes) {
      db.insertBaseType(txn, path, type.m_name, DeriveKind::Extends, baseType);
    }
    for (auto const& baseType : type.m_requireExtends) {
      db.insertBaseType(
          txn, path, type.m_name, DeriveKind::RequireExtends, baseType);
    }
    for (auto const& baseType : type.m_requireImplements) {
      db.insertBaseType(
          txn, path, type.m_name, DeriveKind::RequireImplements, baseType);
    }
    for (auto const& attribute : type.m_attributes) {
      if (attribute.m_args.empty()) {
        db.insertTypeAttribute(
            txn, path, type.m_name, attribute.m_name, std::nullopt, nullptr);
      } else {
        for (auto i = 0; i < attribute.m_args.size(); ++i) {
          db.insertTypeAttribute(
              txn,
              path,
              type.m_name,
              attribute.m_name,
              i,
              &attribute.m_args[i]);
        }
      }
    }
  }

  for (auto const& function : facts.m_functions) {
    db.insertFunction(txn, function, path);
  }

  for (auto const& constant : facts.m_constants) {
    db.insertConstant(txn, constant, path);
  }
}

template <typename S>
bool SymbolMap<S>::isPathDeleted(Path<S> path) const noexcept {
  auto rlock = m_syncedData.rlock();
  auto const& fileExistsMap = rlock->m_fileExistsMap;
  auto const it = fileExistsMap.find(path);
  return it != fileExistsMap.end() && !it->second;
}

template <typename S>
template <typename Ret, typename ReadFn, typename GetFromDBFn, typename WriteFn>
Ret SymbolMap<S>::readOrUpdate(
    ReadFn readFn, GetFromDBFn getFromDBFn, WriteFn writeFn) {
  {
    // Try reading with only a reader lock.
    auto rlock = m_syncedData.rlock();
    auto readOnlyData = readFn(*rlock);
    if (readOnlyData) {
      return *readOnlyData;
    }
  }
  auto dataFromDB = [&]() {
    auto& db = getDB();
    auto txn = db.begin();
    return getFromDBFn(db, txn);
  }();
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

template <typename S>
template <SymKind k>
Path<S> SymbolMap<S>::getOnlyPath(Symbol<S, k> symbol) {
  auto onlyPath = [](auto const& paths) -> Path<S> {
    if (LIKELY(paths.size() == 1)) {
      return *paths.begin();
    } else {
      return Path<S>{nullptr};
    }
  };

  return readOrUpdate<Path<S>>(
      [&](const Data& data) -> std::optional<Path<S>> {
        auto paths = getPathSymMap<S, k>(data).getSymbolPaths(symbol);
        if (paths) {
          return {onlyPath(*paths)};
        } else {
          return std::nullopt;
        }
      },
      [&](AutoloadDB& db, SQLiteTxn& txn) {
        auto pathStrs = [&]() -> std::vector<folly::fs::path> {
          switch (k) {
            case SymKind::Type:
              return db.getTypePath(txn, symbol.slice());
            case SymKind::Function:
              return db.getFunctionPath(txn, symbol.slice());
            case SymKind::Constant:
              return db.getConstantPath(txn, symbol.slice());
          }
        }();

        std::vector<Path<S>> paths;
        paths.reserve(pathStrs.size());
        std::transform(
            pathStrs.begin(),
            pathStrs.end(),
            std::back_inserter(paths),
            [](auto const& path) { return Path<S>{path}; });
        return paths;
      },
      [&](Data& data, std::vector<Path<S>> pathsFromDB) -> Path<S> {
        return onlyPath(getPathSymMap<S, k>(data).getSymbolPaths(
            symbol, std::move(pathsFromDB)));
      });
}

template <typename S>
template <SymKind k>
const typename PathToSymbolsMap<S, k>::PathSymbolMap::ValuesSet&
SymbolMap<S>::getPathSymbols(Path<S> path) {
  using SymbolSetRef = std::reference_wrapper<
      const typename PathToSymbolsMap<S, k>::PathSymbolMap::ValuesSet>;

  return readOrUpdate<SymbolSetRef>(
      [&](const Data& data) -> std::optional<SymbolSetRef> {
        auto const* symbols = getPathSymMap<S, k>(data).getPathSymbols(path);
        if (symbols) {
          return {*symbols};
        } else {
          return std::nullopt;
        }
      },
      [&](AutoloadDB& db, SQLiteTxn& txn) {
        auto symbolStrs =
            [&](const folly::fs::path& path) -> std::vector<std::string> {
          assertx(path.is_relative());
          switch (k) {
            case SymKind::Type:
              return db.getPathTypes(txn, path);
            case SymKind::Function:
              return db.getPathFunctions(txn, path);
            case SymKind::Constant:
              return db.getPathConstants(txn, path);
          }
        }(folly::fs::path{std::string{path.slice()}});

        std::vector<Symbol<S, k>> symbols;
        symbols.reserve(symbolStrs.size());
        std::transform(
            symbolStrs.begin(),
            symbolStrs.end(),
            std::back_inserter(symbols),
            [](auto const& symbol) { return Symbol<S, k>{symbol}; });
        return symbols;
      },
      [&](Data& data, std::vector<Symbol<S, k>> symbolsFromDB) -> SymbolSetRef {
        return getPathSymMap<S, k>(data).getPathSymbols(
            path, std::move(symbolsFromDB));
      });
}

template <typename S>
void SymbolMap<S>::Data::updatePath(Path<S> path, FileFacts facts) {
  typename PathToSymbolsMap<S, SymKind::Type>::SymbolSet types;
  for (auto& type : facts.m_types) {
    auto typeName = Symbol<S, SymKind::Type>{type.m_name};

    types.insert(typeName);
    m_typeKind.setKindAndFlags(typeName, path, type.m_kind, type.m_flags);
    m_typeAttrs.setTypeAttributes(typeName, path, std::move(type.m_attributes));
    m_inheritanceInfo.setBaseTypes(
        typeName, path, DeriveKind::Extends, std::move(type.m_baseTypes));
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
  }

  typename PathToSymbolsMap<S, SymKind::Function>::SymbolSet functions;
  for (auto const& function : facts.m_functions) {
    functions.insert(Symbol<S, SymKind::Function>{function});
  }

  typename PathToSymbolsMap<S, SymKind::Constant>::SymbolSet constants;
  for (auto const& constant : facts.m_constants) {
    constants.insert(Symbol<S, SymKind::Constant>{constant});
  }

  m_typePath.replacePathSymbols(path, std::move(types));
  m_functionPath.replacePathSymbols(path, std::move(functions));
  m_constantPath.replacePathSymbols(path, std::move(constants));
  m_sha1Hashes.insert_or_assign(path, SHA1{facts.m_sha1hex});

  m_fileExistsMap.insert_or_assign(path, true);
}

template <typename S>
void SymbolMap<S>::Data::removePath(
    AutoloadDB& db, SQLiteTxn& txn, Path<S> path) {
  auto pathTypesFromDBStrs = db.getPathTypes(txn, {std::string{path.slice()}});
  std::vector<Symbol<S, SymKind::Type>> pathTypesFromDB;
  pathTypesFromDB.reserve(pathTypesFromDBStrs.size());
  for (auto const& type : pathTypesFromDBStrs) {
    pathTypesFromDB.emplace_back(type);
  }
  auto pathTypes = m_typePath.getPathSymbols(path, std::move(pathTypesFromDB));
  for (auto type : pathTypes) {
    m_inheritanceInfo.removeType(type, path);
    m_typeAttrs.removeType(db, txn, type, path);
  }

  m_typePath.removePath(path);
  m_functionPath.removePath(path);
  m_constantPath.removePath(path);

  m_fileExistsMap.insert_or_assign(path, false);
}

template <typename S> void SymbolMap<S>::waitForDBUpdate() {
  auto updateDBFuture = m_syncedData.withWLock(
      [](Data& data) { return data.m_updateDBFuture.getFuture(); });
  if (updateDBFuture.valid()) {
    updateDBFuture.wait();
  }
}

template <typename S> AutoloadDB& SymbolMap<S>::getDB() const {
  return HPHP::Facts::getDB(m_dbPath, m_dbMode);
}

} // namespace Facts
} // namespace HPHP
