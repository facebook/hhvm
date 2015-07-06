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

#ifndef incl_HPHP_ARRAY_ITERATOR_H_
#define incl_HPHP_ARRAY_ITERATOR_H_

#include <array>
#include <cstdint>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/util/tls-pod-bag.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;
class BaseVector;
class BaseMap;
class BaseSet;
struct Iter;
struct MixedArray;

enum class IterNextIndex : uint16_t {
  ArrayPacked = 0,
  ArrayMixed,
  ArrayStruct,
  Array,
  Vector,
  ImmVector,
  Map,
  ImmMap,
  Set,
  ImmSet,
  Pair,
  Object,
};

/**
 * An iteration normally looks like this:
 *
 *   for (ArrayIter iter(data); iter; ++iter) {
 *     ...
 *   }
 */

/**
 * Iterator for an immutable array.
 */
struct ArrayIter {
  enum Type : uint16_t {
    TypeUndefined = 0,
    TypeArray,
    TypeIterator  // for objects that implement Iterator or
                  // IteratorAggregate
  };

  enum NoInc { noInc = 0 };
  enum NoIncNonNull { noIncNonNull = 0 };

  /*
   * Constructors.  Note that sometimes ArrayIter objects are created
   * without running their C++ constructor.  (See new_iter_array.)
   */
  ArrayIter() {
    m_data = nullptr;
  }
  explicit ArrayIter(const ArrayData* data);
  ArrayIter(const ArrayData* data, NoInc) {
    setArrayData(data);
    if (data) {
      m_pos = data->iter_begin();
    }
  }
  explicit ArrayIter(const MixedArray*) = delete;
  explicit ArrayIter(const Array& array);
  explicit ArrayIter(ObjectData* obj);
  ArrayIter(ObjectData* obj, NoInc);
  explicit ArrayIter(const Object& obj);
  explicit ArrayIter(Cell);
  explicit ArrayIter(const Variant& v);

  // Copy ctor
  ArrayIter(const ArrayIter& iter);

  // Move ctor
  ArrayIter(ArrayIter&& iter) noexcept {
    m_data = iter.m_data;
    m_pos = iter.m_pos;
    m_version = iter.m_version;
    m_itype = iter.m_itype;
    m_nextHelperIdx = iter.m_nextHelperIdx;
    iter.m_data = nullptr;
  }

  // Copy assignment
  ArrayIter& operator=(const ArrayIter& iter);

  // Move assignment
  ArrayIter& operator=(ArrayIter&& iter);

  // Destructor
  ~ArrayIter() {
    destruct();
  }

  void reset() {
    destruct();
    m_data = nullptr;
  }

  explicit operator bool() { return !end(); }
  void operator++() { next(); }
  bool end() const {
    if (LIKELY(hasArrayData())) {
      auto* ad = getArrayData();
      return !ad || m_pos == ad->iter_end();
      return endHelper();
    }
    return endHelper();
  }
  bool endHelper() const;

  void next() {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      assert(ad);
      assert(m_pos != ad->iter_end());
      m_pos = ad->iter_advance(m_pos);
      return;
    }
    nextHelper();
  }
  void nextHelper();

  Variant first() {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      assert(ad);
      assert(m_pos != ad->iter_end());
      return ad->getKey(m_pos);
    }
    return firstHelper();
  }
  Variant firstHelper();
  void nvFirst(TypedValue* out) {
    const ArrayData* ad = getArrayData();
    assert(ad && m_pos != ad->iter_end());
    const_cast<ArrayData*>(ad)->nvGetKey(out, m_pos);
  }

  /*
   * Retrieve the value at the current position.
   */
  Variant second();

  /*
   * Get a reference to the value for the current iterator position.
   * This function is strictly an optimization to second()---you must
   * not modify the Variant.
   *
   * note that secondRef() has slightly different behavior than
   * second() with regard to collection types.  Use secondRefPlus when
   * you need support for these cases.  And note that unlike second(),
   * secondRefPlus() will throw for non-collection types.
   */
  const Variant& secondRef();
  const Variant& secondRefPlus();

  // Inline version of secondRef.  Only for use in iterator helpers.
  const TypedValue* nvSecond() const {
    const ArrayData* ad = getArrayData();
    assert(ad && m_pos != ad->iter_end());
    return ad->getValueRef(m_pos).asTypedValue();
  }

  bool hasArrayData() const {
    return !((intptr_t)m_data & 1);
  }
  bool hasCollection() const {
    return (!hasArrayData() && getObject()->isCollection());
  }
  bool hasIteratorObj() const {
    return (!hasArrayData() && !getObject()->isCollection());
  }

  //
  // Specialized iterator for collections. Used via JIT
  //

  /**
   * Fixed is used for collections that are immutable in size.
   * Templatized Fixed functions expect the collection to implement
   * size() and get().
   * The key is the current position of the iterator.
   */
  enum class Fixed {};
  /**
   * Versionable is used for collections that are mutable and throw if
   * an insertion or deletion is made to the collection while iterating.
   * Templatized Versionable functions expect the collection to implement
   * size(), getVersion() and get().
   * The key is the current position of the iterator.
   */
  enum class Versionable {};
  /**
   * VersionableSparse is used for collections that are mutable and throw if
   * an insertion or deletion is made to the collection while iterating.
   * Moreover the collection elements are accessed via an iterator.
   * Templatized VersionableSparse functions expect the collection to implement
   * getVersion(), iter_begin(), iter_next(), iter_value(), iter_key(), and
   * iter_valid().
   */
  enum class VersionableSparse {};

  // Constructors
  template<class Tuplish>
  ArrayIter(Tuplish* coll, Fixed);
  template<class Vectorish>
  ArrayIter(Vectorish* coll, Versionable);
  template<class Mappish>
  ArrayIter(Mappish* coll, VersionableSparse);

  // iterator "next", "value", "key" functions
  template<class Tuplish>
  bool iterNext(Fixed);
  template<class Vectorish>
  bool iterNext(Versionable);
  template<class Mappish>
  bool iterNext(VersionableSparse);

  template<class Tuplish>
  Variant iterValue(Fixed);
  template<class Vectorish>
  Variant iterValue(Versionable);
  template<class Mappish>
  Variant iterValue(VersionableSparse);

  template<class Tuplish>
  Variant iterKey(Fixed);
  template<class Vectorish>
  Variant iterKey(Versionable);
  template<class Mappish>
  Variant iterKey(VersionableSparse);

  const ArrayData* getArrayData() const {
    assert(hasArrayData());
    return m_data;
  }
  ssize_t getPos() {
    return m_pos;
  }
  void setPos(ssize_t newPos) {
    m_pos = newPos;
  }
  Type getIterType() const {
    return m_itype;
  }
  void setIterType(Type iterType) {
    m_itype = iterType;
  }

  IterNextIndex getHelperIndex() {
    return m_nextHelperIdx;
  }

  ObjectData* getObject() const {
    assert(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~1);
  }

private:
  friend int64_t new_iter_array(Iter*, ArrayData*, TypedValue*);
  template<bool withRef>
  friend int64_t new_iter_array_key(Iter*, ArrayData*, TypedValue*,
                                    TypedValue*);

  void arrInit(const ArrayData* arr);

  template <bool incRef>
  void objInit(ObjectData* obj);

  void cellInit(Cell);

  static void VectorInit(ArrayIter* iter, ObjectData* obj);
  static void MapInit(ArrayIter* iter, ObjectData* obj);
  static void ImmMapInit(ArrayIter* iter, ObjectData* obj);
  static void SetInit(ArrayIter* iter, ObjectData* obj);
  static void PairInit(ArrayIter* iter, ObjectData* obj);
  static void ImmVectorInit(ArrayIter* iter, ObjectData* obj);
  static void ImmSetInit(ArrayIter* iter, ObjectData* obj);
  static void IteratorObjInit(ArrayIter* iter, ObjectData* obj);

  typedef void(*InitFuncPtr)(ArrayIter*,ObjectData*);
  static const InitFuncPtr initFuncTable[];

  void destruct();

  void setArrayData(const ArrayData* ad) {
    assert((intptr_t(ad) & 1) == 0);
    m_data = ad;
    m_nextHelperIdx = IterNextIndex::ArrayMixed;
    if (ad != nullptr) {
      if (ad->isPacked()) {
        m_nextHelperIdx = IterNextIndex::ArrayPacked;
      } else if (!ad->isMixed()) {
        m_nextHelperIdx = IterNextIndex::Array;
      }
    }
  }

  void setObject(ObjectData* obj) {
    assert((intptr_t(obj) & 1) == 0);
    m_obj = (ObjectData*)((intptr_t)obj | 1);
    m_nextHelperIdx = getNextHelperIdx(obj);
  }
  IterNextIndex getNextHelperIdx(ObjectData* obj);

  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
 public:
  // m_pos is used by the array implementation to track the current position
  // in the array. Beware that when m_data is null, m_pos is uninitialized.
  ssize_t m_pos;
 private:
  int m_version;
  // This is unioned so new_iter_array can initialize it more
  // efficiently.
  union {
    struct {
      Type m_itype;
      IterNextIndex m_nextHelperIdx;
    };
    uint32_t m_itypeAndNextHelperIdx;
  };

  friend struct Iter;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * MArrayIter provides the necessary functionality for supporting
 * "foreach by reference" (also called "strong foreach").
 *
 * In the common case, a MArrayIter is bound to a RefData when it is
 * initialized.  When iterating objects with foreach by reference, a
 * MArrayIter may instead be bound directly to an array which m_data
 * points to.  (This is because the array is created as a temporary.)
 *
 * Foreach by reference is a pain.  Iteration needs to be robust in
 * the face of two challenges: (1) the case where an element is unset
 * during iteration, and (2) the case where user code modifies the
 * RefData to be a different array or a non-array value.  In such
 * cases, we should never crash and ideally when an element is unset
 * we should be able to keep track of where we are in the array.
 *
 * MArrayIter works by "registering" itself with the array being
 * iterated over, in a way that any array can find out all active
 * MArrayIters associated with it (if any).  See tl_miter_table below.
 *
 * Using this association, when an array mutation occurs, if there are
 * active MArrayIters the array will update them to ensure they behave
 * coherently.  For example, if an element is unset, the MArrayIter's
 * that were pointing to that element are moved to point to the
 * element before the element being unset.
 *
 * Note that it is possible for an iterator to point to the position
 * before the first element (this is what the "reset" flag is for).
 *
 * MArrayIter has also has a m_container field to keep track of which
 * array it has "registered" itself with.  By comparing the array
 * pointed to through m_ref with the array pointed to by m_container,
 * MArrayIter can detect if user code has modified the inner cell to
 * be a different array or a non-array value.  When this happens, the
 * MArrayIter unregisters itself with the old array (pointed to by
 * m_container) and registers itself with the new array (pointed to by
 * m_ref->tv().m_data.parr) and resumes iteration at the position
 * pointed to by the new array's internal cursor (ArrayData::m_pos).
 * If m_ref points to a non-array value, iteration terminates.
 */
struct MArrayIter {
  MArrayIter()
    : m_data(nullptr)
    , m_pos(0)
    , m_container(nullptr)
    , m_resetFlag(false)
  {}

  explicit MArrayIter(RefData* ref);
  explicit MArrayIter(ArrayData* data);
  ~MArrayIter();

  MArrayIter(const MArrayIter&) = delete;
  MArrayIter& operator=(const MArrayIter&) = delete;

  /*
   * It is only safe to call key() and val() if all of the following
   * conditions are met:
   *  1) The calls to key() and/or val() are immediately preceded by
   *     a call to advance(), prepare(), or end().
   *  2) The iterator points to a valid position in the array.
   */
  Variant key() {
    ArrayData* data = getArray();
    assert(data && data == getContainer());
    assert(!getResetFlag() && data->validMArrayIter(*this));
    return data->getKey(m_pos);
  }

  Variant& val() {
    ArrayData* data = getArray();
    assert(data && data == getContainer());
    assert(!data->hasMultipleRefs() || data->noCopyOnWrite());
    assert(!getResetFlag());
    assert(data->validMArrayIter(*this));
    // Normally it's not ok to modify the return value of getValueRef,
    // but the whole point of mutable array iteration is that this is
    // allowed, so this const_cast is not actually evil.
    return const_cast<Variant&>(data->getValueRef(m_pos));
  }

  void release() { delete this; }

  // Returns true if the iterator points past the last element (or if
  // it points before the first element)
  bool end() const;

  // Move the iterator forward one element
  bool advance();

  // Returns true if the iterator points to a valid element
  bool prepare();

  ArrayData* getArray() const {
    return hasRef() ? getData() : getAd();
  }

  bool hasRef() const {
    return m_ref && !(intptr_t(m_ref) & 1LL);
  }
  bool hasAd() const {
    return bool(intptr_t(m_data) & 1LL);
  }
  RefData* getRef() const {
    assert(hasRef());
    return m_ref;
  }
  ArrayData* getAd() const {
    assert(hasAd());
    return (ArrayData*)(intptr_t(m_data) & ~1LL);
  }
  void setRef(RefData* ref) {
    m_ref = ref;
  }
  void setAd(ArrayData* val) {
    m_data = (ArrayData*)(intptr_t(val) | 1LL);
  }
  ArrayData* getContainer() const {
    return m_container;
  }
  void setContainer(ArrayData* arr) {
    m_container = arr;
  }

  bool getResetFlag() const { return m_resetFlag; }
  void setResetFlag(bool reset) { m_resetFlag = reset; }

private:
  ArrayData* getData() const {
    assert(hasRef());
    return m_ref->tv()->m_type == KindOfArray
      ? m_ref->tv()->m_data.parr
      : nullptr;
  }

  ArrayData* cowCheck();
  void escalateCheck();
  ArrayData* reregister();

private:
  /*
   * m_ref/m_data are used to keep track of the array that we're
   * supposed to be iterating over. The low bit is used to indicate
   * whether we are using m_ref or m_data.
   *
   * Mutable array iteration usually iterates over m_ref---the m_data
   * case here occurs is when we've converted an object to an array
   * before iterating it (and this MArrayIter object actually owns a
   * temporary array).
   */
  union {
    RefData* m_ref;
    ArrayData* m_data;
  };
public:
  // m_pos is used by the array implementation to track the current position
  // in the array.
  ssize_t m_pos;
private:
  // m_container keeps track of which array we're "registered" with. Normally
  // getArray() and m_container refer to the same array. However, the two may
  // differ in cases where user code has modified the inner cell to be a
  // different array or non-array value.
  ArrayData* m_container;
  // The m_resetFlag is used to indicate a mutable array iterator is
  // "before the first" position in the array.
  UNUSED uint32_t m_unused;
  uint32_t m_resetFlag;
};

//////////////////////////////////////////////////////////////////////

/*
 * Active mutable iterators are associated with their corresponding
 * arrays using a table in thread local storage.  The iterators
 * themselves can find their registered container using their
 * m_container pointer, but arrays must linearly search this table to
 * go the other direction.
 *
 * This scheme is optimized for the overwhelmingly common case that
 * there are no active mutable array iterations in the whole request.
 * When there are active mutable iterators, it is also overwhelmingly
 * the case that there is only one, and on real applications exceeding
 * 4 or 5 simultaneously is apparently rare.
 *
 * This table has the following semantics:
 *
 *   o If there are any 'active' MArrayIters (i.e. ones that are
 *     actually associated with arrays), one of them will be present
 *     in the first Ent slot in this table.  This is so that any array
 *     can check that there are no active MArrayIters just by
 *     comparing the first slot of this table with null.  (See
 *     strong_iterators_exist().)
 *
 *   o Secondly we expect that we essentially never exceed a small
 *     number iterators (outside of code specifically designed to
 *     stress mutable array iteration).  We've chosen 7 preallocated
 *     slots because it fills out two cache lines, and we've observed
 *     4 or 5 occasionally in some real programs.  If there are
 *     actually more live than 7, we allocate additional space and
 *     point to it with 'extras'.
 *
 *   o The entries in this table (including 'extras') are not
 *     guaranteed to be contiguous.  Empty entries may be present in
 *     the middle, and there is no ordering.
 *
 *   o If an entry has a non-null array pointer, it must have a
 *     non-null iter pointer.  Checking either one for null are both
 *     valid ways to check if a slot is empty.
 */
struct MIterTable {
  struct Ent { ArrayData* array; MArrayIter* iter; };

  void clear() {
    ents.fill({nullptr, nullptr});
    if (!extras.empty()) {
      extras.release_if([] (const MIterTable::Ent& e) { return true; });
    }
  }

  std::array<Ent,7> ents;
  // Slow path: we expect this `extras' list to rarely be allocated.
  TlsPodBag<Ent,req::Allocator<Ent>> extras;
};
static_assert(sizeof(MIterTable) == 2*64, "");
extern __thread MIterTable tl_miter_table;

void free_strong_iterators(ArrayData*);
ArrayData* move_strong_iterators(ArrayData* dest, ArrayData* src);

//////////////////////////////////////////////////////////////////////

class CufIter {
 public:
  CufIter() : m_func(nullptr), m_ctx(nullptr), m_name(nullptr) {}
  ~CufIter();
  const Func* func() const { return m_func; }
  void* ctx() const { return m_ctx; }
  StringData* name() const { return m_name; }

  void setFunc(const Func* f) { m_func = f; }
  void setCtx(ObjectData* obj) { m_ctx = obj; }
  void setCtx(const Class* cls) {
    m_ctx = cls ? (void*)((char*)cls + 1) : nullptr;
  }
  void setName(StringData* name) { m_name = name; }

  static constexpr uint32_t funcOff() { return offsetof(CufIter, m_func); }
  static constexpr uint32_t ctxOff()  { return offsetof(CufIter, m_ctx); }
  static constexpr uint32_t nameOff() { return offsetof(CufIter, m_name); }
 private:
  const Func* m_func;
  void* m_ctx;
  StringData* m_name;
};

struct Iter {
  const ArrayIter&   arr() const { return m_u.aiter; }
  const MArrayIter& marr() const { return m_u.maiter; }
  const CufIter&     cuf() const { return m_u.cufiter; }
        ArrayIter&   arr()       { return m_u.aiter; }
        MArrayIter& marr()       { return m_u.maiter; }
        CufIter&     cuf()       { return m_u.cufiter; }

  bool init(TypedValue* c1);
  bool next();
  void free();
  void mfree();
  void cfree();

private:
  union Data {
    Data() {}
    ArrayIter aiter;
    MArrayIter maiter;
    CufIter cufiter;
  } m_u;
} __attribute__ ((__aligned__(16)));

//////////////////////////////////////////////////////////////////////

int64_t new_iter_array(Iter* dest, ArrayData* arr, TypedValue* val);
template <bool withRef>
int64_t new_iter_array_key(Iter* dest, ArrayData* arr, TypedValue* val,
                           TypedValue* key);
int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);
int64_t witer_next_key(Iter* dest, TypedValue* val, TypedValue* key);


int64_t new_miter_array_key(Iter* dest, RefData* arr, TypedValue* val,
                           TypedValue* key);
int64_t new_miter_object(Iter* dest, RefData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);
int64_t new_miter_other(Iter* dest, RefData* data);
int64_t miter_next_key(Iter* dest, TypedValue* val, TypedValue* key);

int64_t iter_next_ind(Iter* iter, TypedValue* valOut);
int64_t iter_next_key_ind(Iter* iter, TypedValue* valOut, TypedValue* keyOut);

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_ARRAY_ITERATOR_H_
