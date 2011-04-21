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
#include <runtime/base/complex_types.h>
#include <runtime/base/object_data.h>

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

ArrayIter::ArrayIter()
  : m_data(NULL), m_obj(NULL), m_pos(ArrayData::invalid_index) { }

ArrayIter::ArrayIter(const ArrayData *data) : m_data(data), m_obj(NULL) {
  if (m_data) {
    m_data->incRefCount();
    m_pos = m_data->iter_begin();
  } else {
    m_pos = ArrayData::invalid_index;
  }
}

ArrayIter::ArrayIter(CArrRef array)
  : m_data(array.get()), m_obj(NULL), m_pos(0) {
  if (m_data) {
    m_data->incRefCount();
    m_pos = m_data->iter_begin();
  } else {
    m_pos = ArrayData::invalid_index;
  }
}

ArrayIter::ArrayIter(ObjectData *obj, bool rewind /* = true */)
  : m_data(NULL), m_obj(obj), m_pos(ArrayData::invalid_index) {
  ASSERT(m_obj);
  ASSERT(m_obj->o_instanceof(s_Iterator));
  m_obj->incRefCount();
  if (m_obj->o_instanceof(s_Continuation)) {
    m_obj->o_invoke(s_next, Array());
  } else if (rewind) {
    m_obj->o_invoke(s_rewind, Array());
  }
  // If it is from IteratorAggregate, there is no need to rewind.
}

ArrayIter::~ArrayIter() {
  if (m_data && m_data->decRefCount() == 0) {
    const_cast<ArrayData*>(m_data)->release();
    return;
  }
  if (m_obj && m_obj->decRefCount() == 0) {
    const_cast<ObjectData*>(m_obj)->release();
  }
}

bool ArrayIter::endHelper() {
  ASSERT(m_obj);
  return !m_obj->o_invoke(s_valid, Array());
}

void ArrayIter::nextHelper() {
  ASSERT(m_obj);
  m_obj->o_invoke(s_next, Array());
}

Variant ArrayIter::firstHelper() {
  ASSERT(m_obj);
  return m_obj->o_invoke(s_key, Array());
}

Variant ArrayIter::second() {
  if (m_obj) {
    return m_obj->o_invoke(s_current, Array());
  }
  ASSERT(m_data);
  ASSERT(m_pos != ArrayData::invalid_index);
  return m_data->getValue(m_pos);
}

void ArrayIter::secondHelper(Variant & v) {
  ASSERT(m_obj);
  v = m_obj->o_invoke(s_current, Array());
}

CVarRef ArrayIter::secondRef() {
  if (m_obj) {
    throw FatalErrorException("taking reference on iterator objects");
  }
  ASSERT(m_data);
  ASSERT(m_pos != ArrayData::invalid_index);
  return m_data->getValueRef(m_pos);
}

///////////////////////////////////////////////////////////////////////////////
// MutableArrayIter

MutableArrayIter::MutableArrayIter(const Variant *var, Variant *key,
                                   Variant &val)
  : m_var(var), m_data(NULL), m_key(key), m_val(val), m_fp() {
  ASSERT(m_var);
  ArrayData *data = getData();
  if (data) {
    data->reset();
    data->newFullPos(m_fp);
    ASSERT(m_fp.container == data);
  }
}

MutableArrayIter::MutableArrayIter(ArrayData *data, Variant *key,
                                   Variant &val)
  : m_var(NULL), m_data(data), m_key(key), m_val(val), m_fp() {
  if (data) {
    // protect the data which may be owned by a C++ temp
    data->incRefCount();
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
  if (m_data && m_data->decRefCount() == 0) {
    m_data->release();
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
    // Create a new strong iterator for the new array
    ASSERT(m_fp.container == NULL);
    data->newFullPos(m_fp);
  }
  ASSERT(m_fp.container == data);
  if (!data->setFullPos(m_fp)) return false;
  CVarRef curr = data->currentRef();
  m_val.assignRef(curr);
  if (m_key) m_key->assignVal(data->key());
  data->next();
  data->getFullPos(m_fp);
  return true;
}

ArrayData *MutableArrayIter::getData() {
  if (m_var->is(KindOfArray)) {
    return m_var->getArrayData();
  }
  return NULL;
}
///////////////////////////////////////////////////////////////////////////////
}
