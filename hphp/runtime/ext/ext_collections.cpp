/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/ext/ext_collections.h"
#include "runtime/base/variable_serializer.h"
#include "runtime/base/array/sort_helpers.h"
#include "runtime/ext/ext_array.h"
#include "runtime/ext/ext_math.h"
#include "runtime/ext/ext_intl.h"
#include "runtime/vm/translator/translator-inline.h"
#include "system/lib/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static void throwIntOOB(int64_t key, bool isVector = false) ATTRIBUTE_COLD
  ATTRIBUTE_NORETURN;

void throwIntOOB(int64_t key, bool isVector /* = false */) {
  static const size_t reserveSize = 50;
  String msg(reserveSize, ReserveString);
  char* buf = msg.mutableSlice().ptr;
  int sz = sprintf(buf, "Integer key %" PRId64 " is %s", key,
                   isVector ? "out of bounds" : "not defined");
  assert(sz <= reserveSize);
  msg.setSize(sz);
  Object e(SystemLib::AllocOutOfBoundsExceptionObject(msg));
  throw e;
}

static void throwStrOOB(StringData* key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

void throwStrOOB(StringData* key) {
  const size_t maxDisplaySize = 20;
  const char* dots = "...";
  size_t dotsSize = strlen(dots);
  int keySize = key->size();
  bool keyIsLarge = (keySize > maxDisplaySize);
  size_t displaySize = keyIsLarge ? (maxDisplaySize - dotsSize) : keySize;
  const char* part1 = "String key \"";
  size_t part1Size = strlen(part1);
  size_t part2Size = keyIsLarge ? maxDisplaySize : keySize;
  const char* part3 = "\" is not defined";
  size_t part3Size = strlen(part3);
  // Do some math ahead of time so we know exactly how large
  // the String needs to be
  String msg(part1Size + part2Size + part3Size, ReserveString);
  msg += StringSlice(part1, part1Size);
  msg += StringSlice(key->data(), displaySize);
  if (keyIsLarge) msg += StringSlice(dots, dotsSize);
  msg += StringSlice(part3, part3Size);
  Object e(SystemLib::AllocOutOfBoundsExceptionObject(msg));
  throw e;
}

static inline ArrayIter getArrayIterHelper(CVarRef v, size_t& sz) {
  if (v.isArray()) {
    ArrayData* ad = v.getArrayData();
    sz = ad->size();
    return ArrayIter(ad);
  }
  if (v.isObject()) {
    ObjectData* obj = v.getObjectData();
    if (obj->isCollection()) {
      sz = collectionSize(obj);
      return ArrayIter(obj);
    }
    bool isIterable;
    Object iterable = obj->iterableObject(isIterable);
    if (isIterable) {
      sz = 0;
      return ArrayIter(iterable, ArrayIter::transferOwner);
    }
  }
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Parameter must be an array or an instance of Traversable"));
  throw e;
}

///////////////////////////////////////////////////////////////////////////////

c_Vector::c_Vector(VM::Class* cb) :
    ExtObjectDataFlags<ObjectData::VectorAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_data(nullptr), m_size(0), m_capacity(0), m_version(0) {
}

c_Vector::~c_Vector() {
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    tvRefcountedDecRef(&m_data[i]);
  }
  c_Vector::freeData();
}

void c_Vector::freeData() {
  if (m_data) {
    smart_free(m_data);
    m_data = nullptr;
  }
}

void c_Vector::t___construct(CVarRef iterable /* = null_variant */) {
  if (!iterable.isInitialized()) {
    return;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  if (sz) {
    reserve(sz);
  }
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    add(tv);
  }
}

void c_Vector::grow() {
  if (m_capacity) {
    m_capacity += m_capacity;
  } else {
    m_capacity = 8;
  }
  m_data = (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
}

void c_Vector::resize(int64_t sz, TypedValue* val) {
  assert(val && val->m_type != KindOfRef);
  ++m_version;
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
      tvDup(val, &m_data[m_size]);
    }
  }
}

void c_Vector::reserve(int64_t sz) {
  if (sz <= 0) return;
  if (m_capacity < sz) {
    ++m_version;
    m_capacity = sz;
    m_data =
      (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
  }
}

Array c_Vector::toArrayImpl() const {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    ai.set(tvAsCVarRef(&m_data[i]));
  }
  return ai.create();
}

Array c_Vector::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_Vector::clone() {
  ObjectData* obj = ObjectData::clone();
  auto target = static_cast<c_Vector*>(obj);
  uint sz = m_size;
  TypedValue* data;
  target->m_capacity = target->m_size = sz;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  for (int i = 0; i < sz; ++i) {
    tvDup(&m_data[i], &data[i]);
  }
  return obj;
}

Object c_Vector::t_add(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_Vector::t_addall(CVarRef iterable) {
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

Object c_Vector::t_clear() {
  ++m_version;
  uint sz = m_size;
  for (int i = 0; i < sz; ++i) {
    tvRefcountedDecRef(&m_data[i]);
  }
  smart_free(m_data);
  m_data = nullptr;
  m_size = 0;
  m_capacity = 0;
  return this;
}

bool c_Vector::t_isempty() {
  return (m_size == 0);
}

int64_t c_Vector::t_count() {
  return m_size;
}

Object c_Vector::t_items() {
  return SystemLib::AllocIterableViewObject(this);
}

Object c_Vector::t_keys() {
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

Object c_Vector::t_view() {
  return SystemLib::AllocKeyedIterableViewObject(this);
}

Object c_Vector::t_kvzip() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  for (uint i = 0; i < m_size; ++i) {
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->elm0.m_type = KindOfInt64;
    pair->elm0.m_data.num = i;
    ++pair->m_size;
    pair->add(&m_data[i]);
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
  }
  return obj;
}

Variant c_Vector::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return uninit_null();
}

Variant c_Vector::t_get(CVarRef key) {
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

bool c_Vector::t_contains(CVarRef key) {
  return t_containskey(key);
}

bool c_Vector::t_containskey(CVarRef key) {
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

Object c_Vector::t_removekey(CVarRef key) {
  if (!key.isInteger()) {
    throwBadKeyType();
  }
  int64_t k = key.toInt64();
  if (!contains(k)) {
    return this;
  }
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
  return toArrayImpl();
}

void c_Vector::t_sort(CVarRef col /* = null */) {
  raise_warning("Vector::sort() is deprecated, please use the builtin sort() "
                "function or usort() function instead");
  // Terribly inefficient, but produces correct results for now
  Variant arr = t_toarray();
  if (col.isNull()) {
    f_sort(ref(arr));
  } else {
    if (!col.isObject()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected col to be an instance of Collator"));
      throw e;
    }
    ObjectData* obj = col.getObjectData();
    if (!obj->instanceof(c_Collator::s_cls)) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected col to be an instance of Collator"));
      throw e;
    }
    auto collator = static_cast<c_Collator*>(obj);
    // TODO Task #1429976: What do we do if the Collator encountered errors
    // while sorting? How is this reported to the user?
    collator->t_sort(ref(arr));
  }
  if (!arr.isArray()) {
    assert(false);
    return;
  }
  ArrayData* ad = arr.getArrayData();
  int sz = ad->size();
  ssize_t pos = ad->iter_begin();
  for (int i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    assert(pos != ArrayData::invalid_index);
    tvAsVariant(&m_data[i]) = ad->getValue(pos);
  }
}

void c_Vector::t_reverse() {
  if (m_size <= 1) return;
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
  uint sz = m_size;
  for (uint i = 0; i < sz; ++i) {
    if (search_value.same(tvAsCVarRef(&m_data[i]))) {
      return i;
    }
  }
  return -1;
}

void c_Vector::t_shuffle() {
  for (uint i = 1; i < m_size; ++i) {
    uint j = f_mt_rand(0, i);
    std::swap(m_data[i], m_data[j]);
  }
}

Object c_Vector::t_getiterator() {
  c_VectorIterator* it = NEWOBJ(c_VectorIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  it->m_version = getVersion();
  return it;
}

Object c_Vector::t_map(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  for (uint i = 0; i < m_size; ++i) {
    TypedValue* tv = &vec->m_data[i];
    int32_t version = m_version;
    g_vmContext->invokeFunc(tv, ctx, CREATE_VECTOR1(tvAsCVarRef(&m_data[i])));
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    ++vec->m_size;
  }
  return obj;
}

Object c_Vector::t_filter(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  for (uint i = 0; i < m_size; ++i) {
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFunc(ret.asTypedValue(), ctx,
                            CREATE_VECTOR1(tvAsCVarRef(&m_data[i])));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (ret.toBoolean()) {
      vec->add(&m_data[i]);
    }
  }
  return obj;
}

Object c_Vector::t_zip(CVarRef iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(std::min(sz, size_t(m_size)));
  for (uint i = 0; i < m_size && iter; ++i, ++iter) {
    Variant v = iter.second();
    if (vec->m_capacity <= vec->m_size) {
      vec->grow();
    }
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->add(&m_data[i]);
    pair->add(cvarToCell(&v));
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
  }
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
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  for (uint i = 0; iter; ++i, ++iter) {
    Variant v = iter.second();
    TypedValue* tv = tvToCell(v.asTypedValue());
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
  target->m_capacity = target->m_size = sz;
  TypedValue* data;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  ssize_t pos = ad->iter_begin();
  for (uint i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    assert(pos != ArrayData::invalid_index);
    TypedValue* tv = cvarToCell(&ad->getValueRef(pos));
    tvRefcountedIncRef(tv);
    data[i].m_data.num = tv->m_data.num;
    data[i].m_type = tv->m_type;
  }
  return ret;
}

Object c_Vector::ti_fromvector(CVarRef vec) {
  if (!vec.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
  ObjectData* obj = vec.getObjectData();
  if (!obj->instanceof(c_Vector::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
  auto v = static_cast<c_Vector*>(obj);
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  uint sz = v->m_size;
  TypedValue* data;
  target->m_capacity = target->m_size = sz;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  for (uint i = 0; i < sz; ++i) {
    tvDup(&v->m_data[i], &data[i]);
  }
  return ret;
}

Variant c_Vector::ti_slice(CVarRef vec, CVarRef offset,
                           CVarRef len /* = null */) {
  if (!vec.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
  ObjectData* obj = vec.getObjectData();
  if (!obj->instanceof(c_Vector::s_cls)) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
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
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  auto v = static_cast<c_Vector*>(obj);
  int64_t sz = v->m_size;
  int64_t startPos = offset.toInt64();
  if (UNLIKELY(uint64_t(startPos) >= uint64_t(sz))) {
    if (startPos >= 0) {
      return ret;
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
        return ret;
      }
      endPos = sz + intLen;
      if (endPos <= startPos) {
        return ret;
      }
    }
  } else {
    endPos = sz;
  }
  assert(startPos < endPos);
  uint targetSize = endPos - startPos;
  TypedValue* data;
  target->m_capacity = target->m_size = targetSize;
  target->m_data = data =
    (TypedValue*)smart_malloc(targetSize * sizeof(TypedValue));
  for (uint i = 0; i < targetSize; ++i, ++startPos) {
    tvDup(&v->m_data[startPos], &data[i]);
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
  if (!m_size) {
    return;
  }
  SortFlavor flav = preSort<VectorValAccessor>(VectorValAccessor());
  CALL_SORT(VectorValAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

void c_Vector::usort(CVarRef cmp_function) {
  if (!m_size) {
    return;
  }
  ElmUCompare<VectorValAccessor> comp;
  CallCtx ctx;
  VM::Transl::CallerFrame cf;
  vm_decode_function(cmp_function, cf(), false, ctx);
  comp.ctx = &ctx;
  HPHP::Sort::sort(m_data, m_data + m_size, comp);
}

void c_Vector::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys may be used with Vectors"));
  throw e;
}

TypedValue* c_Vector::OffsetGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<c_Vector*>(obj);
  if (key->m_type == KindOfInt64) {
    return vec->at(key->m_data.num);
  }
  throwBadKeyType();
  return nullptr;
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

bool c_Vector::OffsetIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<c_Vector*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = vec->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? isset(tvAsCVarRef(result)) : false;
}

bool c_Vector::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<c_Vector*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = vec->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? empty(tvAsCVarRef(result)) : true;
}

bool c_Vector::OffsetContains(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto vec = static_cast<c_Vector*>(obj);
  if (key->m_type == KindOfInt64) {
    return vec->contains(key->m_data.num);
  } else {
    throwBadKeyType();
    return false;
  }
}

void c_Vector::OffsetAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  auto vec = static_cast<c_Vector*>(obj);
  vec->add(val);
}

void c_Vector::OffsetUnset(ObjectData* obj, TypedValue* key) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot unset element of a Vector"));
  throw e;
}

bool c_Vector::Equals(ObjectData* obj1, ObjectData* obj2) {
  auto vec1 = static_cast<c_Vector*>(obj1);
  auto vec2 = static_cast<c_Vector*>(obj2);
  uint sz = vec1->m_size;
  if (sz != vec2->m_size) {
    return false;
  }
  for (uint i = 0; i < sz; ++i) {
    if (!equal(tvAsCVarRef(&vec1->m_data[i]),
               tvAsCVarRef(&vec2->m_data[i]))) {
      return false;
    }
  }
  return true;
}

void c_Vector::Unserialize(ObjectData* obj,
                           VariableUnserializer* uns,
                           int64_t sz,
                           char type) {
  if (type != 'V') {
    throw Exception("Vector does not support the '%c' serialization "
                    "format", type);
  }
  auto vec = static_cast<c_Vector*>(obj);
  vec->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    auto tv = &vec->m_data[vec->m_size];
    tv->m_type = KindOfNull;
    ++vec->m_size;
    tvAsVariant(tv).unserialize(uns, Uns::ColValueMode);
  }
}

c_VectorIterator::c_VectorIterator(VM::Class* cb) :
    ExtObjectData(cb) {
}

c_VectorIterator::~c_VectorIterator() {
}

void c_VectorIterator::t___construct() {
}

Variant c_VectorIterator::t_current() {
  c_Vector* vec = m_obj.get();
  if (UNLIKELY(m_version != vec->getVersion())) {
    throw_collection_modified();
  }
  if (!vec->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(&vec->m_data[m_pos]);
}

Variant c_VectorIterator::t_key() {
  c_Vector* vec = m_obj.get();
  if (!vec->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return m_pos;
}

bool c_VectorIterator::t_valid() {
  assert(m_pos >= 0);
  c_Vector* vec = m_obj.get();
  return vec && (m_pos < (ssize_t)vec->m_size);
}

void c_VectorIterator::t_next() {
  m_pos++;
}

void c_VectorIterator::t_rewind() {
  m_pos = 0;
}

///////////////////////////////////////////////////////////////////////////////

static const char emptyMapSlot[sizeof(c_Map::Bucket)] = { 0 };

c_Map::c_Map(VM::Class* cb) :
    ExtObjectDataFlags<ObjectData::MapAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_size(0), m_load(0), m_nLastSlot(0), m_version(0) {
  m_data = (Bucket*)emptyMapSlot;
}

c_Map::~c_Map() {
  deleteBuckets();
  freeData();
}

void c_Map::freeData() {
  if (m_data != (Bucket*)emptyMapSlot) {
    smart_free(m_data);
  }
  m_data = (Bucket*)emptyMapSlot;
}

void c_Map::deleteBuckets() {
  if (!m_size) return;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      tvRefcountedDecRef(&p.data);
      if (p.hasStrKey() && p.skey->decRefCount() == 0) {
        DELETE(StringData)(p.skey);
      }
    }
  }
}

void c_Map::t___construct(CVarRef iterable /* = null_variant */) {
  if (!iterable.isInitialized()) {
    return;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  if (sz) {
    reserve(sz);
  }
  for (; iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = tvToCell(v.asTypedValue());
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else if (k.isString()) {
      update(k.getStringData(), tv);
    } else {
      throwBadKeyType();
    }
  }
}

Array c_Map::toArrayImpl() const {
  ArrayInit ai(m_size);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      if (p.hasIntKey()) {
        ai.set((int64_t)p.ikey, tvAsCVarRef(&p.data));
      } else {
        ai.set(*(const String*)(&p.skey), tvAsCVarRef(&p.data));
      }
    }
  }
  return ai.create();
}

Array c_Map::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_Map::clone() {
  ObjectData* obj = ObjectData::clone();
  auto target = static_cast<c_Map*>(obj);

  if (!m_size) return obj;

  assert(m_nLastSlot != 0);
  target->m_size = m_size;
  target->m_load = m_load;
  target->m_nLastSlot = m_nLastSlot;
  target->m_data = (Bucket*)smart_malloc(numSlots() * sizeof(Bucket));
  memcpy(target->m_data, m_data, numSlots() * sizeof(Bucket));

  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      tvRefcountedIncRef(&p.data);
      if (p.hasStrKey()) {
        p.skey->incRefCount();
      }
    }
  }

  return obj;
}

Object c_Map::t_add(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_Map::t_addall(CVarRef iterable) {
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
  deleteBuckets();
  freeData();
  m_size = 0;
  m_load = 0;
  m_nLastSlot = 0;
  m_data = (Bucket*)emptyMapSlot;
  return this;
}

bool c_Map::t_isempty() {
  return (m_size == 0);
}

int64_t c_Map::t_count() {
  return m_size;
}

Object c_Map::t_items() {
  return SystemLib::AllocKVZippedIterableObject(this);
}

Object c_Map::t_keys() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  for (uint i = 0, j = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    if (p.hasIntKey()) {
      vec->m_data[j].m_data.num = p.ikey;
      vec->m_data[j].m_type = KindOfInt64;
    } else {
      p.skey->incRefCount();
      vec->m_data[j].m_data.pstr = p.skey;
      vec->m_data[j].m_type = KindOfString;
    }
    ++vec->m_size;
    ++j;
  }
  return obj;
}

Object c_Map::t_view() {
  return SystemLib::AllocKeyedIterableViewObject(this);
}

Object c_Map::t_kvzip() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(m_size);
  for (uint i = 0, j = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    if (p.hasIntKey()) {
      pair->elm0.m_data.num = p.ikey;
      pair->elm0.m_type = KindOfInt64;
    } else {
      p.skey->incRefCount();
      pair->elm0.m_data.pstr = p.skey;
      pair->elm0.m_type = KindOfString;
    }
    ++pair->m_size;
    pair->add(&p.data);
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

Object c_Map::t_discard(CVarRef key) {
  return t_remove(key);
}

Array c_Map::t_toarray() {
  return toArrayImpl();
}

Array c_Map::t_copyasarray() {
  return toArrayImpl();
}

Array c_Map::t_tokeysarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    if (p.hasIntKey()) {
      ai.set((int64_t)p.ikey);
    } else {
      ai.set(*(const String*)(&p.skey));
    }
  }
  return ai.create();
}

Object c_Map::t_values() {
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  int64_t sz = m_size;
  if (!sz) {
    return ret;
  }
  TypedValue* data;
  target->m_capacity = target->m_size = m_size;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));

  int64_t j = 0;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    TypedValue* tv = &p.data;
    tvRefcountedIncRef(tv);
    data[j].m_data.num = tv->m_data.num;
    data[j].m_type = tv->m_type;
    ++j;
  }
  return ret;
}

Array c_Map::t_tovaluesarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    ai.set(tvAsCVarRef(&p.data));
  }
  return ai.create();
}

Object c_Map::t_updatefromarray(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected arr to be an array"));
    throw e;
  }
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    TypedValue* tv = cvarToCell(&ad->getValueRef(pos));
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_Map::t_updatefromiterable(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  if (obj->getCollectionType() == Collection::MapType) {
    auto mp = static_cast<c_Map*>(obj);
    for (uint i = 0; i <= mp->m_nLastSlot; ++i) {
      c_Map::Bucket& p = mp->m_data[i];
      if (!p.validValue()) continue;
      if (p.hasIntKey()) {
        update((int64_t)p.ikey, &p.data);
      } else {
        update(p.skey, &p.data);
      }
    }
    return this;
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = tvToCell(v.asTypedValue());
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_Map::t_differencebykey(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  c_Map* target;
  Object ret = target = static_cast<c_Map*>(clone());
  if (obj->getCollectionType() == Collection::MapType) {
    auto mp = static_cast<c_Map*>(obj);
    for (uint i = 0; i <= mp->m_nLastSlot; ++i) {
      c_Map::Bucket& p = mp->m_data[i];
      if (!p.validValue()) continue;
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
  assert(m_nLastSlot != 0);
  mp->m_size = m_size;
  mp->m_load = m_load;
  mp->m_nLastSlot = 0;
  mp->m_data = (Bucket*)smart_malloc(numSlots() * sizeof(Bucket));
  // We need to zero out the first slot in case an exception
  // is thrown during the first iteration, because ~c_Map()
  // will decRef all slots up to (and including) m_nLastSlot.
  mp->m_data[0].data.m_type = (DataType)0;
  for (uint i = 0; i <= m_nLastSlot; mp->m_nLastSlot = i++) {
    Bucket& p = m_data[i];
    Bucket& np = mp->m_data[i];
    if (!p.validValue()) {
      np.data.m_type = p.data.m_type;
      continue;
    }
    TypedValue* tv = &np.data;
    int32_t version = m_version;
    g_vmContext->invokeFunc(tv, ctx, CREATE_VECTOR1(tvAsCVarRef(&p.data)));
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
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFunc(ret.asTypedValue(), ctx,
                            CREATE_VECTOR1(tvAsCVarRef(&p.data)));
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
  mp->reserve(std::min(sz, size_t(m_size)));
  for (uint i = 0; i <= m_nLastSlot && iter; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    Variant v = iter.second();
    c_Pair* pair;
    Object pairObj = pair = NEWOBJ(c_Pair)();
    pair->add(&p.data);
    pair->add(cvarToCell(&v));
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
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Map* target;
  Object ret = target = NEWOBJ(c_Map)();
  if (sz) {
    target->reserve(sz);
  }
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = tvToCell(v.asTypedValue());
    if (UNLIKELY(tv->m_type != KindOfObject ||
                 !tv->m_data.pobj->instanceof(c_Pair::s_cls))) {
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

Object c_Map::ti_fromiterable(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  Object ret;
  if (obj->getCollectionType() == Collection::MapType) {
    ret = obj->clone();
    return ret;
  }
  c_Map* target;
  ret = target = NEWOBJ(c_Map)();
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    if (k.isInteger()) {
      target->update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      target->update(k.getStringData(), tv);
    }
  }
  return ret;
}

void c_Map::throwOOB(int64_t key) {
  throwIntOOB(key);
}

void c_Map::throwOOB(StringData* key) {
  throwStrOOB(key);
}

void c_Map::add(TypedValue* val) {
  if (UNLIKELY(val->m_type != KindOfObject ||
               !val->m_data.pobj->instanceof(c_Pair::s_cls))) {
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

#define STRING_HASH(x)   (int32_t(x) | 0x80000000)

bool inline hitStringKey(const c_Map::Bucket* p, const char* k,
                         int len, int32_t hash) ALWAYS_INLINE;
bool inline hitStringKey(const c_Map::Bucket* p, const char* k,
                  int len, int32_t hash) {
  assert(p->validValue());
  if (p->hasIntKey()) return false;
  const char* data = p->skey->data();
  return data == k || (p->hash() == hash &&
                       p->skey->size() == len &&
                       memcmp(data, k, len) == 0);
}

bool inline hitIntKey(const c_Map::Bucket* p, int64_t ki) ALWAYS_INLINE;
bool inline hitIntKey(const c_Map::Bucket* p, int64_t ki) {
  assert(p->validValue());
  return p->ikey == ki && p->hasIntKey();
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

c_Map::Bucket* c_Map::find(int64_t h) const {
  FIND_BODY(h, hitIntKey(p, h));
}

c_Map::Bucket* c_Map::find(const char* k, int len, strhash_t prehash) const {
  FIND_BODY(prehash, hitStringKey(p, k, len, STRING_HASH(prehash)));
}

c_Map::Bucket* c_Map::findForInsert(int64_t h) const {
  FIND_FOR_INSERT_BODY(h, hitIntKey(p, h));
}

c_Map::Bucket* c_Map::findForInsert(const char* k, int len,
                                    strhash_t prehash) const {
  FIND_FOR_INSERT_BODY(prehash, hitStringKey(p, k, len, STRING_HASH(prehash)));
}

// findForNewInsert() is only safe to use if you know for sure that the
// key is not already present in the Map.
inline ALWAYS_INLINE
c_Map::Bucket* c_Map::findForNewInsert(size_t h0) const {
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

bool c_Map::update(int64_t h, TypedValue* data) {
  assert(data->m_type != KindOfRef);
  Bucket* p = findForInsert(h);
  assert(p);
  if (p->validValue()) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
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
  tvRefcountedIncRef(data);
  p->data.m_data.num = data->m_data.num;
  p->data.m_type = data->m_type;
  p->setIntKey(h);
  return true;
}

bool c_Map::update(StringData *key, TypedValue* data) {
  strhash_t h = key->hash();
  Bucket* p = findForInsert(key->data(), key->size(), h);
  assert(p);
  if (p->validValue()) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
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
  tvRefcountedIncRef(data);
  p->data.m_data.num = data->m_data.num;
  p->data.m_type = data->m_type;
  p->setStrKey(key, h);
  return true;
}

void c_Map::erase(Bucket* p) {
  if (!p) {
    return;
  }
  if (p->validValue()) {
    m_size--;
    tvRefcountedDecRef(&p->data);
    if (p->hasStrKey() && p->skey->decRefCount() == 0) {
      DELETE(StringData)(p->skey);
    }
    p->data.m_type = (DataType)KindOfTombstone;
    if (m_size < computeMinElements() && m_size) {
      adjustCapacity();
    }
  }
}

void c_Map::adjustCapacityImpl(int64_t sz) {
  ++m_version;
  if (sz < 2) {
    if (sz <= 0) return;
    sz = 2;
  }
  if (m_nLastSlot == 0) {
    assert(m_data == (Bucket*)emptyMapSlot);
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
      Bucket* np = findForNewInsert(p->hasIntKey() ? p->ikey : p->hash());
      memcpy(np, p, sizeof(Bucket));
    }
  }
  smart_free(oldBuckets);
}

ssize_t c_Map::iter_begin() const {
  if (!m_size) return 0;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket* p = fetchBucket(i);
    if (p->validValue()) {
      return reinterpret_cast<ssize_t>(p);
    }
  }
  return 0;
}

ssize_t c_Map::iter_next(ssize_t pos) const {
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

ssize_t c_Map::iter_prev(ssize_t pos) const {
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

Variant c_Map::iter_key(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  if (p->hasStrKey()) {
    return p->skey;
  }
  return (int64_t)p->ikey;
}

Variant c_Map::iter_value(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return tvAsCVarRef(&p->data);
}

void c_Map::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with Maps"));
  throw e;
}

void c_Map::Bucket::dump() {
  if (!validValue()) {
    printf("c_Map::Bucket: %s\n", (empty() ? "empty" : "tombstone"));
    return;
  }
  printf("c_Map::Bucket: %" PRIx64 "\n", hashKey());
  if (hasStrKey()) {
    skey->dump();
  }
  tvAsCVarRef(&data).dump();
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
  return result ? isset(tvAsCVarRef(result)) : false;
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
  return result ? empty(tvAsCVarRef(result)) : true;
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

void c_Map::OffsetAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  auto mp = static_cast<c_Map*>(obj);
  mp->add(val);
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

bool c_Map::Equals(ObjectData* obj1, ObjectData* obj2) {
  auto mp1 = static_cast<c_Map*>(obj1);
  auto mp2 = static_cast<c_Map*>(obj2);
  if (mp1->m_size != mp2->m_size) return false;
  for (uint i = 0; i <= mp1->m_nLastSlot; ++i) {
    c_Map::Bucket& p = mp1->m_data[i];
    if (p.validValue()) {
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
    k.unserialize(uns, Uns::ColKeyMode);
    Bucket* p;
    if (k.isInteger()) {
      auto h = k.toInt64();
      p = mp->findForInsert(h);
      if (UNLIKELY(p->validValue())) goto do_unserialize;
      p->setIntKey(h);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      p = mp->findForInsert(key->data(), key->size(), h);
      if (UNLIKELY(p->validValue())) goto do_unserialize;
      p->setStrKey(key, h);
    } else {
      throw Exception("Invalid key");
    }
    ++mp->m_size;
    ++mp->m_load;
    p->data.m_type = KindOfNull;
do_unserialize:
    tvAsVariant(&p->data).unserialize(uns, Uns::ColValueMode);
  }
}

c_MapIterator::c_MapIterator(VM::Class* cb) :
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
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return mp->iter_value(m_pos);
}

Variant c_MapIterator::t_key() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_version != mp->getVersion())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return mp->iter_key(m_pos);
}

bool c_MapIterator::t_valid() {
  return m_pos != 0;
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

IMPLEMENT_SMART_ALLOCATION_CLS(c_StableMap, Bucket);

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

c_StableMap::c_StableMap(VM::Class* cb) :
    ExtObjectDataFlags<ObjectData::StableMapAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_version(0), m_pListHead(nullptr), m_pListTail(nullptr) {
  m_size = 0;
  m_nTableSize = 0;
  m_nTableMask = 0;
  m_arBuckets = (Bucket**)emptyStableMapSlot;
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
    DELETE(Bucket)(q);
  }
}

void c_StableMap::t___construct(CVarRef iterable /* = null_variant */) {
  if (!iterable.isInitialized()) {
    return;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
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

Array c_StableMap::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_StableMap::clone() {
  ObjectData* obj = ObjectData::clone();
  auto target = static_cast<c_StableMap*>(obj);

  if (!m_size) return obj;

  target->m_size = m_size;
  target->m_nTableSize = m_nTableSize;
  target->m_nTableMask = m_nTableMask;
  target->m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));

  Bucket *last = nullptr;
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    Bucket *np = NEW(Bucket)();
    tvDup(&p->data, &np->data);
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

  return obj;
}

Object c_StableMap::t_add(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_StableMap::t_addall(CVarRef iterable) {
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
  return (m_size == 0);
}

int64_t c_StableMap::t_count() {
  return m_size;
}

Object c_StableMap::t_items() {
  return SystemLib::AllocKVZippedIterableObject(this);
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

Object c_StableMap::t_view() {
  return SystemLib::AllocKeyedIterableViewObject(this);
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
    pair->add(&p->data);
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

Object c_StableMap::t_discard(CVarRef key) {
  return t_remove(key);
}

Array c_StableMap::t_toarray() {
  return toArrayImpl();
}

Array c_StableMap::t_copyasarray() {
  return toArrayImpl();
}

Array c_StableMap::t_tokeysarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  Bucket* p = m_pListHead;
  while (p) {
    if (p->hasIntKey()) {
      ai.set((int64_t)p->ikey);
    } else {
      ai.set(*(const String*)(&p->skey));
    }
    p = p->pListNext;
  }
  return ai.create();
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
    tvDup(&p->data, &data[i]);
    p = p->pListNext;
  }
  return ret;
}

Array c_StableMap::t_tovaluesarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  Bucket* p = m_pListHead;
  while (p) {
    ai.set(tvAsCVarRef(&p->data));
    p = p->pListNext;
  }
  return ai.create();
}

Object c_StableMap::t_updatefromarray(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    TypedValue* tv = cvarToCell(&ad->getValueRef(pos));
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_StableMap::t_updatefromiterable(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  if (obj->getCollectionType() == Collection::StableMapType) {
    auto smp = static_cast<c_StableMap*>(obj);
    c_StableMap::Bucket* p = smp->m_pListHead;
    while (p) {
      if (p->hasIntKey()) {
        update((int64_t)p->ikey, &p->data);
      } else {
        update(p->skey, &p->data);
      }
      p = p->pListNext;
    }
    return this;
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_StableMap::t_differencebykey(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  c_StableMap* target;
  Object ret = target = static_cast<c_StableMap*>(clone());
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
    g_vmContext->invokeFunc(ret.asTypedValue(), ctx,
                            CREATE_VECTOR1(tvAsCVarRef(&p->data)));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    Bucket *np = NEW(Bucket)(ret.asTypedValue());
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
    g_vmContext->invokeFunc(ret.asTypedValue(), ctx,
                            CREATE_VECTOR1(tvAsCVarRef(&p->data)));
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
    pair->add(&p->data);
    pair->add(cvarToCell(&v));
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
                 !tv->m_data.pobj->instanceof(c_Pair::s_cls))) {
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

Object c_StableMap::ti_fromiterable(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  Object ret;
  if (obj->getCollectionType() == Collection::StableMapType) {
    ret = obj->clone();
    return ret;
  }
  c_StableMap* target;
  ret = target = NEWOBJ(c_StableMap)();
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    if (k.isInteger()) {
      target->update(k.toInt64(), tv);
    } else {
      assert(k.isString());
      target->update(k.getStringData(), tv);
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
               !val->m_data.pobj->instanceof(c_Pair::s_cls))) {
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

bool inline sm_hit_string_key(const c_StableMap::Bucket* p,
                              const char* k, int len, int32_t hash) ALWAYS_INLINE;
bool inline sm_hit_string_key(const c_StableMap::Bucket* p,
                              const char* k, int len, int32_t hash) {
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
  Bucket* p = find(h);
  if (p) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
  }
  ++m_version;
  if (++m_size > m_nTableSize) {
    adjustCapacity();
  }
  p = NEW(Bucket)(data);
  p->setIntKey(h);
  uint nIndex = (h & m_nTableMask);
  p->pNext = m_arBuckets[nIndex];
  m_arBuckets[nIndex] = p;
  CONNECT_TO_GLOBAL_DLLIST(this, p);
  return true;
}

bool c_StableMap::update(StringData *key, TypedValue* data) {
  strhash_t h = key->hash();
  Bucket* p = find(key->data(), key->size(), h);
  if (p) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
  }
  ++m_version;
  if (++m_size > m_nTableSize) {
    adjustCapacity();
  }
  p = NEW(Bucket)(data);
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
    DELETE(Bucket)(p);
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

ssize_t c_StableMap::iter_begin() const {
  Bucket* p = m_pListHead;
  return reinterpret_cast<ssize_t>(p);
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

Variant c_StableMap::iter_value(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return tvAsCVarRef(&p->data);
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
      return;                                                           \
    }                                                                   \
    Bucket** buffer = (Bucket**)smart_malloc(m_size * sizeof(Bucket*)); \
    preSort<acc_type>(buffer, acc_type(), false);                       \
    ElmUCompare<acc_type> comp;                                         \
    CallCtx ctx;                                                        \
    VM::Transl::CallerFrame cf;                                         \
    vm_decode_function(cmp_function, cf(), false, ctx);                 \
    comp.ctx = &ctx;                                                    \
    try {                                                               \
      HPHP::Sort::sort(buffer, buffer + m_size, comp);                  \
    } catch (...) {                                                     \
      postSort(buffer);                                                 \
      smart_free(buffer);                                               \
      throw;                                                            \
    }                                                                   \
    postSort(buffer);                                                   \
    smart_free(buffer);                                                 \
  } while (0)

void c_StableMap::uasort(CVarRef cmp_function) {
  USER_SORT_BODY(StableMapValAccessor);
}

void c_StableMap::uksort(CVarRef cmp_function) {
  USER_SORT_BODY(StableMapKeyAccessor);
}

#undef USER_SORT_BODY

void c_StableMap::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with StableMaps"));
  throw e;
}

c_StableMap::Bucket::~Bucket() {
  if (hasStrKey() && skey->decRefCount() == 0) {
    DELETE(StringData)(skey);
  }
  tvRefcountedDecRef(&data);
}

void c_StableMap::Bucket::dump() {
  printf("c_StableMap::Bucket: %" PRIx64 ", %p, %p, %p\n",
         hashKey(), pListNext, pListLast, pNext);
  if (hasStrKey()) {
    skey->dump();
  }
  tvAsCVarRef(&data).dump();
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
  return result ? isset(tvAsCVarRef(result)) : false;
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
  return result ? empty(tvAsCVarRef(result)) : true;
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

void c_StableMap::OffsetAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  auto smp = static_cast<c_StableMap*>(obj);
  smp->add(val);
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

bool c_StableMap::Equals(ObjectData* obj1, ObjectData* obj2) {
  auto smp1 = static_cast<c_StableMap*>(obj1);
  auto smp2 = static_cast<c_StableMap*>(obj2);
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
    k.unserialize(uns, Uns::ColKeyMode);
    Bucket* p;
    uint nIndex;
    if (k.isInteger()) {
      auto h = k.toInt64();
      p = smp->find(h);
      if (UNLIKELY(p != nullptr)) goto do_unserialize;
      p = NEW(Bucket)();
      p->setIntKey(h);
      nIndex = (h & smp->m_nTableMask);
    } else if (k.isString()) {
      auto key = k.getStringData();
      auto h = key->hash();
      p = smp->find(key->data(), key->size(), h);
      if (UNLIKELY(p != nullptr)) goto do_unserialize;
      p = NEW(Bucket)();
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
    tvAsVariant(&p->data).unserialize(uns, Uns::ColValueMode);
  }
}

#undef CONNECT_TO_GLOBAL_DLLIST

c_StableMapIterator::c_StableMapIterator(VM::Class* cb) :
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
  return smp->iter_value(m_pos);
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

static const char emptySetSlot[sizeof(c_Set::Bucket)] = { 0 };

c_Set::c_Set(VM::Class* cb) :
    ExtObjectDataFlags<ObjectData::SetAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_size(0), m_load(0), m_nLastSlot(0), m_version(0) {
  m_data = (Bucket*)emptySetSlot;
}

c_Set::~c_Set() {
  deleteBuckets();
  freeData();
}

void c_Set::freeData() {
  if (m_data != (Bucket*)emptySetSlot) {
    smart_free(m_data);
  }
  m_data = (Bucket*)emptySetSlot;
}

void c_Set::deleteBuckets() {
  if (!m_size) return;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      tvRefcountedDecRef(&p.data);
    }
  }
}

void c_Set::t___construct(CVarRef iterable /* = null_variant */) {
  if (!iterable.isInitialized()) {
    return;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    Variant v = iter.second();
    if (v.isInteger()) {
      update(v.toInt64());
    } else if (v.isString()) {
      update(v.getStringData());
    } else {
      throwBadValueType();
    }
  }
}

Array c_Set::toArrayImpl() const {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      ai.set(tvAsCVarRef(&p.data));
    }
  }
  return ai.create();
}

Array c_Set::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_Set::clone() {
  ObjectData* obj = ObjectData::clone();
  auto target = static_cast<c_Set*>(obj);

  if (!m_size) return obj;

  assert(m_nLastSlot != 0);
  target->m_size = m_size;
  target->m_load = m_load;
  target->m_nLastSlot = m_nLastSlot;
  target->m_data = (Bucket*)smart_malloc(numSlots() * sizeof(Bucket));
  memcpy(target->m_data, m_data, numSlots() * sizeof(Bucket));

  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      tvRefcountedIncRef(&p.data);
    }
  }

  return obj;
}

Object c_Set::t_add(CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  add(tv);
  return this;
}

Object c_Set::t_addall(CVarRef iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = cvarToCell(&v);
    add(tv);
  }
  return this;
}

Object c_Set::t_clear() {
  deleteBuckets();
  freeData();
  m_size = 0;
  m_load = 0;
  m_nLastSlot = 0;
  m_data = (Bucket*)emptySetSlot;
  return this;
}

bool c_Set::t_isempty() {
  return (m_size == 0);
}

int64_t c_Set::t_count() {
  return m_size;
}

Object c_Set::t_items() {
  return SystemLib::AllocIterableViewObject(this);
}

Object c_Set::t_view() {
  return SystemLib::AllocIterableViewObject(this);
}

bool c_Set::t_contains(CVarRef key) {
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

Object c_Set::t_remove(CVarRef key) {
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

Array c_Set::t_toarray() {
  return toArrayImpl();
}

Object c_Set::t_getiterator() {
  c_SetIterator* it = NEWOBJ(c_SetIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_version = getVersion();
  return it;
}

Object c_Set::t_map(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Set* st;
  Object obj = st = NEWOBJ(c_Set)();
  if (!m_size) return obj;
  assert(m_nLastSlot != 0);
  st->m_size = m_size;
  st->m_load = m_load;
  st->m_nLastSlot = 0;
  st->m_data = (Bucket*)smart_malloc(numSlots() * sizeof(Bucket));
  // We need to zero out the first slot in case an exception
  // is thrown during the first iteration, because ~c_Set()
  // will decRef all slots up to (and including) m_nLastSlot.
  st->m_data[0].data.m_type = (DataType)0;
  for (uint i = 0; i <= m_nLastSlot; st->m_nLastSlot = i++) {
    Bucket& p = m_data[i];
    Bucket& np = st->m_data[i];
    if (!p.validValue()) {
      np.data.m_type = p.data.m_type;
      continue;
    }
    TypedValue* tv = &np.data;
    int32_t version = m_version;
    g_vmContext->invokeFunc(tv, ctx, CREATE_VECTOR1(tvAsCVarRef(&p.data)));
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    np.data.hash() = p.data.hash();
  }
  return obj;
}

Object c_Set::t_filter(CVarRef callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter must be a valid callback"));
    throw e;
  }
  c_Set* st;
  Object obj = st = NEWOBJ(c_Set)();
  if (!m_size) return obj;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (!p.validValue()) continue;
    Variant ret;
    int32_t version = m_version;
    g_vmContext->invokeFunc(ret.asTypedValue(), ctx,
                            CREATE_VECTOR1(tvAsCVarRef(&p.data)));
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!ret.toBoolean()) continue;
    if (p.hasInt()) {
      st->update(p.data.m_data.num);
    } else {
      assert(p.hasStr());
      st->update(p.data.m_data.pstr);
    }
  }
  return obj;
}

Object c_Set::t_zip(CVarRef iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  if (m_size && iter) {
    // At present, Sets only support int values and string values,
    // so if this Set is non empty and the iterable is non empty
    // the zip operation will always fail
    throwBadValueType();
  }
  Object obj = NEWOBJ(c_Set)();
  return obj;
}

Object c_Set::ti_fromitems(CVarRef iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Set* target;
  Object ret = target = NEWOBJ(c_Set)();
  for (; iter; ++iter) {
    Variant v = iter.second();
    if (v.isInteger()) {
      target->update(v.toInt64());
    } else if (v.isString()) {
      target->update(v.getStringData());
    } else {
      throwBadValueType();
    }
  }
  return ret;
}

Object c_Set::ti_fromarray(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  c_Set* st;
  Object ret = st = NEWOBJ(c_Set)();
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    CVarRef v = ad->getValueRef(pos);
    if (v.isInteger()) {
      st->update(v.toInt64());
    } else if (v.isString()) {
      st->update(v.getStringData());
    } else {
      throwBadValueType();
    }
  }
  return ret;
}

Object c_Set::t_discard(CVarRef key) {
  return t_remove(key);
}

Object c_Set::t_difference(CVarRef iterable) {
  if (!iterable.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter iterable must be an instance of Iterable"));
    throw e;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    t_remove(iter.second());
  }
  return this;
}

Object c_Set::t_updatefromarrayvalues(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(arr, sz);
  for (; iter; ++iter) {
    t_add(iter.second());
  }
  return this;
}

Object c_Set::t_updatefromiterablevalues(CVarRef iterable) {
  if (!iterable.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter iterable must be an instance of Iterable"));
    throw e;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    t_add(iter.second());
  }
  return this;
}

Object c_Set::ti_fromarrays(int _argc,
                            CArrRef _argv /* = null_array */) {
  c_Set* st;
  Object ret = st = NEWOBJ(c_Set)();
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
      st->t_add(ad->getValueRef(pos));
    }
  }
  return ret;
}

Object c_Set::ti_fromiterablevalues(CVarRef iterable) {
  if (!iterable.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter iterable must be an instance of Iterable"));
    throw e;
  }
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  c_Set* st;
  Object ret = st = NEWOBJ(c_Set)();
  for (; iter; ++iter) {
    st->t_add(iter.second());
  }
  return ret;
}

void c_Set::throwOOB(int64_t val) {
  throwIntOOB(val);
}

void c_Set::throwOOB(StringData* val) {
  throwStrOOB(val);
}

void c_Set::throwNoIndexAccess() {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot use object of type Set as array"));
  throw e;
}

void c_Set::add(TypedValue* val) {
  if (val->m_type == KindOfInt64) {
    update(val->m_data.num);
  } else if (IS_STRING_TYPE(val->m_type)) {
    update(val->m_data.pstr);
  } else {
    throwBadValueType();
  }
}

#define STRING_HASH(x)   (int32_t(x) | 0x80000000)

bool inline hitString(const c_Set::Bucket* p, const char* k,
                         int len, int32_t hash) ALWAYS_INLINE;
bool inline hitString(const c_Set::Bucket* p, const char* k,
                         int len, int32_t hash) {
  assert(p->validValue());
  if (p->hasInt()) return false;
  const char* data = p->data.m_data.pstr->data();
  return data == k || (p->hash() == hash &&
                       p->data.m_data.pstr->size() == len &&
                       memcmp(data, k, len) == 0);
}

bool inline hitInt(const c_Set::Bucket* p, int64_t ki) ALWAYS_INLINE;
bool inline hitInt(const c_Set::Bucket* p, int64_t ki) {
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

c_Set::Bucket* c_Set::find(int64_t h) const {
  FIND_BODY(h, hitInt(p, h));
}

c_Set::Bucket* c_Set::find(const char* k, int len, strhash_t prehash) const {
  FIND_BODY(prehash, hitString(p, k, len, STRING_HASH(prehash)));
}

c_Set::Bucket* c_Set::findForInsert(int64_t h) const {
  FIND_FOR_INSERT_BODY(h, hitInt(p, h));
}

c_Set::Bucket* c_Set::findForInsert(const char* k, int len,
                                    strhash_t prehash) const {
  FIND_FOR_INSERT_BODY(prehash, hitString(p, k, len, STRING_HASH(prehash)));
}

// findForNewInsert() is only safe to use if you know for sure that the
// value is not already present in the Set.
inline ALWAYS_INLINE
c_Set::Bucket* c_Set::findForNewInsert(size_t h0) const {
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

void c_Set::update(int64_t h) {
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

void c_Set::update(StringData *key) {
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

void c_Set::erase(Bucket* p) {
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

void c_Set::adjustCapacityImpl(int64_t sz) {
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

ssize_t c_Set::iter_begin() const {
  if (!m_size) return 0;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket* p = fetchBucket(i);
    if (p->validValue()) {
      return reinterpret_cast<ssize_t>(p);
    }
  }
  return 0;
}

ssize_t c_Set::iter_next(ssize_t pos) const {
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

ssize_t c_Set::iter_prev(ssize_t pos) const {
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

Variant c_Set::iter_value(ssize_t pos) const {
  assert(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return tvAsCVarRef(&p->data);
}

void c_Set::throwBadValueType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer values and string values may be used with Sets"));
  throw e;
}

void c_Set::Bucket::dump() {
  if (!validValue()) {
    printf("c_Set::Bucket: %s\n", (empty() ? "empty" : "tombstone"));
    return;
  }
  printf("c_Set::Bucket: %d\n", hash());
  tvAsCVarRef(&data).dump();
}

TypedValue* c_Set::OffsetGet(ObjectData* obj, TypedValue* key) {
  c_Set::throwNoIndexAccess();
}

void c_Set::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  c_Set::throwNoIndexAccess();
}

bool c_Set::OffsetIsset(ObjectData* obj, TypedValue* key) {
  c_Set::throwNoIndexAccess();
}

bool c_Set::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  c_Set::throwNoIndexAccess();
}

bool c_Set::OffsetContains(ObjectData* obj, TypedValue* key) {
  c_Set::throwNoIndexAccess();
}

void c_Set::OffsetAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  auto st = static_cast<c_Set*>(obj);
  st->add(val);
}

void c_Set::OffsetUnset(ObjectData* obj, TypedValue* key) {
  c_Set::throwNoIndexAccess();
}

bool c_Set::Equals(ObjectData* obj1, ObjectData* obj2) {
  auto st1 = static_cast<c_Set*>(obj1);
  auto st2 = static_cast<c_Set*>(obj2);
  if (st1->m_size != st2->m_size) return false;
  for (uint i = 0; i <= st1->m_nLastSlot; ++i) {
    c_Set::Bucket& p = st1->m_data[i];
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

void c_Set::Unserialize(ObjectData* obj,
                        VariableUnserializer* uns,
                        int64_t sz,
                        char type) {
  if (type != 'V') {
    throw Exception("Set does not support the '%c' serialization "
                    "format", type);
  }
  auto st = static_cast<c_Set*>(obj);
  st->reserve(sz);
  for (int64_t i = 0; i < sz; ++i) {
    Variant k;
    // When unserializing an element of a Set, we use ColKeyMode for now.
    // This will make the unserializer to reserve an id for the element
    // but won't allow referencing the element via 'r' or 'R'.
    k.unserialize(uns, Uns::ColKeyMode);
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
      throw Exception("Set values must be integers or strings");
    }
    ++st->m_size;
    ++st->m_load;
  }
}

c_SetIterator::c_SetIterator(VM::Class* cb) :
    ExtObjectData(cb) {
}

c_SetIterator::~c_SetIterator() {
}

void c_SetIterator::t___construct() {
}

Variant c_SetIterator::t_current() {
  c_Set* st = m_obj.get();
  if (UNLIKELY(m_version != st->getVersion())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return st->iter_value(m_pos);
}

Variant c_SetIterator::t_key() {
  return uninit_null();
}

bool c_SetIterator::t_valid() {
  return m_pos != 0;
}

void c_SetIterator::t_next() {
  c_Set* st = m_obj.get();
  if (UNLIKELY(m_version != st->getVersion())) {
    throw_collection_modified();
  }
  m_pos = st->iter_next(m_pos);
}

void c_SetIterator::t_rewind() {
  c_Set* st = m_obj.get();
  if (UNLIKELY(m_version != st->getVersion())) {
    throw_collection_modified();
  }
  m_pos = st->iter_begin();
}

///////////////////////////////////////////////////////////////////////////////

c_Pair::c_Pair(VM::Class* cb) :
    ExtObjectDataFlags<ObjectData::PairAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_size(0) {
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
  ArrayInit ai(2, ArrayInit::vectorInit);
  ai.set(tvAsCVarRef(&elm0));
  ai.set(tvAsCVarRef(&elm1));
  return ai.create();
}

Array c_Pair::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_Pair::clone() {
  auto pair = NEWOBJ(c_Pair)();
  pair->incRefCount();
  pair->m_size = 2;
  tvDup(&elm0, &pair->elm0);
  tvDup(&elm1, &pair->elm1);
  return pair;
}

bool c_Pair::t_isempty() {
  return false;
}

int64_t c_Pair::t_count() {
  return 2;
}

Object c_Pair::t_items() {
  return SystemLib::AllocIterableViewObject(this);
}

Object c_Pair::t_keys() {
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

Object c_Pair::t_view() {
  return SystemLib::AllocKeyedIterableViewObject(this);
}

Object c_Pair::t_kvzip() {
  c_Vector* vec;
  Object obj = vec = NEWOBJ(c_Vector)();
  vec->reserve(2);
  for (uint i = 0; i < 2; ++i) {
    c_Pair* pair = NEWOBJ(c_Pair)();
    pair->incRefCount();
    pair->elm0.m_type = KindOfInt64;
    pair->elm0.m_data.num = i;
    ++pair->m_size;
    pair->add(&getElms()[i]);
    vec->m_data[i].m_data.pobj = pair;
    vec->m_data[i].m_type = KindOfObject;
    ++vec->m_size;
  }
  return obj;
}

Variant c_Pair::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return init_null_variant;
}

Variant c_Pair::t_get(CVarRef key) {
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
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

Array c_Pair::t_toarray() {
  return toArrayImpl();
}

Object c_Pair::t_getiterator() {
  c_PairIterator* it = NEWOBJ(c_PairIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  return it;
}

Object c_Pair::t_map(CVarRef callback) {
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
    g_vmContext->invokeFunc(&vec->m_data[i], ctx,
                            CREATE_VECTOR1(tvAsCVarRef(&getElms()[i])));
    ++vec->m_size;
  }
  return obj;
}

Object c_Pair::t_filter(CVarRef callback) {
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
    g_vmContext->invokeFunc(ret.asTypedValue(), ctx,
                            CREATE_VECTOR1(tvAsCVarRef(&getElms()[i])));
    if (ret.toBoolean()) {
      vec->add(&getElms()[i]);
    }
  }
  return obj;
}

Object c_Pair::t_zip(CVarRef iterable) {
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
    pair->add(&getElms()[i]);
    pair->add(cvarToCell(&v));
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

TypedValue* c_Pair::OffsetGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  if (key->m_type == KindOfInt64) {
    return pair->at(key->m_data.num);
  }
  throwBadKeyType();
  return nullptr;
}

void c_Pair::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot assign to an element of a Pair"));
  throw e;
}

bool c_Pair::OffsetIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = pair->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? isset(tvAsCVarRef(result)) : false;
}

bool c_Pair::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = pair->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? empty(tvAsCVarRef(result)) : true;
}

bool c_Pair::OffsetContains(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  if (key->m_type == KindOfInt64) {
    return pair->contains(key->m_data.num);
  } else {
    throwBadKeyType();
    return false;
  }
}

void c_Pair::OffsetAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  pair->add(val);
}

void c_Pair::OffsetUnset(ObjectData* obj, TypedValue* key) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot unset element of a Pair"));
  throw e;
}

bool c_Pair::Equals(ObjectData* obj1, ObjectData* obj2) {
  auto pair1 = static_cast<c_Pair*>(obj1);
  auto pair2 = static_cast<c_Pair*>(obj2);
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
  tvAsVariant(&pair->elm0).unserialize(uns, Uns::ColValueMode);
  tvAsVariant(&pair->elm1).unserialize(uns, Uns::ColValueMode);
}

c_PairIterator::c_PairIterator(VM::Class* cb) :
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
    throw_collection_property_exception(); \
  } \
  Variant c_##cls::t___unset(Variant name) { \
    throw_collection_property_exception(); \
  }

COLLECTION_MAGIC_METHODS(Vector)
COLLECTION_MAGIC_METHODS(Map)
COLLECTION_MAGIC_METHODS(StableMap)
COLLECTION_MAGIC_METHODS(Set)
COLLECTION_MAGIC_METHODS(Pair)

#undef COLLECTION_MAGIC_METHODS

void collectionSerialize(ObjectData* obj, VariableSerializer* serializer) {
  assert(obj->isCollection());
  int64_t sz = collectionSize(obj);
  if (obj->getCollectionType() == Collection::VectorType ||
      obj->getCollectionType() == Collection::SetType ||
      obj->getCollectionType() == Collection::PairType) {
    serializer->setObjectInfo(obj->o_getClassName(), obj->o_getId(), 'V');
    serializer->writeArrayHeader(sz, true);
    if (serializer->getType() == VariableSerializer::Serialize ||
        serializer->getType() == VariableSerializer::APCSerialize ||
        serializer->getType() == VariableSerializer::DebuggerSerialize) {
      // For the 'V' serialization format, we don't print out keys
      // for Serialize, APCSerialize, DebuggerSerialize
      for (ArrayIter iter(obj); iter; ++iter) {
        serializer->writeArrayValue(iter.second());
      }
    } else {
      for (ArrayIter iter(obj); iter; ++iter) {
        if (obj->getCollectionType() != Collection::SetType) {
          serializer->writeCollectionKey(iter.first());
        } else {
          serializer->writeCollectionKeylessPrefix();
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
      if (tv->m_data.parr->decRefCount() == 0) {
        tv->m_data.parr->release();
      }
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
        case Collection::PairType:
          obj = collectionDeepCopyPair(static_cast<c_Pair*>(obj));
          break;
        default:
          assert(false);
          obj = nullptr;
          break;
      }
      if (tv->m_data.pobj->decRefCount() == 0) {
        tv->m_data.pobj->release();
      }
      tv->m_data.pobj = obj;
      break;
    }
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

ObjectData* collectionDeepCopyVector(c_Vector* vec) {
  Object o = vec = static_cast<c_Vector*>(vec->clone());
  size_t sz = vec->m_size;
  for (size_t i = 0; i < sz; ++i) {
    collectionDeepCopyTV(&vec->m_data[i]);
  }
  return o.detach();
}

ObjectData* collectionDeepCopyMap(c_Map* mp) {
  Object o = mp = static_cast<c_Map*>(mp->clone());
  uint lastSlot = mp->m_nLastSlot;
  for (uint i = 0; i <= lastSlot; ++i) {
    c_Map::Bucket* p = mp->fetchBucket(i);
    if (p->validValue()) {
      collectionDeepCopyTV(&p->data);
    }
  }
  return o.detach();
}

ObjectData* collectionDeepCopyStableMap(c_StableMap* smp) {
  Object o = smp = static_cast<c_StableMap*>(smp->clone());
  for (c_StableMap::Bucket* p = smp->m_pListHead; p; p = p->pListNext) {
    collectionDeepCopyTV(&p->data);
  }
  return o.detach();
}

ObjectData* collectionDeepCopySet(c_Set* st) {
  Object o = st = static_cast<c_Set*>(st->clone());
  return o.detach();
}

ObjectData* collectionDeepCopyPair(c_Pair* pair) {
  Object o = pair = static_cast<c_Pair*>(pair->clone());
  collectionDeepCopyTV(&pair->elm0);
  collectionDeepCopyTV(&pair->elm1);
  return o.detach();
}

///////////////////////////////////////////////////////////////////////////////
}
