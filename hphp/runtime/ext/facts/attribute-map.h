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

#include <string>
#include <utility>
#include <vector>

#include <folly/dynamic.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/hash/Hash.h>

#include "hphp/runtime/ext/facts/attribute-argument-map.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/symbol-types.h"

namespace HPHP {
namespace Facts {

template <typename S, typename Key> struct AttributeMap {
  using KeyToAttrMap = LazyTwoWayMap<Key, Symbol<S, SymKind::Type>>;

  using TypeDefSet = typename KeyToAttrMap::KeysSet;
  using AttrSet = typename KeyToAttrMap::ValuesSet;

  /**
   * Returns the attributes present in the map, or `nullptr` if the map needs
   * to be filled from the DB.
   */
  const AttrSet* getAttributes(Key key) const {
    return m_attrMap.getValuesForKey(key);
  }

  /**
   * Fill the map with `attrsFromDB` and return a complete set of attributes.
   */
  const AttrSet&
  getAttributes(Key key, std::vector<Symbol<S, SymKind::Type>> attrsFromDB) {
    return m_attrMap.getValuesForKey(key, std::move(attrsFromDB));
  }

  /**
   * Returns the keys present in the map, or `nullptr` if the map needs to be
   * filled from the DB.
   */
  const TypeDefSet* getKeysWithAttribute(Symbol<S, SymKind::Type> attr) const {
    return m_attrMap.getKeysForValue(attr);
  }

  /**
   * Fill the map with `keysFromDB` and return a complete set of keys.
   */
  const TypeDefSet& getKeysWithAttribute(
      Symbol<S, SymKind::Type> attr, std::vector<Key> keysFromDB) {
    return m_attrMap.getKeysForValue(attr, std::move(keysFromDB));
  }

  void setAttributes(Key key, std::vector<Attribute> attrVec) {
    AttrSet attrs;
    attrs.reserve(attrVec.size());
    for (auto& attr : attrVec) {
      auto attrSym = Symbol<S, SymKind::Type>{attr.m_name};
      attrs.emplace(attrSym);
      m_attrArgs.setAttributeArgs(key, attrSym, std::move(attr.m_args));
    }
    m_attrMap.setValuesForKey(std::move(key), std::move(attrs));
  }

  const std::vector<folly::dynamic>*
  getAttributeArgs(Key key, Symbol<S, SymKind::Type> attr) const {
    return m_attrArgs.getAttributeArgs(key, attr);
  }

  const std::vector<folly::dynamic>& getAttributeArgs(
      Key key,
      Symbol<S, SymKind::Type> attr,
      std::vector<folly::dynamic> argsFromDB) {
    return m_attrArgs.getAttributeArgs(key, attr, std::move(argsFromDB));
  }

  void removeKey(Key key, std::vector<std::string> attrsFromDB) {
    m_attrMap.setValuesForKey(key, {});
    auto attrs =
        getAttributes(key, Symbol<S, SymKind::Type>::from(attrsFromDB));
    for (auto attr : attrs) {
      m_attrArgs.setAttributeArgs(key, attr, {});
    }
  }

  KeyToAttrMap m_attrMap;
  AttributeArgumentMap<S, Key> m_attrArgs;
};

} // namespace Facts
} // namespace HPHP
