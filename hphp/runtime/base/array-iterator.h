/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/util/min-max-macros.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;
class c_Vector;
class c_Map;
class c_StableMap;
class c_Set;
class c_Pair;
class c_FrozenVector;
class c_FrozenSet;
struct Iter;

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
class ArrayIter {
 public:
  enum Type {
    TypeUndefined = 0,
    TypeArray,
    TypeIterator  // for objects that implement Iterator or
                  // IteratorAggregate
  };

  enum NoInc { noInc = 0 };
  enum NoIncNonNull { noIncNonNull = 0 };

  /**
   * Constructors.
   */
  ArrayIter() : m_pos(ArrayData::invalid_index) {
    m_data = nullptr;
  }
  explicit ArrayIter(const ArrayData* data);
  ArrayIter(const ArrayData* data, NoInc) {
    setArrayData(data);
    if (data) {
      m_pos = data->iter_begin();
    } else {
      m_pos = ArrayData::invalid_index;
    }
  }
  explicit ArrayIter(const HphpArray*) = delete;
  ArrayIter(const HphpArray* data, NoIncNonNull) {
    assert(data);
    setArrayData(data);
    m_pos = data->getIterBegin();
  }
  explicit ArrayIter(CArrRef array);
  explicit ArrayIter(ObjectData* obj);
  ArrayIter(ObjectData* obj, NoInc);
  explicit ArrayIter(CObjRef obj);
  explicit ArrayIter(const Cell& c);
  explicit ArrayIter(CVarRef v);

  // Copy ctor
  ArrayIter(const ArrayIter& iter);

  // Move ctor
  ArrayIter(ArrayIter&& iter) {
    m_data = iter.m_data;
    m_pos = iter.m_pos;
    m_version = iter.m_version;
    m_itype = iter.m_itype;
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
  bool end() {
    if (LIKELY(hasArrayData())) {
      return m_pos == ArrayData::invalid_index;
    }
    return endHelper();
  }
  bool endHelper();

  void next() {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      assert(ad);
      assert(m_pos != ArrayData::invalid_index);
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
      assert(m_pos != ArrayData::invalid_index);
      return ad->getKey(m_pos);
    }
    return firstHelper();
  }
  Variant firstHelper();
  void nvFirst(TypedValue* out) {
    const ArrayData* ad = getArrayData();
    assert(ad && m_pos != ArrayData::invalid_index);
    const_cast<ArrayData*>(ad)->nvGetKey(out, m_pos);
  }

  Variant second();
  CVarRef secondRef();
  CVarRef secondRefPlus();
  TypedValue* nvSecond() {
    const ArrayData* ad = getArrayData();
    assert(ad && m_pos != ArrayData::invalid_index);
    return const_cast<ArrayData*>(ad)->nvGetValueRef(m_pos);
  }
  /**
   * Used by the ext_zend_compat layer.
   * Identical to nvSecond but the output is boxed.
   */
  RefData* zSecond();

  bool hasArrayData() const {
    return !((intptr_t)m_data & 1);
  }
  bool hasCollection() {
    return (!hasArrayData() && getObject()->isCollection());
  }
  bool hasIteratorObj() {
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

  ObjectData* getObject() {
    assert(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~1);
  }

 private:
  void arrInit(const ArrayData* arr);

  template <bool incRef>
  void objInit(ObjectData* obj);

  void cellInit(const Cell& c);

  static void VectorInit(ArrayIter* iter, ObjectData* obj);
  static void MapInit(ArrayIter* iter, ObjectData* obj);
  static void StableMapInit(ArrayIter* iter, ObjectData* obj);
  static void SetInit(ArrayIter* iter, ObjectData* obj);
  static void PairInit(ArrayIter* iter, ObjectData* obj);
  static void FrozenVectorInit(ArrayIter* iter, ObjectData* obj);
  static void FrozenSetInit(ArrayIter* iter, ObjectData* obj);
  static void IteratorObjInit(ArrayIter* iter, ObjectData* obj);

  typedef void(*InitFuncPtr)(ArrayIter*,ObjectData*);
  static const InitFuncPtr initFuncTable[8];

  void destruct();

  c_Vector* getVector() {
    assert(hasCollection() && getCollectionType() == Collection::VectorType);
    return (c_Vector*)((intptr_t)m_obj & ~1);
  }
  c_Map* getMap() {
    assert(hasCollection() && getCollectionType() == Collection::MapType);
    return (c_Map*)((intptr_t)m_obj & ~1);
  }
  c_StableMap* getStableMap() {
    assert(hasCollection() && getCollectionType() == Collection::StableMapType);
    return (c_StableMap*)((intptr_t)m_obj & ~1);
  }
  c_Set* getSet() {
    assert(hasCollection() && getCollectionType() == Collection::SetType);
    return (c_Set*)((intptr_t)m_obj & ~1);
  }
  c_Pair* getPair() {
    assert(hasCollection() && getCollectionType() == Collection::PairType);
    return (c_Pair*)((intptr_t)m_obj & ~1);
  }
  c_FrozenVector* getFrozenVector() {
    assert(hasCollection() &&
           getCollectionType() == Collection::FrozenVectorType);

    return (c_FrozenVector*)((intptr_t)m_obj & ~1);
  }
  c_FrozenSet* getFrozenSet() {
    assert(hasCollection() && getCollectionType() == Collection::FrozenSetType);
    return (c_FrozenSet*)((intptr_t)m_obj & ~1);
  }
  Collection::Type getCollectionType() {
    ObjectData* obj = getObject();
    return obj->getCollectionType();
  }
  ObjectData* getIteratorObj() {
    assert(hasIteratorObj());
    return getObject();
  }

  void setArrayData(const ArrayData* ad) {
    assert((intptr_t(ad) & 1) == 0);
    m_data = ad;
  }
  void setObject(ObjectData* obj) {
    assert((intptr_t(obj) & 1) == 0);
    m_obj = (ObjectData*)((intptr_t)obj | 1);
  }

  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
 public:
  ssize_t m_pos;
 private:
  int m_version;
  Type m_itype;

  friend struct Iter;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * FullPos provides the necessary functionality for supporting "foreach by
 * reference" (also called "strong foreach"). Note that the runtime does not
 * use FullPos directly, but instead uses two classes derived from FullPos
 * (MutableArrayIter and MArrayIter).
 *
 * In the common case, a FullPos is bound to a variable (m_var) when it is
 * initialized. m_var points to an inner cell which points to the array to
 * iterate over. For certain use cases, a FullPos is instead bound directly to
 * an array which m_data points to.
 *
 * Foreach by reference is a pain. Iteration needs to be robust in the face of
 * two challenges: (1) the case where an element is unset during iteration, and
 * (2) the case where user code modifies the inner cell to be a different array
 * or a non-array value. In such cases, we should never crash and ideally when
 * an element is unset we should be able to keep track of where we are in the
 * array.
 *
 * FullPos works by "registering" itself with the array being iterated over.
 * The array maintains a linked list of the FullPos's actively iterating over
 * it. When an element is unset, the FullPos's that were pointing to that
 * element are moved back one position before the element is unset. Note that
 * it is possible for an iterator to point to the position before the first
 * element (this is what the "reset" flag is for). This dance allows FullPos to
 * keep track of where it is in the array even when elements are unset.
 *
 * FullPos has also has a m_container field to keep track of which array it has
 * "registered" itself with. By comparing the array pointed to by m_var with
 * the array pointed to by m_container, FullPos can detect if user code has
 * modified the inner cell to be a different array or a non-array value. When
 * this happens, the FullPos unregisters itself with the old array (pointed to
 * by m_container) and registers itself with the new array (pointed to
 * by m_var->m_data.parr) and resumes iteration at the position pointed to by
 * the new array's internal cursor (ArrayData::m_pos). If m_var points to a
 * non-array value, iteration terminates.
 */
class FullPos {
 protected:
  FullPos() : m_pos(0), m_container(NULL), m_next(NULL) {}

 public:
  void release() { delete this; }

  // Returns true if the iterator points past the last element (or if
  // it points before the first element)
  bool end() const;

  // Move the iterator forward one element
  bool advance();

  // Returns true if the iterator points to a valid element
  bool prepare();

  ArrayData* getArray() const {
    return hasVar() ? getData() : getAd();
  }

  bool hasVar() const {
    return m_var && !(intptr_t(m_var) & 3LL);
  }
  bool hasAd() const {
    return bool(intptr_t(m_data) & 1LL);
  }
  const Variant* getVar() const {
    assert(hasVar());
    return m_var;
  }
  ArrayData* getAd() const {
    assert(hasAd());
    return (ArrayData*)(intptr_t(m_data) & ~1LL);
  }
  void setVar(const Variant* val) {
    m_var = val;
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
  FullPos* getNext() const {
    return (FullPos*)(m_resetBits & ~1);
  }
  void setNext(FullPos* fp) {
    assert((intptr_t(fp) & 1) == 0);
    m_resetBits = intptr_t(fp) | intptr_t(getResetFlag());
  }
  bool getResetFlag() const {
    return m_resetBits & 1;
  }
  void setResetFlag(bool reset) {
    m_resetBits = intptr_t(getNext()) | intptr_t(reset);
  }

 protected:
  ArrayData* getData() const {
    assert(hasVar());
    return m_var->is(KindOfArray) ? m_var->getArrayData() : nullptr;
  }
  ArrayData* cowCheck();
  void escalateCheck();
  ArrayData* reregister();

  // m_var/m_data are used to keep track of the array that were are supposed
  // to be iterating over. The low bit is used to indicate whether we are using
  // m_var or m_data. A helper function getArray() is provided to retrieve the
  // array that this FullPos is supposed to be iterating over.
  union {
    const Variant* m_var;
    ArrayData* m_data;
  };
 public:
  // m_pos is an opaque value used by the array implementation to track the
  // current position in the array.
  ssize_t m_pos;
 private:
  // m_container keeps track of which array we're "registered" with. Normally
  // getArray() and m_container refer to the same array. However, the two may
  // differ in cases where user code has modified the inner cell to be a
  // different array or non-array value.
  ArrayData* m_container;
  // m_next is used so that multiple FullPos's iterating over the same array
  // can be chained together into a singly linked list. The low bit of m_next
  // is used to track the state of the "reset" flag.
  union {
    FullPos* m_next;
    intptr_t m_resetBits;
  };
};

/**
 * Range which visits each entry in a list of FullPos. Removing the
 * front element will crash but removing an already-visited element
 * or future element will work.
 */
class FullPosRange {
 public:
  explicit FullPosRange(FullPos* list) : m_fpos(list) {}
  FullPosRange(const FullPosRange& other) : m_fpos(other.m_fpos) {}
  bool empty() const { return m_fpos == 0; }
  FullPos* front() const { assert(!empty()); return m_fpos; }
  void popFront() { assert(!empty()); m_fpos = m_fpos->getNext(); }
 private:
  FullPos* m_fpos;
};

/**
 * MutableArrayIter is used internally within the HipHop runtime in several
 * places. Ideally, we should phase it out and use MArrayIter instead.
 */
class MutableArrayIter : public FullPos {
 public:
  MutableArrayIter(const Variant* var, Variant* key, Variant& val);
  MutableArrayIter(ArrayData* data, Variant* key, Variant& val);
  ~MutableArrayIter();

  bool advance();

 private:
  Variant* m_key;
  Variant* m_valp;
};

/**
 * MArrayIter is used by the VM to handle the MIter* instructions
 */
class MArrayIter : public FullPos {
 public:
  MArrayIter() { m_data = NULL; }
  explicit MArrayIter(const RefData* ref);
  explicit MArrayIter(ArrayData* data);
  ~MArrayIter();

  /**
   * It is only safe to call key() and val() if all of the following
   * conditions are met:
   *  1) The calls to key() and/or val() are immediately preceded by
   *     a call to advance(), prepare(), or end().
   *  2) The iterator points to a valid position in the array.
   */
  Variant key() {
    ArrayData* data = getArray();
    assert(data && data == getContainer());
    assert(!getResetFlag() && data->validFullPos(*this));
    return data->getKey(m_pos);
  }
  CVarRef val() {
    ArrayData* data = getArray();
    assert(data && data == getContainer());
    assert(!data->hasMultipleRefs() || data->noCopyOnWrite());
    assert(!getResetFlag());
    assert(data->validFullPos(*this));
    return data->getValueRef(m_pos);
  }

  friend struct Iter;
};

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

  static uint32_t funcOff() { return offsetof(CufIter, m_func); }
  static uint32_t ctxOff()  { return offsetof(CufIter, m_ctx); }
  static uint32_t nameOff() { return offsetof(CufIter, m_name); }
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
} __attribute__ ((aligned(16)));

bool interp_init_iterator(Iter* it, TypedValue* c1);
bool interp_init_iterator_m(Iter* it, TypedValue* v1);
bool interp_iter_next(Iter* it);
bool interp_iter_next_m(Iter* it);

int64_t new_iter_array(Iter* dest, ArrayData* arr, TypedValue* val);
template <bool withRef>
int64_t new_iter_array_key(Iter* dest, ArrayData* arr, TypedValue* val,
                           TypedValue* key);
int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);
int64_t iter_next(Iter* dest, TypedValue* val);
template <bool withRef>
int64_t iter_next_key(Iter* dest, TypedValue* val, TypedValue* key);


int64_t new_miter_array_key(Iter* dest, RefData* arr, TypedValue* val,
                           TypedValue* key);
int64_t new_miter_object(Iter* dest, RefData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);
int64_t new_miter_other(Iter* dest, RefData* data);
int64_t miter_next_key(Iter* dest, TypedValue* val, TypedValue* key);

ArrayIter getContainerIter(CVarRef v);
ArrayIter getContainerIter(CVarRef v, size_t& sz);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_ITERATOR_H_
