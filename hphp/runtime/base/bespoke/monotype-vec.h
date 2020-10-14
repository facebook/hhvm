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
  static constexpr size_t kMinSizeIndex = 2;

  /**
   * Initializes layouts for MonotypeVec and for EmptyMonotypeVec.
   * Also initializes the static values of the EmptyMonotypeVec.
   */
  static void InitializeLayouts();

  /**
   * Create a new MonotypeVec. Note that the staticArr option is used to create
   * a non-deduplicated static MonotypeVec. This is only for temporary use to
   * satisfy static RATs while GetScalarArray is unimplemented.
   */
  static MonotypeVec* Make(DataType type, uint32_t size,
                           const TypedValue* values, HeaderKind hk,
                           bool legacy, bool staticArr);
  static MonotypeVec* As(ArrayData* ad);
  static const MonotypeVec* As(const ArrayData* ad);

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

  static MonotypeVec* MakeReserve(uint32_t cap, HeaderKind hk, bool legacy,
                                  bool staticArr);

  size_t capacity() const;
  uint8_t sizeIndex() const;
  Value* rawData();
  const Value* rawData() const;
  Value& valueRefUnchecked(uint32_t idx);
  const Value& valueRefUnchecked(uint32_t idx) const;
  TypedValue typedValueUnchecked(uint32_t idx) const;
  MonotypeVec* prepareForInsert();
  MonotypeVec* copyHelper(uint8_t newSizeIndex, bool incRef) const;
  MonotypeVec* copy() const;
  MonotypeVec* grow();
  DataType type() const;
  ArrayData* escalateWithCapacity(size_t capacity) const;
  bool checkInvariants() const;

  friend EmptyMonotypeVec;
};

struct EmptyMonotypeVec : public BespokeArray {
  static EmptyMonotypeVec* As(ArrayData* ad);
  static const EmptyMonotypeVec* As(const ArrayData* ad);

  static EmptyMonotypeVec* GetVec(bool legacy);
  static EmptyMonotypeVec* GetVArray(bool legacy);

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(EmptyMonotypeVec)
#undef X

private:
  static MonotypeVec* escalateToTyped(EmptyMonotypeVec* adIn, DataType type);

  bool checkInvariants() const;

  friend MonotypeVec;
};

struct EmptyMonotypeVecLayout : public Layout {
  explicit EmptyMonotypeVecLayout(LayoutIndex index);
};

struct MonotypeVecLayout : public Layout {
  MonotypeVecLayout(LayoutIndex index, DataType type);
  static std::string makeDescription(DataType type);
  DataType m_fixedType;
};

}}

#endif
