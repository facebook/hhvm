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
#include <vector>

#include <folly/experimental/io/FsUtil.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/string-ptr.h"

namespace HPHP {
namespace Facts {

enum class SymKind {
  Type,
  Function,
  Constant,
};

constexpr std::string_view toString(SymKind k) {
  switch (k) {
    case SymKind::Type:
      return "type";
    case SymKind::Function:
      return "function";
    case SymKind::Constant:
      return "constant";
  }
  return "unknown";
}

constexpr bool isCaseSensitive(SymKind k) {
  switch (k) {
    case SymKind::Type:
    case SymKind::Function:
      return false;
    case SymKind::Constant:
      return true;
  }
}

/**
 * Type-safe wrapper around StringPtr<S> to represent the path to a file,
 * relative to the repo's root.
 */
template <typename S> struct Path {

  explicit Path(std::nullptr_t) : m_path{nullptr} {
  }
  explicit Path(StringPtr<S> path) : m_path{path} {
    assertx(!m_path.empty());
  }
  explicit Path(const S& path) : Path{makeStringPtr<S>(path)} {
  }
  explicit Path(const folly::fs::path& path)
      : Path{makeStringPtr<S>(path.native())} {
    assertx(path.is_relative());
  }
  explicit Path(const std::string_view path) : Path{makeStringPtr<S>(path)} {
  }

  bool operator==(const Path<S>& o) const noexcept {
    return m_path.same(o.m_path);
  }

  bool operator==(std::nullptr_t) const noexcept {
    return m_path == nullptr;
  }

  bool operator==(const std::string_view s) const noexcept {
    return m_path == s;
  }

  template <typename T> bool operator!=(const T& o) const noexcept {
    return !(operator==(o));
  }

  std::string_view slice() const noexcept {
    return m_path.slice();
  }

  folly::fs::path native() const noexcept {
    return folly::fs::path{std::string{slice()}};
  }

  const S* get() const noexcept {
    return m_path.get();
  }

  StringPtr<S> m_path;
};

/**
 * Type-safe wrapper around a StringPtr<S> to represent the name of a Type,
 * Function, Constant, or Type Alias in the repo.
 *
 * This may compare case
 */
template <typename S, SymKind k> struct Symbol {

  explicit Symbol(StringPtr<S> name) : m_name{name} {
  }
  explicit Symbol(const S& name) : Symbol{makeStringPtr<S>(name)} {
  }
  explicit Symbol(const std::string_view name)
      : Symbol{makeStringPtr<S>(name)} {
  }

  /**
   * This operation is case-insensitive for Types, Functions, and Type Aliases,
   * but not Constants. This mirrors the case-insensitivity of PHP's runtime and
   * autoloader.
   */
  bool operator==(const Symbol<S, k>& o) const noexcept {
    return isCaseSensitive(k) ? m_name.same(o.m_name) : m_name.isame(o.m_name);
  }

  /**
   * Explicitly call slice() if you want to perform a case-sensitive or
   * case-insensitive comparison to an ordinary string instead of another
   * Symbol.
   */
  bool operator==(const std::string_view s) const noexcept = delete;

  bool operator==(std::nullptr_t) const noexcept {
    return m_name == nullptr;
  }

  std::string_view slice() const noexcept {
    return m_name.slice();
  }

  const S* get() const noexcept {
    return m_name.get();
  }

  static std::vector<Symbol<S, k>> from(const std::vector<std::string>& strs) {
    std::vector<Symbol<S, k>> syms;
    syms.reserve(strs.size());
    for (auto const& s : strs) {
      syms.emplace_back(s);
    }
    return syms;
  }

  StringPtr<S> m_name;
};

template <typename S> struct TypeDecl {
  Symbol<S, SymKind::Type> m_name;
  Path<S> m_path;

  bool operator==(const TypeDecl<S>& o) const {
    return m_name == o.m_name && m_path == o.m_path;
  }

  std::vector<Symbol<S, SymKind::Type>>
  getAttributesFromDB(AutoloadDB& db, SQLiteTxn& txn) const;
};

template <typename S> struct MethodDecl {
  TypeDecl<S> m_type;
  Symbol<S, SymKind::Function> m_method;

  bool operator==(const MethodDecl<S>& o) const {
    return m_type == o.m_type && m_method == o.m_method;
  }

  std::vector<Symbol<S, SymKind::Type>>
  getAttributesFromDB(AutoloadDB& db, SQLiteTxn& txn) const;
};

} // namespace Facts
} // namespace HPHP

template <typename S> struct std::hash<HPHP::Facts::Path<S>> {
  std::size_t operator()(const HPHP::Facts::Path<S>& p) const noexcept {
    return p.m_path.hash();
  }
};

template <typename S, HPHP::Facts::SymKind k>
struct std::hash<HPHP::Facts::Symbol<S, k>> {
  std::size_t operator()(const HPHP::Facts::Symbol<S, k>& s) const noexcept {
    // This is a case-insensitive hash, suitable for use whether the equality
    // comparator is case-sensitive or case-insensitive.
    return s.m_name.hash();
  }
};
