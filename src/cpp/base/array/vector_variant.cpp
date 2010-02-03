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

#include <cpp/base/array/empty_array.h>
#include <cpp/base/array/vector_long.h>
#include <cpp/base/array/vector_string.h>
#include <cpp/base/array/vector_variant.h>
#include <cpp/base/array/map_long.h>
#include <cpp/base/array/map_string.h>
#include <cpp/base/array/map_variant.h>
#include <cpp/base/shared/shared_map.h>
#include <cpp/base/array/array_element.h>

using namespace std;

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(VectorVariant, SmartAllocatorImpl::NeedRestore);
///////////////////////////////////////////////////////////////////////////////
// constructors

VectorVariant::VectorVariant(CVarRef v) {
  m_elems.push_back(ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const std::vector<ArrayElement *> &elems) {
  unsigned int size = elems.size();
  m_elems.reserve(size);
  for (unsigned int i = 0; i < size; i++) {
    m_elems.push_back(ArrayFuncs::element(elems[i]->getVariant()));
  }
}

VectorVariant::VectorVariant(const VectorLong *src) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems());
}

VectorVariant::VectorVariant(const VectorLong *src, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const VectorLong *src, int index, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), index, ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const VectorLong *src, const Vector *vec,
                             ArrayOp op) {
  ASSERT(src && vec);
  const VectorString *strs = dynamic_cast<const VectorString *>(vec);
  if (strs) {
    appendImpl(m_elems, src->getElems(), strs->getElems(), op);
  } else {
    const VectorVariant *vars = dynamic_cast<const VectorVariant *>(vec);
    ASSERT(vars);
    appendImpl(m_elems, src->getElems(), vars->getElems(), op);
  }
}

VectorVariant::VectorVariant(const VectorString *src) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems());
}

VectorVariant::VectorVariant(const VectorString *src, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const VectorString *src, int index, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), index, ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const VectorString *src, const Vector *vec,
                             ArrayOp op) {
  ASSERT(src && vec);
  const VectorVariant *vars = dynamic_cast<const VectorVariant *>(vec);
  if (vars) {
    appendImpl(m_elems, src->getElems(), vars->getElems(), op);
  } else {
    const VectorLong *longs = dynamic_cast<const VectorLong *>(vec);
    ASSERT(longs);
    appendImpl(m_elems, src->getElems(), longs->getElems(), op);
  }
}

VectorVariant::VectorVariant(const VectorVariant *src) : Vector(src) {
  ASSERT(src);
  ArrayFuncs::append(m_elems, src->getElems());
}

VectorVariant::VectorVariant(const VectorVariant *src, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const VectorVariant *src, int index, CVarRef v) {
  ASSERT(src);
  appendImpl(m_elems, src->getElems(), index, ArrayFuncs::element(v));
}

VectorVariant::VectorVariant(const VectorVariant *src, const Vector *vec,
                             ArrayOp op) {
  ASSERT(src && vec);

  const VectorVariant *vars = dynamic_cast<const VectorVariant *>(vec);
  if (vars) {
    appendImpl(m_elems, src->getElems(), vars->getElems(), op);
    return;
  }

  const VectorString *strs = dynamic_cast<const VectorString *>(vec);
  if (strs) {
    appendImpl(m_elems, src->getElems(), strs->getElems(), op);
    return;
  }

  const VectorLong *longs = dynamic_cast<const VectorLong *>(vec);
  if (longs) {
    appendImpl(m_elems, src->getElems(), longs->getElems(), op);
    return;
  }

  ASSERT(false);
}

VectorVariant::~VectorVariant() {
  for (int i = m_elems.size() - 1; i >= 0; --i) {
    ArrayFuncs::release(m_elems[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayData

ssize_t VectorVariant::size() const {
  return m_elems.size();
}

Variant VectorVariant::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < size());
  return *m_elems[pos];
}

CVarRef VectorVariant::getValueRef(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < size());
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
                               int64 prehash /* = -1 */) {
  return lvalImpl(getIndex(k), ret, copy, prehash);
}

ArrayData *VectorVariant::lval(litstr k, Variant *&ret, bool copy,
                               int64 prehash /* = -1 */) {

  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorVariant::lval(CStrRef k, Variant *&ret, bool copy,
                               int64 prehash /* = -1 */) {
  MapVariant *escalated = NEW(MapVariant)(this);
  escalated->lval(k, ret, false, prehash);
  return escalated;
}

ArrayData *VectorVariant::lval(CVarRef k, Variant *&ret, bool copy,
                               int64 prehash /* = -1 */) {
  if (k.isInteger()) {
    return lvalImpl(getIndex(k), ret, copy, prehash);
  } else {
    MapVariant *escalated = NEW(MapVariant)(this);
    escalated->lval(k, ret, false, prehash);
    return escalated;
  }
}

ArrayData *VectorVariant::set(int64 k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  if (k == (int64)m_elems.size()) {
    return append(v, copy);
  }
  return setImpl(k, v, copy);
}

ArrayData *VectorVariant::set(litstr k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  return setImpl(k, v, copy);
}

ArrayData *VectorVariant::set(CStrRef k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  return setImpl(k, v, copy);
}

ArrayData *VectorVariant::set(CVarRef k, CVarRef v,
                              bool copy, int64 prehash /* = -1 */) {
  if (k.isInteger()) {
    return set(k.toInt64(), v, copy);
  }
  return setImpl(k, v, copy);
}

ArrayData *VectorVariant::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorVariant::remove(litstr k, bool copy,
                                 int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorVariant::remove(CStrRef k, bool copy,
                                 int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
}

ArrayData *VectorVariant::remove(CVarRef k, bool copy,
                                 int64 prehash /* = -1 */) {
  return removeImpl(getIndex(k), copy);
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

  const EmptyArray *none = dynamic_cast<const EmptyArray *>(elems);
  if (none) {
    return NULL;
  }

  const Map *mapAny = dynamic_cast<const Map *>(elems);
  if (mapAny) {
    return NEW(MapVariant)(this, mapAny, op);
  }

  const Vector *vec = dynamic_cast<const Vector *>(elems);
  if (vec) {
    if (copy) {
      return NEW(VectorVariant)(this, vec, op);
    }
    const VectorVariant *vecVariant = dynamic_cast<const VectorVariant *>(vec);
    if (vecVariant) {
      appendImpl(m_elems, vecVariant->getElems(), op);
      return NULL;
    }
    const VectorString *vecString = dynamic_cast<const VectorString *>(vec);
    if (vecString) {
      appendImpl(m_elems, vecString->getElems(), op);
      return NULL;
    }
    const VectorLong *vecLong = dynamic_cast<const VectorLong *>(vec);
    if (vecLong) {
      appendImpl(m_elems, vecLong->getElems(), op);
      return NULL;
    }
    ASSERT(false);
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

ArrayData *VectorVariant::insert(ssize_t pos, CVarRef v, bool copy) {
  if (pos >= size()) {
    return append(v, false);
  }

  if (copy) {
    ArrayData* ret = NEW(VectorVariant)(this);
    ret->insert(pos, v, false);
    return ret;
  }

  if (pos == ArrayData::invalid_index) pos = 0;

  m_elems.insert(pos, ArrayFuncs::element(v));
  return NULL;
}

void VectorVariant::onSetStatic() {
  for (unsigned int i = 0; i < m_elems.size(); i++) {
    m_elems[i]->setStatic();
  }
}

///////////////////////////////////////////////////////////////////////////////
// helpers

Variant VectorVariant::getImpl(int index) const {
  if (index >= 0) {
    return *m_elems[index];
  }
  return null;
}

ArrayData *VectorVariant::lvalImpl(int index, Variant *&ret, bool copy,
                                   int64 prehash) {
  bool append = index < 0;
  if (copy) {
    VectorVariant *data = NEW(VectorVariant)(this);
    if (append) {
      Variant *v = NEW(Variant)();
      data->m_elems.push_back(v);
      ret = v;
    } else {
      ret = data->m_elems[index];
    }
    return data;
  }
  if (append) {
    Variant *v = NEW(Variant)();
    m_elems.push_back(v);
    ret = v;
  } else {
    ret = m_elems[index];
  }
  return NULL;
}

bool VectorVariant::setImpl(int index, CVarRef v, bool copy, ArrayData *&ret) {
  bool keepVector = false;
  if (index >= 0) {
    keepVector = true;
  }

  if (!keepVector) {
    return false;
  }
  if (copy) {
    ret = NEW(VectorVariant)(this, index, v);
    return true;
  }

  if ((ssize_t)index < size()) {
    ASSERT(index >= 0);
    *m_elems[index] = v;
  } else {
    ASSERT((ssize_t)index == size());
    m_elems.push_back(ArrayFuncs::element(v));
  }
  ret = NULL;
  return true;
}

ArrayData *VectorVariant::removeImpl(int index, bool copy) {
  if (index >= 0) {
    if (size() == 1) {
      return StaticEmptyArray::Get();
    }
    if (copy || (ssize_t)index < size() - 1) {
      return NEW(MapVariant)(this, index);
    }
    m_elems[index]->release();
    m_elems.remove(index);
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
