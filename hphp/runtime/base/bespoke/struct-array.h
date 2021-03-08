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
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/key-order.h"
#include "hphp/runtime/base/bespoke/layout.h"

#include "hphp/runtime/vm/fixed-string-map.h"

namespace HPHP { namespace bespoke {

struct StructLayout;

struct StructArray : public BespokeArray {
  static StructArray* MakeFromVanilla(ArrayData* ad,
                                      const StructLayout* layout);
  template<bool Static>
  static StructArray* MakeReserve(
      HeaderKind kind, bool legacy, const StructLayout* layout);

  uint8_t sizeIndex() const;
  static size_t sizeFromLayout(const StructLayout*);

  static const StructArray* As(const ArrayData* ad);
  static StructArray* As(ArrayData* ad);

  static constexpr size_t kMaxKeyNum = 16;

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(StructArray)
#undef X

private:
  const StructLayout* layout() const;
  const TypedValue* rawData() const;
  TypedValue* rawData();
  ArrayData* escalateWithCapacity(size_t capacity, const char* reason) const;
  arr_lval elemImpl(StringData* k, bool throwOnMissing);
  StructArray* copy() const;
  void incRefValues();
  void decRefValues();

  static constexpr size_t kSlotSize = 4;
  void addNextSlot(Slot slot);
  void removeSlot(Slot slot);
  Slot getSlotInPos(size_t pos) const;
  bool checkInvariants() const;

  uint64_t m_order;
};

struct StructLayout : public ConcreteLayout {
  explicit StructLayout(const KeyOrder&, const LayoutIndex&);

  static LayoutIndex Index(uint8_t raw);
  static const StructLayout* getLayout(const KeyOrder&, bool create);
  static const StructLayout* As(const Layout*);

  size_t numFields() const;
  size_t sizeIndex() const;
  Slot keySlot(const StringData* key) const;
  LowStringPtr key(Slot slot) const;

private:
  size_t arraySize() const;

  // TODO(arnabde): Use an F14Map here instead.
  using FieldIndexMap = FixedStringMap<Slot,true>;
  FieldIndexMap m_fields;
  std::array<LowStringPtr, StructArray::kMaxKeyNum> m_slotToKey{};
  size_t m_size_index;
};

}}
