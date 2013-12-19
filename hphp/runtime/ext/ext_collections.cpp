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

#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/ext/ext_intl.h"
#include "hphp/runtime/ext/base_vector.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/system/systemlib.h"

#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * The "materialization" methods have the form "to[CollectionName]()" and
 * allow us to get an instance of a collection type from another.
 * This template provides a default implementation.
 */
template<typename TCollection>
inline static Object materializeDefaultImpl(ObjectData* obj) {
  TCollection* col;
  Object o = col = NEWOBJ(TCollection)();
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

const int kCollectionObjectDataAttrs =
  ObjectData::IsCppBuiltin |
  ObjectData::IsCollection |
  ObjectData::UseGet |
  ObjectData::UseSet |
  ObjectData::UseIsset |
  ObjectData::UseUnset |
  ObjectData::CallToImpl |
  ObjectData::HasClone;

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

ArrayIter getArrayIterHelper(CVarRef v, size_t& sz) {
  if (v.isArray()) {
    ArrayData* ad = v.getArrayData();
    sz = ad->size();
    return ArrayIter(ad);
  }
  if (v.isObject()) {
    ObjectData* obj = v.getObjectData();
    if (obj->isCollection()) {
      sz = obj->getCollectionSize();
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
  assert(!vec->m_frozenCopy.isNull()); // Should've been checked by the JIT.
  vec->mutate();
}

///////////////////////////////////////////////////////////////////////////////

c_Vector::c_Vector(Class* cls /* = c_Vector::classof() */) : BaseVector(cls) {

  ObjectData::setAttributes(kCollectionObjectDataAttrs);
  o_subclassData.u16 = Collection::VectorType;
}

void c_Vector::t___construct(CVarRef iterable /* = null_variant */) {
  BaseVector::construct(iterable);
}

void c_Vector::resize(int64_t sz, TypedValue* val) {
  assert(val && val->m_type != KindOfRef);
  ++m_version;
  mutate();

  assert(sz >= 0);
  uint requestedSize = (uint)sz;
  if (m_capacity < requestedSize) {
    m_capacity = requestedSize;
    m_data =
      (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
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

Object c_Vector::t_add(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_Vector::t_addall(CVarRef iterable) {
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

Object c_Vector::t_append(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Variant c_Vector::t_pop() {
  ++m_version;
  if (m_size) {
    mutate();
    --m_size;
    Variant ret = tvAsCVarRef(&m_data[m_size]);
    tvRefcountedDecRef(&m_data[m_size]);
    return ret;
  } else {
    Object e(SystemLib::AllocRuntimeExceptionObject(
      "Cannot pop empty Vector"));
    throw e;
  }
}

void c_Vector::t_resize(CVarRef sz, CVarRef value) {
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
  TypedValue* val = cvarToCell(&value);
  resize(intSz, val);
}

void c_Vector::t_reserve(CVarRef sz) {
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
  reserve(intSz);
}

Object c_Vector::t_clear() {
  ++m_version;
  mutate();

  uint sz = m_size;
  for (int i = 0; i < sz; ++i) {
    tvRefcountedDecRef(&m_data[i]);
  }
  if (m_data) smart_free(m_data);
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
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_Vector);
  BaseVector::keys(bv);
  return obj;
}

Object c_Vector::t_values() {
  return Object::attach(BaseVector::Clone<c_Vector>(this));
}

Object c_Vector::t_lazy() {
  return BaseVector::lazy();
}

Object c_Vector::t_kvzip() {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_Vector);
  BaseVector::kvzip(bv);
  return obj;
}

Variant c_Vector::t_at(CVarRef key) {
  return BaseVector::at(key);
}

Variant c_Vector::t_get(CVarRef key) {
  return BaseVector::get(key);
}

bool c_Vector::t_contains(CVarRef key) {
  return t_containskey(key);
}

bool c_Vector::t_containskey(CVarRef key) {
  return BaseVector::containskey(key);
}

Object c_Vector::t_removekey(CVarRef key) {
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

void c_Vector::t_splice(CVarRef offset, CVarRef len /* = null */,
                        CVarRef replacement /* = null */) {
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
    Object e(SystemLib::AllocRuntimeExceptionObject(
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

int64_t c_Vector::t_linearsearch(CVarRef search_value) {
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

Object c_Vector::t_map(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_Vector);
  BaseVector::map(bv, callback);
  return obj;
}

Object c_Vector::t_mapwithkey(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_Vector);
  BaseVector::mapwithkey(bv, callback);
  return obj;
}

Object c_Vector::t_filter(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_Vector);
  BaseVector::filter(bv, callback);
  return obj;
}

Object c_Vector::t_filterwithkey(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_Vector);
  BaseVector::filterwithkey(bv, callback);
  return obj;
}

Object c_Vector::t_zip(CVarRef iterable) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_Vector);
  BaseVector::zip(bv, iterable);
  return obj;
}

Object c_Vector::t_set(CVarRef key, CVarRef value) {
  if (key.isInteger()) {
    TypedValue* tv = cvarToCell(&value);
    set(key.toInt64(), tv);
    return this;
  }
  throwBadKeyType();
  return this;
}

Object c_Vector::t_setall(CVarRef iterable) {
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

Object c_Vector::t_put(CVarRef key, CVarRef value) {
  return t_set(key, value);
}

Object c_Vector::ti_fromitems(CVarRef iterable) {
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

Object c_Vector::ti_fromarray(CVarRef arr) {
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
    (TypedValue*)smart_malloc(size_t(sz) * sizeof(TypedValue));
  ssize_t pos = ad->iter_begin();
  for (uint i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    assert(pos != ArrayData::invalid_index);
    cellDup(*cvarToCell(&ad->getValueRef(pos)), data[i]);
  }
  return ret;
}

Object c_Vector::ti_slice(CVarRef vec, CVarRef offset,
                          CVarRef len /* = null */) {
  return BaseVector::slice<c_Vector>("Vector", vec, offset, len);
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

bool c_Vector::usort(CVarRef cmp_function) {
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

void c_Vector::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  auto vec = static_cast<c_Vector*>(obj);
  if (key->m_type == KindOfInt64) {
    vec->set(key->m_data.num, val);
    return;
  }
  throwBadKeyType();
}

void c_Vector::OffsetUnset(ObjectData* obj, TypedValue* key) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot unset an element of a Vector"));
  throw e;
}

void c_Vector::initFvFields(c_FrozenVector* fv) {
  fv->m_data = m_data;
  fv->m_size = m_size;
  fv->m_capacity = m_capacity;
  fv->m_version = m_version;
}

Object c_Vector::t_tovector() {
  return materializeDefaultImpl<c_Vector>(this);
}

Object c_Vector::t_toset() {
  return materializeDefaultImpl<c_Set>(this);
}

Object c_Vector::t_tofrozenvector() {
    if (m_frozenCopy.isNull()) {
      c_FrozenVector* fv = NEWOBJ(c_FrozenVector)();
      initFvFields(fv);
      m_frozenCopy = fv;
    }

    return m_frozenCopy;
}

Object c_Vector::t_tomap() {
  return materializeDefaultImpl<c_Map>(this);
}

Object c_Vector::t_tostablemap() {
  return materializeDefaultImpl<c_StableMap>(this);
}

Object c_Vector::t_tofrozenset() {
  return materializeDefaultImpl<c_FrozenSet>(this);
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
// c_FrozenVector

// ConstCollection

bool c_FrozenVector::t_isempty() {
  return BaseVector::isempty();
}

int64_t c_FrozenVector::t_count() {
  return BaseVector::count();
}

Object c_FrozenVector::t_items() {
  return BaseVector::items();
}

// ConstIndexAccess

bool c_FrozenVector::t_containskey(CVarRef key) {
  return BaseVector::containskey(key);
}

Variant c_FrozenVector::t_at(CVarRef key) {
  return BaseVector::at(key);
}

Variant c_FrozenVector::t_get(CVarRef key) {
  return BaseVector::get(key);
}

// KeyedIterable

Object c_FrozenVector::t_getiterator() {
  return BaseVector::getiterator();
}

Object c_FrozenVector::t_map(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_FrozenVector);
  BaseVector::map(bv, callback);
  return obj;
}

Object c_FrozenVector::t_mapwithkey(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_FrozenVector);
  BaseVector::mapwithkey(bv, callback);
  return obj;
}

Object c_FrozenVector::t_filter(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_FrozenVector);
  BaseVector::filter(bv, callback);
  return obj;
}

Object c_FrozenVector::t_filterwithkey(CVarRef callback) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_FrozenVector);
  BaseVector::filterwithkey(bv, callback);
  return obj;
}

Object c_FrozenVector::t_zip(CVarRef iterable) {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_FrozenVector);
  BaseVector::zip(bv, iterable);
  return obj;
}

Object c_FrozenVector::t_kvzip() {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_FrozenVector);
  BaseVector::kvzip(bv);
  return obj;
}

Object c_FrozenVector::t_keys() {
  BaseVector* bv;
  Object obj = bv = NEWOBJ(c_FrozenVector);
  BaseVector::keys(bv);
  return obj;
}

Object c_FrozenVector::ti_slice(CVarRef vec, CVarRef offset,
                                CVarRef len /* = null */) {
  return BaseVector::slice<c_FrozenVector>("FrozenVector", vec, offset, len);
}

// Others

void c_FrozenVector::t___construct(CVarRef iterable /* = null_variant */) {
  BaseVector::construct(iterable);
}

Object c_FrozenVector::t_lazy() {
  return BaseVector::lazy();
}

Array c_FrozenVector::t_toarray() {
  return BaseVector::toarray();
}

Array c_FrozenVector::t_tokeysarray() {
  return BaseVector::tokeysarray();
}

Array c_FrozenVector::t_tovaluesarray() {
  return BaseVector::tovaluesarray();
}

int64_t c_FrozenVector::t_linearsearch(CVarRef search_value) {
  return BaseVector::linearsearch(search_value);
}

Object c_FrozenVector::t_values() {
  return Object::attach(BaseVector::Clone<c_FrozenVector>(this));
}


// Non PHP methods.

c_FrozenVector::c_FrozenVector(Class* cls) : BaseVector(cls) {

  ObjectData::setAttributes(kCollectionObjectDataAttrs);
  o_subclassData.u16 = Collection::FrozenVectorType;
}

///////////////////////////////////////////////////////////////////////////////

static int32_t empty_map_hash_slot = c_Map::Empty;

c_Map::c_Map(Class* cb) :
    ExtObjectDataFlags(cb) {
  o_subclassData.u16 = Collection::MapType;
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

c_Map::~c_Map() {
  deleteElms();
  freeData();
}

void c_Map::freeData() {
  if (m_data) smart_free(m_data);
}

void c_Map::deleteElms() {
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

void c_Map::t___construct(CVarRef iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Array c_Map::toArrayImpl() const {
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

c_Map* c_Map::Clone(ObjectData* obj) {
  auto thiz = static_cast<c_Map*>(obj);
  auto target = static_cast<c_Map*>(obj->cloneImpl());

  if (!thiz->m_size) return target;

  assert(target->m_size == 0);
  assert(thiz->m_used != 0);
  target->m_capAndUsed = thiz->m_capAndUsed;
  target->m_tableMask = thiz->m_tableMask;
  target->m_size = thiz->m_size;
  target->m_data =
    (Elm*)smart_malloc(size_t(thiz->m_cap) * sizeof(Elm) +
                       thiz->hashSize() * sizeof(int32_t));
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

void c_Map::init(CVarRef t) {
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

Object c_Map::t_add(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_Map::t_addall(CVarRef iterable) {
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

Object c_Map::t_clear() {
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

bool c_Map::t_isempty() {
  return !toBoolImpl();
}

int64_t c_Map::t_count() {
  return m_size;
}

Object c_Map::t_items() {
  return SystemLib::AllocLazyKVZipIterableObject(this);
}

Object c_Map::t_keys() {
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

Object c_Map::t_lazy() {
  return SystemLib::AllocLazyKeyedIterableViewObject(this);
}

Object c_Map::t_kvzip() {
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
    c_Pair* pair = NEWOBJ(c_Pair)();
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

Variant c_Map::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  } else if (key.isString()) {
    return tvAsCVarRef(at(key.getStringData()));
  }
  throwBadKeyType();
  return uninit_null();
}

Variant c_Map::t_get(CVarRef key) {
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

Object c_Map::t_set(CVarRef key, CVarRef value) {
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

Object c_Map::t_setall(CVarRef iterable) {
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

Object c_Map::t_put(CVarRef key, CVarRef value) {
  return t_set(key, value);
}

bool c_Map::t_contains(CVarRef key) {
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

bool c_Map::t_containskey(CVarRef key) {
  return t_contains(key);
}

Object c_Map::t_remove(CVarRef key) {
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

Object c_Map::t_removekey(CVarRef key) {
  return t_remove(key);
}

Array c_Map::t_toarray() {
  return toArrayImpl();
}

Object c_Map::t_values() {
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  int64_t sz = m_size;
  if (!sz) {
    return ret;
  }
  TypedValue* vecData;
  target->m_capacity = target->m_size = sz;
  target->m_data = vecData =
    (TypedValue*)smart_malloc(sz * sizeof(TypedValue));

  int64_t j = 0;
  for (ssize_t i = 0; i < iterLimit(); ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    cellDup(p.data, vecData[j]);
    ++j;
  }
  return ret;
}

Array c_Map::t_tokeysarray() {
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

Array c_Map::t_tovaluesarray() {
  PackedArrayInit ai(m_size);
  for (ssize_t i = 0; i < iterLimit(); ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    ai.append(tvAsCVarRef(&p.data));
  }
  return ai.toArray();
}

Object c_Map::t_differencebykey(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  c_Map* target = c_Map::Clone(this);
  auto ret = Object::attach(target);
  if (obj->getCollectionType() == Collection::MapType) {
    auto mp = static_cast<c_Map*>(obj);
    for (uint i = 0; i < mp->iterLimit(); ++i) {
      if (mp->isTombstone(i)) continue;
      c_Map::Elm& p = mp->data()[i];
      if (p.hasIntKey()) {
        target->remove((int64_t)p.ikey);
      } else {
        target->remove(p.skey);
      }
    }
    return ret;
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
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

Object c_Map::t_getiterator() {
  c_MapIterator* it = NEWOBJ(c_MapIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_version = getVersion();
  return it;
}

Object c_Map::t_map(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Map* mp;
  Object obj = mp = NEWOBJ(c_Map)();
  if (!m_size) return obj;
  assert(m_used != 0);
  mp->deleteElms();
  mp->freeData();
  mp->m_cap = m_cap;
  mp->m_tableMask = m_tableMask;
  mp->m_size = m_size;
  mp->m_data = (Elm*)smart_malloc(size_t(mp->m_cap) * sizeof(Elm) +
                                  mp->hashSize() * sizeof(int32_t));
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
    g_vmContext->invokeFuncFew(tv, ctx, 1, &p.data);
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

Object c_Map::t_mapwithkey(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Map* mp;
  Object obj = mp = NEWOBJ(c_Map)();
  if (!m_size) return obj;
  assert(m_used != 0);
  mp->deleteElms();
  mp->freeData();
  mp->m_cap = m_cap;
  mp->m_tableMask = m_tableMask;
  mp->m_size = m_size;
  mp->m_data = (Elm*)smart_malloc(size_t(mp->m_cap) * sizeof(Elm) +
                                  mp->hashSize() * sizeof(int32_t));
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
    TypedValue args[2] = {
      (p.hasIntKey() ? make_tv<KindOfInt64>(p.ikey)
                     : make_tv<KindOfString>(p.skey)),
      p.data
    };
    g_vmContext->invokeFuncFew(tv, ctx, 2, args);
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

Object c_Map::t_filter(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Map* mp;
  Object obj = mp = NEWOBJ(c_Map)();
  if (!m_size) return obj;
  uint32_t used = iterLimit();
  for (uint i = 0; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& p = data()[i];
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p.data);
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

Object c_Map::t_filterwithkey(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Map* mp;
  Object obj = mp = NEWOBJ(c_Map)();
  if (!m_size) return obj;
  uint32_t used = iterLimit();
  for (uint32_t i = 0; i < used; ++i) {
    Elm& p = data()[i];
    if (isTombstone(i)) continue;
    Variant ret;
    int32_t version = m_version;
    TypedValue args[2] = {
      (p.hasIntKey() ? make_tv<KindOfInt64>(p.ikey)
                     : make_tv<KindOfString>(p.skey)),
      p.data
    };
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 2, args);
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

Object c_Map::t_zip(CVarRef iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Map* mp;
  Object obj = mp = NEWOBJ(c_Map)();
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

Object c_Map::ti_fromitems(CVarRef iterable) {
  if (iterable.isNull()) return NEWOBJ(c_Map)();
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Map* target;
  Object ret = target = NEWOBJ(c_Map)();
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
      throwBadKeyType();
    }
  }
  return ret;
}

Object c_Map::ti_fromarray(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  c_Map* mp;
  Object ret = mp = NEWOBJ(c_Map)();
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

NEVER_INLINE
void c_Map::throwOOB(int64_t key) {
  throwIntOOB(key);
}

NEVER_INLINE
void c_Map::throwOOB(StringData* key) {
  throwStrOOB(key);
}

NEVER_INLINE
void c_Map::throwTooLarge() {
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
void c_Map::throwReserveTooLarge() {
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
int32_t* c_Map::warnUnbalanced(size_t n, int32_t* ei) {
  if (n > size_t(RuntimeOption::MaxArrayChain)) {
    raise_error("Map is too unbalanced (%lu)", n);
  }
  return ei;
}

TypedValue* c_Map::at(int64_t key) const {
  auto p = find(key);
  if (UNLIKELY(p == Empty)) {
    throwOOB(key);
  }
  return &data()[p].data;
}

TypedValue* c_Map::at(StringData* key) const {
  auto p = find(key, key->hash());
  if (UNLIKELY(p == Empty)) {
    throwOOB(key);
  }
  return &data()[p].data;
}

TypedValue* c_Map::get(int64_t key) const {
  auto p = find(key);
  if (p == Empty) {
    return nullptr;
  }
  return &data()[p].data;
}

TypedValue* c_Map::get(StringData* key) const {
  auto p = find(key, key->hash());
  if (p == Empty) {
    return nullptr;
  }
  return &data()[p].data;
}

void c_Map::add(TypedValue* val) {
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

void c_Map::remove(int64_t key) {
  ++m_version;
  auto* p = findForInsert(key);
  if (validPos(*p)) {
    erase(p);
  }
}

void c_Map::remove(StringData* key) {
  ++m_version;
  auto* p = findForInsert(key, key->hash());
  if (validPos(*p)) {
    erase(p);
  }
}

bool c_Map::contains(int64_t key) const {
  return find(key) != Empty;
}

bool c_Map::contains(StringData* key) const {
  return find(key, key->hash()) != Empty;
}

static bool hitStringKey(const c_Map::Elm& e, const StringData* s,
                         int32_t hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. c_Map guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!c_Map::isTombstone(e.data.m_type));
  return hash == e.hash() && (s == e.skey || s->same(e.skey));
}

static bool hitIntKey(const c_Map::Elm& e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. c_Map guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!c_Map::isTombstone(e.data.m_type));
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
ssize_t c_Map::findImpl(size_t h0, Hit hit) const {
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

ssize_t c_Map::find(int64_t ki) const {
  return findImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

ssize_t c_Map::find(const StringData* s, strhash_t prehash) const {
  auto h = prehash | STRHASH_MSB;
  return findImpl(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

template <class Hit>
ALWAYS_INLINE
int32_t* c_Map::findForInsertImpl(size_t h0, Hit hit) const {
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

int32_t* c_Map::findForInsert(int64_t ki) const {
  return findForInsertImpl(ki, [ki] (const Elm& e) {
    return hitIntKey(e, ki);
  });
}

int32_t* c_Map::findForInsert(const StringData* s, strhash_t prehash) const {
  auto h = prehash | STRHASH_MSB;
  return findForInsertImpl(prehash, [s, h] (const Elm& e) {
    return hitStringKey(e, s, h);
  });
}

// findForNewInsert() is only safe to use if you know for sure that the
// key is not already present in the Map.
ALWAYS_INLINE int32_t*
c_Map::findForNewInsert(int32_t* table, size_t mask, size_t h0) const {
  for (size_t i = 1, probe = h0;; ++i) {
    auto ei = &table[probe & mask];
    if (!validPos(*ei)) return ei;
    probe += i;
    assert(i <= mask && probe == h0 + (i + i*i) / 2);
  }
}

ALWAYS_INLINE
int32_t* c_Map::findForNewInsert(size_t h0) const {
  return findForNewInsert(hashTab(), m_tableMask, h0);
}

void c_Map::update(int64_t h, TypedValue* val) {
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

void c_Map::update(StringData* key, TypedValue* val) {
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

void c_Map::erase(int32_t* pos) {
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
  // of Map's implementation rely on the fact that erase() does not allow
  // the Elm density to drop below ~50%.
  if (isDensityTooLow()) {
    compact();
  }
}

NEVER_INLINE void c_Map::makeRoom() {
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

NEVER_INLINE void c_Map::reserve(int64_t sz) {
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
    auto lgSize = c_Map::SmallLgTableSize;
    for (newCap = c_Map::SmallSize; newCap < sz; newCap <<= 1) ++lgSize;
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

void c_Map::grow(uint32_t newCap, uint32_t newMask) {
  assert(m_size <= m_used && m_used <= m_cap);
  size_t newHashSize = size_t(newMask) + 1;
  assert(Util::isPowerOfTwo(newHashSize) && computeMaxElms(newMask) == newCap);
  assert(m_size <= newCap && newCap <= MaxSize);
  auto* oldData = data();
  auto* data = (Elm*)smart_malloc(size_t(newCap) * sizeof(Elm) +
                                  newHashSize * sizeof(int32_t));
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
  if (oldData) smart_free(oldData);
  m_cap = newCap;
  m_used = m_size;
}

void c_Map::compact() {
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

void c_Map::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with Maps"));
  throw e;
}

Array c_Map::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return static_cast<const c_Map*>(obj)->toArrayImpl();
}

bool c_Map::ToBool(const ObjectData* obj) {
  return static_cast<const c_Map*>(obj)->toBoolImpl();
}

TypedValue* c_Map::OffsetGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<c_Map*>(obj);
  if (key->m_type == KindOfInt64) {
    return mp->at(key->m_data.num);
  }
  if (IS_STRING_TYPE(key->m_type)) {
    return mp->at(key->m_data.pstr);
  }
  throwBadKeyType();
  return nullptr;
}

void c_Map::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  auto mp = static_cast<c_Map*>(obj);
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

bool c_Map::OffsetIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<c_Map*>(obj);
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

bool c_Map::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<c_Map*>(obj);
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

bool c_Map::OffsetContains(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<c_Map*>(obj);
  if (key->m_type == KindOfInt64) {
    return mp->contains(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    return mp->contains(key->m_data.pstr);
  } else {
    throwBadKeyType();
    return false;
  }
}

void c_Map::OffsetUnset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto mp = static_cast<c_Map*>(obj);
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

bool c_Map::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto mp1 = static_cast<const c_Map*>(obj1);
  auto mp2 = static_cast<const c_Map*>(obj2);
  if (mp1->m_size != mp2->m_size) return false;
  for (uint i = 0; i < mp1->iterLimit(); ++i) {
    if (mp1->isTombstone(i)) continue;
    c_Map::Elm& p = mp1->data()[i];
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

void c_Map::Unserialize(ObjectData* obj,
                        VariableUnserializer* uns,
                        int64_t sz,
                        char type) {
  if (type != 'K') {
    throw Exception("Map does not support the '%c' serialization "
                    "format", type);
  }
  auto mp = static_cast<c_Map*>(obj);
  mp->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    Variant k;
    k.unserialize(uns, Uns::Mode::ColKey);
    int32_t* p;
    Elm* e;
    if (k.isInteger()) {
      auto h = k.toInt64();
      p = mp->findForInsert(h);
      if (UNLIKELY(validPos(*p))) goto do_unserialize;
      e = &mp->allocElm(p);
      e->setIntKey(h);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      p = mp->findForInsert(key, h);
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

c_MapIterator::c_MapIterator(Class* cb) :
    ExtObjectData(cb) {
}

c_MapIterator::~c_MapIterator() {
}

void c_MapIterator::t___construct() {
}

Variant c_MapIterator::t_current() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  if (!mp->iter_valid(m_pos)) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(mp->iter_value(m_pos));
}

Variant c_MapIterator::t_key() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  if (!mp->iter_valid(m_pos)) {
    throw_iterator_not_valid();
  }
  return mp->iter_key(m_pos);
}

bool c_MapIterator::t_valid() {
  c_Map* mp = m_obj.get();
  return mp->iter_valid(m_pos);
}

void c_MapIterator::t_next() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  m_pos = mp->iter_next(m_pos);
}

void c_MapIterator::t_rewind() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  m_pos = mp->iter_begin();
}

///////////////////////////////////////////////////////////////////////////////

#define CONNECT_TO_GLOBAL_DLLIST(mapobj, element)                       \
do {                                                                    \
  (element)->pListLast = (mapobj)->m_pListTail;                         \
  (mapobj)->m_pListTail = (element);                                    \
  (element)->pListNext = nullptr;                                       \
  if ((element)->pListLast != nullptr) {                                \
    (element)->pListLast->pListNext = (element);                        \
  }                                                                     \
  if (!(mapobj)->m_pListHead) {                                         \
    (mapobj)->m_pListHead = (element);                                  \
  }                                                                     \
} while (false)

static const char emptyStableMapSlot[sizeof(c_StableMap::Bucket*)] = { 0 };

c_StableMap::c_StableMap(Class* cb)
  : ExtObjectDataFlags(cb)
  , m_version(0)
  , m_pListHead(nullptr)
  , m_pListTail(nullptr)
{
  m_size = 0;
  m_nTableSize = 0;
  m_nTableMask = 0;
  m_arBuckets = (Bucket**)emptyStableMapSlot;

  o_subclassData.u16 = Collection::StableMapType;
}

c_StableMap::~c_StableMap() {
  deleteBuckets();
  freeData();
}

void c_StableMap::freeData() {
  if (m_arBuckets != (Bucket**)emptyStableMapSlot) {
    smart_free(m_arBuckets);
  }
  m_arBuckets = (Bucket**)emptyStableMapSlot;
}

void c_StableMap::deleteBuckets() {
  Bucket* p = m_pListHead;
  while (p) {
    Bucket* q = p;
    p = p->pListNext;
    q->release();
  }
}

void c_StableMap::t___construct(CVarRef iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Array c_StableMap::toArrayImpl() const {
  ArrayInit ai(m_size);
  Bucket* p = m_pListHead;
  while (p) {
    if (p->hasIntKey()) {
      ai.set((int64_t)p->ikey, tvAsCVarRef(&p->data));
    } else {
      ai.set(*(const String*)(&p->skey), tvAsCVarRef(&p->data));
    }
    p = p->pListNext;
  }
  return ai.create();
}

c_StableMap* c_StableMap::Clone(ObjectData* obj) {
  auto thiz = static_cast<c_StableMap*>(obj);
  auto target = static_cast<c_StableMap*>(obj->cloneImpl());

  if (!thiz->m_size) return target;

  target->m_size = thiz->m_size;
  target->m_nTableSize = thiz->m_nTableSize;
  target->m_nTableMask = thiz->m_nTableMask;
  target->m_arBuckets =
    (Bucket**)smart_calloc(thiz->m_nTableSize, sizeof(Bucket*));

  Bucket *last = nullptr;
  for (Bucket* p = thiz->m_pListHead; p; p = p->pListNext) {
    Bucket *np = Bucket::Make();
    cellDup(p->data, np->data);
    uint nIndex;
    if (p->hasIntKey()) {
      np->setIntKey(p->ikey);
      nIndex = p->ikey & target->m_nTableMask;
    } else {
      np->setStrKey(p->skey, p->hash());
      nIndex = p->hash() & target->m_nTableMask;
    }

    np->pNext = target->m_arBuckets[nIndex];
    target->m_arBuckets[nIndex] = np;

    if (last) {
      last->pListNext = np;
      np->pListLast = last;
    } else {
      target->m_pListHead = np;
    }
    last = np;
  }
  target->m_pListTail = last;

  return target;
}

void c_StableMap::init(CVarRef t) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(t, sz);
  if (sz) {
    reserve(sz);
  }
  for (; iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else if (k.isString()) {
      update(k.getStringData(), tv);
    } else {
      throwBadKeyType();
    }
  }
}

Object c_StableMap::t_add(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_StableMap::t_addall(CVarRef iterable) {
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

Object c_StableMap::t_clear() {
  deleteBuckets();
  freeData();
  m_pListHead = nullptr;
  m_pListTail = nullptr;
  m_size = 0;
  m_nTableSize = 0;
  m_nTableMask = 0;
  m_arBuckets = (Bucket**)emptyStableMapSlot;
  return this;
}

bool c_StableMap::t_isempty() {
  return !toBoolImpl();
}

int64_t c_StableMap::t_count() {
  return m_size;
}

Object c_StableMap::t_items() {
  return SystemLib::AllocLazyKVZipIterableObject(this);
}

Object c_StableMap::t_keys() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  uint64_t j = 0;
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
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

Object c_StableMap::t_lazy() {
  return SystemLib::AllocLazyKeyedIterableViewObject(this);
}

Object c_StableMap::t_kvzip() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  uint64_t j = 0;
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    c_Pair* pair = NEWOBJ(c_Pair)();
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

Variant c_StableMap::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  } else if (key.isString()) {
    return tvAsCVarRef(at(key.getStringData()));
  }
  throwBadKeyType();
  return uninit_null();
}

Variant c_StableMap::t_get(CVarRef key) {
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

Object c_StableMap::t_set(CVarRef key, CVarRef value) {
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

Object c_StableMap::t_setall(CVarRef iterable) {
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

Object c_StableMap::t_put(CVarRef key, CVarRef value) {
  return t_set(key, value);
}

bool c_StableMap::t_contains(CVarRef key) {
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

bool c_StableMap::t_containskey(CVarRef key) {
  return t_contains(key);
}

Object c_StableMap::t_remove(CVarRef key) {
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

Object c_StableMap::t_removekey(CVarRef key) {
  return t_remove(key);
}

Array c_StableMap::t_toarray() {
  return toArrayImpl();
}

Object c_StableMap::t_values() {
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  int64_t sz = m_size;
  if (!sz) {
    return ret;
  }
  TypedValue* data;
  target->m_capacity = target->m_size = m_size;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  Bucket* p = m_pListHead;
  for (int64_t i = 0; i < sz; ++i) {
    assert(p);
    cellDup(p->data, data[i]);
    p = p->pListNext;
  }
  return ret;
}

Array c_StableMap::t_tokeysarray() {
  PackedArrayInit ai(m_size);
  Bucket* p = m_pListHead;
  while (p) {
    if (p->hasIntKey()) {
      ai.append(int64_t{p->ikey});
    } else {
      ai.append(*(const String*)(&p->skey));
    }
    p = p->pListNext;
  }
  return ai.toArray();
}

Array c_StableMap::t_tovaluesarray() {
  PackedArrayInit ai(m_size);
  Bucket* p = m_pListHead;
  while (p) {
    ai.append(tvAsCVarRef(&p->data));
    p = p->pListNext;
  }
  return ai.toArray();
}

Object c_StableMap::t_differencebykey(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  c_StableMap* target = c_StableMap::Clone(this);
  auto ret = Object::attach(target);
  if (obj->getCollectionType() == Collection::StableMapType) {
    auto smp = static_cast<c_StableMap*>(obj);
    c_StableMap::Bucket* p = smp->m_pListHead;
    while (p) {
      if (p->hasIntKey()) {
        target->remove((int64_t)p->ikey);
      } else {
        target->remove(p->skey);
      }
      p = p->pListNext;
    }
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
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

Object c_StableMap::t_getiterator() {
  c_StableMapIterator* it = NEWOBJ(c_StableMapIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_version = getVersion();
  return it;
}

Object c_StableMap::t_map(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_StableMap* smp;
  Object obj = smp = NEWOBJ(c_StableMap)();
  if (!m_size) return obj;
  smp->m_size = m_size;
  smp->m_nTableSize = m_nTableSize;
  smp->m_nTableMask = m_nTableMask;
  smp->m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));
  Bucket *last = nullptr;
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p->data);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    Bucket *np = Bucket::Make(ret.asCell());
    uint nIndex;
    if (p->hasIntKey()) {
      np->setIntKey(p->ikey);
      nIndex = p->ikey & smp->m_nTableMask;
    } else {
      np->setStrKey(p->skey, p->hash());
      nIndex = p->hash() & smp->m_nTableMask;
    }
    np->pNext = smp->m_arBuckets[nIndex];
    smp->m_arBuckets[nIndex] = np;
    if (last) {
      last->pListNext = np;
      np->pListLast = last;
    } else {
      smp->m_pListHead = np;
    }
    last = np;
  }
  smp->m_pListTail = last;
  return obj;
}

Object c_StableMap::t_mapwithkey(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_StableMap* smp;
  Object obj = smp = NEWOBJ(c_StableMap)();
  if (!m_size) return obj;
  smp->m_size = m_size;
  smp->m_nTableSize = m_nTableSize;
  smp->m_nTableMask = m_nTableMask;
  smp->m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));
  Bucket *last = nullptr;
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    Variant ret;
    int32_t version = m_version;
    TypedValue args[2] = {
      (p->hasIntKey() ? make_tv<KindOfInt64>(p->ikey)
                      : make_tv<KindOfString>(p->skey)),
      p->data
    };
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 2, args);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    Bucket *np = Bucket::Make(ret.asTypedValue());
    uint nIndex;
    if (p->hasIntKey()) {
      np->setIntKey(p->ikey);
      nIndex = p->ikey & smp->m_nTableMask;
    } else {
      np->setStrKey(p->skey, p->hash());
      nIndex = p->hash() & smp->m_nTableMask;
    }
    np->pNext = smp->m_arBuckets[nIndex];
    smp->m_arBuckets[nIndex] = np;
    if (last) {
      last->pListNext = np;
      np->pListLast = last;
    } else {
      smp->m_pListHead = np;
    }
    last = np;
  }
  smp->m_pListTail = last;
  return obj;
}

Object c_StableMap::t_filter(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_StableMap* smp;
  Object obj = smp = NEWOBJ(c_StableMap)();
  if (!m_size) return obj;
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 1, &p->data);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!ret.toBoolean()) continue;
    if (p->hasIntKey()) {
      smp->update(p->ikey, &p->data);
    } else {
      smp->update(p->skey, &p->data);
    }
  }
  return obj;
}

Object c_StableMap::t_filterwithkey(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_StableMap* smp;
  Object obj = smp = NEWOBJ(c_StableMap)();
  if (!m_size) return obj;
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    Variant ret;
    int32_t version = m_version;
    TypedValue args[2] = {
      (p->hasIntKey() ? make_tv<KindOfInt64>(p->ikey)
                      : make_tv<KindOfString>(p->skey)),
      p->data
    };
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 2, args);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!ret.toBoolean()) continue;
    if (p->hasIntKey()) {
      smp->update(p->ikey, &p->data);
    } else {
      smp->update(p->skey, &p->data);
    }
  }
  return obj;
}

Object c_StableMap::t_zip(CVarRef iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_StableMap* smp;
  Object obj = smp = NEWOBJ(c_StableMap)();
  smp->reserve(std::min(sz, size_t(m_size)));
  for (Bucket* p = m_pListHead; p && iter; p = p->pListNext, ++iter) {
    Variant v = iter.second();
    c_Pair* pair;
    Object pairObj = pair = NEWOBJ(c_Pair)();
    pair->initAdd(&p->data);
    pair->initAdd(cvarToCell(&v));
    TypedValue tv;
    tv.m_data.pobj = pair;
    tv.m_type = KindOfObject;
    if (p->hasIntKey()) {
      smp->update(p->ikey, &tv);
    } else {
      smp->update(p->skey, &tv);
    }
  }
  return obj;
}

Object c_StableMap::ti_fromitems(CVarRef iterable) {
  if (iterable.isNull()) return NEWOBJ(c_StableMap)();
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_StableMap* target;
  Object ret = target = NEWOBJ(c_StableMap)();
  if (sz) {
    target->reserve(sz);
  }
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
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
      throwBadKeyType();
    }
  }
  return ret;
}

Object c_StableMap::ti_fromarray(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  c_StableMap* smp;
  Object ret = smp = NEWOBJ(c_StableMap)();
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    Variant v = ad->getValue(pos);
    TypedValue* tv = cvarToCell(&v);
    if (k.isInteger()) {
      smp->update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      smp->update(k.getStringData(), tv);
    }
  }
  return ret;
}

void c_StableMap::throwOOB(int64_t key) {
  throwIntOOB(key);
}

void c_StableMap::throwOOB(StringData* key) {
  throwStrOOB(key);
}

void c_StableMap::add(TypedValue* val) {
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

ALWAYS_INLINE
bool sm_hit_string_key(const c_StableMap::Bucket* p, const char* k,
                       int len, int32_t hash) {
  if (p->hasIntKey()) return false;
  const char* data = p->skey->data();
  return data == k || (p->hash() == hash &&
                       p->skey->size() == len &&
                       memcmp(data, k, len) == 0);
}

c_StableMap::Bucket* c_StableMap::find(int64_t h) const {
  for (Bucket* p = m_arBuckets[h & m_nTableMask]; p; p = p->pNext) {
    if (p->hasIntKey() && p->ikey == h) {
      return p;
    }
  }
  return nullptr;
}

c_StableMap::Bucket* c_StableMap::find(const char* k, int len,
                                       strhash_t prehash) const {
  int32_t hash = c_StableMap::Bucket::encodeHash(prehash);
  for (Bucket* p = m_arBuckets[prehash & m_nTableMask]; p; p = p->pNext) {
    if (sm_hit_string_key(p, k, len, hash)) return p;
  }
  return nullptr;
}

c_StableMap::Bucket** c_StableMap::findForErase(int64_t h) const {
  Bucket** ret = &(m_arBuckets[h & m_nTableMask]);
  Bucket* p = *ret;
  while (p) {
    if (p->hasIntKey() && p->ikey == h) {
      return ret;
    }
    ret = &(p->pNext);
    p = *ret;
  }
  return nullptr;
}

c_StableMap::Bucket** c_StableMap::findForErase(const char* k, int len,
                                                strhash_t prehash) const {
  Bucket** ret = &(m_arBuckets[prehash & m_nTableMask]);
  Bucket* p = *ret;
  int32_t hash = c_StableMap::Bucket::encodeHash(prehash);
  while (p) {
    if (sm_hit_string_key(p, k, len, hash)) return ret;
    ret = &(p->pNext);
    p = *ret;
  }
  return nullptr;
}

bool c_StableMap::update(int64_t h, TypedValue* data) {
  assert(data->m_type != KindOfRef);
  Bucket* p = find(h);
  if (p) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    tvCopy(*data, p->data);
    return true;
  }
  ++m_version;
  if (++m_size > m_nTableSize) {
    adjustCapacity();
  }
  p = Bucket::Make(data);
  p->setIntKey(h);
  uint nIndex = (h & m_nTableMask);
  p->pNext = m_arBuckets[nIndex];
  m_arBuckets[nIndex] = p;
  CONNECT_TO_GLOBAL_DLLIST(this, p);
  return true;
}

bool c_StableMap::update(StringData *key, TypedValue* data) {
  assert(data->m_type != KindOfRef);
  strhash_t h = key->hash();
  Bucket* p = find(key->data(), key->size(), h);
  if (p) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    tvCopy(*data, p->data);
    return true;
  }
  ++m_version;
  if (++m_size > m_nTableSize) {
    adjustCapacity();
  }
  p = Bucket::Make(data);
  p->setStrKey(key, h);
  uint nIndex = (h & m_nTableMask);
  p->pNext = m_arBuckets[nIndex];
  m_arBuckets[nIndex] = p;
  CONNECT_TO_GLOBAL_DLLIST(this, p);
  return true;
}

void c_StableMap::erase(Bucket** prev) {
  if (!prev) {
    return;
  }
  Bucket* p = *prev;
  if (p) {
    *prev = p->pNext;
    if (p->pListLast) {
      p->pListLast->pListNext = p->pListNext;
    } else {
      /* Deleting the head of the list */
      assert(m_pListHead == p);
      m_pListHead = p->pListNext;
    }
    if (p->pListNext) {
      p->pListNext->pListLast = p->pListLast;
    } else {
      assert(m_pListTail == p);
      m_pListTail = p->pListLast;
    }
    m_size--;
    p->release();
  }
}

void c_StableMap::adjustCapacityImpl(int64_t sz) {
  ++m_version;
  if (sz < 4) {
    if (sz <= 0) return;
    sz = 4;
  } else {
    sz = Util::roundUpToPowerOfTwo(sz);
  }
  if (m_nTableSize == 0) {
    m_nTableSize = sz;
    m_nTableMask = m_nTableSize - 1;
    m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));
    return;
  }
  m_nTableSize = sz;
  m_nTableMask = m_nTableSize - 1;
  smart_free(m_arBuckets);
  m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    uint nIndex = (p->hashKey() & m_nTableMask);
    p->pNext = m_arBuckets[nIndex];
    m_arBuckets[nIndex] = p;
  }
}

ssize_t c_StableMap::iter_next(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  p = p->pListNext;
  return reinterpret_cast<ssize_t>(p);
}

ssize_t c_StableMap::iter_prev(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  p = p->pListLast;
  return reinterpret_cast<ssize_t>(p);
}

Variant c_StableMap::iter_key(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  if (p->hasStrKey()) {
    return p->skey;
  }
  return (int64_t)p->ikey;
}

TypedValue* c_StableMap::iter_value(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return &p->data;
}

struct StableMapKeyAccessor {
  typedef const c_StableMap::Bucket* ElmT;
  bool isInt(ElmT elm) const { return elm->hasIntKey(); }
  bool isStr(ElmT elm) const { return elm->hasStrKey(); }
  int64_t getInt(ElmT elm) const { return elm->ikey; }
  StringData* getStr(ElmT elm) const { return elm->skey; }
  Variant getValue(ElmT elm) const {
    if (isInt(elm)) {
      return getInt(elm);
    }
    assert(isStr(elm));
    return getStr(elm);
  }
};

struct StableMapValAccessor {
  typedef const c_StableMap::Bucket* ElmT;
  bool isInt(ElmT elm) const { return elm->data.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return IS_STRING_TYPE(elm->data.m_type); }
  int64_t getInt(ElmT elm) const { return elm->data.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm->data.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm->data); }
};

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
c_StableMap::SortFlavor c_StableMap::preSort(Bucket** buffer,
                                             const AccessorT& acc,
                                             bool checkTypes) {
  assert(m_size > 0);
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  uint i = 0;
  // Build up an auxillary array of Bucket pointers. We will
  // sort this auxillary array, and then we will rebuild the
  // linked list based on the result.
  if (checkTypes) {
    for (Bucket* p = m_pListHead; p; ++i, p = p->pListNext) {
      allInts = (allInts && acc.isInt(p));
      allStrs = (allStrs && acc.isStr(p));
      buffer[i] = p;
    }
    return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
  } else {
    for (Bucket* p = m_pListHead; p; ++i, p = p->pListNext) {
      buffer[i] = p;
    }
    return GenericSort;
  }
}

/**
 * postSort() runs after sorting has been performed. For StableMap, postSort()
 * handles rewiring the linked list according to the results of the sort.
 */
void c_StableMap::postSort(Bucket** buffer) {
  uint last = m_size-1;
  Bucket* b = m_pListHead = buffer[0];
  b->pListLast = nullptr;
  for (uint i = 0; i < last; ++i) {
    Bucket* bNext = buffer[i+1];
    b->pListNext = bNext;
    bNext->pListLast = b;
    b = bNext;
  }
  m_pListTail = b;
  b->pListNext = nullptr;
}

#define SORT_CASE(flag, cmp_type, acc_type) \
  case flag: { \
    if (ascending) { \
      cmp_type##Compare<acc_type, flag, true> comp; \
      HPHP::Sort::sort(buffer, buffer + m_size, comp); \
    } else { \
      cmp_type##Compare<acc_type, flag, false> comp; \
      HPHP::Sort::sort(buffer, buffer + m_size, comp); \
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
#define SORT_BODY(acc_type) \
  do { \
    if (!m_size) { \
      return; \
    } \
    Bucket** buffer = (Bucket**)smart_malloc(m_size * sizeof(Bucket*)); \
    SortFlavor flav = preSort<acc_type>(buffer, acc_type(), true); \
    try { \
      CALL_SORT(acc_type); \
    } catch (...) { \
      postSort(buffer); \
      smart_free(buffer); \
      throw; \
    } \
    postSort(buffer); \
    smart_free(buffer); \
  } while(0)

void c_StableMap::asort(int sort_flags, bool ascending) {
  SORT_BODY(StableMapValAccessor);
}

void c_StableMap::ksort(int sort_flags, bool ascending) {
  SORT_BODY(StableMapKeyAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT
#undef SORT_BODY

#define USER_SORT_BODY(acc_type)                                        \
  do {                                                                  \
    if (!m_size) {                                                      \
      return true;                                                      \
    }                                                                   \
    CallCtx ctx;                                                        \
    JIT::CallerFrame cf;                                             \
    vm_decode_function(cmp_function, cf(), false, ctx);                 \
    if (!ctx.func) {                                                    \
      return false;                                                     \
    }                                                                   \
    Bucket** buffer = (Bucket**)smart_malloc(m_size * sizeof(Bucket*)); \
    preSort<acc_type>(buffer, acc_type(), false);                       \
    SCOPE_EXIT {                                                        \
      postSort(buffer);                                                 \
      smart_free(buffer);                                               \
    };                                                                  \
    ElmUCompare<acc_type> comp;                                         \
    comp.ctx = &ctx;                                                    \
    HPHP::Sort::sort(buffer, buffer + m_size, comp);                    \
    return true;                                                        \
  } while (0)

bool c_StableMap::uasort(CVarRef cmp_function) {
  USER_SORT_BODY(StableMapValAccessor);
}

bool c_StableMap::uksort(CVarRef cmp_function) {
  USER_SORT_BODY(StableMapKeyAccessor);
}

#undef USER_SORT_BODY

void c_StableMap::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with StableMaps"));
  throw e;
}

c_StableMap::Bucket::~Bucket() {
  if (hasStrKey()) {
    decRefStr(skey);
  }
  tvRefcountedDecRef(&data);
}

void c_StableMap::Bucket::release() {
  this->~Bucket();
  MM().smartFreeSizeLogged(this, sizeof(Bucket));
}

void c_StableMap::Bucket::dump() {
  printf("c_StableMap::Bucket: %" PRIx64 ", %p, %p, %p\n",
         hashKey(), pListNext, pListLast, pNext);
  if (hasStrKey()) {
    skey->dump();
  }
  tvAsCVarRef(&data).dump();
}

Array c_StableMap::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return static_cast<const c_StableMap*>(obj)->toArrayImpl();
}

bool c_StableMap::ToBool(const ObjectData* obj) {
  return static_cast<const c_StableMap*>(obj)->toBoolImpl();
}

TypedValue* c_StableMap::OffsetGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto smp = static_cast<c_StableMap*>(obj);
  if (key->m_type == KindOfInt64) {
    return smp->at(key->m_data.num);
  }
  if (IS_STRING_TYPE(key->m_type)) {
    return smp->at(key->m_data.pstr);
  }
  throwBadKeyType();
  return nullptr;
}

void c_StableMap::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  auto smp = static_cast<c_StableMap*>(obj);
  if (key->m_type == KindOfInt64) {
    smp->set(key->m_data.num, val);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    smp->set(key->m_data.pstr, val);
    return;
  }
  throwBadKeyType();
}

bool c_StableMap::OffsetIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto smp = static_cast<c_StableMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = smp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = smp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellIsNull(result) : false;
}

bool c_StableMap::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto smp = static_cast<c_StableMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = smp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = smp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellToBool(*result) : true;
}

bool c_StableMap::OffsetContains(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto smp = static_cast<c_StableMap*>(obj);
  if (key->m_type == KindOfInt64) {
    return smp->contains(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    return smp->contains(key->m_data.pstr);
  } else {
    throwBadKeyType();
    return false;
  }
}

void c_StableMap::OffsetUnset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto smp = static_cast<c_StableMap*>(obj);
  if (key->m_type == KindOfInt64) {
    smp->remove(key->m_data.num);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    smp->remove(key->m_data.pstr);
    return;
  }
  throwBadKeyType();
}

bool c_StableMap::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto smp1 = static_cast<const c_StableMap*>(obj1);
  auto smp2 = static_cast<const c_StableMap*>(obj2);
  if (smp1->m_size != smp2->m_size) return false;
  auto p1 = smp1->m_pListHead;
  auto p2 = smp2->m_pListHead;
  for (; p1; p1 = p1->pListNext, p2 = p2->pListNext) {
    assert(p2);
    // Check if the keys are identical (===)
    if (p1->hasIntKey()) {
      if (!p2->hasIntKey()) return false;
      if (p1->ikey != p2->ikey) return false;
    } else {
      assert(p1->hasStrKey());
      if (!p2->hasStrKey()) return false;
      if (!p1->skey->same(p2->skey)) return false;
    }
    // Compare the values using equals (==)
    if (!equal(tvAsCVarRef(&p1->data), tvAsCVarRef(&p2->data))) {
      return false;
    }
  }
  return true;
}

void c_StableMap::Unserialize(ObjectData* obj,
                              VariableUnserializer* uns,
                              int64_t sz,
                              char type) {
  if (type != 'K') {
    throw Exception("StableMap does not support the '%c' serialization "
                    "format", type);
  }
  auto smp = static_cast<c_StableMap*>(obj);
  smp->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    Variant k;
    k.unserialize(uns, Uns::Mode::ColKey);
    Bucket* p;
    uint nIndex;
    if (k.isInteger()) {
      auto h = k.toInt64();
      p = smp->find(h);
      if (UNLIKELY(p != nullptr)) goto do_unserialize;
      p = Bucket::Make();
      p->setIntKey(h);
      nIndex = (h & smp->m_nTableMask);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      p = smp->find(key->data(), key->size(), h);
      if (UNLIKELY(p != nullptr)) goto do_unserialize;
      p = Bucket::Make();
      p->setStrKey(key, h);
      nIndex = (h & smp->m_nTableMask);
    } else {
      throw Exception("Invalid key");
    }
    ++smp->m_size;
    p->data.m_type = KindOfNull;
    p->pNext = smp->m_arBuckets[nIndex];
    smp->m_arBuckets[nIndex] = p;
    CONNECT_TO_GLOBAL_DLLIST(smp, p);
do_unserialize:
    tvAsVariant(&p->data).unserialize(uns, Uns::Mode::ColValue);
  }
}

#undef CONNECT_TO_GLOBAL_DLLIST

c_StableMapIterator::c_StableMapIterator(Class* cb) :
    ExtObjectData(cb) {
}

c_StableMapIterator::~c_StableMapIterator() {
}

void c_StableMapIterator::t___construct() {
}

Variant c_StableMapIterator::t_current() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_version != smp->getVersion())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(smp->iter_value(m_pos));
}

Variant c_StableMapIterator::t_key() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_version != smp->getVersion())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return smp->iter_key(m_pos);
}

bool c_StableMapIterator::t_valid() {
  return m_pos != 0;
}

void c_StableMapIterator::t_next() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_version != smp->getVersion())) {
    throw_collection_modified();
  }
  m_pos = smp->iter_next(m_pos);
}

void c_StableMapIterator::t_rewind() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_version != smp->getVersion())) {
    throw_collection_modified();
  }
  m_pos = smp->iter_begin();
}

///////////////////////////////////////////////////////////////////////////////
// BaseSet

// Public

void BaseSet::init(CVarRef t) {
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
  Bucket* p = findForInsert(h);
  assert(p);
  if (p->validValue()) {
    return;
  }
  ++m_version;
  ++m_size;
  if (!p->tombstone()) {
    if (UNLIKELY(++m_load >= computeMaxLoad())) {
      adjustCapacity();
      p = findForInsert(h);
      assert(p);
    }
  }
  p->setInt(h);
}

void BaseSet::add(StringData *key) {
  strhash_t h = key->hash();
  Bucket* p = findForInsert(key->data(), key->size(), h);
  assert(p);
  if (p->validValue()) {
    return;
  }
  ++m_version;
  ++m_size;
  if (!p->tombstone()) {
    if (UNLIKELY(++m_load >= computeMaxLoad())) {
      adjustCapacity();
      p = findForInsert(key->data(), key->size(), h);
      assert(p);
    }
  }
  p->setStr(key, h);
}

void BaseSet::throwOOB(int64_t val) {
  throwIntOOB(val);
}

void BaseSet::throwOOB(StringData* val) {
  throwStrOOB(val);
}

void BaseSet::throwNoIndexAccess() {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "[] operator not supported for accessing elements of Sets"));
  throw e;
}

Array BaseSet::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return static_cast<const BaseSet*>(obj)->toArrayImpl();
}

bool BaseSet::ToBool(const ObjectData* obj) {
  return static_cast<const BaseSet*>(obj)->toBoolImpl();
}

TypedValue* BaseSet::OffsetGet(ObjectData* obj, TypedValue* key) {
  BaseSet::throwNoIndexAccess();
}

void BaseSet::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  BaseSet::throwNoIndexAccess();
}

bool BaseSet::OffsetIsset(ObjectData* obj, TypedValue* key) {
  BaseSet::throwNoIndexAccess();
}

bool BaseSet::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  BaseSet::throwNoIndexAccess();
}

bool BaseSet::OffsetContains(ObjectData* obj, TypedValue* key) {
  BaseSet::throwNoIndexAccess();
}

void BaseSet::OffsetUnset(ObjectData* obj, TypedValue* key) {
  BaseSet::throwNoIndexAccess();
}

bool BaseSet::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto st1 = static_cast<const BaseSet*>(obj1);
  auto st2 = static_cast<const BaseSet*>(obj2);
  if (st1->m_size != st2->m_size) return false;
  for (uint i = 0; i <= st1->m_nLastSlot; ++i) {
    BaseSet::Bucket& p = st1->m_data[i];
    if (p.validValue()) {
      if (p.hasInt()) {
        int64_t key = p.data.m_data.num;
        if (!st2->find(key)) return false;
      } else {
        assert(p.hasStr());
        StringData* key = p.data.m_data.pstr;
        if (!st2->find(key->data(), key->size(), key->hash())) return false;
      }
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
    Bucket* p;
    if (k.isInteger()) {
      auto h = k.toInt64();
      p = st->findForInsert(h);
      if (UNLIKELY(p->validValue())) continue;
      p->setInt(h);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      p = st->findForInsert(key->data(), key->size(), h);
      if (UNLIKELY(p->validValue())) continue;
      p->setStr(key, h);
    } else {
      throw Exception("%s values must be integers or strings", setType);
    }
    ++st->m_size;
    ++st->m_load;
  }
}

// Protected (PHP-accesible methods)

void BaseSet::phpConstruct(CVarRef iterable /* = null_variant */) {
  if (iterable.isNull()) return;
  init(iterable);
}

Object BaseSet::phpAddAll(CVarRef iterable) {
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

bool BaseSet::phpContains(CVarRef key) {
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

Object BaseSet::phpRemove(CVarRef key) {
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

Array BaseSet::phpToValuesArray() {
  PackedArrayInit ai(m_size);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      ai.append(tvAsCVarRef(&p.data));
    }
  }
  return ai.create();
}

Object BaseSet::phpGetIterator() {
  c_SetIterator* it = NEWOBJ(c_SetIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_version = getVersion();
  return it;
}

const char BaseSet::emptySetSlot[sizeof(BaseSet::Bucket)] = {0};

// Protected (Internal)

BaseSet::BaseSet(Class* cls) : ExtObjectData(cls), m_size(0),
    m_data((Bucket*)emptySetSlot), m_load(0), m_nLastSlot(0), m_version(0) {
}

BaseSet::~BaseSet() {
  deleteBuckets();
  freeData();
}

// Private

NEVER_INLINE
void BaseSet::warnOnStrIntDup() const {
  smart::hash_set<int64_t> seenVals;

  for (uint32_t i = 0; i <= m_nLastSlot; i++) {
    Bucket& p = m_data[i];
    int64_t newVal = 0;

    if (!p.validValue()) continue;

    if (p.hasInt()) {
      newVal = p.data.m_data.num;
    } else {
      assert(p.hasStr());
      // isStriclyInteger() puts the int value in newVal as a side effect.
      if (!p.data.m_data.pstr->isStrictlyInteger(newVal)) continue;
    }

    if (seenVals.find(newVal) != seenVals.end()) {
      raise_warning(
        "Set::toArray() for a set containing both int(%ld) and string('%ld')",
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
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      if (p.hasInt()) {
        ai.set(p.data.m_data.num, tvAsCVarRef(&p.data));
      } else {
        assert(p.hasStr());
        ai.set(p.data.m_data.pstr, tvAsCVarRef(&p.data));
      }
    }
  }

  Array arr = ai.create();

  // If both '123' and 123 are present in the set, we better warn the user.
  if (UNLIKELY(arr.length() < m_size)) warnOnStrIntDup();
  return arr;
}

#define STRING_HASH(x)   (int32_t(x) | 0x80000000)

ALWAYS_INLINE
bool hitString(const BaseSet::Bucket* p, const char* k,
                         int len, int32_t hash) {
  assert(p->validValue());
  if (p->hasInt()) return false;
  const char* data = p->data.m_data.pstr->data();
  return data == k || (p->hash() == hash &&
                       p->data.m_data.pstr->size() == len &&
                       memcmp(data, k, len) == 0);
}

ALWAYS_INLINE
bool hitInt(const BaseSet::Bucket* p, int64_t ki) {
  assert(p->validValue());
  return p->hasInt() && p->data.m_data.num == ki;
}

#define FIND_BODY(h0, hit) \
  size_t tableMask = m_nLastSlot; \
  size_t probeIndex = size_t(h0) & tableMask; \
  Bucket* p = fetchBucket(probeIndex); \
  if (LIKELY(p->validValue() && (hit))) { \
    return p; \
  } \
  if (LIKELY(p->empty())) { \
    return nullptr; \
  } \
  for (size_t i = 1;; ++i) { \
    assert(i <= tableMask); \
    probeIndex = (probeIndex + i) & tableMask; \
    assert(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex); \
    p = fetchBucket(probeIndex); \
    if (p->validValue() && (hit)) { \
      return p; \
    } \
    if (p->empty()) { \
      return nullptr; \
    } \
  }

#define FIND_FOR_INSERT_BODY(h0, hit) \
  size_t tableMask = m_nLastSlot; \
  size_t probeIndex = size_t(h0) & tableMask; \
  Bucket* p = fetchBucket(h0 & tableMask); \
  if (LIKELY((p->validValue() && (hit)) || \
             p->empty())) { \
    return p; \
  } \
  Bucket* ts = nullptr; \
  for (size_t i = 1;; ++i) { \
    if (UNLIKELY(p->tombstone() && !ts)) { \
      ts = p; \
    } \
    assert(i <= tableMask); \
    probeIndex = (probeIndex + i) & tableMask; \
    assert(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex); \
    p = fetchBucket(probeIndex); \
    if (LIKELY(p->validValue() && (hit))) { \
      return p; \
    } \
    if (LIKELY(p->empty())) { \
      if (LIKELY(!ts)) { \
        return p; \
      } \
      return ts; \
    } \
  }

BaseSet::Bucket* BaseSet::find(int64_t h) const {
  FIND_BODY(h, hitInt(p, h));
}

BaseSet::Bucket* BaseSet::find(const char* k, int len,
    strhash_t prehash) const {
  FIND_BODY(prehash, hitString(p, k, len, STRING_HASH(prehash)));
}

BaseSet::Bucket* BaseSet::findForInsert(int64_t h) const {
  FIND_FOR_INSERT_BODY(h, hitInt(p, h));
}

BaseSet::Bucket* BaseSet::findForInsert(const char* k, int len,
                                    strhash_t prehash) const {
  FIND_FOR_INSERT_BODY(prehash, hitString(p, k, len, STRING_HASH(prehash)));
}

// findForNewInsert() is only safe to use if you know for sure that the
// value is not already present in the Set.
ALWAYS_INLINE
BaseSet::Bucket* BaseSet::findForNewInsert(size_t h0) const {
  size_t tableMask = m_nLastSlot;
  size_t probeIndex = h0 & tableMask;
  Bucket* p = fetchBucket(probeIndex);
  if (LIKELY(p->empty())) {
    return p;
  }
  for (size_t i = 1;; ++i) {
    assert(i <= tableMask);
    probeIndex = (probeIndex + i) & tableMask;
    assert(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex);
    p = fetchBucket(probeIndex);
    if (LIKELY(p->empty())) {
      return p;
    }
  }
}

#undef STRING_HASH
#undef FIND_BODY
#undef FIND_FOR_INSERT_BODY

void BaseSet::erase(Bucket* p) {
  if (!p) {
    return;
  }
  if (p->validValue()) {
    m_size--;
    tvRefcountedDecRef(&p->data);
    p->data.m_type = (DataType)KindOfTombstone;
    if (m_size < computeMinElements() && m_size) {
      adjustCapacity();
    }
  }
}

void BaseSet::adjustCapacityImpl(int64_t sz) {
  ++m_version;
  if (sz < 2) {
    if (sz <= 0) return;
    sz = 2;
  }
  if (m_nLastSlot == 0) {
    assert(m_data == (Bucket*)emptySetSlot);
    m_nLastSlot = Util::roundUpToPowerOfTwo(sz << 1) - 1;
    m_data = (Bucket*)smart_calloc(numSlots(), sizeof(Bucket));
    return;
  }
  size_t oldNumSlots = numSlots();
  m_nLastSlot = Util::roundUpToPowerOfTwo(sz << 1) - 1;
  m_load = m_size;
  Bucket* oldBuckets = m_data;
  m_data = (Bucket*)smart_calloc(numSlots(), sizeof(Bucket));
  for (uint i = 0; i < oldNumSlots; ++i) {
    Bucket* p = &oldBuckets[i];
    if (p->validValue()) {
      Bucket* np =
        findForNewInsert(p->hasInt() ? p->data.m_data.num : p->hash());
      memcpy(np, p, sizeof(Bucket));
    }
  }
  smart_free(oldBuckets);
}

void BaseSet::deleteBuckets() {
  if (!m_size) return;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      tvRefcountedDecRef(&p.data);
    }
  }
}

ssize_t BaseSet::iter_next(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  Bucket* pLast = fetchBucket(m_nLastSlot);
  ++p;
  while (p <= pLast) {
    if (p->validValue()) {
      return reinterpret_cast<ssize_t>(p);
    }
    ++p;
  }
  return 0;
}

ssize_t BaseSet::iter_prev(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  Bucket* pStart = m_data;
  --p;
  while (p >= pStart) {
    if (p->validValue()) {
      return reinterpret_cast<ssize_t>(p);
    }
    --p;
  }
  return 0;
}

Variant BaseSet::iter_key(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return tvAsVariant(&p->data);
}

const TypedValue* BaseSet::iter_value(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return &p->data;
}

void BaseSet::throwBadValueType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer values and string values may be used with Sets"));
  throw e;
}

///////////////////////////////////////////////////////////////////////////////
// Set

c_Set::c_Set(Class* cls /* = c_Set::classof() */) : BaseSet(cls) {

  ObjectData::setAttributes(kCollectionObjectDataAttrs);
  o_subclassData.u16 = Collection::SetType;
}

void c_Set::t___construct(CVarRef iterable /* = null_variant */) {
  BaseSet::phpConstruct(iterable);
}

Object c_Set::t_add(CVarRef val) {
  return BaseSet::phpAdd(val);
}

Object c_Set::t_addall(CVarRef iterable) {
  return BaseSet::phpAddAll(iterable);
}

Object c_Set::t_clear() {
  return BaseSet::phpClear();
}

bool c_Set::t_isempty() {
  return BaseSet::phpIsEmpty();
}

int64_t c_Set::t_count() {
  return BaseSet::phpCount();
}

Object c_Set::t_items() {
  return BaseSet::phpItems();
}

Object c_Set::t_values() {
  return BaseSet::phpValues<c_Vector>();
}

Object c_Set::t_lazy() {
  return BaseSet::phpLazy();
}

bool c_Set::t_contains(CVarRef key) {
  return BaseSet::phpContains(key);
}

Object c_Set::t_remove(CVarRef key) {
  return BaseSet::phpRemove(key);
}

Array c_Set::t_toarray() {
  return BaseSet::phpToArray();
}

Array c_Set::t_tokeysarray() {
  return BaseSet::phpToKeysArray();
}

Array c_Set::t_tovaluesarray() {
  return BaseSet::phpToValuesArray();
}

Object c_Set::t_getiterator() {
  return BaseSet::phpGetIterator();
}

Object c_Set::t_map(CVarRef callback) {
  return BaseSet::phpMap<c_Set>(callback);
}

Object c_Set::t_filter(CVarRef callback) {
  return BaseSet::phpFilter<c_Set>(callback);
}

Object c_Set::t_zip(CVarRef iterable) {
  return BaseSet::phpZip<c_Set>(iterable);
}

Object c_Set::t_difference(CVarRef iterable) {
  if (iterable.isNull()) return this;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    phpRemove(iter.second());
  }
  return this;
}

Object c_Set::ti_fromitems(CVarRef iterable) {
  return BaseSet::phpFromItems<c_Set>(iterable);
}

Object c_Set::ti_fromarray(CVarRef arr) {
  return BaseSet::phpFromArray<c_Set>(arr);
}

Object c_Set::ti_fromarrays(int _argc, CArrRef _argv /* = null_array */) {
  return BaseSet::phpFromArrays<c_Set>(_argc, _argv);
}

void c_Set::Unserialize(ObjectData* obj, VariableUnserializer* uns,
                        int64_t sz, char type) {

  BaseSet::Unserialize("Set", obj, uns, sz, type);
}

c_Set* c_Set::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_Set>(obj);
}

///////////////////////////////////////////////////////////////////////////////
// FrozenSet

void c_FrozenSet::t___construct(CVarRef iterable /* = null_variant */) {
  BaseSet::phpConstruct(iterable);
}

bool c_FrozenSet::t_isempty() {
  return BaseSet::phpIsEmpty();
}

int64_t c_FrozenSet::t_count() {
  return BaseSet::phpCount();
}

Object c_FrozenSet::t_items() {
  return BaseSet::phpItems();
}

Object c_FrozenSet::t_values() {
  return BaseSet::phpValues<c_FrozenVector>();
}

Object c_FrozenSet::t_lazy() {
  return BaseSet::phpLazy();
}

bool c_FrozenSet::t_contains(CVarRef key) {
  return BaseSet::phpContains(key);
}

Array c_FrozenSet::t_toarray() {
  return BaseSet::phpToArray();
}

Array c_FrozenSet::t_tokeysarray() {
  return BaseSet::phpToKeysArray();
}

Array c_FrozenSet::t_tovaluesarray() {
  return BaseSet::phpToValuesArray();
}

Object c_FrozenSet::t_getiterator() {
  return BaseSet::phpGetIterator();
}

Object c_FrozenSet::t_map(CVarRef callback) {
  return BaseSet::phpMap<c_FrozenSet>(callback);
}

Object c_FrozenSet::t_filter(CVarRef callback) {
  return BaseSet::phpFilter<c_FrozenSet>(callback);
}

Object c_FrozenSet::t_zip(CVarRef iterable) {
  return BaseSet::phpZip<c_FrozenSet>(iterable);
}

Object c_FrozenSet::ti_fromitems(CVarRef iterable) {
  return BaseSet::phpFromItems<c_FrozenSet>(iterable);
}

Object c_FrozenSet::ti_fromarrays(int _argc, CArrRef _argv) {
  return BaseSet::phpFromArrays<c_FrozenSet>(_argc, _argv);
}

c_FrozenSet::c_FrozenSet(Class* cls) : BaseSet(cls) {
  ObjectData::setAttributes(kCollectionObjectDataAttrs);
  o_subclassData.u16 = Collection::FrozenSetType;
}

void c_FrozenSet::Unserialize(ObjectData* obj, VariableUnserializer* uns,
    int64_t sz, char type) {
  BaseSet::Unserialize("FrozenSet", obj, uns, sz, type);
}

c_FrozenSet* c_FrozenSet::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_FrozenSet>(obj);
}


///////////////////////////////////////////////////////////////////////////////

c_SetIterator::c_SetIterator(Class* cb) :
    ExtObjectData(cb) {
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
  Object e(SystemLib::AllocRuntimeExceptionObject(
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
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(2);
  vec->m_size = 2;
  vec->m_data[0].m_data.num = 0;
  vec->m_data[0].m_type = KindOfInt64;
  vec->m_data[1].m_data.num = 1;
  vec->m_data[1].m_type = KindOfInt64;
  return obj;
}

Object c_Pair::t_values() {
  c_Vector* vec;
  Object o = vec = NEWOBJ(c_Vector)();
  vec->init(VarNR(this));
  return o;
}

Object c_Pair::t_lazy() {
  assert(isFullyConstructed());
  return SystemLib::AllocLazyKeyedIterableViewObject(this);
}

Object c_Pair::t_kvzip() {
  assert(isFullyConstructed());
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(2);
  for (uint i = 0; i < 2; ++i) {
    c_Pair* pair = NEWOBJ(c_Pair)();
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

Variant c_Pair::t_at(CVarRef key) {
  assert(isFullyConstructed());
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return init_null_variant;
}

Variant c_Pair::t_get(CVarRef key) {
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

bool c_Pair::t_containskey(CVarRef key) {
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
  c_PairIterator* it = NEWOBJ(c_PairIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  return it;
}

Object c_Pair::t_map(CVarRef callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(2);
  for (uint64_t i = 0; i < 2; ++i) {
    g_vmContext->invokeFuncFew(&vec->m_data[i], ctx, 1, &getElms()[i]);
    ++vec->m_size;
  }
  return obj;
}

Object c_Pair::t_mapwithkey(CVarRef callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(2);
  for (uint64_t i = 0; i < 2; ++i) {
    TypedValue args[2] = { make_tv<KindOfInt64>(i), getElms()[i] };
    g_vmContext->invokeFuncFew(&vec->m_data[i], ctx, 2, args);
    ++vec->m_size;
  }
  return obj;
}

Object c_Pair::t_filter(CVarRef callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  for (uint64_t i = 0; i < 2; ++i) {
    Variant ret;
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 1, &getElms()[i]);
    if (ret.toBoolean()) {
      vec->add(&getElms()[i]);
    }
  }
  return obj;
}

Object c_Pair::t_filterwithkey(CVarRef callback) {
  assert(isFullyConstructed());
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  for (uint64_t i = 0; i < 2; ++i) {
    Variant ret;
    TypedValue args[2] = { make_tv<KindOfInt64>(i), getElms()[i] };
    g_vmContext->invokeFuncFew(ret.asTypedValue(), ctx, 2, args);
    if (ret.toBoolean()) {
      vec->add(&getElms()[i]);
    }
  }
  return obj;
}

Object c_Pair::t_zip(CVarRef iterable) {
  assert(isFullyConstructed());
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(std::min(sz, size_t(2)));
  for (uint64_t i = 0; i < 2 && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (vec->m_capacity <= vec->m_size) {
      vec->grow();
    }
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->initAdd(&getElms()[i]);
    pair->initAdd(cvarToCell(&v));
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
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

TypedValue* c_Pair::OffsetGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assert(pair->isFullyConstructed());
  if (key->m_type == KindOfInt64) {
    return pair->at(key->m_data.num);
  }
  throwBadKeyType();
  return nullptr;
}

void c_Pair::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assert(static_cast<c_Pair*>(obj)->isFullyConstructed());
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot assign to an element of a Pair"));
  throw e;
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

void c_Pair::OffsetUnset(ObjectData* obj, TypedValue* key) {
  assert(static_cast<c_Pair*>(obj)->isFullyConstructed());
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot unset an element of a Pair"));
  throw e;
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
COLLECTION_MAGIC_METHODS(FrozenVector)
COLLECTION_MAGIC_METHODS(Map)
COLLECTION_MAGIC_METHODS(StableMap)
COLLECTION_MAGIC_METHODS(Set)
COLLECTION_MAGIC_METHODS(FrozenSet)
COLLECTION_MAGIC_METHODS(Pair)

#undef COLLECTION_MAGIC_METHODS

#define ITERABLE_MATERIALIZE_METHODS(cls) \
  Object c_##cls::t_tovector() { \
    c_Vector* vec; \
    Object o = vec = NEWOBJ(c_Vector)(); \
    vec->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_tofrozenvector() { \
    c_FrozenVector* fv; \
    Object o = fv = NEWOBJ(c_FrozenVector)(); \
    fv->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_toset() { \
    c_Set* st; \
    Object o = st = NEWOBJ(c_Set)(); \
    st->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_tofrozenset() { \
    c_FrozenSet* st; \
    Object o = st = NEWOBJ(c_FrozenSet)(); \
    st->init(VarNR(this)); \
    return o; \
  }

#define KEYEDITERABLE_MATERIALIZE_METHODS(cls) \
  ITERABLE_MATERIALIZE_METHODS(cls) \
  Object c_##cls::t_tomap() { \
    c_Map* mp; \
    Object o = mp = NEWOBJ(c_Map)(); \
    mp->init(VarNR(this)); \
    return o; \
  } \
  Object c_##cls::t_tostablemap() { \
    c_StableMap* smp; \
    Object o = smp = NEWOBJ(c_StableMap)(); \
    smp->init(VarNR(this)); \
    return o; \
  }

KEYEDITERABLE_MATERIALIZE_METHODS(Map)
KEYEDITERABLE_MATERIALIZE_METHODS(StableMap)
ITERABLE_MATERIALIZE_METHODS(Set)
ITERABLE_MATERIALIZE_METHODS(FrozenSet)
KEYEDITERABLE_MATERIALIZE_METHODS(Pair)
KEYEDITERABLE_MATERIALIZE_METHODS(FrozenVector)

#undef ITERABLE_MATERIALIZE_METHODS
#undef KEYEDITERABLE_MATERIALIZE_METHODS

static inline bool isKeylessCollectionType(Collection::Type ctype) {
  return ctype == Collection::SetType ||
         ctype == Collection::FrozenSetType;
}

void collectionSerialize(ObjectData* obj, VariableSerializer* serializer) {
  assert(obj->isCollection());
  int64_t sz = obj->getCollectionSize();
  if (obj->getCollectionType() == Collection::VectorType ||
      obj->getCollectionType() == Collection::FrozenVectorType ||
      obj->getCollectionType() == Collection::SetType ||
      obj->getCollectionType() == Collection::FrozenSetType ||
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
    assert(obj->getCollectionType() == Collection::MapType ||
           obj->getCollectionType() == Collection::StableMapType);
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
        case Collection::StableMapType:
          obj = collectionDeepCopyStableMap(static_cast<c_StableMap*>(obj));
          break;
        case Collection::SetType:
          obj = collectionDeepCopySet(static_cast<c_Set*>(obj));
          break;
        case Collection::PairType:
          obj = collectionDeepCopyPair(static_cast<c_Pair*>(obj));
          break;
        case Collection::FrozenSetType:
          obj = collectionDeepCopyFrozenSet(static_cast<c_FrozenSet*>(obj));
          break;
        case Collection::FrozenVectorType:
          obj = collectionDeepCopyFrozenVector(
                  static_cast<c_FrozenVector*>(obj));
          break;
        default:
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
static ObjectData* collectionDeepCopyBaseVector(TVector *vec) {
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

ObjectData* collectionDeepCopyFrozenVector(c_FrozenVector* vec) {
  return collectionDeepCopyBaseVector<c_FrozenVector>(vec);
}

ObjectData* collectionDeepCopyMap(c_Map* mp) {
  mp = c_Map::Clone(mp);
  Object o = Object::attach(mp);
  uint used = mp->iterLimit();
  for (uint32_t i = 0; i < used; ++i) {
    if (mp->isTombstone(i)) continue;
    c_Map::Elm* p = &mp->data()[i];
    collectionDeepCopyTV(&p->data);
  }
  return o.detach();
}

ObjectData* collectionDeepCopyStableMap(c_StableMap* smp) {
  smp = c_StableMap::Clone(smp);
  Object o = Object::attach(smp);
  for (c_StableMap::Bucket* p = smp->m_pListHead; p; p = p->pListNext) {
    collectionDeepCopyTV(&p->data);
  }
  return o.detach();
}

ObjectData* collectionDeepCopySet(c_Set* st) {
  return c_Set::Clone(st);
}

ObjectData* collectionDeepCopyFrozenSet(c_FrozenSet* st) {
  return c_FrozenSet::Clone(st);
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
// error messages (e.g. collectionSet() when called on an immutable collection).
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

/**
 * Construct the error message given its type and a collection name.
 */
static std::string getErrMsg(ErrMsgType errType, const std::string& colName) {
  std::string msgBody;

  switch (errType) {
  case ErrMsgType::CannotAssign:
    msgBody = "Cannot assign to an element of a";
    break;
  case ErrMsgType::CannotUnset:
    msgBody = "Cannot unset an element of a";
    break;
  case ErrMsgType::CannotAdd:
    msgBody = "Cannot add an element to a";
    break;
  case ErrMsgType::OnlyIntKeys:
    msgBody = "Only integer keys may be used with";
    break;
  default:
    assert(false);
  }

  return msgBody + " " + colName;
}

/**
 * Throws an exception of the given type.
 */
static void collectionThrowHelper(ErrMsgType errType,
    const std::string& colName) {

  std::function<ObjectData*(std::string)> excAlloc = nullptr;

  switch (errType) {
  case ErrMsgType::CannotAssign:
  case ErrMsgType::CannotUnset:
  case ErrMsgType::CannotAdd:
    excAlloc = SystemLib::AllocRuntimeExceptionObject;
    break;
  case ErrMsgType::OnlyIntKeys:
    excAlloc = SystemLib::AllocInvalidArgumentExceptionObject;
    break;
  default:
    assert(false);
  }

  Object exc = excAlloc(getErrMsg(errType, colName));
  throw exc;
}

///////////////////////////////////////////////////////////////////////////////


TypedValue* collectionGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      return c_Vector::OffsetGet(obj, key);
    case Collection::MapType:
      return c_Map::OffsetGet(obj, key);
    case Collection::StableMapType:
      return c_StableMap::OffsetGet(obj, key);
    case Collection::SetType:
      return c_Set::OffsetGet(obj, key);
    case Collection::PairType:
      return c_Pair::OffsetGet(obj, key);
    case Collection::FrozenVectorType:
      return c_FrozenVector::OffsetGet(obj, key);
    case Collection::FrozenSetType:
      return c_FrozenSet::OffsetGet(obj, key);
    default:
      assert(false);
      return nullptr;
  }
}

void collectionSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      c_Vector::OffsetSet(obj, key, val);
      break;
    case Collection::MapType:
      c_Map::OffsetSet(obj, key, val);
      break;
    case Collection::StableMapType:
      c_StableMap::OffsetSet(obj, key, val);
      break;
    case Collection::SetType:
      c_Set::OffsetSet(obj, key, val);
      break;
    case Collection::PairType:
      c_Pair::OffsetSet(obj, key, val);
    case Collection::FrozenSetType:
      c_FrozenSet::OffsetSet(obj, key, val);
      break;
    case Collection::FrozenVectorType:
      collectionThrowHelper(ErrMsgType::CannotAssign, "FrozenVector");
      break;
    default:
      assert(false);
  }
}

bool collectionIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      return c_Vector::OffsetIsset(obj, key);
    case Collection::MapType:
      return c_Map::OffsetIsset(obj, key);
    case Collection::StableMapType:
      return c_StableMap::OffsetIsset(obj, key);
    case Collection::SetType:
      return c_Set::OffsetIsset(obj, key);
    case Collection::PairType:
      return c_Pair::OffsetIsset(obj, key);
    case Collection::FrozenVectorType:
      return c_FrozenVector::OffsetIsset(obj, key);
    case Collection::FrozenSetType:
      return c_FrozenSet::OffsetIsset(obj, key);
    default:
      assert(false);
      return false;
  }
}

bool collectionEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      return c_Vector::OffsetEmpty(obj, key);
    case Collection::MapType:
      return c_Map::OffsetEmpty(obj, key);
    case Collection::StableMapType:
      return c_StableMap::OffsetEmpty(obj, key);
    case Collection::SetType:
      return c_Set::OffsetEmpty(obj, key);
    case Collection::PairType:
      return c_Pair::OffsetEmpty(obj, key);
    case Collection::FrozenVectorType:
      return c_FrozenVector::OffsetEmpty(obj, key);
    case Collection::FrozenSetType:
      return c_FrozenSet::OffsetEmpty(obj, key);
    default:
      assert(false);
      return false;
  }
}

void collectionUnset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      c_Vector::OffsetUnset(obj, key);
      break;
    case Collection::MapType:
      c_Map::OffsetUnset(obj, key);
      break;
    case Collection::StableMapType:
      c_StableMap::OffsetUnset(obj, key);
      break;
    case Collection::SetType:
      c_Set::OffsetUnset(obj, key);
      break;
    case Collection::PairType:
      c_Pair::OffsetUnset(obj, key);
      break;
    case Collection::FrozenSetType:
      c_FrozenSet::OffsetUnset(obj, key);
      break;
    case Collection::FrozenVectorType:
      collectionThrowHelper(ErrMsgType::CannotUnset, "FrozenVector");
      break;
    default:
      assert(false);
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
    case Collection::StableMapType:
      static_cast<c_StableMap*>(obj)->add(val);
      break;
    case Collection::SetType:
      static_cast<c_Set*>(obj)->add(val);
      break;
    case Collection::PairType:
      assert(static_cast<c_Pair*>(obj)->isFullyConstructed());
      collectionThrowHelper(ErrMsgType::CannotAdd, "Pair");
      break;
    case Collection::FrozenSetType:
      collectionThrowHelper(ErrMsgType::CannotAdd, "FrozenSet");
      break;
    case Collection::FrozenVectorType:
      collectionThrowHelper(ErrMsgType::CannotAdd, "FrozenVector");
      break;
    default:
      assert(false);
  }
}

void collectionInitAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      static_cast<c_Vector*>(obj)->add(val);
      break;
    case Collection::MapType:
      static_cast<c_Map*>(obj)->add(val);
      break;
    case Collection::StableMapType:
      static_cast<c_StableMap*>(obj)->add(val);
      break;
    case Collection::SetType:
      static_cast<c_Set*>(obj)->add(val);
      break;
    case Collection::PairType:
      static_cast<c_Pair*>(obj)->initAdd(val);
      break;
    case Collection::FrozenVectorType:
      static_cast<c_FrozenVector*>(obj)->add(val);
      break;
    case Collection::FrozenSetType:
      static_cast<c_FrozenSet*>(obj)->add(val);
      break;
    default:
      assert(false);
  }
}

Variant& collectionOffsetGet(ObjectData* obj, int64_t offset) {
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      return tvAsVariant(static_cast<c_Vector*>(obj)->at(offset));
    case Collection::MapType:
      return tvAsVariant(static_cast<c_Map*>(obj)->at(offset));
    case Collection::StableMapType:
      return tvAsVariant(static_cast<c_StableMap*>(obj)->at(offset));
    case Collection::SetType:
      c_Set::throwNoIndexAccess();
    case Collection::PairType:
      return tvAsVariant(static_cast<c_Pair*>(obj)->at(offset));
    case Collection::FrozenSetType:
      c_FrozenSet::throwNoIndexAccess();
      break;
    case Collection::FrozenVectorType:
      return tvAsVariant(static_cast<c_FrozenVector*>(obj)->at(offset));
    default:
      assert(false);
      return tvAsVariant(nullptr);
  }
}

Variant& collectionOffsetGet(ObjectData* obj, const String& offset) {
  StringData* key = offset.get();
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      collectionThrowHelper(ErrMsgType::OnlyIntKeys, "Vectors");
      break;
    case Collection::MapType:
      return tvAsVariant(static_cast<c_Map*>(obj)->at(key));
    case Collection::StableMapType:
      return tvAsVariant(static_cast<c_StableMap*>(obj)->at(key));
    case Collection::SetType:
      c_Set::throwNoIndexAccess();
      break;
    case Collection::PairType:
      collectionThrowHelper(ErrMsgType::OnlyIntKeys, "Pairs");
      break;
    case Collection::FrozenSetType:
      c_FrozenSet::throwNoIndexAccess();
      break;
    case Collection::FrozenVectorType:
      collectionThrowHelper(ErrMsgType::OnlyIntKeys, "FrozenVectors");
      break;
    default:
      // Do nothing: we fail below.
      ;
  }
  assert(false);
  return tvAsVariant(nullptr);
}

Variant& collectionOffsetGet(ObjectData* obj, CVarRef offset) {
  TypedValue* key = cvarToCell(&offset);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      return tvAsVariant(c_Vector::OffsetGet(obj, key));
    case Collection::MapType:
      return tvAsVariant(c_Map::OffsetGet(obj, key));
    case Collection::StableMapType:
      return tvAsVariant(c_StableMap::OffsetGet(obj, key));
    case Collection::SetType:
      return tvAsVariant(c_Set::OffsetGet(obj, key));
    case Collection::PairType:
      return tvAsVariant(c_Pair::OffsetGet(obj, key));
    case Collection::FrozenSetType:
      return tvAsVariant(c_FrozenSet::OffsetGet(obj, key));
    case Collection::FrozenVectorType:
      return tvAsVariant(c_FrozenVector::OffsetGet(obj, key));
    default:
      assert(false);
      return tvAsVariant(nullptr);
  }
}

void collectionOffsetSet(ObjectData* obj, int64_t offset, CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  if (UNLIKELY(tv->m_type == KindOfUninit)) {
    tv = (TypedValue*)(&init_null_variant);
  }
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      static_cast<c_Vector*>(obj)->set(offset, tv);
      break;
    case Collection::MapType:
      static_cast<c_Map*>(obj)->set(offset, tv);
      break;
    case Collection::StableMapType:
      static_cast<c_StableMap*>(obj)->set(offset, tv);
      break;
    case Collection::SetType:
      c_Set::throwNoIndexAccess();
      break;
    case Collection::PairType:
      collectionThrowHelper(ErrMsgType::CannotAssign, "Pair");
      break;
    case Collection::FrozenSetType:
      c_FrozenSet::throwNoIndexAccess();
      break;
   case Collection::FrozenVectorType:
      collectionThrowHelper(ErrMsgType::CannotAssign, "FrozenVector");
      break;
    default:
      assert(false);
  }
}

void collectionOffsetSet(ObjectData* obj, const String& offset, CVarRef val) {
  StringData* key = offset.get();
  TypedValue* tv = cvarToCell(&val);
  if (UNLIKELY(tv->m_type == KindOfUninit)) {
    tv = (TypedValue*)(&init_null_variant);
  }
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      collectionThrowHelper(ErrMsgType::OnlyIntKeys, "Vectors");
      break;
    case Collection::MapType:
      static_cast<c_Map*>(obj)->set(key, tv);
      break;
    case Collection::StableMapType:
      static_cast<c_StableMap*>(obj)->set(key, tv);
      break;
    case Collection::SetType:
      c_Set::throwNoIndexAccess();
      break;
    case Collection::PairType:
      collectionThrowHelper(ErrMsgType::CannotAssign, "Pair");
      break;
    case Collection::FrozenSetType:
      c_FrozenSet::throwNoIndexAccess();
      break;
   case Collection::FrozenVectorType:
      collectionThrowHelper(ErrMsgType::CannotAssign, "FrozenVector");
      break;
    default:
      assert(false);
  }
}

void collectionOffsetSet(ObjectData* obj, CVarRef offset, CVarRef val) {
  TypedValue* key = cvarToCell(&offset);
  TypedValue* tv = cvarToCell(&val);
  if (UNLIKELY(tv->m_type == KindOfUninit)) {
    tv = (TypedValue*)(&init_null_variant);
  }
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      c_Vector::OffsetSet(obj, key, tv);
      break;
    case Collection::MapType:
      c_Map::OffsetSet(obj, key, tv);
      break;
    case Collection::StableMapType:
      c_StableMap::OffsetSet(obj, key, tv);
      break;
    case Collection::SetType:
      c_Set::OffsetSet(obj, key, tv);
      break;
    case Collection::PairType:
      c_Pair::OffsetSet(obj, key, tv);
      break;
    case Collection::FrozenSetType:
      c_FrozenSet::OffsetSet(obj, key, tv);
      break;
    case Collection::FrozenVectorType:
      collectionThrowHelper(ErrMsgType::CannotAssign, "FrozenVector");
      break;
    default:
      assert(false);
  }
}

bool collectionOffsetContains(ObjectData* obj, CVarRef offset) {
  TypedValue* key = cvarToCell(&offset);
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      return c_Vector::OffsetContains(obj, key);
    case Collection::MapType:
      return c_Map::OffsetContains(obj, key);
    case Collection::StableMapType:
      return c_StableMap::OffsetContains(obj, key);
    case Collection::SetType:
      return c_Set::OffsetContains(obj, key);
    case Collection::PairType:
      return c_Pair::OffsetContains(obj, key);
    case Collection::FrozenVectorType:
      return c_FrozenVector::OffsetContains(obj, key);
    case Collection::FrozenSetType:
      return c_FrozenSet::OffsetContains(obj, key);
    default:
      assert(false);
      return false;
  }
}

void collectionReserve(ObjectData* obj, int64_t sz) {
  switch (obj->getCollectionType()) {
    case Collection::VectorType:
      static_cast<c_Vector*>(obj)->reserve(sz);
      break;
    case Collection::MapType:
      static_cast<c_Map*>(obj)->reserve(sz);
      break;
    case Collection::StableMapType:
      static_cast<c_StableMap*>(obj)->reserve(sz);
      break;
    case Collection::SetType:
      static_cast<c_Set*>(obj)->reserve(sz);
      break;
    case Collection::PairType:
      // do nothing
      break;
    case Collection::FrozenVectorType:
      static_cast<c_FrozenVector*>(obj)->reserve(sz);
      break;
    case Collection::FrozenSetType:
      static_cast<c_FrozenSet*>(obj)->reserve(sz);
      break;
    default:
      assert(false);
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
      c_Map::Unserialize(obj, uns, sz, type);
      break;
    case Collection::StableMapType:
      c_StableMap::Unserialize(obj, uns, sz, type);
      break;
    case Collection::SetType:
      c_Set::Unserialize(obj, uns, sz, type);
      break;
    case Collection::PairType:
      c_Pair::Unserialize(obj, uns, sz, type);
      break;
    case Collection::FrozenVectorType:
      c_FrozenVector::Unserialize(obj, uns, sz, type);
      break;
    case Collection::FrozenSetType:
      c_FrozenSet::Unserialize(obj, uns, sz, type);
      break;
    default:
      assert(false);
  }
}

bool collectionEquals(const ObjectData* obj1, const ObjectData* obj2) {
  int ct = obj1->getCollectionType();
  assert(ct == obj2->getCollectionType());
  switch (ct) {
    case Collection::VectorType:
      return c_Vector::Equals(obj1, obj2);
    case Collection::MapType:
      return c_Map::Equals(obj1, obj2);
    case Collection::StableMapType:
      return c_StableMap::Equals(obj1, obj2);
    case Collection::SetType:
      return c_Set::Equals(obj1, obj2);
    case Collection::PairType:
      return c_Pair::Equals(obj1, obj2);
    case Collection::FrozenVectorType:
      return c_FrozenVector::Equals(obj1, obj2);
    case Collection::FrozenSetType:
      return c_FrozenSet::Equals(obj1, obj2);
    default:
      assert(false);
      return false;
  }
}

ObjectData* newCollectionHelper(uint32_t type, uint32_t size) {
  ObjectData* obj;
  switch (type) {
    case Collection::VectorType: obj = NEWOBJ(c_Vector)(); break;
    case Collection::MapType: obj = NEWOBJ(c_Map)(); break;
    case Collection::StableMapType: obj = NEWOBJ(c_StableMap)(); break;
    case Collection::SetType: obj = NEWOBJ(c_Set)(); break;
    case Collection::PairType: obj = NEWOBJ(c_Pair)(); break;
    case Collection::FrozenVectorType: obj = NEWOBJ(c_FrozenVector)(); break;
    case Collection::FrozenSetType: obj = NEWOBJ(c_FrozenSet)(); break;
    default:
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
