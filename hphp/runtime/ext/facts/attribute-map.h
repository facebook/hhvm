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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <folly/hash/Hash.h>
#include <folly/json/dynamic.h>

#include "hphp/runtime/ext/facts/attribute-argument-map.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/path-versions.h"
#include "hphp/runtime/ext/facts/symbol-types.h"

namespace HPHP {
namespace Facts {

template <typename Key>
struct AttributeMap {
  using KeyToAttrMap = LazyTwoWayMap<Key, Symbol<SymKind::Type>>;

  using TypeDefs = typename KeyToAttrMap::Keys;
  using Attrs = typename KeyToAttrMap::Values;

  explicit AttributeMap(std::shared_ptr<PathVersions> versions)
      : m_attrMap{std::move(versions)} {}

  /**
   * Returns the attributes present in the map, or `nullptr` if the map needs
   * to be filled from the DB.
   */
  Optional<Attrs> getAttributes(Key key) const {
    return m_attrMap.getValuesForKey(key);
  }

  /**
   * Fill the map with `attrsFromDB` and return a complete set of attributes.
   */
  Attrs getAttributes(Key key, std::vector<Symbol<SymKind::Type>> attrsFromDB) {
    return m_attrMap.getValuesForKey(key, std::move(attrsFromDB));
  }

  /**
   * Returns the keys present in the map, or `nullptr` if the map needs to be
   * filled from the DB.
   */
  Optional<TypeDefs> getKeysWithAttribute(Symbol<SymKind::Type> attr) const {
    return m_attrMap.getKeysForValue(attr);
  }

  /**
   * Fill the map with `keysFromDB` and return a complete set of keys.
   */
  TypeDefs getKeysWithAttribute(
      Symbol<SymKind::Type> attr,
      std::vector<Key> keysFromDB) {
    return m_attrMap.getKeysForValue(attr, std::move(keysFromDB));
  }

  void setAttributes(Key key, rust::Vec<AttrFacts> attrVec) {
    Attrs attrs;
    attrs.reserve(attrVec.size());
    for (auto& attr : attrVec) {
      auto attrSym = Symbol<SymKind::Type>{as_slice(attr.name)};
      attrs.push_back(attrSym);
      std::vector<folly::dynamic> args;
      args.reserve(attr.args.size());
      for (auto& arg : attr.args) {
        args.emplace_back(as_slice(arg)); // string -> folly::dynamic
      }
      m_attrArgs.setAttributeArgs(key, attrSym, std::move(args));
    }
    m_attrMap.setValuesForKey(std::move(key), std::move(attrs));
  }

  const std::vector<folly::dynamic>* getAttributeArgs(
      Key key,
      Symbol<SymKind::Type> attr) const {
    return m_attrArgs.getAttributeArgs(key, attr);
  }

  const std::vector<folly::dynamic>& getAttributeArgs(
      Key key,
      Symbol<SymKind::Type> attr,
      std::vector<folly::dynamic> argsFromDB) {
    return m_attrArgs.getAttributeArgs(key, attr, std::move(argsFromDB));
  }

  KeyToAttrMap m_attrMap;
  AttributeArgumentMap<Key> m_attrArgs;
};

} // namespace Facts
} // namespace HPHP
