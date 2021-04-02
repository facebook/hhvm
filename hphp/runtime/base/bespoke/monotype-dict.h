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
#include "hphp/runtime/base/bespoke/entry-types.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/typed-value.h"

/*
 * MonotypeDict<Key> can represent dicts and darrays with a fixed Key type and
 * with a single DataType for all of its values. There are only three valid
 * values for the Key type parameter:
 *
 *  - Key = int64_t: int-keyed dicts
 *  - Key = StringData*: str-keyed dicts
 *  - Key = LowStringPtr: static-str-keyed dicts
 *
 * Within the implementation, we have a battery of helpers, like coerceKey,
 * hashKey, and keysEqual, templated on Key, that let us express operations
 * on these keys generically. These helpers would static_assert if they were
 * instantiated for a different Key type.
 *
 * coerceKey takes an int or string input key and coerces it to the Key type.
 * If the input cannot be coerced (e.g. if the Key is a LowStringPtr and the
 * string input key has no static counterpart), it returns a tombstone Key,
 * which typically causes escalation. For strings, the tombstone is nullptr;
 * for ints, it's std::numeric_limits<int64_t>::min().
 *
 * The layout of MonotypeDict<Key> is the same for all Key types. The smallest
 * monodict has a capacity of 5 elements in 128 bytes (compare to the smallest
 * MixedArray: 3 elements in 120 rounding up to 128 bytes):
 *
 *  16-byte ArrayData header:
 *      8-byte HeapObject header:
 *          4-byte refcount
 *          1-byte ArrayKind
 *          1-byte GC bits
 *          1-byte aux bits
 *          1-byte size index
 *      4-byte m_size field
 *      2-byte m_tombstones field
 *      2-byte bespoke::LayoutIndex
 *          (low byte stores the DataType!)
 *  5x16-byte MixedArray<Key>::Elm
 *      8-byte Key
 *      8-byte Value
 *  8x4-byte MixedArray<Key>::Index
 *      (hash table indices into the Elm array)
 *
 * When we double the array size, we always have space for one extra element
 * because the header is fixed (so the capacity goes: 5, 11, 23, 47, etc.)
 *
 * As we can see, MonotypeDict is very tightly packed. As a consequence of its
 * design, it can only store up to 1 << 16 tombstones - it shrinks when we are
 * at that limit and we remove another element.
 *
 * We have some basic logic in MonotypeDict to maintain the layout above - see
 * sizeIndex(), used(), numElms(), numIndices(), and elmAtIndex(). They just
 * do simple arithmetic to handle these cases. The core of this arithmetic is
 * the scaleForSizeIndex() helper (which computes a base-2 exponential).
 *
 * The next piece of the implementation is the find(Key, hash) API. This API
 * is templated on the operation we're doing, and returns a different result
 * for each operation type:
 *
 *  - find<Add> can only be used when adding a NEW key to the array.
 *    It returns a non-null pointer to the hash table index to set.
 *  - find<Get> returns a pointer to the Elm for that key, or nullptr
 *    if the key is not present.
 *  - find<Remove> returns the position of the hash table index in the
 *    indices() array to remove, or -1 if the key is not present.
 *  - find<Update> returns an {Elm*, Index*}, where the Elm* may be null
 *    but the Index* never is.
 *
 * The logic of find differs in crucial ways between these operations. For
 * example, Add, Get, and Remove skip over tombstones completely, while Update
 * records the first tombstone index so that it can re-use this index if the
 * element is missing. Add additionally skips key equality checks.
 *
 * Because numElms() < numIndices(), there is always an empty index in the hash
 * table. We use this property to simplify the termination condition for find.
 *
 * After building up all this machinery, the array data operations are mostly
 * careful invocations of find, plus refcounting.
 */

namespace HPHP { namespace bespoke {

struct EmptyMonotypeDict : BespokeArray {
  using Self = EmptyMonotypeDict;
  static void InitializeLayouts();

  static EmptyMonotypeDict* As(ArrayData* ad);
  static const EmptyMonotypeDict* As(const ArrayData* ad);
  static EmptyMonotypeDict* GetDict(bool legacy);

  bool checkInvariants() const;

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(EmptyMonotypeDict)
#undef X
};

template <typename Key>
struct MonotypeDict : BespokeArray {
  static uint8_t ComputeSizeIndex(size_t size);

  // Create a new, empty MonotypeDict with the given capacity. The result will
  // have a refcount of 1, but if Static is true, it will be in static memory.
  template <bool Static = false>
  static MonotypeDict* MakeReserve(bool legacy, size_t capacity, DataType dt);

  static MonotypeDict* MakeFromVanilla(ArrayData* ad, DataType dt);

  static MonotypeDict* As(ArrayData* ad);
  static const MonotypeDict* As(const ArrayData* ad);

  bool checkInvariants() const;

  // Helpers used to JIT fast accessors for these array-likes. We can call
  // these helpers on a MonotypeDict<Key> for any Key type.
  static constexpr size_t entriesOffset() {
    return sizeof(Self);
  }
  static constexpr size_t elmSize() {
    return sizeof(Elm);
  }
  static constexpr size_t elmKeyOffset() {
    return offsetof(Elm, key);
  }
  static constexpr size_t elmValOffset() {
    return offsetof(Elm, val);
  }
  static constexpr size_t tombstonesOffset() {
    return offsetof(Self, m_extra_lo16);
  }
  static constexpr size_t tombstonesSize() {
    return sizeof(m_extra_lo16);
  }
  static constexpr size_t typeOffset() {
    static_assert(folly::kIsLittleEndian);
    return offsetof(Self, m_extra_hi16);
  }
  // This bit is set in our layout index iff we have int keys.
  static constexpr LayoutIndex intKeyMask() {
    return {(kIntMonotypeDictLayoutByte & ~kStrMonotypeDictLayoutByte) << 8};
  }

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(MonotypeDict<Key>)
#undef X

private:
  using Index = uint32_t;
  using Self = MonotypeDict<Key>;
  struct Elm { Key key; Value val; };

  static constexpr Index kEmptyIndex = Index(-1);
  static constexpr Index kTombstoneIndex = Index(-2);

  // Different modes for the core find() implementation.
  struct Add { Index* index; size_t chain_length; };
  struct Get { const Elm* elm; };
  struct Remove { ssize_t hash_pos; };
  struct Update { Elm* elm; Index* index; };
  template <typename Result> Result find(Key key, strhash_t hash);

  Add findForAdd(strhash_t hash);
  const Elm* findForGet(Key key, strhash_t hash) const;

  TypedValue getImpl(Key key) const;
  ssize_t getPosImpl(Key key) const;
  ArrayData* removeImpl(Key key);
  template <typename K> arr_lval elemImpl(Key key, K k, bool throwOnMissing);
  arr_lval lvalDispatch(int64_t k);
  arr_lval lvalDispatch(StringData* k);
  ArrayData* appendImpl(TypedValue v);
  template <typename K>
  ArrayData* setImpl(Key key, K k, TypedValue v);

  // Iterate over values of this MonotypeDict, calling these callbacks for
  // each element. `k` takes a Key; `c`, `m`, and `u` take a TypedValue.
  template <typename KeyFn, typename CountedFn,
            typename MaybeCountedFn, typename UncountedFn>
  void forEachElm(KeyFn k, CountedFn c, MaybeCountedFn m, UncountedFn u) const;

  // Simpler variant that just executes a function for each non-tombstone Elm.
  template <typename ElmFn> void forEachElm(ElmFn e) const;

  void incRefElms();
  void decRefElms();
  void copyHash(const Self* other);
  void initHash();
  MonotypeDict* copy();
  MonotypeDict* prepareForInsert();
  MonotypeDict* compactIfNeeded(bool free);
  MonotypeDict* resize(uint8_t index, bool copy);
  ArrayData* escalateWithCapacity(size_t capacity, const char* reason) const;

  Elm* elms();
  const Elm* elms() const;
  Elm* elmAtIndex(Index index);
  const Elm* elmAtIndex(Index index) const;
  Index* indices();
  const Index* indices() const;

  DataType type() const;
  uint32_t used() const;
  uint32_t tombstones() const;
  uint8_t sizeIndex() const;
  size_t numElms() const;
  size_t numIndices() const;
  void setZombie();
  bool isZombie() const;
  bool hasEmptyLayout() const;

  friend EmptyMonotypeDict;
};

struct TopMonotypeDictLayout : public AbstractLayout {
  explicit TopMonotypeDictLayout(KeyTypes kt);
  static LayoutIndex Index(KeyTypes kt);

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;

  KeyTypes m_keyType;
};

struct EmptyMonotypeDictLayout : public ConcreteLayout {
  explicit EmptyMonotypeDictLayout();
  static LayoutIndex Index();

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;
};

struct MonotypeDictLayout : public ConcreteLayout {
  MonotypeDictLayout(KeyTypes kt, DataType type);
  static LayoutIndex Index(KeyTypes kt, DataType type);

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;

  KeyTypes m_keyType;
  DataType m_valType;
};

bool isMonotypeDictLayout(LayoutIndex index);

BespokeArray* MakeMonotypeDictFromVanilla(ArrayData*, DataType, KeyTypes);

}}
