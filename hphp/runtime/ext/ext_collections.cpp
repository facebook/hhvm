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
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"

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
ALWAYS_INLINE
static Object materializeImpl(ObjectData* obj) {
  auto* col = NEWOBJ(TCollection)();
  Object o = col;
  col->init(VarNR(obj));
  return o;
}

static ALWAYS_INLINE
const Cell container_as_cell(const Variant& container) {
  const auto& cellContainer = *container.asCell();
  if (UNLIKELY(!isContainer(cellContainer))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a container (array or collection)"));
    throw e;
  }
  return cellContainer;
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
  vec->mutateImpl();
}

static inline bool
invokeAndCastToBool(const CallCtx& ctx, int argc,
                    const TypedValue* argv) {
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), ctx, argc, argv);
  return ret.toBoolean();
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
  TypedValue val, uint32_t index) {
  return std::array<TypedValue, 1>{{ val }};
}

ALWAYS_INLINE
static std::array<TypedValue, 2> makeArgsFromVectorKeyAndValue(
  TypedValue val, uint32_t index) {
  return std::array<TypedValue, 2> {{
    make_tv<KindOfInt64>(index),
    val
  }};
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_fromKeysOf(const Variant& container) {
  if (container.isNull()) { return NEWOBJ(TVector)(); }

  const auto& cellContainer = *container.asCell();
  if (UNLIKELY(!isContainer(cellContainer))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a container (array or collection)"));
    throw e;
  }

  ArrayIter iter(cellContainer);
  auto* target = NEWOBJ(TVector)();
  target->reserve(getContainerSize(cellContainer));
  assert(target->canMutateBuffer());
  Object ret = target;
  for (; iter; ++iter) { target->addRaw(iter.first()); }
  return ret;
}

template<class TVector, class MakeArgs>
ALWAYS_INLINE
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
  uint32_t sz = m_size;
  nv->reserve(sz);
  assert(nv->canMutateBuffer());
  int32_t version = m_version;
  for (uint32_t i = 0; i < sz; ++i) {
    TypedValue* tv = &nv->m_data[i];
    auto args = makeArgs(m_data[i], i);
    g_context->invokeFuncFew(tv, ctx, args.size(), &args[0]);
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    nv->incSize();
  }
  return nv;
}

template<class TVector, class MakeArgs>
ALWAYS_INLINE
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
  uint32_t sz = m_size;
  int32_t version = m_version;
  assert(nv->canMutateBuffer());
  for (uint32_t i = 0; i < sz; ++i) {
    auto args = makeArgs(m_data[i], i);
    bool b = invokeAndCastToBool(ctx, args.size(), &args[0]);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) {
      nv->addRaw(&m_data[i]);
    }
  }
  return nv;
}

template<class TVector>
ALWAYS_INLINE
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
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  for (size_t i = 0; i < sz; ++i) {
    cellDup(m_data[i], vec->m_data[i]);
  }
  return obj;
}

template<class TVector, bool checkVersion>
ALWAYS_INLINE
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
  assert(vec->m_size == 0);
  Object obj = vec;
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (uint32_t i = 0; i < m_size; ++i) {
    bool b = invokeAndCastToBool(ctx, 1, &m_data[i]);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
    vec->addRaw(&m_data[i]);
  }
  return obj;
}

template<class TVector>
ALWAYS_INLINE
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
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  for (size_t i = 0; i < sz; ++i) {
    cellDup(m_data[i + skipAmt], vec->m_data[i]);
  }
  return obj;
}

template<class TVector, bool checkVersion>
ALWAYS_INLINE
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
  assert(vec->canMutateBuffer());
  Object obj = vec;
  uint32_t i = 0;
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (; i < m_size; ++i) {
    bool b = invokeAndCastToBool(ctx, 1, &m_data[i]);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
  }
  for (; i < m_size; ++i) {
    vec->addRaw(&m_data[i]);
  }
  return obj;
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer"));
    throw e;
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer"));
    throw e;
  }
  size_t skipAmt = std::min<size_t>(istart, m_size);
  size_t sz = std::min<size_t>(ilen, size_t(m_size) - skipAmt);
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  vec->reserve(sz);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  auto* e = m_data + skipAmt;
  auto* eLimit = e + sz;
  auto* ne = vec->m_data;
  for (; e != eLimit; ++e, ++ne) {
    cellDup(*e, *ne);
  }
  return obj;
}

void BaseVector::zip(BaseVector* bvec, const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  uint32_t sz = m_size;
  bvec->reserve(std::min(itSize, size_t(sz)));
  assert(bvec->canMutateBuffer());
  for (uint32_t i = 0; i < sz && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (bvec->m_capacity <= bvec->m_size) {
      bvec->grow();
    }
    auto* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->initAdd(&m_data[i]);
    pair->initAdd(v);
    bvec->m_data[i].m_data.pobj = pair;
    bvec->m_data[i].m_type = KindOfObject;
    bvec->incSize();
  }
}

void BaseVector::keys(BaseVector* bvec) {
  assert(bvec->m_size == 0);
  bvec->reserve(m_size);
  assert(bvec->canMutateBuffer());
  bvec->setSize(m_size);
  for (uint32_t i = 0; i < m_size; ++i) {
    bvec->m_data[i].m_data.num = i;
    bvec->m_data[i].m_type = KindOfInt64;
  }
}

// Others

Object BaseVector::lazy() {
  return SystemLib::AllocLazyKeyedIterableViewObject(this);
}

Array BaseVector::toarray() {
  return toArrayImpl();
}

Array BaseVector::tokeysarray() {
  PackedArrayInit ai(m_size);
  uint32_t sz = m_size;
  for (uint32_t i = 0; i < sz; ++i) {
    ai.append((int64_t)i);
  }
  return ai.toArray();
}

Array BaseVector::tovaluesarray() {
  return toArrayImpl();
}

int64_t BaseVector::linearsearch(const Variant& search_value) {
  uint32_t sz = m_size;
  for (uint32_t i = 0; i < sz; ++i) {
    if (same(search_value, tvAsCVarRef(&m_data[i]))) {
      return i;
    }
  }
  return -1;
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

bool BaseVector::OffsetContains(ObjectData* obj, const TypedValue* key) {
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

  uint32_t sz = bv1->m_size;
  if (sz != bv2->m_size) {
    return false;
  }

  for (uint32_t i = 0; i < sz; ++i) {
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
  assert(bvec->canMutateBuffer());
  for (int64_t i = 0; i < sz; ++i) {
    auto tv = &bvec->m_data[bvec->m_size];
    tv->m_type = KindOfNull;
    bvec->incSize();
    tvAsVariant(tv).unserialize(uns, Uns::Mode::ColValue);
  }
}

// Helpers

Array BaseVector::toArrayImpl() const {
  if (!m_size) {
    return empty_array();
  }
  auto ad = const_cast<ArrayData*>(arrayData());
  // When Vector::toArray() returns a non-empty array, we want the returned
  // array to have its internal cursor pointing at the first element, so we
  // set m_pos accordingly here. It's safe to write to m_pos even if ad's
  // refcount is greater than 1 because we know one of the following is true:
  // either (1) this array has never been exposed outside of the Vector
  // implementation, in which case we know it's not referenced outside by
  // anything outside the Vector implementation, or (2) this array has been
  // exposed by the Vector implementation, in which case m_pos already points
  // to the first element (and COW semantics ensures that m_pos couldn't have
  // been changed by user code).
  assert(m_size);
  if (ad->m_pos != 0) {
    ad->m_pos = 0;
  }
  return Array(ad);
}

NEVER_INLINE
void BaseVector::grow() {
  if (m_capacity == MaxCapacity()) {
    return;
  }
  dropImmCopy();
  uint32_t newCap =
    m_capacity ? std::min(uint64_t(m_capacity) * 2, MaxCapacity()) : 8;
  reserveImpl(newCap);
}

void BaseVector::addFront(const TypedValue* val) {
  assert(val->m_type != KindOfRef);
  if (m_capacity <= m_size) {
    grow();
  } else {
    mutate();
  }
  assert(canMutateBuffer());
  ++m_version;
  memmove(m_data+1, m_data, m_size * sizeof(TypedValue));
  cellDup(*val, m_data[0]);
  incSize();
}

Variant BaseVector::popFront() {
  if (m_size) {
    mutateAndBump();
    // We're removing an element, so we make sure m_pos is not pointing at
    // garbage by setting it to invalid_index
    arrayData()->m_pos = ArrayData::invalid_index;
    Variant ret(tvAsCVarRef(&m_data[0]), Variant::CellCopy());
    decSize();
    memmove(m_data, m_data+1, m_size * sizeof(TypedValue));
    return ret;
  } else {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
      "Cannot pop empty Vector"));
    throw e;
  }
}

void BaseVector::reserveImpl(uint32_t newCap) {
  auto* oldBuf = m_data;
  auto* oldAd = arrayData();
  m_data = packedData(MixedArray::MakeReserve(newCap));
  m_capacity = packedCodeToCap(arrayData()->m_packedCapCode);
  arrayData()->m_size = m_size;
  if (LIKELY(!oldAd->hasMultipleRefs())) {
    std::memcpy(m_data, oldBuf, m_size * sizeof(TypedValue));
    // Mark oldAd as having 0 elements so that the array release logic doesn't
    // decRef the elements (since we teleported the elements to a new array)
    assert(oldAd != staticEmptyArray());
    assert(oldAd->isPacked());
    oldAd->m_size = 0;
    oldAd->m_pos = ArrayData::invalid_index;
    decRefArr(oldAd);
  } else {
    auto* dst = m_data;
    auto* src = oldBuf;
    auto* stop = src + m_size;
    for (; src != stop; ++src, ++dst) {
      cellDup(*src, *dst);
    }
    oldAd->decRefCount();
  }
}

void BaseVector::reserve(int64_t sz) {
  if (sz <= 0) return;
  if (m_capacity < sz) {
    dropImmCopy();
    ++m_version;
    reserveImpl(sz);
  }
}

BaseVector::BaseVector(Class* cls)
    : ExtCollectionObjectData(cls)
    , m_size(0), m_capacity(0), m_data(packedData(staticEmptyArray()))
    , m_version(0) {
}

/**
 * Delegate the responsibility for freeing the buffer to the immutable copy,
 * if it exists.
 */
BaseVector::~BaseVector() {
  decRefArr(arrayData());
}

NEVER_INLINE
void BaseVector::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
             "Only integer keys may be used with Vectors"));
  throw e;
}

void BaseVector::init(const Variant& t) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  if (sz) { reserve(sz); }

  if (LIKELY(!iter.hasIteratorObj())) {
    if (iter) {
      mutateAndBump();
      assert(canMutateBuffer());
      do {
        addRaw(iter.secondRefPlus());
        ++iter;
      } while (iter);
    }
  } else {
    for (; iter; ++iter) {
      auto v = iter.second();
      add(v.asCell());
    }
  }
}

void BaseVector::mutateImpl() {
  assert(arrayData()->hasMultipleRefs());
  dropImmCopy();
  if (canMutateBuffer()) {
    return;
  }
  assert(arrayData()->hasMultipleRefs());
  if (!m_size) {
    arrayData()->decRefCount();
    m_data = packedData(staticEmptyArray());
    m_capacity = 0;
    return;
  }
  auto* oldAd = arrayData();
  m_data = packedData(PackedArray::Copy(oldAd));
  arrayData()->incRefCount();
  assert(oldAd->hasMultipleRefs());
  oldAd->decRefCount();
}

///////////////////////////////////////////////////////////////////////////////

c_Vector::c_Vector(Class* cls /* = c_Vector::classof() */) : BaseVector(cls) {
  o_subclassData.u16 = Collection::VectorType;
}

void c_Vector::t___construct(const Variant& iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Object c_Vector::t_add(const Variant& val) {
  add(val.asCell());
  return this;
}

Object c_Vector::t_addall(const Variant& iterable) {
  // TODO Task# 4324040: Refactor this logic with init()
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  if (sz) {
    reserve(m_size + sz);
  }
  if (LIKELY(!iter.hasIteratorObj())) {
    if (iter) {
      mutateAndBump();
      assert(canMutateBuffer());
      do {
        addRaw(iter.secondRefPlus());
        ++iter;
      } while (iter);
    }
  } else {
    for (; iter; ++iter) {
      auto v = iter.second();
      add(v.asCell());
    }
  }
  return this;
}

Object c_Vector::t_addallkeysof(const Variant& container) {
  if (container.isNull()) {
    return this;
  }

  const auto& containerCell = container_as_cell(container);

  auto sz = getContainerSize(containerCell);
  ArrayIter iter(containerCell);
  if (!sz || !iter) {
    return this;
  }
  reserve(m_size + sz);

  mutateAndBump();
  assert(canMutateBuffer());
  do {
    addRaw(iter.first());
    ++iter;
  } while (iter);

  return this;
}

Object c_Vector::t_append(const Variant& val) {
  add(val.asCell());
  return this;
}

Variant c_Vector::t_pop() {
  if (m_size) {
    mutateAndBump();
    // We're removing an element, so we make sure m_pos is not pointing at
    // garbage by setting it to invalid_index
    arrayData()->m_pos = ArrayData::invalid_index;
    decSize();
    return Variant(tvAsCVarRef(&m_data[m_size]), Variant::CellCopy());
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
  const auto* val = value.asCell();
  assert(intSz >= 0);
  if (intSz == m_size) {
    return;
  }
  if (intSz > (int64_t)m_capacity) {
    reserve(intSz);
  } else {
    mutate();
  }
  assert(canMutateBuffer());
  ++m_version;
  uint32_t requestedSize = (uint32_t)intSz;
  if (m_size > requestedSize) {
    // We're removing elements, so we make sure m_pos is not pointing at
    // garbage by setting it to invalid_index
    arrayData()->m_pos = ArrayData::invalid_index;
    do {
      decSize();
      tvRefcountedDecRef(&m_data[m_size]);
    } while (m_size > requestedSize);
  } else {
    for (; m_size < requestedSize; incSize()) {
      cellDup(*val, m_data[m_size]);
    }
  }
}

void c_Vector::t_reserve(const Variant& sz) {
  auto intSz = checkRequestedCapacity(sz);
  reserve(intSz);
}

Object c_Vector::t_clear() {
  ++m_version;
  dropImmCopy();
  decRefArr(arrayData());
  m_data = packedData(staticEmptyArray());
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

Variant c_Vector::t_at(const Variant& key) {
  return BaseVector::at(key);
}

Variant c_Vector::t_get(const Variant& key) {
  return Variant(get(key), Variant::CellDup());
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
  mutateAndBump();
  // We're removing an element, so we make sure m_pos is not pointing at
  // garbage by setting it to invalid_index
  arrayData()->m_pos = ArrayData::invalid_index;
  uint64_t datum = m_data[k].m_data.num;
  DataType t = m_data[k].m_type;
  if (k+1 < m_size) {
    memmove(&m_data[k], &m_data[k+1],
            (m_size-(k+1)) * sizeof(TypedValue));
  }
  decSize();
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
  mutateAndBump();
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
  mutateAndBump();
  // We're removing elements, so we make sure m_pos is not pointing at
  // garbage by setting it to invalid_index
  arrayData()->m_pos = ArrayData::invalid_index;
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
  setSize(m_size - (endPos - startPos));
}

int64_t c_Vector::t_linearsearch(const Variant& search_value) {
  return BaseVector::linearsearch(search_value);
}

void c_Vector::t_shuffle() {
  if (m_size <= 1) {
    return;
  }
  mutateAndBump();
  for (uint32_t i = 1; i < m_size; ++i) {
    uint32_t j = f_mt_rand(0, i);
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

Object c_Vector::t_slice(const Variant& start, const Variant& len) {
  return BaseVector::php_slice<c_Vector>(start, len);
}

Object c_ImmVector::t_slice(const Variant& start, const Variant& len) {
  return BaseVector::php_slice<c_ImmVector>(start, len);
}

Object c_Vector::t_concat(const Variant& iterable) {
  return BaseVector::php_concat<c_Vector>(iterable);
}

Object c_ImmVector::t_concat(const Variant& iterable) {
  return BaseVector::php_concat<c_ImmVector>(iterable);
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_concat(const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  uint32_t sz = m_size;
  vec->reserve((size_t)sz + itSize);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  for (uint32_t i = 0; i < sz; ++i) {
    cellDup(m_data[i], vec->m_data[i]);
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return obj;
}

Variant c_Vector::t_firstvalue() {
  return BaseVector::php_firstValue();
}

Variant c_ImmVector::t_firstvalue() {
  return BaseVector::php_firstValue();
}

Variant BaseVector::php_firstValue() {
  if (!m_size) return init_null();
  return tvAsCVarRef(&m_data[0]);
}

Variant c_Vector::t_firstkey() {
  return BaseVector::php_firstKey();
}

Variant c_ImmVector::t_firstkey() {
  return BaseVector::php_firstKey();
}

Variant BaseVector::php_firstKey() {
  if (!m_size) return init_null();
  return 0;
}

Variant c_Vector::t_lastvalue() {
  return BaseVector::php_lastValue();
}

Variant c_ImmVector::t_lastvalue() {
  return BaseVector::php_lastValue();
}

Variant BaseVector::php_lastValue() {
  if (!m_size) return init_null();
  return tvAsCVarRef(&m_data[m_size - 1]);
}

Variant c_Vector::t_lastkey() {
  return BaseVector::php_lastKey();
}

Variant c_ImmVector::t_lastkey() {
  return BaseVector::php_lastKey();
}

Variant BaseVector::php_lastKey() {
  if (!m_size) return init_null();
  return (int64_t)m_size - 1;
}

Object c_Vector::t_set(const Variant& key, const Variant& value) {
  set(key, value);
  return this;
}

Object c_Vector::t_setall(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    set(iter.first(), iter.second());
  }
  return this;
}

Object c_Vector::ti_fromitems(const Variant& iterable) {
  if (iterable.isNull()) return NEWOBJ(c_Vector)();
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto* target = NEWOBJ(c_Vector)();
  Object ret = target;
  for (uint32_t i = 0; iter; ++i, ++iter) {
    target->addRaw(iter.second());
  }
  return ret;
}

Object c_Vector::ti_fromkeysof(const Variant& container) {
  return BaseVector::php_fromKeysOf<c_Vector>(container);
}

Object c_ImmVector::ti_fromkeysof(const Variant& container) {
  return BaseVector::php_fromKeysOf<c_ImmVector>(container);
}

Object c_Vector::ti_fromarray(const Variant& arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  auto* target = NEWOBJ(c_Vector)();
  Object ret = target;
  auto* ad = arr.getArrayData();
  uint32_t sz = ad->size();
  target->reserve(sz);
  assert(target->canMutateBuffer());
  target->setSize(sz);
  auto* data = target->m_data;
  ssize_t pos = ad->iter_begin();
  for (uint32_t i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    assert(pos != ArrayData::invalid_index);
    cellDup(*(ad->getValueRef(pos).asCell()), data[i]);
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
  uint32_t sz = m_size;
  bool allInts = true;
  bool allStrs = true;
  for (uint32_t i = 0; i < sz; ++i) {
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
  if (m_size <= 1) {
    return;
  }
  mutateAndBump();
  SortFlavor flav = preSort<VectorValAccessor>(VectorValAccessor());
  CALL_SORT(VectorValAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

bool c_Vector::usort(const Variant& cmp_function) {
  if (m_size <= 1) {
    return true;
  }
  mutateAndBump();
  ElmUCompare<VectorValAccessor> comp;
  CallCtx ctx;
  CallerFrame cf;
  vm_decode_function(cmp_function, cf(), false, ctx);
  if (!ctx.func) {
    return false;
  }
  comp.ctx = &ctx;
  Sort::sort(m_data, m_data + m_size, comp);
  return true;
}

void c_Vector::OffsetSet(ObjectData* obj, const TypedValue* key,
                         const TypedValue* val) {
  static_cast<c_Vector*>(obj)->set(key, val);
}

void c_Vector::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot unset an element of a Vector"));
  throw e;
}

// This function will create a immutable copy of this Vector (if it doesn't
// already exist) and then return it
Object c_Vector::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto* vec = NEWOBJ(c_ImmVector)();
    m_immCopy = vec;
    arrayData()->incRefCount();
    vec->m_data = m_data;
    vec->m_size = m_size;
    vec->m_capacity = m_capacity;
    vec->m_version = m_version;
  }
  assert(!m_immCopy.isNull());
  assert(m_data == static_cast<c_ImmVector*>(m_immCopy.get())->m_data);
  assert(arrayData()->hasMultipleRefs());
  return m_immCopy;
}

Object c_Vector::t_tovector() { return Object::attach(c_Vector::Clone(this)); }
Object c_Vector::t_toimmvector() { return getImmutableCopy(); }
Object c_Vector::t_tomap() { return materializeImpl<c_Map>(this); }
Object c_Vector::t_toimmmap() { return materializeImpl<c_ImmMap>(this); }
Object c_Vector::t_toset() { return materializeImpl<c_Set>(this); }
Object c_Vector::t_toimmset() { return materializeImpl<c_ImmSet>(this); }
Object c_Vector::t_immutable() { return getImmutableCopy(); }

Object c_ImmVector::t_tovector() { return materializeImpl<c_Vector>(this); }
Object c_ImmVector::t_toimmvector() { return this; }
Object c_ImmVector::t_tomap() { return materializeImpl<c_Map>(this); }
Object c_ImmVector::t_toimmmap() { return materializeImpl<c_ImmMap>(this); }
Object c_ImmVector::t_toset() { return materializeImpl<c_Set>(this); }
Object c_ImmVector::t_toimmset() { return materializeImpl<c_ImmSet>(this); }
Object c_ImmVector::t_immutable() { return this; }

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
  return Variant(get(key), Variant::CellDup());
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

Object c_ImmVector::t_keys() {
  auto* vec = NEWOBJ(c_ImmVector);
  Object obj = vec;
  BaseVector::keys(vec);
  return obj;
}

// Others

void c_ImmVector::t___construct(const Variant& iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
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

// Non PHP methods.

c_ImmVector::c_ImmVector(Class* cls) : BaseVector(cls) {
  o_subclassData.u16 = Collection::ImmVectorType;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The Map implementation makes use of s_theEmptyMixedArray, a special static
 * empty array that has all of MixedArray's fields initialized to convenient
 * values. This "static empty mixed array" is only used internally within the
 * Map implementation and it's never exposed outside of the Map implementation.
 * hashTab()[0] is set to Empty so that the find() method won't find anything,
 * and m_cap is set to 0 so that any attempt to add an element will trigger a
 * grow operation.
 *
 * Using this static empty mixed array allows us to always assume m_data is
 * non-null, and it is better than calling MakeReserveMixed because it avoids
 * doing any allocation.
 */

std::aligned_storage<
  sizeof(MixedArray) + sizeof(int32_t),
  alignof(MixedArray)
>::type s_theEmptyMixedArray;

struct BaseMap::EmptyMixedInitializer {
  EmptyMixedInitializer() {
    void* vpEmpty = &s_theEmptyMixedArray;

    auto const ad   = static_cast<MixedArray*>(vpEmpty);
    ad->m_kind      = ArrayData::kEmptyKind;
    ad->m_size      = 0;
    ad->m_pos       = ArrayData::invalid_index;
    ad->m_count     = 0;
    ad->m_used      = 0;
    ad->m_cap       = 0;
    ad->m_tableMask = 0;
    ad->m_nextKI    = 0;
    ad->hashTab()[0] = Empty;

    ad->setStatic();
  }
};
BaseMap::EmptyMixedInitializer BaseMap::s_empty_mixed_initializer;

///////////////////////////////////////////////////////////////////////////////

c_Map::c_Map(Class* cls) : BaseMap(cls) {
  o_subclassData.u16 = Collection::MapType;
}

// Protected (Internal)

BaseMap::BaseMap(Class* cls)
    : ExtCollectionObjectData(cls)
    , m_size(0), m_version(0), m_data(mixedData(staticEmptyMixedArray())) {
}

BaseMap::~BaseMap() {
  decRefArr(arrayData());
}

void c_Map::t___construct(const Variant& iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Array BaseMap::php_toArray() const {
  if (!m_size) {
    return empty_array();
  }
  if (UNLIKELY(intLikeStrKeys())) {
    // In the rare case where this Map contains integer-like string keys,
    // instead of exposing this Map's buffer we return a MixedArray copy of
    // the buffer with the int-like string keys converted to integers. This
    // is important because int-like string keys cannot be accessed by user
    // code via the brackets operator (i.e. "$c[$k]").
    ArrayInit ai(m_size, ArrayInit::Mixed{});
    auto* eLimit = elmLimit();
    for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
      if (e->hasIntKey()) {
        ai.set((int64_t)e->ikey, tvAsCVarRef(&e->data));
      } else {
        ai.set(String(StrNR(e->skey)).toKey(), tvAsCVarRef(&e->data));
      }
    }
    return ai.toArray();
  }
  auto ad = const_cast<ArrayData*>(
    reinterpret_cast<const ArrayData*>(arrayData())
  );
  // When Map:toArray() returns a non-empty array, we want the returned
  // array to have its internal cursor pointing at the first element, so we
  // set m_pos accordingly here. It's safe to write to m_pos even if ad's
  // refcount is greater than 1 because we know one of the following is true:
  // either (1) this array has never been exposed outside of the Map
  // implementation, in which case we know it's not referenced outside by
  // anything outside the Map implementation, or (2) this array has been
  // exposed by the Map implementation, in which case m_pos was already set
  // to point to the first element (and COW semantics ensures that m_pos
  // couldn't have been changed by user code).
  assert(m_size);
  auto firstPos = nthElmPos(0);
  if (ad->m_pos != firstPos) {
    ad->m_pos = firstPos;
  }
  return Array(ad);
}

template<typename TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, TMap*>::type
BaseMap::Clone(ObjectData* obj) {
  auto thiz = static_cast<TMap*>(obj);
  auto target = static_cast<TMap*>(obj->cloneImpl());
  if (!thiz->m_size) {
    return target;
  }
  thiz->arrayData()->incRefCount();
  target->m_size = thiz->m_size;
  target->m_data = thiz->m_data;
  target->setIntLikeStrKeys(thiz->intLikeStrKeys());
  return target;
}

c_Map* c_Map::Clone(ObjectData* obj) {
  return BaseMap::Clone<c_Map>(obj);
}

void BaseMap::mutateImpl() {
  assert(arrayData()->hasMultipleRefs());
  dropImmCopy();
  if (canMutateBuffer()) {
    return;
  }
  if (!m_size) {
    arrayData()->decRefCount();
    m_size = 0;
    m_data = mixedData(staticEmptyMixedArray());
    setIntLikeStrKeys(false);
    return;
  }
  auto* oldAd = arrayData();
  m_data = mixedData(
    reinterpret_cast<MixedArray*>(MixedArray::Copy(oldAd))
  );
  arrayData()->incRefCount();
  assert(oldAd->hasMultipleRefs());
  oldAd->decRefCount();
}

void BaseMap::init(const Variant& t) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  if (sz) {
    reserve(sz);
  }
  if (LIKELY(!iter.hasIteratorObj() && !m_size)) {
    if (iter) {
      mutateAndBump();
      do {
        setRaw(iter.first(), iter.secondRefPlus());
        ++iter;
      } while (iter);
    }
  } else {
    for (; iter; ++iter) {
      set(iter.first(), iter.second());
    }
  }
}

Object c_Map::t_add(const Variant& val) {
  add(val);
  return this;
}

Object c_Map::t_addall(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto oldCap = cap();
  reserve(m_size + sz); // presume minimum key collisions
  for (; iter; ++iter) {
    add(iter.second());
  }
  adjustAfterReserve(oldCap); // ... and shrink back if that was incorrect
  return this;
}

void c_Map::t_reserve(const Variant& sz) {
  if (UNLIKELY(!sz.isInteger())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter sz must be a non-negative integer"));
    throw e;
  }
  int64_t intSz = sz.toInt64();
  if (UNLIKELY(intSz < 0)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter sz must be a non-negative integer"));
    throw e;
  }
  reserve(intSz); // checks for intSz > MaxReserveSize
}

Object c_Map::t_clear() {
  ++m_version;
  dropImmCopy();
  decRefArr(arrayData());
  m_data = mixedData(staticEmptyMixedArray());
  m_size = 0;
  setIntLikeStrKeys(false);
  return this;
}

bool c_ImmMap::t_isempty() { return php_isEmpty(); }

bool c_Map::t_isempty() { return php_isEmpty(); }

int64_t c_ImmMap::t_count() { return size(); }

int64_t c_Map::t_count() { return size(); }

Object c_ImmMap::t_items() { return php_items(); }

Object c_Map::t_items() { return php_items(); }

Object BaseMap::php_keys() const {
  auto* vec = NEWOBJ(c_Vector)();
  Object obj = vec;
  vec->reserve(m_size);
  assert(vec->canMutateBuffer());
  auto* e = firstElm();
  auto* eLimit = elmLimit();
  ssize_t j = 0;
  for (; e != eLimit; e = nextElm(e, eLimit), vec->incSize(), ++j) {
    if (e->hasIntKey()) {
      vec->m_data[j].m_data.num = e->ikey;
      vec->m_data[j].m_type = KindOfInt64;
    } else {
      e->skey->incRefCount();
      vec->m_data[j].m_data.pstr = e->skey;
      vec->m_data[j].m_type = KindOfString;
    }
  }
  return obj;
}

Object c_ImmMap::t_keys() { return php_keys(); }

Object c_Map::t_keys() { return php_keys(); }

Object c_ImmMap::t_lazy() { return php_lazy(); }

Object c_Map::t_lazy() { return php_lazy(); }

Variant BaseMap::php_at(const Variant& key) const {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  } else if (key.isString()) {
    return tvAsCVarRef(at(key.getStringData()));
  }
  throwBadKeyType();
  return init_null();
}

Variant c_ImmMap::t_at(const Variant& key) { return php_at(key); }

Variant c_Map::t_at(const Variant& key) { return php_at(key); }

Variant BaseMap::php_get(const Variant& key) const {
  if (key.isInteger()) {
    TypedValue* tv = get(key.toInt64());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return init_null();
    }
  } else if (key.isString()) {
    TypedValue* tv = get(key.getStringData());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return init_null();
    }
  }
  throwBadKeyType();
  return init_null();
}

Variant c_ImmMap::t_get(const Variant& key) { return php_get(key); }

Variant c_Map::t_get(const Variant& key) { return php_get(key); }

Object c_Map::t_set(const Variant& key, const Variant& value) {
  set(key, value);
  return this;
}

Object c_Map::t_setall(const Variant& iterable) {
  // TODO Task# 4324040: Refactor this logic with init()
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    set(iter.first(), iter.second());
  }
  return this;
}

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

Object c_Map::t_remove(const Variant& key) {
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

Object c_Map::t_removekey(const Variant& key) { return t_remove(key); }

Array c_ImmMap::t_toarray() { return php_toArray(); }

Array c_Map::t_toarray() { return php_toArray(); }

Object BaseMap::php_values() const {
  auto* target = NEWOBJ(c_Vector)();
  Object ret = target;
  int64_t sz = m_size;
  target->reserve(sz);
  assert(target->canMutateBuffer());
  target->setSize(sz);
  auto* out = target->m_data;
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit), ++out) {
    cellDup(e->data, *out);
  }
  return ret;
}

Object c_ImmMap::t_values() { return php_values(); }

Object c_Map::t_values() { return php_values(); }

Array BaseMap::php_toKeysArray() const {
  PackedArrayInit ai(m_size);
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasIntKey()) {
      ai.append(int64_t{e->ikey});
    } else {
      ai.append(VarNR(e->skey));
    }
  }
  return ai.toArray();
}

Array c_ImmMap::t_tokeysarray() { return php_toKeysArray(); }

Array c_Map::t_tokeysarray() { return php_toKeysArray(); }

Array BaseMap::php_toValuesArray() const {
  PackedArrayInit ai(m_size);
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    ai.append(tvAsCVarRef(&e->data));
  }
  return ai.toArray();
}

Array c_ImmMap::t_tovaluesarray() { return php_toValuesArray(); }

Array c_Map::t_tovaluesarray() { return php_toValuesArray(); }

template<typename TMap>
ALWAYS_INLINE
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
    auto* eLimit = mp->elmLimit();
    for (auto* e = mp->firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
      if (e->hasIntKey()) {
        target->remove((int64_t)e->ikey);
      } else {
        target->remove(e->skey);
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
  const BaseMap::Elm& e) {
  return std::array<TypedValue, 2> {{
    (e.hasIntKey() ? make_tv<KindOfInt64>(e.ikey)
     : make_tv<KindOfString>(e.skey)),
    e.data
  }};
}

ALWAYS_INLINE
static std::array<TypedValue, 1> makeArgsFromMapValue(const BaseMap::Elm& e) {
  // note that this is a potentially unnecessary copy
  // that might be reinterpret_cast ed away
  // http://stackoverflow.com/questions/11205186/treat-c-cstyle-array-as-stdarray
  return std::array<TypedValue, 1> {{ e.data }};
}

template<typename TMap, class MakeArgs>
ALWAYS_INLINE
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
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  if (!m_size) return obj;
  assert(posLimit() != 0);
  assert(hashSize() > 0);
  assert(mp->arrayData() == staticEmptyMixedArray());
  mp->m_data = mixedData(
    reinterpret_cast<MixedArray*>(MixedArray::MakeReserveMixed(cap()))
  );
  mp->setIntLikeStrKeys(intLikeStrKeys());
  wordcpy(mp->hashTab(), hashTab(), hashSize());
  uint32_t used = posLimit();
  int32_t version = m_version;
  for (uint32_t i = 0; i < used; ++i, mp->incPosLimit()) {
    const Elm& e = data()[i];
    Elm& ne = mp->data()[i];
    if (isTombstone(i)) {
      ne.data.m_type = e.data.m_type;
      continue;
    }
    TypedValue* tv = &ne.data;
    auto args = makeArgs(e);
    g_context->invokeFuncFew(tv, ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    if (e.hasStrKey()) {
      e.skey->incRefCount();
    }
    ne.ikey = e.ikey;
    ne.data.hash() = e.data.hash();
    mp->incSize();
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
ALWAYS_INLINE
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
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  if (!m_size) return obj;

  int32_t version = m_version;
  for (ssize_t ipos = iter_begin(); iter_valid(ipos); ipos = iter_next(ipos)) {
    auto* e = iter_elm(ipos);
    auto args = makeArgs(*e);
    bool b = invokeAndCastToBool(ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!b) continue;
    e = iter_elm(ipos);
    if (e->hasIntKey()) {
      mp->setRaw(e->ikey, &e->data);
    } else {
      mp->setRaw(e->skey, &e->data);
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
  auto size = m_size;
  if (!size) { return this; }
  for (ssize_t ipos = iter_begin(); iter_valid(ipos); ipos = iter_next(ipos)) {
    auto* e = iter_elm(ipos);
    auto args = makeArgs(*e);
    int32_t version = m_version;
    bool b = invokeAndCastToBool(ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) {
      continue;
    }
    mutateAndBump();
    version = m_version;
    e = iter_elm(ipos);
    ssize_t pp = (e->hasIntKey()
                   ? findForRemove(e->ikey)
                   : findForRemove(e->skey, e->skey->hash()));
    eraseNoCompact(pp);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
  }

  assert(m_size <= size);
  adjustIfDensityTooLow();
  return this;
}

Object c_Map::t_retain(const Variant& callback) {
  return php_retain(callback, &makeArgsFromMapValue);
}

Object c_Map::t_retainwithkey(const Variant& callback) {
  return php_retain(callback, &makeArgsFromMapKeyAndValue);
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_zip(const Variant& iterable) const {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  if (!m_size) {
    return obj;
  }
  mp->reserve(std::min(sz, size_t(m_size)));
  uint used = posLimit();
  for (uint i = 0; i < used && iter; ++i) {
    if (isTombstone(i)) continue;
    const Elm& e = data()[i];
    Variant v = iter.second();
    auto* pair = NEWOBJ(c_Pair)();
    Object pairObj = pair;
    pair->initAdd(&e.data);
    pair->initAdd(v);
    TypedValue tv;
    tv.m_data.pobj = pair;
    tv.m_type = KindOfObject;
    if (e.hasIntKey()) {
      mp->setRaw(e.ikey, &tv);
    } else {
      mp->setRaw(e.skey, &tv);
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
ALWAYS_INLINE
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
  mp->setSize(sz);
  mp->setPosLimit(sz);
  auto table = mp->hashTab();
  auto mask = mp->tableMask();
  for (uint32_t frPos = 0, toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesUnsafe(frPos);
    auto& toE = mp->m_data[toPos];
    dupElm(m_data[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      mp->updateNextKI(toE.ikey);
    } else {
      mp->updateIntLikeStrKeys(toE.skey);
    }
  }
  return obj;
}

template<class TMap, bool checkVersion>
ALWAYS_INLINE
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
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (ssize_t ipos = iter_begin(); iter_valid(ipos); ipos = iter_next(ipos)) {
    auto* e = iter_elm(ipos);
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) continue;
    e = iter_elm(ipos);
    if (e->hasIntKey()) {
      mp->setRaw(e->ikey, &e->data);
    } else {
      mp->setRaw(e->skey, &e->data);
    }
  }
  return obj;
}

template<class TMap>
ALWAYS_INLINE
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
  mp->setSize(sz);
  mp->setPosLimit(sz);
  uint32_t frPos = nthElmPos(len);
  auto table = mp->hashTab();
  auto mask = mp->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesUnsafe(frPos);
    auto& toE = mp->m_data[toPos];
    dupElm(m_data[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      mp->updateNextKI(toE.ikey);
    } else {
      mp->updateIntLikeStrKeys(toE.skey);
    }
  }
  return obj;
}

template<class TMap, bool checkVersion>
ALWAYS_INLINE
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
  int32_t version;
  if (checkVersion) {
    version = m_version;
  }
  ssize_t ipos;
  for (ipos = iter_begin(); iter_valid(ipos); ipos = iter_next(ipos)) {
    auto* e = iter_elm(ipos);
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
  }
  auto* eLimit = elmLimit();
  auto* e = iter_elm(ipos);
  for (; e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasIntKey()) {
      mp->setRaw(e->ikey, &e->data);
    } else {
      mp->setRaw(e->skey, &e->data);
    }
  }
  return obj;
}

template<class TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer"));
    throw e;
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer"));
    throw e;
  }
  size_t skipAmt = std::min<size_t>(istart, m_size);
  size_t sz = std::min<size_t>(ilen, size_t(m_size) - skipAmt);
  auto* mp = NEWOBJ(TMap)();
  Object obj = mp;
  mp->reserve(sz);
  mp->setSize(sz);
  mp->setPosLimit(sz);
  uint32_t frPos = nthElmPos(skipAmt);
  auto table = mp->hashTab();
  auto mask = mp->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesUnsafe(frPos);
    auto& toE = mp->m_data[toPos];
    dupElm(m_data[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      mp->updateNextKI(toE.ikey);
    } else {
      mp->updateIntLikeStrKeys(toE.skey);
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

Object c_Map::t_slice(const Variant& start, const Variant& len) {
  return BaseMap::php_slice<c_Map>(start, len);
}

Object c_ImmMap::t_slice(const Variant& start, const Variant& len) {
  return BaseMap::php_slice<c_ImmMap>(start, len);
}

Object c_Map::t_concat(const Variant& iterable) {
  return BaseMap::php_concat<c_Vector>(iterable);
}

Object c_ImmMap::t_concat(const Variant& iterable) {
  return BaseMap::php_concat<c_ImmVector>(iterable);
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseMap::php_concat(const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  uint sz = m_size;
  vec->reserve((size_t)sz + itSize);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  uint32_t used = posLimit();
  for (uint32_t i = 0, j = 0; i < used; ++i) {
    if (isTombstone(i)) {
      continue;
    }
    cellDup(data()[i].data, vec->m_data[j]);
    ++j;
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return obj;
}

Variant c_Map::t_firstvalue() {
  return BaseMap::php_firstValue();
}

Variant c_ImmMap::t_firstvalue() {
  return BaseMap::php_firstValue();
}

Variant BaseMap::php_firstValue() {
  if (!m_size) return init_null();
  auto* e = firstElm();
  assert(e != elmLimit());
  return tvAsCVarRef(&e->data);
}

Variant c_Map::t_firstkey() {
  return BaseMap::php_firstKey();
}

Variant c_ImmMap::t_firstkey() {
  return BaseMap::php_firstKey();
}

Variant BaseMap::php_firstKey() {
  if (!m_size) return init_null();
  auto* e = firstElm();
  assert(e != elmLimit());
  if (e->hasIntKey()) {
    return e->ikey;
  } else {
    return e->skey;
  }
}

Variant c_Map::t_lastvalue() {
  return BaseMap::php_lastValue();
}

Variant c_ImmMap::t_lastvalue() {
  return BaseMap::php_lastValue();
}

Variant BaseMap::php_lastValue() {
  if (!m_size) return init_null();
  // TODO Task# 4281431: If nthElmPos(n) is optimized to
  // walk backward from the end when n > m_size/2, then
  // we could use that here instead of having to use a
  // manual while loop.
  uint32_t pos = posLimit() - 1;
  while (isTombstone(pos)) {
    assert(pos > 0);
    --pos;
  }
  return tvAsCVarRef(&m_data[pos].data);
}

Variant c_Map::t_lastkey() {
  return BaseMap::php_lastKey();
}

Variant c_ImmMap::t_lastkey() {
  return BaseMap::php_lastKey();
}

Variant BaseMap::php_lastKey() {
  if (!m_size) return init_null();
  // TODO Task# 4281431: If nthElmPos(n) is optimized to
  // walk backward from the end when n > m_size/2, then
  // we could use that here instead of having to use a
  // manual while loop.
  uint32_t pos = posLimit() - 1;
  while (isTombstone(pos)) {
    assert(pos > 0);
    --pos;
  }
  if (m_data[pos].hasIntKey()) {
    return m_data[pos].ikey;
  } else {
    return m_data[pos].skey;
  }
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_mapFromItems(const Variant& iterable) {
  if (iterable.isNull()) return NEWOBJ(TMap)();
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto* target = NEWOBJ(TMap)();
  Object ret = target;
  target->reserve(sz);
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
    target->setRaw(&pair->elm0, &pair->elm1);
  }
  return ret;
}

Object c_ImmMap::ti_fromitems(const Variant& iterable) {
  return php_mapFromItems<c_ImmMap>(iterable);
}

Object c_Map::ti_fromitems(const Variant& iterable) {
  return php_mapFromItems<c_Map>(iterable);
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_mapFromArray(const Variant& arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  auto* mp = NEWOBJ(TMap)();
  Object ret = mp;
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    auto* tv = ad->getValueRef(pos).asCell();
    if (k.isInteger()) {
      mp->setRaw(k.toInt64(), tv);
    } else {
      assert(k.isString());
      mp->setRaw(k.getStringData(), tv);
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
  mp->mutate();
  uint used = mp->posLimit();
  for (uint32_t i = 0; i < used; ++i) {
    if (mp->isTombstone(i)) continue;
    auto* e = &mp->data()[i];
    collectionDeepCopyTV(&e->data);
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
  return (TypedValue*)&fetchElm(data(), p)->data;
}

TypedValue* BaseMap::at(StringData* key) const {
  auto p = find(key, key->hash());
  if (UNLIKELY(p == Empty)) {
    throwOOB(key);
  }
  return (TypedValue*)&fetchElm(data(), p)->data;
}

TypedValue* BaseMap::get(int64_t key) const {
  auto p = find(key);
  if (p == Empty) {
    return nullptr;
  }
  return (TypedValue*)&fetchElm(data(), p)->data;
}

TypedValue* BaseMap::get(StringData* key) const {
  auto p = find(key, key->hash());
  if (p == Empty) {
    return nullptr;
  }
  return (TypedValue*)&fetchElm(data(), p)->data;
}

void BaseMap::add(const TypedValue* val) {
  if (UNLIKELY(val->m_type != KindOfObject ||
               val->m_data.pobj->getVMClass() != c_Pair::classof())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be an instance of Pair"));
    throw e;
  }
  auto pair = static_cast<c_Pair*>(val->m_data.pobj);
  set(&pair->elm0, &pair->elm1);
}

Variant BaseMap::pop() {
  if (m_size) {
    mutateAndBump();
    auto* e = elmLimit() - 1;
    for (;; --e) {
      assert(e >= data());
      if (!isTombstone(e)) break;
    }
    Variant ret = tvAsCVarRef(&e->data);
    ssize_t ei;
    if (e->hasIntKey()) {
      ei = findForRemove(e->ikey);
    } else {
      assert(e->hasStrKey());
      ei = findForRemove(e->skey, e->skey->hash());
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
    mutateAndBump();
    auto* e = data();
    for (;; ++e) {
      assert(e != elmLimit());
      if (!isTombstone(e)) break;
    }
    Variant ret = tvAsCVarRef(&e->data);
    ssize_t ei;
    if (e->hasIntKey()) {
      ei = findForRemove(e->ikey);
    } else {
      assert(e->hasStrKey());
      ei = findForRemove(e->skey, e->skey->hash());
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
  mutateAndBump();
  auto p = findForRemove(key);
  if (validPos(p)) {
    erase(p);
  }
}

void BaseMap::remove(StringData* key) {
  mutateAndBump();
  auto p = findForRemove(key, key->hash());
  if (validPos(p)) {
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
  assert(!BaseMap::isTombstone(&e));
  return hash == e.hash() && (s == e.skey || s->same(e.skey));
}

static bool hitIntKey(const BaseMap::Elm& e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. c_Map guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!BaseMap::isTombstone(&e));
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
  size_t mask = tableMask();
  auto* elms = data();
  auto* hashtable = hashTab();
  for (size_t probeIndex = h0, i = 1;; ++i) {
    ssize_t pos = hashtable[probeIndex & mask];
    if ((validPos(pos) && hit(*fetchElm(elms, pos))) || pos == Empty) {
      return pos;
    }
    probeIndex += i;
    assert(i <= mask && probeIndex == h0 + (i + i*i) / 2);
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
  size_t mask = tableMask();
  auto* elms = data();
  auto* hashtable = hashTab();
  int32_t* ret = nullptr;
  for (size_t probe = h0, i = 1;; ++i) {
    auto ei = &hashtable[probe & mask];
    ssize_t pos = *ei;
    if (validPos(pos)) {
      if (hit(*fetchElm(elms, pos))) {
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
    if (!validPos(*ei)) {
      return ei;
    }
    probe += i;
    assert(i <= mask && probe == h0 + (i + i*i) / 2);
  }
}

ALWAYS_INLINE
int32_t* BaseMap::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), tableMask(), h0);
}

template <bool raw>
ALWAYS_INLINE
void BaseMap::setImpl(int64_t h, const TypedValue* val) {
  if (!raw) {
    mutate();
  }
  assert(val->m_type != KindOfRef);
  assert(canMutateBuffer());
retry:
  auto* p = findForInsert(h);
  assert(p);
  if (validPos(*p)) {
    auto& e = *fetchElm(data(), *p);
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
  if (!raw) {
    ++m_version;
  }
  auto& e = allocElm(p);
  cellDup(*val, e.data);
  e.setIntKey(h);
  updateNextKI(h);
}

template <bool raw>
void BaseMap::setImpl(StringData* key, const TypedValue* val) {
  if (!raw) {
    mutate();
  }
  assert(val->m_type != KindOfRef);
  assert(canMutateBuffer());
retry:
  strhash_t h = key->hash();
  auto* p = findForInsert(key, h);
  assert(p);
  if (validPos(*p)) {
    auto& e = *fetchElm(data(), *p);
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
  if (!raw) {
    ++m_version;
  }
  auto& e = allocElm(p);
  cellDup(*val, e.data);
  e.setStrKey(key, h);
  updateIntLikeStrKeys(key);
}

void BaseMap::setRaw(int64_t h, const TypedValue* val) {
  setImpl<true>(h, val);
}

void BaseMap::setRaw(StringData* key, const TypedValue* val) {
  setImpl<true>(key, val);
}

void BaseMap::set(int64_t h, const TypedValue* val) {
  setImpl<false>(h, val);
}

void BaseMap::set(StringData* key, const TypedValue* val) {
  setImpl<false>(key, val);
}

void BaseMap::eraseNoCompact(ssize_t pos) {
  assert(canMutateBuffer());
  assert(validPos(pos) && !isTombstone(pos));
  assert(m_size > 0);
  // We're removing an element, so we make sure m_pos is not pointing at
  // garbage by setting it to invalid_index
  arrayData()->m_pos = ArrayData::invalid_index;
  ((MixedArray*)arrayData())->eraseNoCompact(pos);
  --m_size;
}

NEVER_INLINE void BaseMap::makeRoom() {
  assert(isFull());
  assert(posLimit() == cap());
  if (LIKELY(!isDensityTooLow())) {
    if (UNLIKELY(cap() == MaxSize)) {
      throwTooLarge();
    }
    grow(cap() ? cap()*2 : SmallSize, cap() ? tableMask()*2+1 : SmallMask);
  } else {
    compact();
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isFull());
}

NEVER_INLINE void BaseMap::reserve(int64_t sz) {
  assert(m_size <= posLimit() && posLimit() <= cap());
  uint32_t newCap;
  uint32_t newMask;
  if (LIKELY(sz > int64_t(cap()))) {
    if (UNLIKELY(sz > int64_t(MaxReserveSize))) {
      throwReserveTooLarge();
    }
    // Fast path: The requested capacity is greater than the current capacity.
    // Grow to the smallest allowed capacity that is sufficient.
    auto lgSize = MinLgTableSize;
    for (newCap = SmallSize; newCap < sz; newCap <<= 1) ++lgSize;
    newMask = (size_t(1U) << lgSize) - 1;
    assert(lgSize <= MaxLgTableSize && newCap > cap());
    // Fall through to the call to grow() below
  } else if (LIKELY(!hasTombstones())) {
    // Fast path: There are no tombstones and the requested capacity is less
    // than or equal to the current capacity. Do nothing and return.
    return;
  } else if (sz + int64_t(posLimit() - m_size) <= int64_t(cap()) ||
             isDensityTooLow()) {
    // If we reach this case, then either (1) density is too low (this is
    // possible because of methods like retain()), in which case we compact
    // to make room and return, OR (2) density is not too low and either
    // sz < m_size or there's enough room to add sz-m_size elements, in
    // which case we do nothing and return.
    adjustIfDensityTooLow();
    assert(sz + int64_t(posLimit() - m_size) <= int64_t(cap()));
    return;
  } else {
    // If we reach this case, then density is not too low and sz > m_size and
    // there is not enough room to add sz-m_size elements. While would could
    // compact to make room, it's better for Hysteresis if we grow capacity
    // by 2x instead.
    assert(!isDensityTooLow());
    assert(sz + int64_t(posLimit() - m_size) > int64_t(cap()));
    assert(cap() < MaxSize && tableMask() != 0);
    newCap = cap() * 2;
    newMask = tableMask() * 2 + 1;
    assert(0 < sz && sz <= int64_t(newCap));
    // Fall through to the call to grow() below
  }
  grow(newCap, newMask);
  assert(canMutateBuffer());
}

ALWAYS_INLINE
void BaseMap::resizeHelper(uint32_t newCap) {
  assert(newCap >= m_size);
  assert(m_immCopy.isNull());
  // Allocate a new ArrayData with the specified capacity and dup
  // all the elements (without copying over tombstones).
  auto* ad = (arrayData() == staticEmptyMixedArray()) ?
    reinterpret_cast<MixedArray*>(MixedArray::MakeReserveMixed(newCap)) :
    MixedArray::CopyReserve((MixedArray*)arrayData(), newCap);
  decRefArr(arrayData());
  m_data = mixedData(ad);
  assert(canMutateBuffer());
}

void BaseMap::grow(uint32_t newCap, uint32_t newMask) {
  assert(m_size <= posLimit() && posLimit() <= cap() && cap() <= newCap);
  assert(SmallSize <= newCap && newCap <= MaxSize);
  assert(m_size <= newCap);
  assert(newMask > 0 && ((newMask+1) & newMask) == 0);
  assert(newMask == folly::nextPowTwo<uint64_t>(newCap) - 1);
  assert(newCap == computeMaxElms(newMask));
  auto* oldAd = arrayData();
  dropImmCopy();
  if (m_size > 0 && !oldAd->hasMultipleRefs()) {
    // MixedArray::Grow can only handle non-empty cases where the
    // buffer's refcount is 1.
    m_data = mixedData(MixedArray::Grow((MixedArray*)oldAd, newCap, newMask));
    arrayData()->incRefCount();
    decRefArr(oldAd);
  } else {
    // For cases where m_size is zero or the buffer's refcount is
    // greater than 1, call resizeHelper().
    resizeHelper(newCap);
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
}

void BaseMap::compact() {
  assert(isDensityTooLow());
  dropImmCopy();
  if (!arrayData()->hasMultipleRefs()) {
    // MixedArray::compact can only handle cases where the buffer's
    // refcount is 1.
    ((MixedArray*)arrayData())->compact(false);
  } else {
    // For cases where the buffer's refcount is greater than 1, call
    // resizeHelper().
    resizeHelper(cap());
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isDensityTooLow());
}

void BaseMap::shrink(uint32_t oldCap /* = 0 */) {
  assert(isCapacityTooHigh() && (oldCap == 0 || oldCap < cap()));
  assert(m_size <= posLimit() && posLimit() <= cap());
  dropImmCopy();
  uint32_t newCap;
  if (oldCap != 0) {
    // If an old capacity was specified, use that
    newCap = oldCap;
    assert(newCap == computeMaxElms(folly::nextPowTwo<uint64_t>(newCap) - 1));
  } else {
    if (m_size == 0 && nextKI() == 0) {
      decRefArr(arrayData());
      m_data = mixedData(staticEmptyMixedArray());
      return;
    }
    // If no old capacity was provided, we compute the largest capacity
    // where m_size/cap() is less than or equal to 0.5 for good hysteresis
    size_t doubleSz = size_t(m_size) * 2;
    uint32_t capThreshold = (doubleSz < size_t(MaxSize)) ? doubleSz : MaxSize;
    for (newCap = SmallSize * 2; newCap < capThreshold; newCap <<= 1) {}
  }
  assert(SmallSize <= newCap && newCap <= MaxSize);
  assert(m_size <= newCap);
  auto* oldAd = arrayData();
  if (!oldAd->hasMultipleRefs()) {
    // If the buffer's refcount is 1, we can teleport the elements
    // to a new buffer
    auto* oldBuf = data();
    auto oldUsed = posLimit();
    auto oldNextKI = nextKI();
    auto* data = mixedData(
      reinterpret_cast<MixedArray*>(MixedArray::MakeReserveMixed(newCap))
    );
    m_data = data;
    auto* table = (int32_t*)(data + size_t(newCap));
    arrayData()->m_size = m_size;
    setPosLimit(m_size);
    setNextKI(oldNextKI);
    for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
      frPos = skipTombstonesUnsafe(frPos, oldUsed, oldBuf);
      copyElm(oldBuf[frPos], data[toPos]);
      *findForNewInsert(table, tableMask(), data[toPos].probe()) = toPos;
    }
    ((MixedArray*)oldAd)->setZombie();
    decRefArr(oldAd);
  } else {
    // For cases where the buffer's refcount is greater than 1, call
    // resizeHelper()
    resizeHelper(newCap);
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isCapacityTooHigh());
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
  auto* start = data();
  auto* end = data() + posLimit();
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  for (;;) {
    if (checkTypes) {
      while (!isTombstone(start)) {
        allInts = (allInts && acc.isInt(*start));
        allStrs = (allStrs && acc.isStr(*start));
        ++start;
        if (start == end) {
          goto done;
        }
      }
    } else {
      while (!isTombstone(start)) {
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
    while (isTombstone(end)) {
      --end;
      if (start == end) {
        goto done;
      }
    }
    copyElm(*end, *start);
  }
  done:
  setPosLimit(start - data());
  // The logic above possibly moved elements and tombstones around within
  // the buffer, so we make sure m_pos is not pointing at garbage by setting
  // it to invalid_index
  arrayData()->m_pos = ArrayData::invalid_index;
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
void BaseMap::postSort() {  // Must provide the nothrow guarantee
  assert(m_size > 0);
  auto const table = hashTab();
  initHash(table, hashSize());
  auto mask = tableMask();
  auto data = this->data();
  for (uint32_t pos = 0; pos < posLimit(); ++pos) {
    auto& e = data[pos];
    *findForNewInsert(table, mask, e.probe()) = pos;
  }
}

#define SORT_CASE(flag, cmp_type, acc_type)                     \
  case flag: {                                                  \
    if (ascending) {                                            \
      cmp_type##Compare<acc_type, flag, true> comp;             \
      HPHP::Sort::sort(data(), data() + m_size, comp);          \
    } else {                                                    \
      cmp_type##Compare<acc_type, flag, false> comp;            \
      HPHP::Sort::sort(data(), data() + m_size, comp);          \
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
  if (m_size <= 1) return;
  mutateAndBump();
  SORT_BODY(MapValAccessor);
}

void BaseMap::ksort(int sort_flags, bool ascending) {
  if (m_size <= 1) return;
  mutateAndBump();
  SORT_BODY(MapKeyAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT
#undef SORT_BODY

#define USER_SORT_BODY(acc_type)                                \
  do {                                                          \
    CallCtx ctx;                                                \
    CallerFrame cf;                                             \
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
  if (m_size <= 1) return true;
  mutateAndBump();
  USER_SORT_BODY(MapValAccessor);
}

bool BaseMap::uksort(const Variant& cmp_function) {
  if (m_size <= 1) return true;
  mutateAndBump();
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
                        const TypedValue* val) {
  static_cast<BaseMap*>(obj)->set(key, val);
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

bool BaseMap::OffsetContains(ObjectData* obj, const TypedValue* key) {
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

// This function will create a immutable copy of this Map (if it doesn't
// already exist) and then return it
Object c_Map::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto* mp = NEWOBJ(c_ImmMap)();
    m_immCopy = mp;
    arrayData()->incRefCount();
    mp->m_size = m_size;
    mp->m_version = m_version;
    mp->m_data = m_data;
    mp->setIntLikeStrKeys(intLikeStrKeys());
  }
  assert(!m_immCopy.isNull());
  assert(m_data == static_cast<c_ImmMap*>(m_immCopy.get())->m_data);
  assert(arrayData()->hasMultipleRefs());
  return m_immCopy;
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
      for (uint i = 0; i < mp1->posLimit(); ++i) {
        if (mp1->isTombstone(i)) continue;
        const BaseMap::Elm& e = mp1->data()[i];
        TypedValue* tv2;
        if (e.hasIntKey()) {
          tv2 = mp2->get(e.ikey);
        } else {
          assert(e.hasStrKey());
          tv2 = mp2->get(e.skey);
        }
        if (!tv2) return false;
        if (!equal(tvAsCVarRef(&e.data), tvAsCVarRef(tv2))) return false;
      }
      return true;
    }
    case EqualityFlavor::OrderMatters: {
      // obj1 and obj2 must compare equal according to OrderIrrelevant;
      // additionally, the (identical) keys of obj1 and obj2 must be in the
      // same iteration order.
      uint compared = 0;
      for (uint ix1 = 0, ix2 = 0;
           ix1 < mp1->posLimit() && ix2 < mp2->posLimit() ; ) {

        auto tomb1 = mp1->isTombstone(ix1);
        auto tomb2 = mp2->isTombstone(ix2);

        if (tomb1 || tomb2) {
          if (tomb1) { ++ix1; }
          if (tomb2) { ++ix2; }
          continue;
        }

        const BaseMap::Elm& e1 = mp1->data()[ix1];
        const BaseMap::Elm& e2 = mp2->data()[ix2];

        if (e1.hasIntKey()) {
          if (!e2.hasIntKey() ||
              e1.ikey != e2.ikey) {
            return false;
          }
        } else {
          assert(e1.hasStrKey());
          if (!e2.hasStrKey() || !equal(e1.skey, e2.skey)) {
            return false;
          }
        }
        if (!equal(tvAsCVarRef(&e1.data), tvAsCVarRef(&e2.data))) {
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
      mp->updateNextKI(h);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      p = mp->findForInsert(key, h);
      // Check to be robust against manually crafted inputs with conflicting
      // elements
      if (UNLIKELY(validPos(*p))) goto do_unserialize;
      e = &mp->allocElm(p);
      e->setStrKey(key, h);
      mp->updateIntLikeStrKeys(key);
    } else {
      throw Exception("Invalid key");
    }
    e->data.m_type = KindOfNull;
do_unserialize:
    tvAsVariant(&e->data).unserialize(uns, Uns::Mode::ColValue);
  }
}

Object c_Map::t_tovector() { return materializeImpl<c_Vector>(this); }
Object c_Map::t_toimmvector() { return materializeImpl<c_ImmVector>(this); }
Object c_Map::t_tomap() { return Object::attach(c_Map::Clone(this)); }
Object c_Map::t_toimmmap() { return getImmutableCopy(); }
Object c_Map::t_toset() { return materializeImpl<c_Set>(this); }
Object c_Map::t_toimmset() { return materializeImpl<c_ImmSet>(this); }
Object c_Map::t_immutable() { return getImmutableCopy(); }

Object c_ImmMap::t_tovector() { return materializeImpl<c_Vector>(this); }
Object c_ImmMap::t_toimmvector() { return materializeImpl<c_ImmVector>(this); }
Object c_ImmMap::t_tomap() { return materializeImpl<c_Map>(this); }
Object c_ImmMap::t_toimmmap() { return this; }
Object c_ImmMap::t_toset() { return materializeImpl<c_Set>(this); }
Object c_ImmMap::t_toimmset() { return materializeImpl<c_ImmSet>(this); }
Object c_ImmMap::t_immutable() { return this; }

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
  if (iterable.isNull()) return;
  init(iterable);
}

c_ImmMap* c_ImmMap::Clone(ObjectData* obj) {
  return BaseMap::Clone<c_ImmMap>(obj);
}

///////////////////////////////////////////////////////////////////////////////
// BaseSet

static int32_t empty_set_hash_slot = BaseSet::Empty;

// Public

void BaseSet::copyBuffer() {
  assert(hasImmutableBuffer());
  if (!m_size) {
    uint32_t used = 0;
    uint32_t cap = 0;
    m_capAndUsed = (uint64_t(cap) << 32) | uint64_t(used);
    m_tableMask = 0;
    m_data = nullptr;
    m_hash = &empty_set_hash_slot;
    m_immCopy.reset();
    return;
  }
  assert(hashSize() > 0);
  auto needed = size_t(m_cap) * sizeof(Elm) + hashSize() * sizeof(int32_t);
  auto* newData = (Elm*)MM().objMallocLogged(needed);
  assert(newData);
  auto* newHash = (int32_t*)(newData + m_cap);
  assert(newHash);
  wordcpy(newHash, m_hash, hashSize());
  auto* eLimit = elmLimit();
  auto* te = newData;
  for (auto* e = firstElm(); e != eLimit; ++e, ++te) {
    if (isTombstone(e)) {
      te->data.m_type = e->data.m_type;
    } else {
      dupElm(*e, *te);
    }
  }
  m_data = newData;
  m_hash = newHash;
  m_immCopy.reset();
}

void BaseSet::addAllKeysOf(const Cell& container) {
  assert(isContainer(container));

  auto sz = getContainerSize(container);
  ArrayIter iter(container);
  if (!sz || !iter) { return; }

  mutateAndBump();
  // In theory we could be deferring the version bump above because all the
  // elements of iter could already be present in the set.
  reserve(m_size + sz);
  for (; iter; ++iter) { addRaw(iter.first()); }
  compactIfNecessary();
}

void BaseSet::addAll(const Variant& t) {
  if (t.isNull()) { return; } // nothing to do

  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  if (!iter) { return; }

  mutateAndBump();
  // In theory we could be deferring the version bump above for the
  // container case because all the elements of iter could already be
  // present in the set: there's no destructor invocations because Sets are
  // int/string only. We can save m_size and version bump after the
  // insertion loop only if the size has increased. However, as presently
  // constituted, such an ->addAll() call is a time bomb and a no-op,
  if (LIKELY(!iter.hasIteratorObj())) {
    reserve(m_size + sz);  // presume that iter is not one value sz times ...
    do {
      addRaw(iter.secondRefPlus());
      ++iter;
    } while (iter);
    compactIfNecessary(); // ... and compact back if we're wrong
  } else {
    assert(sz == 0); // iter is an Iterable with unknown number of elements
    do {
      add(iter.second());
      ++iter;
    } while (iter);
  }
}

void BaseSet::init(const Variant& t) {
  addAll(t);
}

template<bool raw>
ALWAYS_INLINE
void BaseSet::addImpl(int64_t h) {
  if (!raw) {
    mutate();
  }
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
  if (!raw) {
    ++m_version;
  }
}

template<bool raw>
ALWAYS_INLINE
void BaseSet::addImpl(StringData *key) {
  if (!raw) {
    mutate();
  }
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
  if (!raw) {
    ++m_version;
  }
}

void BaseSet::addRaw(int64_t h) {
  addImpl<true>(h);
}

void BaseSet::addRaw(StringData *key) {
  addImpl<true>(key);
}

void BaseSet::add(int64_t h) {
  addImpl<false>(h);
}

void BaseSet::add(StringData *key) {
  addImpl<false>(key);
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
  mutate();
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
  mutate();
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
    mutateAndBump();
    auto* e = elmLimit() - 1;
    for (;; --e) {
      assert(e >= data());
      if (!isTombstone(e)) break;
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
    mutateAndBump();
    auto* e = data();
    for (;; ++e) {
      assert(e != elmLimit());
      if (!isTombstone(e)) break;
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

void BaseSet::remove(int64_t key) {
  mutateAndBump();
  auto* p = findForInsert(key);
  if (validPos(*p)) {
    erase(p);
  }
}

void BaseSet::remove(StringData* key) {
  mutateAndBump();
  auto* p = findForInsert(key, key->hash());
  if (validPos(*p)) {
    erase(p);
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
  if (LIKELY(!isDensityTooLow())) {
    if (UNLIKELY(m_cap == MaxSize)) {
      throwTooLarge();
    }
    grow(m_cap ? m_cap*2 : SmallSize, m_cap ? m_tableMask*2+1 : SmallMask);
  } else {
    compact();
  }
  assert(!isFull());
}

NEVER_INLINE void BaseSet::reserve(int64_t sz) {
  assert(m_size <= m_used && m_used <= m_cap);
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
  assert(newCap >= SmallSize && newMask >= SmallMask);
  size_t newHashSize = size_t(newMask) + 1;
  assert(folly::isPowTwo(newHashSize) && computeMaxElms(newMask) == newCap);
  assert(m_size <= newCap && newCap <= MaxSize);
  auto* oldData = data();
  auto oldHashSize = oldData ? hashSize() : 0;
  auto oldCap = m_cap;
  auto needed = size_t(newCap) * sizeof(Elm) + newHashSize * sizeof(int32_t);
  auto* data = (Elm*)MM().objMallocLogged(needed);
  auto* table = (int32_t*)(data + size_t(newCap));
  m_data = data;
  m_hash = table;
  m_tableMask = newMask;
  auto oldUsed DEBUG_ONLY = m_used;
  m_capAndUsed = (uint64_t(newCap) << 32) | uint64_t(m_size);
  initHash(table, newHashSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(frPos, oldData)) {
      assert(frPos + 1 < oldUsed);
      ++frPos;
    }
    copyElm(oldData[frPos], data[toPos]);
    *findForNewInsert(table, newMask, data[toPos].probe()) = toPos;
  }
  if (!hasImmutableBuffer()) {
    if (oldData) {
      MM().objFreeLogged(
        oldData, size_t(oldCap) * sizeof(Elm) + oldHashSize * sizeof(int32_t));
    }
  } else {
    // In this case we don't own the oldData buffer, so we don't free
    // it and we incRef all of the elements
    for (uint32_t toPos = 0; toPos < m_size; ++toPos) {
      tvRefcountedIncRef(&data[toPos].data);
    }
    m_immCopy.reset();
  }
  assert(!hasImmutableBuffer());
}

void BaseSet::compact() {
  assert(!hasImmutableBuffer());
  assert(isDensityTooLow());
  auto* elms = data();
  assert(elms);
  auto mask = m_tableMask;
  size_t tableSize = hashSize();
  auto table = hashTab();
  initHash(table, tableSize);
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(frPos, elms)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = elms[toPos];
    if (toPos != frPos) {
      copyElm(elms[frPos], toE);
    }
    *findForNewInsert(table, mask, toE.probe()) = toPos;
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

// This function will create a immutable copy of this Set (if it doesn't
// already exist) and then return it
Object c_Set::getImmutableCopy() {
  if (!hasImmutableBuffer()) {
    assert(m_immCopy.isNull());
    auto* st = NEWOBJ(c_ImmSet)();
    m_immCopy = st;
    st->m_data = m_data;
    st->m_hash = m_hash;
    st->m_size = m_size;
    st->m_used = m_used;
    st->m_cap = m_cap;
    st->m_tableMask = m_tableMask;
    st->m_version = m_version;
  }
  return m_immCopy;
}

bool BaseSet::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto st1 = static_cast<const BaseSet*>(obj1);
  auto st2 = static_cast<const BaseSet*>(obj2);
  if (st1->m_size != st2->m_size) return false;

  auto* eLimit = st1->elmLimit();
  for (auto* e = st1->firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasInt()) {
      if (!st2->contains(e->data.m_data.num)) return false;
    } else {
      assert(e->hasStr());
      if (!st2->contains(e->data.m_data.pstr)) return false;
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
  assert(thiz->hashSize() > 0);
  target->m_capAndUsed = thiz->m_capAndUsed;
  target->m_tableMask = thiz->m_tableMask;
  target->m_size = thiz->m_size;
  auto needed =
    size_t(thiz->m_cap) * sizeof(Elm) + thiz->hashSize() * sizeof(int32_t);
  target->m_data = (Elm*)MM().objMallocLogged(needed);
  target->m_hash = (int32_t*)(target->m_data + target->m_cap);
  wordcpy(target->hashTab(), thiz->hashTab(), thiz->hashSize());

  for (ssize_t i = 0; i < thiz->posLimit(); ++i) {
    Elm& e = thiz->data()[i];
    Elm& te = target->data()[i];
    if (isTombstone(&e)) {
      te.data.m_type = e.data.m_type;
    } else {
      dupElm(e, te);
    }
  }

  return target;
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

Object c_Set::t_remove(const Variant& key) {
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
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    ai.append(tvAsCVarRef(&e->data));
  }
  return ai.toArray();
}

Object BaseSet::php_getIterator() {
  auto* it = NEWOBJ(c_SetIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_version = getVersion();
  return it;
}

template<class TSet>
ALWAYS_INLINE
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
  auto* st = NEWOBJ(TSet)();
  assert(!st->hasImmutableBuffer());
  Object obj = st;
  if (!m_size) return obj;
  st->reserve(posLimit()); // presume callback maintains size ...
  for (ssize_t ipos = iter_begin(); iter_valid(ipos); ipos = iter_next(ipos)) {
    auto* e = iter_elm(ipos);
    TypedValue tvCbRet;
    int32_t pVer = m_version;
    g_context->invokeFuncFew(&tvCbRet, ctx, 1, &e->data);
    // Now that tvCbRet is live, make sure to decref even if we throw.
    SCOPE_EXIT { tvRefcountedDecRef(&tvCbRet); };
    if (UNLIKELY(m_version != pVer)) throw_collection_modified();
    st->addRaw(&tvCbRet);
  }
  st->compactIfNecessary(); // ... and compact if we were wrong
  return obj;
}

template<class TSet>
ALWAYS_INLINE
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
  auto* st = NEWOBJ(TSet)();
  assert(!st->hasImmutableBuffer());
  Object obj = st;
  if (!m_size) return obj;
  // we don't st->reserve, because we don't know how selective callback will be
  int32_t version = m_version;
  for (ssize_t ipos = iter_begin(); iter_valid(ipos); ipos = iter_next(ipos)) {
    auto* e = iter_elm(ipos);
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!b) continue;
    e = iter_elm(ipos);
    if (e->hasInt()) {
      st->addRaw(e->data.m_data.num);
    } else {
      assert(e->hasStr());
      st->addRaw(e->data.m_data.pstr);
    }
  }
  return obj;
}

template<class TSet>
ALWAYS_INLINE
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

ALWAYS_INLINE
Object BaseSet::php_retain(const Variant& callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter must be a valid callback"));
    throw e;
  }
  auto size = m_size;
  if (!size) { return this; }
  for (ssize_t ipos = iter_begin(); iter_valid(ipos); ipos = iter_next(ipos)) {
    int32_t version = m_version;
    auto* e = iter_elm(ipos);
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) { continue; }
    mutateAndBump();
    version = m_version;
    e = iter_elm(ipos);
    int32_t* pp;
    if (e->hasInt()) {
      pp = findForInsert(e->data.m_data.num);
    } else {
      assert(e->hasStr());
      auto const sd = e->data.m_data.pstr;
      pp = findForInsert(sd, sd->hash());
    }
    eraseNoCompact(pp);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
  }
  assert(m_size <= size);
  compactIfNecessary();
  return this;
}

template<class TSet>
ALWAYS_INLINE
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
    while (isTombstone(frPos)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = st->m_data[toPos];
    dupElm(m_data[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
  }
  return obj;
}

template<class TSet, bool checkVersion>
ALWAYS_INLINE
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
  assert(!st->hasImmutableBuffer());
  Object obj = st;
  if (!m_size) return obj;
  uint32_t used = posLimit();
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (uint i = 0; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm* e = &data()[i];
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) continue;
    e = &data()[i];
    if (e->hasInt()) {
      st->addRaw(e->data.m_data.num);
    } else {
      assert(e->hasStr());
      st->addRaw(e->data.m_data.pstr);
    }
  }
  return obj;
}

template<class TSet>
ALWAYS_INLINE
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
  uint32_t frPos = nthElmPos(len);
  auto table = st->hashTab();
  auto mask = st->m_tableMask;
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(frPos)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = st->m_data[toPos];
    dupElm(m_data[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
  }
  return obj;
}

template<class TSet, bool checkVersion>
ALWAYS_INLINE
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
  assert(!st->hasImmutableBuffer());
  Object obj = st;
  if (!m_size) return obj;
  uint32_t used = posLimit();
  // we don't st->reserve(), because we don't know how selective fn will be
  uint i = 0;
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& e = data()[i];
    bool b = invokeAndCastToBool(ctx, 1, &e.data);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
  }
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& e = data()[i];
    if (e.hasInt()) {
      st->addRaw(e.data.m_data.num);
    } else {
      assert(e.hasStr());
      st->addRaw(e.data.m_data.pstr);
    }
  }
  return obj;
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer"));
    throw e;
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer"));
    throw e;
  }
  size_t skipAmt = std::min<size_t>(istart, m_size);
  size_t sz = std::min<size_t>(ilen, size_t(m_size) - skipAmt);
  auto* st = NEWOBJ(TSet)();
  Object obj = st;
  st->reserve(sz);
  st->m_size = st->m_used = sz;
  uint32_t frPos = nthElmPos(skipAmt);
  auto table = st->hashTab();
  auto mask = st->m_tableMask;
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(frPos)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    auto& toE = st->m_data[toPos];
    dupElm(m_data[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
  }
  return obj;
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromItems(const Variant& iterable) {
  auto* st = NEWOBJ(TSet)();
  Object ret = st;
  assert(!st->hasImmutableBuffer());
  st->addAll(iterable);
  return ret;
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromKeysOf(const Variant& container) {
  if (container.isNull()) { return NEWOBJ(TSet)(); }

  const auto& cellContainer = container_as_cell(container);

  auto* target = NEWOBJ(TSet)();
  Object ret = target;
  target->addAllKeysOf(cellContainer);
  return ret;
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromArray(const Variant& arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  auto* st = NEWOBJ(TSet)();
  assert(!st->hasImmutableBuffer());
  Object ret = st;
  ArrayData* ad = arr.getArrayData();
  st->reserve(ad->size()); // presume that ad is not one value repeated ...
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    st->addRaw(ad->getValueRef(pos));
  }
  st->compactIfNecessary(); // ... and compact if we were wrong
  return ret;
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromArrays(int _argc, const Array& _argv /* = null_array */) {
  auto* st = NEWOBJ(TSet)();
  assert(!st->hasImmutableBuffer());
  Object ret = st;
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant arr = iter.second();
    if (!arr.isArray()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Parameters must be arrays"));
      throw e;
    }
    ArrayData* ad = arr.getArrayData();
    // if needed, the correct size can be calculated in a separate loop
    st->reserve(st->size() + ad->size());
    for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
         pos = ad->iter_advance(pos)) {
      st->addRaw(ad->getValueRef(pos));
    }
  }
  st->compactIfNecessary(); // all arrays could contain one value ... repeated
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
  if (m_data && !hasImmutableBuffer()) {
    assert(m_immCopy.isNull());
    deleteElms();
    freeData();
  }
}

void BaseSet::freeData() {
  if (m_data) {
    MM().objFreeLogged(
      m_data, size_t(m_cap) * sizeof(Elm) + hashSize() * sizeof(int32_t));
  }
}

void BaseSet::deleteElms() {
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    tvRefcountedDecRef(&e->data);
  }
}

// Private

NEVER_INLINE
void BaseSet::warnOnStrIntDup() const {
  smart::hash_set<int64_t> seenVals;

  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    int64_t newVal = 0;

    if (e->hasInt()) {
      newVal = e->data.m_data.num;
    } else {
      assert(e->hasStr());
      // isStriclyInteger() puts the int value in newVal as a side effect.
      if (!e->data.m_data.pstr->isStrictlyInteger(newVal)) continue;
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
  ArrayInit ai(m_size, ArrayInit::Mixed{});
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasInt()) {
      ai.set(e->data.m_data.num, tvAsCVarRef(&e->data));
    } else {
      assert(e->hasStr());
      ai.setKeyUnconverted(e->data.m_data.pstr, tvAsCVarRef(&e->data));
    }
  }

  Array arr = ai.toArray();

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
  assert(!BaseSet::isTombstone(&e));
  return hash == e.hash() && e.hasStr() &&
         (s == e.data.m_data.pstr || s->same(e.data.m_data.pstr));
}

static bool hitInt(const BaseSet::Elm& e, int64_t ki) {
  // hitInt() should only be called on an Elm that is referenced by a
  // hash table entry. c_Set guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!BaseSet::isTombstone(&e));
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

void BaseSet::eraseNoCompact(int32_t* pos) {
  assert(!hasImmutableBuffer());
  assert(validPos(*pos) && !isTombstone(*pos));
  assert(data());
  auto* elms = data();
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
}

void BaseSet::erase(int32_t* pos) {
  assert(!hasImmutableBuffer());
  eraseNoCompact(pos);
  // Compact in order to keep elms from being overly sparse. In general,
  // Set's implement tries to avoid letting Elm density drop below 50%.
  compactIfNecessary();
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
  addAll(iterable);
}

Object c_Set::t_add(const Variant& val) {
  add(val);
  return this;
}

Object c_Set::t_addall(const Variant& iterable) {
  addAll(iterable);
  return this;
}

Object c_Set::t_addallkeysof(const Variant& container) {
  if (!container.isNull()) {
    const auto& containerCell = container_as_cell(container);
    addAllKeysOf(containerCell);
  }
  return this;
}

void c_Set::t_reserve(const Variant& sz) {
  if (UNLIKELY(!sz.isInteger())) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter sz must be a non-negative integer"));
    throw e;
  }
  int64_t intSz = sz.toInt64();
  if (UNLIKELY(intSz < 0)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
               "Parameter sz must be a non-negative integer"));
    throw e;
  }
  reserve(intSz); // checks for intSz > MaxReserveSize
}

Object c_Set::t_clear() {
  if (!hasImmutableBuffer()) {
    deleteElms();
    freeData();
  } else {
    m_immCopy.reset();
  }
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

Object c_Set::t_retain(const Variant& callback) {
  return php_retain(callback);
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

Object c_Set::t_slice(const Variant& start, const Variant& len) {
  return BaseSet::php_slice<c_Set>(start, len);
}

Object c_ImmSet::t_slice(const Variant& start, const Variant& len) {
  return BaseSet::php_slice<c_ImmSet>(start, len);
}

Object c_Set::t_concat(const Variant& iterable) {
  return BaseSet::php_concat<c_Vector>(iterable);
}

Object c_ImmSet::t_concat(const Variant& iterable) {
  return BaseSet::php_concat<c_ImmVector>(iterable);
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseSet::php_concat(const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  auto* vec = NEWOBJ(TVector)();
  Object obj = vec;
  uint sz = m_size;
  vec->reserve((size_t)sz + itSize);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);

  uint32_t used = posLimit();
  for (uint32_t i = 0, j = 0; i < used; ++i) {
    if (isTombstone(i)) {
      continue;
    }
    cellDup(data()[i].data, vec->m_data[j]);
    ++j;
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return obj;
}

Variant c_Set::t_firstvalue() {
  return BaseSet::php_firstValue();
}

Variant c_ImmSet::t_firstvalue() {
  return BaseSet::php_firstValue();
}

Variant BaseSet::php_firstValue() {
  if (!m_size) return init_null();
  auto* e = firstElm();
  assert(e != elmLimit());
  return tvAsCVarRef(&e->data);
}

Variant c_Set::t_lastvalue() {
  return BaseSet::php_lastValue();
}

Variant c_ImmSet::t_lastvalue() {
  return BaseSet::php_lastValue();
}

Variant BaseSet::php_lastValue() {
  if (!m_size) return init_null();
  // TODO Task# 4281431: If nthElmPos(n) is optimized to
  // walk backward from the end when n > m_size/2, then
  // we could use that here instead of having to use a
  // manual while loop.
  uint32_t pos = posLimit() - 1;
  while (isTombstone(pos)) {
    assert(pos > 0);
    --pos;
  }
  return tvAsCVarRef(&m_data[pos].data);
}

Object c_Set::t_removeall(const Variant& iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    Variant v = iter.second();
    if (v.isInteger()) {
      remove(v.toInt64());
    } else if (v.isString()) {
      remove(v.getStringData());
    } else {
      throwBadValueType();
    }
  }
  return this;
}

Object c_Set::t_difference(const Variant& iterable) {
  return t_removeall(iterable);
}

Object c_Set::ti_fromitems(const Variant& iterable) {
  return BaseSet::php_fromItems<c_Set>(iterable);
}

Object c_Set::ti_fromkeysof(const Variant& container) {
  return BaseSet::php_fromKeysOf<c_Set>(container);
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

///////////////////////////////////////////////////////////////////////////////
// ImmSet

void c_ImmSet::t___construct(const Variant& iterable /* = null_variant */) {
  addAll(iterable);
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

Object c_ImmSet::ti_fromkeysof(const Variant& container) {
  return BaseSet::php_fromKeysOf<c_ImmSet>(container);
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

Object c_Set::t_tovector() { return materializeImpl<c_Vector>(this); }
Object c_Set::t_toimmvector() { return materializeImpl<c_ImmVector>(this); }
Object c_Set::t_toset() { return Object::attach(c_Set::Clone(this)); }
Object c_Set::t_toimmset() { return getImmutableCopy(); }
Object c_Set::t_immutable() { return getImmutableCopy(); }

Object c_ImmSet::t_tovector() { return materializeImpl<c_Vector>(this); }
Object c_ImmSet::t_toimmvector() { return materializeImpl<c_ImmVector>(this); }
Object c_ImmSet::t_toset() { return materializeImpl<c_Set>(this); }
Object c_ImmSet::t_toimmset() { return this; }
Object c_ImmSet::t_immutable() { return this; }

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
  if (!st->iter_valid(m_pos)) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(st->iter_value(m_pos));
}

Variant c_SetIterator::t_key() {
  return t_current();
}

bool c_SetIterator::t_valid() {
  auto const st = m_obj.get();
  return st->iter_valid(m_pos);
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
  assert(vec->canMutateBuffer());
  vec->setSize(2);
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

Variant c_Pair::t_at(const Variant& key) {
  assert(isFullyConstructed());
  auto* k = key.asCell();
  if (k->m_type == KindOfInt64) {
    return Variant(tvAsCVarRef(at(k->m_data.num)), Variant::CellDup());
  }
  throwBadKeyType();
}

Variant c_Pair::t_get(const Variant& key) {
  assert(isFullyConstructed());
  auto* k = key.asCell();
  if (k->m_type == KindOfInt64) {
    TypedValue* tv = get(k->m_data.num);
    if (tv) {
      return Variant(tvAsCVarRef(tv), Variant::CellDup());
    } else {
      return init_null();
    }
  }
  throwBadKeyType();
}

bool c_Pair::t_containskey(const Variant& key) {
  assert(isFullyConstructed());
  auto* k = key.asCell();
  if (k->m_type == KindOfInt64) {
    return contains(k->m_data.num);
  }
  throwBadKeyType();
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
  assert(vec->canMutateBuffer());
  for (uint64_t i = 0; i < 2; ++i) {
    g_context->invokeFuncFew(&vec->m_data[i], ctx, 1, &getElms()[i]);
    vec->incSize();
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
  assert(vec->canMutateBuffer());
  for (uint64_t i = 0; i < 2; ++i) {
    TypedValue args[2] = { make_tv<KindOfInt64>(i), getElms()[i] };
    g_context->invokeFuncFew(&vec->m_data[i], ctx, 2, args);
    vec->incSize();
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
    if (invokeAndCastToBool(ctx, 1, &getElms()[i])) {
      vec->addRaw(&getElms()[i]);
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
    TypedValue args[2] = { make_tv<KindOfInt64>(i), getElms()[i] };
    if (invokeAndCastToBool(ctx, 2, args)) {
      vec->addRaw(&getElms()[i]);
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
  assert(vec->canMutateBuffer());
  for (uint64_t i = 0; i < 2 && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (vec->m_capacity <= vec->m_size) {
      vec->grow();
    }
    auto* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->initAdd(&getElms()[i]);
    pair->initAdd(v);
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    vec->incSize();
  }
  return obj;
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
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
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
    if (!invokeAndCastToBool(ctx, 1, &getElms()[i])) break;
    vec->addRaw(&getElms()[i]);
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
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
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
    if (!invokeAndCastToBool(ctx, 1, &getElms()[i])) break;
  }
  for (; i < 2; ++i) {
    vec->addRaw(&getElms()[i]);
  }
  return obj;
}

Object c_Pair::t_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer"));
    throw e;
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer"));
    throw e;
  }
  size_t skipAmt = std::min<size_t>(istart, 2);
  size_t sz = std::min<size_t>(ilen, size_t(2) - skipAmt);
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  vec->reserve(sz);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  for (size_t i = 0; i < sz; ++i) {
    cellDup(getElms()[i + skipAmt], vec->m_data[i]);
  }
  return obj;
}

Object c_Pair::t_concat(const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  auto* vec = NEWOBJ(c_ImmVector)();
  Object obj = vec;
  vec->reserve((size_t)2 + itSize);
  assert(vec->canMutateBuffer());
  vec->setSize(2);

  for (uint32_t i = 0; i < 2; ++i) {
    cellDup(getElms()[i], vec->m_data[i]);
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return obj;
}

Variant c_Pair::t_firstvalue() {
  return tvAsCVarRef(&getElms()[0]);
}

Variant c_Pair::t_firstkey() {
  return 0;
}

Variant c_Pair::t_lastvalue() {
  return tvAsCVarRef(&getElms()[1]);
}

Variant c_Pair::t_lastkey() {
  return 1;
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

bool c_Pair::OffsetContains(ObjectData* obj, const TypedValue* key) {
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

Object c_Pair::t_tovector() { return materializeImpl<c_Vector>(this); }
Object c_Pair::t_toimmvector() { return materializeImpl<c_ImmVector>(this); }
Object c_Pair::t_tomap() { return materializeImpl<c_Map>(this); }
Object c_Pair::t_toimmmap() { return materializeImpl<c_ImmMap>(this); }
Object c_Pair::t_toset() { return materializeImpl<c_Set>(this); }
Object c_Pair::t_toimmset() { return materializeImpl<c_ImmSet>(this); }
Object c_Pair::t_immutable() { return this; }

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
  ArrayInit ai(arr->size(), ArrayInit::Mixed{});
  for (ArrayIter iter(arr); iter; ++iter) {
    Variant v = iter.secondRef();
    collectionDeepCopyTV(v.asTypedValue());
    ai.set(iter.first(), std::move(v));
  }
  return ai.toArray().detach();
}

template<typename TVector>
ObjectData* collectionDeepCopyBaseVector(TVector *vec) {
  vec = TVector::Clone(vec);
  Object o = Object::attach(vec);
  vec->mutate();
  assert(vec->canMutateBuffer());
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
      if (UNLIKELY(!vec->canMutateBuffer() &&
                   ret->m_type != KindOfObject &&
                   ret->m_type != KindOfResource)) {
        vec->mutate();
        ret = BaseVector::OffsetAt<true>(obj, key);
      }
      return ret;
    }
    case Collection::ImmVectorType:
      ret = BaseVector::OffsetAt<true>(obj, key);
      break;
    case Collection::MapType: {
      ret = BaseMap::OffsetAt<true>(obj, key);
      // We're about to expose an element of a Map in an lvalue context;
      // if the element is a value-type (anything other than objects and
      // resources) we need to sever any buffer sharing that might be going on
      auto* mp = static_cast<c_Map*>(obj);
      if (UNLIKELY(!mp->canMutateBuffer() &&
                   ret->m_type != KindOfObject &&
                   ret->m_type != KindOfResource)) {
        mp->mutate();
        ret = BaseMap::OffsetAt<true>(obj, key);
      }
      return ret;
    }
    case Collection::ImmMapType:
      ret = BaseMap::OffsetAt<true>(obj, key);
      break;
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::PairType:
      ret = c_Pair::OffsetAt<true>(obj, key);
      break;
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
      static_cast<c_Map*>(obj)->mutate();
      return BaseMap::OffsetAt<true>(obj, key);
    case Collection::SetType:
    case Collection::ImmSetType:
      BaseSet::throwNoIndexAccess();
    case Collection::ImmVectorType:
    case Collection::ImmMapType:
    case Collection::PairType:
      throw_cannot_modify_immutable_object(obj->o_getClassName().data());
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

void collectionSet(ObjectData* obj, const TypedValue* key,
                   const TypedValue* val) {
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
  auto* key = offset.asCell();
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
