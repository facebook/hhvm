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

#include <runtime/base/array/array_iterator.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/array/hphp_array.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/object_data.h>
#include <runtime/ext/ext_collection.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Static strings.

static StaticString s_rewind("rewind");
static StaticString s_valid("valid");
static StaticString s_next("next");
static StaticString s_key("key");
static StaticString s_current("current");
static StaticString s_Iterator("Iterator");
static StaticString s_Continuation("Continuation");

///////////////////////////////////////////////////////////////////////////////
// ArrayIter

ArrayIter::ArrayIter() : m_pos(ArrayData::invalid_index) {
  m_data = NULL;
}

HOT_FUNC
ArrayIter::ArrayIter(const ArrayData *data) {
  setArrayData(data);
  if (data) {
    data->incRefCount();
    m_pos = data->iter_begin();
  } else {
    m_pos = ArrayData::invalid_index;
  }
}

HOT_FUNC
ArrayIter::ArrayIter(CArrRef array) : m_pos(0) {
  const ArrayData* ad = array.get();
  setArrayData(ad);
  if (ad) {
    ad->incRefCount();
    m_pos = ad->iter_begin();
  } else {
    m_pos = ArrayData::invalid_index;
  }
}

void ArrayIter::reset() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    m_data = NULL;
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
    return;
  }
  ObjectData* obj = getRawObject();
  m_data = NULL;
  ASSERT(obj);
  decRefObj(obj);
}

void ArrayIter::begin(CVarRef map, CStrRef context) {
  try {
    new (this) ArrayIter(map.begin(context));
  } catch (...) {
    m_data = NULL;
    throw;
  }
}

void ArrayIter::begin(CArrRef map, CStrRef context) {
  try {
    new (this) ArrayIter(map.get());
  } catch (...) {
    m_data = NULL;
    throw;
  }
}

template <bool incRef>
void ArrayIter::objInit(ObjectData *obj) {
  ASSERT(obj);
  setObject(obj);
  if (incRef) {
    obj->incRefCount();
  }
  if (!obj->isCollection()) {
    ASSERT(obj->o_instanceof(s_Iterator));
    obj->o_invoke(s_rewind, Array());
  } else {
    if (hasVector()) {
      c_Vector* vec = getVector();
      m_versionNumber = vec->getVersionNumber();
      m_pos = 0;
    } else if (hasMap()) {
      c_Map* mp = getMap();
      m_versionNumber = mp->getVersionNumber();
      m_pos = mp->iter_begin();
    } else if (hasStableMap()) {
      c_StableMap* smp = getStableMap();
      m_versionNumber = smp->getVersionNumber();
      m_pos = smp->iter_begin();
    } else {
      ASSERT(false);
    }
  }
}

ArrayIter::ArrayIter(ObjectData *obj)
  : m_pos(ArrayData::invalid_index) {
  objInit<true>(obj);
}

ArrayIter::ArrayIter(Object &obj, TransferOwner)
  : m_pos(ArrayData::invalid_index) {
  objInit<false>(obj.get());
  (void) obj.detach();
}

// Special constructor used by the VM. This constructor does not increment the
// refcount of the specified object.
ArrayIter::ArrayIter(ObjectData *obj, NoInc)
  : m_pos(ArrayData::invalid_index) {
  objInit<false>(obj);
}

HOT_FUNC
ArrayIter::~ArrayIter() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
    return;
  }
  ObjectData* obj = getRawObject();
  ASSERT(obj);
  decRefObj(obj);
}

bool ArrayIter::endHelper() {
  if (hasVector()) {
    c_Vector* vec = getVector();
    return m_pos >= vec->t_count();
  }
  if (hasMap()) {
    return m_pos == 0;
  }
  if (hasStableMap()) {
    return m_pos == 0;
  }
  ASSERT(hasObject());
  ObjectData* obj = getObject();
  return !obj->o_invoke(s_valid, Array());
}

void ArrayIter::nextHelper() {
  if (hasVector()) {
    m_pos++;
    return;
  }
  if (hasMap()) {
    ASSERT(m_pos != 0);
    c_Map* mp = getMap();
    if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
      throw_collection_modified();
    }
    m_pos = mp->iter_next(m_pos);
    return;
  }
  if (hasStableMap()) {
    ASSERT(m_pos != 0);
    c_StableMap* smp = getStableMap();
    if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
      throw_collection_modified();
    }
    m_pos = smp->iter_next(m_pos);
    return;
  }
  ASSERT(hasObject());
  ObjectData* obj = getObject();
  obj->o_invoke(s_next, Array());
}

Variant ArrayIter::firstHelper() {
  if (hasVector()) {
    return m_pos;
  }
  if (hasMap()) {
    ASSERT(m_pos != 0);
    c_Map* mp = getMap();
    if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
      throw_collection_modified();
    }
    return mp->iter_key(m_pos);
  }
  if (hasStableMap()) {
    ASSERT(m_pos != 0);
    c_StableMap* smp = getStableMap();
    if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
      throw_collection_modified();
    }
    return smp->iter_key(m_pos);
  }
  ASSERT(hasObject());
  ObjectData* obj = getObject();
  return obj->o_invoke(s_key, Array());
}

HOT_FUNC
Variant ArrayIter::second() {
  if (hasVector()) {
    c_Vector* vec = getVector();
    if (UNLIKELY(m_versionNumber != vec->getVersionNumber())) {
      throw_collection_modified();
    }
    return tvAsCVarRef(vec->at(m_pos));
  }
  if (hasMap()) {
    c_Map* mp = getMap();
    if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
      throw_collection_modified();
    }
    return mp->iter_value(m_pos);
  }
  if (hasStableMap()) {
    c_StableMap* smp = getStableMap();
    if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
      throw_collection_modified();
    }
    return smp->iter_value(m_pos);
  }
  if (hasObject()) {
    ObjectData* obj = getObject();
    return obj->o_invoke(s_current, Array());
  }
  ASSERT(hasArrayData());
  ASSERT(m_pos != ArrayData::invalid_index);
  const ArrayData* ad = getArrayData();
  ASSERT(ad);
  return ad->getValue(m_pos);
}

void ArrayIter::secondHelper(Variant & v) {
  if (hasVector()) {
    c_Vector* vec = getVector();
    if (UNLIKELY(m_versionNumber != vec->getVersionNumber())) {
      throw_collection_modified();
    }
    v = tvAsCVarRef(vec->at(m_pos));
    return;
  }
  if (hasMap()) {
    c_Map* mp = getMap();
    if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
      throw_collection_modified();
    }
    v = mp->iter_value(m_pos);
    return;
  }
  if (hasStableMap()) {
    c_StableMap* smp = getStableMap();
    if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
      throw_collection_modified();
    }
    v = smp->iter_value(m_pos);
    return;
  }
  ASSERT(hasObject());
  ObjectData* obj = getObject();
  v = obj->o_invoke(s_current, Array());
}

HOT_FUNC
CVarRef ArrayIter::secondRef() {
  if (!hasArrayData()) {
    throw FatalErrorException("taking reference on iterator objects");
  }
  ASSERT(hasArrayData());
  ASSERT(m_pos != ArrayData::invalid_index);
  const ArrayData* ad = getArrayData();
  ASSERT(ad);
  return ad->getValueRef(m_pos);
}

///////////////////////////////////////////////////////////////////////////////
// MutableArrayIter

MutableArrayIter::MutableArrayIter(const Variant *var, Variant *key,
                                   Variant &val)
  : m_var(var), m_data(NULL), m_key(key), m_valp(&val), m_fp() {
  ASSERT(m_var);
  escalateCheck();
  ArrayData* data = cowCheck();
  if (data) {
    data->reset();
    data->newFullPos(m_fp);
    ASSERT(m_fp.container == data);
  }
}

MutableArrayIter::MutableArrayIter(ArrayData *data, Variant *key,
                                   Variant &val)
  : m_var(NULL), m_data(data), m_key(key), m_valp(&val), m_fp() {
  if (data) {
    escalateCheck();
    data = cowCheck();
    data->reset();
    data->newFullPos(m_fp);
    ASSERT(m_fp.container == data);
  }
}

MutableArrayIter::~MutableArrayIter() {
  // free the iterator
  if (m_fp.container != NULL) {
    m_fp.container->freeFullPos(m_fp);
    ASSERT(m_fp.container == NULL);
  }
  // unprotect the data
  if (m_data) decRefArr(m_data);
}

void MutableArrayIter::reset() {
  if (m_fp.container != NULL) {
    m_fp.container->freeFullPos(m_fp);
    ASSERT(m_fp.container == NULL);
  }
  // unprotect the data
  if (m_data) {
    decRefArr(m_data);
    m_data = NULL;
  }
}

void MutableArrayIter::begin(Variant& map, Variant* key, Variant& val,
                             CStrRef context) {
  try {
    new (this) MutableArrayIter(map.begin(key, val, context));
  } catch (...) {
    m_fp.container = NULL;
    m_data = NULL;
    throw;
  }
}

bool MutableArrayIter::advance() {
  ArrayData *data = m_var ? getData() : m_data;
  if (!data) return false;
  // If the foreach loop's array changed since the previous iteration,
  // we recover by creating a new strong iterator for the new array,
  // starting with at the position indicated by the new array's internal
  // pointer.
  if (m_fp.container != data) {
    // Free the current strong iterator if its valid
    if (m_fp.container != NULL) {
      m_fp.container->freeFullPos(m_fp);
    }
    ASSERT(m_fp.container == NULL);
    // If needed, escalate the array to an array type that can support
    // foreach by reference
    escalateCheck();
    // Trigger COW if needed, copying over strong iterators
    data = cowCheck();
    // Create a new strong iterator for the new array
    data->newFullPos(m_fp);
  } else {
    // Trigger COW if needed, copying over strong iterators
    data = cowCheck();
  }
  ASSERT(m_fp.container == data);
  if (!data->setFullPos(m_fp)) return false;
  CVarRef curr = data->currentRef();
  m_valp->assignRef(curr);
  if (m_key) m_key->assignVal(data->key());
  data->next();
  data->getFullPos(m_fp);
  return true;
}

void MutableArrayIter::escalateCheck() {
  ArrayData* data;
  if (m_var) {
    data = getData();
    if (!data) return;
    ArrayData* esc = data->escalate(true);
    if (data != esc) {
      *const_cast<Variant*>(m_var) = esc;
    }
  } else {
    ASSERT(m_data);
    data = m_data;
    ArrayData* esc = data->escalate(true);
    if (data != esc) {
      esc->incRefCount();
      decRefArr(data);
      m_data = esc;
    }
  }
}

ArrayData* MutableArrayIter::cowCheck() {
  ArrayData* data;
  if (m_var) {
    data = getData();
    if (!data) return NULL;
    if (data->getCount() > 1 && !data->noCopyOnWrite()) {
      *const_cast<Variant*>(m_var) = (data = data->copyWithStrongIterators());
    }
  } else {
    ASSERT(m_data);
    data = m_data;
    if (data->getCount() > 1 && !data->noCopyOnWrite()) {
      ArrayData* copied = data->copyWithStrongIterators();
      copied->incRefCount();
      decRefArr(data);
      m_data = data = copied;
    }
  }
  return data;
}

ArrayData* MutableArrayIter::getData() {
  ASSERT(m_var);
  if (m_var->is(KindOfArray)) {
    return m_var->getArrayData();
  }
  return NULL;
}

MIterCtx::~MIterCtx() {
  m_mArray->~MutableArrayIter();
  smart_free(m_mArray);
  tvRefcountedDecRef(&m_key);
  tvRefcountedDecRef(&m_val);
  if (m_ref) decRefRef(const_cast<RefData*>(m_ref));
}

///////////////////////////////////////////////////////////////////////////////
}
