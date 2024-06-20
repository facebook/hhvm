// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <string>
#include <utility>
#include <vector>

#include <folly/hash/Hash.h>
#include <folly/json/dynamic.h>

#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/util/hash-map.h"

namespace HPHP {
namespace Facts {

template <typename Key>
struct AttributeArgumentMap {
  using ArgKey = std::tuple<Key, Symbol<SymKind::Type>>;
  using AttributeToArgumentsMap =
      hphp_hash_map<Symbol<SymKind::Type>, std::vector<folly::dynamic>>;
  using KeyToArgMap = hphp_hash_map<Key, AttributeToArgumentsMap>;

  void setAttributeArgs(
      Key key,
      Symbol<SymKind::Type> attr,
      std::vector<folly::dynamic> args) {
    m_attrArgs[key].insert_or_assign(attr, std::move(args));
  }

  const std::vector<folly::dynamic>* getAttributeArgs(
      Key key,
      Symbol<SymKind::Type> attr) const {
    auto const attr_iter = m_attrArgs.find(key);
    if (attr_iter != m_attrArgs.end()) {
      auto const arg_iter = attr_iter->second.find(attr);
      if (arg_iter != attr_iter->second.end()) {
        return &arg_iter->second;
      }
    }
    return nullptr;
  }

  const std::vector<folly::dynamic>& getAttributeArgs(
      Key key,
      Symbol<SymKind::Type> attr,
      const std::vector<folly::dynamic>& argsFromDB) {
    auto const attr_iter = m_attrArgs.find(key);
    if (attr_iter != m_attrArgs.end()) {
      auto const arg_iter = attr_iter->second.find(attr);
      if (arg_iter != attr_iter->second.end()) {
        return arg_iter->second;
      }
    }
    auto [iter, _] = m_attrArgs[key].try_emplace(std::move(attr), argsFromDB);
    return iter->second;
  }

  void erase(Key key) {
    m_attrArgs.erase(key);
  }

 private:
  KeyToArgMap m_attrArgs;
};

} // namespace Facts
} // namespace HPHP
