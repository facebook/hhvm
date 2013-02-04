/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_ARRAY_ITERATOR_H__
#define __HPHP_ARRAY_ITERATOR_H__

#include <runtime/base/types.h>
#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/array/hphp_array.h>
#include <util/min_max_macros.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;
class c_Vector;
class c_Map;
class c_StableMap;
namespace VM {
  struct Iter;
}

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

  /**
   * Constructors.
   */
  ArrayIter();
  ArrayIter(const ArrayData* data);

  enum NoInc { noInc = 0 };
  // Special constructor used by the VM. This constructor does not increment
  // the refcount of the specified array.
  ArrayIter(const ArrayData* data, NoInc) {
    setArrayData(data);
    if (data) {
      m_pos = data->iter_begin();
    } else {
      m_pos = ArrayData::invalid_index;
    }
  }
  // This is also a special constructor used by the VM. This constructor
  // doesn't increment the array's refcount and assumes that the array is not
  // empty.
  enum NoIncNonNull { noIncNonNull = 0 };
  ArrayIter(const HphpArray* data, NoIncNonNull) {
    assert(data);
    setArrayData(data);
    m_pos = data->getIterBegin();
  }
  ArrayIter(CArrRef array);
  void begin(CVarRef map, CStrRef);
  void begin(CArrRef map, CStrRef);
  void reset();

 private:
  // not defined.
  // Either use ArrayIter(const ArrayData*) or
  //            ArrayIter(const HphpArray*, NoIncNonNull)
  ArrayIter(const HphpArray*);
  template <bool incRef>
  void objInit(ObjectData* obj);

 public:
  ArrayIter(ObjectData* obj);
  ArrayIter(ObjectData* obj, NoInc);
  enum TransferOwner { transferOwner };
  ArrayIter(Object& obj, TransferOwner);

  ~ArrayIter();

  operator bool() { return !end(); }
  void operator++() { next(); }

  bool end() {
    if (LIKELY(hasArrayData())) {
      return m_pos == ArrayData::invalid_index;
    }
    return endHelper();
  }
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

  Variant first() {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      assert(ad);
      assert(m_pos != ArrayData::invalid_index);
      return ad->getKey(m_pos);
    }
    return firstHelper();
  }
  Variant second();
  void second(Variant &v) {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      assert(ad);
      assert(m_pos != ArrayData::invalid_index);
      v = ad->getValueRef(m_pos);
      return;
    }
    secondHelper(v);
  }
  CVarRef secondRef();

  void nvFirst(TypedValue* out) {
    const ArrayData* ad = getArrayData();
    assert(ad && m_pos != ArrayData::invalid_index);
    const_cast<ArrayData*>(ad)->nvGetKey(out, m_pos);
  }
  TypedValue* nvSecond() {
    const ArrayData* ad = getArrayData();
    assert(ad && m_pos != ArrayData::invalid_index);
    return const_cast<ArrayData*>(ad)->nvGetValueRef(m_pos);
  }

  bool hasArrayData() {
    return !((intptr_t)m_data & 1);
  }

 private:
  bool hasVector() {
    return (!hasArrayData() &&
            getRawObject()->getCollectionType() == Collection::VectorType);
  }
  bool hasMap() {
    return (!hasArrayData() &&
            getRawObject()->getCollectionType() == Collection::MapType);
  }
  bool hasStableMap() {
    return (!hasArrayData() &&
            getRawObject()->getCollectionType() == Collection::StableMapType);
  }
  bool hasObject() {
    return (!hasArrayData() &&
            getRawObject()->getCollectionType() == Collection::InvalidType);
  }
 public:
  const ArrayData* getArrayData() {
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
 private:
  c_Vector* getVector() {
    assert(hasVector());
    return (c_Vector*)((intptr_t)m_obj & ~1);
  }
  c_Map* getMap() {
    assert(hasMap());
    return (c_Map*)((intptr_t)m_obj & ~1);
  }
  c_StableMap* getStableMap() {
    assert(hasStableMap());
    return (c_StableMap*)((intptr_t)m_obj & ~1);
  }
  ObjectData* getObject() {
    assert(hasObject());
    return (ObjectData*)((intptr_t)m_obj & ~1);
  }
  ObjectData* getRawObject() {
    assert(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~1);
  }

  void setArrayData(const ArrayData* ad) {
    assert((intptr_t(ad) & 1) == 0);
    m_data = ad;
  }
  void setObject(ObjectData* obj) {
    assert((intptr_t(obj) & 1) == 0);
    m_obj = (ObjectData*)((intptr_t)obj | 1);
  }

  bool endHelper();
  void nextHelper();
  Variant firstHelper();
  void secondHelper(Variant &v);

  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
 public:
  ssize_t m_pos;
 private:
  int m_versionNumber;
  Type m_itype;

  friend struct VM::Iter;
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
  void reset();
  void release() { delete this; }

  // Returns true if the iterator points past the last element (or if
  // it points before the first element)
  bool end() const;

  // Move the iterator forward one element
  bool advance();

  // Returns true if the iterator points to a valid element
  bool prepare();

  ArrayData* getArray() const {
    ArrayData *data = hasVar() ? getData() : getAd();
    return data;
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
  ArrayData* getData() const;
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
  FullPosRange(FullPos* list) : m_fpos(list) {}
  FullPosRange(const FullPosRange& other) : m_fpos(other.m_fpos) {}
  bool empty() const { return m_fpos == 0; }
  FullPos* front() const { assert(!empty()); return m_fpos; }
  void popFront() { assert(!empty()); m_fpos = m_fpos->getNext(); }
 private:
  FullPos* m_fpos;  
};

/**
 * MutableArrayIter is used by code genereated by HPHPc, and it is also used
 * internally within the HipHop runtime
 */
class MutableArrayIter : public FullPos {
 public:
  MutableArrayIter() { m_var = NULL; }
  MutableArrayIter(const Variant* var, Variant* key, Variant& val);
  MutableArrayIter(ArrayData* data, Variant* key, Variant& val);
  ~MutableArrayIter();

  bool advance();

  void begin(Variant& map, Variant* key, Variant& val, CStrRef context);

 private:
  Variant* m_key;
  Variant* m_valp;
};

/**
 * MArrayIter is used by the VM
 */
class MArrayIter : public FullPos {
 public:
  MArrayIter() { m_data = NULL; }
  MArrayIter(const RefData* ref);
  MArrayIter(ArrayData* data);
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
    assert(data->getCount() <= 1 || data->noCopyOnWrite());
    assert(!getResetFlag() && data->validFullPos(*this));
    return data->getValueRef(m_pos);
  }

  friend struct VM::Iter;
};

namespace VM {
  struct Iter {
    ArrayIter& arr() {
      return *(ArrayIter*)m_u;
    }
    MArrayIter& marr() {
      return *(MArrayIter*)m_u;
    }
    bool init(TypedValue* c1);
    bool minit(TypedValue* v1);
    bool next();
    bool mnext();
    void free();
    void mfree();
   private:
    // C++ won't let you have union members with constructors. So we get to
    // implement unions by hand.
    char m_u[MAX(sizeof(ArrayIter), sizeof(MArrayIter))];
  } __attribute__ ((aligned(16)));

  bool interp_init_iterator(Iter* it, TypedValue* c1);
  bool interp_init_iterator_m(Iter* it, TypedValue* v1);
  bool interp_iter_next(Iter* it);
  bool interp_iter_next_m(Iter* it);

  int64 new_iter_array(HPHP::VM::Iter* dest, ArrayData* arr,
                       TypedValue* val);
  int64 new_iter_array_key(HPHP::VM::Iter* dest, ArrayData* arr,
                           TypedValue* val, TypedValue* key);
  int64 new_iter_object(HPHP::VM::Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);
  int64 iter_next(HPHP::VM::Iter* dest, TypedValue* val);
  int64 iter_next_key(HPHP::VM::Iter* dest, TypedValue* val, TypedValue* key);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_ITERATOR_H__
