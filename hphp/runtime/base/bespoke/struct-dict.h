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
  static StructDict* MakeFromVanilla(ArrayData* ad, const StructLayout* layout);

  template<bool Static>
  static StructDict* MakeReserve(const StructLayout* layout, bool legacy);

  static StructDict* MakeEmpty(const StructLayout* layout);
  static StructDict* AllocStructDict(uint8_t sizeIndex, uint32_t extra);

  static StructDict* MakeStructDict(
      uint8_t sizeIndex, uint32_t extra, uint32_t size,
      const uint8_t* slots, const TypedValue* vals);

  uint8_t sizeIndex() const;
  static size_t sizeFromLayout(const StructLayout*);

  static const StructDict* As(const ArrayData* ad);
  static StructDict* As(ArrayData* ad);

  // Precondition: v meets the type bound for this slot
  static ArrayData* SetStrInSlot(StructDict* adIn, Slot slot, TypedValue v);
  static void SetStrInSlotInPlace(StructDict* adIn, Slot slot, TypedValue v);

  // Precondition: this slot is not a required field
  static ArrayData* RemoveStrInSlot(StructDict* adIn, Slot slot);

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(StructDict)
#undef X

  const StructLayout* layout() const;

  size_t numFields() const;
  size_t typeOffset() const;
  size_t valueOffset() const;
  size_t positionOffset() const;

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

  static constexpr size_t valueOffsetOffset() {
    return offsetof(StructDict, m_extra_hi8);
  }
  static constexpr size_t valueOffsetSize() {
    return sizeof(m_extra_hi8);
  }

  static constexpr size_t numFieldsOffset() {
    return offsetof(StructDict, m_extra_lo8);
  }
  static constexpr size_t numFieldsSize() {
    return sizeof(m_extra_lo8);
  }

  static TypedValue NvGetStrNonStatic(
      const StructDict* sad, const StringData* k);
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
  struct Field {
    LowStringPtr key;
    bool required = false;
    uint8_t type_mask = 0;

    bool operator==(const Field& o) const {
      return key == o.key && required == o.required && type_mask == o.type_mask;
    }

    static constexpr size_t typeMaskOffset() {
      return offsetof(Field, type_mask);
    }
    static constexpr size_t typeMaskSize() {
      return sizeof(type_mask);
    }
  };

  using FieldVector = std::vector<Field>;

  static LayoutIndex Index(uint16_t raw);
  static bool IsStructLayout(LayoutIndex index);

  static const StructLayout* As(const Layout*);
  static const StructLayout* GetLayout(const FieldVector&, bool create);
  static const StructLayout* Deserialize(LayoutIndex index, const FieldVector&);

  size_t numFields() const;
  size_t sizeIndex() const;
  uint32_t extraInitializer() const;
  size_t numRequiredFields() const { return m_num_required_fields; }

  static Slot keySlot(LayoutIndex index, const StringData* key);
  static Slot keySlotStatic(LayoutIndex index, const StringData* key);
  static Slot keySlotNonStatic(LayoutIndex index, const StringData* key);

  Slot keySlot(const StringData* key) const;
  Slot keySlotNonStatic(const StringData* key) const;

  const Field& field(Slot slot) const;

  // Types come first in a StructDict payload, so this offset can be static.
  static constexpr size_t staticTypeOffset() { return sizeof(StructDict); }

  size_t typeOffset() const { return typeOffsetForSlot(0); }
  size_t valueOffset() const { return valueOffsetForSlot(0); }
  size_t positionOffset() const;

  // Offset of DataType and Value for 'slot' from beginning of a StructDict.
  size_t typeOffsetForSlot(Slot slot) const;
  size_t valueOffsetForSlot(Slot slot) const;

  // Check the type bound on this slot, at runtime or in the JIT
  bool checkTypeBound(Slot slot, TypedValue tv) const;
  Type getTypeBound(Slot slot) const;
  Type getUnionTypeBound() const;
  static uint8_t BoundToMask(const Type& type);
  static Type MaskToBound(uint8_t type_mask);

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  bool slotAlwaysPresent(const Type&) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;

  Type getTypeBound(Type slot) const override;

  Optional<int64_t> numElements() const override;

  void createColoringHashMap() const;

  // Perfect hashing implementation.
  struct PerfectHashEntry {
    LowStringPtr str;
    uint16_t valueOffset;
    uint8_t typeMask;
    uint8_t slot;
  };

  static constexpr size_t kMaxColor = 255;
  using PerfectHashTable = PerfectHashEntry[kMaxColor + 1];

  static PerfectHashTable* hashTable(const Layout* layout);
  static PerfectHashTable* hashTableSet();

  static constexpr size_t fieldsOffset() {
    return offsetof(StructLayout, m_fields);
  }

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

  StructLayout(LayoutIndex index, const FieldVector&);

  // Fields used to initialize a new StructDict. The "m_extra_initializer" is
  // computed when we create the layout and used to initialize three fields in
  // the array header in one go in the JIT.
  //
  // The field's layout should pun our usage of ArrayData's m_extra field.
  union {
    struct {
      uint8_t m_num_fields;
      uint8_t m_value_offset_in_values;
      bespoke::LayoutIndex m_layout_index;
    };
    uint32_t m_extra_initializer;
  };
  uint8_t m_size_index;
  uint8_t m_num_required_fields = 0;

  folly::F14FastMap<StaticKey, Slot, Hash, Equal> m_key_to_slot;

  // Variable-size array field; must be last in this struct.
  Field m_fields[1];
};

struct TopStructLayout : public AbstractLayout {
  TopStructLayout();
  static LayoutIndex Index();

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  bool slotAlwaysPresent(const Type&) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;
  Type getTypeBound(Type slot) const override;
};

}}
