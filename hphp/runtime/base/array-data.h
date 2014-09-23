/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <climits>
#include <vector>

#include "folly/Likely.h"

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;

struct ArrayData {
  // Runtime type tag of possible array types.  This is intentionally
  // not an enum class, since we're using it pretty much as raw bits
  // (these tag values are not private), which avoids boilerplate
  // when:
  //  - doing relational comparisons
  //  - using kind as an index
  //  - doing bit ops when storing in the union'd words below
  //
  // Beware if you change the order or the numerical values, as there are
  // a few places in the code that depends on the order or the numeric
  // values. Also, all of the values need to be continguous from 0 to =
  // kNumKinds-1 since we use these values to index into a table.
  enum ArrayKind : uint8_t {
    kPackedKind = 0,  // PackedArray with keys in range [0..size)
    kMixedKind = 1,   // MixedArray arbitrary int or string keys, maybe holes
    kStrMapKind = 2,  // StrMapArray, string keys, mixed values, like MixedArray
    kIntMapKind = 3,  // IntMapArray, int keys, maybe holes, like MixedArray
    kVPackedKind = 4, // PackedArray with extra warnings for certain operations
    kEmptyKind = 5,   // The singleton static empty array
    kSharedKind = 6,  // SharedArray
    kNvtwKind = 7,    // NameValueTableWrapper
    kProxyKind = 8,   // ProxyArray
    kNumKinds = 9     // insert new values before kNumKinds.
  };

protected:
  /*
   * NOTE: MixedArray no longer calls this constructor.  If you change
   * it, change the MixedArray::Make functions as appropriate.
   */
  explicit ArrayData(ArrayKind kind)
    : m_kind(kind)
    , m_size(-1)
    , m_pos(0)
    , m_count(0)
  {}

  /*
   * NOTE: MixedArray no longer calls this destructor.  If you need to
   * add logic, revisit MixedArray::Release{,Packed}.
   *
   * Include mixed-array-defs.h if you need the definition of this
   * destructor.  It is inline only.
   */
  ~ArrayData();

public:
  IMPLEMENT_COUNTABLE_METHODS
  void setRefCount(RefCount n) { m_count = n; }

  /**
   * Create a new ArrayData with specified array element(s).
   */
  static ArrayData *Create();
  static ArrayData *Create(const Variant& value);
  static ArrayData *Create(const Variant& name, const Variant& value);
  static ArrayData *CreateRef(Variant& value);
  static ArrayData *CreateRef(const Variant& name, Variant& value);

  /*
   * Called to return an ArrayData to the smart allocator.  This is
   * normally called when the reference count goes to zero (e.g. via a
   * helper like decRefArr).
   */
  void release();

  /**
   * Whether this array has any element.
   */
  bool empty() const {
    return size() == 0;
  }

  /**
   * return the array kind for fast typechecks
   */
  ArrayKind kind() const {
    return m_kind;
  }

  /**
   * Number of elements this array has.
   */
  size_t size() const {
    if (UNLIKELY((int)m_size) < 0) return vsize();
    return m_size;
  }

  // unlike ArrayData::size(), this functions doesn't delegate
  // to the vsize() function, so its more efficient to use this when
  // you know you don't have a NameValueTableWrapper or ProxyArray
  size_t getSize() const {
    return m_size;
  }

  /**
   * Number of elements this array has.
   */
  size_t vsize() const;

  /*
   * getValueRef() gets a reference to value at position "pos".  You
   * must not change the returned Variant.
   */
  const Variant& getValueRef(ssize_t pos) const;

  /*
   * Return true for array types that don't have COW semantics.
   */
  bool noCopyOnWrite() const;

  /*
   * Returns true if this is a PackedArray or a varray
   */
  bool isPacked() const {
    bool b = !(m_kind & ~uint8_t{4});
    assert(b == (m_kind == kPackedKind || m_kind == kVPackedKind));
    return b;
  }
  /*
   * Returns true if this is a MixedArray, msarray, or miarray
   */
  bool isMixed() const {
    bool b = ((uint64_t{m_kind} - uint64_t{1}) <= uint64_t{2});
    assert(b == (m_kind == kMixedKind || m_kind == kStrMapKind ||
                 m_kind == kIntMapKind));
    return b;
  }

  /*
   * These three functions can be used to test if this array is a varray,
   * msarray, or miarray respectively.
   */
  bool isVPackedArray() const { return m_kind == kVPackedKind; }
  bool isStrMapArray() const { return m_kind == kStrMapKind; }
  bool isIntMapArray() const { return m_kind == kIntMapKind; }

  /*
   * Returns true if this is any kind of "checked" array (varray, msarray,
   * or miarray).
   */
  bool isCheckedArray() const {
    bool b = ((uint64_t{m_kind} - uint64_t{2}) <= uint64_t{2});
    assert(b == (m_kind == kStrMapKind || m_kind == kIntMapKind ||
                 m_kind == kVPackedKind));
    return b;
  }

  /*
   * Returns true if this is a msarray or miarray.
   */
  bool isStrMapArrayOrIntMapArray() const {
    bool b = ((uint64_t{m_kind} - uint64_t{2}) <= uint64_t{1});
    assert(b == (m_kind == kStrMapKind || m_kind == kIntMapKind));
    return b;
  }
  /*
   * Returns true if this is a varray or miarray.
   */
  bool isVPackedArrayOrIntMapArray() const {
    bool b = ((uint64_t{m_kind} - uint64_t{3}) <= uint64_t{1});
    assert(b == (m_kind == kIntMapKind || m_kind == kVPackedKind));
    return b;
  }

  bool isSharedArray() const { return m_kind == kSharedKind; }
  bool isNameValueTableWrapper() const { return m_kind == kNvtwKind; }
  bool isProxyArray() const { return m_kind == kProxyKind; }

  /*
   * Returns whether or not this array contains "vector-like" data.
   * I.e. iteration order produces int keys 0 to m_size-1 in sequence.
   *
   * For non-Packed array types this is generally an O(N) operation
   * right now.
   */
  bool isVectorData() const;

  /**
   * Position-based iterations, implemented using iter_begin,
   * iter_advance, iter_prev, iter_rewind.
   */
  Variant reset();
  Variant prev();
  Variant current() const;
  Variant next();
  Variant end();
  Variant key() const;
  Variant value(int32_t &pos) const;
  Variant each();

  bool isHead()            const { return m_pos == iter_begin(); }
  bool isTail()            const { return m_pos == iter_last(); }
  bool isInvalid()         const { return m_pos == iter_end(); }

  /**
   * Testing whether a key exists.
   */
  bool exists(int64_t k) const;
  bool exists(const StringData* k) const;

  /*
   * Interface for VM helpers.  ArrayData implements generic versions
   * using the other ArrayData api; subclasses may customize methods either
   * by providing a custom static method in g_array_funcs.
   *
   * An old comment said: nvGetKey does not touch out->_count, so can
   * be used for inner or outer cells.  (It's unclear if anything is
   * relying on this, but try not to in new code.)
   */
  const TypedValue* nvGet(int64_t k) const;
  const TypedValue* nvGetConverted(int64_t k) const;
  const TypedValue* nvGet(const StringData* k) const;
  void nvGetKey(TypedValue* out, ssize_t pos) const;

  // wrappers that call getValueRef()
  Variant getValue(ssize_t pos) const;
  Variant getKey(ssize_t pos) const;

  /**
   * Getting l-value (that Variant pointer) at specified key. Return this if
   * escalation is not needed, or an escalated array data.
   */
  ArrayData *lval(int64_t k, Variant *&ret, bool copy);
  ArrayData *lval(StringData* k, Variant *&ret, bool copy);

  /**
   * Getting l-value (that Variant pointer) of a new element with the next
   * available integer key. Return this if escalation is not needed, or an
   * escalated array data. Note that adding a new element with the next
   * available integer key may fail, in which case ret is set to point to
   * the lval blackhole (see lvalBlackHole() for details).
   */
  ArrayData *lvalNew(Variant *&ret, bool copy);
  ArrayData *lvalNewRef(Variant *&ret, bool copy);

  /**
   * Setting a value at specified key. If "copy" is true, make a copy first
   * then set the value. Return this if escalation is not needed, or an
   * escalated array data.
   */
  ArrayData *set(int64_t k, const Variant& v, bool copy);
  ArrayData *setConverted(int64_t k, const Variant& v, bool copy);
  ArrayData *set(StringData* k, const Variant& v, bool copy);

  ArrayData *setRef(int64_t k, Variant& v, bool copy);
  ArrayData *setRef(StringData* k, Variant& v, bool copy);

  ArrayData* zSet(int64_t k, RefData* r);
  ArrayData* zSet(StringData* k, RefData* r);
  ArrayData* zAppend(RefData* r, int64_t* key_ptr);

  /**
   * The same as set(), but with the precondition that the key does
   * not already exist in this array.  (This is to allow more
   * efficient implementation of this case in some derived classes.)
   */
  ArrayData *add(int64_t k, const Variant& v, bool copy);
  ArrayData *add(StringData* k, const Variant& v, bool copy);

  /**
   * Remove a value at specified key. If "copy" is true, make a copy first
   * then remove the value. Return this if escalation is not needed, or an
   * escalated array data.
   */
  ArrayData *remove(int64_t k, bool copy);
  ArrayData *remove(const StringData* k, bool copy);

  /**
   * Inline accessors that convert keys to StringData* before delegating to
   * the virtual method.  Helpers that take a const Variant& key dispatch to either
   * the StringData* or int64_t key-type helpers.
   */
  bool exists(const String& k) const;
  bool exists(const Variant& k) const;
  const Variant& get(int64_t k, bool error = false) const;
  const Variant& get(const StringData* k, bool error = false) const;
  const Variant& get(const String& k, bool error = false) const;
  const Variant& get(const Variant& k, bool error = false) const;
  ArrayData *lval(const String& k, Variant *&ret, bool copy);
  ArrayData *lval(const Variant& k, Variant *&ret, bool copy);
  ArrayData *set(const String& k, const Variant& v, bool copy);
  ArrayData *set(const Variant& k, const Variant& v, bool copy);
  ArrayData *set(const StringData*, const Variant&, bool) = delete;
  ArrayData *setRef(const String& k, Variant& v, bool copy);
  ArrayData *setRef(const Variant& k, Variant& v, bool copy);
  ArrayData *setRef(const StringData*, Variant&, bool) = delete;
  ArrayData *add(const String& k, const Variant& v, bool copy);
  ArrayData *add(const Variant& k, const Variant& v, bool copy);
  ArrayData *remove(const String& k, bool copy);
  ArrayData *remove(const Variant& k, bool copy);

  // See the documentation for IterEnd, IterBegin, etc. in array-data.cpp
  ssize_t iter_begin() const;
  ssize_t iter_last() const;
  ssize_t iter_end() const;
  ssize_t iter_advance(ssize_t prev) const;
  ssize_t iter_rewind(ssize_t prev) const;

  /*
   * Checks if a mutable iterator points to a valid element within
   * this array.  The iterator must be associated with this array (see
   * array-iterator.cpp).  This will return false if the iterator
   * points past the last element, or if the iterator points before
   * the first element.
   */
  bool validMArrayIter(const MArrayIter& fp) const;

  /*
   * Advances the mutable iterator to the next element in the array.
   * The iterator must be associated with this array (see
   * array-iterator.cpp).  Returns false if the iterator has moved
   * past the last element, otherwise returns true.
   */
  bool advanceMArrayIter(MArrayIter& fp);

  ArrayData* escalateForSort();
  void ksort(int sort_flags, bool ascending);
  void sort(int sort_flags, bool ascending);
  void asort(int sort_flags, bool ascending);
  bool uksort(const Variant& cmp_function);
  bool usort(const Variant& cmp_function);
  bool uasort(const Variant& cmp_function);

  /**
   * Make a copy of myself.
   *
   * The nonSmartCopy() version means not to use the smart allocator.
   * Is only implemented for array types that need to be able to go
   * into the static array list.
   */
  ArrayData* copy() const;
  ArrayData* copyWithStrongIterators() const;
  ArrayData* nonSmartCopy() const;

  /**
   * Append a value to the array. If "copy" is true, make a copy first
   * then append the value. Return NULL if escalation is not needed, or an
   * escalated array data.
   */
  ArrayData* append(const Variant& v, bool copy);
  ArrayData* appendRef(Variant& v, bool copy);

  /**
   * Similar to append(v, copy), with reference in v preserved.
   */
  ArrayData* appendWithRef(const Variant& v, bool copy);

  ArrayData* plusEq(const ArrayData* elems);
  ArrayData* merge(const ArrayData* elems);

  /**
   * Stack function: pop the last item and return it.
   */
  ArrayData* pop(Variant &value);

  /**
   * Queue function: remove the 1st item and return it.
   */
  ArrayData* dequeue(Variant &value);

  /**
   * Array function: prepend a new item.
   */
  ArrayData* prepend(const Variant& v, bool copy);

  /**
   * Only map classes need this. Re-index all numeric keys to start from 0.
   */
  void renumber();

  void onSetEvalScalar();

  // TODO(#3903818): move serialization out of ArrayData, Variant, etc.
  void serialize(VariableSerializer *serializer,
                 bool skipNestCheck = false) const;

  /**
   * Comparisons.
   */
  int compare(const ArrayData *v2) const;
  bool equal(const ArrayData *v2, bool strict) const;

  void setPosition(int32_t p) {
    assert(m_pos == p || !isStatic());
    m_pos = p;
  }
  int32_t getPosition() const { return m_pos; }

  ArrayData *escalate() const;

  static ArrayData* GetScalarArray(ArrayData *arr);
  static ArrayData* GetScalarArray(ArrayData *arr, const std::string& key);

  static constexpr size_t offsetofKind() {
    return offsetof(ArrayData, m_kind);
  }

  static constexpr size_t offsetofSize() {
    return offsetof(ArrayData, m_size);
  }
  static constexpr size_t sizeofKind() { return sizeof(m_kind); }
  static constexpr size_t sizeofSize() { return sizeof(m_size); }

  static const char* kindToString(ArrayKind kind);

private:
  void serializeImpl(VariableSerializer *serializer) const;
  friend size_t getMemSize(const ArrayData*);
  static void compileTimeAssertions() {
    static_assert(offsetof(ArrayData, m_count) == FAST_REFCOUNT_OFFSET, "");
  }

protected:
  // error-handling helpers
  static const Variant& getNotFound(int64_t k);
  static const Variant& getNotFound(const StringData* k);
  const Variant& getNotFound(int64_t k, bool error) const;
  const Variant& getNotFound(const StringData* k, bool error) const;
  static const Variant& getNotFound(const String& k);
  static const Variant& getNotFound(const Variant& k);

  static bool IsValidKey(const String& k);
  static bool IsValidKey(const Variant& k);
  static bool IsValidKey(const StringData* k) { return k; }

protected:
  friend struct PackedArray;
  friend struct EmptyArray;
  friend struct MixedArray;
  friend class BaseVector;
  friend class c_Vector;
  friend class c_ImmVector;
  friend class HashCollection;
  friend class BaseMap;
  friend class c_Map;
  friend class c_ImmMap;
  // The following fields are blocked into unions with qwords so we
  // can combine the stores when initializing arrays.  (gcc won't do
  // this on its own.)
  union {
    struct {
      union {
        struct {
          UNUSED uint16_t m_unused1;
          UNUSED uint8_t m_unused0;
          ArrayKind m_kind;
        };
        // Packed arrays overlay their encoded capacity with the kind field.
        // kPackedKind is zero, and aliases the top byte of m_packedCapCode,
        // so it won't influence the encoded capacity. For details on the
        // encoding see the definition of packedCapToCode().
        uint32_t m_packedCapCode;
      };
      uint32_t m_size;
    };
    uint64_t m_kindAndSize;
  };
  union {
    struct {
      int32_t m_pos;
      mutable RefCount m_count;
    };
    uint64_t m_posAndCount;   // be careful, m_pos is signed
  };
};

//////////////////////////////////////////////////////////////////////

extern std::aligned_storage<
  sizeof(ArrayData),
  alignof(ArrayData)
>::type s_theEmptyArray;

/*
 * Return the "static empty array".  This is a singleton static array
 * that can be used whenever an empty array is needed.  It has
 * kEmptyKind and uses the functions in empty-array.cpp.
 */
ALWAYS_INLINE ArrayData* staticEmptyArray() {
  void* vp = &s_theEmptyArray;
  return static_cast<ArrayData*>(vp);
}

//////////////////////////////////////////////////////////////////////

/*
 * ArrayFunctions is a hand-built virtual dispatch table.  Each field represents
 * one virtual method with an array of function pointers, one per ArrayKind.
 * There is one global instance of this table.  Arranging it this way allows
 * dispatch to be done with a single indexed load, using m_kind as the index.
 */
struct ArrayFunctions {
  // NK stands for number of array kinds; here just for shorthand.
  static auto const NK = size_t(ArrayData::ArrayKind::kNumKinds);
  void (*release[NK])(ArrayData*);
  const TypedValue* (*nvGetInt[NK])(const ArrayData*, int64_t k);
  const TypedValue* (*nvGetIntConverted[NK])(const ArrayData*, int64_t k);
  const TypedValue* (*nvGetStr[NK])(const ArrayData*, const StringData* k);
  void (*nvGetKey[NK])(const ArrayData*, TypedValue* out, ssize_t pos);
  ArrayData* (*setInt[NK])(ArrayData*, int64_t k, Cell v, bool copy);
  ArrayData* (*setIntConverted[NK])(ArrayData*, int64_t k, Cell v, bool copy);
  ArrayData* (*setStr[NK])(ArrayData*, StringData* k, Cell v,
                           bool copy);
  size_t (*vsize[NK])(const ArrayData*);
  const Variant& (*getValueRef[NK])(const ArrayData*, ssize_t pos);
  bool (*isVectorData[NK])(const ArrayData*);
  bool (*existsInt[NK])(const ArrayData*, int64_t k);
  bool (*existsStr[NK])(const ArrayData*, const StringData* k);
  ArrayData* (*lvalInt[NK])(ArrayData*, int64_t k, Variant*& ret,
                            bool copy);
  ArrayData* (*lvalStr[NK])(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  ArrayData* (*lvalNew[NK])(ArrayData*, Variant *&ret, bool copy);
  ArrayData* (*lvalNewRef[NK])(ArrayData*, Variant *&ret, bool copy);
  ArrayData* (*setRefInt[NK])(ArrayData*, int64_t k, Variant& v, bool copy);
  ArrayData* (*setRefStr[NK])(ArrayData*, StringData* k, Variant& v, bool copy);
  ArrayData* (*addInt[NK])(ArrayData*, int64_t k, Cell v, bool copy);
  ArrayData* (*addStr[NK])(ArrayData*, StringData* k, Cell v, bool copy);
  ArrayData* (*removeInt[NK])(ArrayData*, int64_t k, bool copy);
  ArrayData* (*removeStr[NK])(ArrayData*, const StringData* k, bool copy);
  ssize_t (*iterBegin[NK])(const ArrayData*);
  ssize_t (*iterLast[NK])(const ArrayData*);
  ssize_t (*iterEnd[NK])(const ArrayData*);
  ssize_t (*iterAdvance[NK])(const ArrayData*, ssize_t pos);
  ssize_t (*iterRewind[NK])(const ArrayData*, ssize_t pos);
  bool (*validMArrayIter[NK])(const ArrayData*, const MArrayIter&);
  bool (*advanceMArrayIter[NK])(ArrayData*, MArrayIter&);
  ArrayData* (*escalateForSort[NK])(ArrayData*);
  void (*ksort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*sort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*asort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  bool (*uksort[NK])(ArrayData* ad, const Variant& cmp_function);
  bool (*usort[NK])(ArrayData* ad, const Variant& cmp_function);
  bool (*uasort[NK])(ArrayData* ad, const Variant& cmp_function);
  ArrayData* (*copy[NK])(const ArrayData*);
  ArrayData* (*copyWithStrongIterators[NK])(const ArrayData*);
  ArrayData* (*nonSmartCopy[NK])(const ArrayData*);
  ArrayData* (*append[NK])(ArrayData*, const Variant& v, bool copy);
  ArrayData* (*appendRef[NK])(ArrayData*, Variant& v, bool copy);
  ArrayData* (*appendWithRef[NK])(ArrayData*, const Variant& v, bool copy);
  ArrayData* (*plusEq[NK])(ArrayData*, const ArrayData* elems);
  ArrayData* (*merge[NK])(ArrayData*, const ArrayData* elems);
  ArrayData* (*pop[NK])(ArrayData*, Variant& value);
  ArrayData* (*dequeue[NK])(ArrayData*, Variant& value);
  ArrayData* (*prepend[NK])(ArrayData*, const Variant& value, bool copy);
  void (*renumber[NK])(ArrayData*);
  void (*onSetEvalScalar[NK])(ArrayData*);
  ArrayData* (*escalate[NK])(const ArrayData*);
  ArrayData* (*zSetInt[NK])(ArrayData*, int64_t k, RefData* v);
  ArrayData* (*zSetStr[NK])(ArrayData*, StringData* k, RefData* v);
  ArrayData* (*zAppend[NK])(ArrayData*, RefData* v, int64_t* key_ptr);
};

extern const ArrayFunctions g_array_funcs;

ALWAYS_INLINE
void decRefArr(ArrayData* arr) {
  arr->decRefAndRelease();
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_DATA_H_
