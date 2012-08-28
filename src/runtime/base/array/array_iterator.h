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
  ArrayIter(const ArrayData* data, int);
  ArrayIter(CArrRef array);
  ArrayIter(ObjectData* obj, bool rewind = true);
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

  bool isHphpArray() {
    ASSERT(hasArrayData());
    const ArrayData* ad = getArrayData();
    ASSERT(ad);
    return IsHphpArray(ad);
  }

  void nvFirst(TypedValue* out) {
    ASSERT(hasArrayData());
    const ArrayData* ad = getArrayData();
    ASSERT(ad);
    ASSERT(m_pos != ArrayData::invalid_index);
    ASSERT(isHphpArray());
    HphpArray* ha = (HphpArray*)ad;
    ha->nvGetKey(out, m_pos);
  }

  TypedValue* nvSecond() {
    ASSERT(hasArrayData());
    const ArrayData* ad = getArrayData();
    ASSERT(ad);
    ASSERT(m_pos != ArrayData::invalid_index);
    ASSERT(isHphpArray());
    HphpArray* ha = (HphpArray*)ad;
    return ha->nvGetValueRef(m_pos);
  }

private:
  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
  ssize_t m_pos;
  int m_versionNumber;

  bool hasArrayData() {
    return !((intptr_t)m_data & 1);
  }
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

  const ArrayData* getArrayData() {
    ASSERT(hasArrayData());
    return m_data;
  }
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

struct FullPos {
  ssize_t pos;
  ArrayData * container;
  FullPos() : pos(0), container(NULL) {}
};

/**
 * Iterator for "foreach ($arr as &$v)" or "foreach ($array as $n => &$v)".
 * In this case, any changes to $arr inside iteration needs to be visible to
 * the iteration. Therefore, we need to store Variant* with the iterator to
 * see those changes. This class should only be used for generated code.
 */
class MutableArrayIter {
public:
  MutableArrayIter(const Variant* var, Variant* key, Variant& val);
  MutableArrayIter(ArrayData* data, Variant* key, Variant& val);
  ~MutableArrayIter();
  void release() { delete this;}
  bool advance();

private:
  const Variant* m_var;
  ArrayData* m_data;
  Variant* m_key;
  Variant& m_val;
  FullPos m_fp;
  int size();
  ArrayData* getData();
  ArrayData* cowCheck();
  void escalateCheck();
};

struct MIterCtx {
  TypedValue m_key;
  TypedValue m_val;
  const RefData* m_ref;
  MutableArrayIter *m_mArray; // big! Defer allocation.
  MIterCtx(ArrayData *ad) {
    ASSERT(!ad->isStatic());
    tvWriteUninit(&m_key);
    tvWriteUninit(&m_val);
    m_ref = NULL;
    m_mArray = new MutableArrayIter(ad, &tvAsVariant(&m_key),
                                    tvAsVariant(&m_val));
  }
  MIterCtx(const RefData* ref) {
    tvWriteUninit(&m_key);
    tvWriteUninit(&m_val);
    // Reference must be an inner cell
    ASSERT(ref->_count > 0);
    m_ref = ref;
    m_ref->incRefCount();
    // Bind ref to m_var
    m_mArray = new MutableArrayIter((Variant*)(ref->tv()),
                                    &tvAsVariant(&m_key),
                                    tvAsVariant(&m_val));
  }
  ~MIterCtx();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_ITERATOR_H__
