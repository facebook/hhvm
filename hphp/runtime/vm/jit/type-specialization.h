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
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/base/repo-auth-type.h"

#include <folly/Optional.h>

#include <cstdint>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;
struct RecordDesc;

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
   * Constructors. The default constructor produces a "Top" specialization
   * without the vanilla bit set, but all other constructors have it set.
   */
  constexpr explicit ArraySpec(LayoutTag = LayoutTag::Unknown);
  explicit ArraySpec(ArrayData::ArrayKind kind);
  explicit ArraySpec(const RepoAuthType::Array* arrTy);
  ArraySpec(ArrayData::ArrayKind kind, const RepoAuthType::Array* arrTy);

  /*
   * Set or unset the vanilla and dvarray bits on an ArraySpec.
   */
  ArraySpec narrowToDVArray() const;
  ArraySpec narrowToVanilla() const;
  ArraySpec widenToBespoke() const;

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
  bool dvarray() const;
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
    IsDVArray = 1 << 4,
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
 * Class and RecordDesc type specialization.
 */
template<typename T> // T is either Class or RecordDesc
struct ClsRecSpec {
  /*
   * Constructor tags.
   */
  enum class SubTag {};
  enum class ExactTag {};

  /*
   * Constructors.
   */
  constexpr ClsRecSpec();
  ClsRecSpec(const T*, SubTag);
  ClsRecSpec(const T*, ExactTag);

  /*
   * Human-readable debug string.
   */
  std::string toString() const;

  /*
   * Accessors.
   */
  uintptr_t bits() const;
  bool exact() const;

  // Methods for accessing Class* and RecordDesc* of Class and RecordDesc
  // specializations respectively.
  template<typename D = void,
           typename = std::enable_if_t<std::is_same<T, Class>::value, D>>
  const Class* cls() const { return typeCns(); }
  template<typename D = void,
           typename = std::enable_if_t<std::is_same<T, Class>::value, D>>
  const Class* exactCls() const { return exactTypeCns(); }
  template<typename D = void,
           typename = std::enable_if_t<std::is_same<T, RecordDesc>::value, D>>
  const RecordDesc* rec() const { return typeCns(); }
  template<typename D = void,
           typename = std::enable_if_t<std::is_same<T, RecordDesc>::value, D>>
  const RecordDesc* exactRec() const { return exactTypeCns(); }

  /*
   * Casts.
   *
   * Bottom and Top cast to false; everything else casts to true.
   */
  explicit operator bool() const;

  /*
   * Comparisons.
   */
  bool operator==(const ClsRecSpec<T>& rhs) const;
  bool operator!=(const ClsRecSpec<T>& rhs) const;
  bool operator<=(const ClsRecSpec<T>& rhs) const;
  bool operator>=(const ClsRecSpec<T>& rhs) const;
  bool operator<(const ClsRecSpec<T>& rhs) const;
  bool operator>(const ClsRecSpec<T>& rhs) const;

  /*
   * Combinators.
   */
  ClsRecSpec<T> operator|(const ClsRecSpec<T>& rhs) const;
  ClsRecSpec<T> operator&(const ClsRecSpec<T>& rhs) const;
  ClsRecSpec<T> operator-(const ClsRecSpec<T>& rhs) const;

  /*
   * Top and bottom types.
   */
  static constexpr ClsRecSpec<T> Top();
  static constexpr ClsRecSpec<T> Bottom();

private:
  /*
   * Bottom constructor.
   */
  enum class BottomTag {};
  explicit constexpr ClsRecSpec(BottomTag);

  const T* typeCns() const;
  const T* exactTypeCns() const;


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

using ClassSpec = ClsRecSpec<Class>;
using RecordSpec = ClsRecSpec<RecordDesc>;
template<>
ClassSpec ClassSpec::operator&(const ClassSpec&) const;

template<>
RecordSpec RecordSpec::operator&(const RecordSpec&) const;

///////////////////////////////////////////////////////////////////////////////

/*
 * Specialization kinds.
 */
enum class SpecKind : uint8_t {
  None = 0,
  Array = 1 << 0,
  Class = 1 << 1,
  Record = 1 << 2,
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
  TypeSpec(ArraySpec, ClassSpec, RecordSpec);

  /*
   * Accessors.
   */
  SpecKind kind() const;
  ArraySpec arrSpec() const;
  ClassSpec clsSpec() const;
  RecordSpec recSpec() const;

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
  RecordSpec m_recSpec;
};

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/type-specialization-inl.h"

#endif
