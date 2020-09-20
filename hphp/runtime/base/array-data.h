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

#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/str-key-table.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"

#include <folly/Likely.h>

#include <climits>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct APCArray;
struct Array;
struct String;
struct StringData;
struct VariableSerializer;
struct Variant;

namespace arrprov { struct Tag; }

/*
 * arr_lval is a tv_lval augmented with an ArrayData*, and is used to return an
 * lval from array mutations. `arr` holds the copied/escalated/grown array if
 * any of those happened during the operation, or the original array if not. It
 * can otherwise be treated as a tv_lval, and is implicitly converted to one
 * shortly after being created in most cases.
 */
struct arr_lval : tv_lval {
  template<typename... Args>
  arr_lval(ArrayData* arr, Args... lval_args)
    : tv_lval{std::forward<Args>(lval_args)...}
    , arr{arr}
  {}

  ArrayData* const arr;
};

/*
 * We use this enum as a template parameter in a few key places to determine
 * whether we should explicitly perform legacy PHP intish key cast.
 */
enum class IntishCast : int8_t { None, Cast };

struct ArrayData : MaybeCountable {
  /*
   * Runtime type tag of possible array types.  This is intentionally not an
   * enum class, since we're using it pretty much as raw bits (these tag values
   * are not private), which avoids boilerplate when:
   *  - doing relational comparisons
   *  - using kind as an index
   *  - doing bit ops when storing in the union'd words below
   *
   * Beware if you change the order or the numerical values, as there are a few
   * dependencies.  Also, all of the values need to be continuous from 0 to =
   * kNumKinds-1 since we use these values to index into a table.
   */
  enum ArrayKind : uint8_t {
    kMixedKind,         // darray: dict-like array with int or string keys
    kBespokeDArrayKind,
    kPackedKind,        // varray: vec-like array with keys in range [0..size)
    kBespokeVArrayKind,
    kDictKind,
    kBespokeDictKind,
    kVecKind,
    kBespokeVecKind,
    kKeysetKind,
    kBespokeKeysetKind,
    kNumKinds           // Insert new values before kNumKinds.
  };

  /*
   * This bit is set for bespoke ArrayKinds, and not for vanilla kinds.
   */
  static auto constexpr kBespokeKindMask = uint8_t{0x01};

  /*
   * For uncounted Packed, Mixed, Dict and Vec, indicates that the
   * array was co-allocated with an APCTypedValue (at apctv+1).
   */
  static auto constexpr kHasApcTv = 1;

  /*
   * Indicates that this dict or vec should use some legacy (i.e.,
   * PHP-compatible) behaviors, including serialization
   */
  static auto constexpr kLegacyArray = 2;

  /*
   * Indicates that this array has a side table that contains information about
   * its keys.
   */
  static auto constexpr kHasStrKeyTable = 4;

  /////////////////////////////////////////////////////////////////////////////
  // Creation and destruction.

protected:
  /*
   * We can't `= delete` this because we subclass ArrayData.
   */
  ~ArrayData() { always_assert(false); }

public:
  /*
   * Create a new empty ArrayData with the appropriate ArrayKind.
   */
  static ArrayData* Create();
  static ArrayData* CreateVec(arrprov::Tag tag = {});
  static ArrayData* CreateDict(arrprov::Tag tag = {});
  static ArrayData* CreateKeyset();
  static ArrayData* CreateVArray(arrprov::Tag tag = {});
  static ArrayData* CreateDArray(arrprov::Tag tag = {});

  /*
   * Create a new kPackedKind ArrayData with a single element, `value'.
   *
   * Initializes `value' if it's UninitNull.
   */
  static ArrayData* Create(TypedValue value);
  static ArrayData* Create(const Variant& value);

  /*
   * Create a new kMixedKind ArrayData with a single key `name' and value
   * `value'.
   *
   * Initializes `value' if it's UninitNull.
   */
  static ArrayData* Create(TypedValue name, TypedValue value);
  static ArrayData* Create(const Variant& name, TypedValue value);
  static ArrayData* Create(const Variant& name, const Variant& value);

  /*
   * Make a copy of the array.
   *
   * copy() makes a normal request-allocated ArrayData, whereas copyStatic()
   * makes a static ArrayData.
   */
  ArrayData* copy() const;
  ArrayData* copyStatic() const;

  /*
   * Convert between array kinds.
   */
  ArrayData* toPHPArray(bool copy);
  ArrayData* toPHPArrayIntishCast(bool copy);
  ArrayData* toDict(bool copy);
  ArrayData* toVec(bool copy);
  ArrayData* toKeyset(bool copy);
  ArrayData* toVArray(bool copy);
  ArrayData* toDArray(bool copy);

  /*
   * Return the array to the request heap.
   *
   * This is normally called when the reference count goes to zero (e.g., via a
   * helper like decRefArr()).
   */
  void release() noexcept;

  /*
   * Decref the array and release() it if its refcount goes to zero.
   */
  void decRefAndRelease();

  /////////////////////////////////////////////////////////////////////////////
  // Introspection.

  /*
   * Number of elements. Never requires virtual dispatch.
   */
  size_t size() const;

  /*
   * Whether the array has no elements.
   */
  bool empty() const;

  /*
   * Whether the array's m_kind is set to a valid value.
   */
  bool kindIsValid() const;

  /*
   * Array kind.
   *
   * @requires: kindIsValid()
   */
  ArrayKind kind() const;

  /*
   * Whether the array has a particular kind.
   */
  bool isPackedKind() const;
  bool isMixedKind() const;
  bool isPlainKind() const;
  bool isDictKind() const;
  bool isVecKind() const;
  bool isKeysetKind() const;

  /*
   * Whether the array has a particular Hack type
   */
  bool isPHPArrayType() const;
  bool isHackArrayType() const;
  bool isVecType() const;
  bool isDictType() const;
  bool isKeysetType() const;

  /*
   * Whether the ArrayData is backed by PackedArray or MixedArray.
   */
  bool hasVanillaPackedLayout() const;
  bool hasVanillaMixedLayout() const;

  /*
   * Whether the array-like has the standard layout. This check excludes
   * array-likes with a "bespoke" hidden-class layout.
   */
  bool isVanilla() const;

  /*
   * A faster test to see if both of the array-likes are vanilla.
   */
  static bool bothVanilla(const ArrayData*, const ArrayData*);

  /*
   * Only used for uncounted arrays. Indicates that there's a
   * co-allocated APCTypedValue preceding this array.
   */
  bool hasApcTv() const;

  /*
   * Whether the array has legacy behaviors enabled (this bit can only be set
   * for vecs and dicts).
   */
  bool isLegacyArray() const;
  void setLegacyArray(bool legacy);

  bool hasStrKeyTable() const;

  /* Get the aux bits in the header that must be preserved
   * when we copy or resize the array
   */
  uint8_t auxBits() const;

  /*
   * Is the array a varray, darray, either, or neither?
   */
  bool isVArray() const;
  bool isDArray() const;
  bool isDVArray() const;
  bool isNotDVArray() const;
  bool isHAMSafeVArray() const;
  bool isHAMSafeDArray() const;
  bool isHAMSafeDVArray() const;

  static bool dvArrayEqual(const ArrayData* a, const ArrayData* b);

  /*
   * Whether the array contains "vector-like" data---i.e., iteration order
   * produces int keys 0 to size() - 1 in sequence.
   *
   * For non-hasPackedLayout() arrays, this is generally an O(N) operation.
   */
  bool isVectorData() const;

  /*
   * Return true for array kinds that don't have COW semantics.
   */
  bool noCopyOnWrite() const;

  /*
   * ensure a circular self-reference is not being created
   */
  bool notCyclic(TypedValue v) const;

  /*
   * Get the DataType (persistent or non-persistent version) corresponding to
   * the array's kind.
   */
  DataType toDataType() const;
  DataType toPersistentDataType() const;

  /////////////////////////////////////////////////////////////////////////////
  // Element manipulation.
  //
  // @see: array-data.cpp, for further documentation in the array function
  // table.

  /*
   * Test whether an element exists at key `k'.
   */
  bool exists(int64_t k) const;
  bool exists(const StringData* k) const;
  bool exists(TypedValue k) const;
  bool exists(const String& k) const;
  bool exists(const Variant& k) const;

  /*
   * Get an lval for the element at key `k'.
   */
  arr_lval lval(int64_t k);
  arr_lval lval(StringData* k);
  arr_lval lval(TypedValue k);
  arr_lval lval(const String& k);
  arr_lval lval(const Variant& k);

  /*
   * Get the value of the element at key `k'.
   *
   * @requires: exists(k)
   */
  TypedValue at(int64_t k) const;
  TypedValue at(const StringData* k) const;
  TypedValue at(TypedValue k) const;

  /*
   * Get the internal position for element with key `k', if it exists.
   * If the key is not present then these return the canonical invalid position
   * (i.e. iter_end()).
   * The returned values can be passed to atPos or nvGetKey (if they're not the
   * canonical invalid position).
   */
  ssize_t nvGetIntPos(int64_t k) const;
  ssize_t nvGetStrPos(const StringData* k) const;

  /*
   * Get the value or key for the element at raw position `pos'.
   *
   * nvGetKey will inc-ref the key before returning it, but nvGetVal will not
   * do any refcount ops. TODO(kshaunak): Eliminate this discrepancy.
   *
   * @requires: `pos' refers to a valid array element.
   */
  TypedValue nvGetKey(ssize_t pos) const;
  TypedValue nvGetVal(ssize_t pos) const;

  /*
   * Variant wrappers around nvGetVal() and nvGetKey(). Both of these methods
   * will inc-ref the value before returning it (so that callers own a copy).
   */
  Variant getKey(ssize_t pos) const;
  Variant getValue(ssize_t pos) const;

  /*
   * Get the value of the element at key `k'.
   *
   * If `error` is false, get returns an Uninit TypedValue if `k` is missing.
   * If `error` is true, get throws if `k` is missing.
   */
  TypedValue get(TypedValue k, bool error = false) const;
  TypedValue get(int64_t k, bool error = false) const;
  TypedValue get(const StringData* k, bool error = false) const;
  TypedValue get(const String& k, bool error = false) const;
  TypedValue get(const Variant& k, bool error = false) const;

  /*
   * Set the element at key `k' to `v'. set() methods make a copy first if
   * cowCheck() returns true. If `v' is a ref, its inner value is used.
   *
   * Semantically, setMove() methods 1) do a set, 2) dec-ref the value, and
   * 3) if the operation required copy/escalation, dec-ref the old array. This
   * sequence is needed for member ops and can be implemented more efficiently
   * if done as a single unit.
   *
   * These methods return `this' if copy/escalation are not needed, or a
   * copied/escalated array data if they are.
   */
  ArrayData* set(int64_t k, TypedValue v);
  ArrayData* set(StringData* k, TypedValue v);
  ArrayData* set(TypedValue k, TypedValue v);
  ArrayData* set(const String& k, TypedValue v);
  ArrayData* setMove(int64_t k, TypedValue v);
  ArrayData* setMove(StringData* k, TypedValue v);

  ArrayData* set(int64_t k, const Variant& v);
  ArrayData* set(StringData* k, const Variant& v);
  ArrayData* set(const String& k, const Variant& v);
  ArrayData* set(const Variant& k, const Variant& v);

  ArrayData* set(const StringData*, TypedValue) = delete;
  ArrayData* set(const StringData*, const Variant&) = delete;

  /*
   * Remove the value at key `k', making a copy first if necessary. Returns
   * `this' if copy/escalation are not needed, or a copied/escalated ArrayData.
   */
  ArrayData* remove(int64_t k);
  ArrayData* remove(const StringData* k);
  ArrayData* remove(TypedValue k);
  ArrayData* remove(const String& k);
  ArrayData* remove(const Variant& k);

  /**
   * Append `v' to the array, making a copy first if necessary. Returns `this`
   * if copy/escalation are not needed, or a copied/escalated ArrayData.
   */
  ArrayData* append(TypedValue v);

  /////////////////////////////////////////////////////////////////////////////
  // Iteration.

  /*
   * @see: array-data.cpp, for documentation for IterEnd, IterBegin, etc.
   */
  ssize_t iter_begin() const;
  ssize_t iter_last() const;
  ssize_t iter_end() const;
  ssize_t iter_advance(ssize_t prev) const;
  ssize_t iter_rewind(ssize_t prev) const;

  /*
   * Like getValue(), except if `pos' is specifically the canonical invalid
   * position (i.e., iter_end()), return false.
   */
  Variant value(int32_t pos) const;

  /////////////////////////////////////////////////////////////////////////////
  // PHP array functions.

  /*
   * Called prior to sorting this array. Some array kinds
   * have layouts that are overly constrained to sort in-place.
   */
  ArrayData* escalateForSort(SortFunction sort_function);

  /*
   * PHP sort implementations.
   */
  void ksort(int sort_flags, bool ascending);
  void sort(int sort_flags, bool ascending);
  void asort(int sort_flags, bool ascending);
  bool uksort(const Variant& cmp_function);
  bool usort(const Variant& cmp_function);
  bool uasort(const Variant& cmp_function);

  /*
   * PHP array_merge() implementations.
   */
  ArrayData* merge(const ArrayData* elems);

  /*
   * Remove the first or last element of the array, and assign it to `value'.
   *
   * These implement:
   *  - dequeue(): array_shift()
   *  - pop(): array_pop()
   *
   * Return `this' if copy/escalation are not needed, or a copied/escalated
   * array data.
   */
  ArrayData* dequeue(Variant& value);
  ArrayData* pop(Variant& value);

  /*
   * Prepend `v' to the array, making a copy first if cowCheck() returns true.
   *
   * This implements array_unshift().
   *
   * Return `this' if copy/escalation are not needed, or a copied/escalated
   * array data.
   */
  ArrayData* prepend(TypedValue v);

  /*
   * Comparisons.
   */
  bool same(const ArrayData* v2) const;

  static bool Equal(const ArrayData*, const ArrayData*);
  static bool NotEqual(const ArrayData*, const ArrayData*);
  static bool Same(const ArrayData*, const ArrayData*);
  static bool NotSame(const ArrayData*, const ArrayData*);
  static bool Lt(const ArrayData*, const ArrayData*);
  static bool Lte(const ArrayData*, const ArrayData*);
  static bool Gt(const ArrayData*, const ArrayData*);
  static bool Gte(const ArrayData*, const ArrayData*);
  static int64_t Compare(const ArrayData*, const ArrayData*);

  /////////////////////////////////////////////////////////////////////////////
  // Static arrays.

  /*
   * If `arr' points to a static array, do nothing.  Otherwise, make a static
   * copy, destroy the original, and update `*arr`.
   *
   * If `tag` is set or `arr` has provenance data, we copy the tag to the new
   * static array.  (A set `tag` overrides the provenance of `arr`.)
   */
  static void GetScalarArray(ArrayData** arr,
                             arrprov::Tag tag = {});

  /*
   * Promote the array referenced by `arr` to a static array and return it.
   */
  static ArrayData* GetScalarArray(Array&& arr);
  static ArrayData* GetScalarArray(Variant&& arr);

  /*
   * Static-ify the contents of the array.
   */
  void onSetEvalScalar();

  /////////////////////////////////////////////////////////////////////////////
  // Other functions.
  //
  // You should avoid adding methods to this section.  If the logic you're
  // implementing is specific to a particular subsystem, define it as a helper
  // there instead.
  //
  // If you absolutely must add more methods to ArrayData here, just follow
  // these simple guidelines:
  //
  //    (1) Don't add more methods to ArrayData here.

  /*
   * Perform intish-string array key conversion on `key'.
   *
   * Return whether `key' should undergo intish-cast when used in this array
   * (which may depend on the array kind, e.g.).  If true, `i' is set to the
   * intish value of `key'.
   */
  bool intishCastKey(const StringData* key, int64_t& i) const;

  /*
   * Re-index all numeric keys to start from 0. This operation may require
   * escalation on the array, so it returns the new result.
   */
  ArrayData* renumber();

  /*
   * Get the string name for the array kind `kind'.
   */
  static const char* kindToString(ArrayKind kind);

  static constexpr uint32_t MaxElemsOnStack = 64;

  /*
   * Offset accessors.
   */
  static constexpr size_t offsetofSize() { return offsetof(ArrayData, m_size); }
  static constexpr size_t sizeofSize() { return sizeof(m_size); }

  const StrKeyTable& missingKeySideTable() const {
    assertx(this->hasStrKeyTable());
    auto const pointer = reinterpret_cast<const char*>(this)
      - sizeof(StrKeyTable);
    return *reinterpret_cast<const StrKeyTable*>(pointer);
  }

  StrKeyTable* mutableStrKeyTable() {
    assertx(this->hasStrKeyTable());
    auto const pointer = reinterpret_cast<char*>(this)
      - sizeof(StrKeyTable);
    return reinterpret_cast<StrKeyTable*>(pointer);
  }

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Helpers for IterateV and IterateKV.
   */
  template <typename Fn, class... Args> ALWAYS_INLINE
  static typename std::enable_if<
    std::is_same<typename std::result_of<Fn(Args...)>::type, void>::value,
    bool
  >::type call_helper(Fn f, Args&&... args) {
    f(std::forward<Args>(args)...);
    return false;
  }

  template <typename Fn, class... Args> ALWAYS_INLINE
  static typename std::enable_if<
    std::is_same<typename std::result_of<Fn(Args...)>::type, bool>::value,
    bool
  >::type call_helper(Fn f, Args&&... args) {
    return f(std::forward<Args>(args)...);
  }

  template <typename B, class... Args> ALWAYS_INLINE
  static typename std::enable_if<
    std::is_same<B, bool>::value,
    bool
  >::type call_helper(B f, Args&&... /*args*/) {
    return f;
  }

  /*
   * Throw an out of bounds exception if 'k' is undefined. The text of the
   * message depends on the array's type.
   */
  [[noreturn]] void getNotFound(int64_t k) const;
  [[noreturn]] void getNotFound(const StringData* k) const;

  /////////////////////////////////////////////////////////////////////////////

protected:
  /*
   * Is `k' of an arraykey type (i.e., int or string)?
   */
  static bool IsValidKey(TypedValue k);
  static bool IsValidKey(const Variant& k);
  static bool IsValidKey(const String& k);
  static bool IsValidKey(const StringData* k);

  /////////////////////////////////////////////////////////////////////////////

private:
  friend size_t getMemSize(const ArrayData*, bool);

  static bool EqualHelper(const ArrayData*, const ArrayData*, bool);
  static int64_t CompareHelper(const ArrayData*, const ArrayData*);

  /////////////////////////////////////////////////////////////////////////////

protected:
  friend struct PackedArray;
  friend struct EmptyArray;
  friend struct MixedArray;
  friend struct BaseVector;
  friend struct c_Vector;
  friend struct c_ImmVector;
  friend struct HashCollection;
  friend struct BaseMap;
  friend struct c_Map;
  friend struct c_ImmMap;
  friend struct arrprov::Tag;

  uint32_t m_size;
  /*
   * m_extra is used both to store bespoke IDs and for array provenance (for
   * v/darrays.) It's fine to share the field since we already refuse to enable
   * these features together.
   *
   * When RO::EvalArrayProvenance is on, this stores an arrprov::Tag--otherwise
   * we use this field as follows:
   *
   * When the array is bespoke (m_kind & kBespokeKindMask):
   *
   *   bits 0..15: for private bespoke-array use. we don't require these to be
   *                have any specific value but are reserved for use by bespoke
   *                layouts
   *
   *   bits 16..30: store the bespoke layout index
   *
   *   bit 31:      must be set
   *
   * When the array is vanilla and array provenance is off, m_extra must be 0.
   */
  union {
    uint32_t m_extra;
    struct {
      /* NB the names are definitely little-endian centric but whatever */
      uint16_t m_extra_lo16;
      uint16_t m_extra_hi16;
    };
  };
};

static_assert(ArrayData::kPackedKind == uint8_t(HeaderKind::Packed), "");
static_assert(ArrayData::kMixedKind == uint8_t(HeaderKind::Mixed), "");
static_assert(ArrayData::kDictKind == uint8_t(HeaderKind::Dict), "");
static_assert(ArrayData::kVecKind == uint8_t(HeaderKind::Vec), "");

//////////////////////////////////////////////////////////////////////

constexpr size_t kEmptyMixedArraySize = 120;
constexpr size_t kEmptySetArraySize = 96;

/*
 * Storage for the static empty arrays.
 */
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVec;
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVArray;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDictArray;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDArray;
extern std::aligned_storage<kEmptySetArraySize, 16>::type s_theEmptySetArray;

extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyMarkedVArray;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyMarkedDArray;
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyMarkedVec;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyMarkedDictArray;

/*
 * Return the static empty array, for PHP and Hack arrays.
 *
 * These are singleton static arrays that can be used whenever an empty array
 * is needed. We should avoid using these methods, as these arrays don't have
 * provenance information; use ArrayData::CreateDArray and friends instead.
 */
ArrayData* staticEmptyVArray();
ArrayData* staticEmptyDArray();
ArrayData* staticEmptyVec();
ArrayData* staticEmptyDictArray();
ArrayData* staticEmptyKeysetArray();

/*
 * Static empty marked arrays; they're common enough (due to constant-folding)
 * that it's useful to keep a singleton value for them, too.
 */
ArrayData* staticEmptyMarkedVArray();
ArrayData* staticEmptyMarkedDArray();
ArrayData* staticEmptyMarkedVec();
ArrayData* staticEmptyMarkedDictArray();

/*
 * Call arr->decRefAndRelease().
 */
void decRefArr(ArrayData* arr);

size_t loadedStaticArrayCount();

///////////////////////////////////////////////////////////////////////////////

/*
 * Hand-built virtual dispatch table for array functions.
 *
 * Each field represents one virtual method with an array of function pointers,
 * one per ArrayKind.  There is one global instance of this table.
 *
 * Arranging it this way allows dispatch to be done with a single indexed load,
 * using kind as the index.
 */
struct ArrayFunctions {
  /*
   * NK stands for number of array kinds.
   */
  static auto const NK = size_t{ArrayData::kNumKinds};

  void (*release[NK])(ArrayData*);
  TypedValue (*nvGetInt[NK])(const ArrayData*, int64_t k);
  TypedValue (*nvGetStr[NK])(const ArrayData*, const StringData* k);
  ssize_t (*nvGetIntPos[NK])(const ArrayData*, int64_t k);
  ssize_t (*nvGetStrPos[NK])(const ArrayData*, const StringData* k);
  TypedValue (*getPosKey[NK])(const ArrayData*, ssize_t pos);
  TypedValue (*getPosVal[NK])(const ArrayData*, ssize_t pos);
  ArrayData* (*setInt[NK])(ArrayData*, int64_t k, TypedValue v);
  ArrayData* (*setIntMove[NK])(ArrayData*, int64_t k, TypedValue v);
  ArrayData* (*setStr[NK])(ArrayData*, StringData* k, TypedValue v);
  ArrayData* (*setStrMove[NK])(ArrayData*, StringData* k, TypedValue v);
  bool (*isVectorData[NK])(const ArrayData*);
  bool (*existsInt[NK])(const ArrayData*, int64_t k);
  bool (*existsStr[NK])(const ArrayData*, const StringData* k);
  arr_lval (*lvalInt[NK])(ArrayData*, int64_t k);
  arr_lval (*lvalStr[NK])(ArrayData*, StringData* k);
  ArrayData* (*removeInt[NK])(ArrayData*, int64_t k);
  ArrayData* (*removeStr[NK])(ArrayData*, const StringData* k);
  ssize_t (*iterBegin[NK])(const ArrayData*);
  ssize_t (*iterLast[NK])(const ArrayData*);
  ssize_t (*iterEnd[NK])(const ArrayData*);
  ssize_t (*iterAdvance[NK])(const ArrayData*, ssize_t pos);
  ssize_t (*iterRewind[NK])(const ArrayData*, ssize_t pos);
  ArrayData* (*escalateForSort[NK])(ArrayData*, SortFunction);
  void (*ksort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*sort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*asort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  bool (*uksort[NK])(ArrayData* ad, const Variant& cmp_function);
  bool (*usort[NK])(ArrayData* ad, const Variant& cmp_function);
  bool (*uasort[NK])(ArrayData* ad, const Variant& cmp_function);
  ArrayData* (*copy[NK])(const ArrayData*);
  ArrayData* (*copyStatic[NK])(const ArrayData*);
  ArrayData* (*append[NK])(ArrayData*, TypedValue v);
  ArrayData* (*merge[NK])(ArrayData*, const ArrayData* elems);
  ArrayData* (*pop[NK])(ArrayData*, Variant& value);
  ArrayData* (*dequeue[NK])(ArrayData*, Variant& value);
  ArrayData* (*prepend[NK])(ArrayData*, TypedValue v);
  ArrayData* (*renumber[NK])(ArrayData*);
  void (*onSetEvalScalar[NK])(ArrayData*);
  ArrayData* (*toDict[NK])(ArrayData*, bool);
  ArrayData* (*toVec[NK])(ArrayData*, bool);
  ArrayData* (*toKeyset[NK])(ArrayData*, bool);
  ArrayData* (*toVArray[NK])(ArrayData*, bool);
  ArrayData* (*toDArray[NK])(ArrayData*, bool);
};

extern const ArrayFunctions g_array_funcs;

///////////////////////////////////////////////////////////////////////////////
/*
 * Raise notices, warnings, and errors for array-related operations.
 */

[[noreturn]] void throwInvalidArrayKeyException(const TypedValue* key,
                                                const ArrayData* ad);
[[noreturn]] void throwInvalidArrayKeyException(const StringData* key,
                                                const ArrayData* ad);
[[noreturn]] void throwOOBArrayKeyException(TypedValue key,
                                            const ArrayData* ad);
[[noreturn]] void throwOOBArrayKeyException(int64_t key,
                                            const ArrayData* ad);
[[noreturn]] void throwOOBArrayKeyException(const StringData* key,
                                            const ArrayData* ad);
[[noreturn]] void throwFalseyPromoteException(const char* type);
[[noreturn]] void throwMissingElementException(const char* op);
[[noreturn]] void throwInvalidKeysetOperation();
[[noreturn]] void throwInvalidAdditionException(const ArrayData* ad);
[[noreturn]] void throwVarrayUnsetException();
[[noreturn]] void throwVecUnsetException();

void raiseHackArrCompatArrHackArrCmp();
void raiseHackArrCompatDVArrCmp(const ArrayData*, const ArrayData*, bool);

std::string makeHackArrCompatImplicitArrayKeyMsg(const TypedValue* key);

StringData* getHackArrCompatNullHackArrayKeyMsg();

bool checkHACCompare();

/*
 * Add a provenance tag for the current vmpc to `ad`, copying instead from
 * `src` if it's provided (and if it has a tag).  Returns `ad` for convenience.
 *
 * This function does not assert that `ad` does not have an existing tag, and
 * instead overrides it.
 */
ArrayData* tagArrProv(ArrayData* ad, const ArrayData* src = nullptr);
ArrayData* tagArrProv(ArrayData* ad, const APCArray* src);

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/array-data-inl.h"

