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

bool BaseVector::t_isempty() {
  return !toBoolImpl();
}

int64_t BaseVector::t_count() {
  return m_size;
}

Object BaseVector::t_items() {
  return SystemLib::AllocLazyIterableViewObject(this);
}

// ConstIndexAccess

bool BaseVector::t_containskey(CVarRef key) {
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

Variant BaseVector::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return uninit_null();
}

Variant BaseVector::t_get(CVarRef key) {
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

Object BaseVector::t_getiterator() {
  c_VectorIterator* it = NEWOBJ(c_VectorIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  it->m_version = getVersion();
  return it;
}

Object BaseVector::t_map(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  uint sz = m_size;
  vec->reserve(sz);
  for (uint i = 0; i < sz; ++i) {
    TypedValue* tv = &vec->m_data[i];
    int32_t version = m_version;
    g_vmContext->invokeFuncFew(tv, ctx, 1, &m_data[i]);
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    ++vec->m_size;
  }
  return obj;
}

Object BaseVector::t_mapwithkey(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  uint sz = m_size;
  vec->reserve(sz);
  for (uint i = 0; i < sz; ++i) {
    TypedValue* tv = &vec->m_data[i];
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
    ++vec->m_size;
  }
  return obj;
}

Object BaseVector::t_filter(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 1, &m_data[i]);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (ret.toBoolean()) {
      vec->add(&m_data[i]);
    }
  }
  return obj;
}

Object BaseVector::t_filterwithkey(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
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
      vec->add(&m_data[i]);
    }
  }
  return obj;
}

Object BaseVector::t_zip(CVarRef iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  uint sz = m_size;
  vec->reserve(std::min(itSize, size_t(sz)));
  for (uint i = 0; i < sz && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (vec->m_capacity <= vec->m_size) {
      vec->grow();
    }
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->initAdd(&m_data[i]);
    pair->initAdd(cvarToCell(&v));
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
  }
  return obj;
}

Object BaseVector::t_kvzip() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  for (uint i = 0; i < m_size; ++i) {
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->elm0.m_type = KindOfInt64;
    pair->elm0.m_data.num = i;
    ++pair->m_size;
    pair->initAdd(&m_data[i]);
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
  }
  return obj;
}

Object BaseVector::t_keys() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  vec->m_size = m_size;
  for (uint i = 0; i < m_size; ++i) {
    vec->m_data[i].m_data.num = i;
    vec->m_data[i].m_type = KindOfInt64;
  }
  return obj;
}

// Others

void BaseVector::t___construct(CVarRef iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

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
  return result ? !tvIsNull(tvToCell(result)) : false;
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

// Helpers

Array BaseVector::toArrayImpl() const {
  PackedArrayInit ai(m_size);
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    ai.append(tvAsCVarRef(&m_data[i]));
  }
  return ai.toArray();
}

void BaseVector::freeData() {
  if (m_data) {
    smart_free(m_data);
    m_data = nullptr;
  }
}

void BaseVector::grow() {
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

    m_capacity = sz;
    m_data =
      (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
  }
}

BaseVector::BaseVector(Class* cls) : ExtObjectData(cls),
    m_size(0), m_data(nullptr), m_capacity(0), m_version(0) {
}

BaseVector::~BaseVector() {
  for (uint i = 0; i < m_size; ++i) {
    tvRefcountedDecRef(&m_data[i]);
  }

  freeData();
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

///////////////////////////////////////////////////////////////////////////////
}
