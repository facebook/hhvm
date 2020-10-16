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
#include "hphp/runtime/vm/srckey.h"

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
 * TODO(kshaunak): we should escalate from static-string-keys to string-keys.
 *
 * The layout of MonotypeDict<Key> is the same for all Key types. The smallest
 * monodict has a capacity of 6 elements in 128 bytes (compare to the smallest
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
 *      2-byte m_used field
 *      2-byte bespoke::LayoutIndex
 *          (low byte stores the DataType!)
 *  6x16-byte MixedArray<Key>::Elm
 *      8-byte Key
 *      8-byte Value
 *  8x2-byte MixedArray<Key>::Index
 *      (hash table indices into the Elm array)
 *
 * As we can see, MonotypeDict is very tightly packed. As a consequence of its
 * design, it can only store up to 1 << 16 elements, and escalates to vanilla
 * when it hits this capacity bound.
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
 * In the hash table, the number of non-empty indices (valid and tombstone)
 * is always bounded by m_used. We can prove this invariant inductively. Set
 * with a new key adds a valid key but increments m_used. Set with an old key
 * has no effect. Remove converts a valid key to a tombstone (no change to
 * the count of non-empty indices) and does not change m_used.
 *
 * Because m_used <= numElms(), and because numElms() < numIndices(), there is
 * always an empty index in the hash table. We use this property to simplify
 * the termination condition for find.
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
  static EmptyMonotypeDict* GetDArray(bool legacy);

  bool checkInvariants() const;

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(EmptyMonotypeDict)
#undef X
};

template <typename Key>
struct MonotypeDict : BespokeArray {
  static uint8_t ComputeSizeIndex(size_t size);
  static MonotypeDict* MakeReserve(HeaderKind kind, size_t size, DataType dt);
  static MonotypeDict* MakeFromVanilla(ArrayData* ad, DataType dt);

  static MonotypeDict* As(ArrayData* ad);
  static const MonotypeDict* As(const ArrayData* ad);

  bool checkInvariants() const;

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(MonotypeDict<Key>)
#undef X

private:
  using Index = int16_t;
  using Self = MonotypeDict<Key>;
  struct Elm { Key key; Value val; };

  static constexpr Index kEmptyIndex = -1;
  static constexpr Index kTombstoneIndex = -2;

  // Different modes for the core find() implementation.
  struct Add { Index* index; };
  struct Get { const Elm* elm; };
  struct Remove { ssize_t hash_pos; };
  struct Update { Elm* elm; Index* index; };
  template <typename Result> Result find(Key key, strhash_t hash);

  Index* findForAdd(strhash_t hash);
  const Elm* findForGet(Key key, strhash_t hash) const;

  TypedValue getImpl(Key key) const;
  ssize_t getPosImpl(Key key) const;
  ArrayData* removeImpl(Key key);
  template <typename K> arr_lval elemImpl(Key key, K k);
  template <typename K> ArrayData* setImpl(Key key, K k, TypedValue v);

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
  MonotypeDict* resize(uint8_t index, bool copy);
  ArrayData* escalateWithCapacity(size_t capacity) const;

  Elm* elms();
  const Elm* elms() const;
  Elm* elmAtIndex(Index index);
  const Elm* elmAtIndex(Index index) const;
  Index* indices();
  const Index* indices() const;

  DataType type() const;
  uint32_t used() const;
  uint8_t sizeIndex() const;
  size_t numElms() const;
  size_t numIndices() const;
  void setZombie();
  bool isZombie() const;
};

ArrayData* MakeMonotypeDictFromVanilla(ArrayData* ad, DataType dt, KeyTypes kt);

}}
