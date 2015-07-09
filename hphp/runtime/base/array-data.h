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

#include <folly/Likely.h>

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/cap-code.h"
#include "hphp/util/md5.h"

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
  // values. Also, all of the values need to be continuous from 0 to =
  // kNumKinds-1 since we use these values to index into a table.
  enum ArrayKind : uint8_t {
    kPackedKind = 0,  // PackedArray with keys in range [0..size)
    kStructKind = 1,  // StructArray with static string keys
    kMixedKind = 2,   // MixedArray arbitrary int or string keys, maybe holes
    kEmptyKind = 3,   // The singleton static empty array
    kApcKind = 4,     // APCLocalArray
    kGlobalsKind = 5, // GlobalsArray
    kProxyKind = 6,   // ProxyArray
    kNumKinds = 7     // insert new values before kNumKinds.
  };

protected:
  /*
   * NOTE: MixedArray no longer calls this constructor.  If you change
   * it, change the MixedArray::Make functions as appropriate.
   */
  explicit ArrayData(ArrayKind kind)
    : m_sizeAndPos(uint32_t(-1)) {
    m_hdr.init(static_cast<HeaderKind>(kind), 1);
    assert(m_size == -1);
    assert(m_pos == 0);
    assert(m_hdr.kind == static_cast<HeaderKind>(kind));
    assert(hasExactlyOneRef());
  }

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

  /**
   * Create a new ArrayData with specified array element(s).
   */
  static ArrayData *Create();
  static ArrayData *Create(const Variant& value);
  static ArrayData *Create(const Variant& name, const Variant& value);
  static ArrayData *CreateRef(Variant& value);
  static ArrayData *CreateRef(const Variant& name, Variant& value);

  /*
   * Called to return an ArrayData to the request heap.  This is
   * normally called when the reference count goes to zero (e.g. via a
   * helper like decRefArr).
   */
  void release() noexcept;

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
    return static_cast<ArrayKind>(m_hdr.kind);
  }

  /*
   * Return the capacity stored in the header. Not to be confused
   * with MixedArray::capacity
   */
  uint32_t cap() const {
    return m_hdr.aux.decode();
  }

  /**
   * Number of elements this array has.
   */
  size_t size() const {
    if (UNLIKELY((int)m_size) < 0) return vsize();
    return m_size;
  }

  // Unlike ArrayData::size(), this function doesn't delegate
  // to the vsize() function, so its more efficient to use this when
  // you know you don't have a GlobalsArray or ProxyArray.
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

  bool isPacked() const { return kind() == kPackedKind; }
  bool isStruct() const { return kind() == kStructKind; }
  bool isMixed() const { return kind() == kMixedKind; }
  bool isApcArray() const { return kind() == kApcKind; }
  bool isGlobalsArray() const { return kind() == kGlobalsKind; }
  bool isProxyArray() const { return kind() == kProxyKind; }
  bool isEmptyArray() const { return kind() == kEmptyKind; }

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

  ArrayData* escalateForSort(SortFunction sort_function);
  void ksort(int sort_flags, bool ascending);
  void sort(int sort_flags, bool ascending);
  void asort(int sort_flags, bool ascending);
  bool uksort(const Variant& cmp_function);
  bool usort(const Variant& cmp_function);
  bool uasort(const Variant& cmp_function);

  /**
   * Make a copy of myself.
   *
   * copyStatic() means not to use the request-scoped heap.
   * It is only implemented for array types that need to be able to go
   * into the static array list.
   */
  ArrayData* copy() const;
  ArrayData* copyWithStrongIterators() const;
  ArrayData* copyStatic() const;

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

  using ScalarArrayKey = MD5;
  struct ScalarHash {
    size_t operator()(const ScalarArrayKey& key) const {
      return key.hash();
    }
    size_t hash(const ScalarArrayKey& key) const {
      return key.hash();
    }
    bool equal(const ScalarArrayKey& k1,
               const ScalarArrayKey& k2) const {
      return k1 == k2;
    }
  };

  static ScalarArrayKey GetScalarArrayKey(ArrayData* arr);
  static ScalarArrayKey GetScalarArrayKey(const char* str, size_t sz);
  static ArrayData* GetScalarArray(ArrayData *arr);
  static ArrayData* GetScalarArray(ArrayData *arr, const ScalarArrayKey& key);

  static constexpr size_t offsetofSize() { return offsetof(ArrayData, m_size); }
  static constexpr size_t sizeofSize() { return sizeof(m_size); }

  static const char* kindToString(ArrayKind kind);

private:
  void serializeImpl(VariableSerializer *serializer) const;
  friend size_t getMemSize(const ArrayData*);
  static void compileTimeAssertions() {
    static_assert(offsetof(ArrayData, m_hdr) == HeaderOffset, "");
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
  friend struct StructArray;
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
      uint32_t m_size;
      int32_t m_pos;
    };
    uint64_t m_sizeAndPos; // careful, m_pos is signed
  };
  HeaderWord<CapCode> m_hdr;
};

static_assert(ArrayData::kPackedKind == uint8_t(HeaderKind::Packed), "");
static_assert(ArrayData::kStructKind == uint8_t(HeaderKind::Struct), "");
static_assert(ArrayData::kMixedKind == uint8_t(HeaderKind::Mixed), "");
static_assert(ArrayData::kEmptyKind == uint8_t(HeaderKind::Empty), "");
static_assert(ArrayData::kApcKind == uint8_t(HeaderKind::Apc), "");
static_assert(ArrayData::kGlobalsKind == uint8_t(HeaderKind::Globals), "");
static_assert(ArrayData::kProxyKind == uint8_t(HeaderKind::Proxy), "");

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
 * dispatch to be done with a single indexed load, using kind as the index.
 */
struct ArrayFunctions {
  // NK stands for number of array kinds; here just for shorthand.
  static auto const NK = size_t(ArrayData::ArrayKind::kNumKinds);
  void (*release[NK])(ArrayData*);
  const TypedValue* (*nvGetInt[NK])(const ArrayData*, int64_t k);
  const TypedValue* (*nvGetStr[NK])(const ArrayData*, const StringData* k);
  void (*nvGetKey[NK])(const ArrayData*, TypedValue* out, ssize_t pos);
  ArrayData* (*setInt[NK])(ArrayData*, int64_t k, Cell v, bool copy);
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
  ArrayData* (*escalateForSort[NK])(ArrayData*, SortFunction);
  void (*ksort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*sort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  void (*asort[NK])(ArrayData* ad, int sort_flags, bool ascending);
  bool (*uksort[NK])(ArrayData* ad, const Variant& cmp_function);
  bool (*usort[NK])(ArrayData* ad, const Variant& cmp_function);
  bool (*uasort[NK])(ArrayData* ad, const Variant& cmp_function);
  ArrayData* (*copy[NK])(const ArrayData*);
  ArrayData* (*copyWithStrongIterators[NK])(const ArrayData*);
  ArrayData* (*copyStatic[NK])(const ArrayData*);
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

extern ArrayFunctions g_array_funcs;
extern const ArrayFunctions g_array_funcs_unmodified;

ALWAYS_INLINE
void decRefArr(ArrayData* arr) {
  arr->decRefAndRelease();
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_DATA_H_
