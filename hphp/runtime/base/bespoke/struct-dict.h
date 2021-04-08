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
#include "hphp/runtime/base/string-data.h"

namespace HPHP { namespace bespoke {

struct StructLayout;

/*
 * Hidden-class style layout for a dict/darray. Static string keys are stored
 * in the layout itself instead of in the array. The layout maps these keys to
 * to physical slots. Each array has space for all of its layout's slots.
 */
struct StructDict : public BespokeArray {
  static StructDict* MakeFromVanilla(ArrayData* ad,
                                     const StructLayout* layout);
  template<bool Static>
  static StructDict* MakeReserve(
      HeaderKind kind, bool legacy, const StructLayout* layout);

  static StructDict* AllocStructDict(
      const StructLayout* layout);

  // The `slots` array must be aligned to 8 bytes, and any leftover bytes must
  // be padded with KindOfUninit, because type bytes follow position bytes.
  static StructDict* MakeStructDict(
      const StructLayout* layout, uint32_t size,
      const uint8_t* slots, const TypedValue* vals);

  uint8_t sizeIndex() const;
  static size_t sizeFromLayout(const StructLayout*);

  static const StructDict* As(const ArrayData* ad);
  static StructDict* As(ArrayData* ad);

  static ArrayData* SetStrInSlot(StructDict* adIn, Slot slot, TypedValue v);
  static void SetStrInSlotInPlace(StructDict* adIn, Slot slot, TypedValue v);

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(StructDict)
#undef X

  const StructLayout* layout() const;

  size_t numFields() const;
  size_t typeOffset() const { return numFields(); }
  size_t valueOffsetInValueSize() const;
  const DataType* rawTypes() const;
  DataType* rawTypes();
  const Value* rawValues() const;
  Value* rawValues();
  const uint8_t* rawPositions() const;
  uint8_t* rawPositions();
  TypedValue typedValueUnchecked(Slot slot) const;

  ArrayData* escalateWithCapacity(size_t capacity, const char* reason) const;
  arr_lval elemImpl(StringData* k, bool throwOnMissing);
  StructDict* copy() const;
  void incRefValues();
  void decRefValues();

  void addNextSlot(Slot slot);
  void removeSlot(Slot slot);
  Slot getSlotInPos(size_t pos) const;
  bool checkInvariants() const;
};

/*
 * Layout data structure defining a hidden class. This data structure contains
 * two main pieces of data: a map from static string keys to physical slots,
 * and an array of field descriptors for each slot.
 *
 * Right now, the only data in the field descriptor is the field's string key.
 * However, in the future, we'll place two further constraints on fields which
 * will allow us to better-optimize JIT-ed code:
 *
 *  1. Type restrictions. Some fields will may only allow values of a certain
 *     data type (modulo countedness), saving us type checks on lookups.
 *
 *  2. "optional" vs. "required". Right now, all fields are optional. If we
 *     make some fields required, we can skip existence checks on lookup.
 */
struct StructLayout : public ConcreteLayout {
  struct Field { LowStringPtr key; };

  static LayoutIndex Index(uint8_t raw);
  static const StructLayout* As(const Layout*);
  static const StructLayout* GetLayout(const KeyOrder&, bool create);
  static const StructLayout* Deserialize(LayoutIndex index, const KeyOrder&);

  size_t numFields() const;
  size_t sizeIndex() const;
  Slot keySlot(const StringData* key) const;
  const Field& field(Slot slot) const;

  KeyOrder keyOrder() const { return m_key_order; }
  size_t typeOffset() const { return m_type_offset; }
  size_t valueOffset() const { return m_value_offset; }

  // Offset of DataType and Value for 'slot' from beginning of a StructDict.
  size_t typeOffsetForSlot(Slot slot) const;
  size_t valueOffsetForSlot(Slot slot) const;

  std::pair<Type, bool> elemType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;

private:
  // Callers must check whether the key is static before using one of these
  // wrapper types. The wrappers dispatch to the right hash/equal function.
  struct StaticKey { LowStringPtr key; };
  struct NonStaticKey { const StringData* key; };

  // Use heterogeneous lookup to optimize the lookup for static keys.
  struct Hash {
    using is_transparent = void;
    size_t operator()(const StaticKey& k) const { return k.key->hashStatic(); }
    size_t operator()(const NonStaticKey& k) const { return k.key->hash(); }
  };
  struct Equal {
    using is_transparent = void;
    bool operator()(const StaticKey& k1, const StaticKey& k2) const {
      return k1.key == k2.key;
    }
    bool operator()(const NonStaticKey& k1, const StaticKey& k2) const {
      return k1.key->same(k2.key);
    }
  };

  StructLayout(LayoutIndex index, const KeyOrder&);

  KeyOrder m_key_order;
  size_t m_size_index;

  // Offsets of datatypes and values in a StructDict
  // from the end of the array header.
  size_t m_type_offset;
  size_t m_value_offset;

  folly::F14FastMap<StaticKey, Slot, Hash, Equal> m_key_to_slot;

  // Variable-size array field; must be last in this struct.
  Field m_fields[1];
};

}}
