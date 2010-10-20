/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/array/zend_array.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(SharedMap, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////

SharedMap::SharedMap(SharedVariant* source)
  : m_arr(source) {
  source->incRef();
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

Variant SharedMap::get(CVarRef k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %s", k.toString().data());
    }
    return null;
  }
  SharedVariant *sv = m_arr->getValue(index);
  ASSERT(sv);
  return getLocal(sv);
}

Variant SharedMap::get(CStrRef k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %s", k.data());
    }
    return null;
  }
  SharedVariant *sv = m_arr->getValue(index);
  ASSERT(sv);
  return getLocal(sv);
}

Variant SharedMap::get(litstr k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %s", k);
    }
    return null;
  }
  SharedVariant *sv = m_arr->getValue(index);
  ASSERT(sv);
  return getLocal(sv);
}

Variant SharedMap::get(int64 k, bool error /* = false */) const {
  int index = m_arr->getIndex(k);
  if (index == -1) {
    if (error) {
      raise_notice("Undefined index: %ld", k);
    }
    return null;
  }
  SharedVariant *sv = m_arr->getValue(index);
  ASSERT(sv);
  return getLocal(sv);
}

ArrayData *SharedMap::lval(Variant *&ret, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(ret, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
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

ArrayData *SharedMap::set(int64 k, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::set(litstr k, CVarRef v, bool copy) {
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

ArrayData *SharedMap::remove(int64 k, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::remove(litstr k, bool copy) {
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

ArrayData *SharedMap::append(CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->append(v, false);
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
