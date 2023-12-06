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

#include <folly/hash/Hash.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/path-versions.h"
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
struct EdgeToSupertype {
  bool operator==(const EdgeToSupertype& o) const noexcept {
    return m_type == o.m_type && m_kind == o.m_kind && m_path == o.m_path;
  }

  Symbol<SymKind::Type> m_type;
  DeriveKind m_kind;
  Path m_path;
};

} // namespace Facts
} // namespace HPHP

template <>
struct std::hash<HPHP::Facts::EdgeToSupertype> {
  size_t operator()(const HPHP::Facts::EdgeToSupertype& typeDef) const {
    return folly::hash::hash_combine(
        std::hash<HPHP::Facts::Path>{}(typeDef.m_path),
        std::hash<HPHP::Facts::Symbol<HPHP::Facts::SymKind::Type>>{}(
            typeDef.m_type),
        std::hash<int>{}(static_cast<int>(typeDef.m_kind)));
  }
};

namespace HPHP {
namespace Facts {

/**
 * Represents an edge from a type to one its subtypes, using the given
 * kind of inheritance..
 */
struct SubtypeQuery {
  bool operator==(const SubtypeQuery& o) const noexcept {
    return m_type == o.m_type && m_kind == o.m_kind;
  }

  Symbol<SymKind::Type> m_type;
  DeriveKind m_kind;
};

} // namespace Facts
} // namespace HPHP

template <>
struct std::hash<HPHP::Facts::SubtypeQuery> {
  size_t operator()(const HPHP::Facts::SubtypeQuery& query) const {
    return folly::hash::hash_combine(
        std::hash<HPHP::Facts::Symbol<HPHP::Facts::SymKind::Type>>{}(
            query.m_type),
        std::hash<int>{}(static_cast<int>(query.m_kind)));
  }
};

namespace HPHP {
namespace Facts {

/**
 * Information about which types use, extend, or implement which other types.
 */
struct InheritanceInfo {
  using TypeToBaseTypesMap = LazyTwoWayMap<EdgeToSupertype, SubtypeQuery>;

  using TypeDefs = typename TypeToBaseTypesMap::Keys;
  using Types = typename TypeToBaseTypesMap::Values;

  explicit InheritanceInfo(std::shared_ptr<PathVersions> versions)
      : m_baseTypesMap{std::move(versions)} {}

  /**
   * Return inheritance data about the given type.
   *
   * The const overloads will return `std::nullopt` if we don't yet have all the
   * data we need to definitively answer this query. If the caller receives
   * `std::nullopt`, they will need to call a non-const overload.
   */

  Optional<Types> getBaseTypes(
      Symbol<SymKind::Type> derivedType,
      Path derivedTypePath,
      DeriveKind kind) const {
    return m_baseTypesMap.getValuesForKey(
        {.m_type = derivedType, .m_kind = kind, .m_path = derivedTypePath});
  }

  Types getBaseTypes(
      Symbol<SymKind::Type> derivedType,
      Path derivedTypePath,
      DeriveKind kind,
      std::vector<SubtypeQuery> edgesFromDB) {
    return m_baseTypesMap.getValuesForKey(
        EdgeToSupertype{
            .m_type = derivedType, .m_kind = kind, .m_path = derivedTypePath},
        std::move(edgesFromDB));
  }

  Optional<TypeDefs> getDerivedTypes(
      Symbol<SymKind::Type> baseType,
      DeriveKind kind) const {
    return m_baseTypesMap.getKeysForValue({.m_type = baseType, .m_kind = kind});
  }

  TypeDefs getDerivedTypes(
      Symbol<SymKind::Type> baseType,
      DeriveKind kind,
      std::vector<EdgeToSupertype> edgesFromDB) {
    return m_baseTypesMap.getKeysForValue(
        SubtypeQuery{.m_type = baseType, .m_kind = kind},
        std::move(edgesFromDB));
  }

  /**
   * Mark `derivedType` as inheriting from each of the `baseTypes`.
   */
  void setBaseTypes(
      Symbol<SymKind::Type> derivedType,
      Path derivedTypePath,
      DeriveKind kind,
      const rust::Vec<rust::String>& baseTypeStrs) {
    Types baseTypes;
    baseTypes.reserve(baseTypeStrs.size());
    for (auto const& baseTypeStr : baseTypeStrs) {
      baseTypes.push_back(
          {.m_type = Symbol<SymKind::Type>{as_slice(baseTypeStr)},
           .m_kind = kind});
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
