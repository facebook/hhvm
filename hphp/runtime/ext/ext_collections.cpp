/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"

#include <folly/ScopeGuard.h>
#include <algorithm>
#include <array>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * The "materialization" methods have the form "to[CollectionName]()" and
 * allow us to get an instance of a collection type from another.
 * This template provides a default implementation.
 */
template<typename TCollection>
inline static Object materializeDefaultImpl(ObjectData* obj) {
  auto* col = NEWOBJ(TCollection)();
  Object o = col;
  col->init(VarNR(obj));
  return o;
}

///////////////////////////////////////////////////////////////////////////////

static void throwIntOOB(int64_t key, bool isVector = false)
  ATTRIBUTE_NORETURN;

void throwIntOOB(int64_t key, bool isVector /* = false */) {
  static const size_t reserveSize = 50;
  String msg(reserveSize, ReserveString);
  char* buf = msg.bufferSlice().ptr;
  int sz = sprintf(buf, "Integer key %" PRId64 " is %s", key,
                   isVector ? "out of bounds" : "not defined");
  assert(sz <= reserveSize);
  msg.setSize(sz);
  Object e(SystemLib::AllocOutOfBoundsExceptionObject(msg));
  throw e;
}

void throwOOB(int64_t key) {
  throwIntOOB(key, true);
}

static void throwStrOOB(StringData* key) ATTRIBUTE_NORETURN;

void throwStrOOB(StringData* key) {
  const size_t maxDisplaySize = 100;
  int keySize = key->size();
  bool keyIsLarge = (keySize > maxDisplaySize);
  const char* part1 = "String key \"";
  const char* part3 = keyIsLarge ? "\" (truncated) is not defined" :
                                   "\" is not defined";
  StringSlice ss1(part1, strlen(part1));
  StringSlice ss2(key->data(), keyIsLarge ? maxDisplaySize : keySize);
  StringSlice ss3(part3, strlen(part3));
  String msg(ss1.len + ss2.len + ss3.len, ReserveString);
  msg += ss1;
  msg += ss2;
  msg += ss3;
  Object e(SystemLib::AllocOutOfBoundsExceptionObject(msg));
  throw e;
}

ArrayIter getArrayIterHelper(const Variant& v, size_t& sz) {
  if (v.isArray()) {
    ArrayData* ad = v.getArrayData();
    sz = ad->size();
    return ArrayIter(ad);
  }
  if (v.isObject()) {
    ObjectData* obj = v.getObjectData();
    if (obj->isCollection()) {
      sz = getCollectionSize(obj);
      return ArrayIter(obj);
    }
    bool isIterable;
    Object iterable = obj->iterableObject(isIterable);
    if (isIterable) {
      sz = 0;
      return ArrayIter(iterable.detach(), ArrayIter::noInc);
    }
  }
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Parameter must be an array or an instance of Traversable"));
  throw e;
}

void triggerCow(c_Vector* vec) {
  assert(vec->hasImmutableBuffer()); // Should've been checked by the JIT.
  vec->mutate();
}

namespace {

ALWAYS_INLINE
void* reallocHelper(void* ptr, size_t oldSize, size_t newSize) {
  assert(oldSize > 0 || !ptr);
  assert(newSize > 0);

  auto retptr = MM().objMallocLogged(newSize);

  if (ptr) {
    auto const copySize = std::min(oldSize, newSize);
    retptr = memcpy(retptr, ptr, copySize);
    MM().objFreeLogged(ptr, oldSize);
  }

  return retptr;
}

}

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
bool BaseVector::containskey(const Variant& key) {
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

Variant BaseVector::at(const Variant& key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return uninit_null();
}

Variant BaseVector::get(const Variant& key) {
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
  auto* it = NEWOBJ(c_VectorIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  it->m_version = getVersion();
  return it;
}

ALWAYS_INLINE
static std::array<TypedValue, 1> makeArgsFromVectorValue(
  TypedValue val, uint index) {
  return std::array<TypedValue, 1>{{ val }};
}

ALWAYS_INLINE
static std::array<TypedValue, 2> makeArgsFromVectorKeyAndValue(
  TypedValue val, uint index) {
  return std::array<TypedValue, 2> {{
    make_tv<KindOfInt64>(index),
    val
  }};
}

template<class TVector, class MakeArgs>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_map(const Variant& callback, MakeArgs makeArgs) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }

  TVector* nv = NEWOBJ(TVector);
  uint sz = m_size;
  nv->reserve(sz);
  for (uint i = 0; i < sz; ++i) {
    TypedValue* tv = &nv->m_data[i];
    int32_t version = m_version;
    auto args = makeArgs(m_data[i], i);
    g_context->invokeFuncFew(tv, ctx, args.size(), &args[0]);
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    ++nv->m_size;
  }
  return nv;
}

template<class TVector, class MakeArgs>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_filter(const Variant& callback, MakeArgs makeArgs) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  TVector* nv = NEWOBJ(TVector);
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    Variant ret;
    int32_t version = m_version;
    auto args = makeArgs(m_data[i], i);
    g_context->invokeFuncFew(ret.asTypedValue(), ctx, args.size(), &args[0]);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (ret.toBoolean()) {
      nv->add(&m_data[i]);
    }
  }
  return nv;
}

Object c_Vector::ti_slice(const Variant& vec, const Variant& offset,
                          const Variant& len /* = uninit_null() */) {
  ObjectData* obj;
  if (!vec.isObject() ||
      (obj = vec.getObjectData())->getVMClass() != c_Vector::classof()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter 1 must be an instance of Vector"));
    throw e;
  }
  if (!offset.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter 2 must be an integer"));
    throw e;
  }
  if (!len.isNull() && !len.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter 3 must be null or an integer"));
    throw e;
  }
  auto* target = NEWOBJ(c_Vector)();
  Object ret = target;
  auto* v = static_cast<c_Vector*>(obj);
  int64_t sz = v->m_size;
  int64_t startPos = offset.toInt64();
  if (UNLIKELY(uint64_t(startPos) >= uint64_t(sz))) {
    if (startPos >= 0) {
      assert(startPos >= sz);
      return ret;
    }
    startPos = std::max<int64_t>(sz + startPos, 0);
  }
  int64_t endPos;
  if (len.isInteger()) {
    int64_t intLen = len.toInt64();
    if (LIKELY(intLen >= 0)) {
      endPos = startPos + std::min<int64_t>(intLen, sz - startPos);
    } else {
      endPos = sz + intLen;
    }
  } else {
    endPos = sz;
  }
  if (startPos >= endPos) {
    return ret;
  }
  uint targetSize = endPos - startPos;
  target->reserve(targetSize);
  target->m_size = targetSize;
  auto* data = target->m_data;
  for (uint i = 0; i < targetSize; ++i, ++startPos) {
    cellDup(v->m_data[startPos], data[i]);
  }
  return ret;
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_take(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  if (len <= 0) {
    return obj;
  }
  size_t sz = std::min(size_t(len), size_t(m_size));
  vec->reserve(sz);
  vec->m_size = sz;
  for (size_t i = 0; i < sz; ++i) {
    cellDup(m_data[i], vec->m_data[i]);
  }
  return obj;
}

template<class TVector, bool checkVersion>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  for (uint i = 0; i < m_size; ++i) {
    Variant retval;
    if (checkVersion) {
      int32_t version = m_version;
      g_context->invokeFuncFew(retval.asTypedValue(), ctx, 1, &m_data[i]);
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    } else {
      g_context->invokeFuncFew(retval.asTypedValue(), ctx, 1, &m_data[i]);
    }
    if (!retval.toBoolean()) break;
    vec->add(&m_data[i]);
  }
  return obj;
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  if (len <= 0) len = 0;
  size_t skipAmt = std::min<size_t>(len, m_size);
  size_t sz = size_t(m_size) - skipAmt;
  vec->reserve(sz);
  vec->m_size = sz;
  for (size_t i = 0; i < sz; ++i) {
    cellDup(m_data[i + skipAmt], vec->m_data[i]);
  }
  return obj;
}

template<class TVector, bool checkVersion>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  uint i = 0;
  for (; i < m_size; ++i) {
    Variant retval;
    if (checkVersion) {
      int32_t version = m_version;
      g_context->invokeFuncFew(retval.asTypedValue(), ctx, 1, &m_data[i]);
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    } else {
      g_context->invokeFuncFew(retval.asTypedValue(), ctx, 1, &m_data[i]);
    }
    if (!retval.toBoolean()) break;
  }
  for (; i < m_size; ++i) {
    vec->add(&m_data[i]);
  }
  return obj;
}

void BaseVector::zip(BaseVector* bvec, const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  uint sz = m_size;
  bvec->reserve(std::min(itSize, size_t(sz)));
  for (uint i = 0; i < sz && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (bvec->m_capacity <= bvec->m_size) {
      bvec->grow();
    }
    auto* pair = NEWOBJ(c_Pair)();
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
    auto* pair = NEWOBJ(c_Pair)();
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

void BaseVector::construct(const Variant& iterable /* = null_variant */) {
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

int64_t BaseVector::linearsearch(const Variant& search_value) {
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

template <bool throwOnMiss>
TypedValue* BaseVector::OffsetAt(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<BaseVector*>(obj);
  if (key->m_type == KindOfInt64) {
    return throwOnMiss ? vec->at(key->m_data.num)
                       : vec->get(key->m_data.num);
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

  if (m_capacity == MaxCapacity()) {
    return;
  }

  auto const oldSize = m_capacity * sizeof(TypedValue);
  if (m_capacity) {
    m_capacity = std::min(uint64_t(m_capacity) * 2, MaxCapacity());
  } else {
    m_capacity = 8;
  }
  m_data = (TypedValue*)reallocHelper(m_data, oldSize,
                                      m_capacity * sizeof(TypedValue));
}

void BaseVector::addFront(TypedValue* val) {
  assert(val->m_type != KindOfRef);
  ++m_version;
  mutate();
  if (m_capacity <= m_size) {
    grow();
  }
  memmove(m_data+1, m_data, m_size * sizeof(TypedValue));
  cellDup(*val, m_data[0]);
  ++m_size;
}

Variant BaseVector::popFront() {
  if (m_size) {
    ++m_version;
    mutate();
    Variant ret = tvAsCVarRef(&m_data[0]);
    tvRefcountedDecRef(&m_data[0]);
    --m_size;
    memmove(m_data, m_data+1, m_size * sizeof(TypedValue));
    return ret;
  } else {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Cannot pop empty Vector"));
    throw e;
  }
}

void BaseVector::reserve(int64_t sz) {
  if (sz <= 0) return;

  if (m_capacity < sz) {
    ++m_version;
    mutate();

    m_data = (TypedValue*)reallocHelper(m_data, m_capacity * sizeof(TypedValue),
                                        sz * sizeof(TypedValue));
    m_capacity = sz;
  }
}

BaseVector::BaseVector(Class* cls)
    : ExtCollectionObjectData(cls)
    , m_size(0), m_data(nullptr), m_capacity(0)
    , m_version(0), m_immCopy(nullptr) {
}

/**
 * Delegate the responsibility for freeing the buffer to the immutable copy,
 * if it exists.
 */
BaseVector::~BaseVector() {
  if (!hasImmutableBuffer() && m_data) {
    assert(m_immCopy.isNull());
    for (uint i = 0; i < m_size; ++i) {
      tvRefcountedDecRef(&m_data[i]);
    }

    if (m_data) {
      MM().objFreeLogged(m_data, m_capacity * sizeof(TypedValue));
    }
    m_data = nullptr;
  }
}

void BaseVector::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
             "Only integer keys may be used with Vectors"));
  throw e;
}

void BaseVector::init(const Variant& t) {
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
  assert(hasImmutableBuffer());
  if (!m_size) {
    m_data = nullptr;
    m_capacity = 0;
    m_immCopy.reset();
    return;
  }

  TypedValue* newData =
    (TypedValue*)MM().objMallocLogged(m_capacity * sizeof(TypedValue));

  assert(newData);

  for (uint i = 0; i < m_size; i++) {
    cellDup(m_data[i], newData[i]);
  }

  m_data = newData;
  m_immCopy.reset();
}


///////////////////////////////////////////////////////////////////////////////

c_Vector::c_Vector(Class* cls /* = c_Vector::classof() */) : BaseVector(cls) {
  o_subclassData.u16 = Collection::VectorType;
}

void c_Vector::t___construct(const Variant& iterable /* = null_variant */) {
  BaseVector::construct(iterable);
}

void c_Vector::resize(int64_t sz, TypedValue* val) {
  assert(val && val->m_type != KindOfRef);
  ++m_version;
  mutate();

  assert(sz >= 0);
  uint requestedSize = (uint)sz;
  if (m_capacity < requestedSize) {
    m_data = (TypedValue*)
      reallocHelper(m_data, m_capacity * sizeof(TypedValue),
                    requestedSize * sizeof(TypedValue));
    m_capacity = requestedSize;
  }
  if (m_size > requestedSize) {
    do {
      --m_size;
      tvRefcountedDecRef(&m_data[m_size]);
    } while (m_size > requestedSize);
  } else {
    for (; m_size < requestedSize; ++m_size) {
      cellDup(*val, m_data[m_size]);
    }
  }
}

Object c_Vector::t_add(const Variant& val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_Vector::t_addall(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  if (sz) {
    reserve(m_size + sz);
  }
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    add(tv);
  }
  return this;
}

Object c_Vector::t_append(const Variant& val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Variant c_Vector::t_pop() {
  if (m_size) {
    ++m_version;
    mutate();
    --m_size;
    Variant ret = tvAsCVarRef(&m_data[m_size]);
    tvRefcountedDecRef(&m_data[m_size]);
    return ret;
  } else {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Cannot pop empty Vector"));
    throw e;
  }
}

int64_t c_Vector::checkRequestedCapacity(const Variant& sz) {
  if (!sz.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter sz must be a non-negative integer"));
    throw e;
  }
  int64_t intSz = sz.toInt64();
  if (intSz < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter sz must be a non-negative integer"));
    throw e;
  }
  if (intSz > MaxCapacity()) {
    auto msg = folly::format(
      "Parameter sz must be at most {}; {} passed",
      MaxCapacity(),
      intSz
    ).str();
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(msg));
    throw e;
  }
  return intSz;
}

void c_Vector::t_resize(const Variant& sz, const Variant& value) {
  auto intSz = checkRequestedCapacity(sz);
  TypedValue* val = cvarToCell(&value);
  resize(intSz, val);
}

void c_Vector::t_reserve(const Variant& sz) {
  auto intSz = checkRequestedCapacity(sz);
  reserve(intSz);
}

Object c_Vector::t_clear() {
  ++m_version;
  mutate();

  uint sz = m_size;
  for (int i = 0; i < sz; ++i) {
    tvRefcountedDecRef(&m_data[i]);
  }
  if (m_data) {
    MM().objFreeLogged(m_data, m_capacity * sizeof(TypedValue));
  }
  m_data = nullptr;
  m_size = 0;
  m_capacity = 0;
  return this;
}

bool c_Vector::t_isempty() {
  return BaseVector::isempty();
}

int64_t c_Vector::t_count() {
  return BaseVector::count();
}

Object c_Vector::t_items() {
  return BaseVector::items();
}

Object c_Vector::t_keys() {
  auto* vec = NEWOBJ(c_Vector);
  Object obj = vec;
  BaseVector::keys(vec);
  return obj;
}

Object c_Vector::t_values() {
  return Object::attach(BaseVector::Clone<c_Vector>(this));
}

Object c_Vector::t_lazy() {
  return BaseVector::lazy();
}

Object c_Vector::t_kvzip() {
  auto* vec = NEWOBJ(c_Vector);
  Object obj = vec;
  BaseVector::kvzip(vec);
  return obj;
}

Variant c_Vector::t_at(const Variant& key) {
  return BaseVector::at(key);
}

Variant c_Vector::t_get(const Variant& key) {
  return BaseVector::get(key);
}

bool c_Vector::t_contains(const Variant& key) {
  return t_containskey(key);
}

bool c_Vector::t_containskey(const Variant& key) {
  return BaseVector::containskey(key);
}

Object c_Vector::t_removekey(const Variant& key) {
  if (!key.isInteger()) {
    throwBadKeyType();
  }
  int64_t k = key.toInt64();
  if (!contains(k)) {
    return this;
  }
  mutate();
  uint64_t datum = m_data[k].m_data.num;
  DataType t = m_data[k].m_type;
  if (k+1 < m_size) {
    memmove(&m_data[k], &m_data[k+1],
            (m_size-(k+1)) * sizeof(TypedValue));
  }
  --m_size;
  tvRefcountedDecRefHelper(t, datum);
  return this;
}

Array c_Vector::t_toarray() {
  return BaseVector::toarray();
}

Array c_Vector::t_tokeysarray() {
  return BaseVector::tokeysarray();
}

Array c_Vector::t_tovaluesarray() {
  return BaseVector::tovaluesarray();
}

void c_Vector::t_reverse() {
  if (m_size <= 1) return;
  mutate();
  TypedValue* start = m_data;
  TypedValue* end = m_data + m_size - 1;
  for (; start < end; ++start, --end) {
    std::swap(start->m_data.num, end->m_data.num);
    std::swap(start->m_type, end->m_type);
  }
}

void c_Vector::t_splice(const Variant& offset, const Variant& len /* = null */,
                        const Variant& replacement /* = null */) {
  if (!offset.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter offset must be an integer"));
    throw e;
  }
  if (!len.isNull() && !len.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter len must be null or an integer"));
    throw e;
  }
  if (!replacement.isNull()) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Vector::splice does not support replacement parameter"));
    throw e;
  }
  mutate();
  int64_t sz = m_size;
  int64_t startPos = offset.toInt64();
  if (UNLIKELY(uint64_t(startPos) >= uint64_t(sz))) {
    if (startPos >= 0) {
      return;
    }
    startPos += sz;
    if (startPos < 0) {
      startPos = 0;
    }
  }
  int64_t endPos;
  if (len.isInteger()) {
    int64_t intLen = len.toInt64();
    if (LIKELY(intLen > 0)) {
      endPos = startPos + intLen;
      if (endPos > sz) {
        endPos = sz;
      }
    } else {
      if (intLen == 0) {
        return;
      }
      endPos = sz + intLen;
      if (endPos <= startPos) {
        return;
      }
    }
  } else {
    endPos = sz;
  }
  // Null out each element before decreffing it. We need to do this in case
  // a __destruct method reenters and accesses this Vector object.
  for (int64_t i = startPos; i < endPos; ++i) {
    uint64_t datum = m_data[i].m_data.num;
    DataType t = m_data[i].m_type;
    tvWriteNull(&m_data[i]);
    tvRefcountedDecRefHelper(t, datum);
  }
  // Move elements that came after the deleted elements (if there are any)
  if (endPos < sz) {
    memmove(&m_data[startPos], &m_data[endPos],
            (sz - endPos) * sizeof(TypedValue));
  }
  m_size -= (endPos - startPos);
}

int64_t c_Vector::t_linearsearch(const Variant& search_value) {
  return BaseVector::linearsearch(search_value);
}

void c_Vector::t_shuffle() {
  mutate();
  for (uint i = 1; i < m_size; ++i) {
    uint j = f_mt_rand(0, i);
    std::swap(m_data[i], m_data[j]);
  }
}

Object c_Vector::t_getiterator() {
  return BaseVector::getiterator();
}

Object c_Vector::t_map(const Variant& callback) {
  return BaseVector::php_map<c_Vector>(
    callback, &makeArgsFromVectorValue);
}

Object c_Vector::t_mapwithkey(const Variant& callback) {
  return BaseVector::php_map<c_Vector>(
    callback, &makeArgsFromVectorKeyAndValue);
}

Object c_Vector::t_filter(const Variant& callback) {
  return BaseVector::php_filter<c_Vector>(
    callback, &makeArgsFromVectorValue);
}

Object c_Vector::t_filterwithkey(const Variant& callback) {
  return BaseVector::php_filter<c_Vector>(
    callback, &makeArgsFromVectorKeyAndValue);
}

Object c_Vector::t_zip(const Variant& iterable) {
  auto* vec = NEWOBJ(c_Vector);
  Object obj = vec;
  BaseVector::zip(vec, iterable);
  return obj;
}

Object c_Vector::t_take(const Variant& n) {
  return BaseVector::php_take<c_Vector>(n);
}

Object c_ImmVector::t_take(const Variant& n) {
  return BaseVector::php_take<c_ImmVector>(n);
}

Object c_Vector::t_takewhile(const Variant& fn) {
  return BaseVector::php_takeWhile<c_Vector, true>(fn);
}

Object c_ImmVector::t_takewhile(const Variant& fn) {
  return BaseVector::php_takeWhile<c_ImmVector, false>(fn);
}

Object c_Vector::t_skip(const Variant& n) {
  return BaseVector::php_skip<c_Vector>(n);
}

Object c_ImmVector::t_skip(const Variant& n) {
  return BaseVector::php_skip<c_ImmVector>(n);
}

Object c_Vector::t_skipwhile(const Variant& fn) {
  return BaseVector::php_skipWhile<c_Vector, true>(fn);
}

Object c_ImmVector::t_skipwhile(const Variant& fn) {
  return BaseVector::php_skipWhile<c_ImmVector, false>(fn);
}

Object c_Vector::t_set(const Variant& key, const Variant& value) {
  if (key.isInteger()) {
    TypedValue* tv = cvarToCell(&value);
    set(key.toInt64(), tv);
    return this;
  }
  throwBadKeyType();
  return this;
}

Object c_Vector::t_setall(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tvKey = cvarToCell(&k);
    TypedValue* tvVal = cvarToCell(&v);
    if (tvKey->m_type != KindOfInt64) {
      throwBadKeyType();
    }
    set(tvKey->m_data.num, tvVal);
  }
  return this;
}

Object c_Vector::ti_fromitems(const Variant& iterable) {
  if (iterable.isNull()) return NEWOBJ(c_Vector)();
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  for (uint i = 0; iter; ++i, ++iter) {
    Variant v = iter.second();
    TypedValue* tv = v.asCell();
    target->add(tv);
  }
  return ret;
}

Object c_Vector::ti_fromarray(const Variant& arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  ArrayData* ad = arr.getArrayData();
  uint sz = ad->size();
  if (!sz) {
    return ret;
  }
  target->m_capacity = target->m_size = sz;
  TypedValue* data;
  target->m_data = data =
    (TypedValue*)MM().objMallocLogged(size_t(sz) * sizeof(TypedValue));
  ssize_t pos = ad->iter_begin();
  for (uint i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    assert(pos != ArrayData::invalid_index);
    cellDup(*cvarToCell(&ad->getValueRef(pos)), data[i]);
  }
  return ret;
}

void c_Vector::throwOOB(int64_t key) {
  throwIntOOB(key, true);
}

struct VectorValAccessor {
  typedef const TypedValue& ElmT;
  bool isInt(ElmT elm) const { return elm.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return IS_STRING_TYPE(elm.m_type); }
  int64_t getInt(ElmT elm) const { return elm.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm); }
};

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
c_Vector::SortFlavor c_Vector::preSort(const AccessorT& acc) {
  assert(m_size > 0);
  uint sz = m_size;
  bool allInts = true;
  bool allStrs = true;
  for (uint i = 0; i < sz; ++i) {
    allInts = (allInts && acc.isInt(m_data[i]));
    allStrs = (allStrs && acc.isStr(m_data[i]));
  }
  return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
}

#define SORT_CASE(flag, cmp_type, acc_type) \
  case flag: { \
    if (ascending) { \
      cmp_type##Compare<acc_type, flag, true> comp; \
      HPHP::Sort::sort(m_data, m_data + m_size, comp); \
    } else { \
      cmp_type##Compare<acc_type, flag, false> comp; \
      HPHP::Sort::sort(m_data, m_data + m_size, comp); \
    } \
    break; \
  }
#define SORT_CASE_BLOCK(cmp_type, acc_type) \
  switch (sort_flags) { \
    default: /* fall through to SORT_REGULAR case */ \
    SORT_CASE(SORT_REGULAR, cmp_type, acc_type) \
    SORT_CASE(SORT_NUMERIC, cmp_type, acc_type) \
    SORT_CASE(SORT_STRING, cmp_type, acc_type) \
    SORT_CASE(SORT_LOCALE_STRING, cmp_type, acc_type) \
    SORT_CASE(SORT_NATURAL, cmp_type, acc_type) \
    SORT_CASE(SORT_NATURAL_CASE, cmp_type, acc_type) \
  }
#define CALL_SORT(acc_type) \
  if (flav == StringSort) { \
    SORT_CASE_BLOCK(StrElm, acc_type) \
  } else if (flav == IntegerSort) { \
    SORT_CASE_BLOCK(IntElm, acc_type) \
  } else { \
    SORT_CASE_BLOCK(Elm, acc_type) \
  }

void c_Vector::sort(int sort_flags, bool ascending) {
  mutate();
  if (!m_size) {
    return;
  }
  SortFlavor flav = preSort<VectorValAccessor>(VectorValAccessor());
  CALL_SORT(VectorValAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

bool c_Vector::usort(const Variant& cmp_function) {
  if (!m_size) {
    return true;
  }
  ElmUCompare<VectorValAccessor> comp;
  CallCtx ctx;
  JIT::CallerFrame cf;
  vm_decode_function(cmp_function, cf(), false, ctx);
  if (!ctx.func) {
    return false;
  }
  comp.ctx = &ctx;
  HPHP::Sort::sort(m_data, m_data + m_size, comp);
  return true;
}

void c_Vector::OffsetSet(ObjectData* obj, const TypedValue* key,
                         TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  auto vec = static_cast<c_Vector*>(obj);
  if (key->m_type == KindOfInt64) {
    vec->set(key->m_data.num, val);
    return;
  }
  throwBadKeyType();
}

void c_Vector::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot unset an element of a Vector"));
  throw e;
}

// This function will create a immutable copy of this Vector (if it doesn't
// already exist) and then return it
Object c_Vector::getImmutableCopy() {
  if (!hasImmutableBuffer()) {
    assert(m_immCopy.isNull());
    auto* vec = NEWOBJ(c_ImmVector)();
    m_immCopy = vec;
    vec->m_data = m_data;
    vec->m_size = m_size;
    vec->m_capacity = m_capacity;
    vec->m_version = m_version;
  }
  return m_immCopy;
}

Object c_Vector::t_tovector() {
  return materializeDefaultImpl<c_Vector>(this);
}

Object c_Vector::t_toset() {
  return materializeDefaultImpl<c_Set>(this);
}

Object c_Vector::t_toimmvector() {
  return getImmutableCopy();
}

Object c_Vector::t_immutable() {
  return getImmutableCopy();
}

Object c_Vector::t_tomap() {
  return materializeDefaultImpl<c_Map>(this);
}

Object c_Vector::t_toimmmap() {
  return materializeDefaultImpl<c_ImmMap>(this);
}

Object c_Vector::t_toimmset() {
  return materializeDefaultImpl<c_ImmSet>(this);
}

c_VectorIterator::c_VectorIterator(Class* cls
    /*= c_VectorIterator::classof()*/) : ExtObjectData(cls) {
}

c_VectorIterator::~c_VectorIterator() {
}

void c_VectorIterator::t___construct() {
}

Variant c_VectorIterator::t_current() {
  BaseVector* vec = m_obj.get();
  if (UNLIKELY(m_version != vec->getVersion())) {
    throw_collection_modified();
  }
  if (!vec->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(&vec->m_data[m_pos]);
}

Variant c_VectorIterator::t_key() {
  BaseVector* vec = m_obj.get();
  if (!vec->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return m_pos;
}

bool c_VectorIterator::t_valid() {
  assert(m_pos >= 0);
  BaseVector* vec = m_obj.get();
  return vec && (m_pos < (ssize_t)vec->m_size);
}

void c_VectorIterator::t_next() {
  m_pos++;
}

void c_VectorIterator::t_rewind() {
  m_pos = 0;
}

///////////////////////////////////////////////////////////////////////////////
// c_ImmVector

// ConstCollection

bool c_ImmVector::t_isempty() {
  return BaseVector::isempty();
}

int64_t c_ImmVector::t_count() {
  return BaseVector::count();
}

Object c_ImmVector::t_items() {
  return BaseVector::items();
}

// ConstIndexAccess

bool c_ImmVector::t_containskey(const Variant& key) {
  return BaseVector::containskey(key);
}

Variant c_ImmVector::t_at(const Variant& key) {
  return BaseVector::at(key);
}

Variant c_ImmVector::t_get(const Variant& key) {
  return BaseVector::get(key);
}

// KeyedIterable

Object c_ImmVector::t_getiterator() {
  return BaseVector::getiterator();
}

Object c_ImmVector::t_map(const Variant& callback) {
  return php_map<c_ImmVector>(callback, &makeArgsFromVectorValue);
}

Object c_ImmVector::t_mapwithkey(const Variant& callback) {
  return php_map<c_ImmVector>(callback, &makeArgsFromVectorKeyAndValue);
}

Object c_ImmVector::t_filter(const Variant& callback) {
  return php_filter<c_ImmVector>(callback, &makeArgsFromVectorValue);
}

Object c_ImmVector::t_filterwithkey(const Variant& callback) {
  return php_filter<c_ImmVector>(callback, &makeArgsFromVectorKeyAndValue);
}

Object c_ImmVector::t_zip(const Variant& iterable) {
  auto* vec = NEWOBJ(c_ImmVector);
  Object obj = vec;
  BaseVector::zip(vec, iterable);
  return obj;
}

Object c_ImmVector::t_kvzip() {
  auto* vec = NEWOBJ(c_ImmVector);
  Object obj = vec;
  BaseVector::kvzip(vec);
  return obj;
}

Object c_ImmVector::t_keys() {
  auto* vec = NEWOBJ(c_ImmVector);
  Object obj = vec;
  BaseVector::keys(vec);
  return obj;
}

// Others

void c_ImmVector::t___construct(const Variant& iterable /* = null_variant */) {
  BaseVector::construct(iterable);
}

Object c_ImmVector::t_lazy() {
  return BaseVector::lazy();
}

Array c_ImmVector::t_toarray() {
  return BaseVector::toarray();
}

Array c_ImmVector::t_tokeysarray() {
  return BaseVector::tokeysarray();
}

Array c_ImmVector::t_tovaluesarray() {
  return BaseVector::tovaluesarray();
}

int64_t c_ImmVector::t_linearsearch(const Variant& search_value) {
  return BaseVector::linearsearch(search_value);
}

Object c_ImmVector::t_values() {
  return Object::attach(BaseVector::Clone<c_ImmVector>(this));
}

Object c_ImmVector::t_immutable() {
  return this;
}

// Non PHP methods.

c_ImmVector::c_ImmVector(Class* cls) : BaseVector(cls) {
  o_subclassData.u16 = Collection::ImmVectorType;
}

///////////////////////////////////////////////////////////////////////////////

static int32_t empty_map_hash_slot = BaseMap::Empty;

c_Map::c_Map(Class* cls) : BaseMap(cls) {
  o_subclassData.u16 = Collection::MapType;
}

// Protected (Internal)

BaseMap::BaseMap(Class* cls) : ExtCollectionObjectData(cls) {
  uint32_t used = 0;
  uint32_t cap = 0;
  uint32_t tableMask = 0;
  int32_t version = 0;
  m_size = 0;
  m_capAndUsed = (uint64_t(cap) << 32) | uint64_t(used);
  m_maskAndVersion = (uint64_t(version) << 32) | uint64_t(tableMask);
  m_data = nullptr;
  m_hash = &empty_map_hash_slot;
}

BaseMap::~BaseMap() {
  deleteElms();
  freeData();
}

void BaseMap::freeData() {
  if (m_data) {
    MM().objFreeLogged(
      m_data, size_t(m_cap) * sizeof(Elm) + hashSize() * sizeof(int32_t));
  }
}

void BaseMap::deleteElms() {
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    tvRefcountedDecRef(&p->data);
    if (p->hasStrKey()) {
      decRefStr(p->skey);
    }
  }
}

void BaseMap::php_construct(const Variant& iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

void c_Map::t___construct(const Variant& iterable /* = null_variant */) {
  return php_construct(iterable);
}

Array BaseMap::php_toArray() const {
  ArrayInit ai(m_size);
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    if (p->hasIntKey()) {
      ai.set((int64_t)p->ikey, tvAsCVarRef(&p->data));
    } else {
      ai.set(*(const String*)(&p->skey), tvAsCVarRef(&p->data));
    }
  }
  return ai.create();
}

template<typename TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, TMap*>::type
BaseMap::Clone(ObjectData* obj) {
  auto thiz = static_cast<TMap*>(obj);
  auto target = static_cast<TMap*>(obj->cloneImpl());

  if (!thiz->m_size) return target;

  assert(target->m_size == 0);
  assert(thiz->m_used != 0);
  target->m_capAndUsed = thiz->m_capAndUsed;
  target->m_tableMask = thiz->m_tableMask;
  target->m_size = thiz->m_size;
  auto needed =
    size_t(thiz->m_cap) * sizeof(Elm) + thiz->hashSize() * sizeof(int32_t);
  target->m_data = (Elm*)MM().objMallocLogged(needed);
  target->m_hash = (int32_t*)(target->m_data + target->m_cap);
  wordcpy(target->hashTab(), thiz->hashTab(), thiz->hashSize());

  for (ssize_t i = 0; i < thiz->iterLimit(); ++i) {
    Elm& e = thiz->data()[i];
    Elm& te = target->data()[i];
    if (thiz->isTombstone(e.data.m_type)) {
      te.data.m_type = e.data.m_type;
      continue;
    }
    te.skey = e.skey;
    te.data.hash() = e.data.hash();
    if (te.hasStrKey()) te.skey->incRefCount();
    cellDup(e.data, te.data);
    assert(te.hash() == e.hash()); // ensure not clobbered.
  }

  return target;
}

c_Map* c_Map::Clone(ObjectData* obj) {
  return BaseMap::Clone<c_Map>(obj);
}

void BaseMap::init(const Variant& t) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  if (sz) {
    reserve(sz);
  }
  for (; iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = v.asCell();
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else if (k.isString()) {
      update(k.getStringData(), tv);
    } else {
      throwBadKeyType();
    }
  }
}

Object BaseMap::php_add(const Variant& val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_Map::t_add(const Variant& val) { return php_add(val); }

Object BaseMap::php_addAll(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  reserve(std::max(sz, size_t(m_size)));
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    add(tv);
  }
  return this;
}

Object c_Map::t_addall(const Variant& val) { return php_addAll(val); }

Object BaseMap::php_clear() {
  deleteElms();
  freeData();
  uint32_t used = 0;
  uint32_t cap = 0;
  uint32_t tableMask = 0;
  int32_t version = m_version + 1;
  m_size = 0;
  m_capAndUsed = (uint64_t(cap) << 32) | uint64_t(used);
  m_maskAndVersion = (uint64_t(version) << 32) | uint64_t(tableMask);
  m_data = nullptr;
  m_hash = &empty_map_hash_slot;
  return this;
}

Object c_Map::t_clear() { return php_clear(); }

bool c_ImmMap::t_isempty() { return php_isEmpty(); }

bool c_Map::t_isempty() { return php_isEmpty();  }

int64_t c_ImmMap::t_count() { return size(); }

int64_t c_Map::t_count() { return size(); }

Object c_ImmMap::t_items() { return php_items(); }

Object c_Map::t_items() { return php_items(); }

Object BaseMap::php_keys() const {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  if (!m_size) {
    return obj;
  }
  vec->reserve(m_size);
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  ssize_t j = 0;
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    if (p->hasIntKey()) {
      vec->m_data[j].m_data.num = p->ikey;
      vec->m_data[j].m_type = KindOfInt64;
    } else {
      p->skey->incRefCount();
      vec->m_data[j].m_data.pstr = p->skey;
      vec->m_data[j].m_type = KindOfString;
    }
    ++vec->m_size;
    ++j;
  }
  return obj;
}

Object c_ImmMap::t_keys() { return php_keys(); }

Object c_Map::t_keys() { return php_keys(); }

Object c_ImmMap::t_lazy() { return php_lazy(); }

Object c_Map::t_lazy() { return php_lazy(); }

Object BaseMap::php_kvzip() const {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  if (!m_size) {
    return obj;
  }
  vec->reserve(m_size);
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  ssize_t j = 0;
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    auto* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    if (p->hasIntKey()) {
      pair->elm0.m_data.num = p->ikey;
      pair->elm0.m_type = KindOfInt64;
    } else {
      p->skey->incRefCount();
      pair->elm0.m_data.pstr = p->skey;
      pair->elm0.m_type = KindOfString;
    }
    ++pair->m_size;
    pair->initAdd(&p->data);
    vec->m_data[j].m_data.pobj = pair;
    vec->m_data[j].m_type = KindOfObject;
    ++vec->m_size;
    ++j;
  }
  return obj;
}

Object c_ImmMap::t_kvzip() { return php_kvzip(); }

Object c_Map::t_kvzip() { return php_kvzip(); }

Variant BaseMap::php_at(const Variant& key) const {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  } else if (key.isString()) {
    return tvAsCVarRef(at(key.getStringData()));
  }
  throwBadKeyType();
  return uninit_null();
}

Variant c_ImmMap::t_at(const Variant& key) { return php_at(key); }

Variant c_Map::t_at(const Variant& key) { return php_at(key); }

Variant BaseMap::php_get(const Variant& key) const {
  if (key.isInteger()) {
    TypedValue* tv = get(key.toInt64());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return uninit_null();
    }
  } else if (key.isString()) {
    TypedValue* tv = get(key.getStringData());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return uninit_null();
    }
  }
  throwBadKeyType();
  return uninit_null();
}

Variant c_ImmMap::t_get(const Variant& key) { return php_get(key); }

Variant c_Map::t_get(const Variant& key) { return php_get(key); }

Object BaseMap::php_set(const Variant& key, const Variant& value) {
  TypedValue* val = cvarToCell(&value);
  if (key.isInteger()) {
    update(key.toInt64(), val);
  } else if (key.isString()) {
    update(key.getStringData(), val);
  } else {
    throwBadKeyType();
  }
  return this;
}

Object c_Map::t_set(const Variant& key, const Variant& value) {
  return php_set(key, value);
}

Object BaseMap::php_setAll(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tvKey = cvarToCell(&k);
    TypedValue* tvVal = cvarToCell(&v);
    if (tvKey->m_type == KindOfInt64) {
      set(tvKey->m_data.num, tvVal);
    } else if (IS_STRING_TYPE(tvKey->m_type)) {
      set(tvKey->m_data.pstr, tvVal);
    } else {
      throwBadKeyType();
    }
  }
  return this;
}

Object c_Map::t_setall(const Variant& iterable) { return php_setAll(iterable); }

bool BaseMap::php_contains(const Variant& key) const {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    return contains(key.toInt64());
  }
  if (IS_STRING_TYPE(t)) {
    return contains(key.getStringData());
  }
  throwBadKeyType();
  return false;
}

bool c_ImmMap::t_contains(const Variant& key) { return php_contains(key); }

bool c_Map::t_contains(const Variant& key) { return php_contains(key); }

bool c_ImmMap::t_containskey(const Variant& key) { return php_contains(key); }

bool c_Map::t_containskey(const Variant& key) { return php_contains(key); }

Object BaseMap::php_remove(const Variant& key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    remove(key.toInt64());
  } else if (IS_STRING_TYPE(t)) {
    remove(key.getStringData());
  } else {
    throwBadKeyType();
  }
  return this;
}

Object c_Map::t_remove(const Variant& key) { return php_remove(key); }

Object c_Map::t_removekey(const Variant& key) { return php_remove(key); }

Array c_ImmMap::t_toarray() { return php_toArray(); }

Array c_Map::t_toarray() { return php_toArray(); }

Object BaseMap::php_values() const {
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  int64_t sz = m_size;
  if (!sz) {
    return ret;
  }
  TypedValue* vecData;
  target->m_capacity = target->m_size = sz;
  target->m_data = vecData =
    (TypedValue*)MM().objMallocLogged(sz * sizeof(TypedValue));

  int64_t j = 0;
  for (ssize_t i = 0; i < iterLimit(); ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    cellDup(p.data, vecData[j]);
    ++j;
  }
  return ret;
}

Object c_ImmMap::t_values() { return php_values(); }

Object c_Map::t_values() { return php_values(); }

Array BaseMap::php_toKeysArray() const {
  PackedArrayInit ai(m_size);
  for (ssize_t i = 0; i < iterLimit(); ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    if (p.hasIntKey()) {
      ai.append(int64_t{p.ikey});
    } else {
      ai.append(*(const String*)(&p.skey));
    }
  }
  return ai.toArray();
}

Array c_ImmMap::t_tokeysarray() { return php_toKeysArray(); }

Array c_Map::t_tokeysarray() { return php_toKeysArray(); }

Array BaseMap::php_toValuesArray() const {
  PackedArrayInit ai(m_size);
  for (ssize_t i = 0; i < iterLimit(); ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    ai.append(tvAsCVarRef(&p.data));
  }
  return ai.toArray();
}

Array c_ImmMap::t_tovaluesarray() { return php_toValuesArray(); }

Array c_Map::t_tovaluesarray() { return php_toValuesArray(); }

template<typename TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_differenceByKey(const Variant& it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  TMap* target = BaseMap::Clone<TMap>(this);
  auto ret = Object::attach(target);
  if (Collection::isMapType(obj->getCollectionType())) {
    auto mp = static_cast<BaseMap*>(obj);
    for (uint i = 0; i < mp->iterLimit(); ++i) {
      if (mp->isTombstone(i)) continue;
      BaseMap::Elm& p = mp->data()[i];
      if (p.hasIntKey()) {
        target->remove((int64_t)p.ikey);
      } else {
        target->remove(p.skey);
      }
    }
    return ret;
  }
  for (ArrayIter iter(obj); iter; ++iter) {
    Variant k = iter.first();
    if (k.isInteger()) {
      target->remove(k.toInt64());
    } else {
      assert(k.isString());
      target->remove(k.getStringData());
    }
  }
  return ret;
}

Object c_ImmMap::t_differencebykey(const Variant& it) {
  return php_differenceByKey<c_ImmMap>(it);
}

Object c_Map::t_differencebykey(const Variant& it) {
  return php_differenceByKey<c_Map>(it);
}

Object c_Map::t_immutable() {
  auto* mp = NEWOBJ(c_ImmMap)();
  Object o = mp;
  mp->init(VarNR(this));
  return o;
}

Object c_ImmMap::t_immutable() {
  return this;
}

Object BaseMap::php_getIterator() {
  auto* it = NEWOBJ(c_MapIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_version = getVersion();
  return it;
}

Object c_ImmMap::t_getiterator() { return php_getIterator(); }

Object c_Map::t_getiterator() { return php_getIterator(); }

ALWAYS_INLINE
static std::array<TypedValue, 2> makeArgsFromMapKeyAndValue(
  BaseMap::Elm& e) {
  return std::array<TypedValue, 2> {{
    (e.hasIntKey() ? make_tv<KindOfInt64>(e.ikey)
     : make_tv<KindOfString>(e.skey)),
    e.data
  }};
}

ALWAYS_INLINE
static std::array<TypedValue, 1> makeArgsFromMapValue(BaseMap::Elm& e) {
  // note that this is a potentially unnecessary copy
  // that might be reinterpret_cast ed away
  // http://stackoverflow.com/questions/11205186/treat-c-cstyle-array-as-stdarray
  return std::array<TypedValue, 1> {{ e.data }};
}

template<typename TMap, class MakeArgs>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_map(const Variant& callback, MakeArgs makeArgs) const {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  TMap* mp;
  Object obj = mp = NEWOBJ(TMap)();
  if (!m_size) return obj;
  assert(m_used != 0);
  mp->deleteElms();
  mp->freeData();
  mp->m_cap = m_cap;
  mp->m_tableMask = m_tableMask;
  mp->m_size = m_size;
  auto needed =
    size_t(mp->m_cap) * sizeof(Elm) + mp->hashSize() * sizeof(int32_t);
  mp->m_data = (Elm*)MM().objMallocLogged(needed);
  mp->m_hash = (int32_t*)(mp->m_data + mp->m_cap);
  wordcpy(mp->hashTab(), hashTab(), hashSize());
  uint32_t used = iterLimit();
  mp->m_used = 0;
  for (uint32_t i = 0; i < used; mp->m_used = ++i) {
    Elm& p = data()[i];
    Elm& np = mp->data()[i];
    if (isTombstone(i)) {
      np.data.m_type = p.data.m_type;
      continue;
    }
    TypedValue* tv = &np.data;
    int32_t version = m_version;
    auto args = makeArgs(p);
    g_context->invokeFuncFew(tv, ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    if (p.hasStrKey()) {
      p.skey->incRefCount();
    }
    np.ikey = p.ikey;
    np.data.hash() = p.data.hash();
  }
  return obj;
}

Object c_ImmMap::t_map(const Variant& callback) {
  return php_map<c_ImmMap>(callback, &makeArgsFromMapValue);
}

Object c_Map::t_map(const Variant& callback) {
  return php_map<c_Map>(callback, &makeArgsFromMapValue);
}

Object c_ImmMap::t_mapwithkey(const Variant& callback) {
  return php_map<c_ImmMap>(callback, &makeArgsFromMapKeyAndValue);
}

Object c_Map::t_mapwithkey(const Variant& callback) {
  return php_map<c_Map>(callback, &makeArgsFromMapKeyAndValue);
}

template<typename TMap, class MakeArgs>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_filter(const Variant& callback, MakeArgs makeArgs) const {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  TMap* mp;
  Object obj = mp = NEWOBJ(TMap)();
  if (!m_size) return obj;

  uint32_t used = iterLimit();
  for (uint i = 0; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant ret;
    int32_t version = m_version;
    auto args = makeArgs(p);
    g_context->invokeFuncFew(ret.asTypedValue(), ctx,
                               args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!ret.toBoolean()) continue;
    if (p.hasIntKey()) {
      mp->update(p.ikey, &p.data);
    } else {
      mp->update(p.skey, &p.data);
    }
  }
  return obj;
}

Object c_ImmMap::t_filter(const Variant& callback) {
  return php_filter<c_ImmMap>(callback, &makeArgsFromMapValue);
}

Object c_Map::t_filter(const Variant& callback) {
  return php_filter<c_Map>(callback, &makeArgsFromMapValue);
}

Object c_ImmMap::t_filterwithkey(const Variant& callback) {
  return php_filter<c_ImmMap>(callback, &makeArgsFromMapKeyAndValue);
}

Object c_Map::t_filterwithkey(const Variant& callback) {
  return php_filter<c_Map>(callback, &makeArgsFromMapKeyAndValue);
}

template<class MakeArgs>
Object BaseMap::php_retain(const Variant& callback, MakeArgs makeArgs) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  ++m_version;
  auto size = m_size;
  if (!size) { return this; }

  uint32_t used = iterLimit();
  for (int32_t i = 0; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant ret;
    int32_t version = m_version;
    auto args = makeArgs(p);
    g_context->invokeFuncFew(ret.asTypedValue(), ctx,
                               args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (ret.toBoolean()) continue;

    // checking version above allows us to defer compaction, since we
    // know that no operations that mutate internal state have been
    // allowed to happen inside this loop other than this erase
    int32_t* pos = (p.hasIntKey()
                    ? findForInsert(p.ikey)
                    : findForInsert(p.skey, p.skey->hash()));
    eraseNoCompact(pos);
  }
  assert(m_size <= size);
  if (m_size < size) { compactIfNecessary(); }
  return this;
}

Object c_Map::t_retain(const Variant& callback) {
  return php_retain(callback, &makeArgsFromMapValue);
}

Object c_Map::t_retainwithkey(const Variant& callback) {
  return php_retain(callback, &makeArgsFromMapKeyAndValue);
}

template<typename TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_zip(const Variant& iterable) const {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  TMap* mp;
  Object obj = mp = NEWOBJ(TMap)();
  if (!m_size) {
    return obj;
  }
  mp->reserve(std::min(sz, size_t(m_size)));
  uint used = iterLimit();
  for (uint i = 0; i < used && iter; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant v = iter.second();
    c_Pair* pair;
    Object pairObj = pair = NEWOBJ(c_Pair)();
    pair->initAdd(&p.data);
    pair->initAdd(cvarToCell(&v));
    TypedValue tv;
    tv.m_data.pobj = pair;
    tv.m_type = KindOfObject;
    if (p.hasIntKey()) {
      mp->update(p.ikey, &tv);
    } else {
      mp->update(p.skey, &tv);
    }
    ++iter;
  }
  return obj;
}

Object c_ImmMap::t_zip(const Variant& iterable) {
  return php_zip<c_ImmMap>(iterable);
}

Object c_Map::t_zip(const Variant& iterable) {
  return php_zip<c_Map>(iterable);
}

template<class TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_take(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  if (len >= int64_t(m_size)) {
    // We know the resulting Map will simply be a copy of this Map,
    // so we can just call Clone() and return early here.
    return Object::attach(TMap::Clone(this));
  }
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  if (len <= 0) {
    // We know the resulting Map will be empty, so we can return
    // early here.
    return obj;
  }
  size_t sz = size_t(len);
  mp->reserve(sz);
  mp->m_size = mp->m_used = sz;
  auto table = mp->hashTab();
  auto mask = mp->m_tableMask;
  for (uint32_t frPos = 0, toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(m_data[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = mp->m_data[toPos];
    toE.skey = m_data[frPos].skey;
    toE.data.hash() = m_data[frPos].data.hash();
    if (toE.hasStrKey()) toE.skey->incRefCount();
    cellDup(m_data[frPos].data, toE.data);
    auto ie = findForNewInsert(table, mask,
                               toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }
  return obj;
}

template<class TMap, bool checkVersion>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  if (!m_size) return obj;
  uint32_t used = iterLimit();
  for (uint i = 0; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant ret;
    if (checkVersion) {
      int32_t version = m_version;
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    } else {
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
    }
    if (!ret.toBoolean()) continue;
    if (p.hasIntKey()) {
      mp->update(p.ikey, &p.data);
    } else {
      mp->update(p.skey, &p.data);
    }
  }
  return obj;
}

template<class TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  if (len <= 0) {
    // We know the resulting Map will simply be a copy of this Map,
    // so we can just call Clone() and return early here.
    return Object::attach(TMap::Clone(this));
  }
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  if (len >= m_size) {
    // We know the resulting Map will be empty, so we can return
    // early here.
    return obj;
  }
  size_t sz = size_t(m_size) - size_t(len);
  assert(sz);
  mp->reserve(sz);
  mp->m_size = mp->m_used = sz;
  uint32_t frPos;
  if (LIKELY(!hasTombstones())) {
    // Fast path: Map contains no tombstones
    frPos = len;
  } else {
    // Slow path: Map has at least one tombstone, so we need to
    // count forward
    frPos = 0;
    while (len > 0) {
      while (isTombstone(m_data[frPos].data.m_type)) {
        assert(frPos + 1 < m_used);
        ++frPos;
      }
      --len;
      ++frPos;
    }
  }
  auto table = mp->hashTab();
  auto mask = mp->m_tableMask;
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(m_data[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = mp->m_data[toPos];
    toE.skey = m_data[frPos].skey;
    toE.data.hash() = m_data[frPos].data.hash();
    if (toE.hasStrKey()) toE.skey->incRefCount();
    cellDup(m_data[frPos].data, toE.data);
    auto ie = findForNewInsert(table, mask,
                               toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }
  return obj;
}

template<class TMap, bool checkVersion>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  if (!m_size) return obj;
  uint32_t used = iterLimit();
  uint i = 0;
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant ret;
    if (checkVersion) {
      int32_t version = m_version;
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    } else {
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
    }
    if (!ret.toBoolean()) break;
  }
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    if (p.hasIntKey()) {
      mp->update(p.ikey, &p.data);
    } else {
      mp->update(p.skey, &p.data);
    }
  }
  return obj;
}

Object c_Map::t_take(const Variant& n) {
  return BaseMap::php_take<c_Map>(n);
}

Object c_ImmMap::t_take(const Variant& n) {
  return BaseMap::php_take<c_ImmMap>(n);
}

Object c_Map::t_takewhile(const Variant& fn) {
  return BaseMap::php_takeWhile<c_Map, true>(fn);
}

Object c_ImmMap::t_takewhile(const Variant& fn) {
  return BaseMap::php_takeWhile<c_ImmMap, false>(fn);
}

Object c_Map::t_skip(const Variant& n) {
  return BaseMap::php_skip<c_Map>(n);
}

Object c_ImmMap::t_skip(const Variant& n) {
  return BaseMap::php_skip<c_ImmMap>(n);
}

Object c_Map::t_skipwhile(const Variant& fn) {
  return BaseMap::php_skipWhile<c_Map, true>(fn);
}

Object c_ImmMap::t_skipwhile(const Variant& fn) {
  return BaseMap::php_skipWhile<c_ImmMap, false>(fn);
}

template<typename TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_mapFromIterable(const Variant& iterable) {
  if (iterable.isNull()) return NEWOBJ(TMap)();
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  TMap* target;
  Object ret = target = NEWOBJ(TMap)();
  if (sz) {
    target->reserve(sz);
  }
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = v.asCell();
    if (UNLIKELY(tv->m_type != KindOfObject ||
                 tv->m_data.pobj->getVMClass() != c_Pair::classof())) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
                 "Parameter must be an instance of Iterable<Pair>"));
      throw e;
    }
    auto pair = static_cast<c_Pair*>(tv->m_data.pobj);
    TypedValue* tvKey = &pair->elm0;
    TypedValue* tvValue = &pair->elm1;
    assert(tvKey->m_type != KindOfRef);
    assert(tvValue->m_type != KindOfRef);
    if (tvKey->m_type == KindOfInt64) {
      target->update(tvKey->m_data.num, tvValue);
    } else if (IS_STRING_TYPE(tvKey->m_type)) {
      target->update(tvKey->m_data.pstr, tvValue);
    } else {
      TMap::throwBadKeyType();
    }
  }
  return ret;
}

Object c_ImmMap::ti_fromitems(const Variant& iterable) {
  return php_mapFromIterable<c_ImmMap>(iterable);
}

Object c_Map::ti_fromitems(const Variant& iterable) {
  return php_mapFromIterable<c_Map>(iterable);
}

template<typename TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_mapFromArray(const Variant& arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  TMap* mp;
  Object ret = mp = NEWOBJ(TMap)();
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    TypedValue* tv = cvarToCell(&ad->getValueRef(pos));
    if (k.isInteger()) {
      mp->update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      mp->update(k.getStringData(), tv);
    }
  }
  return ret;
}

Object c_Map::ti_fromarray(const Variant& arr) {
  return php_mapFromArray<c_Map>(arr);
}

template<typename TMap>
  typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, ObjectData*>::type
collectionDeepCopyBaseMap(TMap* mp) {
  mp = TMap::Clone(mp);
  Object o = Object::attach(mp);
  uint used = mp->iterLimit();
  for (uint32_t i = 0; i < used; ++i) {
    if (mp->isTombstone(i)) continue;
    BaseMap::Elm* p = &mp->data()[i];
    collectionDeepCopyTV(&p->data);
  }
  return o.detach();
}

ObjectData* collectionDeepCopyImmMap(c_ImmMap* map) {
  return collectionDeepCopyBaseMap<c_ImmMap>(map);
}

ObjectData* collectionDeepCopyMap(c_Map* map) {
  return collectionDeepCopyBaseMap<c_Map>(map);
}

NEVER_INLINE
void BaseMap::throwOOB(int64_t key) {
  throwIntOOB(key);
}

NEVER_INLINE
void BaseMap::throwOOB(StringData* key) {
  throwStrOOB(key);
}

NEVER_INLINE
void BaseMap::throwTooLarge() {
  static const size_t reserveSize = 130;
  String msg(reserveSize, ReserveString);
  char* buf = msg.bufferSlice().ptr;
  int sz = sprintf(buf,
                   "Map object has reached its maximum capacity of "
                   "%u element slots and does not have room to add a "
                   "new element",
                   MaxSize);
  assert(sz <= reserveSize);
  msg.setSize(sz);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(msg));
  throw e;
}

NEVER_INLINE
void BaseMap::throwReserveTooLarge() {
  static const size_t reserveSize = 80;
  String msg(reserveSize, ReserveString);
  char* buf = msg.bufferSlice().ptr;
  int sz = sprintf(buf,
                   "Map does not support reserving room for more than "
                   "%u elements",
                   MaxReserveSize);
  assert(sz <= reserveSize);
  msg.setSize(sz);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(msg));
  throw e;
}

NEVER_INLINE
int32_t* BaseMap::warnUnbalanced(size_t n, int32_t* ei) {
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    raise_error("Map is too unbalanced (%lu)", n);
  }
  return ei;
}

TypedValue* BaseMap::at(int64_t key) const {
  auto p = find(key);
  if (UNLIKELY(p == Empty)) {
    throwOOB(key);
  }
  return &data()[p].data;
}

TypedValue* BaseMap::at(StringData* key) const {
  auto p = find(key, key->hash());
  if (UNLIKELY(p == Empty)) {
    throwOOB(key);
  }
  return &data()[p].data;
}

TypedValue* BaseMap::get(int64_t key) const {
  auto p = find(key);
  if (p == Empty) {
    return nullptr;
  }
  return &data()[p].data;
}

TypedValue* BaseMap::get(StringData* key) const {
  auto p = find(key, key->hash());
  if (p == Empty) {
    return nullptr;
  }
  return &data()[p].data;
}

void BaseMap::add(TypedValue* val) {
  if (UNLIKELY(val->m_type != KindOfObject ||
               val->m_data.pobj->getVMClass() != c_Pair::classof())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be an instance of Pair"));
    throw e;
  }
  auto pair = static_cast<c_Pair*>(val->m_data.pobj);
  TypedValue* tvKey = &pair->elm0;
  TypedValue* tvValue = &pair->elm1;
  assert(tvKey->m_type != KindOfRef);
  assert(tvValue->m_type != KindOfRef);
  if (tvKey->m_type == KindOfInt64) {
    update(tvKey->m_data.num, tvValue);
  } else if (IS_STRING_TYPE(tvKey->m_type)) {
    update(tvKey->m_data.pstr, tvValue);
  } else {
    throwBadKeyType();
  }
}

Variant BaseMap::pop() {
  if (m_size) {
    ++m_version;
    Elm* e = data() + iterLimit() - 1;
    for (;; --e) {
      assert(e >= data());
      if (!isTombstone(e->data.m_type)) break;
    }
    Variant ret = tvAsCVarRef(&e->data);
    int32_t* ei;
    if (e->hasIntKey()) {
      ei = findForInsert(e->ikey);
    } else {
      assert(e->hasStrKey());
      ei = findForInsert(e->skey, e->skey->hash());
    }
    erase(ei);
    return ret;
  } else {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Cannot pop empty Map"));
    throw e;
  }
}

Variant BaseMap::popFront() {
  if (m_size) {
    ++m_version;
    Elm* e = data();
    for (;; ++e) {
      assert(e != data() + iterLimit());
      if (!isTombstone(e->data.m_type)) break;
    }
    Variant ret = tvAsCVarRef(&e->data);
    int32_t* ei;
    if (e->hasIntKey()) {
      ei = findForInsert(e->ikey);
    } else {
      assert(e->hasStrKey());
      ei = findForInsert(e->skey, e->skey->hash());
    }
    erase(ei);
    return ret;
  } else {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Cannot pop empty Map"));
    throw e;
  }
}

void BaseMap::remove(int64_t key) {
  ++m_version;
  auto* p = findForInsert(key);
  if (validPos(*p)) {
    erase(p);
  }
}

void BaseMap::remove(StringData* key) {
  ++m_version;
  auto* p = findForInsert(key, key->hash());
  if (validPos(*p)) {
    erase(p);
  }
}

bool BaseMap::contains(int64_t key) const {
  return find(key) != Empty;
}

bool BaseMap::contains(StringData* key) const {
  return find(key, key->hash()) != Empty;
}

static bool hitStringKey(const BaseMap::Elm& e, const StringData* s,
                         int32_t hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. c_Map guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!BaseMap::isTombstone(e.data.m_type));
  return hash == e.hash() && (s == e.skey || s->same(e.skey));
}

static bool hitIntKey(const BaseMap::Elm& e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. c_Map guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!BaseMap::isTombstone(e.data.m_type));
  return e.ikey == ki && e.hasIntKey();
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2. In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.

template <class Hit>
ALWAYS_INLINE
ssize_t BaseMap::findImpl(size_t h0, Hit hit) const {
  size_t tableMask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  for (size_t probeIndex = h0, i = 1;; ++i) {
    ssize_t pos = hashtable[probeIndex & tableMask];
    if ((validPos(pos) && hit(elms[pos])) || pos == Empty) {
      return pos;
    }
    probeIndex += i;
    assert(i <= tableMask && probeIndex == h0 + (i + i*i) / 2);
  }
}

ssize_t BaseMap::find(int64_t ki) const {
  return findImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

ssize_t BaseMap::find(const StringData* s, strhash_t prehash) const {
  auto h = prehash | STRHASH_MSB;
  return findImpl(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

template <class Hit>
ALWAYS_INLINE
int32_t* BaseMap::findForInsertImpl(size_t h0, Hit hit) const {
  // mask, probe, and pos are explicitly 64-bit, because performance
  // regressed when they were 32-bit types via auto. Test carefully.
  size_t mask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  int32_t* ret = nullptr;
  for (size_t probe = h0, i = 1;; ++i) {
    auto ei = &hashtable[probe & mask];
    ssize_t pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        return ei;
      }
    } else {
      if (!ret) ret = ei;
      if (pos == Empty) {
        return LIKELY(i <= 100) ? ret : warnUnbalanced(i, ret);
      }
    }
    probe += i;
    assert(i <= mask && probe == h0 + (i + i*i) / 2);
  }
}

int32_t* BaseMap::findForInsert(int64_t ki) const {
  return findForInsertImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

int32_t* BaseMap::findForInsert(const StringData* s, strhash_t prehash) const {
  auto h = prehash | STRHASH_MSB;
  return findForInsertImpl(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

// findForNewInsert() is only safe to use if you know for sure that the
// key is not already present in the Map.
ALWAYS_INLINE int32_t*
BaseMap::findForNewInsert(int32_t* table, size_t mask, size_t h0) const {
  for (size_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) return ei;
    probe += i;
    assert(i <= mask && probe == h0 + (i + i*i) / 2);
  }
}

ALWAYS_INLINE
int32_t* BaseMap::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), m_tableMask, h0);
}

void BaseMap::update(int64_t h, TypedValue* val) {
  assert(val->m_type != KindOfRef);
retry:
  auto* p = findForInsert(h);
  assert(p);
  if (validPos(*p)) {
    auto& e = data()[*p];
    DataType oldType = e.data.m_type;
    uint64_t oldDatum = e.data.m_data.num;
    cellDup(*val, e.data);
    if (IS_REFCOUNTED_TYPE(oldType)) {
      tvDecRefHelper(oldType, oldDatum);
    }
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    goto retry;
  }
  auto& e = allocElm(p);
  cellDup(*val, e.data);
  e.setIntKey(h);
  ++m_version;
}

void BaseMap::update(StringData* key, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  strhash_t h = key->hash();
retry:
  auto* p = findForInsert(key, h);
  assert(p);
  if (validPos(*p)) {
    auto& e = data()[*p];
    DataType oldType = e.data.m_type;
    uint64_t oldDatum = e.data.m_data.num;
    cellDup(*val, e.data);
    if (IS_REFCOUNTED_TYPE(oldType)) {
      tvDecRefHelper(oldType, oldDatum);
    }
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    goto retry;
  }
  auto& e = allocElm(p);
  cellDup(*val, e.data);
  e.setStrKey(key, h);
  ++m_version;
}

void BaseMap::eraseNoCompact(int32_t* pos) {
  assert(validPos(*pos) && !isTombstone(*pos));
  assert(data());
  Elm* elms = data();
  auto& e = elms[*pos];
  // Mark the hash slot as a tombstone.
  *pos = Tombstone;
  // Decref the key if it's a string.
  if (e.hasStrKey()) {
    decRefStr(e.skey);
  }
  // Mark the Elm as a tombstone.
  TypedValue* tv = &e.data;
  DataType oldType = tv->m_type;
  uint64_t oldDatum = tv->m_data.num;
  tv->m_type = KindOfInvalid;
  --m_size;
  // Don't adjust m_used. This frees us from having to explicitly keep track
  // of hash load, since we can instead rely on the isFull() condition to
  // trigger a grow/compact before hash load gets too high.
  assert(m_used <= m_cap);
  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

void BaseMap::erase(int32_t* pos) {
  eraseNoCompact(pos);
  compactIfNecessary();
}

NEVER_INLINE void BaseMap::makeRoom() {
  assert(isFull());
  // erase() guarantees that element density will never drop below ~50%,
  // so we always grow to make room here.
  assert(!isDensityTooLow());
  if (UNLIKELY(m_cap == MaxSize)) {
    throwTooLarge();
  }
  // Double the current capacity
  grow(m_cap ? m_cap*2 : SmallSize, m_cap ? m_tableMask*2+1 : SmallMask);
}

NEVER_INLINE void BaseMap::reserve(int64_t sz) {
  assert(m_size <= m_used && m_used <= m_cap && !isDensityTooLow());
  uint32_t newCap, newMask;
  if (UNLIKELY(sz > int64_t(MaxReserveSize))) {
    throwReserveTooLarge();
  }
  // Compute the new capacity and new hash mask if we need to grow or bail
  // out if there is nothing to do.
  if (sz > int64_t(m_cap)) {
    // The requested capacity is greater than the current capacity. Compute
    // the smallest allowed capacity that is sufficient.
    auto lgSize = BaseMap::SmallLgTableSize;
    for (newCap = BaseMap::SmallSize; newCap < sz; newCap <<= 1) ++lgSize;
    newMask = (size_t(1U) << lgSize) - 1;
    assert(lgSize <= MaxLgTableSize && newCap > m_cap);
  } else {
    // If sz <= m_size or if the Map can accommodate sz-m_size additional
    // elements without growing or compacting, then there is nothing to do.
    if (sz <= int64_t(m_size) ||
        size_t(m_used) + size_t(sz - m_size) <= size_t(m_cap)) return;
    // Otherwise prepare to double the current capacity
    assert(0 < sz && sz <= int64_t(m_cap));
    assert(m_cap < MaxSize && m_tableMask != 0);
    newCap = m_cap * 2;
    newMask = m_tableMask * 2 + 1;
  }
  // Perform the grow operation.
  grow(newCap, newMask);
}

void BaseMap::grow(uint32_t newCap, uint32_t newMask) {
  assert(m_size <= m_used && m_used <= m_cap);
  size_t newHashSize = size_t(newMask) + 1;
  assert(folly::isPowTwo(newHashSize) && computeMaxElms(newMask) == newCap);
  assert(m_size <= newCap && newCap <= MaxSize);
  auto* oldData = data();
  auto oldHashSize = oldData ? hashSize() : 0;
  auto needed = size_t(newCap) * sizeof(Elm) + newHashSize * sizeof(int32_t);
  auto* data = (Elm*)MM().objMallocLogged(needed);
  auto* table = (int32_t*)(data + size_t(newCap));
  m_data = data;
  m_hash = table;
  m_tableMask = newMask;
  initHash(table, newHashSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(oldData[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = data[toPos];
    toE = oldData[frPos];
    auto ie = findForNewInsert(table, newMask,
                               toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }
  if (oldData) {
    MM().objFreeLogged(
      oldData, size_t(m_cap) * sizeof(Elm) + oldHashSize * sizeof(int32_t));
  }
  m_cap = newCap;
  m_used = m_size;
}

void BaseMap::compactIfNecessary() {
  if (!isDensityTooLow()) { return; }
  auto* elms = data();
  assert(elms);
  auto mask = m_tableMask;
  size_t tableSize = hashSize();
  auto table = hashTab();
  initHash(table, tableSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(elms[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = elms[toPos];
    if (toPos != frPos) {
      toE = elms[frPos];
    }
    auto ie = findForNewInsert(table, mask,
                               toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }
  m_used = m_size;
}

void BaseMap::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with Maps"));
  throw e;
}

Array BaseMap::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return static_cast<const BaseMap*>(obj)->php_toArray();
}

bool BaseMap::ToBool(const ObjectData* obj) {
  return static_cast<const BaseMap*>(obj)->toBoolImpl();
}


struct MapKeyAccessor {
  typedef const BaseMap::Elm& ElmT;
  bool isInt(ElmT elm) const { return elm.hasIntKey(); }
  bool isStr(ElmT elm) const { return elm.hasStrKey(); }
  int64_t getInt(ElmT elm) const { return elm.ikey; }
  StringData* getStr(ElmT elm) const { return elm.skey; }
  Variant getValue(ElmT elm) const {
    if (isInt(elm)) {
      return getInt(elm);
    }
    assert(isStr(elm));
    return getStr(elm);
  }
};

struct MapValAccessor {
  typedef const BaseMap::Elm& ElmT;
  bool isInt(ElmT elm) const { return elm.data.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return IS_STRING_TYPE(elm.data.m_type); }
  int64_t getInt(ElmT elm) const { return elm.data.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm.data.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm.data); }
};

/**
 * preSort() does an initial pass to do some preparatory work before the
 * sort algorithm runs. For sorts that use builtin comparators, the types
 * of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized
 * comparator and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
BaseMap::SortFlavor BaseMap::preSort(const AccessorT& acc, bool checkTypes) {
  assert(m_size > 0);
  if (!checkTypes && !hasTombstones()) {
    // No need to loop over the elements, we're done
    return GenericSort;
  }
  Elm* start = data();
  Elm* end = data() + m_used;
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  for (;;) {
    if (checkTypes) {
      while (!isTombstone(start->data.m_type)) {
        allInts = (allInts && acc.isInt(*start));
        allStrs = (allStrs && acc.isStr(*start));
        ++start;
        if (start == end) {
          goto done;
        }
      }
    } else {
      while (!isTombstone(start->data.m_type)) {
        ++start;
        if (start == end) {
          goto done;
        }
      }
    }
    --end;
    if (start == end) {
      goto done;
    }
    while (isTombstone(end->data.m_type)) {
      --end;
      if (start == end) {
        goto done;
      }
    }
    memcpy(start, end, sizeof(Elm));
  }
  done:
  m_used = start - data();
  assert(!hasTombstones());
  if (checkTypes) {
    return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
  } else {
    return GenericSort;
  }
}

/**
 * postSort() runs after the sort has been performed. For c_Map,
 * postSort() handles rebuilding the hash.
 */
void BaseMap::postSort() {
  assert(m_size > 0);
  auto const table = hashTab();
  initHash(table, hashSize());
  {
    auto mask = m_tableMask;
    auto data = this->data();
    for (uint32_t pos = 0; pos < m_used; ++pos) {
      auto& e = data[pos];
      auto ei = findForNewInsert(table, mask,
                                 e.hasIntKey() ? e.ikey : e.hash());
      *ei = pos;
    }
  }
}

#define SORT_CASE(flag, cmp_type, acc_type)                     \
  case flag: {                                                  \
    if (ascending) {                                            \
      cmp_type##Compare<acc_type, flag, true> comp;             \
        HPHP::Sort::sort(data(), data() + m_size, comp);        \
    } else {                                                    \
      cmp_type##Compare<acc_type, flag, false> comp;            \
        HPHP::Sort::sort(data(), data() + m_size, comp);        \
    }                                                           \
    break;                                                      \
  }
#define SORT_CASE_BLOCK(cmp_type, acc_type)                     \
  switch (sort_flags) {                                         \
    default: /* fall through to SORT_REGULAR case */            \
      SORT_CASE(SORT_REGULAR, cmp_type, acc_type)               \
        SORT_CASE(SORT_NUMERIC, cmp_type, acc_type)             \
        SORT_CASE(SORT_STRING, cmp_type, acc_type)              \
        SORT_CASE(SORT_LOCALE_STRING, cmp_type, acc_type)       \
        SORT_CASE(SORT_NATURAL, cmp_type, acc_type)             \
        SORT_CASE(SORT_NATURAL_CASE, cmp_type, acc_type)        \
        }
#define CALL_SORT(acc_type)                     \
  if (flav == StringSort) {                     \
    SORT_CASE_BLOCK(StrElm, acc_type)           \
      } else if (flav == IntegerSort) {         \
    SORT_CASE_BLOCK(IntElm, acc_type)           \
      } else {                                  \
    SORT_CASE_BLOCK(Elm, acc_type)              \
      }
#define SORT_BODY(acc_type)                                     \
  do {                                                          \
    if (!m_size) {                                              \
      return;                                                   \
    }                                                           \
    SortFlavor flav = preSort<acc_type>(acc_type(), true);      \
    try {                                                       \
      CALL_SORT(acc_type);                                      \
    } catch (...) {                                             \
      /* make sure the map is left in a consistent state */     \
      postSort();                                               \
      throw;                                                    \
    }                                                           \
    postSort();                                                 \
  } while(0)

void BaseMap::asort(int sort_flags, bool ascending) {
  SORT_BODY(MapValAccessor);
}

void BaseMap::ksort(int sort_flags, bool ascending) {
  SORT_BODY(MapKeyAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT
#undef SORT_BODY

#define USER_SORT_BODY(acc_type)                                \
  do {                                                          \
    if (!m_size) {                                              \
      return true;                                              \
    }                                                           \
    CallCtx ctx;                                                \
    JIT::CallerFrame cf;                                        \
    vm_decode_function(cmp_function, cf(), false, ctx);         \
    if (!ctx.func) {                                            \
      return false;                                             \
    }                                                           \
    preSort<acc_type>(acc_type(), false);                       \
    SCOPE_EXIT {                                                \
      /* make sure the map is left in a consistent state */     \
      postSort();                                               \
    };                                                          \
    ElmUCompare<acc_type> comp;                                 \
    comp.ctx = &ctx;                                            \
    HPHP::Sort::sort(data(), data() + m_size, comp);            \
    return true;                                                \
  } while (0)

bool BaseMap::uasort(const Variant& cmp_function) {
  USER_SORT_BODY(MapValAccessor);
}

bool BaseMap::uksort(const Variant& cmp_function) {
  USER_SORT_BODY(MapKeyAccessor);
}

#undef USER_SORT_BODY

template <bool throwOnMiss>
TypedValue* BaseMap::OffsetAt(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    return throwOnMiss ? mp->at(key->m_data.num)
                       : mp->get(key->m_data.num);
  }
  if (IS_STRING_TYPE(key->m_type)) {
    return throwOnMiss ? mp->at(key->m_data.pstr)
                       : mp->get(key->m_data.pstr);
  }
  throwBadKeyType();
  return nullptr;
}

void BaseMap::OffsetSet(ObjectData* obj, const TypedValue* key,
                        TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  auto mp = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    mp->set(key->m_data.num, val);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    mp->set(key->m_data.pstr, val);
    return;
  }
  throwBadKeyType();
}

bool BaseMap::OffsetIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<BaseMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = mp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = mp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellIsNull(result) : false;
}

bool BaseMap::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<BaseMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = mp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = mp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellToBool(*result) : true;
}

bool BaseMap::OffsetContains(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    return mp->contains(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    return mp->contains(key->m_data.pstr);
  } else {
    throwBadKeyType();
    return false;
  }
}

void BaseMap::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    mp->remove(key->m_data.num);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    mp->remove(key->m_data.pstr);
    return;
  }
  throwBadKeyType();
}

bool BaseMap::Equals(EqualityFlavor eq,
                     const ObjectData* obj1, const ObjectData* obj2) {

  auto mp1 = static_cast<const BaseMap*>(obj1);
  auto mp2 = static_cast<const BaseMap*>(obj2);
  auto size = mp1->size();

  if (size != mp2->size()) { return false; }
  if (size == 0) { return true; }

  switch (eq) {
    case EqualityFlavor::OrderIrrelevant: {
      // obj1 and obj2 must have the exact same set of keys, and the values
      // for each key must compare equal (==). This equality behavior
      // matches that of == on two PHP (associative) arrays.
      for (uint i = 0; i < mp1->iterLimit(); ++i) {
        if (mp1->isTombstone(i)) continue;
        BaseMap::Elm& p = mp1->data()[i];
        TypedValue* tv2;
        if (p.hasIntKey()) {
          tv2 = mp2->get(p.ikey);
        } else {
          assert(p.hasStrKey());
          tv2 = mp2->get(p.skey);
        }
        if (!tv2) return false;
        if (!equal(tvAsCVarRef(&p.data), tvAsCVarRef(tv2))) return false;
      }
      return true;
    }
    case EqualityFlavor::OrderMatters: {
      // obj1 and obj2 must compare equal according to OrderIrrelevant;
      // additionally, the (identical) keys of obj1 and obj2 must be in the
      // same iteration order.
      uint compared = 0;
      for (uint ix1 = 0, ix2 = 0;
           ix1 < mp1->iterLimit() && ix2 < mp2->iterLimit() ; ) {

        auto tomb1 = mp1->isTombstone(ix1);
        auto tomb2 = mp2->isTombstone(ix2);

        if (tomb1 || tomb2) {
          if (tomb1) { ++ix1; }
          if (tomb2) { ++ix2; }
          continue;
        }

        BaseMap::Elm& p1 = mp1->data()[ix1];
        BaseMap::Elm& p2 = mp2->data()[ix2];

        if (p1.hasIntKey()) {
          if (!p2.hasIntKey() ||
              p1.ikey != p2.ikey) {
            return false;
          }
        } else {
          assert(p1.hasStrKey());
          if (!p2.hasStrKey() || !equal(p1.skey, p2.skey)) {
            return false;
          }
        }
        if (!equal(tvAsCVarRef(&p1.data), tvAsCVarRef(&p2.data))) {
          return false;
        }

        ++ix1; ++ix2; ++compared;
      }

      return (compared == size);
    }
  }
  not_reached();
}

void BaseMap::Unserialize(ObjectData* obj,
                          VariableUnserializer* uns,
                          int64_t sz,
                          char type) {
  if (type != 'K') {
    throw Exception("Map does not support the '%c' serialization "
                    "format", type);
  }
  auto mp = static_cast<BaseMap*>(obj);
  mp->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    Variant k;
    k.unserialize(uns, Uns::Mode::ColKey);
    int32_t* p;
    Elm* e = nullptr;
    if (k.isInteger()) {
      auto h = k.toInt64();
      p = mp->findForInsert(h);
      // Check to be robust against manually crafted inputs with conflicting
      // elements
      if (UNLIKELY(validPos(*p))) goto do_unserialize;
      e = &mp->allocElm(p);
      e->setIntKey(h);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      p = mp->findForInsert(key, h);
      // Check to be robust against manually crafted inputs with conflicting
      // elements
      if (UNLIKELY(validPos(*p))) goto do_unserialize;
      e = &mp->allocElm(p);
      e->setStrKey(key, h);
    } else {
      throw Exception("Invalid key");
    }
    e->data.m_type = KindOfNull;
do_unserialize:
    tvAsVariant(&e->data).unserialize(uns, Uns::Mode::ColValue);
  }
}

///////////////////////////////////////////////////////////////////////////////

c_MapIterator::c_MapIterator(Class* cb) : ExtObjectData(cb) {}

c_MapIterator::~c_MapIterator() {
}

void c_MapIterator::t___construct() {
}

Variant c_MapIterator::t_current() {
  auto const mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  if (!mp->iter_valid(m_pos)) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(mp->iter_value(m_pos));
}

Variant c_MapIterator::t_key() {
  auto const mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  if (!mp->iter_valid(m_pos)) {
    throw_iterator_not_valid();
  }
  return mp->iter_key(m_pos);
}

bool c_MapIterator::t_valid() {
  auto const mp = m_obj.get();
  return mp->iter_valid(m_pos);
}

void c_MapIterator::t_next() {
  auto const mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  m_pos = mp->iter_next(m_pos);
}

void c_MapIterator::t_rewind() {
  auto const mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  m_pos = mp->iter_begin();
}

///////////////////////////////////////////////////////////////////////////////

c_ImmMap::c_ImmMap(Class* cb) : BaseMap(cb) {
  o_subclassData.u16 = Collection::ImmMapType;
}

void c_ImmMap::t___construct(const Variant& iterable /* = null_variant */) {
  php_construct(iterable);
}

c_ImmMap* c_ImmMap::Clone(ObjectData* obj) {
  return BaseMap::Clone<c_ImmMap>(obj);
}

///////////////////////////////////////////////////////////////////////////////
// BaseSet

// Public

void BaseSet::init(const Variant& t) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  for (; iter; ++iter) {
    Variant v = iter.second();
    if (v.isInteger()) {
      add(v.toInt64());
    } else if (v.isString()) {
      add(v.getStringData());
    } else {
      throwBadValueType();
    }
  }
}

void BaseSet::add(int64_t h) {
  auto* p = findForInsert(h);
  assert(p);
  if (validPos(*p)) {
    // When there is a conflict, the add() API is supposed to replace the
    // existing element with the new element in place. However since Sets
    // currently only support integer and string elements, there is no way
    // user code can really tell whether the existing element was replaced
    // so for efficiency we do nothing.
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(h);
  }
  auto& e = allocElm(p);
  e.setInt(h);
  ++m_version;
}

void BaseSet::add(StringData *key) {
  strhash_t h = key->hash();
  auto* p = findForInsert(key, h);
  assert(p);
  if (validPos(*p)) {
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(key, h);
  }
  auto& e = allocElm(p);
  e.setStr(key, h);
  ++m_version;
}

BaseSet::Elm& BaseSet::allocElmFront(int32_t* ei) {
  assert(ei && !validPos(*ei) && m_size <= m_used && m_used < m_cap);
  // Move the existing elements to make element slot 0 available.
  memmove(data() + 1, data(), m_used * sizeof(Elm));
  ++m_used;
  // Update the hashtable to reflect the fact that everything was moved
  // over one position
  auto* hash = hashTab();
  auto* hashEnd = hash + hashSize();
  for (; hash != hashEnd; ++hash) {
    if (validPos(*hash)) {
      ++(*hash);
    }
  }
  // Set the hash entry we found to point to element slot 0.
  (*ei) = 0;
  // Store the value into element slot 0.
  ++m_size;
  return data()[0];
}

void BaseSet::addFront(int64_t h) {
  auto* p = findForInsert(h);
  assert(p);
  if (validPos(*p)) {
    // When there is a conflict, the addFront() API is supposed to replace
    // the existing element with the new element in place. However since
    // Sets currently only support integer and string elements, there is
    // no way user code can really tell whether the existing element was
    // replaced so for efficiency we do nothing.
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(h);
  }
  auto& e = allocElmFront(p);
  e.setInt(h);
  ++m_version;
}

void BaseSet::addFront(StringData *key) {
  strhash_t h = key->hash();
  auto* p = findForInsert(key, h);
  assert(p);
  if (validPos(*p)) {
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(key, h);
  }
  auto& e = allocElmFront(p);
  e.setStr(key, h);
  ++m_version;
}

Variant BaseSet::pop() {
  if (m_size) {
    ++m_version;
    Elm* e = data() + iterLimit() - 1;
    for (;; --e) {
      assert(e >= data());
      if (!isTombstone(e->data.m_type)) break;
    }
    Variant ret = tvAsCVarRef(&e->data);
    int32_t* ei;
    if (e->hasInt()) {
      ei = findForInsert(e->data.m_data.num);
    } else {
      auto* key = e->data.m_data.pstr;
      ei = findForInsert(key, key->hash());
    }
    erase(ei);
    return ret;
  } else {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Cannot pop empty Set"));
    throw e;
  }
}

Variant BaseSet::popFront() {
  if (m_size) {
    ++m_version;
    Elm* e = data();
    for (;; ++e) {
      assert(e != data() + iterLimit());
      if (!isTombstone(e->data.m_type)) break;
    }
    Variant ret = tvAsCVarRef(&e->data);
    int32_t* ei;
    if (e->hasInt()) {
      ei = findForInsert(e->data.m_data.num);
    } else {
      auto* key = e->data.m_data.pstr;
      ei = findForInsert(key, key->hash());
    }
    erase(ei);
    return ret;
  } else {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Cannot pop empty Set"));
    throw e;
  }
}

void BaseSet::throwOOB(int64_t val) {
  throwIntOOB(val);
}

void BaseSet::throwOOB(StringData* val) {
  throwStrOOB(val);
}

void BaseSet::throwNoIndexAccess() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
    "[] operator not supported for accessing elements of Sets"));
  throw e;
}

NEVER_INLINE void BaseSet::makeRoom() {
  assert(isFull());
  // erase() guarantees that element density will never drop below ~50%,
  // so we always grow to make room here.
  assert(!isDensityTooLow());
  if (UNLIKELY(m_cap == MaxSize)) {
    throwTooLarge();
  }
  // Double the current capacity
  grow(m_cap ? m_cap*2 : SmallSize, m_cap ? m_tableMask*2+1 : SmallMask);
}

NEVER_INLINE void BaseSet::reserve(int64_t sz) {
  assert(m_size <= m_used && m_used <= m_cap && !isDensityTooLow());
  uint32_t newCap, newMask;
  if (UNLIKELY(sz > int64_t(MaxReserveSize))) {
    throwReserveTooLarge();
  }
  // Compute the new capacity and new hash mask if we need to grow or bail
  // out if there is nothing to do.
  if (sz > int64_t(m_cap)) {
    // The requested capacity is greater than the current capacity. Compute
    // the smallest allowed capacity that is sufficient.
    auto lgSize = BaseSet::SmallLgTableSize;
    for (newCap = BaseSet::SmallSize; newCap < sz; newCap <<= 1) ++lgSize;
    newMask = (size_t(1U) << lgSize) - 1;
    assert(lgSize <= MaxLgTableSize && newCap > m_cap);
  } else {
    // If sz <= m_size or if the Set can accommodate sz-m_size additional
    // elements without growing or compacting, then there is nothing to do.
    if (sz <= int64_t(m_size) ||
        size_t(m_used) + size_t(sz - m_size) <= size_t(m_cap)) return;
    // Otherwise prepare to double the current capacity
    assert(0 < sz && sz <= int64_t(m_cap));
    assert(m_cap < MaxSize && m_tableMask != 0);
    newCap = m_cap * 2;
    newMask = m_tableMask * 2 + 1;
  }
  // Perform the grow operation.
  grow(newCap, newMask);
}

void BaseSet::grow(uint32_t newCap, uint32_t newMask) {
  assert(m_size <= m_used && m_used <= m_cap);
  size_t newHashSize = size_t(newMask) + 1;
  assert(folly::isPowTwo(newHashSize) && computeMaxElms(newMask) == newCap);
  assert(m_size <= newCap && newCap <= MaxSize);
  auto* oldData = data();
  auto oldHashSize = oldData ? hashSize() : 0;
  auto needed = size_t(newCap) * sizeof(Elm) + newHashSize * sizeof(int32_t);
  auto* data = (Elm*)MM().objMallocLogged(needed);
  auto* table = (int32_t*)(data + size_t(newCap));
  m_data = data;
  m_hash = table;
  m_tableMask = newMask;
  initHash(table, newHashSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(oldData[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = data[toPos];
    toE = oldData[frPos];
    auto ie = findForNewInsert(table, newMask,
            toE.hasInt() ? toE.data.m_data.num : toE.data.m_data.pstr->hash());
    *ie = toPos;
  }
  if (oldData) {
    MM().objFreeLogged(
      oldData, size_t(m_cap) * sizeof(Elm) + oldHashSize * sizeof(int32_t));
  }
  m_cap = newCap;
  m_used = m_size;
}

void BaseSet::compact() {
  auto* elms = data();
  assert(elms);
  auto mask = m_tableMask;
  size_t tableSize = hashSize();
  auto table = hashTab();
  initHash(table, tableSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(elms[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = elms[toPos];
    if (toPos != frPos) {
      toE = elms[frPos];
    }
    auto ie = findForNewInsert(table, mask,
            toE.hasInt() ? toE.data.m_data.num : toE.data.m_data.pstr->hash());
    *ie = toPos;
  }
  m_used = m_size;
}

Array BaseSet::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return static_cast<const BaseSet*>(obj)->toArrayImpl();
}

bool BaseSet::ToBool(const ObjectData* obj) {
  return static_cast<const BaseSet*>(obj)->toBoolImpl();
}

bool BaseSet::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto st1 = static_cast<const BaseSet*>(obj1);
  auto st2 = static_cast<const BaseSet*>(obj2);
  if (st1->m_size != st2->m_size) return false;

  Elm* p = st1->data();
  Elm* pLimit = p + st1->iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    if (p->hasInt()) {
      int64_t key = p->data.m_data.num;
      if (!st2->contains(key)) return false;
    } else {
      assert(p->hasStr());
      StringData* key = p->data.m_data.pstr;
      if (!st2->contains(key)) return false;
    }
  }
  return true;
}

void BaseSet::Unserialize(const char* setType, ObjectData* obj,
                          VariableUnserializer* uns, int64_t sz, char type) {
  if (type != 'V') {
    throw Exception("%s does not support the '%c' serialization "
                    "format", setType, type);
  }
  auto st = static_cast<BaseSet*>(obj);
  st->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    Variant k;
    // When unserializing an element of a Set, we use Mode::ColKey for now.
    // This will make the unserializer to reserve an id for the element
    // but won't allow referencing the element via 'r' or 'R'.
    k.unserialize(uns, Uns::Mode::ColKey);
    if (k.isInteger()) {
      auto h = k.toInt64();
      auto p = st->findForInsert(h);
      assert(p);
      // Check to be robust against manually crafted inputs with conflicting
      // elements
      if (UNLIKELY(validPos(*p))) continue;
      auto& e = st->allocElm(p);
      e.setInt(h);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      auto p = st->findForInsert(key, h);
      assert(p);
      // Check to be robust against manually crafted inputs with conflicting
      // elements
      if (UNLIKELY(validPos(*p))) continue;
      auto& e = st->allocElm(p);
      e.setStr(key, h);
    } else {
      throw Exception("%s values must be integers or strings", setType);
    }
  }
}

bool BaseSet::contains(int64_t key) const {
  return find(key) != Empty;
}

bool BaseSet::contains(StringData* key) const {
  return find(key, key->hash()) != Empty;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, TSet*>::type
BaseSet::Clone(ObjectData* obj) {
  auto thiz = static_cast<TSet*>(obj);
  auto target = static_cast<TSet*>(obj->cloneImpl());

  if (!thiz->m_size) return target;

  assert(target->m_size == 0);
  assert(thiz->m_used != 0);
  target->m_capAndUsed = thiz->m_capAndUsed;
  target->m_tableMask = thiz->m_tableMask;
  target->m_size = thiz->m_size;
  auto needed =
    size_t(thiz->m_cap) * sizeof(Elm) + thiz->hashSize() * sizeof(int32_t);
  target->m_data = (Elm*)MM().objMallocLogged(needed);
  target->m_hash = (int32_t*)(target->m_data + target->m_cap);
  wordcpy(target->hashTab(), thiz->hashTab(), thiz->hashSize());

  for (ssize_t i = 0; i < thiz->iterLimit(); ++i) {
    Elm& e = thiz->data()[i];
    Elm& te = target->data()[i];
    if (thiz->isTombstone(e.data.m_type)) {
      te.data.m_type = e.data.m_type;
      continue;
    }
    te.data.hash() = e.data.hash();
    cellDup(e.data, te.data);
    if (te.hasStr()) te.data.m_data.pstr->incRefCount();
    assert(te.hash() == e.hash()); // ensure not clobbered.
  }

  return target;
}

// Protected (PHP-accesible methods)

void BaseSet::php_construct(const Variant& iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Object BaseSet::php_addAll(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    add(tv);
  }
  return this;
}

static int32_t empty_set_hash_slot = BaseSet::Empty;

Object BaseSet::php_clear() {
  deleteElms();
  freeData();
  uint32_t used = 0;
  uint32_t cap = 0;
  uint32_t tableMask = 0;
  int32_t version = m_version + 1;
  m_size = 0;
  m_capAndUsed = (uint64_t(cap) << 32) | uint64_t(used);
  m_maskAndVersion = (uint64_t(version) << 32) | uint64_t(tableMask);
  m_data = nullptr;
  m_hash = &empty_set_hash_slot;
  return this;
}

bool BaseSet::php_contains(const Variant& key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    return contains(key.toInt64());
  }
  if (IS_STRING_TYPE(t)) {
    return contains(key.getStringData());
  }
  throwBadValueType();
  return false;
}

Object BaseSet::php_remove(const Variant& key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    remove(key.toInt64());
  } else if (IS_STRING_TYPE(t)) {
    remove(key.getStringData());
  } else {
    throwBadValueType();
  }
  return this;
}

Array BaseSet::php_toValuesArray() {
  PackedArrayInit ai(m_size);
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    ai.append(tvAsCVarRef(&p->data));
  }
  return ai.create();
}

Object BaseSet::php_getIterator() {
  auto* it = NEWOBJ(c_SetIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_version = getVersion();
  return it;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_map(const Variant& callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  TSet* st;
  Object obj = st = NEWOBJ(TSet)();
  if (!m_size) return obj;
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    TypedValue tvCbRet;
    int32_t pVer = m_version;
    g_context->invokeFuncFew(&tvCbRet, ctx, 1, &p->data);
    // Now that tvCbRet is live, make sure to decref even if we throw.
    SCOPE_EXIT { tvRefcountedDecRef(&tvCbRet); };
    if (UNLIKELY(m_version != pVer)) throw_collection_modified();
    st->add(&tvCbRet);
  }
  return obj;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_filter(const Variant& callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  TSet* st;
  Object obj = st = NEWOBJ(TSet)();
  if (!m_size) return obj;
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    Variant ret;
    int32_t version = m_version;
    g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p->data);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!ret.toBoolean()) continue;
    if (p->hasInt()) {
      st->add(p->data.m_data.num);
    } else {
      assert(p->hasStr());
      st->add(p->data.m_data.pstr);
    }
  }
  return obj;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_zip(const Variant& iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  if (m_size && iter) {
    // At present, BaseSets only support int values and string values,
    // so if this BaseSet is non empty and the iterable is non empty
    // the zip operation will always fail
    throwBadValueType();
  }
  Object obj = NEWOBJ(TSet)();
  return obj;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_take(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  if (len >= int64_t(m_size)) {
    // We know the result Set will simply be a copy of this Set,
    // so we can just call Clone() and return early here.
    return Object::attach(TSet::Clone(this));
  }
  auto* st = NEWOBJ(TSet)();
  Object obj = st;
  if (len <= 0) {
    // We know the resulting Set will be empty, so we can return
    // early here.
    return obj;
  }
  size_t sz = size_t(len);
  st->reserve(sz);
  st->m_size = st->m_used = sz;
  auto table = st->hashTab();
  auto mask = st->m_tableMask;
  for (uint32_t frPos = 0, toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(m_data[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = st->m_data[toPos];
    toE.data.hash() = m_data[frPos].data.hash();
    cellDup(m_data[frPos].data, toE.data);
    auto ie = findForNewInsert(table, mask,
            toE.hasInt() ? toE.data.m_data.num : toE.data.m_data.pstr->hash());
    *ie = toPos;
  }
  return obj;
}

template<class TSet, bool checkVersion>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* st = NEWOBJ(TSet);
  Object obj = st;
  if (!m_size) return obj;
  uint32_t used = iterLimit();
  for (uint i = 0; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant ret;
    if (checkVersion) {
      int32_t version = m_version;
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    } else {
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
    }
    if (!ret.toBoolean()) continue;
    if (p.hasInt()) {
      st->add(p.data.m_data.num);
    } else {
      assert(p.hasStr());
      st->add(p.data.m_data.pstr);
    }
  }
  return obj;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  if (len <= 0) {
    // We know the resulting Set will simply be a copy of this Set,
    // so we can just call Clone() and return early here.
    return Object::attach(TSet::Clone(this));
  }
  auto* st = NEWOBJ(TSet)();
  Object obj = st;
  if (len >= m_size) {
    // We know the resulting Set will be empty, so we can return
    // early here.
    return obj;
  }
  size_t sz = size_t(m_size) - size_t(len);
  assert(sz);
  st->reserve(sz);
  st->m_size = st->m_used = sz;
  uint32_t frPos;
  if (LIKELY(!hasTombstones())) {
    // Fast path: Set contains no tombstones
    frPos = len;
  } else {
    // Slow path: Set has at least one tombstone, so we need to
    // count forward
    frPos = 0;
    while (len > 0) {
      while (isTombstone(m_data[frPos].data.m_type)) {
        assert(frPos + 1 < m_used);
        ++frPos;
      }
      --len;
      ++frPos;
    }
  }
  auto table = st->hashTab();
  auto mask = st->m_tableMask;
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(m_data[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = st->m_data[toPos];
    toE.data.hash() = m_data[frPos].data.hash();
    cellDup(m_data[frPos].data, toE.data);
    auto ie = findForNewInsert(table, mask,
            toE.hasInt() ? toE.data.m_data.num : toE.data.m_data.pstr->hash());
    *ie = toPos;
  }
  return obj;
}

template<class TSet, bool checkVersion>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* st = NEWOBJ(TSet)();
  Object obj = st;
  if (!m_size) return obj;
  uint32_t used = iterLimit();
  uint i = 0;
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant ret;
    if (checkVersion) {
      int32_t version = m_version;
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    } else {
      g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
    }
    if (!ret.toBoolean()) break;
  }
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    if (p.hasInt()) {
      st->add(p.data.m_data.num);
    } else {
      assert(p.hasStr());
      st->add(p.data.m_data.pstr);
    }
  }
  return obj;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromItems(const Variant& iterable) {
  if (iterable.isNull()) return NEWOBJ(TSet)();
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  TSet* target;
  Object ret = target = NEWOBJ(TSet)();
  for (; iter; ++iter) {
    Variant v = iter.second();
    if (v.isInteger()) {
      target->add(v.toInt64());
    } else if (v.isString()) {
      target->add(v.getStringData());
    } else {
      throwBadValueType();
    }
  }
  return ret;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromArray(const Variant& arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  TSet* st;
  Object ret = st = NEWOBJ(TSet)();
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    const Variant& v = ad->getValueRef(pos);
    if (v.isInteger()) {
      st->add(v.toInt64());
    } else if (v.isString()) {
      st->add(v.getStringData());
    } else {
      throwBadValueType();
    }
  }
  return ret;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromArrays(int _argc, const Array& _argv /* = null_array */) {
  TSet* st;
  Object ret = st = NEWOBJ(TSet)();
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant arr = iter.second();
    if (!arr.isArray()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Parameters must be arrays"));
      throw e;
    }
    ArrayData* ad = arr.getArrayData();
    for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
         pos = ad->iter_advance(pos)) {
      st->php_add(ad->getValueRef(pos));
    }
  }
  return ret;
}

// Protected (Internal)

BaseSet::BaseSet(Class* cls) : ExtCollectionObjectData(cls) {
  uint32_t used = 0;
  uint32_t cap = 0;
  uint32_t tableMask = 0;
  int32_t version = 0;
  m_size = 0;
  m_capAndUsed = (uint64_t(cap) << 32) | uint64_t(used);
  m_maskAndVersion = (uint64_t(version) << 32) | uint64_t(tableMask);
  m_data = nullptr;
  m_hash = &empty_set_hash_slot;
}

BaseSet::~BaseSet() {
  deleteElms();
  freeData();
}

void BaseSet::freeData() {
  if (m_data) {
    MM().objFreeLogged(
      m_data, size_t(m_cap) * sizeof(Elm) + hashSize() * sizeof(int32_t));
  }
}

void BaseSet::deleteElms() {
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    tvRefcountedDecRef(&p->data);
  }
}

// Private

NEVER_INLINE
void BaseSet::warnOnStrIntDup() const {
  smart::hash_set<int64_t> seenVals;

  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    int64_t newVal = 0;

    if (p->hasInt()) {
      newVal = p->data.m_data.num;
    } else {
      assert(p->hasStr());
      // isStriclyInteger() puts the int value in newVal as a side effect.
      if (!p->data.m_data.pstr->isStrictlyInteger(newVal)) continue;
    }

    if (seenVals.find(newVal) != seenVals.end()) {
      raise_warning(
        "Set::toArray() for a set containing both int(%" PRId64 ") "
        "and string('%" PRId64 "')",
        newVal,
        newVal
      );

      return;
    }

    seenVals.insert(newVal);
  }
  // Do nothing if no 'duplicates' were found.
}

Array BaseSet::toArrayImpl() const {
  ArrayInit ai(m_size);
  Elm* p = data();
  Elm* pLimit = p + iterLimit();
  for (; p != pLimit; ++p) {
    if (isTombstone(p->data.m_type)) continue;
    if (p->hasInt()) {
      ai.set(p->data.m_data.num, tvAsCVarRef(&p->data));
    } else {
      assert(p->hasStr());
      ai.set(p->data.m_data.pstr, tvAsCVarRef(&p->data));
    }
  }

  Array arr = ai.create();

  // If both '123' and 123 are present in the set, we better warn the user.
  if (UNLIKELY(arr.length() < m_size)) warnOnStrIntDup();
  return arr;
}

NEVER_INLINE
void BaseSet::throwTooLarge() {
  static const size_t reserveSize = 130;
  String msg(reserveSize, ReserveString);
  char* buf = msg.bufferSlice().ptr;
  int sz = sprintf(buf,
                   "Set object has reached its maximum capacity of "
                   "%u element slots and does not have room to add a "
                   "new element",
                   MaxSize);
  assert(sz <= reserveSize);
  msg.setSize(sz);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(msg));
  throw e;
}

NEVER_INLINE
void BaseSet::throwReserveTooLarge() {
  static const size_t reserveSize = 80;
  String msg(reserveSize, ReserveString);
  char* buf = msg.bufferSlice().ptr;
  int sz = sprintf(buf,
                   "Set does not support reserving room for more than "
                   "%u elements",
                   MaxReserveSize);
  assert(sz <= reserveSize);
  msg.setSize(sz);
  Object e(SystemLib::AllocInvalidOperationExceptionObject(msg));
  throw e;
}

NEVER_INLINE
int32_t* BaseSet::warnUnbalanced(size_t n, int32_t* ei) {
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    raise_error("Set is too unbalanced (%lu)", n);
  }
  return ei;
}

static bool hitString(const BaseSet::Elm& e, const StringData* s,
                      int32_t hash) {
  // hitString() should only be called on an Elm that is referenced by a
  // hash table entry. c_Set guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!BaseSet::isTombstone(e.data.m_type));
  return hash == e.hash() && e.hasStr() &&
         (s == e.data.m_data.pstr || s->same(e.data.m_data.pstr));
}

static bool hitInt(const BaseSet::Elm& e, int64_t ki) {
  // hitInt() should only be called on an Elm that is referenced by a
  // hash table entry. c_Set guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!BaseSet::isTombstone(e.data.m_type));
  return e.hasInt() && e.data.m_data.num == ki;
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2. In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.

template <class Hit>
ALWAYS_INLINE
ssize_t BaseSet::findImpl(size_t h0, Hit hit) const {
  size_t tableMask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  for (size_t probeIndex = h0, i = 1;; ++i) {
    ssize_t pos = hashtable[probeIndex & tableMask];
    if ((validPos(pos) && hit(elms[pos])) || pos == Empty) {
      return pos;
    }
    probeIndex += i;
    assert(i <= tableMask && probeIndex == h0 + (i + i*i) / 2);
  }
}

ssize_t BaseSet::find(int64_t ki) const {
  return findImpl(ki, [ki] (const Elm& e) {
    return hitInt(e, ki);
  });
}

ssize_t BaseSet::find(const StringData* s, strhash_t prehash) const {
  auto h = prehash | STRHASH_MSB;
  return findImpl(prehash, [s, h] (const Elm& e) {
    return hitString(e, s, h);
  });
}

template <class Hit>
ALWAYS_INLINE
int32_t* BaseSet::findForInsertImpl(size_t h0, Hit hit) const {
  // mask, probe, and pos are explicitly 64-bit, because performance
  // regressed when they were 32-bit types via auto. Test carefully.
  size_t mask = m_tableMask;
  auto* elms = data();
  auto* hashtable = hashTab();
  int32_t* ret = nullptr;
  for (size_t probe = h0, i = 1;; ++i) {
    auto ei = &hashtable[probe & mask];
    ssize_t pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        return ei;
      }
    } else {
      if (!ret) ret = ei;
      if (pos == Empty) {
        return LIKELY(i <= 100) ? ret : warnUnbalanced(i, ret);
      }
    }
    probe += i;
    assert(i <= mask && probe == h0 + (i + i*i) / 2);
  }
}

int32_t* BaseSet::findForInsert(int64_t ki) const {
  return findForInsertImpl(ki, [ki] (const Elm& e) {
    return hitInt(e, ki);
  });
}

int32_t* BaseSet::findForInsert(const StringData* s, strhash_t prehash) const {
  auto h = prehash | STRHASH_MSB;
  return findForInsertImpl(prehash, [s, h] (const Elm& e) {
    return hitString(e, s, h);
  });
}

// findForNewInsert() is only safe to use if you know for sure that the
// key is not already present in the Set.
ALWAYS_INLINE int32_t*
BaseSet::findForNewInsert(int32_t* table, size_t mask, size_t h0) const {
  for (size_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) return ei;
    probe += i;
    assert(i <= mask && probe == h0 + (i + i*i) / 2);
  }
}

ALWAYS_INLINE
int32_t* BaseSet::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), m_tableMask, h0);
}

void BaseSet::erase(int32_t* pos) {
  assert(validPos(*pos) && !isTombstone(*pos));
  assert(data());
  Elm* elms = data();
  auto& e = elms[*pos];
  // Mark the hash slot as a tombstone.
  *pos = Tombstone;
  // Mark the Elm as a tombstone.
  TypedValue* tv = &e.data;
  DataType oldType = tv->m_type;
  uint64_t oldDatum = tv->m_data.num;
  tv->m_type = KindOfInvalid;
  --m_size;
  // Don't adjust m_used. This frees us from having to explicitly keep track
  // of hash load, since we can instead rely on the isFull() condition to
  // trigger a grow/compact before hash load gets too high.
  assert(m_used <= m_cap);
  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);
  // Compact in order to keep elms from being overly sparse. Other parts
  // of Set's implementation rely on the fact that erase() does not allow
  // the Elm density to drop below ~50%.
  if (isDensityTooLow()) {
    compact();
  }
}

void BaseSet::throwBadValueType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer values and string values may be used with Sets"));
  throw e;
}

///////////////////////////////////////////////////////////////////////////////
// Set

c_Set::c_Set(Class* cls /* = c_Set::classof() */) : BaseSet(cls) {
  o_subclassData.u16 = Collection::SetType;
}

void c_Set::t___construct(const Variant& iterable /* = null_variant */) {
  BaseSet::php_construct(iterable);
}

Object c_Set::t_add(const Variant& val) {
  return BaseSet::php_add(val);
}

Object c_Set::t_addall(const Variant& iterable) {
  return BaseSet::php_addAll(iterable);
}

Object c_Set::t_clear() {
  return BaseSet::php_clear();
}

bool c_Set::t_isempty() {
  return BaseSet::php_isEmpty();
}

int64_t c_Set::t_count() {
  return BaseSet::php_count();
}

Object c_Set::t_items() {
  return BaseSet::php_items();
}

Object c_Set::t_values() {
  return BaseSet::php_values<c_Vector>();
}

Object c_Set::t_lazy() {
  return BaseSet::php_lazy();
}

bool c_Set::t_contains(const Variant& key) {
  return BaseSet::php_contains(key);
}

Object c_Set::t_remove(const Variant& key) {
  return BaseSet::php_remove(key);
}

Array c_Set::t_toarray() {
  return BaseSet::php_toArray();
}

Array c_Set::t_tokeysarray() {
  return BaseSet::php_toKeysArray();
}

Array c_Set::t_tovaluesarray() {
  return BaseSet::php_toValuesArray();
}

Object c_Set::t_getiterator() {
  return BaseSet::php_getIterator();
}

Object c_Set::t_map(const Variant& callback) {
  return BaseSet::php_map<c_Set>(callback);
}

Object c_Set::t_filter(const Variant& callback) {
  return BaseSet::php_filter<c_Set>(callback);
}

Object c_Set::t_zip(const Variant& iterable) {
  return BaseSet::php_zip<c_Set>(iterable);
}

Object c_Set::t_take(const Variant& n) {
  return BaseSet::php_take<c_Set>(n);
}

Object c_ImmSet::t_take(const Variant& n) {
  return BaseSet::php_take<c_ImmSet>(n);
}

Object c_Set::t_takewhile(const Variant& fn) {
  return BaseSet::php_takeWhile<c_Set, true>(fn);
}

Object c_ImmSet::t_takewhile(const Variant& fn) {
  return BaseSet::php_takeWhile<c_ImmSet, false>(fn);
}

Object c_Set::t_skip(const Variant& n) {
  return BaseSet::php_skip<c_Set>(n);
}

Object c_ImmSet::t_skip(const Variant& n) {
  return BaseSet::php_skip<c_ImmSet>(n);
}

Object c_Set::t_skipwhile(const Variant& fn) {
  return BaseSet::php_skipWhile<c_Set, true>(fn);
}

Object c_ImmSet::t_skipwhile(const Variant& fn) {
  return BaseSet::php_skipWhile<c_ImmSet, false>(fn);
}

Object c_Set::t_removeall(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    php_remove(iter.second());
  }
  return this;
}

Object c_Set::t_difference(const Variant& iterable) {
  return t_removeall(iterable);
}

Object c_Set::ti_fromitems(const Variant& iterable) {
  return BaseSet::php_fromItems<c_Set>(iterable);
}

Object c_Set::ti_fromarray(const Variant& arr) {
  return BaseSet::php_fromArray<c_Set>(arr);
}

Object c_Set::ti_fromarrays(int _argc, const Array& _argv /* = null_array */) {
  return BaseSet::php_fromArrays<c_Set>(_argc, _argv);
}

void c_Set::Unserialize(ObjectData* obj, VariableUnserializer* uns,
                        int64_t sz, char type) {

  BaseSet::Unserialize("Set", obj, uns, sz, type);
}

c_Set* c_Set::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_Set>(obj);
}

Object c_Set::t_immutable() {
  auto* st = NEWOBJ(c_ImmSet)();
  Object o = st;
  st->init(VarNR(this));
  return o;
}

///////////////////////////////////////////////////////////////////////////////
// ImmSet

void c_ImmSet::t___construct(const Variant& iterable /* = null_variant */) {
  BaseSet::php_construct(iterable);
}

bool c_ImmSet::t_isempty() {
  return BaseSet::php_isEmpty();
}

int64_t c_ImmSet::t_count() {
  return BaseSet::php_count();
}

Object c_ImmSet::t_items() {
  return BaseSet::php_items();
}

Object c_ImmSet::t_values() {
  return BaseSet::php_values<c_ImmVector>();
}

Object c_ImmSet::t_lazy() {
  return BaseSet::php_lazy();
}

bool c_ImmSet::t_contains(const Variant& key) {
  return BaseSet::php_contains(key);
}

Array c_ImmSet::t_toarray() {
  return BaseSet::php_toArray();
}

Array c_ImmSet::t_tokeysarray() {
  return BaseSet::php_toKeysArray();
}

Array c_ImmSet::t_tovaluesarray() {
  return BaseSet::php_toValuesArray();
}

Object c_ImmSet::t_getiterator() {
  return BaseSet::php_getIterator();
}

Object c_ImmSet::t_map(const Variant& callback) {
  return BaseSet::php_map<c_ImmSet>(callback);
}

Object c_ImmSet::t_filter(const Variant& callback) {
  return BaseSet::php_filter<c_ImmSet>(callback);
}

Object c_ImmSet::t_zip(const Variant& iterable) {
  return BaseSet::php_zip<c_ImmSet>(iterable);
}

Object c_ImmSet::ti_fromitems(const Variant& iterable) {
  return BaseSet::php_fromItems<c_ImmSet>(iterable);
}

Object c_ImmSet::ti_fromarrays(int _argc, const Array& _argv) {
  return BaseSet::php_fromArrays<c_ImmSet>(_argc, _argv);
}

c_ImmSet::c_ImmSet(Class* cls) : BaseSet(cls) {
  o_subclassData.u16 = Collection::ImmSetType;
}

void c_ImmSet::Unserialize(ObjectData* obj, VariableUnserializer* uns,
    int64_t sz, char type) {
  BaseSet::Unserialize("ImmSet", obj, uns, sz, type);
}

c_ImmSet* c_ImmSet::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_ImmSet>(obj);
}

Object c_ImmSet::t_immutable() {
  return this;
}

///////////////////////////////////////////////////////////////////////////////

c_SetIterator::c_SetIterator(Class* cb) : ExtObjectData(cb) {
}

c_SetIterator::~c_SetIterator() {
}

void c_SetIterator::t___construct() {
}

Variant c_SetIterator::t_current() {
  BaseSet* st = m_obj.get();
  if (UNLIKELY(m_version != st->getVersion())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(st->iter_value(m_pos));
}

Variant c_SetIterator::t_key() {
  return t_current();
}

bool c_SetIterator::t_valid() {
  return m_pos != 0;
}

void c_SetIterator::t_next() {
  BaseSet* st = m_obj.get();
  if (UNLIKELY(m_version != st->getVersion())) {
    throw_collection_modified();
  }
  m_pos = st->iter_next(m_pos);
}

void c_SetIterator::t_rewind() {
  BaseSet* st = m_obj.get();
  if (UNLIKELY(m_version != st->getVersion())) {
    throw_collection_modified();
  }
  m_pos = st->iter_begin();
}

///////////////////////////////////////////////////////////////////////////////

c_Pair::c_Pair(Class* cb)
  : ExtObjectDataFlags(cb)
  , m_size(0)
{
  o_subclassData.u16 = Collection::PairType;
}

c_Pair::~c_Pair() {
  if (LIKELY(m_size == 2)) {
    tvRefcountedDecRef(&elm0);
    tvRefcountedDecRef(&elm1);
    return;
  }
  if (m_size == 1) {
    tvRefcountedDecRef(&elm0);
  }
}

void c_Pair::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
    "Pairs cannot be created using the new operator"));
  throw e;
}

Array c_Pair::toArrayImpl() const {
  assert(isFullyConstructed());
  return make_packed_array(tvAsCVarRef(&elm0), tvAsCVarRef(&elm1));
}

c_Pair* c_Pair::Clone(ObjectData* obj) {
  auto thiz = static_cast<c_Pair*>(obj);
  auto pair = static_cast<c_Pair*>(obj->cloneImpl());
  assert(thiz->isFullyConstructed());
  pair->incRefCount();
  pair->m_size = 2;
  cellDup(thiz->elm0, pair->elm0);
  cellDup(thiz->elm1, pair->elm1);
  return pair;
}

bool c_Pair::t_isempty() {
  assert(isFullyConstructed());
  return false;
}

int64_t c_Pair::t_count() {
  assert(isFullyConstructed());
  return 2;
}

Object c_Pair::t_items() {
  assert(isFullyConstructed());
  return SystemLib::AllocLazyIterableViewObject(this);
}

Object c_Pair::t_keys() {
  assert(isFullyConstructed());
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  vec->reserve(2);
  vec->m_size = 2;
  vec->m_data[0].m_data.num = 0;
  vec->m_data[0].m_type = KindOfInt64;
  vec->m_data[1].m_data.num = 1;
  vec->m_data[1].m_type = KindOfInt64;
  return obj;
}

Object c_Pair::t_values() {
  auto* vec = NEWOBJ(c_ImmVector)();
  Object o = vec;
  vec->init(VarNR(this));
  return o;
}

Object c_Pair::t_lazy() {
  assert(isFullyConstructed());
  return SystemLib::AllocLazyKeyedIterableViewObject(this);
}

Object c_Pair::t_kvzip() {
  assert(isFullyConstructed());
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  vec->reserve(2);
  for (uint i = 0; i < 2; ++i) {
    auto* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->elm0.m_type = KindOfInt64;
    pair->elm0.m_data.num = i;
    ++pair->m_size;
    pair->initAdd(&getElms()[i]);
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
  }
  return obj;
}

Variant c_Pair::t_at(const Variant& key) {
  assert(isFullyConstructed());
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return init_null_variant;
}

Variant c_Pair::t_get(const Variant& key) {
  assert(isFullyConstructed());
  if (key.isInteger()) {
    TypedValue* tv = get(key.toInt64());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return init_null_variant;
    }
  }
  throwBadKeyType();
  return init_null_variant;
}

bool c_Pair::t_containskey(const Variant& key) {
  assert(isFullyConstructed());
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

Array c_Pair::t_toarray() {
  assert(isFullyConstructed());
  return toArrayImpl();
}

Array c_Pair::t_tokeysarray() {
  assert(isFullyConstructed());
  PackedArrayInit ai(2);
  ai.append((int64_t)0);
  ai.append((int64_t)1);
  return ai.toArray();
}

Array c_Pair::t_tovaluesarray() {
  assert(isFullyConstructed());
  return toArrayImpl();
}

Object c_Pair::t_getiterator() {
  assert(isFullyConstructed());
  auto* it = NEWOBJ(c_PairIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  return it;
}

Object c_Pair::t_map(const Variant& callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  vec->reserve(2);
  for (uint64_t i = 0; i < 2; ++i) {
    g_context->invokeFuncFew(&vec->m_data[i], ctx, 1, &getElms()[i]);
    ++vec->m_size;
  }
  return obj;
}

Object c_Pair::t_mapwithkey(const Variant& callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  vec->reserve(2);
  for (uint64_t i = 0; i < 2; ++i) {
    TypedValue args[2] = { make_tv<KindOfInt64>(i), getElms()[i] };
    g_context->invokeFuncFew(&vec->m_data[i], ctx, 2, args);
    ++vec->m_size;
  }
  return obj;
}

Object c_Pair::t_filter(const Variant& callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  for (uint64_t i = 0; i < 2; ++i) {
    Variant ret;
    g_context->invokeFuncFew(ret.asTypedValue(), ctx, 1, &getElms()[i]);
    if (ret.toBoolean()) {
      vec->add(&getElms()[i]);
    }
  }
  return obj;
}

Object c_Pair::t_filterwithkey(const Variant& callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  for (uint64_t i = 0; i < 2; ++i) {
    Variant ret;
    TypedValue args[2] = { make_tv<KindOfInt64>(i), getElms()[i] };
    g_context->invokeFuncFew(ret.asTypedValue(), ctx, 2, args);
    if (ret.toBoolean()) {
      vec->add(&getElms()[i]);
    }
  }
  return obj;
}

Object c_Pair::t_zip(const Variant& iterable) {
  assert(isFullyConstructed());
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  vec->reserve(std::min(sz, size_t(2)));
  for (uint64_t i = 0; i < 2 && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (vec->m_capacity <= vec->m_size) {
      vec->grow();
    }
    auto* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->initAdd(&getElms()[i]);
    pair->initAdd(cvarToCell(&v));
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
  }
  return obj;
}

Object c_Pair::t_immutable() {
  return this;
}

Object c_Pair::t_take(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  auto* vec = NEWOBJ(c_Vector)();
  Object obj = vec;
  if (len <= 0) {
    return obj;
  }
  size_t sz = std::min(size_t(len), size_t(2));
  vec->reserve(sz);
  vec->m_size = sz;
  for (size_t i = 0; i < sz; ++i) {
    cellDup(getElms()[i], vec->m_data[i]);
  }
  return obj;
}

Object c_Pair::t_takewhile(const Variant& callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(c_Vector)();
  Object obj = vec;
  for (uint i = 0; i < 2; ++i) {
    Variant retval;
    g_context->invokeFuncFew(retval.asTypedValue(), ctx, 1, &getElms()[i]);
    if (!retval.toBoolean()) break;
    vec->add(&getElms()[i]);
  }
  return obj;
}

Object c_Pair::t_skip(const Variant& n) {
  if (!n.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter n must be an integer"));
    throw e;
  }
  int64_t len = n.toInt64();
  auto* vec = NEWOBJ(c_Vector);
  Object obj = vec;
  if (len <= 0) len = 0;
  size_t skipAmt = std::min<size_t>(len, 2);
  size_t sz = size_t(m_size) - skipAmt;
  vec->reserve(sz);
  vec->m_size = sz;
  for (size_t i = 0; i < sz; ++i) {
    cellDup(getElms()[i + skipAmt], vec->m_data[i]);
  }
  return obj;
}

Object c_Pair::t_skipwhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto* vec = NEWOBJ(c_Vector)();
  Object obj = vec;
  uint i = 0;
  for (; i < 2; ++i) {
    Variant retval;
    g_context->invokeFuncFew(retval.asTypedValue(), ctx, 1, &getElms()[i]);
    if (!retval.toBoolean()) break;
  }
  for (; i < 2; ++i) {
    vec->add(&getElms()[i]);
  }
  return obj;
}

void c_Pair::throwOOB(int64_t key) {
  throwIntOOB(key, true);
}

void c_Pair::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys may be used with Pairs"));
  throw e;
}

Array c_Pair::ToArray(const ObjectData* obj) {
  auto pair = static_cast<const c_Pair*>(obj);
  assert(pair->isFullyConstructed());
  check_collection_cast_to_array();
  return pair->toArrayImpl();
}

template <bool throwOnMiss>
TypedValue* c_Pair::OffsetAt(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assert(pair->isFullyConstructed());
  if (key->m_type == KindOfInt64) {
    return throwOnMiss ? pair->at(key->m_data.num)
                       : pair->get(key->m_data.num);
  }
  throwBadKeyType();
  return nullptr;
}

bool c_Pair::OffsetIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assert(pair->isFullyConstructed());
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = pair->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellIsNull(result) : false;
}

bool c_Pair::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assert(pair->isFullyConstructed());
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = pair->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellToBool(*result) : true;
}

bool c_Pair::OffsetContains(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assert(pair->isFullyConstructed());
  if (key->m_type == KindOfInt64) {
    return pair->contains(key->m_data.num);
  } else {
    throwBadKeyType();
    return false;
  }
}

bool c_Pair::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto pair1 = static_cast<const c_Pair*>(obj1);
  auto pair2 = static_cast<const c_Pair*>(obj2);
  assert(pair1->isFullyConstructed());
  assert(pair2->isFullyConstructed());
  return equal(tvAsCVarRef(&pair1->elm0), tvAsCVarRef(&pair2->elm0)) &&
         equal(tvAsCVarRef(&pair1->elm1), tvAsCVarRef(&pair2->elm1));
}

void c_Pair::Unserialize(ObjectData* obj,
                         VariableUnserializer* uns,
                         int64_t sz,
                         char type) {
  assert(sz == 2);
  if (type != 'V') {
    throw Exception("Pair does not support the '%c' serialization "
                    "format", type);
  }
  auto pair = static_cast<c_Pair*>(obj);
  pair->m_size = 2;
  pair->elm0.m_type = KindOfNull;
  pair->elm1.m_type = KindOfNull;
  tvAsVariant(&pair->elm0).unserialize(uns, Uns::Mode::ColValue);
  tvAsVariant(&pair->elm1).unserialize(uns, Uns::Mode::ColValue);
}

c_PairIterator::c_PairIterator(Class* cb) :
    ExtObjectData(cb) {
}

c_PairIterator::~c_PairIterator() {
}

void c_PairIterator::t___construct() {
}

Variant c_PairIterator::t_current() {
  c_Pair* pair = m_obj.get();
  if (!pair->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(&pair->getElms()[m_pos]);
}

Variant c_PairIterator::t_key() {
  c_Pair* pair = m_obj.get();
  if (!pair->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return m_pos;
}

bool c_PairIterator::t_valid() {
  assert(m_pos >= 0);
  c_Pair* pair = m_obj.get();
  return pair && (m_pos < ssize_t(2));
}

void c_PairIterator::t_next() {
  m_pos++;
}

void c_PairIterator::t_rewind() {
  m_pos = 0;
}

///////////////////////////////////////////////////////////////////////////////

#define COLLECTION_MAGIC_METHODS(cls) \
  String c_##cls::t___tostring() { \
    return #cls; \
  } \
  Variant c_##cls::t___get(Variant name) { \
    throw_collection_property_exception(); \
  } \
  Variant c_##cls::t___set(Variant name, Variant value) { \
    throw_collection_property_exception(); \
  } \
  bool c_##cls::t___isset(Variant name) { \
    return false; \
  } \
  Variant c_##cls::t___unset(Variant name) { \
    throw_collection_property_exception(); \
  }

COLLECTION_MAGIC_METHODS(Vector)
COLLECTION_MAGIC_METHODS(ImmVector)
COLLECTION_MAGIC_METHODS(Map)
COLLECTION_MAGIC_METHODS(ImmMap)
COLLECTION_MAGIC_METHODS(Set)
COLLECTION_MAGIC_METHODS(ImmSet)
COLLECTION_MAGIC_METHODS(Pair)

#undef COLLECTION_MAGIC_METHODS

#define ITERABLE_MATERIALIZE_METHODS(cls) \
  Object c_##cls::t_tovector() { \
    auto* vec = NEWOBJ(c_Vector)(); \
    Object o = vec; \
    vec->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_toimmvector() { \
    auto* vec = NEWOBJ(c_ImmVector)(); \
    Object o = vec; \
    vec->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_toset() { \
    auto* st = NEWOBJ(c_Set)(); \
    Object o = st; \
    st->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_toimmset() { \
    auto* st = NEWOBJ(c_ImmSet)(); \
    Object o = st; \
    st->init(VarNR(this)); \
    return o; \
  }

#define KEYEDITERABLE_MATERIALIZE_METHODS(cls) \
  ITERABLE_MATERIALIZE_METHODS(cls) \
  Object c_##cls::t_tomap() { \
    auto* mp = NEWOBJ(c_Map)(); \
    Object o = mp; \
    mp->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_toimmmap() { \
    auto* mp = NEWOBJ(c_ImmMap)(); \
    Object o = mp; \
    mp->init(VarNR(this)); \
    return o; \
  }
KEYEDITERABLE_MATERIALIZE_METHODS(Map)
KEYEDITERABLE_MATERIALIZE_METHODS(ImmMap)
ITERABLE_MATERIALIZE_METHODS(Set)
ITERABLE_MATERIALIZE_METHODS(ImmSet)
KEYEDITERABLE_MATERIALIZE_METHODS(Pair)
KEYEDITERABLE_MATERIALIZE_METHODS(ImmVector)

#undef ITERABLE_MATERIALIZE_METHODS
#undef KEYEDITERABLE_MATERIALIZE_METHODS

static inline bool isKeylessCollectionType(Collection::Type ctype) {
  return Collection::isSetType(ctype);
}

void collectionSerialize(ObjectData* obj, VariableSerializer* serializer) {
  assert(obj->isCollection());
  int64_t sz = getCollectionSize(obj);
  if (Collection::isVectorType(obj->getCollectionType()) ||
      Collection::isSetType(obj->getCollectionType()) ||
      obj->getCollectionType() == Collection::PairType) {
    serializer->setObjectInfo(obj->o_getClassName(), obj->o_getId(), 'V');
    serializer->writeArrayHeader(sz, true);
    if (serializer->getType() == VariableSerializer::Type::Serialize ||
        serializer->getType() == VariableSerializer::Type::APCSerialize ||
        serializer->getType() == VariableSerializer::Type::DebuggerSerialize ||
        serializer->getType() == VariableSerializer::Type::VarExport ||
        serializer->getType() == VariableSerializer::Type::PHPOutput) {
      // For the 'V' serialization format, we don't print out keys
      // for Serialize, APCSerialize, DebuggerSerialize
      for (ArrayIter iter(obj); iter; ++iter) {
        serializer->writeCollectionKeylessPrefix();
        serializer->writeArrayValue(iter.second());
      }
    } else {
      for (ArrayIter iter(obj); iter; ++iter) {
        if (isKeylessCollectionType(obj->getCollectionType())) {
          serializer->writeCollectionKeylessPrefix();
        } else {
          serializer->writeCollectionKey(iter.first());
        }
        serializer->writeArrayValue(iter.second());
      }
    }
    serializer->writeArrayFooter();
  } else {
    assert(Collection::isMapType(obj->getCollectionType()));
    serializer->setObjectInfo(obj->o_getClassName(), obj->o_getId(), 'K');
    serializer->writeArrayHeader(sz, false);
    for (ArrayIter iter(obj); iter; ++iter) {
      serializer->writeCollectionKey(iter.first());
      serializer->writeArrayValue(iter.second());
    }
    serializer->writeArrayFooter();
  }
}

void collectionDeepCopyTV(TypedValue* tv) {
  switch (tv->m_type) {
    case KindOfArray: {
      ArrayData* arr = collectionDeepCopyArray(tv->m_data.parr);
      decRefArr(tv->m_data.parr);
      tv->m_data.parr = arr;
      break;
    }
    case KindOfObject: {
      ObjectData* obj = tv->m_data.pobj;
      if (!obj->isCollection()) break;
      switch (obj->getCollectionType()) {
        case Collection::VectorType:
          obj = collectionDeepCopyVector(static_cast<c_Vector*>(obj));
          break;
        case Collection::MapType:
          obj = collectionDeepCopyMap(static_cast<c_Map*>(obj));
          break;
        case Collection::ImmMapType:
          obj = collectionDeepCopyImmMap(static_cast<c_ImmMap*>(obj));
          break;
        case Collection::SetType:
          obj = collectionDeepCopySet(static_cast<c_Set*>(obj));
          break;
        case Collection::PairType:
          obj = collectionDeepCopyPair(static_cast<c_Pair*>(obj));
          break;
        case Collection::ImmSetType:
          obj = collectionDeepCopyImmSet(static_cast<c_ImmSet*>(obj));
          break;
        case Collection::ImmVectorType:
          obj = collectionDeepCopyImmVector(
                  static_cast<c_ImmVector*>(obj));
          break;
        case Collection::InvalidType:
          assert(false);
          obj = nullptr;
          break;
      }
      decRefObj(tv->m_data.pobj);
      tv->m_data.pobj = obj;
      break;
    }
    case KindOfResource:
    case KindOfRef: {
      assert(false);
      break;
    }
    default: break;
  }
}

ArrayData* collectionDeepCopyArray(ArrayData* arr) {
  Array a = arr = arr->copy();
  for (ArrayIter iter(arr); iter; ++iter) {
    collectionDeepCopyTV((TypedValue*)(&iter.secondRef()));
  }
  return a.detach();
}

template<typename TVector>
ObjectData* collectionDeepCopyBaseVector(TVector *vec) {
  vec = TVector::Clone(vec);
  Object o = Object::attach(vec);
  size_t sz = vec->m_size;
  for (size_t i = 0; i < sz; ++i) {
    collectionDeepCopyTV(&vec->m_data[i]);
  }
  return o.detach();
}

ObjectData* collectionDeepCopyVector(c_Vector* vec) {
  return collectionDeepCopyBaseVector<c_Vector>(vec);
}

ObjectData* collectionDeepCopyImmVector(c_ImmVector* vec) {
  return collectionDeepCopyBaseVector<c_ImmVector>(vec);
}

ObjectData* collectionDeepCopySet(c_Set* st) {
  return c_Set::Clone(st);
}

ObjectData* collectionDeepCopyImmSet(c_ImmSet* st) {
  return c_ImmSet::Clone(st);
}

ObjectData* collectionDeepCopyPair(c_Pair* pair) {
  pair = c_Pair::Clone(pair);
  Object o = Object::attach(pair);
  collectionDeepCopyTV(&pair->elm0);
  collectionDeepCopyTV(&pair->elm1);
  return o.detach();
}

///////////////////////////////////////////////////////////////////////////////
// Many of the collectionXYZ functions need to throw exceptions with common
// error messages (e.g. collectionInitSet() when called on an immutable collection).
// So we provide them with shared error-signaling logic.

/**
 * The different types of error messages.
 */
enum class ErrMsgType {
  CannotAssign,
  CannotUnset,
  CannotAdd,
  OnlyIntKeys,
};

///////////////////////////////////////////////////////////////////////////////

template <bool throwOnMiss>
static inline TypedValue* collectionAtImpl(ObjectData* obj,
                                           const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
    case Collection::ImmVectorType:
      return BaseVector::OffsetAt<throwOnMiss>(obj, key);
    case Collection::MapType:
    case Collection::ImmMapType:
      return BaseMap::OffsetAt<throwOnMiss>(obj, key);
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::PairType:
      return c_Pair::OffsetAt<throwOnMiss>(obj, key);
    case Collection::InvalidType:
      break;
  }
  assert(false);
  return nullptr;
}

// collectionAt() is used to get the address of an element for reading only.
// Throws an exception if the element is not present.
TypedValue* collectionAt(ObjectData* obj, const TypedValue* key) {
  return collectionAtImpl<true>(obj, key);
}

// collectionGet() is used to get the address of an element for reading
// only. Returns nullptr if the element is not present.
TypedValue* collectionGet(ObjectData* obj, TypedValue* key) {
  return collectionAtImpl<false>(obj, key);
}

// collectionAtLval() is used to get the address of an element when the
// caller is NOT going to do direct write per se, but it intends to use
// the element as the base of a member operation in an "lvalue" context
// (which could mutate the element in various ways).
TypedValue* collectionAtLval(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  TypedValue* ret;
  switch (obj->getCollectionType()) {
    case Collection::VectorType: {
      ret = BaseVector::OffsetAt<true>(obj, key);
      // We're about to expose an element of a Vector in an lvalue context;
      // if the element is a value-type (anything other than objects and
      // resources) we need to sever any buffer sharing that might be going on
      auto* vec = static_cast<c_Vector*>(obj);
      if (UNLIKELY(vec->hasImmutableBuffer() &&
                   ret->m_type != KindOfObject &&
                   ret->m_type != KindOfResource)) {
        vec->mutate();
        ret = BaseVector::OffsetAt<true>(obj, key);
      }
      return ret;
    }
    case Collection::ImmVectorType: {
      ret = BaseVector::OffsetAt<true>(obj, key);
      break;
    }
    case Collection::MapType:
      return BaseMap::OffsetAt<true>(obj, key);
    case Collection::ImmMapType: {
      ret = BaseMap::OffsetAt<true>(obj, key);
      break;
    }
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::PairType: {
      ret = c_Pair::OffsetAt<true>(obj, key);
      if (ret->m_type != KindOfObject && ret->m_type != KindOfResource) {
        warn_cannot_modify_immutable_object(obj->o_getClassName().data());
      }
      return ret;
    }
    case Collection::InvalidType:
      assert(false);
      break;
  }
  // Value-type elements (anything other than objects and resources) of
  // an immutable collection "inherit" the collection's immutable status.
  // We do not allow value-type elements of an immutable collection to
  // be read in an "lvalue" context in order to prevent null->array
  // promotion, null->stdClass promotion, and mutating strings or arrays
  // in place (see "test/slow/collection_classes/invalid-operations.php"
  // for examples).
  if (ret->m_type != KindOfObject && ret->m_type != KindOfResource) {
    throw_cannot_modify_immutable_object(obj->o_getClassName().data());
  }
  return ret;
}

// collectionAtRw() is used to get the address of an element for reading
// and writing. It is typically used for read-modify-write operations (the
// SetOp* and IncDec* instructions).
TypedValue* collectionAtRw(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      // Since we're exposing an element of a Vector in an read/write context,
      // we need to sever any buffer sharing that might be going on.
      static_cast<c_Vector*>(obj)->mutate();
      return BaseVector::OffsetAt<true>(obj, key);
    case Collection::MapType:
      return BaseMap::OffsetAt<true>(obj, key);
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::ImmVectorType:
    case Collection::ImmMapType:
      throw_cannot_modify_immutable_object(obj->o_getClassName().data());
      break;
    case Collection::PairType: {
      auto* ret = c_Pair::OffsetAt<true>(obj, key);
      warn_cannot_modify_immutable_object(obj->o_getClassName().data());
      return ret;
    }
    case Collection::InvalidType:
      break;
  }
  assert(false);
  return nullptr;
}

void collectionInitSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);

  assert(Collection::isMapType(obj->getCollectionType()));
  BaseMap::OffsetSet(obj, key, val);
}

void collectionSet(ObjectData* obj, const TypedValue* key, TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);

  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      c_Vector::OffsetSet(obj, key, val);
      break;
    case Collection::MapType:
      BaseMap::OffsetSet(obj, key, val);
      break;
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::ImmVectorType:
    case Collection::ImmMapType:
    case Collection::PairType:
      throw_cannot_modify_immutable_object(obj->o_getClassName().data());
      break;
    case Collection::InvalidType:
      assert(false);
  }
}

bool collectionIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
    case Collection::ImmVectorType:
      return BaseVector::OffsetIsset(obj, key);
    case Collection::MapType:
    case Collection::ImmMapType:
      return BaseMap::OffsetIsset(obj, key);
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::PairType:
      return c_Pair::OffsetIsset(obj, key);
    case Collection::InvalidType:
      assert(false);
      return false;
  }
  not_reached();
}

bool collectionEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
    case Collection::ImmVectorType:
      return BaseVector::OffsetEmpty(obj, key);
    case Collection::MapType:
    case Collection::ImmMapType:
      return BaseMap::OffsetEmpty(obj, key);
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::PairType:
      return c_Pair::OffsetEmpty(obj, key);
    case Collection::InvalidType:
      assert(false);
      return false;
  }
  not_reached();
}

void collectionUnset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      c_Vector::OffsetUnset(obj, key);
      break;
    case Collection::MapType:
      c_Map::OffsetUnset(obj, key);
      break;
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::ImmVectorType:
    case Collection::ImmMapType:
    case Collection::PairType:
      throw_cannot_modify_immutable_object(obj->o_getClassName().data());
      break;
    case Collection::InvalidType:
      assert(false);
      break;
  }
}

void collectionAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);

  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      static_cast<c_Vector*>(obj)->add(val);
      break;
    case Collection::MapType:
      static_cast<c_Map*>(obj)->add(val);
      break;
    case Collection::SetType:
      static_cast<c_Set*>(obj)->add(val);
      break;
    case Collection::ImmVectorType:
    case Collection::ImmMapType:
    case Collection::ImmSetType:
    case Collection::PairType:
      throw_cannot_modify_immutable_object(obj->o_getClassName().data());
      break;
    case Collection::InvalidType:
      assert(false);
      break;
  }
}

void collectionInitAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
    case Collection::ImmVectorType:
      static_cast<BaseVector*>(obj)->add(val);
      break;
    case Collection::SetType:
    case Collection::ImmSetType:
      static_cast<BaseSet*>(obj)->add(val);
      break;
    case Collection::PairType:
      static_cast<c_Pair*>(obj)->initAdd(val);
      break;
    case Collection::MapType:
    case Collection::ImmMapType:
    case Collection::InvalidType:
      assert(false);
      break;
  }
}

bool collectionContains(ObjectData* obj, const Variant& offset) {
  TypedValue* key = cvarToCell(&offset);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
    case Collection::ImmVectorType:
      return BaseVector::OffsetContains(obj, key);
    case Collection::MapType:
    case Collection::ImmMapType:
      return BaseMap::OffsetContains(obj, key);
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::PairType:
      return c_Pair::OffsetContains(obj, key);
    case Collection::InvalidType:
      assert(false);
      return false;
  }
  not_reached();
}

void collectionReserve(ObjectData* obj, int64_t sz) {
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
    case Collection::ImmVectorType:
      static_cast<BaseVector*>(obj)->reserve(sz);
      break;
    case Collection::MapType:
    case Collection::ImmMapType:
      static_cast<BaseMap*>(obj)->reserve(sz);
      break;
    case Collection::SetType:
    case Collection::ImmSetType:
      static_cast<BaseSet*>(obj)->reserve(sz);
      break;
    case Collection::PairType:
      // do nothing
      break;
    case Collection::InvalidType:
      assert(false);
      break;
  }
}

void collectionUnserialize(ObjectData* obj, VariableUnserializer* uns,
                           int64_t sz, char type) {
  assert(obj->isCollection());
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      c_Vector::Unserialize(obj, uns, sz, type);
      break;
    case Collection::MapType:
    case Collection::ImmMapType:
      BaseMap::Unserialize(obj, uns, sz, type);
      break;
    case Collection::SetType:
      c_Set::Unserialize(obj, uns, sz, type);
      break;
    case Collection::ImmVectorType:
      c_ImmVector::Unserialize(obj, uns, sz, type);
      break;
    case Collection::ImmSetType:
      c_ImmSet::Unserialize(obj, uns, sz, type);
      break;
    case Collection::PairType:
      c_Pair::Unserialize(obj, uns, sz, type);
      break;
    case Collection::InvalidType:
      assert(false);
      break;
  }
}

bool collectionEquals(const ObjectData* obj1, const ObjectData* obj2) {
  Collection::Type ct = obj1->getCollectionType();
  assert(!Collection::isInvalidType(ct));
  Collection::Type ct2 = obj2->getCollectionType();

  if (Collection::isMapType(ct) && Collection::isMapType(ct2)) {
    // For migration purposes, distinct Map types should compare equal
    return BaseMap::Equals(
      BaseMap::EqualityFlavor::OrderIrrelevant, obj1, obj2);
  }

  if (Collection::isVectorType(ct) && Collection::isVectorType(ct2)) {
    return BaseVector::Equals(obj1, obj2);
  }

  if (Collection::isSetType(ct) && Collection::isSetType(ct2)) {
    return BaseSet::Equals(obj1, obj2);
  }

  if (ct != ct2) { return false; }
  assert(ct == Collection::PairType);
  return c_Pair::Equals(obj1, obj2);
}

ObjectData* newCollectionHelper(uint32_t type, uint32_t size) {
  ObjectData* obj;
  switch (type) {
    case Collection::VectorType: obj = NEWOBJ(c_Vector)(); break;
    case Collection::MapType: obj = NEWOBJ(c_Map)(); break;
    case Collection::SetType: obj = NEWOBJ(c_Set)(); break;
    case Collection::PairType: obj = NEWOBJ(c_Pair)(); break;
    case Collection::ImmVectorType: obj = NEWOBJ(c_ImmVector)(); break;
    case Collection::ImmMapType: obj = NEWOBJ(c_ImmMap)(); break;
    case Collection::ImmSetType: obj = NEWOBJ(c_ImmSet)(); break;
    case Collection::InvalidType:
      obj = nullptr;
      raise_error("NewCol: Invalid collection type");
      break;
  }
  // Reserve enough room for nElms elements in advance
  if (size) {
    collectionReserve(obj, size);
  }
  return obj;
}

///////////////////////////////////////////////////////////////////////////////

}
