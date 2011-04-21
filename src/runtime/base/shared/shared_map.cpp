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

#include <runtime/base/type_conversions.h>
#include <runtime/base/shared/shared_map.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(SharedMap, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////

SharedMap::SharedMap(SharedVariant* source) : m_arr(source) {
  source->incRef();
}

CVarRef SharedMap::getValueRef(ssize_t pos) const {
  SharedVariant *sv = m_arr->getValue(pos);
  DataType t = sv->getType();
  if (!IS_REFCOUNTED_TYPE(t)) return sv->asCVarRef();
  Variant *pv = m_localCache.lvalPtr((int64)pos, false, false);
  if (pv) return *pv;
  Variant &r = m_localCache.addLval((int64)pos);
  r = sv->toLocal();
  return r;
}

Variant SharedMap::getValueUncached(ssize_t pos) const {
  SharedVariant *sv = m_arr->getValue(pos);
  DataType t = sv->getType();
  if (!IS_REFCOUNTED_TYPE(t)) return sv->asCVarRef();
  Variant *pv = m_localCache.lvalPtr((int64)pos, false, false);
  if (pv) return *pv;
  return sv->toLocal();
}

bool SharedMap::exists(CVarRef k) const {
  return m_arr->getIndex(k) != -1;
}
bool SharedMap::exists(CStrRef k) const {
  return m_arr->getIndex(k) != -1;
}
bool SharedMap::exists(litstr k) const {
  return m_arr->getIndex(k) != -1;
}
bool SharedMap::exists(int64 k) const {
  return m_arr->getIndex(k) != -1;
}

ssize_t SharedMap::getIndex(int64 k) const {
  return m_arr->getIndex(k);
}
ssize_t SharedMap::getIndex(litstr k) const {
  return m_arr->getIndex(k);
}
ssize_t SharedMap::getIndex(CStrRef k) const {
  return m_arr->getIndex(k);
}
ssize_t SharedMap::getIndex(CVarRef k) const {
  return m_arr->getIndex(k);
}

CVarRef SharedMap::get(CVarRef k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %s", k.toString().data());
    }
    return null_variant;
  }
  return getValueRef(index);
}

CVarRef SharedMap::get(CStrRef k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %s", k.data());
    }
    return null_variant;
  }
  return getValueRef(index);
}

CVarRef SharedMap::get(litstr k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %s", k);
    }
    return null_variant;
  }
  return getValueRef(index);
}

CVarRef SharedMap::get(int64 k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %ld", k);
    }
    return null_variant;
  }
  return getValueRef(index);
}

ArrayData *SharedMap::lval(int64 k, Variant *&ret, bool copy,
                           bool checkExist /* = false */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::lval(litstr k, Variant *&ret, bool copy,
                           bool checkExist /* = false */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::lval(CStrRef k, Variant *&ret, bool copy,
                           bool checkExist /* = false */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::lval(CVarRef k, Variant *&ret, bool copy,
                           bool checkExist /* = false */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::lvalNew(Variant *&ret, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lvalNew(ret, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::set(int64 k, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::set(CStrRef k, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::set(CVarRef k, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::setRef(int64 k, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->setRef(k, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::setRef(CStrRef k, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->setRef(k, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::setRef(CVarRef k, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->setRef(k, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::remove(int64 k, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::remove(CStrRef k, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::remove(CVarRef k, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::copy() const {
  return escalate();
}

ArrayData *SharedMap::fiberCopy() const {
  ArrayInit ai(size());
  for (int i = 0; i < size(); i++) {
    ai.add(getKey(i), getValueUncached(i), true);
  }
  return ai.create();
}

ArrayData *SharedMap::append(CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->append(v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::appendRef(CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->appendRef(v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::appendWithRef(CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->appendWithRef(v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->append(elems, op, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::prepend(CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->prepend(v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::escalate(bool mutableIteration /* = false */) const {
  ArrayData *ret = NULL;
  m_arr->loadElems(ret, *this, mutableIteration);
  ASSERT(!ret->isStatic());
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
