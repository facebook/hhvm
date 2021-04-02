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

#include <folly/experimental/io/FsUtil.h>
#include <folly/hash/Hash.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/symbol-types.h"

namespace HPHP {
namespace Facts {

/**
 * Represents an edge from a type to its supertype.
 *
 * If you wrote `class DerivedClass extends BaseClass {}` in `foo.hck`, we'd
 * denote it as
 *
 *   EdgeToSupertype{.m_type=BaseClass,
 *                   .m_kind=DeriveKind::Extends,
 *                   .m_path="foo.hck"}
 */
template <typename S> struct EdgeToSupertype {

  bool operator==(const EdgeToSupertype& o) const noexcept {
    return m_type == o.m_type && m_kind == o.m_kind && m_path == o.m_path;
  }

  Symbol<S, SymKind::Type> m_type;
  DeriveKind m_kind;
  Path<S> m_path;
};

/**
 * Represents an edge from a type to one its subtypes, using the given
 * kind of inheritance..
 */
template <typename S> struct SubtypeQuery {

  bool operator==(const SubtypeQuery& o) const noexcept {
    return m_type == o.m_type && m_kind == o.m_kind;
  }

  Symbol<S, SymKind::Type> m_type;
  DeriveKind m_kind;
};

/**
 * Information about which types use, extend, or implement which other types.
 */
template <typename S> struct InheritanceInfo {

  using TypeToBaseTypesMap = LazyTwoWayMap<EdgeToSupertype<S>, SubtypeQuery<S>>;

  using TypeDefSet = typename TypeToBaseTypesMap::KeysSet;
  using TypeSet = typename TypeToBaseTypesMap::ValuesSet;

  /**
   * Return inheritance data about the given type.
   *
   * The const overloads will return `nullptr` if we don't yet have all the
   * data we need to definitively answer this query. If the caller receives
   * `nullptr`, they will need to call a non-const overload.
   */

  const TypeSet* getBaseTypes(
      Symbol<S, SymKind::Type> derivedType,
      Path<S> derivedTypePath,
      DeriveKind kind) const {
    return m_baseTypesMap.getValuesForKey(
        {.m_type = derivedType, .m_kind = kind, .m_path = derivedTypePath});
  }

  const TypeSet& getBaseTypes(
      Symbol<S, SymKind::Type> derivedType,
      Path<S> derivedTypePath,
      DeriveKind kind,
      std::vector<SubtypeQuery<S>> edgesFromDB) {
    return m_baseTypesMap.getValuesForKey(
        EdgeToSupertype<S>{
            .m_type = derivedType, .m_kind = kind, .m_path = derivedTypePath},
        std::move(edgesFromDB));
  }

  const TypeDefSet*
  getDerivedTypes(Symbol<S, SymKind::Type> baseType, DeriveKind kind) const {
    return m_baseTypesMap.getKeysForValue({.m_type = baseType, .m_kind = kind});
  }

  const TypeDefSet& getDerivedTypes(
      Symbol<S, SymKind::Type> baseType,
      DeriveKind kind,
      std::vector<EdgeToSupertype<S>> edgesFromDB) {
    return m_baseTypesMap.getKeysForValue(
        SubtypeQuery<S>{.m_type = baseType, .m_kind = kind},
        std::move(edgesFromDB));
  }

  /**
   * Remove all known information about the given type defined at the given
   * path.
   */
  void
  removeType(Symbol<S, SymKind::Type> derivedType, Path<S> derivedTypePath) {
    for (auto kind :
         {DeriveKind::Extends,
          DeriveKind::RequireExtends,
          DeriveKind::RequireImplements}) {
      m_baseTypesMap.setValuesForKey(
          {.m_type = derivedType, .m_kind = kind, .m_path = derivedTypePath},
          {});
    }
  }

  /**
   * Mark `derivedType` as inheriting from each of the `baseTypes`.
   */
  void setBaseTypes(
      Symbol<S, SymKind::Type> derivedType,
      Path<S> derivedTypePath,
      DeriveKind kind,
      const std::vector<std::string>& baseTypeStrs) {
    TypeSet baseTypes;
    baseTypes.reserve(baseTypeStrs.size());
    for (auto const& baseTypeStr : baseTypeStrs) {
      baseTypes.insert(
          {.m_type = Symbol<S, SymKind::Type>{baseTypeStr}, .m_kind = kind});
    }
    return m_baseTypesMap.setValuesForKey(
        {.m_type = derivedType, .m_kind = kind, .m_path = derivedTypePath},
        std::move(baseTypes));
  }

private:
  TypeToBaseTypesMap m_baseTypesMap;
};

} // namespace Facts
} // namespace HPHP

template <typename S> struct std::hash<HPHP::Facts::EdgeToSupertype<S>> {
  size_t operator()(const HPHP::Facts::EdgeToSupertype<S>& typeDef) const {
    return folly::hash::hash_combine(
        std::hash<HPHP::Facts::Path<S>>{}(typeDef.m_path),
        std::hash<HPHP::Facts::Symbol<S, HPHP::Facts::SymKind::Type>>{}(
            typeDef.m_type),
        std::hash<int>{}(static_cast<int>(typeDef.m_kind)));
  }
};

template <typename S> struct std::hash<HPHP::Facts::SubtypeQuery<S>> {
  size_t operator()(const HPHP::Facts::SubtypeQuery<S>& query) const {
    return folly::hash::hash_combine(
        std::hash<HPHP::Facts::Symbol<S, HPHP::Facts::SymKind::Type>>{}(
            query.m_type),
        std::hash<int>{}(static_cast<int>(query.m_kind)));
  }
};
