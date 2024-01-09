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

#include <filesystem>
#include <string>
#include <vector>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/string-ptr.h"

namespace HPHP {
namespace Facts {

enum class SymKind {
  Type,
  Function,
  Constant,
  Module,
};

constexpr std::string_view toString(SymKind k) {
  switch (k) {
    case SymKind::Type:
      return "type";
    case SymKind::Function:
      return "function";
    case SymKind::Constant:
      return "constant";
    case SymKind::Module:
      return "module";
  }
  return "unknown";
}

constexpr bool isCaseSensitive(SymKind k) {
  switch (k) {
    case SymKind::Type:
    case SymKind::Function:
      return false;
    case SymKind::Constant:
    case SymKind::Module:
      return true;
  }
}

/**
 * Type-safe wrapper around StringPtr to represent the path to a file,
 * relative to the repo's root.
 */
struct Path {
  explicit Path(std::nullptr_t) : m_path{nullptr} {}
  explicit Path(StringPtr path) : m_path{path} {
    assertx(!m_path.empty());
  }
  explicit Path(const StringData& path) : Path{makeStringPtr(path)} {}
  explicit Path(const std::filesystem::path& path)
      : Path{makeStringPtr(path.native())} {
    assertx(path.is_relative());
  }
  explicit Path(const std::string_view path) : Path{makeStringPtr(path)} {}

  bool operator==(const Path& o) const noexcept {
    return m_path.same(o.m_path);
  }

  bool operator==(std::nullptr_t) const noexcept {
    return m_path == nullptr;
  }

  bool operator==(const std::string_view s) const noexcept {
    return m_path == s;
  }

  template <typename T>
  bool operator!=(const T& o) const noexcept {
    return !(operator==(o));
  }

  std::string_view slice() const noexcept {
    return m_path.slice();
  }

  std::filesystem::path native() const noexcept {
    return std::filesystem::path{std::string{slice()}};
  }

  const StringData* get() const noexcept {
    return m_path.get();
  }

  StringPtr m_path;
};

/**
 * Type-safe wrapper around a StringPtr to represent the name of a Type,
 * Function, Constant, or Type Alias in the repo.
 *
 * This may compare case
 */
template <SymKind k>
struct Symbol {
  explicit Symbol(StringPtr name) : m_name{name} {}
  explicit Symbol(const StringData& name) : Symbol{makeStringPtr(name)} {}
  explicit Symbol(const std::string_view name) : Symbol{makeStringPtr(name)} {}

  /**
   * This operation is case-insensitive for Types, Functions, and Type Aliases,
   * but not Constants. This mirrors the case-insensitivity of PHP's runtime and
   * autoloader.
   */
  bool operator==(const Symbol<k>& o) const noexcept {
    switch (k) {
      case SymKind::Type:
        return m_name.tsame(o.m_name);
      case SymKind::Function:
        return m_name.fsame(o.m_name);
      case SymKind::Constant:
      case SymKind::Module:
        return m_name.same(o.m_name);
    }
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

  const StringData* get() const noexcept {
    return m_name.get();
  }

  static std::vector<Symbol<k>> from(const std::vector<std::string>& strs) {
    std::vector<Symbol<k>> syms;
    syms.reserve(strs.size());
    for (auto const& s : strs) {
      syms.emplace_back(s);
    }
    return syms;
  }

  StringPtr m_name;
};

struct TypeDecl {
  Symbol<SymKind::Type> m_name;
  Path m_path;

  bool operator==(const TypeDecl& o) const {
    return m_name == o.m_name && m_path == o.m_path;
  }
};

struct MethodDecl {
  TypeDecl m_type;
  Symbol<SymKind::Function> m_method;

  bool operator==(const MethodDecl& o) const {
    return m_type == o.m_type && m_method == o.m_method;
  }
};

} // namespace Facts
} // namespace HPHP

template <>
struct std::hash<HPHP::Facts::Path> {
  std::size_t operator()(const HPHP::Facts::Path& p) const noexcept {
    return p.m_path.hash();
  }
};

template <HPHP::Facts::SymKind k>
struct std::hash<HPHP::Facts::Symbol<k>> {
  std::size_t operator()(const HPHP::Facts::Symbol<k>& s) const noexcept {
    // This is a case-insensitive hash, suitable for use whether the equality
    // comparator is case-sensitive or case-insensitive.
    return s.m_name.hash();
  }
};

namespace std {

template <>
struct hash<typename HPHP::Facts::TypeDecl> {
  size_t operator()(const typename HPHP::Facts::TypeDecl& d) const {
    return folly::hash::hash_combine(
        std::hash<HPHP::Facts::Symbol<HPHP::Facts::SymKind::Type>>{}(d.m_name),
        std::hash<HPHP::Facts::Path>{}(d.m_path));
  }
};

template <>
struct hash<typename HPHP::Facts::MethodDecl> {
  size_t operator()(const typename HPHP::Facts::MethodDecl& d) const {
    return folly::hash::hash_combine(
        std::hash<HPHP::Facts::TypeDecl>{}(d.m_type),
        std::hash<HPHP::Facts::Symbol<HPHP::Facts::SymKind::Function>>{}(
            d.m_method));
  }
};

} // namespace std
