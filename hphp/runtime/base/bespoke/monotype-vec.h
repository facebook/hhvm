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

#ifndef HPHP_MONOTYPE_VEC_H_
#define HPHP_MONOTYPE_VEC_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/layout.h"

namespace HPHP { namespace bespoke {

struct EmptyMonotypeVec;

struct MonotypeVec : public BespokeArray {
  /**
   * Initializes layouts for MonotypeVec and for EmptyMonotypeVec.
   * Also initializes the static values of the EmptyMonotypeVec.
   */
  static void InitializeLayouts();

  /**
   * Create a new, empty MonotypeVec with the given capacity. The result will
   * have a refcount of 1, but if Static is true, it will be in static memory.
   */
  template <bool Static = false>
  static MonotypeVec* MakeReserve(bool legacy, uint32_t capacity, DataType dt);

  /**
   * Create a new MonotypeVec from the given vanilla vec or varray. The values
   * of the source array must have the given type. The new array will match
   * the input in kind, legacy bit, and static-ness.
   */
  static MonotypeVec* MakeFromVanilla(ArrayData* ad, DataType dt);

  static MonotypeVec* As(ArrayData* ad);
  static const MonotypeVec* As(const ArrayData* ad);

  static uint64_t entriesOffset() { return sizeof(MonotypeVec); }
  static uint64_t typeOffset() {
    static_assert(folly::kIsLittleEndian);
    return offsetof(MonotypeVec, m_extra_hi16);
  }

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(MonotypeVec)
#undef X

private:
  /**
   * Call `c` for each Countable value of the MonotypeVec and `mc` for each
   * MaybeCountable value. `c` takes a DataType and a Countable*, while `mc`
   * takes a DataType and a MaybeCountable*.
   */
  template <typename CountableFn, typename MaybeCountableFn>
  void forEachCountableValue(CountableFn c, MaybeCountableFn mc);
  void decRefValues();
  void incRefValues();

  ArrayData* appendImpl(TypedValue v);
  ArrayData* setIntImpl(int64_t k, TypedValue v);
  arr_lval elemImpl(int64_t k, bool throwOnMissing);

  size_t capacity() const;
  uint8_t sizeIndex() const;
  Value* rawData();
  const Value* rawData() const;
  Value& valueRefUnchecked(size_t idx);
  const Value& valueRefUnchecked(size_t idx) const;
  TypedValue typedValueUnchecked(size_t idx) const;
  MonotypeVec* prepareForInsert();
  MonotypeVec* copyHelper(uint8_t newSizeIndex, bool incRef) const;
  MonotypeVec* copy() const;
  MonotypeVec* grow();
  DataType type() const;
  ArrayData* escalateWithCapacity(size_t capacity, const char* reason) const;
  bool checkInvariants() const;

  friend EmptyMonotypeVec;
};

struct EmptyMonotypeVec : public BespokeArray {
  static EmptyMonotypeVec* As(ArrayData* ad);
  static const EmptyMonotypeVec* As(const ArrayData* ad);
  static EmptyMonotypeVec* GetVec(bool legacy);

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(EmptyMonotypeVec)
#undef X

private:
  bool checkInvariants() const;

  friend MonotypeVec;
};

struct EmptyMonotypeVecLayout : public ConcreteLayout {
  EmptyMonotypeVecLayout();
  static LayoutIndex Index();

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;
};

struct MonotypeVecLayout : public ConcreteLayout {
  explicit MonotypeVecLayout(DataType type);
  static LayoutIndex Index(DataType type);

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;

  DataType m_fixedType;
};

struct TopMonotypeVecLayout : public AbstractLayout {
  TopMonotypeVecLayout();
  static LayoutIndex Index();
};

bool isMonotypeVecLayout(LayoutIndex index);

}}

#endif
