/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/ext/array/ext_array.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/util/text-util.h"

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
  auto col = req::make<TCollection>();
  col->init(VarNR(obj));
  return Object{std::move(col)};
}

static ALWAYS_INLINE
const Cell container_as_cell(const Variant& container) {
  const auto& cellContainer = *container.asCell();
  if (UNLIKELY(!isContainer(cellContainer))) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a container (array or collection)");
  }
  return cellContainer;
}

///////////////////////////////////////////////////////////////////////////////

ATTRIBUTE_NORETURN
static void throwIntOOB(int64_t key, bool isVector = false);

void throwIntOOB(int64_t key, bool isVector /* = false */) {
  String msg(50, ReserveString);
  auto buf = msg.bufferSlice();
  auto sz = snprintf(
    buf.data(), buf.size() + 1,
    "Integer key %" PRId64 " is %s", key,
    isVector ? "out of bounds" : "not defined"
  );
  msg.setSize(std::min<int>(sz, buf.size()));
  SystemLib::throwOutOfBoundsExceptionObject(msg);
}

void throwOOB(int64_t key) {
  throwIntOOB(key, true);
}

ATTRIBUTE_NORETURN static void throwStrOOB(StringData* key);

void throwStrOOB(StringData* key) {
  constexpr size_t maxDisplaySize = 100;
  auto const keySize = key->size();
  auto const keyIsLarge = (keySize > maxDisplaySize);
  folly::StringPiece part1{"String key \""};
  folly::StringPiece part3 = keyIsLarge ? "\" (truncated) is not defined" :
                                   "\" is not defined";
  folly::StringPiece part2(key->data(), keyIsLarge ? maxDisplaySize : keySize);
  String msg(part1.size() + part2.size() + part2.size(), ReserveString);
  msg += part1;
  msg += part2;
  msg += part3;
  SystemLib::throwOutOfBoundsExceptionObject(msg);
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
      sz = collections::getSize(obj);
      return ArrayIter(obj);
    }
    bool isIterable;
    Object iterable = obj->iterableObject(isIterable);
    if (isIterable) {
      sz = 0;
      return ArrayIter(iterable.detach(), ArrayIter::noInc);
    }
  }
  SystemLib::throwInvalidArgumentExceptionObject(
    "Parameter must be an array or an instance of Traversable");
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
bool BaseVector::t_isempty() {
  return !toBoolImpl();
}

int64_t BaseVector::t_count() {
  return m_size;
}

Object BaseVector::t_items() {
  return SystemLib::AllocLazyIterableViewObject(Variant{this});
}

// ConstIndexAccess
bool BaseVector::t_containskey(const Variant& key) {
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

// KeyedIterable
Object BaseVector::t_getiterator() {
  auto iter = collections::VectorIterator::newInstance();
  Native::data<collections::VectorIterator>(iter)->setVector(this);
  return iter;
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
  if (container.isNull()) { return Object{req::make<TVector>()}; }

  const auto& cellContainer = *container.asCell();
  if (UNLIKELY(!isContainer(cellContainer))) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a container (array or collection)");
  }

  ArrayIter iter(cellContainer);
  auto target = req::make<TVector>();
  target->reserve(getContainerSize(cellContainer));
  assert(target->canMutateBuffer());
  for (; iter; ++iter) { target->addRaw(iter.first()); }
  return Object{std::move(target)};
}

template<class TVector, class MakeArgs>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_map(const Variant& callback, MakeArgs makeArgs) const {
  VMRegAnchor _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }

  auto nv = req::make<TVector>();
  uint32_t sz = m_size;
  nv->reserve(sz);
  assert(nv->canMutateBuffer());
  int32_t version = m_version;
  for (uint32_t i = 0; i < sz; ++i) {
    TypedValue* tv = &nv->data()[i];
    auto args = makeArgs(data()[i], i);
    g_context->invokeFuncFew(tv, ctx, args.size(), &args[0]);
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    nv->incSize();
  }
  return Object{std::move(nv)};
}

template<class TVector, class MakeArgs>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_filter(const Variant& callback, MakeArgs makeArgs) const {
  VMRegAnchor _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  auto nv = req::make<TVector>();
  uint32_t sz = m_size;
  int32_t version = m_version;
  assert(nv->canMutateBuffer());
  for (uint32_t i = 0; i < sz; ++i) {
    auto args = makeArgs(data()[i], i);
    bool b = invokeAndCastToBool(ctx, args.size(), &args[0]);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) {
      nv->addRaw(&data()[i]);
    }
  }
  return Object{std::move(nv)};
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_take(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  auto vec = req::make<TVector>();
  if (len <= 0) {
    return Object{std::move(vec)};
  }
  size_t sz = std::min(size_t(len), size_t(m_size));
  vec->reserve(sz);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  for (size_t i = 0; i < sz; ++i) {
    cellDup(data()[i], vec->data()[i]);
  }
  return Object{std::move(vec)};
}

template<class TVector, bool checkVersion>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  auto vec = req::make<TVector>();
  assert(vec->m_size == 0);
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (uint32_t i = 0; i < m_size; ++i) {
    bool b = invokeAndCastToBool(ctx, 1, &data()[i]);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
    vec->addRaw(&data()[i]);
  }
  return Object{std::move(vec)};
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  auto vec = req::make<TVector>();
  if (len <= 0) len = 0;
  size_t skipAmt = std::min<size_t>(len, m_size);
  size_t sz = size_t(m_size) - skipAmt;
  vec->reserve(sz);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  for (size_t i = 0; i < sz; ++i) {
    cellDup(data()[i + skipAmt], vec->data()[i]);
  }
  return Object{std::move(vec)};
}

template<class TVector, bool checkVersion>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto vec = req::make<TVector>();
  assert(vec->canMutateBuffer());
  uint32_t i = 0;
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (; i < m_size; ++i) {
    bool b = invokeAndCastToBool(ctx, 1, &data()[i]);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
  }
  for (; i < m_size; ++i) {
    vec->addRaw(&data()[i]);
  }
  return Object{std::move(vec)};
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer");
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer");
  }
  size_t skipAmt = std::min<size_t>(istart, m_size);
  size_t sz = std::min<size_t>(ilen, size_t(m_size) - skipAmt);
  auto vec = req::make<TVector>();
  vec->reserve(sz);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  auto* e = data() + skipAmt;
  auto* eLimit = e + sz;
  auto* ne = vec->data();
  for (; e != eLimit; ++e, ++ne) {
    cellDup(*e, *ne);
  }
  return Object{std::move(vec)};
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
    auto pair = req::make<c_Pair>(c_Pair::NoInit{});
    pair->initAdd(&data()[i]);
    pair->initAdd(v);
    bvec->data()[i].m_data.pobj = pair.detach();
    bvec->data()[i].m_type = KindOfObject;
    bvec->incSize();
  }
}

void BaseVector::keys(BaseVector* bvec) {
  assert(bvec->m_size == 0);
  bvec->reserve(m_size);
  assert(bvec->canMutateBuffer());
  bvec->setSize(m_size);
  for (uint32_t i = 0; i < m_size; ++i) {
    bvec->data()[i].m_data.num = i;
    bvec->data()[i].m_type = KindOfInt64;
  }
}

// Others

Object BaseVector::t_lazy() {
  return SystemLib::AllocLazyKeyedIterableViewObject(Variant{this});
}

Array BaseVector::t_toarray() {
  if (!m_size) {
    return empty_array();
  }
  return Array(const_cast<ArrayData*>(arrayData()));
}

Array BaseVector::t_tokeysarray() {
  PackedArrayInit ai(m_size);
  uint32_t sz = m_size;
  for (uint32_t i = 0; i < sz; ++i) {
    ai.append((int64_t)i);
  }
  return ai.toArray();
}

Array BaseVector::t_tovaluesarray() {
  return t_toarray();
}

int64_t BaseVector::t_linearsearch(const Variant& search_value) {
  uint32_t sz = m_size;
  for (uint32_t i = 0; i < sz; ++i) {
    if (same(search_value, tvAsCVarRef(&data()[i]))) {
      return i;
    }
  }
  return -1;
}

bool BaseVector::OffsetIsset(ObjectData* obj, const TypedValue* key) {
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

bool BaseVector::OffsetEmpty(ObjectData* obj, const TypedValue* key) {
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

bool BaseVector::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto bv1 = static_cast<const BaseVector*>(obj1);
  auto bv2 = static_cast<const BaseVector*>(obj2);

  uint32_t sz = bv1->m_size;
  if (sz != bv2->m_size) {
    return false;
  }

  for (uint32_t i = 0; i < sz; ++i) {
    if (!HPHP::equal(tvAsCVarRef(&bv1->data()[i]),
                     tvAsCVarRef(&bv2->data()[i]))) {

      return false;
    }
  }

  return true;
}

// Helpers

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
  memmove(data()+1, data(), m_size * sizeof(TypedValue));
  cellDup(*val, data()[0]);
  incSize();
}

Variant BaseVector::popFront() {
  if (m_size) {
    mutateAndBump();
    Variant ret(tvAsCVarRef(&data()[0]), Variant::CellCopy());
    decSize();
    memmove(data(), data()+1, m_size * sizeof(TypedValue));
    return ret;
  } else {
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot pop empty Vector");
  }
}

void BaseVector::reserveImpl(uint32_t newCap) {
  auto* oldBuf = data();
  auto* oldAd = arrayData();
  m_arr = PackedArray::MakeReserve(newCap);
  m_capacity = arrayData()->cap();
  arrayData()->m_size = m_size;
  if (LIKELY(!oldAd->cowCheck())) {
    std::memcpy(data(), oldBuf, m_size * sizeof(TypedValue));
    // Mark oldAd as having 0 elements so that the array release logic doesn't
    // decRef the elements (since we teleported the elements to a new array)
    assert(oldAd != staticEmptyArray());
    assert(oldAd->isPacked());
    oldAd->m_size = 0;
    decRefArr(oldAd);
  } else {
    auto* dst = data();
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

BaseVector::BaseVector(Class* cls, HeaderKind kind, uint32_t cap)
  : ExtCollectionObjectData(cls, kind)
  , m_size(0)
  , m_versionAndCap(cap)
  , m_arr(cap == 0 ? staticEmptyArray() : PackedArray::MakeReserve(cap))
{}

/**
 * Delegate the responsibility for freeing the buffer to the immutable copy,
 * if it exists.
 */
BaseVector::~BaseVector() {
  // Avoid indirect call, as we know it is a packed array.
  auto const packed = arrayData();
  if (packed->decReleaseCheck()) PackedArray::Release(packed);
}

NEVER_INLINE
void BaseVector::throwBadKeyType() {
  SystemLib::throwInvalidArgumentExceptionObject(
             "Only integer keys may be used with Vectors");
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
    m_arr = staticEmptyArray();
    m_capacity = 0;
    return;
  }
  auto* oldAd = arrayData();
  m_arr = PackedArray::Copy(oldAd);
  assert(oldAd->hasMultipleRefs());
  oldAd->decRefCount();
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, TVector*>::type
BaseVector::Clone(ObjectData* obj) {
  auto thiz = static_cast<TVector*>(obj);
  auto target = static_cast<TVector*>(obj->cloneImpl());
  if (!thiz->m_size) {
    return target;
  }
  thiz->arrayData()->incRefCount();
  target->m_arr = thiz->m_arr;
  target->m_size = thiz->m_size;
  target->m_capacity = thiz->m_capacity;
  return target;
}

c_Vector* c_Vector::Clone(ObjectData* obj) {
  return BaseVector::Clone<c_Vector>(obj);
}

c_ImmVector* c_ImmVector::Clone(ObjectData* obj) {
  return BaseVector::Clone<c_ImmVector>(obj);
}

///////////////////////////////////////////////////////////////////////////////

void BaseVector::t___construct(const Variant& iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Object c_Vector::t_add(const Variant& val) {
  add(val.asCell());
  return Object{this};
}

Object c_Vector::t_addall(const Variant& iterable) {
  // TODO Task# 4324040: Refactor this logic with init()
  if (iterable.isNull()) return Object{this};
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
  return Object{this};
}

Object c_Vector::t_addallkeysof(const Variant& container) {
  if (container.isNull()) {
    return Object{this};
  }

  const auto& containerCell = container_as_cell(container);

  auto sz = getContainerSize(containerCell);
  ArrayIter iter(containerCell);
  if (!sz || !iter) {
    return Object{this};
  }
  reserve(m_size + sz);

  mutateAndBump();
  assert(canMutateBuffer());
  do {
    addRaw(iter.first());
    ++iter;
  } while (iter);

  return Object{this};
}

Object c_Vector::t_append(const Variant& val) {
  add(val.asCell());
  return Object{this};
}

Variant c_Vector::t_pop() {
  if (m_size) {
    mutateAndBump();
    decSize();
    return Variant(tvAsCVarRef(&data()[m_size]), Variant::CellCopy());
  } else {
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot pop empty Vector");
  }
}

int64_t c_Vector::checkRequestedCapacity(const Variant& sz) {
  if (!sz.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter sz must be a non-negative integer");
  }
  int64_t intSz = sz.toInt64();
  if (intSz < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter sz must be a non-negative integer");
  }
  if (intSz > MaxCapacity()) {
    auto msg = folly::format(
      "Parameter sz must be at most {}; {} passed",
      MaxCapacity(),
      intSz
    ).str();
    SystemLib::throwInvalidArgumentExceptionObject(msg);
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
    do {
      decSize();
      tvRefcountedDecRef(&data()[m_size]);
    } while (m_size > requestedSize);
  } else {
    for (; m_size < requestedSize; incSize()) {
      cellDup(*val, data()[m_size]);
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
  m_arr = staticEmptyArray();
  m_size = 0;
  m_capacity = 0;
  return Object{this};
}

Object c_Vector::t_keys() {
  auto vec = req::make<c_Vector>();
  BaseVector::keys(vec.get());
  return Object{std::move(vec)};
}

Object c_Vector::t_values() {
  return Object::attach(BaseVector::Clone<c_Vector>(this));
}

Variant BaseVector::t_at(const Variant& key) {
  return tvAsCVarRef(at(key.asCell()));
}

Variant BaseVector::t_get(const Variant& key) {
  const auto* k = key.asCell();
  if (LIKELY(k->m_type == KindOfInt64)) {
    if ((uint64_t)k->m_data.num >= (uint64_t)m_size) {
      return null_variant;
    }
    return tvAsCVarRef(&data()[k->m_data.num]);
  }
  throwBadKeyType();
}

bool c_Vector::t_contains(const Variant& key) {
  return t_containskey(key);
}

Object c_Vector::t_removekey(const Variant& key) {
  if (!key.isInteger()) {
    throwBadKeyType();
  }
  int64_t k = key.toInt64();
  if (!contains(k)) {
    return Object{this};
  }
  mutateAndBump();
  uint64_t datum = data()[k].m_data.num;
  DataType t = data()[k].m_type;
  if (k+1 < m_size) {
    memmove(&data()[k], &data()[k+1],
            (m_size-(k+1)) * sizeof(TypedValue));
  }
  decSize();
  tvRefcountedDecRefHelper(t, datum);
  return Object{this};
}

void c_Vector::t_reverse() {
  if (m_size <= 1) return;
  mutateAndBump();
  TypedValue* start = data();
  TypedValue* end = start + m_size - 1;
  for (; start < end; ++start, --end) {
    std::swap(start->m_data.num, end->m_data.num);
    std::swap(start->m_type, end->m_type);
  }
}

void c_Vector::t_splice(const Variant& offset, const Variant& len /* = null */,
                        const Variant& replacement /* = null */) {
  if (!offset.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter offset must be an integer");
  }
  if (!len.isNull() && !len.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter len must be null or an integer");
  }
  if (!replacement.isNull()) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Vector::splice does not support replacement parameter");
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
  // Null out each element before decreffing it. We need to do this in case
  // a __destruct method reenters and accesses this Vector object.
  for (int64_t i = startPos; i < endPos; ++i) {
    uint64_t datum = data()[i].m_data.num;
    DataType t = data()[i].m_type;
    tvWriteNull(&data()[i]);
    tvRefcountedDecRefHelper(t, datum);
  }
  // Move elements that came after the deleted elements (if there are any)
  if (endPos < sz) {
    memmove(&data()[startPos], &data()[endPos],
            (sz - endPos) * sizeof(TypedValue));
  }
  setSize(m_size - (endPos - startPos));
}

void c_Vector::t_shuffle() {
  if (m_size <= 1) {
    return;
  }
  mutateAndBump();
  for (uint32_t i = 1; i < m_size; ++i) {
    uint32_t j = math_mt_rand(0, i);
    std::swap(data()[i], data()[j]);
  }
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
  auto vec = req::make<c_Vector>();
  BaseVector::zip(vec.get(), iterable);
  return Object{std::move(vec)};
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
  auto vec = req::make<TVector>();
  uint32_t sz = m_size;
  vec->reserve((size_t)sz + itSize);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  for (uint32_t i = 0; i < sz; ++i) {
    cellDup(data()[i], vec->data()[i]);
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return Object{std::move(vec)};
}

Variant BaseVector::t_firstvalue() {
  if (!m_size) return init_null();
  return tvAsCVarRef(&data()[0]);
}

Variant BaseVector::t_firstkey() {
  if (!m_size) return init_null();
  return 0;
}

Variant BaseVector::t_lastvalue() {
  if (!m_size) return init_null();
  return tvAsCVarRef(&data()[m_size - 1]);
}

Variant BaseVector::t_lastkey() {
  if (!m_size) return init_null();
  return (int64_t)m_size - 1;
}

Object c_Vector::t_set(const Variant& key, const Variant& value) {
  set(key, value);
  return Object{this};
}

Object c_Vector::t_setall(const Variant& iterable) {
  if (iterable.isNull()) return Object{this};
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    set(iter.first(), iter.second());
  }
  return Object{this};
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_fromItems(const Variant& iterable) {
  auto target = req::make<TVector>();
  if (iterable.isNull()) return Object{std::move(target)};
  target->init(iterable);
  return Object{std::move(target)};
}

Object c_Vector::ti_fromitems(const Variant& iterable) {
  return BaseVector::php_fromItems<c_Vector>(iterable);
}

Object c_ImmVector::ti_fromitems(const Variant& iterable) {
  return BaseVector::php_fromItems<c_ImmVector>(iterable);
}

Object c_Vector::ti_fromkeysof(const Variant& container) {
  return BaseVector::php_fromKeysOf<c_Vector>(container);
}

Object c_ImmVector::ti_fromkeysof(const Variant& container) {
  return BaseVector::php_fromKeysOf<c_ImmVector>(container);
}

Object c_Vector::ti_fromarray(const Variant& arr) {
  if (!arr.isArray()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter arr must be an array");
  }
  auto target = req::make<c_Vector>();
  auto* ad = arr.getArrayData();
  uint32_t sz = ad->size();
  target->reserve(sz);
  assert(target->canMutateBuffer());
  target->setSize(sz);
  auto* data = target->data();
  ssize_t pos = ad->iter_begin();
  for (uint32_t i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    assert(pos != ad->iter_end());
    cellDup(*(ad->getValueRef(pos).asCell()), data[i]);
  }
  return Object{std::move(target)};
}

void c_Vector::throwOOB(int64_t key) {
  throwIntOOB(key, true);
}

using VectorValAccessor = TVAccessor;

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
SortFlavor c_Vector::preSort(const AccessorT& acc) {
  assert(m_size > 0);
  uint32_t sz = m_size;
  bool allInts = true;
  bool allStrs = true;
  for (uint32_t i = 0; i < sz; ++i) {
    allInts = (allInts && acc.isInt(data()[i]));
    allStrs = (allStrs && acc.isStr(data()[i]));
  }
  return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
}

#define SORT_CASE(flag, cmp_type, acc_type) \
  case flag: { \
    if (ascending) { \
      cmp_type##Compare<acc_type, flag, true> comp; \
      HPHP::Sort::sort(data(), data() + m_size, comp); \
    } else { \
      cmp_type##Compare<acc_type, flag, false> comp; \
      HPHP::Sort::sort(data(), data() + m_size, comp); \
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
  Sort::sort(data(), data() + m_size, comp);
  return true;
}

void c_Vector::OffsetSet(ObjectData* obj, const TypedValue* key,
                         const TypedValue* val) {
  static_cast<c_Vector*>(obj)->set(key, val);
}

void c_Vector::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  SystemLib::throwRuntimeExceptionObject(
    "Cannot unset an element of a Vector");
}

// This function will create a immutable copy of this Vector (if it doesn't
// already exist) and then return it
Object c_Vector::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto vec = req::make<c_ImmVector>();
    vec->m_arr = m_arr;
    vec->m_size = m_size;
    vec->m_capacity = m_capacity;
    vec->m_version = m_version;
    m_immCopy = std::move(vec);
    arrayData()->incRefCount();
  }
  assert(!m_immCopy.isNull());
  assert(data() == static_cast<c_ImmVector*>(m_immCopy.get())->data());
  assert(arrayData()->hasMultipleRefs());
  return m_immCopy;
}

Object c_Vector::t_tovector() { return Object::attach(c_Vector::Clone(this)); }
Object c_ImmVector::t_tovector() { return materializeImpl<c_Vector>(this); }

Object c_Vector::t_toimmvector() { return getImmutableCopy(); }
Object c_ImmVector::t_toimmvector() { return Object{this}; }

Object BaseVector::t_tomap() { return materializeImpl<c_Map>(this); }

Object BaseVector::t_toimmmap() { return materializeImpl<c_ImmMap>(this); }

Object BaseVector::t_toset() { return materializeImpl<c_Set>(this); }

Object BaseVector::t_toimmset() { return materializeImpl<c_ImmSet>(this); }

Object c_Vector::t_immutable() { return getImmutableCopy(); }
Object c_ImmVector::t_immutable() { return Object{this}; }

///////////////////////////////////////////////////////////////////////////////
// c_ImmVector

// KeyedIterable

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
  auto vec = req::make<c_ImmVector>();
  BaseVector::zip(vec.get(), iterable);
  return Object{std::move(vec)};
}

Object c_ImmVector::t_keys() {
  auto vec = req::make<c_ImmVector>();
  BaseVector::keys(vec.get());
  return Object{std::move(vec)};
}

// Others

Object c_ImmVector::t_values() {
  return Object::attach(BaseVector::Clone<c_ImmVector>(this));
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The HashCollection implementation makes use of s_theEmptyMixedArray, a
 * special static empty array that has all of MixedArray's fields initialized
 * to convenient values.  This "static empty mixed array" is only used
 * internally within the HashCollection implementation and it's never exposed
 * outside of the HashCollection implementation.  hashTab()[0] is set to Empty
 * so that the find() method won't find anything, and m_cap is set to 0 so that
 * any attempt to add an element will trigger a grow operation.
 *
 * Using this static empty mixed array allows us to always assume data() is
 * non-null, and it is better than calling MakeReserveMixed because it avoids
 * doing any allocation.
 */

EmptyMixedArrayStorage s_theEmptyMixedArray;

struct HashCollection::EmptyMixedInitializer {
  EmptyMixedInitializer() {
    auto a = reinterpret_cast<MixedArray*>(&s_theEmptyMixedArray);
    MixedArray::InitSmall(a, StaticValue, 0/*used*/, 0/*nextIntKey*/);
  }
};

HashCollection::EmptyMixedInitializer
HashCollection::s_empty_mixed_initializer;

HashCollection::HashCollection(Class* cls, HeaderKind kind, uint32_t cap)
  : ExtCollectionObjectData(cls, kind)
  , m_versionAndSize(0)
  , m_arr(cap == 0 ? staticEmptyMixedArray() :
          MixedArray::asMixed(MixedArray::MakeReserveMixed(cap)))
{}

Array HashCollection::t_toarray() {
  if (!m_size) {
    return empty_array();
  }
  if (UNLIKELY(intLikeStrKeys())) {
    // In the rare case where this collection contains integer-like string
    // keys, instead of exposing this collection's buffer we return a copy
    // of the buffer with the int-like string keys converted to integers.
    // This is important because int-like string keys cannot be accessed by
    // user code via the brackets operator (i.e. "$c[$k]").
    ArrayInit ai(m_size, ArrayInit::Mixed{});
    auto* eLimit = elmLimit();
    for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
      if (e->hasIntKey()) {
        ai.set((int64_t)e->ikey, tvAsCVarRef(&e->data));
      } else {
        assert(e->hasStrKey());
        ai.set(String(StrNR(e->skey)).toKey(), tvAsCVarRef(&e->data));
      }
    }
    Array arr = ai.toArray();
    // If both a given integer x and its equivalent string presentation
    // were both keys in the collection, we better warn the user.
    if (UNLIKELY(arr.length() < m_size)) warnOnStrIntDup();
    return arr;
  }
  auto ad = const_cast<ArrayData*>(
    reinterpret_cast<const ArrayData*>(arrayData())
  );
  assert(m_size);
  assert(ad->m_pos == nthElmPos(0));
  return Array(ad);
}

void HashCollection::mutateImpl() {
  assert(arrayData()->hasMultipleRefs());
  dropImmCopy();
  if (canMutateBuffer()) {
    return;
  }
  if (!m_size) {
    setIntLikeStrKeys(false);
  }
  auto* oldAd = arrayData();
  m_arr = MixedArray::asMixed(MixedArray::Copy(oldAd));
  assert(oldAd->hasMultipleRefs());
  oldAd->decRefCount();
}

NEVER_INLINE
void HashCollection::throwTooLarge() {
  assert(getClassName().size() == 6);
  auto clsName = getClassName().get()->slice();
  String msg(130, ReserveString);
  auto buf = msg.bufferSlice();
  auto sz = snprintf(buf.data(), buf.size() + 1,
    "%s object has reached its maximum capacity of %u element "
    "slots and does not have room to add a new element",
    clsName.data() + 3, // strip "HH\" prefix
    MaxSize
  );
  msg.setSize(std::min<int>(sz, buf.size()));
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

NEVER_INLINE
void HashCollection::throwReserveTooLarge() {
  assert(getClassName().size() == 6);
  auto clsName = getClassName().get()->slice();
  String msg(80, ReserveString);
  auto buf = msg.bufferSlice();
  auto sz = snprintf(buf.data(), buf.size() + 1,
    "%s does not support reserving room for more than %u elements",
    clsName.data() + 3, // strip "HH\" prefix
    MaxReserveSize
  );
  msg.setSize(std::min<int>(sz, buf.size()));
  SystemLib::throwInvalidOperationExceptionObject(msg);
}

NEVER_INLINE
int32_t* HashCollection::warnUnbalanced(size_t n, int32_t* ei) const {
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    raise_error("%s is too unbalanced (%lu)",
                getClassName().data() + 3, // strip "HH\" prefix
                n);
  }
  return ei;
}

void HashCollection::remove(int64_t key) {
  mutateAndBump();
  auto p = findForRemove(key);
  if (validPos(p)) {
    erase(p);
  }
}

void HashCollection::remove(StringData* key) {
  mutateAndBump();
  auto p = findForRemove(key, key->hash());
  if (validPos(p)) {
    erase(p);
  }
}

bool HashCollection::contains(int64_t key) const {
  return find(key) != Empty;
}

bool HashCollection::contains(StringData* key) const {
  return find(key, key->hash()) != Empty;
}

static bool hitStringKey(const HashCollection::Elm& e,
                         const StringData* s, int32_t hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. HashCollection guarantees that when it adds a hash
  // table entry that it always sets it to refer to a valid element. Likewise
  // when it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!HashCollection::isTombstone(&e));
  return hash == e.hash() && (s == e.skey || s->same(e.skey));
}

static bool hitIntKey(const HashCollection::Elm& e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. HashCollection guarantees that when it adds a hash
  // table entry that it always sets it to refer to a valid element. Likewise
  // when it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!HashCollection::isTombstone(&e));
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
ssize_t HashCollection::findImpl(size_t h0, Hit hit) const {
  uint32_t mask = tableMask();
  auto elms = data();
  auto hashtable = hashTab();
  for (uint32_t probeIndex = h0, i = 1;; ++i) {
    auto pos = hashtable[probeIndex & mask];
    if (validPos(pos)) {
      if (hit(elms[pos])) return pos;
    } else if (pos & 1) {
      assert(pos == Empty);
      return pos;
    }
    probeIndex += i;
    assertx(i <= mask);
    assertx(probeIndex == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

ssize_t HashCollection::find(int64_t ki) const {
  return findImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

ssize_t
HashCollection::find(const StringData* s, strhash_t h) const {
  return findImpl(h, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

template <class Hit>
ALWAYS_INLINE
int32_t* HashCollection::findForInsertImpl(size_t h0, Hit hit) const {
  uint32_t mask = tableMask();
  auto elms = data();
  auto hashtable = hashTab();
  int32_t* ret = nullptr;
  for (uint32_t probe = h0, i = 1;; ++i) {
    auto ei = &hashtable[probe & mask];
    ssize_t pos = *ei;
    if (validPos(pos)) {
      if (hit(elms[pos])) {
        return ei;
      }
    } else {
      if (!ret) ret = ei;
      if (pos & 1) {
        assert(pos == Empty);
        return LIKELY(i <= 100) ? ret : warnUnbalanced(i, ret);
      }
    }
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

int32_t* HashCollection::findForInsert(int64_t ki) const {
  return findForInsertImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

int32_t* HashCollection::findForInsert(const StringData* s, strhash_t h) const {
  return findForInsertImpl(h, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

// findForNewInsert() is only safe to use if you know for sure that the
// key is not already present in the HashCollection.
ALWAYS_INLINE int32_t* HashCollection::findForNewInsert(
  int32_t* table, size_t mask, size_t h0) const {
  for (uint32_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) {
      return ei;
    }
    probe += i;
    assertx(i <= mask);
    assertx(probe == static_cast<uint32_t>(h0) + (i + i * i) / 2);
  }
}

ALWAYS_INLINE
int32_t* HashCollection::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), tableMask(), h0);
}

void HashCollection::eraseNoCompact(ssize_t pos) {
  assert(canMutateBuffer());
  assert(validPos(pos) && !isTombstone(pos));
  assert(m_size > 0);
  arrayData()->eraseNoCompact(pos);
  --m_size;
}

NEVER_INLINE void HashCollection::makeRoom() {
  assert(isFull());
  assert(posLimit() == cap());
  if (LIKELY(!isDensityTooLow())) {
    if (UNLIKELY(cap() == MaxSize)) {
      throwTooLarge();
    }
    assertx(scale() > 0);
    grow(scale() * 2);
  } else {
    compact();
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isFull());
}

NEVER_INLINE void HashCollection::reserve(int64_t sz) {
  assert(m_size <= posLimit() && posLimit() <= cap());
  auto cap = static_cast<int64_t>(this->cap());
  if (LIKELY(sz > cap)) {
    if (UNLIKELY(sz > int64_t(MaxReserveSize))) {
      throwReserveTooLarge();
    }
    // Fast path: The requested capacity is greater than the current capacity.
    // Grow to the smallest allowed capacity that is sufficient.
    grow(computeScaleFromSize(sz));
    assert(canMutateBuffer());
    return;
  }
  if (LIKELY(!hasTombstones())) {
    // Fast path: There are no tombstones and the requested capacity is less
    // than or equal to the current capacity.
    mutate();
    return;
  }
  if (sz + int64_t(posLimit() - m_size) <= cap || isDensityTooLow()) {
    // If we reach this case, then either (1) density is too low (this is
    // possible because of methods like retain()), in which case we compact
    // to make room and return, OR (2) density is not too low and either
    // sz < m_size or there's enough room to add sz-m_size elements, in
    // which case we do nothing and return.
    compactOrShrinkIfDensityTooLow();
    assert(sz + int64_t(posLimit() - m_size) <= cap);
    mutate();
    return;
  }
  // If we reach this case, then density is not too low and sz > m_size and
  // there is not enough room to add sz-m_size elements. While would could
  // compact to make room, it's better for Hysteresis if we grow capacity
  // by 2x instead.
  assert(!isDensityTooLow());
  assert(sz + int64_t(posLimit() - m_size) > cap);
  assert(cap < MaxSize && tableMask() != 0);
  auto newScale = scale() * 2;
  assert(sz > 0 && MixedArray::Capacity(newScale) >= sz);
  grow(newScale);
  assert(canMutateBuffer());
}

ALWAYS_INLINE
void HashCollection::resizeHelper(uint32_t newCap) {
  assert(newCap >= m_size);
  assert(m_immCopy.isNull());
  // Allocate a new ArrayData with the specified capacity and dup
  // all the elements (without copying over tombstones).
  auto ad = arrayData() == staticEmptyMixedArray() ?
    MixedArray::asMixed(MixedArray::MakeReserveMixed(newCap)) :
    MixedArray::CopyReserve(m_arr, newCap);
  decRefArr(m_arr);
  m_arr = ad;
  assert(canMutateBuffer());
}

void HashCollection::grow(uint32_t newScale) {
  auto newCap = MixedArray::Capacity(newScale);
  assert(m_size <= posLimit() && posLimit() <= cap() && cap() <= newCap);
  assert(SmallSize <= newCap && newCap <= MaxSize);
  assert(m_size <= newCap);
  auto oldAd = arrayData();
  dropImmCopy();
  if (m_size > 0 && !oldAd->cowCheck()) {
    // MixedArray::Grow can only handle non-empty cases where the
    // buffer's refcount is 1.
    m_arr = MixedArray::Grow(oldAd, newScale);
    decRefArr(oldAd);
  } else {
    // For cases where m_size is zero or the buffer's refcount is
    // greater than 1, call resizeHelper().
    resizeHelper(newCap);
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
}

void HashCollection::compact() {
  assert(isDensityTooLow());
  dropImmCopy();
  if (!arrayData()->cowCheck()) {
    // MixedArray::compact can only handle cases where the buffer's
    // refcount is 1.
    arrayData()->compact(false);
  } else {
    // For cases where the buffer's refcount is greater than 1, call
    // resizeHelper().
    resizeHelper(cap());
  }
  assert(canMutateBuffer());
  assert(m_immCopy.isNull());
  assert(!isDensityTooLow());
}

void HashCollection::shrink(uint32_t oldCap /* = 0 */) {
  assert(isCapacityTooHigh() && (oldCap == 0 || oldCap < cap()));
  assert(m_size <= posLimit() && posLimit() <= cap());
  dropImmCopy();
  uint32_t newCap;
  if (oldCap != 0) {
    // If an old capacity was specified, use that
    newCap = oldCap;
    // .. unless the old capacity is too small, in which case we use the
    // smallest capacity that is large enough to hold the current number
    // of elements.
    for (; newCap < m_size; newCap <<= 1) {}
    assert(newCap == computeMaxElms(folly::nextPowTwo<uint64_t>(newCap) - 1));
  } else {
    if (m_size == 0 && nextKI() == 0) {
      decRefArr(m_arr);
      m_arr = staticEmptyMixedArray();
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
  if (!oldAd->cowCheck()) {
    // If the buffer's refcount is 1, we can teleport the elements
    // to a new buffer
    auto oldBuf = data();
    auto oldUsed = posLimit();
    auto oldNextKI = nextKI();
    auto arr = MixedArray::asMixed(MixedArray::MakeReserveMixed(newCap));
    auto data = mixedData(arr);
    m_arr = arr;
    auto table = (int32_t*)(data + size_t(newCap));
    m_arr->m_size = m_size;
    setPosLimit(m_size);
    setNextKI(oldNextKI);
    for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
      frPos = skipTombstonesNoBoundsCheck(frPos, oldUsed, oldBuf);
      copyElm(oldBuf[frPos], data[toPos]);
      *findForNewInsert(table, tableMask(), data[toPos].probe()) = toPos;
    }
    oldAd->setZombie();
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

HashCollection::Elm& HashCollection::allocElmFront(int32_t* ei) {
  assert(ei && !validPos(*ei) && m_size <= posLimit() && posLimit() < cap());
  // Move the existing elements to make element slot 0 available.
  memmove(data() + 1, data(), posLimit() * sizeof(Elm));
  incPosLimit();
  // Update the hashtable to reflect the fact that everything was
  // moved over one position
  auto* hash = hashTab();
  auto* hashEnd = hash + hashSize();
  for (; hash != hashEnd; ++hash) {
    if (validPos(*hash)) {
      ++(*hash);
    }
  }
  // Set the hash entry we found to point to element slot 0.
  (*ei) = 0;
  // Adjust m_pos so that is points at this new first element.
  arrayData()->m_pos = 0;
  // Adjust size to reflect that we're adding a new element.
  incSize();
  // Store the value into element slot 0.
  return data()[0];
}

///////////////////////////////////////////////////////////////////////////////

// Protected (Internal)

BaseMap::~BaseMap() {
  auto const mixed = MixedArray::asMixed(arrayData());
  // Avoid indirect call, as we know it is a MixedArray
  if (mixed->decReleaseCheck()) MixedArray::Release(mixed);
}

void BaseMap::t___construct(const Variant& iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
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
  target->m_arr = thiz->m_arr;
  target->setIntLikeStrKeys(thiz->intLikeStrKeys());
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
  return Object{this};
}

Object c_Map::t_addall(const Variant& iterable) {
  if (iterable.isNull()) return Object{this};
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto oldCap = cap();
  reserve(m_size + sz); // presume minimum key collisions ...
  for (; iter; ++iter) {
    add(iter.second());
  }
  shrinkIfCapacityTooHigh(oldCap); // ... and shrink back if that was incorrect
  return Object{this};
}

void HashCollection::t_reserve(const Variant& sz) {
  if (UNLIKELY(!sz.isInteger())) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter sz must be a non-negative integer");
  }
  int64_t intSz = sz.toInt64();
  if (UNLIKELY(intSz < 0)) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter sz must be a non-negative integer");
  }
  reserve(intSz); // checks for intSz > MaxReserveSize
}

Object c_Map::t_clear() {
  ++m_version;
  dropImmCopy();
  decRefArr(arrayData());
  m_arr = staticEmptyMixedArray();
  m_size = 0;
  setIntLikeStrKeys(false);
  return Object{this};
}

bool HashCollection::t_isempty() {
  return size() == 0;
}

int64_t HashCollection::t_count() {
  return size();
}

Object HashCollection::t_lazy() {
  return SystemLib::AllocLazyKeyedIterableViewObject(Variant{this});
}

Object BaseMap::t_items() {
  return SystemLib::AllocLazyKVZipIterableObject(Variant{this});
}

Variant BaseMap::t_at(const Variant& key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  } else if (key.isString()) {
    return tvAsCVarRef(at(key.getStringData()));
  }
  throwBadKeyType();
  return init_null();
}

Variant BaseMap::t_get(const Variant& key) {
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

Object c_Map::t_set(const Variant& key, const Variant& value) {
  set(key, value);
  return Object{this};
}

Object c_Map::t_setall(const Variant& iterable) {
  // TODO Task# 4324040: Refactor this logic with init()
  if (iterable.isNull()) return Object{this};
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    set(iter.first(), iter.second());
  }
  return Object{this};
}

bool BaseMap::t_contains(const Variant& key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    return contains(key.toInt64());
  }
  if (isStringType(t)) {
    return contains(key.getStringData());
  }
  throwBadKeyType();
  return false;
}

bool BaseMap::t_containskey(const Variant& key) { return t_contains(key); }

Object c_Map::t_remove(const Variant& key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    remove(key.toInt64());
  } else if (isStringType(t)) {
    remove(key.getStringData());
  } else {
    throwBadKeyType();
  }
  return Object{this};
}

Object c_Map::t_removekey(const Variant& key) { return t_remove(key); }

Object c_Map::t_values() {
  return BaseMap::php_values<c_Vector>();
}

Object c_Map::t_keys() {
  return BaseMap::php_keys<c_Vector>();
}

Array HashCollection::t_tokeysarray() {
  PackedArrayInit ai(m_size);
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasIntKey()) {
      ai.append(int64_t{e->ikey});
    } else {
      assert(e->hasStrKey());
      ai.append(VarNR(e->skey));
    }
  }
  return ai.toArray();
}

Array HashCollection::t_tovaluesarray() {
  PackedArrayInit ai(m_size);
  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    ai.append(tvAsCVarRef(&e->data));
  }
  return ai.toArray();
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_differenceByKey(const Variant& it) {
  if (!it.isObject()) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter it must be an instance of Iterable");
  }
  ObjectData* obj = it.getObjectData();
  TMap* target = BaseMap::Clone<TMap>(this);
  auto ret = Object::attach(target);
  if (obj->isCollection()) {
    if (isMapCollection(obj->collectionType())) {
      auto map = static_cast<BaseMap*>(obj);
      auto* eLimit = map->elmLimit();
      for (auto* e = map->firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
        if (e->hasIntKey()) {
          target->remove((int64_t)e->ikey);
        } else {
          assert(e->hasStrKey());
          target->remove(e->skey);
        }
      }
      return ret;
    }
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

Object BaseMap::t_getiterator() {
  auto iter = collections::MapIterator::newInstance();
  Native::data<collections::MapIterator>(iter)->setMap(this);
  return iter;
}

ALWAYS_INLINE static std::array<TypedValue, 2>
makeArgsFromHashKeyAndValue(const HashCollection::Elm& e) {
  return std::array<TypedValue, 2> {{
    (e.hasIntKey()
      ? make_tv<KindOfInt64>(e.ikey)
      : make_tv<KindOfString>(e.skey)),
    e.data
  }};
}

ALWAYS_INLINE static std::array<TypedValue, 1>
makeArgsFromHashValue(const HashCollection::Elm& e) {
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
  VMRegAnchor _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto map = req::make<TMap>();
  if (!m_size) return Object{std::move(map)};
  assert(posLimit() != 0);
  assert(hashSize() > 0);
  assert(map->arrayData() == staticEmptyMixedArray());
  map->m_arr = MixedArray::asMixed(MixedArray::MakeReserveMixed(cap()));
  map->setIntLikeStrKeys(intLikeStrKeys());
  wordcpy(map->hashTab(), hashTab(), hashSize());
  {
    uint32_t used = posLimit();
    int32_t version = m_version;
    uint32_t i = 0;
    // When the loop below finishes or when an exception is thrown,
    // make sure that posLimit() get set to the correct value and
    // that m_pos gets set to point to the first element.
    SCOPE_EXIT {
      map->setPosLimit(i);
      map->arrayData()->m_pos = map->nthElmPos(0);
    };
    for (; i < used; ++i) {
      const Elm& e = data()[i];
      Elm& ne = map->data()[i];
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
      map->incSize();
      // Needed so that the new elements are accounted for when GC scanning.
      map->incPosLimit();
    }
  }
  return Object{std::move(map)};
}

Object c_ImmMap::t_values() {
  return BaseMap::php_values<c_ImmVector>();
}

Object c_ImmMap::t_keys() {
  return BaseMap::php_keys<c_ImmVector>();
}

Object c_ImmMap::t_map(const Variant& callback) {
  return php_map<c_ImmMap>(callback, &makeArgsFromHashValue);
}

Object c_Map::t_map(const Variant& callback) {
  return php_map<c_Map>(callback, &makeArgsFromHashValue);
}

Object c_ImmMap::t_mapwithkey(const Variant& callback) {
  return php_map<c_ImmMap>(callback, &makeArgsFromHashKeyAndValue);
}

Object c_Map::t_mapwithkey(const Variant& callback) {
  return php_map<c_Map>(callback, &makeArgsFromHashKeyAndValue);
}

template<typename TMap, class MakeArgs>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_filter(const Variant& callback, MakeArgs makeArgs) const {
  VMRegAnchor _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto map = req::make<TMap>();
  if (!m_size) return Object(std::move(map));
  map->mutate();
  int32_t version = m_version;
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto* e = iter_elm(pos);
    auto args = makeArgs(*e);
    bool b = invokeAndCastToBool(ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!b) continue;
    e = iter_elm(pos);
    if (e->hasIntKey()) {
      map->setRaw(e->ikey, &e->data);
    } else {
      assert(e->hasStrKey());
      map->setRaw(e->skey, &e->data);
    }
  }
  return Object(std::move(map));
}

Object c_ImmMap::t_filter(const Variant& callback) {
  return php_filter<c_ImmMap>(callback, &makeArgsFromHashValue);
}

Object c_Map::t_filter(const Variant& callback) {
  return php_filter<c_Map>(callback, &makeArgsFromHashValue);
}

Object c_ImmMap::t_filterwithkey(const Variant& callback) {
  return php_filter<c_ImmMap>(callback, &makeArgsFromHashKeyAndValue);
}

Object c_Map::t_filterwithkey(const Variant& callback) {
  return php_filter<c_Map>(callback, &makeArgsFromHashKeyAndValue);
}

template<class MakeArgs>
Object BaseMap::php_retain(const Variant& callback, MakeArgs makeArgs) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto size = m_size;
  if (!size) { return Object{this}; }
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto* e = iter_elm(pos);
    int32_t version = m_version;
    auto args = makeArgs(*e);
    bool b = invokeAndCastToBool(ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) {
      continue;
    }
    mutateAndBump();
    version = m_version;
    e = iter_elm(pos);
    ssize_t pp = (e->hasIntKey()
                   ? findForRemove(e->ikey)
                   : findForRemove(e->skey, e->skey->hash()));
    eraseNoCompact(pp);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
  }

  assert(m_size <= size);
  compactOrShrinkIfDensityTooLow();
  return Object{this};
}

Object c_Map::t_retain(const Variant& callback) {
  return php_retain(callback, &makeArgsFromHashValue);
}

Object c_Map::t_retainwithkey(const Variant& callback) {
  return php_retain(callback, &makeArgsFromHashKeyAndValue);
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_zip(const Variant& iterable) const {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto map = req::make<TMap>();
  if (!m_size) {
    return Object{std::move(map)};
  }
  map->reserve(std::min(sz, size_t(m_size)));
  uint32_t used = posLimit();
  for (uint32_t i = 0; i < used && iter; ++i) {
    if (isTombstone(i)) continue;
    const Elm& e = data()[i];
    Variant v = iter.second();
    auto pair = req::make<c_Pair>(c_Pair::NoInit{});
    pair->initAdd(&e.data);
    pair->initAdd(v);
    TypedValue tv;
    tv.m_data.pobj = pair.detach();
    tv.m_type = KindOfObject;
    if (e.hasIntKey()) {
      map->setRaw(e.ikey, &tv);
    } else {
      assert(e.hasStrKey());
      map->setRaw(e.skey, &tv);
    }
    ++iter;
  }
  return Object{std::move(map)};
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
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len >= int64_t(m_size)) {
    // We know the resulting Map will simply be a copy of this Map,
    // so we can just call Clone() and return early here.
    return Object::attach(TMap::Clone(this));
  }
  auto map = req::make<TMap>();
  if (len <= 0) {
    // We know the resulting Map will be empty, so we can return
    // early here.
    return Object{std::move(map)};
  }
  size_t sz = size_t(len);
  map->reserve(sz);
  map->setSize(sz);
  map->setPosLimit(sz);
  auto table = map->hashTab();
  auto mask = map->tableMask();
  for (uint32_t frPos = 0, toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = map->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      map->updateNextKI(toE.ikey);
    } else {
      assert(toE.hasStrKey());
      map->updateIntLikeStrKeys(toE.skey);
    }
  }
  return Object{std::move(map)};
}

template<class TMap, bool checkVersion>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto map = req::make<TMap>();
  if (!m_size) return Object{std::move(map)};
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto* e = iter_elm(pos);
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) continue;
    e = iter_elm(pos);
    if (e->hasIntKey()) {
      map->set(e->ikey, &e->data);
    } else {
      assert(e->hasStrKey());
      map->set(e->skey, &e->data);
    }
  }
  return Object{std::move(map)};
}

template<class TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len <= 0) {
    // We know the resulting Map will simply be a copy of this Map,
    // so we can just call Clone() and return early here.
    return Object::attach(TMap::Clone(this));
  }
  auto map = req::make<TMap>();
  if (len >= m_size) {
    // We know the resulting Map will be empty, so we can return
    // early here.
    return Object{std::move(map)};
  }
  size_t sz = size_t(m_size) - size_t(len);
  assert(sz);
  map->reserve(sz);
  map->setSize(sz);
  map->setPosLimit(sz);
  uint32_t frPos = nthElmPos(len);
  auto table = map->hashTab();
  auto mask = map->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = map->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      map->updateNextKI(toE.ikey);
    } else {
      assert(toE.hasStrKey());
      map->updateIntLikeStrKeys(toE.skey);
    }
  }
  return Object{std::move(map)};
}

template<class TMap, bool checkVersion>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto map = req::make<TMap>();
  if (!m_size) return Object{std::move(map)};
  int32_t version;
  if (checkVersion) {
    version = m_version;
  }
  ssize_t pos;
  for (pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto* e = iter_elm(pos);
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (checkVersion) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
  }
  if (iter_valid(pos)) {
    auto* eLimit = elmLimit();
    auto* e = iter_elm(pos);
    for (; e != eLimit; e = nextElm(e, eLimit)) {
      if (e->hasIntKey()) {
        map->set(e->ikey, &e->data);
      } else {
        assert(e->hasStrKey());
        map->set(e->skey, &e->data);
      }
    }
  }
  return Object{std::move(map)};
}

template<class TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer");
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer");
  }
  size_t skipAmt = std::min<size_t>(istart, m_size);
  size_t sz = std::min<size_t>(ilen, size_t(m_size) - skipAmt);
  auto map = req::make<TMap>();
  map->reserve(sz);
  map->setSize(sz);
  map->setPosLimit(sz);
  uint32_t frPos = nthElmPos(skipAmt);
  auto table = map->hashTab();
  auto mask = map->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = map->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      map->updateNextKI(toE.ikey);
    } else {
      assert(toE.hasStrKey());
      map->updateIntLikeStrKeys(toE.skey);
    }
  }
  return Object{std::move(map)};
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
  auto vec = req::make<TVector>();
  uint32_t sz = m_size;
  vec->reserve((size_t)sz + itSize);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);
  uint32_t used = posLimit();
  for (uint32_t i = 0, j = 0; i < used; ++i) {
    if (isTombstone(i)) {
      continue;
    }
    cellDup(data()[i].data, vec->data()[j]);
    ++j;
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return Object{std::move(vec)};
}

Variant BaseMap::t_firstvalue() {
  if (!m_size) return init_null();
  auto* e = firstElm();
  assert(e != elmLimit());
  return tvAsCVarRef(&e->data);
}

Variant BaseMap::t_firstkey() {
  if (!m_size) return init_null();
  auto* e = firstElm();
  assert(e != elmLimit());
  if (e->hasIntKey()) {
    return e->ikey;
  } else {
    assert(e->hasStrKey());
    return Variant{e->skey};
  }
}

Variant BaseMap::t_lastvalue() {
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
  return tvAsCVarRef(&data()[pos].data);
}

Variant BaseMap::t_lastkey() {
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
  if (data()[pos].hasIntKey()) {
    return data()[pos].ikey;
  }
  assert(data()[pos].hasStrKey());
  return Variant{data()[pos].skey};
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_mapFromItems(const Variant& iterable) {
  if (iterable.isNull()) return Object{req::make<TMap>()};
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto target = req::make<TMap>();
  target->reserve(sz);
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = v.asCell();
    if (UNLIKELY(tv->m_type != KindOfObject ||
                 tv->m_data.pobj->getVMClass() != c_Pair::classof())) {
      SystemLib::throwInvalidArgumentExceptionObject(
                 "Parameter must be an instance of Iterable<Pair>");
    }
    auto pair = static_cast<c_Pair*>(tv->m_data.pobj);
    target->setRaw(&pair->elm0, &pair->elm1);
  }
  return Object{std::move(target)};
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
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter arr must be an array");
  }
  auto map = req::make<TMap>();
  ArrayData* ad = arr.getArrayData();
  map->reserve(ad->size());
  for (ssize_t pos = ad->iter_begin(), limit = ad->iter_end(); pos != limit;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    auto* tv = ad->getValueRef(pos).asCell();
    if (k.isInteger()) {
      map->setRaw(k.toInt64(), tv);
    } else {
      assert(k.isString());
      map->setRaw(k.getStringData(), tv);
    }
  }
  return Object(std::move(map));
}

Object c_Map::ti_fromarray(const Variant& arr) {
  return php_mapFromArray<c_Map>(arr);
}

NEVER_INLINE
void BaseMap::throwOOB(int64_t key) {
  throwIntOOB(key);
}

NEVER_INLINE
void BaseMap::throwOOB(StringData* key) {
  throwStrOOB(key);
}

TypedValue* BaseMap::at(int64_t key) const {
  auto p = find(key);
  if (UNLIKELY(p == Empty)) {
    throwOOB(key);
  }
  return const_cast<TypedValue*>(
    static_cast<const TypedValue*>(&(data()[p].data))
  );
}

TypedValue* BaseMap::at(StringData* key) const {
  auto p = find(key, key->hash());
  if (UNLIKELY(p == Empty)) {
    throwOOB(key);
  }
  return const_cast<TypedValue*>(
    static_cast<const TypedValue*>(&(data()[p].data))
  );
}

TypedValue* BaseMap::get(int64_t key) const {
  auto p = find(key);
  if (p == Empty) {
    return nullptr;
  }
  return const_cast<TypedValue*>(
    static_cast<const TypedValue*>(&(data()[p].data))
  );
}

TypedValue* BaseMap::get(StringData* key) const {
  auto p = find(key, key->hash());
  if (p == Empty) {
    return nullptr;
  }
  return const_cast<TypedValue*>(
    static_cast<const TypedValue*>(&(data()[p].data))
  );
}

void BaseMap::add(const TypedValue* val) {
  if (UNLIKELY(val->m_type != KindOfObject ||
               val->m_data.pobj->getVMClass() != c_Pair::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be an instance of Pair");
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
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot pop empty Map");
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
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot pop empty Map");
  }
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
  auto p = findForInsert(h);
  assert(p);
  if (validPos(*p)) {
    auto& e = data()[*p];
    TypedValue old = e.data;
    cellDup(*val, e.data);
    tvRefcountedDecRef(old);
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
    auto& e = data()[*p];
    TypedValue old = e.data;
    cellDup(*val, e.data);
    tvRefcountedDecRef(old);
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

void BaseMap::throwBadKeyType() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with Maps");
}

Array BaseMap::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return const_cast<BaseMap*>(
    static_cast<const BaseMap*>(obj)
  )->t_toarray();
}

bool BaseMap::ToBool(const ObjectData* obj) {
  return static_cast<const BaseMap*>(obj)->toBoolImpl();
}

/**
 * preSort() does an initial pass to do some preparatory work before the
 * sort algorithm runs. For sorts that use builtin comparators, the types
 * of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized
 * comparator and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
SortFlavor HashCollection::preSort(const AccessorT& acc, bool checkTypes) {
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
  // The logic above possibly moved elements and tombstones around
  // within the buffer, so we make sure m_pos is not pointing at
  // garbage by resetting it. The logic above ensures that the first
  // slot is not a tombstone, so it's safe to set m_pos to 0.
  arrayData()->m_pos = 0;
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
void HashCollection::postSort() {  // Must provide the nothrow guarantee
  arrayData()->postSort(false);
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

void HashCollection::asort(int sort_flags, bool ascending) {
  if (m_size <= 1) return;
  mutateAndBump();
  SORT_BODY(AssocValAccessor<HashCollection::Elm>);
}

void HashCollection::ksort(int sort_flags, bool ascending) {
  if (m_size <= 1) return;
  mutateAndBump();
  SORT_BODY(AssocKeyAccessor<HashCollection::Elm>);
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

bool HashCollection::uasort(const Variant& cmp_function) {
  if (m_size <= 1) return true;
  mutateAndBump();
  USER_SORT_BODY(AssocValAccessor<HashCollection::Elm>);
}

bool HashCollection::uksort(const Variant& cmp_function) {
  if (m_size <= 1) return true;
  mutateAndBump();
  USER_SORT_BODY(AssocKeyAccessor<HashCollection::Elm>);
}

#undef USER_SORT_BODY

void BaseMap::OffsetSet(ObjectData* obj, const TypedValue* key,
                        const TypedValue* val) {
  static_cast<BaseMap*>(obj)->set(key, val);
}

bool BaseMap::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto map = static_cast<BaseMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = map->get(key->m_data.num);
  } else if (isStringType(key->m_type)) {
    result = map->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellIsNull(result) : false;
}

bool BaseMap::OffsetEmpty(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto map = static_cast<BaseMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = map->get(key->m_data.num);
  } else if (isStringType(key->m_type)) {
    result = map->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellToBool(*result) : true;
}

bool BaseMap::OffsetContains(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto map = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    return map->contains(key->m_data.num);
  } else if (isStringType(key->m_type)) {
    return map->contains(key->m_data.pstr);
  } else {
    throwBadKeyType();
    return false;
  }
}

void BaseMap::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto map = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    map->remove(key->m_data.num);
    return;
  }
  if (isStringType(key->m_type)) {
    map->remove(key->m_data.pstr);
    return;
  }
  throwBadKeyType();
}

// This function will create a immutable copy of this Map (if it doesn't
// already exist) and then return it
Object c_Map::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto map = req::make<c_ImmMap>();
    map->m_size = m_size;
    map->m_version = m_version;
    map->m_arr = m_arr;
    map->setIntLikeStrKeys(intLikeStrKeys());
    m_immCopy = std::move(map);
    arrayData()->incRefCount();
  }
  assert(!m_immCopy.isNull());
  assert(data() == static_cast<c_ImmMap*>(m_immCopy.get())->data());
  assert(arrayData()->hasMultipleRefs());
  return m_immCopy;
}

bool BaseMap::Equals(EqualityFlavor eq,
                     const ObjectData* obj1, const ObjectData* obj2) {

  auto map1 = static_cast<const BaseMap*>(obj1);
  auto map2 = static_cast<const BaseMap*>(obj2);
  auto size = map1->size();

  if (size != map2->size()) { return false; }
  if (size == 0) { return true; }

  switch (eq) {
    case EqualityFlavor::OrderIrrelevant: {
      // obj1 and obj2 must have the exact same set of keys, and the values
      // for each key must compare equal (==). This equality behavior
      // matches that of == on two PHP (associative) arrays.
      for (uint32_t i = 0; i < map1->posLimit(); ++i) {
        if (map1->isTombstone(i)) continue;
        const HashCollection::Elm& e = map1->data()[i];
        TypedValue* tv2;
        if (e.hasIntKey()) {
          tv2 = map2->get(e.ikey);
        } else {
          assert(e.hasStrKey());
          tv2 = map2->get(e.skey);
        }
        if (!tv2) return false;
        if (!HPHP::equal(tvAsCVarRef(&e.data), tvAsCVarRef(tv2))) return false;
      }
      return true;
    }
    case EqualityFlavor::OrderMatters: {
      // obj1 and obj2 must compare equal according to OrderIrrelevant;
      // additionally, the (identical) keys of obj1 and obj2 must be in the
      // same iteration order.
      uint32_t compared = 0;
      for (uint32_t ix1 = 0, ix2 = 0;
           ix1 < map1->posLimit() && ix2 < map2->posLimit() ; ) {

        auto tomb1 = map1->isTombstone(ix1);
        auto tomb2 = map2->isTombstone(ix2);

        if (tomb1 || tomb2) {
          if (tomb1) { ++ix1; }
          if (tomb2) { ++ix2; }
          continue;
        }

        const HashCollection::Elm& e1 = map1->data()[ix1];
        const HashCollection::Elm& e2 = map2->data()[ix2];

        if (e1.hasIntKey()) {
          if (!e2.hasIntKey() ||
              e1.ikey != e2.ikey) {
            return false;
          }
        } else {
          assert(e1.hasStrKey());
          if (!e2.hasStrKey() || !HPHP::equal(e1.skey, e2.skey)) {
            return false;
          }
        }
        if (!HPHP::equal(tvAsCVarRef(&e1.data), tvAsCVarRef(&e2.data))) {
          return false;
        }

        ++ix1; ++ix2; ++compared;
      }

      return (compared == size);
    }
  }
  not_reached();
}

Object BaseMap::t_tovector() { return materializeImpl<c_Vector>(this); }

Object BaseMap::t_toimmvector() { return materializeImpl<c_ImmVector>(this); }

Object c_Map::t_tomap() { return Object::attach(c_Map::Clone(this)); }
Object c_ImmMap::t_tomap() { return materializeImpl<c_Map>(this); }

Object c_Map::t_toimmmap() { return getImmutableCopy(); }
Object c_ImmMap::t_toimmmap() { return Object{this}; }

Object BaseMap::t_toset() { return materializeImpl<c_Set>(this); }

Object BaseMap::t_toimmset() { return materializeImpl<c_ImmSet>(this); }

Object c_Map::t_immutable() { return getImmutableCopy(); }
Object c_ImmMap::t_immutable() { return Object{this}; }

///////////////////////////////////////////////////////////////////////////////

c_ImmMap* c_ImmMap::Clone(ObjectData* obj) {
  return BaseMap::Clone<c_ImmMap>(obj);
}

///////////////////////////////////////////////////////////////////////////////
// BaseSet

// Public

void BaseSet::addAllKeysOf(const Cell container) {
  assert(isContainer(container));

  auto sz = getContainerSize(container);
  ArrayIter iter(container);
  if (!sz || !iter) { return; }

  mutateAndBump();
  // In theory we could be deferring the version bump above because all the
  // elements of iter could already be present in the set.
  auto oldCap = cap();
  reserve(m_size + sz); // presume minimum collisions ...
  for (; iter; ++iter) { addRaw(iter.first()); }
  shrinkIfCapacityTooHigh(oldCap); // ... and shrink back if that was incorrect
}

void BaseSet::addAll(const Variant& t) {
  if (t.isNull()) { return; } // nothing to do

  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  if (!iter) { return; }

  mutateAndBump();
  assert(canMutateBuffer());
  // In theory we could be deferring the version bump above for the
  // container case because all the elements of iter could already be
  // present in the set: there's no destructor invocations because Sets are
  // int/string only. We can save m_size and version bump after the
  // insertion loop only if the size has increased. However, as presently
  // constituted, such an ->addAll() call is a time bomb and a no-op,
  if (LIKELY(!iter.hasIteratorObj())) {
    auto oldCap = cap();
    reserve(m_size + sz); // presume minimum collisions ...
    do {
      addRaw(iter.secondRefPlus());
      ++iter;
    } while (iter);
    // ... and shrink back if that was incorrect
    shrinkIfCapacityTooHigh(oldCap);
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
  e.setIntKey(h);
  e.data.m_type = KindOfInt64;
  e.data.m_data.num = h;
  updateNextKI(h);
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
  // This increments the string's refcount twice, once for
  // the key and once for the value
  e.setStrKey(key, h);
  cellDup(make_tv<KindOfString>(key), e.data);
  updateIntLikeStrKeys(key);
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
  e.setIntKey(h);
  e.data.m_type = KindOfInt64;
  e.data.m_data.num = h;
  updateNextKI(h);
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
  // This increments the string's refcount twice, once for
  // the key and once for the value
  e.setStrKey(key, h);
  cellDup(make_tv<KindOfString>(key), e.data);
  updateIntLikeStrKeys(key);
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
    ssize_t ei;
    if (e->hasIntKey()) {
      ei = findForRemove(e->data.m_data.num);
    } else {
      assert(e->hasStrKey());
      auto* key = e->data.m_data.pstr;
      ei = findForRemove(key, key->hash());
    }
    erase(ei);
    return ret;
  } else {
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot pop empty Set");
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
    ssize_t ei;
    if (e->hasIntKey()) {
      ei = findForRemove(e->data.m_data.num);
    } else {
      assert(e->hasStrKey());
      auto* key = e->data.m_data.pstr;
      ei = findForRemove(key, key->hash());
    }
    erase(ei);
    return ret;
  } else {
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot pop empty Set");
  }
}

void BaseSet::throwOOB(int64_t val) {
  throwIntOOB(val);
}

void BaseSet::throwOOB(StringData* val) {
  throwStrOOB(val);
}

void BaseSet::throwNoMutableIndexAccess() {
  SystemLib::throwInvalidOperationExceptionObject(
    "[] operator cannot be used to modify elements of a Set");
}

Array BaseSet::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return const_cast<BaseSet*>(
    static_cast<const BaseSet*>(obj)
  )->t_toarray();
}

bool BaseSet::ToBool(const ObjectData* obj) {
  return static_cast<const BaseSet*>(obj)->toBoolImpl();
}

// This function will create a immutable copy of this Set (if it doesn't
// already exist) and then return it
Object c_Set::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto set = req::make<c_ImmSet>();
    set->m_size = m_size;
    set->m_version = m_version;
    set->m_arr = m_arr;
    set->setIntLikeStrKeys(intLikeStrKeys());
    m_immCopy = std::move(set);
    arrayData()->incRefCount();
  }
  assert(!m_immCopy.isNull());
  assert(data() == static_cast<c_ImmSet*>(m_immCopy.get())->data());
  assert(arrayData()->hasMultipleRefs());
  return m_immCopy;
}

bool BaseSet::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto st1 = static_cast<const BaseSet*>(obj1);
  auto st2 = static_cast<const BaseSet*>(obj2);
  if (st1->m_size != st2->m_size) return false;

  auto* eLimit = st1->elmLimit();
  for (auto* e = st1->firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    if (e->hasIntKey()) {
      if (!st2->contains(e->data.m_data.num)) return false;
    } else {
      assert(e->hasStrKey());
      if (!st2->contains(e->data.m_data.pstr)) return false;
    }
  }
  return true;
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, TSet*>::type
BaseSet::Clone(ObjectData* obj) {
  auto thiz = static_cast<TSet*>(obj);
  auto target = static_cast<TSet*>(obj->cloneImpl());
  if (!thiz->m_size) {
    return target;
  }
  thiz->arrayData()->incRefCount();
  target->m_size = thiz->m_size;
  target->m_arr = thiz->m_arr;
  target->setIntLikeStrKeys(thiz->intLikeStrKeys());
  return target;
}

bool BaseSet::t_contains(const Variant& key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    return contains(key.toInt64());
  }
  if (isStringType(t)) {
    return contains(key.getStringData());
  }
  throwBadValueType();
  return false;
}

Object c_Set::t_remove(const Variant& key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    remove(key.toInt64());
  } else if (isStringType(t)) {
    remove(key.getStringData());
  } else {
    throwBadValueType();
  }
  return Object{this};
}

bool BaseSet::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    return set->contains(key->m_data.num);
  }
  if (isStringType(key->m_type)) {
    return set->contains(key->m_data.pstr);
  }
  throwBadValueType();
  return false;
}

bool BaseSet::OffsetEmpty(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    return set->contains(key->m_data.num) ? !cellToBool(*key) : true;
  }
  if (isStringType(key->m_type)) {
    return set->contains(key->m_data.pstr) ? !cellToBool(*key) : true;
  }
  throwBadValueType();
  return true;
}

bool BaseSet::OffsetContains(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    return set->contains(key->m_data.num);
  }
  if (isStringType(key->m_type)) {
    return set->contains(key->m_data.pstr);
  }
  throwBadValueType();
  return false;
}

void BaseSet::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    set->remove(key->m_data.num);
    return;
  }
  if (isStringType(key->m_type)) {
    set->remove(key->m_data.pstr);
    return;
  }
  throwBadValueType();
}

Object BaseSet::t_getiterator() {
  auto iter = collections::SetIterator::newInstance();
  Native::data<collections::SetIterator>(iter)->setSet(this);
  return iter;
}

template<typename TSet, class MakeArgs>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_map(const Variant& callback, MakeArgs makeArgs) const {
  VMRegAnchor _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object{std::move(set)};
  assert(posLimit() != 0);
  assert(hashSize() > 0);
  assert(set->arrayData() == staticEmptyMixedArray());
  auto oldCap = set->cap();
  set->reserve(posLimit()); // presume minimum collisions ...
  assert(set->canMutateBuffer());
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto* e = iter_elm(pos);
    TypedValue tvCbRet;
    int32_t pVer = m_version;
    auto args = makeArgs(*e);
    g_context->invokeFuncFew(&tvCbRet, ctx, args.size(), &(args[0]));
    // Now that tvCbRet is live, make sure to decref even if we throw.
    SCOPE_EXIT { tvRefcountedDecRef(&tvCbRet); };
    if (UNLIKELY(m_version != pVer)) throw_collection_modified();
    set->addRaw(&tvCbRet);
  }
  // ... and shrink back if that was incorrect
  set->shrinkIfCapacityTooHigh(oldCap);
  return Object{std::move(set)};
}

template<typename TSet, class MakeArgs>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_filter(const Variant& callback, MakeArgs makeArgs) const {
  VMRegAnchor _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object(std::move(set));
  // we don't reserve(), because we don't know how selective callback will be
  set->mutate();
  int32_t version = m_version;
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto* e = iter_elm(pos);
    auto args = makeArgs(*e);
    bool b = invokeAndCastToBool(ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!b) continue;
    e = iter_elm(pos);
    if (e->hasIntKey()) {
      set->addRaw(e->data.m_data.num);
    } else {
      assert(e->hasStrKey());
      set->addRaw(e->data.m_data.pstr);
    }
  }
  return Object(std::move(set));
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
  return Object(req::make<TSet>());
}

template<class MakeArgs>
ALWAYS_INLINE
Object BaseSet::php_retain(const Variant& callback, MakeArgs makeArgs) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto size = m_size;
  if (!size) { return Object{this}; }
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    int32_t version = m_version;
    auto* e = iter_elm(pos);
    auto args = makeArgs(*e);
    bool b = invokeAndCastToBool(ctx, args.size(), &(args[0]));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) { continue; }
    mutateAndBump();
    version = m_version;
    e = iter_elm(pos);
    ssize_t pp = (e->hasIntKey()
                   ? findForRemove(e->ikey)
                   : findForRemove(e->skey, e->skey->hash()));
    eraseNoCompact(pp);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
  }
  assert(m_size <= size);
  compactOrShrinkIfDensityTooLow();
  return Object{this};
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_take(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len >= int64_t(m_size)) {
    // We know the result Set will simply be a copy of this Set,
    // so we can just call Clone() and return early here.
    return Object::attach(TSet::Clone(this));
  }
  auto set = req::make<TSet>();
  if (len <= 0) {
    // We know the resulting Set will be empty, so we can return
    // early here.
    return Object{std::move(set)};
  }
  size_t sz = size_t(len);
  set->reserve(sz);
  set->setSize(sz);
  set->setPosLimit(sz);
  auto table = set->hashTab();
  auto mask = set->tableMask();
  for (uint32_t frPos = 0, toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = set->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      set->updateNextKI(toE.ikey);
    } else {
      assert(toE.hasStrKey());
      set->updateIntLikeStrKeys(toE.skey);
    }
  }
  return Object{std::move(set)};
}

template<class TSet, bool checkVersion>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object(std::move(set));
  set->mutate();
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  uint32_t used = posLimit();
  for (uint32_t i = 0; i < used; ++i) {
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
    if (e->hasIntKey()) {
      set->addRaw(e->data.m_data.num);
    } else {
      assert(e->hasStrKey());
      set->addRaw(e->data.m_data.pstr);
    }
  }
  return Object(std::move(set));
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len <= 0) {
    // We know the resulting Set will simply be a copy of this Set,
    // so we can just call Clone() and return early here.
    return Object::attach(TSet::Clone(this));
  }
  auto set = req::make<TSet>();
  if (len >= m_size) {
    // We know the resulting Set will be empty, so we can return
    // early here.
    return Object{std::move(set)};
  }
  size_t sz = size_t(m_size) - size_t(len);
  assert(sz);
  set->reserve(sz);
  set->setSize(sz);
  set->setPosLimit(sz);
  uint32_t frPos = nthElmPos(len);
  auto table = set->hashTab();
  auto mask = set->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(frPos)) {
      assert(frPos + 1 < posLimit());
      ++frPos;
    }
    auto& toE = set->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      set->updateNextKI(toE.ikey);
    } else {
      assert(toE.hasStrKey());
      set->updateIntLikeStrKeys(toE.skey);
    }
  }
  return Object{std::move(set)};
}

template<class TSet, bool checkVersion>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object(std::move(set));
  // we don't reserve(), because we don't know how selective fn will be
  set->mutate();
  int32_t version UNUSED;
  if (checkVersion) {
    version = m_version;
  }
  uint32_t used = posLimit();
  uint32_t i = 0;
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
    if (e.hasIntKey()) {
      set->addRaw(e.data.m_data.num);
    } else {
      assert(e.hasStrKey());
      set->addRaw(e.data.m_data.pstr);
    }
  }
  return Object(std::move(set));
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer");
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer");
  }
  size_t skipAmt = std::min<size_t>(istart, m_size);
  size_t sz = std::min<size_t>(ilen, size_t(m_size) - skipAmt);
  auto set = req::make<TSet>();
  set->reserve(sz);
  set->setSize(sz);
  set->setPosLimit(sz);
  uint32_t frPos = nthElmPos(skipAmt);
  auto table = set->hashTab();
  auto mask = set->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = set->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      set->updateNextKI(toE.ikey);
    } else {
      assert(toE.hasStrKey());
      set->updateIntLikeStrKeys(toE.skey);
    }
  }
  return Object{std::move(set)};
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromItems(const Variant& iterable) {
  auto set = req::make<TSet>();
  set->addAll(iterable);
  return Object(std::move(set));
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromKeysOf(const Variant& container) {
  if (container.isNull()) {
    return Object(req::make<TSet>());
  }
  const auto& cellContainer = container_as_cell(container);
  auto target = req::make<TSet>();
  target->addAllKeysOf(cellContainer);
  return Object(std::move(target));
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromArray(const Variant& arr) {
  if (!arr.isArray()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter arr must be an array");
  }
  auto set = req::make<TSet>();
  ArrayData* ad = arr.getArrayData();
  auto oldCap = set->cap();
  set->reserve(ad->size()); // presume minimum collisions ...
  ssize_t pos_limit = ad->iter_end();
  for (ssize_t pos = ad->iter_begin(); pos != pos_limit;
       pos = ad->iter_advance(pos)) {
    set->addRaw(ad->getValueRef(pos));
  }
  set->shrinkIfCapacityTooHigh(oldCap); // ... and shrink if we were wrong
  return Object(std::move(set));
}

template<class TSet>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_fromArrays(int _argc, const Array& _argv /* = null_array */) {
  auto set = req::make<TSet>();
  auto oldCap = set->cap();
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant arr = iter.second();
    if (!arr.isArray()) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Parameters must be arrays");
    }
    ArrayData* ad = arr.getArrayData();
    set->reserve(set->size() + ad->size()); // presume minimum collisions ...
    ssize_t pos_limit = ad->iter_end();
    for (ssize_t pos = ad->iter_begin(); pos != pos_limit;
         pos = ad->iter_advance(pos)) {
      set->addRaw(ad->getValueRef(pos));
    }
  }
  set->shrinkIfCapacityTooHigh(oldCap); // ... and shrink if we were wrong
  return Object(std::move(set));
}

// Protected (Internal)

BaseSet::~BaseSet() {
  auto const mixed = MixedArray::asMixed(arrayData());
  // Avoid indirect call, as we know it is a MixedArray
  if (mixed->decReleaseCheck()) MixedArray::Release(mixed);
}

NEVER_INLINE
void HashCollection::warnOnStrIntDup() const {
  req::hash_set<int64_t> seenVals;

  auto* eLimit = elmLimit();
  for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
    int64_t newVal = 0;

    if (e->hasIntKey()) {
      newVal = e->ikey;
    } else {
      assert(e->hasStrKey());
      // isStriclyInteger() puts the int value in newVal as a side effect.
      if (!e->skey->isStrictlyInteger(newVal)) continue;
    }

    if (seenVals.find(newVal) != seenVals.end()) {
      auto cls = getVMClass()->name()->toCppString();
      auto pos = cls.rfind('\\');
      if (pos != std::string::npos) {
        cls = cls.substr(pos + 1);
      }
      raise_warning(
        "%s::toArray() for a %s containing both int(%" PRId64 ") "
        "and string('%" PRId64 "')",
        cls.c_str(),
        toLower(cls).c_str(),
        newVal,
        newVal
      );

      return;
    }

    seenVals.insert(newVal);
  }
  // Do nothing if no 'duplicates' were found.
}

void BaseSet::throwBadValueType() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer values and string values may be used with Sets");
}

///////////////////////////////////////////////////////////////////////////////
// Set

void BaseSet::t___construct(const Variant& iterable /* = null_variant */) {
  addAll(iterable);
}

Object c_Set::t_add(const Variant& val) {
  add(val);
  return Object{this};
}

Object c_Set::t_addall(const Variant& iterable) {
  addAll(iterable);
  return Object{this};
}

Object c_Set::t_addallkeysof(const Variant& container) {
  if (!container.isNull()) {
    const auto& containerCell = container_as_cell(container);
    addAllKeysOf(containerCell);
  }
  return Object{this};
}

Object c_Set::t_clear() {
  ++m_version;
  dropImmCopy();
  decRefArr(arrayData());
  m_arr = staticEmptyMixedArray();
  m_size = 0;
  setIntLikeStrKeys(false);
  return Object{this};
}

Object BaseSet::t_items() {
  return SystemLib::AllocLazyIterableViewObject(Variant{this});
}

Object c_Set::t_values() {
  return BaseSet::php_values<c_Vector>();
}

Object c_Set::t_keys() {
  return BaseSet::php_values<c_Vector>();
}

Object c_Set::t_map(const Variant& callback) {
  return php_map<c_Set>(callback, &makeArgsFromHashValue);
}

Object c_Set::t_mapwithkey(const Variant& callback) {
  return php_map<c_Set>(callback, &makeArgsFromHashKeyAndValue);
}

Object c_Set::t_filter(const Variant& callback) {
  return php_filter<c_Set>(callback, &makeArgsFromHashValue);
}

Object c_Set::t_filterwithkey(const Variant& callback) {
  return php_filter<c_Set>(callback, &makeArgsFromHashKeyAndValue);
}

Object c_Set::t_retain(const Variant& callback) {
  return php_retain(callback, &makeArgsFromHashValue);
}

Object c_Set::t_retainwithkey(const Variant& callback) {
  return php_retain(callback, &makeArgsFromHashKeyAndValue);
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
  auto vec = req::make<TVector>();
  uint32_t sz = m_size;
  vec->reserve((size_t)sz + itSize);
  assert(vec->canMutateBuffer());
  vec->setSize(sz);

  uint32_t used = posLimit();
  for (uint32_t i = 0, j = 0; i < used; ++i) {
    if (isTombstone(i)) {
      continue;
    }
    cellDup(data()[i].data, vec->data()[j]);
    ++j;
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return Object{std::move(vec)};
}

Variant BaseSet::t_firstvalue() {
  if (!m_size) return init_null();
  auto* e = firstElm();
  assert(e != elmLimit());
  return tvAsCVarRef(&e->data);
}

Variant BaseSet::t_firstkey() {
  return t_firstvalue();
}

Variant BaseSet::t_lastvalue() {
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
  return tvAsCVarRef(&data()[pos].data);
}

Variant BaseSet::t_lastkey() {
  return t_lastvalue();
}

Object c_Set::t_removeall(const Variant& iterable) {
  if (iterable.isNull()) return Object{this};
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
  return Object{this};
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

c_Set* c_Set::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_Set>(obj);
}

///////////////////////////////////////////////////////////////////////////////
// ImmSet

Object c_ImmSet::t_values() {
  return BaseSet::php_values<c_ImmVector>();
}

Object c_ImmSet::t_keys() {
  return BaseSet::php_values<c_ImmVector>();
}

Object c_ImmSet::t_map(const Variant& callback) {
  return php_map<c_ImmSet>(callback, &makeArgsFromHashValue);
}

Object c_ImmSet::t_mapwithkey(const Variant& callback) {
  return php_map<c_ImmSet>(callback, &makeArgsFromHashKeyAndValue);
}

Object c_ImmSet::t_filter(const Variant& callback) {
  return php_filter<c_ImmSet>(callback, &makeArgsFromHashValue);
}

Object c_ImmSet::t_filterwithkey(const Variant& callback) {
  return php_filter<c_ImmSet>(callback, &makeArgsFromHashKeyAndValue);
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

c_ImmSet* c_ImmSet::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_ImmSet>(obj);
}

Object BaseSet::t_tovector() { return materializeImpl<c_Vector>(this); }

Object BaseSet::t_toimmvector() { return materializeImpl<c_ImmVector>(this); }

Object BaseSet::t_tomap() { return materializeImpl<c_Map>(this); }

Object BaseSet::t_toimmmap() { return materializeImpl<c_ImmMap>(this); }

Object c_Set::t_toset() { return Object::attach(c_Set::Clone(this)); }
Object c_ImmSet::t_toset() { return materializeImpl<c_Set>(this); }

Object c_Set::t_toimmset() { return getImmutableCopy(); }
Object c_ImmSet::t_toimmset() { return Object{this}; }

Object c_Set::t_immutable() { return getImmutableCopy(); }
Object c_ImmSet::t_immutable() { return Object{this}; }

///////////////////////////////////////////////////////////////////////////////

#define COLLECTION_MAGIC_METHODS(cls) \
  Variant cls::t___get(Variant name) { \
    throw_collection_property_exception(); \
  } \
  Variant cls::t___set(Variant name, Variant value) { \
    throw_collection_property_exception(); \
  } \
  bool cls::t___isset(Variant name) { \
    return false; \
  } \
  Variant cls::t___unset(Variant name) { \
    throw_collection_property_exception(); \
  }

COLLECTION_MAGIC_METHODS(BaseVector)
COLLECTION_MAGIC_METHODS(HashCollection)

#undef COLLECTION_MAGIC_METHODS

#define COLLECTION_TOSTRING_METHOD(cls) \
  String c_##cls::t___tostring() { return #cls; }

COLLECTION_TOSTRING_METHOD(Vector)
COLLECTION_TOSTRING_METHOD(ImmVector)
COLLECTION_TOSTRING_METHOD(Map)
COLLECTION_TOSTRING_METHOD(ImmMap)
COLLECTION_TOSTRING_METHOD(Set)
COLLECTION_TOSTRING_METHOD(ImmSet)

#undef COLLECTION_TOSTRING_METHOD

///////////////////////////////////////////////////////////////////////////////
// Many of the collectionXYZ functions need to throw exceptions
// with common error messages
// (e.g. collections::initMapElem() when called on an immutable collection).
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

}
