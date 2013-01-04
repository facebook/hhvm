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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;
class c_Vector;
class c_Map;
class c_StableMap;

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
  /**
   * Constructors.
   */
  ArrayIter();
  ArrayIter(const ArrayData* data);

  enum NoInc { noInc = 0 };
// Special constructor used by the VM. This constructor does not
// increment the refcount of the specified array.
  ArrayIter(const ArrayData* data, NoInc) {
    setArrayData(data);
    if (data) {
      m_pos = data->iter_begin();
    } else {
      m_pos = ArrayData::invalid_index;
    }
  }
  // This is also a special constructor used by the VM. This
  // constructor doesn't increment the array's refcount and assumes
  // that the array is not empty.
  enum NoIncNonNull { noIncNonNull = 0 };
  ArrayIter(const HphpArray* data, NoIncNonNull) {
    ASSERT(data);
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
      ASSERT(ad);
      ASSERT(m_pos != ArrayData::invalid_index);
      m_pos = ad->iter_advance(m_pos);
      return;
    }
    nextHelper();
  }
  Variant first() {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      ASSERT(ad);
      ASSERT(m_pos != ArrayData::invalid_index);
      return ad->getKey(m_pos);
    }
    return firstHelper();
  }
  Variant second();
  void second(Variant &v) {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      ASSERT(ad);
      ASSERT(m_pos != ArrayData::invalid_index);
      v = ad->getValueRef(m_pos);
      return;
    }
    secondHelper(v);
  }
  CVarRef secondRef();

  void nvFirst(TypedValue* out) {
    const ArrayData* ad = getArrayData();
    ASSERT(ad && m_pos != ArrayData::invalid_index);
    const_cast<ArrayData*>(ad)->nvGetKey(out, m_pos);
  }

  TypedValue* nvSecond() {
    const ArrayData* ad = getArrayData();
    ASSERT(ad && m_pos != ArrayData::invalid_index);
    return const_cast<ArrayData*>(ad)->nvGetValueRef(m_pos);
  }

  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
  ssize_t m_pos;
  int m_versionNumber;

 public:
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
    ASSERT(hasArrayData());
    return m_data;
  }
  ssize_t getPos() {
    return m_pos;
  }
  void setPos(ssize_t newPos) {
    m_pos = newPos;
  }
 private:
  c_Vector* getVector() {
    ASSERT(hasVector());
    return (c_Vector*)((intptr_t)m_obj & ~1);
  }
  c_Map* getMap() {
    ASSERT(hasMap());
    return (c_Map*)((intptr_t)m_obj & ~1);
  }
  c_StableMap* getStableMap() {
    ASSERT(hasStableMap());
    return (c_StableMap*)((intptr_t)m_obj & ~1);
  }
  ObjectData* getObject() {
    ASSERT(hasObject());
    return (ObjectData*)((intptr_t)m_obj & ~1);
  }
  ObjectData* getRawObject() {
    ASSERT(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~1);
  }

  void setArrayData(const ArrayData* ad) {
    ASSERT((intptr_t(ad) & 1) == 0);
    m_data = ad;
  }
  void setObject(ObjectData* obj) {
    ASSERT((intptr_t(obj) & 1) == 0);
    m_obj = (ObjectData*)((intptr_t)obj | 1);
  }

  bool endHelper();
  void nextHelper();
  Variant firstHelper();
  void secondHelper(Variant &v);
};

///////////////////////////////////////////////////////////////////////////////

/**
 * FullPos represents a position in an array that could reallocate; so we store
 * information to access an element, instead of a pointer directly to
 * the element.
 *
 * Each Array with an active MutableArrayIter keeps a linked list of active
 * FullPos's; list is maintained through the FullPos.next fields and the
 * head is ArrayData.m_strongIterators.
 */
struct FullPos {
  ssize_t pos;  // pos within container
  ArrayData* container; // the array itself
  FullPos* next; // next FullPos of another iterator for the same array
  FullPos() : pos(0), container(NULL), next(NULL) {}
};

/**
 * Range which visits each entry in a list of FullPos.  Removing the
 * front element will crash but removing an already-visited element
 * or future element will work.
 */
class FullPosRange {
public:
  FullPosRange(FullPos* list) : m_fp(list) {}
  FullPosRange(const FullPosRange& other) : m_fp(other.m_fp) {}
  bool empty() const { return m_fp == 0; }
  FullPos* front() const { ASSERT(!empty()); return m_fp; }
  void popFront() { ASSERT(!empty()); m_fp = m_fp->next; }
private:
  FullPos* m_fp;
};

/**
 * Iterator for "foreach ($arr as &$v)" or "foreach ($array as $n => &$v)".
 * In this case, any changes to $arr inside iteration needs to be visible to
 * the iteration. Therefore, we need to store Variant* with the iterator to
 * see those changes. This class should only be used for generated code.
 */
class MutableArrayIter {
public:
  MutableArrayIter() : m_data(NULL) {}
  MutableArrayIter(const Variant* var, Variant* key, Variant& val);
  MutableArrayIter(ArrayData* data, Variant* key, Variant& val);
  ~MutableArrayIter();
  void begin(Variant& map, Variant* key, Variant& val, CStrRef context);
  void reset();
  void release() { delete this; }
  bool advance();
private:
  const Variant* m_var;
  ArrayData* m_data;
  Variant* m_key;
  Variant* m_valp;
  FullPos m_fp;
  int size();
  ArrayData* getData();
  ArrayData* cowCheck();
  void escalateCheck();
};

struct MIterCtx {
  const RefData* prepRef(const RefData* ref) {
    ref->incRefCount();
    return ref;
  }
  MutableArrayIter* initMArray(ArrayData* data, Variant* key, Variant& val) {
    MutableArrayIter* mArray =
      (MutableArrayIter*)smart_malloc(sizeof(MutableArrayIter));
    (void) new (mArray) MutableArrayIter(data, key, val);
    return mArray;
  }
  MutableArrayIter* initMArray(const Variant* var, Variant* key, Variant& val) {
    MutableArrayIter* mArray =
      (MutableArrayIter*)smart_malloc(sizeof(MutableArrayIter));
    (void) new (mArray) MutableArrayIter(var, key, val);
    return mArray;
  }
  MIterCtx(ArrayData *ad)
    : m_key(*(const TypedValue*)&null_variant),
      m_val(*(const TypedValue*)&null_variant), m_ref(NULL),
      m_mArray(initMArray(ad, &tvAsVariant(&m_key), tvAsVariant(&m_val))) {
    ASSERT(!ad->isStatic());
  }
  MIterCtx(const RefData* ref)
    : m_key(*(TypedValue*)&null_variant), m_val(*(TypedValue*)&null_variant),
      m_ref(prepRef(ref)),
      m_mArray(initMArray((Variant*)(ref->tv()), &tvAsVariant(&m_key),
                          tvAsVariant(&m_val))) {
    // Reference must be an inner cell
    ASSERT(ref->_count > 0);
  }
  ~MIterCtx();

  TypedValue& key() { return m_key; }
  TypedValue& val() { return m_val; }
  MutableArrayIter& mArray() const { return *m_mArray; }

private:
  TypedValue m_key;
  TypedValue m_val;
  const RefData* m_ref;
  // MutableArrayIter is big; allocate it separately (rather than directly
  // embedding it) in order to keep iterators on the VM stack to a more
  // reasonable size.  If all iterators were mutable, and they were guaranteed
  // to be used by every function invocation, then this optimization would not
  // make sense.
  MutableArrayIter* m_mArray;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_ITERATOR_H__
