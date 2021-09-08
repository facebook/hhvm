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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/array-layout.h"

#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Array type specialization.
 *
 * This type lattice is a simple cross product:
 *
 *   ArrayLayout x (Maybe RepoAuthoritativeType)
 *
 * HHBBC doesn't know about vanilla or bespoke layouts, so when HHBBC provides
 * us an RAT for an array, we include it in the specialization but we set the
 * layout to ArrayLayout::Top().
 *
 * In the JIT, if we create an array of a known layout (e.g. with AllocVArray)
 * or we test the layout (e.g. with CheckType), we'll get the layout info.
 */
struct ArraySpec {
  /*
   * Constructors.
   */
  constexpr ArraySpec();
  explicit ArraySpec(ArrayLayout layout);
  explicit ArraySpec(const RepoAuthType::Array* type);
  ArraySpec(ArrayLayout layout, const RepoAuthType::Array* type);

  /*
   * Update the layout of the ArraySpec with the given info, if it is possible
   * to do so without a contradiction - otherwise, return Bottom.
   */
  ArraySpec narrowToLayout(ArrayLayout layout) const;

  /*
   * Only for post-deserialization fixup.
   *
   * As a precondition, the ArraySpec must already have a RAT type.
   */
  void setType(const RepoAuthType::Array* adjusted);

  /*
   * Human-readable debug string.
   */
  std::string toString() const;

  /*
   * Accessors.
   *
   * bits() returns the raw bits for this ArraySpec.
   * type() returns nullptr if no RAT is set.
   */
  uintptr_t bits() const;
  ArrayLayout layout() const;
  const RepoAuthType::Array* type() const;

  /*
   * Expose various ArrayLayout predicates.
   */
  bool vanilla() const  { return layout().vanilla(); }
  bool bespoke() const  { return layout().bespoke(); }
  bool logging() const  { return layout().logging(); }
  bool monotype() const { return layout().monotype(); }
  bool is_struct() const { return layout().is_struct(); }
  bool is_concrete() const { return layout().is_concrete(); }

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
  static constexpr ArraySpec Top();
  static constexpr ArraySpec Bottom();

private:
  /*
   * Bottom constructor.
   */
  enum class BottomTag {};
  explicit constexpr ArraySpec(BottomTag);

  /*
   * Checked by the ArraySpec constructors, to ensure that we don't create
   * invalid array specializations like "Dict=DictKind".
   */
  bool checkInvariants() const;

  /*
   * Data members.
   */
  union {
    struct {
      uint16_t m_layout;    // an ArrayLayout's toUint16 data
      uintptr_t m_type: 48; // const RepoAuthType::Array*, or nullptr
    };
    uintptr_t m_bits;
  };
};

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
  ClassSpec(const Class*, SubTag);
  ClassSpec(const Class*, ExactTag);

  /*
   * Human-readable debug string.
   */
  std::string toString() const;

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
  static constexpr ClassSpec Top();
  static constexpr ClassSpec Bottom();

private:
  /*
   * Bottom constructor.
   */
  enum class BottomTag {};
  explicit constexpr ClassSpec(BottomTag);

  /*
   * Sort tag.
   */
  enum SortOf : uint8_t {
    IsTop,
    IsBottom,
    IsSub,
    IsExact,
  };

  /*
   * Data members.
   */
  union {
    struct {
      uintptr_t m_sort : 8;
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
  Class = 1 << 1
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
