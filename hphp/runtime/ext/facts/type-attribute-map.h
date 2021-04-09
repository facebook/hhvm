// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <string>
#include <utility>
#include <vector>

#include <folly/dynamic.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/hash/Hash.h>

#include "hphp/runtime/ext/facts/attribute-argument-map.h"
#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/symbol-types.h"

namespace HPHP {
namespace Facts {

template <typename S>
using TypeAndPath = std::pair<Symbol<S, SymKind::Type>, Path<S>>;

template <typename S> struct TypeAttributeMap {
  using TypeToAttributeMap =
      LazyTwoWayMap<TypeAndPath<S>, Symbol<S, SymKind::Type>>;

  using TypeDefSet = typename TypeToAttributeMap::KeysSet;
  using AttrSet = typename TypeToAttributeMap::ValuesSet;

  const AttrSet*
  getAttributesForType(Symbol<S, SymKind::Type> type, Path<S> typePath) const {
    return m_typeAttrMap.getValuesForKey({type, typePath});
  }
  const AttrSet& getAttributesForType(
      Symbol<S, SymKind::Type> type,
      Path<S> typePath,
      std::vector<Symbol<S, SymKind::Type>> attrsFromDB) {
    return m_typeAttrMap.getValuesForKey(
        {type, typePath}, std::move(attrsFromDB));
  }

  const TypeDefSet* getTypesWithAttribute(Symbol<S, SymKind::Type> attr) const {
    return m_typeAttrMap.getKeysForValue(attr);
  }
  const TypeDefSet& getTypesWithAttribute(
      Symbol<S, SymKind::Type> attr, std::vector<TypeAndPath<S>> typesFromDB) {
    return m_typeAttrMap.getKeysForValue(attr, std::move(typesFromDB));
  }

  void setTypeAttributes(
      Symbol<S, SymKind::Type> type,
      Path<S> typePath,
      std::vector<TypeAttribute> attrVec) {
    AttrSet attrs;
    attrs.reserve(attrVec.size());
    for (auto& attr : attrVec) {
      auto attrSym = Symbol<S, SymKind::Type>{attr.m_name};
      attrs.emplace(attrSym);
      m_attrArgs.setAttributeArgs(
          type, typePath, attrSym, std::move(attr.m_args));
    }
    m_typeAttrMap.setValuesForKey({type, typePath}, std::move(attrs));
  }

  const std::vector<folly::dynamic>* getAttributeArgs(
      Symbol<S, SymKind::Type> type,
      Path<S> path,
      Symbol<S, SymKind::Type> attr) const {
    return m_attrArgs.getAttributeArgs(type, path, attr);
  }

  const std::vector<folly::dynamic>& getAttributeArgs(
      Symbol<S, SymKind::Type> type,
      Path<S> path,
      Symbol<S, SymKind::Type> attr,
      std::vector<folly::dynamic> argsFromDB) {
    return m_attrArgs.getAttributeArgs(type, path, attr, std::move(argsFromDB));
  }

  void removeType(
      AutoloadDB& db,
      SQLiteTxn& txn,
      Symbol<S, SymKind::Type> type,
      Path<S> typePath) {
    m_typeAttrMap.setValuesForKey({type, typePath}, {});
    auto attrsFromDBStrs = db.getAttributesOfType(
        txn, type.slice(), {std::string{typePath.slice()}});
    std::vector<Symbol<S, SymKind::Type>> attrsFromDB;
    attrsFromDB.reserve(attrsFromDBStrs.size());
    for (auto const& attrStr : attrsFromDBStrs) {
      attrsFromDB.emplace_back(attrStr);
    }
    auto typeAttrs =
        getAttributesForType(type, typePath, std::move(attrsFromDB));
    for (auto attr : typeAttrs) {
      m_attrArgs.setAttributeArgs(type, typePath, attr, {});
    }
  }

  TypeToAttributeMap m_typeAttrMap;
  AttributeArgumentMap<S> m_attrArgs;
};

} // namespace Facts
} // namespace HPHP

template <typename S> struct std::hash<HPHP::Facts::TypeAndPath<S>> {
  size_t operator()(const HPHP::Facts::TypeAndPath<S>& typeDef) const {
    auto const& [type, path] = typeDef;
    return folly::hash::hash_combine(
        std::hash<HPHP::Facts::Symbol<S, HPHP::Facts::SymKind::Type>>{}(type),
        std::hash<HPHP::Facts::Path<S>>{}(path));
  }
};
