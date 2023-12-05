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

#include <functional>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <folly/Format.h>
#include <folly/dynamic.h>
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"

namespace HPHP {
namespace Facts {

using TypeKindMask = int;
using hackc::TypeKind;
inline constexpr TypeKindMask kTypeKindAll = static_cast<int>(TypeKind::Class) |
    static_cast<int>(TypeKind::Interface) | static_cast<int>(TypeKind::Enum) |
    static_cast<int>(TypeKind::Trait) | static_cast<int>(TypeKind::TypeAlias);

constexpr std::string_view kTypeKindClass = "class";
constexpr std::string_view kTypeKindInterface = "interface";
constexpr std::string_view kTypeKindEnum = "enum";
constexpr std::string_view kTypeKindTrait = "trait";
constexpr std::string_view kTypeKindTypeAlias = "typeAlias";

using TypeFlagMask = int;
enum class TypeFlag {
  Empty = 0,
  Abstract = 1 << 0,
  Final = 1 << 1,
};

// Two different forms of inheritance.
//
// `trait T { require extends B; }` means that, while T does not extend B
// itself, any class using T must also extend B in order to be sound.
// `trait T { require class B; }` means that trait T can only be used
// by class T
using DeriveKindMask = int;
enum class DeriveKind {
  Extends = 1 << 0,
  RequireExtends = 1 << 1,
  // This should be merged into RequireExtends.
  RequireImplements = 1 << 2,
  RequireClass = 1 << 3,
};
constexpr DeriveKindMask kDeriveKindAll =
    static_cast<int>(DeriveKind::Extends) |
    static_cast<int>(DeriveKind::RequireExtends) |
    static_cast<int>(DeriveKind::RequireImplements) |
    static_cast<int>(DeriveKind::RequireClass);

// Represents `<<IAmAnAttribute(0, 'Hello', null)>>` as
// `{"IAmAnAttribute", vec[0, "Hello", null]}`
struct Attribute {
  std::string m_name;
  std::vector<folly::dynamic> m_args;
};

struct MethodDetails {
  std::string m_name;
  std::vector<Attribute> m_attributes;
};

struct TypeDetails {
  std::string m_name;
  TypeKind m_kind;
  int m_flags{0};

  // List of types which this `extends`, `implements`, or `use`s
  std::vector<std::string> m_baseTypes;

  // List of attributes and their arguments
  std::vector<Attribute> m_attributes;

  // List of classes which this `require class`
  std::vector<std::string> m_requireClass;

  // List of classes or interfaces which this `require extends`
  std::vector<std::string> m_requireExtends;

  // List of interfaces which this `require implements`
  std::vector<std::string> m_requireImplements;

  std::vector<MethodDetails> m_methods;

  bool isAbstract() const noexcept {
    return m_flags & static_cast<int>(TypeFlag::Abstract);
  }

  bool isFinal() const noexcept {
    return m_flags & static_cast<int>(TypeFlag::Final);
  }
};

struct ModuleDetails {
  std::string m_name;
};

struct FileFacts {
  bool isEmpty() const noexcept {
    return m_types.empty() && m_functions.empty() && m_constants.empty() &&
        m_modules.empty() && m_attributes.empty();
  }

  std::vector<TypeDetails> m_types;
  std::vector<std::string> m_functions;
  std::vector<std::string> m_constants;
  std::vector<ModuleDetails> m_modules;
  std::vector<Attribute> m_attributes;
  std::string m_sha1hex;
};

/**
 * A string from Watchman representing a point in time.
 */
struct Clock {
  /**
   * True iff this represents an initial load, where all files have changed
   * since this point in time
   */
  bool isInitial() const noexcept {
    return m_clock.empty() && m_mergebase.empty();
  }

  bool operator==(const Clock& o) const noexcept {
    return m_clock == o.m_clock && m_mergebase == o.m_mergebase;
  }

  bool operator!=(const Clock& o) const noexcept {
    return !operator==(o);
  }

  /**
   * Represents a timestamp on the local machine.
   */
  std::string m_clock;

  /**
   * Represents a public commit in the repo.
   */
  std::string m_mergebase;
};

} // namespace Facts
} // namespace HPHP

namespace folly {
template <>
class FormatValue<HPHP::Facts::Clock> {
 public:
  explicit FormatValue(HPHP::Facts::Clock v) : m_val(v) {}

  template <typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(
        folly::sformat("\"{}|{}\"", m_val.m_clock, m_val.m_mergebase), arg, cb);
  }

 private:
  const HPHP::Facts::Clock m_val;
};
} // namespace folly

namespace std {

template <>
struct hash<HPHP::Facts::Attribute> {
  std::size_t operator()(const HPHP::Facts::Attribute& attr) const noexcept {
    return folly::hash::hash_combine(
        std::hash<std::string>{}(attr.m_name),
        folly::hash::hash_range(attr.m_args.begin(), attr.m_args.end()));
  }
};

template <>
struct hash<HPHP::Facts::MethodDetails> {
  std::size_t operator()(
      const HPHP::Facts::MethodDetails& method) const noexcept {
    return folly::hash::hash_combine(
        std::hash<std::string>{}(method.m_name),
        folly::hash::hash_range(
            method.m_attributes.begin(), method.m_attributes.end()));
  }
};

template <>
struct hash<HPHP::Facts::TypeDetails> {
  std::size_t operator()(const HPHP::Facts::TypeDetails& type) const noexcept {
    return folly::hash::hash_combine(
        std::hash<std::string>{}(type.m_name),
        std::hash<::HPHP::Facts::TypeKind>{}(type.m_kind),
        std::hash<int>{}(type.m_flags),
        folly::hash::hash_range(
            type.m_baseTypes.begin(), type.m_baseTypes.end()),
        folly::hash::hash_range(
            type.m_attributes.begin(), type.m_attributes.end()),
        folly::hash::hash_range(
            type.m_requireClass.begin(), type.m_requireClass.end()),
        folly::hash::hash_range(
            type.m_requireExtends.begin(), type.m_requireExtends.end()),
        folly::hash::hash_range(
            type.m_requireImplements.begin(), type.m_requireImplements.end()),
        folly::hash::hash_range(type.m_methods.begin(), type.m_methods.end()));
  }
};
} // namespace std

template <>
struct fmt::formatter<HPHP::Facts::Clock> {
  // We don't support any settings within the braces, so nothing to do here.
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const HPHP::Facts::Clock& c, FormatContext& ctx)
      -> decltype(ctx.out()) {
    return format_to(ctx.out(), "Clock({}, {})", c.m_clock, c.m_mergebase);
  }
};
