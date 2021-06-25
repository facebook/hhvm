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

struct Array;
struct MakeUncountedEnv;
struct String;
struct StringData;
struct Variant;

namespace bespoke {
  struct LoggingArray;
  struct MonotypeVec;

  // Type-safe layout index, so that we can't mix it up with other ints.
  struct LayoutIndex {
    bool operator==(LayoutIndex o) const { return raw == o.raw; }
    bool operator!=(LayoutIndex o) const { return raw != o.raw; }
    bool operator<(LayoutIndex o) const { return raw < o.raw; }

    // We use the high byte of the index for type tests and virtual dispatch.
    uint8_t byte() const { return raw >> 8; }

  public:
    uint16_t raw;
  };
}

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
    kVecKind,
    kBespokeVecKind,
    kDictKind,
    kBespokeDictKind,
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
   * Indicates that this array has a side table that contains information
   * about its keys (which must all be static strings).
   */
  static auto constexpr kHasStrKeyTable = 4;

  /*
   * Indicates that this array-like was sampled for bespoke logging. Set for
   * arrays produced by Hack constructors - e.g. Vec, NewStructDict - but not
   * for arrays produced by native constructors - e.g. builtins, varargs.
   */
  static auto constexpr kSampledArray = 8;

  /*
   * See notes on the m_layout_index field for constraints on this value.
   */
  static auto constexpr kVanillaLayoutIndex = bespoke::LayoutIndex { 0xffff };

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
  static ArrayData* CreateVec(bool legacy = false);
  static ArrayData* CreateDict(bool legacy = false);
  static ArrayData* CreateKeyset();

  /*
   * Convert between array kinds.
   */
  ArrayData* toVec(bool copy);
  ArrayData* toDict(bool copy);
  ArrayData* toKeyset(bool copy);
  ArrayData* toDictIntishCast(bool copy);

  /*
   * Return the array to the request heap or the global heap.
   *
   * The counted variant is called when the refcount goes to zero (e.g. from
   * the JIT or from a helper like decRefArr). The uncounted variant is called
   * when the refcount goes to "uncounted zero", from DecRefUncounted.
   */
  void release() DEBUG_NOEXCEPT;
  void releaseUncounted();

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
   * Whether the array has a particular type.
   */
  bool isVecType() const;
  bool isDictType() const;
  bool isKeysetType() const;

  /*
   * Whether the array has a particular type *and layout*.
   */
  bool isVanillaVec() const;
  bool isVanillaDict() const;
  bool isVanillaKeyset() const;

  /*
   * Whether the array-like has the standard layout for its type.
   * This check excludes array-likes with a "bespoke" hidden-class layout.
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
   * Whether the array has legacy behaviors enabled. This method can only be
   * called for dvarrays, vecs and dicts.
   *
   * The default setter has the normal copy/escalation behavior. If it returns
   * a new ArrayData, the caller must dec-ref the old one. The in-place setter
   * may only be called if the array is known to have exactly one ref.
   */
  bool isLegacyArray() const;
  ArrayData* setLegacyArray(bool copy, bool legacy);
  void setLegacyArrayInPlace(bool legacy);

  bool hasStrKeyTable() const;

  bool isSampledArray() const;
  void setSampledArrayInPlace();
  ArrayData* makeSampledStaticArray() const;

  /*
   * Get the aux bits in the header that must be preserved
   * when we copy or resize the array
   */
  uint8_t auxBits() const;

  /*
   * Whether the array contains "vector-like" data---i.e., iteration order
   * produces int keys 0 to size() - 1 in sequence.
   *
   * For non-hasPackedLayout() arrays, this is generally an O(N) operation.
   */
  bool isVectorData() const;

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
   * Get the value or key for the element at raw position `pos'. This op
   * never does any ref-counting on the key.
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
   * Get the value of the element at key `k'. Returns an Uninit TypedValue if
   * the key `k` is missing from the array.
   */
  TypedValue get(int64_t k) const;
  TypedValue get(const StringData* k) const;

  /*
   * Get the value of the element at key `k'. Throws if `k` is missing.
   */
  TypedValue getThrow(int64_t k) const;
  TypedValue getThrow(const StringData* k) const;

  /*
   * Get the value of the element at key `k'.
   *
   * If `error` is false, get returns an Uninit TypedValue if `k` is missing.
   * If `error` is true, get throws if `k` is missing.
   */
  TypedValue get(int64_t k, bool error) const;
  TypedValue get(const StringData* k, bool error) const;
  TypedValue get(TypedValue k, bool error = false) const;
  TypedValue get(const String& k, bool error = false) const;
  TypedValue get(const Variant& k, bool error = false) const;

  /*
   * Set the element at key `k' to `v'. `v' must be a TInitCell.
   *
   * Semantically, setMove() methods 1) do a set, 2) dec-ref the value, and
   * 3) if the operation required copy/escalation, dec-ref the old array. This
   * sequence is needed for member ops and can be implemented more efficiently
   * if done as a single unit.
   *
   * These methods return `this' if copy/escalation are not needed, or a
   * copied/escalated array data if they are.
   */
  ArrayData* setMove(int64_t k, TypedValue v);
  ArrayData* setMove(StringData* k, TypedValue v);

  ArrayData* setMove(TypedValue k, TypedValue v);
  ArrayData* setMove(const String& k, TypedValue v);
  ArrayData* setMove(int64_t k, const Variant& v);
  ArrayData* setMove(StringData* k, const Variant& v);
  ArrayData* setMove(const String& k, const Variant& v);
  ArrayData* setMove(const Variant& k, const Variant& v);

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
   *
   * As with setMove, `v' must be a TInitCell.
   *
   * appendMove dec-refs the old array if we needed copy / escalation, and
   * does not do any refcounting ops on the value.
   */
  ArrayData* appendMove(TypedValue v);

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
   * Remove the first or last element of the array, and assign it to `value'.
   * Return a copied/escalated array if necessary, or `this` otherwise.
   */
  ArrayData* pop(Variant& value);

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
  // Shared arrays: static and uncounted.

  /*
   * If `arr' points to a static array, do nothing.  Otherwise, make a static
   * copy, destroy the original, and update `*arr`.
   *
   * If `tag` is set or `arr` has provenance data, we copy the tag to the new
   * static array.  (A set `tag` overrides the provenance of `arr`.)
   */
  static void GetScalarArray(ArrayData** arr);

  /*
   * Promote the array referenced by `arr` to a static array and return it.
   */
  static ArrayData* GetScalarArray(Array&& arr);
  static ArrayData* GetScalarArray(Variant&& arr);

  /*
   * Static-ify the contents of the array.
   */
  void onSetEvalScalar();

  /*
   * Return a new uncounted version of the given array. The array must be a
   * refcounted array - otherwise, we should call persistentIncRef() instead.
   */
  ArrayData* makeUncounted(const MakeUncountedEnv& env, bool hasApcTv);

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
   * Return whether `key' should undergo intish-cast.  If true, `i' is set to
   * the intish value of `key'.
   */
  static bool IntishCastKey(const StringData* key, int64_t& i);

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
  static constexpr size_t offsetofExtra() { return offsetof(ArrayData, m_extra); }
  static constexpr size_t sizeofExtra() { return sizeof(m_extra); }

  static constexpr size_t offsetOfBespokeIndex() {
    return offsetof(ArrayData, m_layout_index);
  }

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

  /*
   * Make a copy of the array. Only for internal use. To make a static array,
   * we convert its contents static values, then copy it to static memory.
   */
  ArrayData* copyStatic() const;

  /////////////////////////////////////////////////////////////////////////////

protected:
  friend struct BespokeArray;
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
  friend struct bespoke::LoggingArray;
  friend struct bespoke::MonotypeVec;

  uint32_t m_size;

  /*
   * m_layout_index identifies the array's layout. For vanilla arrays, it's
   * always kVanillaLayoutIndex. For bespoke arrays, it's the index of some
   * concrete bespoke::Layout hidden class.
   *
   * m_extra_lo8, m_extra_hi8, and m_extra_lo16 are for use by each concrete
   * array class. We don't constrain the values in these fields in any way.
   *
   * m_extra is provided for fast initialization of all of these fields.
   */
  union {
    struct {
      /* NB the names are definitely little-endian centric but whatever */
      union {
        struct {
          uint8_t m_extra_lo8;
          uint8_t m_extra_hi8;
        };
        uint16_t m_extra_lo16;
      };
      bespoke::LayoutIndex m_layout_index;
    };
    uint32_t m_extra;
  };
};

static_assert(ArrayData::kVecKind == uint8_t(HeaderKind::Vec));
static_assert(ArrayData::kDictKind == uint8_t(HeaderKind::Dict));
static_assert(ArrayData::kKeysetKind == uint8_t(HeaderKind::Keyset));
static_assert(ArrayData::kBespokeVecKind == uint8_t(HeaderKind::BespokeVec));
static_assert(ArrayData::kBespokeDictKind == uint8_t(HeaderKind::BespokeDict));
static_assert(ArrayData::kBespokeKeysetKind == uint8_t(HeaderKind::BespokeKeyset));

//////////////////////////////////////////////////////////////////////

// The size of the StrKeyTable, which is stored in front of the array, needs to
// rounded up to a multiple of 16, so that we can enforce the base array pointer
// is 16-byte aligned.
constexpr size_t kEmptyMixedArrayStrKeyTableSize =
  ((sizeof(StrKeyTable) - 1) / 16 + 1) * 16;

constexpr size_t kEmptyMixedArraySize = 120 + kEmptyMixedArrayStrKeyTableSize;
constexpr size_t kEmptySetArraySize = 96;

/*
 * Storage for the static empty arrays.
 */
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVec;
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVArray;
extern std::aligned_storage<kEmptySetArraySize, 16>::type s_theEmptySetArray;

extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyMarkedVArray;
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyMarkedVec;

/*
 * Pointers to canonical empty Dicts/DArrays.
 */
extern ArrayData* s_theEmptyDictArrayPtr;
extern ArrayData* s_theEmptyDArrayPtr;
extern ArrayData* s_theEmptyMarkedDArrayPtr;
extern ArrayData* s_theEmptyMarkedDictArrayPtr;

/*
 * Return the static empty array, for PHP and Hack arrays.
 *
 * These are singleton static arrays that can be used whenever an empty array
 * is needed. We should avoid using these methods, as these arrays don't have
 * provenance information; use ArrayData::CreateDArray and friends instead.
 */
ArrayData* staticEmptyVec();
ArrayData* staticEmptyDictArray();
ArrayData* staticEmptyKeysetArray();

/*
 * Static empty marked arrays; they're common enough (due to constant-folding)
 * that it's useful to keep a singleton value for them, too.
 */
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
  void (*releaseUncounted[NK])(ArrayData*);
  TypedValue (*nvGetInt[NK])(const ArrayData*, int64_t k);
  TypedValue (*nvGetStr[NK])(const ArrayData*, const StringData* k);
  TypedValue (*getPosKey[NK])(const ArrayData*, ssize_t pos);
  TypedValue (*getPosVal[NK])(const ArrayData*, ssize_t pos);
  ArrayData* (*setIntMove[NK])(ArrayData*, int64_t k, TypedValue v);
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
  ArrayData* (*copyStatic[NK])(const ArrayData*);
  ArrayData* (*appendMove[NK])(ArrayData*, TypedValue v);
  ArrayData* (*pop[NK])(ArrayData*, Variant& value);
  void (*onSetEvalScalar[NK])(ArrayData*);
  ArrayData* (*makeUncounted[NK])(
    ArrayData*, const MakeUncountedEnv& env, bool hasApcTv);
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
[[noreturn]] void throwInvalidKeysetOperation();
[[noreturn]] void throwReadOnlyCollectionMutation();
[[noreturn]] void throwVarrayUnsetException();
[[noreturn]] void throwVecUnsetException();

void raiseHackArrCompatArrHackArrCmp();

std::string makeHackArrCompatImplicitArrayKeyMsg(const TypedValue* key);

StringData* getHackArrCompatNullHackArrayKeyMsg();

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/array-data-inl.h"
