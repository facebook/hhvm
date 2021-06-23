// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <string>
#include <utility>
#include <vector>

#include <folly/dynamic.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/hash/Hash.h>

#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/util/hash-map.h"

namespace HPHP {
namespace Facts {

template <typename S, typename Key> struct AttributeArgumentMap {

  using ArgKey = std::tuple<Key, Symbol<S, SymKind::Type>>;

  void setAttributeArgs(
      Key key,
      Symbol<S, SymKind::Type> attr,
      std::vector<folly::dynamic> args) {
    m_attrArgs.insert_or_assign({key, attr}, std::move(args));
  }

  const std::vector<folly::dynamic>*
  getAttributeArgs(Key key, Symbol<S, SymKind::Type> attr) const {
    ArgKey argKey{key, attr};

    auto const it = m_attrArgs.find(argKey);
    if (it != m_attrArgs.end()) {
      return &it->second;
    }
    return nullptr;
  }

  const std::vector<folly::dynamic>& getAttributeArgs(
      Key key,
      Symbol<S, SymKind::Type> attr,
      const std::vector<folly::dynamic>& argsFromDB) {
    ArgKey argKey{key, attr};

    auto it = m_attrArgs.find(argKey);
    if (it != m_attrArgs.end()) {
      return it->second;
    }

    return m_attrArgs.insert({std::move(argKey), argsFromDB}).first->second;
  }

private:
  hphp_hash_map<ArgKey, std::vector<folly::dynamic>> m_attrArgs;
};

} // namespace Facts
} // namespace HPHP
