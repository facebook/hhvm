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
#include "hphp/runtime/base/bespoke/struct-data-layout.h"
#include "hphp/runtime/base/string-data.h"

namespace HPHP::bespoke {

struct StructLayout;

namespace detail_struct_data_layout {
struct TypePosValLayout;
struct UnalignedTVLayout;
}

/*
 * Hidden-class style layout for a dict/darray. Static string keys are stored
 * in the layout itself instead of in the array. The layout maps these keys to
 * to physical slots. Each array has space for all of its layout's slots.
 */
struct StructDict : public BespokeArray {

  static StructDict* MakeFromVanilla(ArrayData*, const StructLayout*);

  template<bool Static>
  static StructDict* MakeReserve(const StructLayout* layout, bool legacy);

  static StructDict* MakeEmpty(const StructLayout* layout);
  static StructDict* AllocStructDict(uint8_t sizeIndex,
                                     uint32_t extra,
                                     bool mayContainCounted);

  static StructDict* MakeStructDictSmall(
      uint8_t sizeIndex, uint32_t extra, uint32_t size, bool mayContainCounted,
      const uint8_t* slots, const TypedValue* vals);

  static StructDict* MakeStructDictBig(
      uint8_t sizeIndex, uint32_t extra, uint32_t size, bool mayContainCounted,
      const uint16_t* slots, const TypedValue* vals);

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

  bool isBigStruct() const;

  const StructLayout* layout() const;

  size_t numFields() const;

  size_t positionOffset() const;

  const void* rawPositions() const;
  void* rawPositions();

  tv_lval lvalUnchecked(Slot slot);
  tv_rval rvalUnchecked(Slot slot) const;

  ArrayData* escalateWithCapacity(size_t capacity, const char* reason) const;
  arr_lval elemImpl(StringData* k, bool throwOnMissing);
  StructDict* copy() const;
  bool mayContainCounted() const { return m_aux16 & kMayContainCounted; }
  void incRefValues();
  void decRefValues();

  void addNextSlot(Slot slot);
  bool checkInvariants() const;

  static size_t numFieldsOffset() {
    return StructDataLayout::numFieldsOffset();
  }
  static size_t numFieldsSize() {
    return StructDataLayout::numFieldsSize();
  }

  static TypedValue NvGetStrNonStatic(
      const StructDict* sad, const StringData* k);

  friend detail_struct_data_layout::TypePosValLayout;
  friend detail_struct_data_layout::UnalignedTVLayout;

private:
  template<typename PosType>
  static StructDict* MakeFromVanillaImpl(ArrayData*, const StructLayout*);

  template<typename PosType>
  static StructDict* MakeStructDict(
    uint8_t sizeIndex, uint32_t extra, uint32_t size, bool mayContainCounted,
    const PosType* slots, const TypedValue* tvs);

  template<typename PosType> void removeSlot(Slot slot);
  template<typename PosType> Slot getSlotInPos(size_t pos) const;
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

  bool isBigStruct() const;
  size_t numFields() const;
  size_t sizeIndex() const;
  uint32_t extraInitializer() const;
  size_t numRequiredFields() const { return m_num_required_fields; }
  bool mayContainCounted() const { return m_may_contain_counted; }

  static Slot keySlot(LayoutIndex index, const StringData* key);
  static Slot keySlotStatic(LayoutIndex index, const StringData* key);
  static Slot keySlotNonStatic(LayoutIndex index, const StringData* key);

  Slot keySlot(const StringData* key) const;
  Slot keySlotNonStatic(const StringData* key) const;

  const Field& field(Slot slot) const;

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

  void createColoringHashMap(size_t numColoredFields) const;

  // Perfect hashing implementation.
  // If maybeDup is true, a hash miss does not imply that the field is not
  // present in the layout and StructLayout::m_key_to_slot needs to be checked.
  struct PerfectHashEntry {
    LowStringPtr str;
    uint8_t typeMask;
    bool maybeDup;
    uint16_t slot;
  };
  static_assert(!use_lowptr || sizeof(PerfectHashEntry) == 8);

  static constexpr size_t kMaxColor = 511;
  static_assert(kMaxColor > 2);
  using PerfectHashTable = PerfectHashEntry[kMaxColor + 1];

  static PerfectHashTable* hashTable(const Layout* layout);
  static PerfectHashTable* hashTableSet();
  static size_t maxColoredFields();
  static void setMaxColoredFields(size_t);

  static constexpr size_t fieldsOffset() {
    return offsetof(StructLayout, m_fields);
  }

  static size_t maxNumKeys() {
    return std::min<size_t>(
      RuntimeOption::EvalBespokeStructDictMaxNumKeys,
      std::numeric_limits<StructDataLayout::PosType>::max()
    );
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

  friend detail_struct_data_layout::TypePosValLayout;
  friend detail_struct_data_layout::UnalignedTVLayout;

  union {
    StructDataLayout::HeaderData m_header;
    uint32_t m_extra_initializer;
  };
  static_assert(sizeof(StructDataLayout::HeaderData) == 4);
  uint8_t m_size_index;
  bool m_may_contain_counted;
  uint16_t m_num_required_fields = 0;

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

// This returns a valid number after FinalizeHierarchy
// in non-jumpstart consumers only.
size_t numStructLayoutsCreated();
}
