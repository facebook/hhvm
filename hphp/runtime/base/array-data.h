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

#ifndef incl_HPHP_ARRAY_DATA_H_
#define incl_HPHP_ARRAY_DATA_H_

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/sort-flags.h"
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
struct String;
struct StringData;
struct RefData;
struct VariableSerializer;
struct Variant;

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
    kPackedKind = 0,  // PackedArray with keys in range [0..size)
    kMixedKind = 1,   // MixedArray arbitrary int or string keys, maybe holes
    kEmptyKind = 2,   // The singleton static empty array
    kApcKind = 3,     // APCLocalArray
    kGlobalsKind = 4, // GlobalsArray
    kShapeKind = 5,   // Shape
    kDictKind = 6,    // Hack dict
    kVecKind = 7,     // Hack vec
    kKeysetKind = 8,  // Hack keyset
    kNumKinds = 9     // insert new values before kNumKinds.
  };

  /*
   * A secondary array kind axis for PHP arrays.
   *
   * We use darrays and varrays in place of regular arrays for arrays that we
   * want to replace with Hack arrays.  These arrays will emit notices whenever
   * we attempt to perform operations on them which would be illegal for Hack
   * arrays.
   */
  enum DVArray : uint8_t {
    kNotDVArray = 0,
    kVArray     = 1,
    kDArray     = 2
  };
  static auto constexpr kDVArrayMask = static_cast<DVArray>(3);
  /*
   * For uncounted Packed, Mixed, Dict and Vec, indicates that the
   * array was co-allocated with an APCTypedValue (at apctv+1).
   */
  static auto constexpr kHasApcTv    = 4;

  /*
   * Indicates that this dict or vec should use some legacy (i.e.,
   * PHP-compatible) behaviors, including serialization
   */
  static auto constexpr kLegacyArray = 8;

  /////////////////////////////////////////////////////////////////////////////
  // Creation and destruction.

protected:
  explicit ArrayData(ArrayKind kind, RefCount initial_count = OneReference);

  /*
   * We can't `= delete` this because we subclass ArrayData.
   */
  ~ArrayData() { always_assert(false); }

public:
  /*
   * Create a new empty ArrayData with the appropriate ArrayKind.
   */
  static ArrayData* Create();
  static ArrayData* CreateVec();
  static ArrayData* CreateDict();
  static ArrayData* CreateShape();
  static ArrayData* CreateKeyset();
  static ArrayData* CreateVArray();
  static ArrayData* CreateDArray();

  /*
   * Create a new kPackedKind ArrayData with a single element, `value'.
   *
   * Unboxes `value' and initializes it if it's UninitNull.
   */
  static ArrayData* Create(TypedValue value);
  static ArrayData* Create(const Variant& value);

  /*
   * Create a new kMixedKind ArrayData with a single key `name' and value
   * `value'.
   *
   * Unboxes `value' and initializes it if it's UninitNull.
   */
  static ArrayData* Create(TypedValue name, TypedValue value);
  static ArrayData* Create(const Variant& name, TypedValue value);
  static ArrayData* Create(const Variant& name, const Variant& value);

  /*
   * Like Create(name, value), but preserves reffiness unless `value' is
   * singly-referenced.
   */
  static ArrayData* CreateWithRef(TypedValue name, TypedValue value);
  static ArrayData* CreateWithRef(const Variant& name, TypedValue value);

  /*
   * Like Create(value) or Create(name, value), except `value' is boxed before
   * insertion.
   */
  static ArrayData* CreateRef(Variant& value);
  static ArrayData* CreateRef(TypedValue name, tv_lval value);
  static ArrayData* CreateRef(const Variant& name, tv_lval value);

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
  ArrayData* toShape(bool copy);
  ArrayData* toDict(bool copy);
  ArrayData* toVec(bool copy);
  ArrayData* toKeyset(bool copy);
  ArrayData* toVArray(bool copy);
  ArrayData* toDArray(bool copy);

  /*
   * Converts this to a Shape in place if this is a compatible type with Shapes.
   * If this is not refCounted it will instead make a copy before converting
   * to a Shape.
   *
   * Dicts are compatible when RuntimeOption::EvalHackArrDVArrs is set and
   * DArrays are compatible when it is not set. Empty arrays are always
   * compatible. No other types are compatible.
   *
   * This function will return staticEmptyShapeArray when this is empty and it
   * will return this otherwise.
   */
  ArrayData* toShapeInPlaceIfCompatible();

  /*
   * Return an array with identical contents to this array, but of an array
   * kind which can handle all array operations.
   *
   * Certain array kinds (e.g., APCLocalArray) can't handle all operations
   * (e.g., binding modifications), and such operations either automagically
   * escalate, or require escalation as a precondition.
   *
   * If the array is already of a kind that can handle all operations,
   * escalate() is guaranteed to return `this'.
   */
  ArrayData* escalate() const;

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
   * Number of elements.
   *
   * vsize() is the slow path for size() that always calls through the array
   * function table.
   */
  size_t size() const;
  size_t vsize() const;

  /*
   * Fast-path number of elements.
   *
   * Only valid for arrays that aren't GlobalsArray.
   */
  size_t getSize() const;

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
  bool isPacked() const;
  bool isMixed() const;
  bool isMixedOrShape() const;
  bool isApcArray() const;
  bool isGlobalsArray() const;
  bool isEmptyArray() const;
  bool isDict() const;
  bool isDictOrShape() const;
  bool isShape() const;
  bool isVecArray() const;
  bool isKeyset() const;

  /*
   * Whether the ArrayData is backed by PackedArray or MixedArray.
   */
  bool hasPackedLayout() const;
  bool hasMixedLayout() const;

  /*
   * Whether the array is a PHP (non-Hack) or Hack array.
   */
  bool isPHPArray() const;
  bool isHackArray() const;

  /*
   * The DVArray kind for the array.
   */
  DVArray dvArray() const;
  void setDVArray(DVArray);

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

  /* Get the aux bits in the header that must be preserved
   * when we copy or resize the array
   */
  uint8_t auxBits() const;

  /*
   * Is the array a varray, darray, or neither?
   */
  bool isVArray() const;
  bool isDArray() const;
  bool isNotDVArray() const;
  bool isVecOrVArray() const;
  bool isDictOrDArray() const;
  bool isDictOrDArrayOrShape() const;

  static bool dvArrayEqual(const ArrayData* a, const ArrayData* b);

  /*
   * Check whether the array has an sane DVArray setting for its kind.
   */
  bool dvArraySanityCheck() const;

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
  bool notCyclic(Cell v) const;

  /*
   * Should int-like string keys be implicitly converted to integers before
   * they are inserted?
   */
  bool useWeakKeys() const;

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
  bool exists(Cell k) const;
  bool exists(const String& k) const;
  bool exists(const Variant& k) const;

  /*
   * Get an lval for the element at key `k'.
   *
   * The lvalRef() variant should be used when the caller might box the
   * returned lval.
   */
  arr_lval lval(int64_t k, bool copy);
  arr_lval lval(StringData* k, bool copy);
  arr_lval lval(Cell k, bool copy);
  arr_lval lval(const String& k, bool copy);
  arr_lval lval(const Variant& k, bool copy);
  arr_lval lvalRef(int64_t k, bool copy);
  arr_lval lvalRef(StringData* k, bool copy);
  arr_lval lvalRef(Cell k, bool copy);
  arr_lval lvalRef(const String& k, bool copy);
  arr_lval lvalRef(const Variant& k, bool copy);

  /*
   * Get an lval for a new element at the next available integer key.
   *
   * Note that adding a new element with the next available integer key may
   * fail, in which case we return the lval blackhole (see lvalBlackHole() for
   * details).
   */
  arr_lval lvalNew(bool copy);
  arr_lval lvalNewRef(bool copy);

  /*
   * Get an rval for the element at key `k'.
   *
   * If the array has no element at `k', return a null tv_rval.
   */
  tv_rval rval(int64_t k) const;
  tv_rval rval(const StringData* k) const;

  /*
   * Like rval(), except throws an exception instead if `k' is out of bounds
   * and the array is a Hack array.
   */
  tv_rval rvalStrict(int64_t k) const;
  tv_rval rvalStrict(const StringData* k) const;

  /*
   * Get an rval for the element at raw position `pos'.
   *
   * @requires: `pos' refers to a valid array element.
   */
  tv_rval rvalPos(ssize_t pos) const;

  /*
   * Get the value of the element at key `k'.
   *
   * @requires: exists(k)
   */
  TypedValue at(int64_t k) const;
  TypedValue at(const StringData* k) const;
  TypedValue at(Cell k) const;

  /*
   * Get the value or key for the element at raw position `pos'.
   *
   * @requires: `pos' refers to a valid array element.
   */
  TypedValue atPos(ssize_t pos) const;
  Cell nvGetKey(ssize_t pos) const;

  /*
   * Variant wrappers around atPos() and nvGetKey().
   */
  Variant getValue(ssize_t pos) const;
  Variant getKey(ssize_t pos) const;

  /*
   * Get the value of the element at key `k'.
   *
   * This behaves like `error ? rvalStrict(k) : rval(k)`, except if the
   * resultant rval !has_val(), we raise a notice and return a dummy rval
   * instead.
   */
  tv_rval get(Cell k, bool error = false) const;
  tv_rval get(int64_t k, bool error = false) const;
  tv_rval get(const StringData* k, bool error = false) const;
  tv_rval get(const String& k, bool error = false) const;
  tv_rval get(const Variant& k, bool error = false) const;

  /*
   * Set the element at key `k' to `v'. set() methods make a copy first if
   * cowCheck() returns true. If `v' is a ref, its inner value is used.
   *
   * setInPlace() methods attempt to modify the array in place without
   * calling cowCheck(), but may still return a new array for escalation
   * or reallocating to grow.
   *
   * Return `this' if copy/escalation are not needed, or a copied/escalated
   * array data.
   */
  ArrayData* set(int64_t k, Cell v);
  ArrayData* set(StringData* k, Cell v);
  ArrayData* set(Cell k, Cell v);
  ArrayData* set(const String& k, Cell v);
  ArrayData* setInPlace(int64_t k, Cell v);
  ArrayData* setInPlace(StringData* k, Cell v);
  ArrayData* setInPlace(Cell k, Cell v);
  ArrayData* setInPlace(const String& k, Cell v);

  ArrayData* set(int64_t k, const Variant& v);
  ArrayData* set(StringData* k, const Variant& v);
  ArrayData* set(const String& k, const Variant& v);
  ArrayData* set(const Variant& k, const Variant& v);
  ArrayData* setInPlace(StringData* k, const Variant& v);
  ArrayData* setInPlace(const String& k, const Variant& v);
  ArrayData* setInPlace(const Variant& k, const Variant& v);

  ArrayData* set(const StringData*, Cell) = delete;
  ArrayData* set(const StringData*, const Variant&) = delete;
  ArrayData* setInPlace(const StringData*, Cell) = delete;
  ArrayData* setInPlace(const StringData*, const Variant&) = delete;

  /*
   * Like set(), except the reffiness of `v' is preserved unless it is
   * singly-referenced.
   */
  ArrayData* setWithRef(int64_t k, TypedValue v);
  ArrayData* setWithRef(StringData* k, TypedValue v);
  ArrayData* setWithRef(Cell k, TypedValue v);
  ArrayData* setWithRef(const String& k, TypedValue v);
  ArrayData* setWithRef(const StringData*, TypedValue) = delete;
  ArrayData* setWithRefInPlace(Cell k, TypedValue v);
  ArrayData* setWithRefInPlace(const String& k, TypedValue v);
  ArrayData* setWithRefInPlace(StringData* k, TypedValue v);
  ArrayData* setWithRefInPlace(int64_t k, TypedValue v);

  /*
   * Remove the value at key `k'. remove() will make a copy first if necesary;
   * removeInPlace() will never copy, but may escalate.
   *
   * Return `this' if copy/escalation are not needed, or a copied/escalated
   * array data.
   */
  ArrayData* remove(int64_t k);
  ArrayData* remove(const StringData* k);
  ArrayData* remove(Cell k);
  ArrayData* remove(const String& k);
  ArrayData* remove(const Variant& k);
  ArrayData* removeInPlace(int64_t k);
  ArrayData* removeInPlace(const StringData* k);

  /**
   * Append `v' to the array. append() makes a copy first if `copy' is set,
   * appendInPlace() may only escalate or grow.
   *
   * Return `this' if copy/escalation are not needed, or a copied/escalated
   * array data.
   */
  ArrayData* append(Cell v);
  ArrayData* appendInPlace(Cell v);

  /*
   * Like append(), except the reffiness of `v' is preserved unless it is
   * singly-referenced.
   */
  ArrayData* appendWithRef(TypedValue v);
  ArrayData* appendWithRefInPlace(TypedValue v);
  ArrayData* appendWithRef(const Variant& v);

  /////////////////////////////////////////////////////////////////////////////
  // Iteration.

  /*
   * Get the position of the array's internal cursor.
   */
  int32_t getPosition() const;

  /*
   * Set the array's internal cursor to raw position `p'.
   */
  void setPosition(int32_t p);

  /*
   * @see: array-data.cpp, for documentation for IterEnd, IterBegin, etc.
   */
  ssize_t iter_begin() const;
  ssize_t iter_last() const;
  ssize_t iter_end() const;
  ssize_t iter_advance(ssize_t prev) const;
  ssize_t iter_rewind(ssize_t prev) const;

  /*
   * Get the value or key currently referenced by the arrays' internal cursor.
   *
   * If the cursor is invalid, return:
   *  - current(): false
   *  - key(): uninit null
   */
  Variant current() const;
  Variant key() const;

  /*
   * Reset the array's internal cursor to the first or last element and return
   * its value.
   *
   * Return false if the array is empty.
   */
  Variant reset();
  Variant end();

  /*
   * Rewind or advance the array's internal cursor, then return the value it
   * points to.
   *
   * Return false if the cursor is or becomes invalid.
   */
  Variant prev();
  Variant next();

  /*
   * Like getValue(), except if `pos' is specifically the canonical invalid
   * position (i.e., iter_end()), return false.
   */
  Variant value(int32_t pos) const;

  /*
   * Return a 4-element array with keys 0 and "key" set to key() and keys 1 and
   * "value" set to current(), then advance the array's internal cursor.
   *
   * If the cursor is invalid, return false.
   */
  Variant each();

  /*
   * Is the array's internal cursor pointing to...
   *
   *  - Head: the first element?
   *  - Tail: the last element?
   *  - Invalid: the canonical invalid position?
   */
  bool isHead() const;
  bool isTail() const;
  bool isInvalid() const;

  /////////////////////////////////////////////////////////////////////////////
  // PHP array functions.

  /*
   * Spiritually similar to escalate(), but for sort functions.
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
   * PHP += and array_merge() implementations.
   */
  ArrayData* plusEq(const ArrayData* elems);
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
  ArrayData* prepend(Cell v);

  /*
   * Comparisons.
   */
  int compare(const ArrayData* v2) const;
  bool equal(const ArrayData* v2, bool strict) const;

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
   * If arr points to a static array, do nothing. Otherwise, make a
   * static copy, destroy the original and update arr.
   */
  static void GetScalarArray(ArrayData** arr);
  /* Promote the array referenced by arr to a static array, and return it */
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
  template <IntishCast IC = IntishCast::None>
  bool convertKey(const StringData* key, int64_t& i) const;

  /*
   * Re-index all numeric keys to start from 0.
   */
  void renumber();

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
  static constexpr size_t offsetofDVArray() {
    return offsetof(ArrayData, m_aux16);
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

  /////////////////////////////////////////////////////////////////////////////

protected:
  /*
   * Raise a notice that `k' is undefined, and return an Uninit.
   */
  static tv_rval getNotFound(int64_t k);
  static tv_rval getNotFound(const StringData* k);

  /*
   * Raise a notice that `k' is undefined if `error' is set (and if this is not
   * the globals array), and return an Uninit.
   */
  tv_rval getNotFound(int64_t k, bool error) const;
  tv_rval getNotFound(const StringData* k, bool error) const;

  /*
   * Is `k' of an arraykey type (i.e., int or string)?
   */
  static bool IsValidKey(Cell k);
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

  // The following fields are blocked into unions with qwords so we
  // can combine the stores when initializing arrays.  (gcc won't do
  // this on its own.)
  union {
    struct {
      uint32_t m_size;
      int32_t m_pos;
    };
    uint64_t m_sizeAndPos; // careful, m_pos is signed
  };
};

static_assert(ArrayData::kPackedKind == uint8_t(HeaderKind::Packed), "");
static_assert(ArrayData::kMixedKind == uint8_t(HeaderKind::Mixed), "");
static_assert(ArrayData::kEmptyKind == uint8_t(HeaderKind::Empty), "");
static_assert(ArrayData::kApcKind == uint8_t(HeaderKind::Apc), "");
static_assert(ArrayData::kGlobalsKind == uint8_t(HeaderKind::Globals), "");
static_assert(ArrayData::kShapeKind == uint8_t(HeaderKind::Shape), "");
static_assert(ArrayData::kDictKind == uint8_t(HeaderKind::Dict), "");
static_assert(ArrayData::kVecKind == uint8_t(HeaderKind::VecArray), "");

//////////////////////////////////////////////////////////////////////

constexpr size_t kEmptyMixedArraySize = 120;
constexpr size_t kEmptySetArraySize = 96;

/*
 * Storage for the static empty arrays.
 */
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyArray;
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVecArray;
extern std::aligned_storage<sizeof(ArrayData), 16>::type s_theEmptyVArray;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDictArray;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyDArray;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type
  s_theEmptyShapeDArray;
extern std::aligned_storage<kEmptyMixedArraySize, 16>::type s_theEmptyShapeDict;
extern std::aligned_storage<kEmptySetArraySize, 16>::type s_theEmptySetArray;

/*
 * Return the static empty array, for PHP and Hack arrays.
 *
 * These are singleton static arrays that can be used whenever an empty array
 * is needed.  staticEmptyArray() has kEmptyKind, and the others have the
 * corresponding Hack array kind.
 */
ArrayData* staticEmptyArray();
ArrayData* staticEmptyVArray();
ArrayData* staticEmptyDArray();
ArrayData* staticEmptyVecArray();
ArrayData* staticEmptyDictArray();
ArrayData* staticEmptyShapeArray();
ArrayData* staticEmptyKeysetArray();

/*
 * Call arr->decRefAndRelease().
 */
void decRefArr(ArrayData* arr);

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
  tv_rval (*nvGetInt[NK])(const ArrayData*, int64_t k);
  tv_rval (*nvTryGetInt[NK])(const ArrayData*, int64_t k);
  tv_rval (*nvGetStr[NK])(const ArrayData*, const StringData* k);
  tv_rval (*nvTryGetStr[NK])(const ArrayData*, const StringData* k);
  Cell (*nvGetKey[NK])(const ArrayData*, ssize_t pos);
  ArrayData* (*setInt[NK])(ArrayData*, int64_t k, Cell v);
  ArrayData* (*setIntInPlace[NK])(ArrayData*, int64_t k, Cell v);
  ArrayData* (*setStr[NK])(ArrayData*, StringData* k, Cell v);
  ArrayData* (*setStrInPlace[NK])(ArrayData*, StringData* k, Cell v);
  ArrayData* (*setWithRefInt[NK])(ArrayData*, int64_t k, TypedValue v);
  ArrayData* (*setWithRefIntInPlace[NK])(ArrayData*, int64_t k, TypedValue v);
  ArrayData* (*setWithRefStr[NK])(ArrayData*, StringData* k, TypedValue v);
  ArrayData* (*setWithRefStrInPlace[NK])(ArrayData*, StringData* k,
                                         TypedValue v);
  size_t (*vsize[NK])(const ArrayData*);
  tv_rval (*nvGetPos[NK])(const ArrayData*, ssize_t pos);
  bool (*isVectorData[NK])(const ArrayData*);
  bool (*existsInt[NK])(const ArrayData*, int64_t k);
  bool (*existsStr[NK])(const ArrayData*, const StringData* k);
  arr_lval (*lvalInt[NK])(ArrayData*, int64_t k, bool copy);
  arr_lval (*lvalIntRef[NK])(ArrayData*, int64_t k, bool copy);
  arr_lval (*lvalStr[NK])(ArrayData*, StringData* k, bool copy);
  arr_lval (*lvalStrRef[NK])(ArrayData*, StringData* k, bool copy);
  arr_lval (*lvalNew[NK])(ArrayData*, bool copy);
  arr_lval (*lvalNewRef[NK])(ArrayData*, bool copy);
  ArrayData* (*removeInt[NK])(ArrayData*, int64_t k);
  ArrayData* (*removeIntInPlace[NK])(ArrayData*, int64_t k);
  ArrayData* (*removeStr[NK])(ArrayData*, const StringData* k);
  ArrayData* (*removeStrInPlace[NK])(ArrayData*, const StringData* k);
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
  ArrayData* (*append[NK])(ArrayData*, Cell v);
  ArrayData* (*appendInPlace[NK])(ArrayData*, Cell v);
  ArrayData* (*appendWithRef[NK])(ArrayData*, TypedValue v);
  ArrayData* (*appendWithRefInPlace[NK])(ArrayData*, TypedValue v);
  ArrayData* (*plusEq[NK])(ArrayData*, const ArrayData* elems);
  ArrayData* (*merge[NK])(ArrayData*, const ArrayData* elems);
  ArrayData* (*pop[NK])(ArrayData*, Variant& value);
  ArrayData* (*dequeue[NK])(ArrayData*, Variant& value);
  ArrayData* (*prepend[NK])(ArrayData*, Cell v);
  void (*renumber[NK])(ArrayData*);
  void (*onSetEvalScalar[NK])(ArrayData*);
  ArrayData* (*escalate[NK])(const ArrayData*);
  ArrayData* (*toPHPArray[NK])(ArrayData*, bool);
  ArrayData* (*toPHPArrayIntishCast[NK])(ArrayData*, bool);
  ArrayData* (*toShape[NK])(ArrayData*, bool);
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
[[noreturn]] void throwRefInvalidArrayValueException(const ArrayData* ad);
[[noreturn]] void throwRefInvalidArrayValueException(const Array& arr);
[[noreturn]] void throwInvalidKeysetOperation();
[[noreturn]] void throwInvalidAdditionException(const ArrayData* ad);
[[noreturn]] void throwVecUnsetException();

void raiseHackArrCompatRefBind(int64_t);
void raiseHackArrCompatRefBind(const StringData*);
void raiseHackArrCompatRefBind(TypedValue);
void raiseHackArrCompatRefNew();
void raiseHackArrCompatRefIter();

void raiseHackArrCompatAdd();

void raiseHackArrCompatArrMixedCmp();
void raiseHackArrCompatDVArrCmp(const ArrayData*, const ArrayData*);

void raiseHackArrCompatMissingIncDec();
void raiseHackArrCompatMissingSetOp();

std::string makeHackArrCompatImplicitArrayKeyMsg(const TypedValue* key);
void raiseHackArrCompatImplicitArrayKey(const TypedValue* key);

bool checkHACRefBind();
bool checkHACFalseyPromote();
bool checkHACEmptyStringPromote();
bool checkHACCompare();
bool checkHACArrayPlus();
bool checkHACArrayKeyCast();

/*
 * Like isStrictlyInteger() but changes behavior with the value of `ic'.
 */
template <IntishCast IC = IntishCast::None>
folly::Optional<int64_t> tryIntishCast(const StringData* key);

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/array-data-inl.h"

#endif // incl_HPHP_ARRAY_DATA_H_
