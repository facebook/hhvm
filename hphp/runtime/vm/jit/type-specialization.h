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

#ifndef incl_HPHP_JIT_TYPE_SPECIALIZATION_H_
#define incl_HPHP_JIT_TYPE_SPECIALIZATION_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/repo-auth-type.h"

#include <folly/Optional.h>

#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Array type specialization.
 *
 * This type lattice is logically the following cross product:
 *  (Maybe ArrayKind) x (Maybe RepoAuthoritativeType) x LayoutTag
 *
 * HHBBC doesn't know about vanilla or bespoke layouts, so when HHBBC provides
 * us an ArrayKind or an RAT, we include those values in this specialization
 * bit but we have an "unknown" layout tag. This tag makes the kind / type info
 * unusable until we check that the array is vanilla.
 *
 * In the JIT, if we create an array with a known kind (e.g. with AllocVArray)
 * or we test the kind (e.g. with CheckType), we'll also get the vanilla bit.
 */
struct ArraySpec {
  enum class LayoutTag { Unknown, Vanilla };

  /*
   * Constructors.
   */
  constexpr explicit ArraySpec(LayoutTag = LayoutTag::Unknown);
  explicit ArraySpec(ArrayData::ArrayKind kind, LayoutTag = LayoutTag::Vanilla);
  explicit ArraySpec(const RepoAuthType::Array* arrTy);
  ArraySpec(ArrayData::ArrayKind kind, const RepoAuthType::Array* arrTy);

  /*
   * Post-deserialization fixup. We expose getRawType() so that we can adjust
   * the underlying type for non-vanilla ArraySpecs where type() hides it.
   */
  const RepoAuthType::Array* getRawType() const;
  void setRawType(const RepoAuthType::Array* adjusted);

  /*
   * Human-readable debug string.
   */
  std::string toString() const;

  /*
   * Accessors.
   *
   * These return falsey values (folly::none or nullptr) if we can't guarantee
   * that a value of this type has a known ArrayKind or RepoAuthoritativeType.
   *
   * Note that we can return falsey values for, say, kind even w/ HasKind set.
   * We need to know that the array is vanilla before returning a kind or type.
   *
   * bits() returns the raw bits.
   */
  uintptr_t bits() const;
  folly::Optional<ArrayData::ArrayKind> kind() const;
  const RepoAuthType::Array* type() const;
  bool vanilla() const;

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
   * Mask of specializations that a given ArraySpec represents.
   */
  enum SortOf : uint8_t {
    IsTop     = 0,
    IsBottom  = 1 << 0,
    HasKind   = 1 << 1,
    HasType   = 1 << 2,
    IsVanilla = 1 << 3,
  };
  friend SortOf operator|(SortOf, SortOf);
  friend SortOf operator&(SortOf, SortOf);

  /*
   * Data members.
   */
  union {
    struct {
      uintptr_t m_sort : 8;
      uintptr_t m_kind : 8;
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
