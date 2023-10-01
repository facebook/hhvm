/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#pragma once

namespace HPHP {

//////////////////////////////////////////////////////////////////////

enum class TypeConstraintFlags : uint16_t {
  NoFlags = 0x0,

  /*
   * Nullable type hints check they are either the specified type,
   * or null.
   */
  Nullable = 0x1,

  /*
   * Indicates a union
   */
  Union = 0x2,

  /*
   * Extended hints are hints that do not apply to normal, vanilla
   * php.  For example "?Foo".
   */
  ExtendedHint = 0x4,

  /*
   * Indicates that a type constraint is a type variable. For example,
   * the constraint on $x is a TypeVar.
   * class Foo<T> {
   *   public function bar(T $x) { ... }
   * }
   */
  TypeVar = 0x8,

  /*
   * Soft type hints: triggers warning, but never fatals
   * E.g. "@int"
   */
  Soft = 0x10,

  /*
   * Indicates a type constraint is a type constant, which is similar to a
   * type alias defined inside a class. For instance, the constraint on $x
   * is a TypeConstant:
   *
   * class Foo {
   *   const type T = int;
   *   public function bar(Foo::T $x) { ... }
   * }
   */
  TypeConstant = 0x20,

  /*
   * Indicates that a Object type-constraint was resolved by hhbbc,
   * and the actual type is in m_type. When set, Object is guaranteed
   * to be an object, not a type-alias.
   */
  Resolved = 0x40,

  /*
   * Indicates that a type-constraint should be displayed as nullable (even if
   * isNullable()) is false. This is used to maintain proper display of
   * type-constraints even when resolved.
   */
  DisplayNullable = 0x100,

  /*
   * Indicates that a type-constraint came from an upper-bound constraint.
   */
  UpperBound = 0x200,
};

constexpr TypeConstraintFlags
operator|(TypeConstraintFlags a, TypeConstraintFlags b) {
  return TypeConstraintFlags(static_cast<int>(a) | static_cast<int>(b));
}

constexpr TypeConstraintFlags
operator&(TypeConstraintFlags a, TypeConstraintFlags b) {
  return TypeConstraintFlags(static_cast<int>(a) & static_cast<int>(b));
}

constexpr TypeConstraintFlags
operator~(TypeConstraintFlags a) {
  return TypeConstraintFlags(~static_cast<int>(a));
}

constexpr void operator|=(TypeConstraintFlags& a, TypeConstraintFlags b) {
  a = a | b;
}

constexpr void operator&=(TypeConstraintFlags& a, TypeConstraintFlags b) {
  a = a & b;
}

constexpr bool operator!(TypeConstraintFlags a) {
  return a == TypeConstraintFlags::NoFlags;
}

constexpr bool contains(TypeConstraintFlags a, TypeConstraintFlags b) {
  return (a & b) != TypeConstraintFlags::NoFlags;
}

}
