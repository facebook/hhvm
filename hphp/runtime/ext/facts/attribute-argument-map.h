// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <folly/dynamic.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/hash/Hash.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/util/hash-map.h"

namespace HPHP {
namespace Facts {

template <typename S> struct AttrArgKey {
  Symbol<S, SymKind::Type> m_type;
  Path<S> m_path;
  Symbol<S, SymKind::Type> m_attr;

  bool operator==(const AttrArgKey<S>& o) const {
    return m_type == o.m_type && m_path == o.m_path && m_attr == o.m_attr;
  }
};

template <typename S> struct AttributeArgumentMap {

  void setAttributeArgs(
      Symbol<S, SymKind::Type> type,
      Path<S> path,
      Symbol<S, SymKind::Type> attr,
      std::vector<folly::dynamic> args) {
    m_attrArgs.insert_or_assign(
        {.m_type = type, .m_path = path, .m_attr = attr}, std::move(args));
  }

  const std::vector<folly::dynamic>* getAttributeArgs(
      Symbol<S, SymKind::Type> type,
      Path<S> path,
      Symbol<S, SymKind::Type> attr) const {
    AttrArgKey<S> key{.m_type = type, .m_path = path, .m_attr = attr};

    auto const it = m_attrArgs.find(key);
    if (it != m_attrArgs.end()) {
      return &it->second;
    }
    return nullptr;
  }

  const std::vector<folly::dynamic>& getAttributeArgs(
      Symbol<S, SymKind::Type> type,
      Path<S> path,
      Symbol<S, SymKind::Type> attr,
      const std::vector<folly::dynamic>& argsFromDB) {
    AttrArgKey<S> key{.m_type = type, .m_path = path, .m_attr = attr};

    auto it = m_attrArgs.find(key);
    if (it != m_attrArgs.end()) {
      return it->second;
    }

    return m_attrArgs.insert({std::move(key), argsFromDB}).first->second;
  }

private:
  hphp_hash_map<AttrArgKey<S>, std::vector<folly::dynamic>> m_attrArgs;
};

} // namespace Facts
} // namespace HPHP

template <typename S> struct std::hash<HPHP::Facts::AttrArgKey<S>> {
  size_t operator()(const HPHP::Facts::AttrArgKey<S>& a) const {
    using HPHP::Facts::Path, HPHP::Facts::Symbol, HPHP::Facts::SymKind;
    return folly::hash::hash_combine(
        std::hash<Symbol<S, SymKind::Type>>{}(a.m_type),
        std::hash<Path<S>>{}(a.m_path),
        std::hash<Symbol<S, SymKind::Type>>{}(a.m_attr));
  }
};
