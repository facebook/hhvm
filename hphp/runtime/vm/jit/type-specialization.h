/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_TYPE_SPECIALIZATION_H_
#define incl_HPHP_JIT_TYPE_SPECIALIZATION_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/repo-auth-type.h"

#include <folly/Optional.h>

#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;
struct Shape;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Array type specialization.
 *
 * May contain an ArrayKind and/or a pointer to a RAT or a Shape.
 */
struct ArraySpec {
  /*
   * Constructors.
   */
  constexpr ArraySpec();
  explicit ArraySpec(ArrayData::ArrayKind kind);
  explicit ArraySpec(const RepoAuthType::Array* arrTy);
  ArraySpec(ArrayData::ArrayKind kind, const RepoAuthType::Array* arrTy);
  explicit ArraySpec(const Shape* shape);

  /*
   * Accessors.
   *
   * These return falsey values (folly::none or nullptr) if the respective bit
   * in `m_sort' is not set.
   *
   * bits() returns the raw bits.
   */
  uintptr_t bits() const;
  folly::Optional<ArrayData::ArrayKind> kind() const;
  const RepoAuthType::Array* type() const;
  const Shape* shape() const;

  /*
   * Casts.
   *
   * Bottom and Top cast to false; everything else casts to true.
   */
  explicit operator bool() const;

  /*
   * Comparisons.
   */
  bool operator==(const ArraySpec& rhs) const;
  bool operator!=(const ArraySpec& rhs) const;
  bool operator<=(const ArraySpec& rhs) const;
  bool operator>=(const ArraySpec& rhs) const;
  bool operator<(const ArraySpec& rhs) const;
  bool operator>(const ArraySpec& rhs) const;

  /*
   * Combinators.
   */
  ArraySpec operator|(const ArraySpec& rhs) const;
  ArraySpec operator&(const ArraySpec& rhs) const;
  ArraySpec operator-(const ArraySpec& rhs) const;

  /*
   * Top and bottom types.
   */
  static const ArraySpec Top;
  static const ArraySpec Bottom;

private:
  /*
   * Bottom constructor.
   */
  enum class BottomTag {};
  explicit ArraySpec(BottomTag);

  /*
   * Mask of specializations that a given ArraySpec represents.
   */
  enum SortOf : uint8_t {
    IsTop     = 0, // top
    IsBottom  = 1 << 0,
    HasKind   = 1 << 1,
    HasType   = 1 << 2,
    HasShape  = 1 << 3,
  };
  friend SortOf operator|(SortOf, SortOf);
  friend SortOf operator&(SortOf, SortOf);

  /*
   * Data members.
   */
  union {
    struct {
      SortOf m_sort : 8;
      ArrayData::ArrayKind m_kind : 8;
      uintptr_t m_ptr : 48;
    };
    uintptr_t m_bits;
  };
};

ArraySpec::SortOf operator|(ArraySpec::SortOf l, ArraySpec::SortOf r);
ArraySpec::SortOf operator&(ArraySpec::SortOf l, ArraySpec::SortOf r);

///////////////////////////////////////////////////////////////////////////////

/*
 * Class type specialization.
 */
struct ClassSpec {
  /*
   * Constructor tags.
   */
  enum class SubTag {};
  enum class ExactTag {};

  /*
   * Constructors.
   */
  constexpr ClassSpec();
  ClassSpec(const Class* cls, SubTag);
  ClassSpec(const Class* cls, ExactTag);

  /*
   * Accessors.
   */
  uintptr_t bits() const;
  bool exact() const;
  const Class* cls() const;
  const Class* exactCls() const;

  /*
   * Casts.
   *
   * Bottom and Top cast to false; everything else casts to true.
   */
  explicit operator bool() const;

  /*
   * Comparisons.
   */
  bool operator==(const ClassSpec& rhs) const;
  bool operator!=(const ClassSpec& rhs) const;
  bool operator<=(const ClassSpec& rhs) const;
  bool operator>=(const ClassSpec& rhs) const;
  bool operator<(const ClassSpec& rhs) const;
  bool operator>(const ClassSpec& rhs) const;

  /*
   * Combinators.
   */
  ClassSpec operator|(const ClassSpec& rhs) const;
  ClassSpec operator&(const ClassSpec& rhs) const;
  ClassSpec operator-(const ClassSpec& rhs) const;

  /*
   * Top and bottom types.
   */
  static const ClassSpec Top;
  static const ClassSpec Bottom;

private:
  /*
   * Bottom constructor.
   */
  enum class BottomTag {};
  explicit ClassSpec(BottomTag);

  /*
   * Sort tag.
   */
  enum SortOf : uint8_t { IsTop, IsBottom, IsSub, IsExact, };

  /*
   * Data members.
   */
  union {
    struct {
      SortOf m_sort : 8;
      uintptr_t m_ptr : 56;
    };
    uintptr_t m_bits;
  };
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Specialization kinds.
 */
enum class SpecKind : uint8_t {
  None = 0,
  Array = 1 << 0,
  Class = 1 << 1,
};

SpecKind operator|(SpecKind l, SpecKind r);
SpecKind operator&(SpecKind l, SpecKind r);
SpecKind& operator|=(SpecKind& l, SpecKind r);

/*
 * Discriminated specialization union.
 *
 * Used to encapsulate combinator logic.
 */
struct TypeSpec {
  /*
   * Constructors.
   */
  TypeSpec();
  TypeSpec(ArraySpec, ClassSpec);

  /*
   * Accessors.
   */
  SpecKind kind() const;
  ArraySpec arrSpec() const;
  ClassSpec clsSpec() const;

  /*
   * Comparisons.
   */
  bool operator==(const TypeSpec& rhs) const;
  bool operator!=(const TypeSpec& rhs) const;
  bool operator<=(const TypeSpec& rhs) const;
  bool operator>=(const TypeSpec& rhs) const;

  /*
   * Combinators.
   */
  TypeSpec operator|(const TypeSpec& rhs) const;
  TypeSpec operator&(const TypeSpec& rhs) const;
  TypeSpec operator-(const TypeSpec& rhs) const;

private:
  /*
   * Data members.
   */
  SpecKind m_kind;
  ArraySpec m_arrSpec;
  ClassSpec m_clsSpec;
};

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/type-specialization-inl.h"

#endif
