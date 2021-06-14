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

#include <folly/dynamic.h>

namespace HPHP {
namespace Facts {

using TypeKindMask = int;
enum class TypeKind {
  Unknown = 0,
  Class = 1 << 0,
  Interface = 1 << 1,
  Enum = 1 << 2,
  Trait = 1 << 3,
  TypeAlias = 1 << 4,
};
inline constexpr TypeKindMask kTypeKindAll =
    static_cast<int>(TypeKind::Class) | static_cast<int>(TypeKind::Interface) |
    static_cast<int>(TypeKind::Enum) | static_cast<int>(TypeKind::Trait) |
    static_cast<int>(TypeKind::TypeAlias);

inline constexpr std::string_view kTypeKindClass = "class";
inline constexpr std::string_view kTypeKindInterface = "interface";
inline constexpr std::string_view kTypeKindEnum = "enum";
inline constexpr std::string_view kTypeKindTrait = "trait";
inline constexpr std::string_view kTypeKindTypeAlias = "typeAlias";
inline constexpr std::string_view kTypeKindMixed = "mixed";
inline constexpr std::string_view kTypeKindUnknown = "unknown";

constexpr std::string_view toString(TypeKind kind) {
  switch (kind) {
    case TypeKind::Class:
      return kTypeKindClass;
    case TypeKind::Enum:
      return kTypeKindEnum;
    case TypeKind::Interface:
      return kTypeKindInterface;
    case TypeKind::Trait:
      return kTypeKindTrait;
    case TypeKind::TypeAlias:
      return kTypeKindTypeAlias;
    default:
      return "";
  }
}

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
using DeriveKindMask = int;
enum class DeriveKind {
  Extends = 1 << 0,
  RequireExtends = 1 << 1,
  // This should be merged into RequireExtends.
  RequireImplements = 1 << 2,
};
constexpr DeriveKindMask kDeriveKindAll =
    static_cast<int>(DeriveKind::Extends) |
    static_cast<int>(DeriveKind::RequireExtends) |
    static_cast<int>(DeriveKind::RequireImplements);

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

struct FileFacts {

  bool isEmpty() const noexcept {
    return m_types.empty() && m_functions.empty() && m_constants.empty();
  }

  std::vector<TypeDetails> m_types;
  std::vector<std::string> m_functions;
  std::vector<std::string> m_constants;
  std::vector<Attribute> m_attributes;
  std::string m_sha1hex;
};

} // namespace Facts
} // namespace HPHP
