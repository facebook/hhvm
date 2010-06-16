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

#include <runtime/base/array/vector_variant.h>
#include <runtime/base/shared/shared_map.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP {

StaticEmptyArray StaticEmptyArray::s_theEmptyArray;

IMPLEMENT_SMART_ALLOCATION(VectorVariant, SmartAllocatorImpl::NeedRestore);

///////////////////////////////////////////////////////////////////////////////
// constructors

VectorVariant::VectorVariant(CVarRef v) {
  m_elems.push_back(ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const VectorVariant *src) : ArrayData(src) {
  ASSERT(src);
  ArrayFuncs::append(m_elems, src->getElems());
}

VectorVariant::VectorVariant(const VectorVariant *src, CVarRef v) {
  ASSERT(src);
  m_elems.reserve(src->m_elems.size() + 1);
  ArrayFuncs::append(m_elems, src->m_elems);
  m_elems.push_back(ArrayFuncs::element(v));

}

VectorVariant::VectorVariant(const VectorVariant *src, int index, CVarRef v) {
  ASSERT(src);
  if (index == (int)src->m_elems.size()) {
    m_elems.reserve(src->m_elems.size() + 1);
    ArrayFuncs::append(m_elems, src->m_elems);
    m_elems.push_back(ArrayFuncs::element(v));
  } else {
    ASSERT(index >= 0 && index < (int)src->m_elems.size());
    ArrayFuncs::append(m_elems, src->m_elems);
    ArrayFuncs::set(m_elems, index, ArrayFuncs::element(v));
  }
}

VectorVariant::VectorVariant(const VectorVariant *src, const VectorVariant *vec,
                             ArrayOp op) {
  ASSERT(src && vec);
  int count1 = src->m_elems.size();
  int count2 = vec->m_elems.size();
  switch (op) {
  case Plus:
    if (count1 > count2) {
      ArrayFuncs::append(m_elems, src->m_elems);
    } else {
      m_elems.reserve(count2);
      ArrayFuncs::append(m_elems, src->m_elems);
      ArrayFuncs::append(m_elems, vec->m_elems, count1);
    }
    break;
  case Merge:
    m_elems.reserve(count1 + count2);
    ArrayFuncs::append(m_elems, src->m_elems);
    ArrayFuncs::append(m_elems, vec->m_elems);
    break;
  default:
    ASSERT(false);
    break;
  }
}

VectorVariant::~VectorVariant() {
  for (int i = m_elems.size() - 1; i >= 0; --i) {
    ArrayFuncs::release(m_elems[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

Variant VectorVariant::getKey(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index && pos < m_elems.size());
  return (int64)pos;
}

Variant VectorVariant::get(int64 k, int64 prehash /* = -1 */,
                           bool error /* = false */) const {
  if (k >= 0 && (ssize_t)k < m_elems.size()) {
    return *m_elems[k];
  }
  if (error) {
    raise_notice("Undefined index: %d", k);
  }
  return null;
}

Variant VectorVariant::get(litstr k, int64 prehash /* = -1 */,
                           bool error /* = false */) const {
  if (error) {
    raise_notice("Undefined index: %s", (const char*)k);
  }
  return null;
}

Variant VectorVariant::get(CStrRef k, int64 prehash /* = -1 */,
                           bool error /* = false */) const {
  if (error) {
    raise_notice("Undefined index: %s", k.data());
  }
  return null;
}

Variant VectorVariant::get(CVarRef k, int64 prehash /* = -1 */,
                           bool error /* = false */) const {
  if (k.isNumeric()) {
    int64 index = k.toInt64();
    if (index >= 0 && (ssize_t)index < m_elems.size()) {
      return *m_elems[index];
    }
    if (error) {
      raise_notice("Undefined index: %d", index);
    }
    return null;
  }
  if (error) {
    raise_notice("Undefined index: %s", k.toString().data());
  }
  return null;
}

bool VectorVariant::exists(int64 k, int64 prehash /* = -1 */) const {
  if (k >= 0 && (ssize_t)k < m_elems.size()) {
    return true;
  }
  return false;
}

bool VectorVariant::exists(litstr k, int64 prehash /* = -1 */) const {
  return false;
}

bool VectorVariant::exists(CStrRef k, int64 prehash /* = -1 */) const {
  return false;
}

bool VectorVariant::exists(CVarRef k, int64 prehash /* = -1 */) const {
  if (k.isNumeric()) {
    int64 index = k.toInt64();
    if (index >= 0 && (ssize_t)index < m_elems.size()) {
      return true;
    }
  }
  return false;
}

bool VectorVariant::idxExists(ssize_t idx) const {
  return idx != ArrayData::invalid_index;
}

ssize_t VectorVariant::getIndex(int64 k, int64 prehash /* = -1 */) const {
  if (k >= 0 && (ssize_t)k < m_elems.size()) {
    return k;
  }
  return ArrayData::invalid_index;
}

ssize_t VectorVariant::getIndex(litstr k, int64 prehash /* = -1 */) const {
  return ArrayData::invalid_index;
}

ssize_t VectorVariant::getIndex(CStrRef k, int64 prehash /* = -1 */) const {
  return ArrayData::invalid_index;
}

ssize_t VectorVariant::getIndex(CVarRef k, int64 prehash /* = -1 */) const {
  if (k.isNumeric()) {
    int64 index = k.toInt64();
    if (index >= 0 && (ssize_t)index < m_elems.size()) {
      return index;
    }
  }
  return ArrayData::invalid_index;
}

ssize_t VectorVariant::size() const {
  return m_elems.size();
}

Variant VectorVariant::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < m_elems.size());
  return *m_elems[pos];
}

void VectorVariant::fetchValue(ssize_t pos, Variant & v) const {
  ASSERT(pos >= 0 && pos < m_elems.size());
  v = *m_elems[pos];
}

CVarRef VectorVariant::getValueRef(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < m_elems.size());
  return *m_elems[pos];
}

ArrayData *VectorVariant::lval(Variant *&ret, bool copy) {
  ASSERT(!m_elems.empty());

  if (copy) {
    VectorVariant *data = NEW(VectorVariant)(this);
    ret = data->m_elems.back();
    return data;
  }

  ret = m_elems.back();
  return NULL;
}

ArrayData *VectorVariant::lval(int64 k, Variant *&ret, bool copy,
                               int64 prehash /* = -1 */,
                               bool checkExist /* = false */) {
  if (k >= 0 && k < m_elems.size()) {
    if (!copy || checkExist) {
      ret = m_elems[k];
      return NULL;
    }
    VectorVariant *data = NEW(VectorVariant)(this);
    ret = data->m_elems[k];
    return data;
  }
  if (k == m_elems.size()) {
    Variant *v = NEW(Variant)();
    ret = v;
    if (!copy) {
      m_elems.push_back(v);
      return NULL;
    }
    VectorVariant *data = NEW(VectorVariant)(this);
    data->m_elems.push_back(v);
    return data;
  }
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorVariant::lval(litstr k, Variant *&ret, bool copy,
                               int64 prehash /* = -1 */,
                               bool checkExist /* = false */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorVariant::lval(CStrRef k, Variant *&ret, bool copy,
                               int64 prehash /* = -1 */,
                               bool checkExist /* = false */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorVariant::lval(CVarRef k, Variant *&ret, bool copy,
                               int64 prehash /* = -1 */,
                               bool checkExist /* = false */) {
  if (k.isInteger()) {
    return lval(k.toInt64(), ret, copy, prehash, checkExist);
  } else {
    MapVariant *escalated = NEW(MapVariant)(this);
    escalated->lval(k, ret, false, prehash);
    return escalated;
  }
}

ArrayData *VectorVariant::set(int64 k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  if (k >= 0 && k < (int64)m_elems.size()) {
    if (!copy) {
      *m_elems[k] = v;
      return NULL;
    }
    return NEW(VectorVariant)(this, k, v);
  }
  if (k == (int64)m_elems.size()) {
    if (!copy) {
      m_elems.push_back(ArrayFuncs::element(v));
      return NULL;
    }
    return NEW(VectorVariant)(this, k, v);
  }
  ArrayData * ret = NEW(MapVariant)(this, Variant(k), v);
  return ret;
}

ArrayData *VectorVariant::set(litstr k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  ArrayData * ret = NEW(MapVariant)(this, Variant(k), v);
  return ret;
}

ArrayData *VectorVariant::set(CStrRef k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  ArrayData * ret = NEW(MapVariant)(this, Variant(k), v);
  return ret;
}

ArrayData *VectorVariant::set(CVarRef k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  if (k.isNumeric()) {
    int64 index = k.toInt64();
    if (index >= 0 && index < (int64)m_elems.size()) {
      if (!copy) {
        *m_elems[index] = v;
        return NULL;
      }
      return NEW(VectorVariant)(this, index, v);
    }
    if (index == (int64)m_elems.size()) {
      if (!copy) {
        m_elems.push_back(ArrayFuncs::element(v));
        return NULL;
      }
      return NEW(VectorVariant)(this, index, v);
    }
  }
  ArrayData * ret = NEW(MapVariant)(this, k, v);
  return ret;
}

ArrayData *VectorVariant::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  if (k >= 0 && (ssize_t)k < m_elems.size()) {
    if (!copy && (ssize_t)k == m_elems.size()-1) {
      m_elems[k]->release();
      m_elems.remove(k);
      return NULL;
    }
    MapVariant * ret = NEW(MapVariant)(this);
    ret->remove(k, false);
    return ret;
  }
  if (copy) {
    return NEW(VectorVariant)(this);
  }
  return NULL;
}

ArrayData *VectorVariant::remove(litstr k, bool copy,
                                 int64 prehash /* = -1 */) {
  if (copy) {
    return NEW(VectorVariant)(this);
  }
  return NULL;
}

ArrayData *VectorVariant::remove(CStrRef k, bool copy,
                                 int64 prehash /* = -1 */) {
  if (copy) {
    return NEW(VectorVariant)(this);
  }
  return NULL;
}

ArrayData *VectorVariant::remove(CVarRef k, bool copy,
                                 int64 prehash /* = -1 */) {
  if (k.isNumeric()) {
    int64 index = k.toInt64();
    if (index >= 0 && (ssize_t)index < m_elems.size()) {
      if (!copy && (ssize_t)index == m_elems.size()-1) {
        m_elems[index]->release();
        m_elems.remove(index);
        return NULL;
      }
      MapVariant * ret = NEW(MapVariant)(this);
      ret->remove(index, false);
      return ret;
    }
  }
  if (copy) {
    return NEW(VectorVariant)(this);
  }
  return NULL;
}

ArrayData *VectorVariant::copy() const {
  return NEW(VectorVariant)(this);
}

ArrayData *VectorVariant::append(CVarRef v, bool copy) {
  if (copy) {
    return NEW(VectorVariant)(this, v);
  }
  m_elems.push_back(ArrayFuncs::element(v));
  return NULL;
}

ArrayData *VectorVariant::append(const ArrayData *elems, ArrayOp op,
                                 bool copy) {
  ASSERT(elems);
  const MapVariant *mapAny = dynamic_cast<const MapVariant *>(elems);
  if (mapAny) {
    return NEW(MapVariant)(this, mapAny, op);
  }

  const VectorVariant *vec = dynamic_cast<const VectorVariant *>(elems);
  if (vec) {
    if (copy) {
      return NEW(VectorVariant)(this, vec, op);
    }
    switch (op) {
    case Plus:
      {
        int count1 = m_elems.size();
        int count2 = vec->m_elems.size();
        if (count1 <= count2) {
          ArrayFuncs::append(m_elems, vec->m_elems, count1);
        }
      }
      break;
    case Merge:
      ArrayFuncs::append(m_elems, vec->m_elems);
      break;
    default:
      ASSERT(false);
      break;
    }
    return NULL;
  }

  const SharedMap *mapShared = dynamic_cast<const SharedMap *>(elems);
  if (mapShared) {
    MapVariant *escalated = mapShared->escalateToMapVariant();
    MapVariant *ret = NEW(MapVariant)(this, escalated, op);
    DELETE(MapVariant)(escalated);
    return ret;
  }
  ASSERT(false);
  return NULL;
}

ArrayData *VectorVariant::prepend(CVarRef v, bool copy) {
  if (copy) {
    ArrayData* ret = NEW(VectorVariant)(this);
    ret->prepend(v, false);
    return ret;
  }

  m_elems.insert(0, ArrayFuncs::element(v));
  return NULL;
}

void VectorVariant::onSetStatic() {
  for (unsigned int i = 0; i < m_elems.size(); i++) {
    m_elems[i]->setStatic();
  }
}

ArrayData *VectorVariant::escalate(bool mutableIteration /* = false */) const {
  // Assume ZendArray is not used.
  ASSERT(!RuntimeOption::UseZendArray);
  return NEW(MapVariant)(this);
}

///////////////////////////////////////////////////////////////////////////////
}
