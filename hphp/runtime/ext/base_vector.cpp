/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/base_vector.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/ext_collections.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// ConstCollection

bool BaseVector::isempty() {
  return !toBoolImpl();
}

int64_t BaseVector::count() {
  return m_size;
}

Object BaseVector::items() {
  return SystemLib::AllocLazyIterableViewObject(this);
}

// ConstIndexAccess

bool BaseVector::containskey(CVarRef key) {
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

Variant BaseVector::at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return uninit_null();
}

Variant BaseVector::get(CVarRef key) {
  if (key.isInteger()) {
    TypedValue* tv = get(key.toInt64());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return uninit_null();
    }
  }
  throwBadKeyType();
  return uninit_null();
}

// KeyedIterable

Object BaseVector::getiterator() {
  c_VectorIterator* it = NEWOBJ(c_VectorIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  it->m_version = getVersion();
  return it;
}

void BaseVector::map(BaseVector* bvec, CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  uint sz = m_size;
  bvec->reserve(sz);
  for (uint i = 0; i < sz; ++i) {
    TypedValue* tv = &bvec->m_data[i];
    int32_t version = m_version;
    g_vmContext->invokeFuncFew(tv, ctx, 1, &m_data[i]);
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    ++bvec->m_size;
  }
}

void BaseVector::mapwithkey(BaseVector* bvec, CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  uint sz = m_size;
  bvec->reserve(sz);
  for (uint i = 0; i < sz; ++i) {
    TypedValue* tv = &bvec->m_data[i];
    int32_t version = m_version;
    TypedValue args[2] = {
      make_tv<KindOfInt64>(i),
      m_data[i]
    };
    g_vmContext->invokeFuncFew(tv, ctx, 2, args);
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    ++bvec->m_size;
  }
}

void BaseVector::filter(BaseVector* bvec, CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 1, &m_data[i]);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (ret.toBoolean()) {
      bvec->add(&m_data[i]);
    }
  }
}

void BaseVector::filterwithkey(BaseVector* bvec, CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    Variant ret;
    int32_t version = m_version;
    TypedValue args[2] = {
      make_tv<KindOfInt64>(i),
      m_data[i]
    };
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 2, args);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (ret.toBoolean()) {
      bvec->add(&m_data[i]);
    }
  }
}

void BaseVector::zip(BaseVector* bvec, CVarRef iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  uint sz = m_size;
  bvec->reserve(std::min(itSize, size_t(sz)));
  for (uint i = 0; i < sz && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (bvec->m_capacity <= bvec->m_size) {
      bvec->grow();
    }
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->initAdd(&m_data[i]);
    pair->initAdd(cvarToCell(&v));
    bvec->m_data[i].m_data.pobj = pair;
    bvec->m_data[i].m_type = KindOfObject;
    ++bvec->m_size;
  }
}

void BaseVector::kvzip(BaseVector* bvec) {
  bvec->reserve(m_size);
  for (uint i = 0; i < m_size; ++i) {
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->elm0.m_type = KindOfInt64;
    pair->elm0.m_data.num = i;
    ++pair->m_size;
    pair->initAdd(&m_data[i]);
    bvec->m_data[i].m_data.pobj = pair;
    bvec->m_data[i].m_type = KindOfObject;
    ++bvec->m_size;
  }
}

void BaseVector::keys(BaseVector* bvec) {
  bvec->reserve(m_size);
  bvec->m_size = m_size;
  for (uint i = 0; i < m_size; ++i) {
    bvec->m_data[i].m_data.num = i;
    bvec->m_data[i].m_type = KindOfInt64;
  }
}

// Others

void BaseVector::construct(CVarRef iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Object BaseVector::lazy() {
  return SystemLib::AllocLazyKeyedIterableViewObject(this);
}

Array BaseVector::toarray() {
  return toArrayImpl();
}

Array BaseVector::tokeysarray() {
  PackedArrayInit ai(m_size);
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    ai.append((int64_t)i);
  }
  return ai.toArray();
}

Array BaseVector::tovaluesarray() {
  return toArrayImpl();
}

int64_t BaseVector::linearsearch(CVarRef search_value) {
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    if (same(search_value, tvAsCVarRef(&m_data[i]))) {
      return i;
    }
  }
  return -1;
}

// Non PHP-land methods.

bool BaseVector::OffsetIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<BaseVector*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = vec->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellIsNull(tvToCell(result)) : false;
}

bool BaseVector::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<BaseVector*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = vec->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellToBool(*result) : true;
}

bool BaseVector::OffsetContains(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<BaseVector*>(obj);
  if (key->m_type == KindOfInt64) {
    return vec->contains(key->m_data.num);
  } else {
    throwBadKeyType();
    return false;
  }
}

TypedValue* BaseVector::OffsetGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<BaseVector*>(obj);
  if (key->m_type == KindOfInt64) {
    return vec->at(key->m_data.num);
  }
  throwBadKeyType();
  return nullptr;
}

bool BaseVector::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto bv1 = static_cast<const BaseVector*>(obj1);
  auto bv2 = static_cast<const BaseVector*>(obj2);

  uint sz = bv1->m_size;
  if (sz != bv2->m_size) {
    return false;
  }

  for (uint i = 0; i < sz; ++i) {
    if (!equal(tvAsCVarRef(&bv1->m_data[i]),
               tvAsCVarRef(&bv2->m_data[i]))) {

      return false;
    }
  }

  return true;
}

void BaseVector::Unserialize(const char* vectorType,
                             ObjectData* obj,
                             VariableUnserializer* uns,
                             int64_t sz,
                             char type) {
  if (type != 'V') {
    throw Exception("%s does not support the '%c' serialization "
                    "format", vectorType, type);
  }
  auto bvec = static_cast<BaseVector*>(obj);
  bvec->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    auto tv = &bvec->m_data[bvec->m_size];
    tv->m_type = KindOfNull;
    ++bvec->m_size;
    tvAsVariant(tv).unserialize(uns, Uns::Mode::ColValue);
  }
}

// Helpers

Array BaseVector::toArrayImpl() const {
  PackedArrayInit ai(m_size);
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    ai.append(tvAsCVarRef(&m_data[i]));
  }
  return ai.toArray();
}

void BaseVector::grow() {
  mutate();

  if (m_capacity) {
    m_capacity += m_capacity;
  } else {
    m_capacity = 8;
  }
  m_data = (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
}

void BaseVector::reserve(int64_t sz) {
  if (sz <= 0) return;

  if (m_capacity < sz) {
    ++m_version;
    mutate();

    m_capacity = sz;
    m_data =
      (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
  }
}

BaseVector::BaseVector(Class* cls) : ExtObjectData(cls),
    m_size(0), m_data(nullptr), m_capacity(0),
    m_version(0), m_frozenCopy(nullptr) {
}

/**
 * Delegate the responsibility for freeing the buffer to the
 * frozen copy, if it exists.
 */
BaseVector::~BaseVector() {
  if (m_frozenCopy.isNull() && m_data) {
    for (uint i = 0; i < m_size; ++i) {
      tvRefcountedDecRef(&m_data[i]);
    }

    smart_free(m_data);
    m_data = nullptr;
  }
}

void BaseVector::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys may be used with Vectors"));
  throw e;
}

void BaseVector::init(CVarRef t) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  if (sz) {
    reserve(sz);
  }
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    add(tv);
  }
}

void BaseVector::cow() {
  TypedValue* newData =
      (TypedValue*)smart_malloc(m_capacity * sizeof(TypedValue));

  assert(newData);

  for (uint i = 0; i < m_size; i++) {
    cellDup(m_data[i], newData[i]);
  }

  m_data = newData;
  m_frozenCopy.reset();
}

///////////////////////////////////////////////////////////////////////////////
}
