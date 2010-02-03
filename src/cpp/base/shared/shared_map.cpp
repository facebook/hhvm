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

#include <cpp/base/shared/shared_map.h>
#include <cpp/base/array/map_variant.h>
#include <cpp/base/array/zend_array.h>
#include <cpp/base/runtime_option.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(SharedMap, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////

SharedMap::SharedMap(SharedVariant* source)
  : m_arr(source) {
  source->incRef();
}


bool SharedMap::exists(CVarRef k, int64 prehash /* = -1*/) const {
  return m_arr->exists(k);
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
                            int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::lval(litstr k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::lval(CStrRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::lval(CVarRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->lval(k, ret, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::set(int64 k, CVarRef v, bool copy, int64 prehash) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::set(litstr k, CVarRef v, bool copy, int64 prehash) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::set(CStrRef k, CVarRef v, bool copy, int64 prehash) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::set(CVarRef k, CVarRef v, bool copy, int64 prehash) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->set(k, v, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false, prehash);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}
ArrayData *SharedMap::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->remove(k, false, prehash);
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

ArrayData *SharedMap::insert(ssize_t pos, CVarRef v, bool copy) {
  ArrayData *escalated = escalate();
  ArrayData *ee = escalated->insert(pos, v, false);
  if (ee) {
    escalated->release();
    return ee;
  }
  return escalated;
}

ArrayData *SharedMap::escalate() const {
  ArrayData *ret = NULL;
  if (RuntimeOption::UseZendArray) {
    std::vector<ArrayElement *> elems;
    m_arr->loadElems(elems);
    if (!elems.empty()) {
      ret = ArrayData::Create(elems);
    } else {
      ret = StaticEmptyZendArray::Get()->copy();
    }
  } else {
    ret = escalateToMapVariant();
  }
  ASSERT(!ret->isStatic());
  return ret;
}

MapVariant *SharedMap::escalateToMapVariant() const {
  std::vector<ArrayElement *> elems;
  m_arr->loadElems(elems);
  MapVariant *escalated = NEW(MapVariant)(elems);
  size_t count = elems.size();
  for (uint i = 0; i < count; i++) {
    elems[i]->release();
  }
  return escalated;
}

///////////////////////////////////////////////////////////////////////////////
}
