#include "hphp/runtime/ext/collections/ext_collections-vector.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_HH_Vector("HH\\Vector"),
  s_HH_ImmVector("HH\\ImmVector"),
  s_VectorIterator("VectorIterator");

/////////////////////////////////////////////////////////////////////////////
// VectorIterator

static Variant HHVM_METHOD(VectorIterator, current) {
  return Native::data<VectorIterator>(this_)->current();
}

static Variant HHVM_METHOD(VectorIterator, key) {
  return Native::data<VectorIterator>(this_)->key();
}

static bool HHVM_METHOD(VectorIterator, valid) {
  return Native::data<VectorIterator>(this_)->valid();
}

static void HHVM_METHOD(VectorIterator, next) {
  Native::data<VectorIterator>(this_)->next();
}

static void HHVM_METHOD(VectorIterator, rewind) {
  Native::data<VectorIterator>(this_)->rewind();
}

}
/////////////////////////////////////////////////////////////////////////////
// Helpers

NEVER_INLINE
void BaseVector::throwBadKeyType() {
  SystemLib::throwInvalidArgumentExceptionObject(
             "Only integer keys may be used with Vectors");
}

ALWAYS_INLINE static
bool invokeAndCastToBool(const CallCtx& ctx, int argc,
                         const TypedValue* argv) {
  auto ret = Variant::attach(
    g_context->invokeFuncFew(ctx, argc, argv)
  );
  return ret.toBoolean();
}

/////////////////////////////////////////////////////////////////////////////
// BaseVector

template<class TVector, bool useKey>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_map(const Variant& callback) {
  VMRegGuard _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }

  if (m_size == 0) {
    return Object{req::make<TVector>()};
  }

  auto nv = req::make<TVector>(m_size);
  int32_t version = m_version;
  constexpr int64_t argc = useKey ? 2 : 1;
  TypedValue argv[argc];
  if (useKey) {
    argv[0] = make_tv<KindOfInt64>(0);
  }
  auto from = data();
  auto end = from + m_size;
  auto to = nv->data();
  do {
    argv[argc-1] = *from;
    *to = g_context->invokeFuncFew(ctx, argc, argv);
    nv->incSize();
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (useKey) {
      argv[0].m_data.num++;
    }
    ++to;
  } while (++from < end);
  return Object{std::move(nv)};
}

template<class TVector, bool useKey>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_filter(const Variant& callback) {
  VMRegGuard _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  if (m_size == 0) {
    return Object{req::make<TVector>()};
  }
  auto nv = req::make<TVector>(0);
  int32_t version = m_version;
  constexpr int64_t argc = useKey ? 2 : 1;
  TypedValue argv[argc];
  if (useKey) {
    argv[0] = make_tv<KindOfInt64>(0);
  }
  auto elm = data();
  auto end = elm + m_size;
  do {
    argv[argc-1] = *elm;
    bool b = invokeAndCastToBool(ctx, argc, argv);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) {
      nv->addRaw(*elm);
    }
    if (useKey) {
      argv[0].m_data.num++;
    }
  } while (++elm < end);
  return Object{std::move(nv)};
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_take(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = std::max(n.toInt64(), int64_t{0});
  uint32_t sz = std::min(len, int64_t(m_size));
  if (sz == 0) {
    return Object{req::make<TVector>()};
  }
  auto vec = req::make<TVector>(sz);
  vec->setSize(sz);
  auto from = data();
  auto end = from + sz;
  auto to = vec->data();
  do {
    cellDup(*from++, *to++);
  } while (from < end);
  return Object{std::move(vec)};
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  if (m_size == 0) {
    return Object{req::make<TVector>()};
  }
  auto vec = req::make<TVector>(0);
  int32_t version UNUSED;
  if (std::is_same<c_Vector, TVector>::value) {
    version = m_version;
  }
  auto elm = data();
  auto end = elm + m_size;
  do {
    bool b = invokeAndCastToBool(ctx, 1, elm);
    if (std::is_same<c_Vector, TVector>::value) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
    vec->addRaw(*elm);
  } while (++elm < end);
  return Object{std::move(vec)};
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = std::max(n.toInt64(), int64_t{0});
  uint32_t skipAmt = std::min(len, int64_t(m_size));
  uint32_t sz = m_size - skipAmt;
  if (sz == 0) {
    return Object{req::make<TVector>()};
  }
  auto vec = req::make<TVector>(sz);
  vec->setSize(sz);
  auto from = data() + skipAmt;
  auto end = data() + m_size;
  auto to = vec->data();
  do {
    cellDup(*from++, *to++);
  } while (from < end);
  return Object{std::move(vec)};
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  if (m_size == 0) {
    return Object{req::make<TVector>()};
  }
  int32_t version UNUSED;
  if (std::is_same<c_Vector, TVector>::value) {
    version = m_version;
  }
  auto from = data();
  auto end = from + m_size;
  do {
    bool b = invokeAndCastToBool(ctx, 1, from);
    if (std::is_same<c_Vector, TVector>::value) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
  } while (++from < end);
  auto sz = end - from;
  if (sz == 0) {
    return Object{req::make<TVector>()};
  }
  auto vec = req::make<TVector>(sz);
  vec->setSize(sz);
  auto to = vec->data();
  do {
    cellDup(*from++, *to++);
  } while (from < end);
  return Object{std::move(vec)};
}

template<class TVector>
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
  uint32_t skipAmt = std::min(istart, int64_t(m_size));
  uint32_t sz = std::min(ilen, int64_t(m_size - skipAmt));
  if (sz == 0) {
    return Object{req::make<TVector>()};
  }
  auto vec = req::make<TVector>(sz);
  vec->setSize(sz);
  auto from = data() + skipAmt;
  auto end = from + sz;
  auto to = vec->data();
  do {
    cellDup(*from++, *to++);
  } while (from < end);
  return Object{std::move(vec)};
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_concat(const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  uint32_t sz = m_size + itSize;
  auto vec = req::make<TVector>(sz);
  if (m_size > 0) {
    vec->setSize(m_size);
    auto from = data();
    auto end = from + m_size;
    auto to = vec->data();
    do {
      cellDup(*from++, *to++);
    } while (from < end);
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return Object{std::move(vec)};
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_zip(const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  if (m_size == 0 || !iter) {
    return Object{req::make<TVector>()};
  }
  uint32_t sz = std::min(itSize, size_t(m_size));
  auto vec = req::make<TVector>(sz);
  auto elm = data();
  auto end = elm + m_size;
  do {
    Variant v = iter.second();
    auto pair = req::make<c_Pair>(*elm, *v.asCell());
    vec->addRaw(make_tv<KindOfObject>(pair.get()));
    ++iter;
  } while (++elm < end && iter);
  return Object{std::move(vec)};
}

// Used by __construct for Vector and ImmVector
// Used by addAll for Vector only
void BaseVector::addAllImpl(const Variant& t) {
  if (t.isNull()) return;

  auto ok = IterateV(
    *t.asTypedValue(),
    [&, this](ArrayData* adata) {
      if (adata->empty()) return true;
      if (!m_size && adata->isVecArray()) {
        ++m_version;
        dropImmCopy();
        auto oldAd = arrayData();
        m_arr = adata;
        adata->incRefCount();
        m_size = arrayData()->m_size;
        decRefArr(oldAd);
        return true;
      }
      reserve(m_size + adata->size());
      return false;
    },
    [this](TypedValue v) {
      addRaw(tvToCell(v));
    },
    [&, this](ObjectData* coll) {
      if (coll->collectionType() == CollectionType::Pair) {
        reserve(m_size + 2);
      }
    },
    [this](const TypedValue* item) {
      add(*tvToCell(item));
    });

  if (UNLIKELY(!ok)) {
    throw_invalid_collection_parameter();
  }
}

int64_t BaseVector::linearSearch(const Variant& search_value) {
  for (auto elm = data(), end = elm + m_size; elm < end; ++elm) {
    if (same(search_value, tvAsCVarRef(elm))) {
      return elm - data();
    }
  }
  return -1;
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::php_keys() {
  if (m_size == 0) {
    return Object{req::make<TVector>()};
  }
  auto vec = req::make<TVector>(m_size);
  vec->setSize(m_size);
  auto elm = vec->data();
  int64_t i = 0;
  do {
    *elm++ = make_tv<KindOfInt64>(i++);
  } while (i < m_size);
  return Object{std::move(vec)};
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
  return PackedArray::VecEqual(bv1->arrayData(), bv2->arrayData());
}

void BaseVector::addFront(TypedValue tv) {
  assert(tv.m_type != KindOfRef);
  ++m_version;
  dropImmCopy();
  auto oldAd = arrayData();
  m_arr = PackedArray::PrependVec(oldAd, tv, oldAd->cowCheck());
  if (m_arr != oldAd) {
    decRefArr(oldAd);
  }
  m_size = arrayData()->m_size;
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
  auto oldBuf = data();
  auto oldAd = arrayData();
  m_arr = PackedArray::MakeReserveVec(newCap);
  arrayData()->m_size = m_size;
  if (LIKELY(!oldAd->cowCheck())) {
    assert(oldAd->isVecArray());
    if (m_size > 0) {
      std::memcpy(data(), oldBuf, m_size * sizeof(TypedValue));
      // Mark oldAd as having 0 elements so that the array release logic doesn't
      // decRef the elements (since we teleported the elements to a new array)
      oldAd->m_size = 0;
    }
    decRefArr(oldAd);
  } else {
    if (m_size > 0) {
      auto from = oldBuf;
      auto end = oldBuf + m_size;
      auto to = data();
      do {
        cellDup(*from++, *to++);
      } while (from < end);
    }
    assert(!oldAd->decWillRelease());
    oldAd->decRefCount();
  }
}

void BaseVector::reserve(uint32_t sz) {
  ++m_version;
  dropImmCopy();
  if (sz > PackedArray::capacity(arrayData())) {
    reserveImpl(sz);
  } else if (!canMutateBuffer()) {
    mutateImpl();
  }
  assert(canMutateBuffer());
}

/**
 * Delegate the responsibility for freeing the buffer to the immutable copy,
 * if it exists.
 */
BaseVector::~BaseVector() {
  // Avoid indirect call, as we know it is a vec array.
  auto const vec = arrayData();
  if (vec->decReleaseCheck()) PackedArray::ReleaseVec(vec);
}

void BaseVector::mutateImpl() {
  auto oldAd = arrayData();
  m_arr = PackedArray::CopyVec(oldAd);
  assert(!oldAd->decWillRelease());
  oldAd->decRefCount();
}

template<class TVector>
ALWAYS_INLINE typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, TVector*>::type
BaseVector::Clone(ObjectData* obj) {
  auto thiz = static_cast<TVector*>(obj);
  thiz->arrayData()->incRefCount();
  return req::make<TVector>(thiz->arrayData()).detach();
}

template<class TVector> typename
  std::enable_if<std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::fromKeysOf(const TypedValue& container) {
  uint32_t sz = getContainerSize(container);
  if (sz == 0) {
    return Object{req::make<TVector>()};
  }
  auto vec = req::make<TVector>(sz);
  vec->setSize(sz);
  ArrayIter iter(container);
  assert(iter);
  auto elm = vec->data();
  do {
    cellDup(*iter.first().asCell(), *elm++);
    ++iter;
  } while (iter);
  return Object{std::move(vec)};
}

// This function will create a immutable copy of this Vector (if it doesn't
// already exist) and then return it
Object BaseVector::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto vec = req::make<c_ImmVector>(arrayData());
    arrayData()->incRefCount();
    vec->m_version = m_version;
    m_immCopy = std::move(vec);
  }
  assert(!m_immCopy.isNull());
  assert(arrayData() ==
         static_cast<c_ImmVector*>(m_immCopy.get())->arrayData());
  assert(!canMutateBuffer());
  return m_immCopy;
}

Object BaseVector::getIterator() {
  auto iter = collections::VectorIterator::newInstance();
  Native::data<collections::VectorIterator>(iter)->setVector(this);
  return iter;
}

//////////////////////////////////////////////////////////////////////////////
// c_Vector

Class* c_Vector::s_cls;

void c_Vector::clear() {
  ++m_version;
  dropImmCopy();
  decRefArr(arrayData());
  m_arr = staticEmptyVecArray();
  m_size = 0;
}

void c_Vector::removeKey(int64_t k) {
  if (!contains(k)) return;
  mutateAndBump();
  auto const old = data()[k];
  if (k+1 < m_size) {
    memmove(&data()[k], &data()[k+1],
            (m_size-(k+1)) * sizeof(TypedValue));
  }
  decSize();
  tvDecRefGen(old);
}

void c_Vector::addAllKeysOf(const Variant& container) {
  if (container.isNull()) return;
  const auto& containerCell = container_as_cell(container);

  auto sz = getContainerSize(containerCell);
  ArrayIter iter(containerCell);
  if (!sz || !iter) return;
  reserve(m_size + sz);
  do {
    addRaw(iter.first());
    ++iter;
  } while (iter);
}

Variant c_Vector::pop() {
  if (m_size) {
    mutateAndBump();
    decSize();
    return Variant(tvAsCVarRef(&data()[m_size]), Variant::CellCopy());
  } else {
    SystemLib::throwInvalidOperationExceptionObject(
      "Cannot pop empty Vector");
  }
}

void c_Vector::resize(uint32_t sz, const Cell* val) {
  if (sz == m_size) {
    return;
  }
  if (sz == 0) {
    ++m_version;
    dropImmCopy();
    decRefArr(arrayData());
    m_arr = staticEmptyVecArray();
    m_size = 0;
  } else if (m_size > sz) {
    // If there were any objects in the part that's being resized away, their
    // desctuctors may mutate this Vector (and need to see it in the fully
    // resized state). The easiest way to do this is to copy the part we're
    // keeping into a new vec, swap them, and decref the old one.
    ++m_version;
    dropImmCopy();
    auto oldAd = arrayData();
    auto from = data();
    auto end = from + sz;
    m_arr = PackedArray::MakeReserveVec(sz);
    auto to = data();
    do {
      cellDup(*from++, *to++);
    } while (from < end);
    arrayData()->m_size = m_size = sz;
    decRefArr(oldAd);
  } else {
    reserve(sz);
    auto elm = data() + m_size;
    auto end = data() + sz;
    do {
      cellDup(*val, *elm);
    } while (++elm < end);
    setSize(sz);
  }
}

void c_Vector::reverse() {
  if (m_size < 2) return;
  mutateAndBump();
  auto start = data();
  auto end = start + m_size - 1;
  do {
    std::swap(start->m_data.num, end->m_data.num);
    std::swap(start->m_type, end->m_type);
  } while (++start < --end);
}

void c_Vector::splice(int64_t startPos, int64_t endPos) {
  // If there were any objects in the part that's being spliced away, their
  // desctuctors may mutate this Vector (and need to see it in the fully
  // spliced state). The easiest way to do this is to copy the part we're
  // keeping into a new vec, swap them, and decref the old one.
  assert(0 <= startPos && startPos < endPos && endPos <= m_size);
  uint32_t sz = m_size - (endPos - startPos);
  ++m_version;
  dropImmCopy();
  auto oldBuf = data();
  auto oldAd = arrayData();
  m_arr = PackedArray::MakeReserveVec(sz);
  auto to = data();
  for (auto from = oldBuf, end = from + startPos; from < end; ++from, ++to) {
    cellDup(*from, *to);
  }
  auto from = oldBuf + endPos;
  auto end = oldBuf + m_size;
  for (; from < end; ++from, ++to) {
    cellDup(*from, *to);
  }
  arrayData()->m_size = m_size = sz;
  decRefArr(oldAd);
}

void c_Vector::php_splice(const Variant& offset,
                          const Variant& len /* = null */,
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
  auto sz = size();
  auto start = offset.toInt64();
  if (UNLIKELY(start >= sz)) return;
  if (start < 0) {
    start += sz;
    if (start < 0) { start = 0; }
  }
  int64_t end;
  if (!len.isNull()) {
    end = len.toInt64();
    if (LIKELY(end > 0)) {
      end += start;
      if (end > sz) end = sz;
    } else {
      if (!end) return;
      end += sz;
      if (end <= start) return;
    }
  } else {
    end = sz;
  }
  splice(start, end);
}

void c_Vector::shuffle() {
  if (m_size < 2) {
    return;
  }
  mutateAndBump();
  uint32_t i = 1;
  do {
    uint32_t j = math_mt_rand(0, i);
    std::swap(data()[i], data()[j]);
  } while (++i < m_size);
}

c_Vector* c_Vector::Clone(ObjectData* obj) {
  return BaseVector::Clone<c_Vector>(obj);
}

Object c_Vector::fromArray(const Class*, const Variant& arr) {
  if (!arr.isArray()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter arr must be an array");
  }
  auto ad = arr.getArrayData();
  uint32_t sz = ad->size();
  if (sz == 0) {
    return Object{req::make<c_Vector>()};
  }
  auto target = req::make<c_Vector>(sz);
  target->setSize(sz);
  auto elm = target->data();
  auto end = elm + sz;
  ssize_t pos = ad->iter_begin();
  do {
    assert(pos != ad->iter_end());
    cellDup(tvToCell(ad->atPos(pos)), *elm);
    pos = ad->iter_advance(pos);
  } while (++elm < end);
  return Object{std::move(target)};
}

void c_Vector::OffsetSet(ObjectData* obj, const TypedValue* key,
                         const TypedValue* val) {
  static_cast<c_Vector*>(obj)->set(key, val);
}

void c_Vector::OffsetUnset(ObjectData* /*obj*/, const TypedValue* /*key*/) {
  SystemLib::throwRuntimeExceptionObject(
    "Cannot unset an element of a Vector");
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
  bool allInts = true;
  bool allStrs = true;
  auto elm = data();
  auto end = elm + m_size;
  do {
    if (acc.isInt(*elm)) {
      if (!allInts) {
        return GenericSort;
      }
      allStrs = false;
    } else if (acc.isStr(*elm)) {
      if (!allStrs) {
        return GenericSort;
      }
      allInts = false;
    } else {
      return GenericSort;
    }
  } while (++elm < end);
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

/////////////////////////////////////////////////////////////////////////////
// c_ImmVector

Class* c_ImmVector::s_cls;

c_ImmVector* c_ImmVector::Clone(ObjectData* obj) {
  return BaseVector::Clone<c_ImmVector>(obj);
}

namespace collections {
/////////////////////////////////////////////////////////////////////////////
// BaseVector

template<class TVector>
ALWAYS_INLINE typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
HHVM_STATIC_METHOD(BaseVector, fromItems, const Variant& iterable) {
  auto target = req::make<TVector>();
  if (iterable.isNull()) return Object{std::move(target)};
  target->init(iterable);
  return Object{std::move(target)};
}

template<class TVector>
ALWAYS_INLINE typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
HHVM_STATIC_METHOD(BaseVector, fromKeysOf, const Variant& container) {
  if (container.isNull()) { return Object{req::make<TVector>()}; }

  const auto& cellContainer = *container.asCell();
  if (UNLIKELY(!isContainer(cellContainer))) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a container (array or collection)");
  }
  return BaseVector::fromKeysOf<TVector>(cellContainer);
}

/////////////////////////////////////////////////////////////////////////////

void CollectionsExtension::initVector() {
  HHVM_ME(VectorIterator, current);
  HHVM_ME(VectorIterator, key);
  HHVM_ME(VectorIterator, valid);
  HHVM_ME(VectorIterator, next);
  HHVM_ME(VectorIterator, rewind);

  Native::registerNativeDataInfo<VectorIterator>(
    s_VectorIterator.get(),
    Native::NDIFlags::NO_SWEEP
  );

  // Common Vector/ImmVector

#define BASE_ME(mn, impl) \
  HHVM_NAMED_ME(HH\\Vector,    mn, impl); \
  HHVM_NAMED_ME(HH\\ImmVector, mn, impl);
  BASE_ME(__construct,  &BaseVector::init);
  BASE_ME(count,        &BaseVector::size);
  BASE_ME(at,           &BaseVector::php_at);
  BASE_ME(get,          &BaseVector::php_get);
  BASE_ME(toArray,      &BaseVector::toArray);
  BASE_ME(getIterator,  &BaseVector::getIterator);
  BASE_ME(firstValue,   &BaseVector::firstValue);
  BASE_ME(lastValue,    &BaseVector::lastValue);
  BASE_ME(linearSearch, &BaseVector::linearSearch);
#undef BASE_ME

#define TMPL_ME(mn, impl) \
  HHVM_NAMED_ME(HH\\Vector,    mn, impl<c_Vector>); \
  HHVM_NAMED_ME(HH\\ImmVector, mn, impl<c_ImmVector>);
  TMPL_ME(keys,           &BaseVector::php_keys);
  TMPL_ME(take,           &BaseVector::php_take);
  TMPL_ME(takeWhile,      &BaseVector::php_takeWhile);
  TMPL_ME(skip,           &BaseVector::php_skip);
  TMPL_ME(skipWhile,      &BaseVector::php_skipWhile);
  TMPL_ME(slice,          &BaseVector::php_slice);
  TMPL_ME(concat,         &BaseVector::php_concat);
  TMPL_ME(zip,            &BaseVector::php_zip);
#undef TMPL_ME

  auto const m     = &BaseVector::php_map<c_Vector, false>;
  auto const immm  = &BaseVector::php_map<c_ImmVector, false>;
  auto const mk    = &BaseVector::php_map<c_Vector, true>;
  auto const immmk = &BaseVector::php_map<c_ImmVector, true>;
  HHVM_NAMED_ME(HH\\Vector,    map,        m);
  HHVM_NAMED_ME(HH\\ImmVector, map,        immm);
  HHVM_NAMED_ME(HH\\Vector,    mapWithKey, mk);
  HHVM_NAMED_ME(HH\\ImmVector, mapWithKey, immmk);

  auto const f     = &BaseVector::php_filter<c_Vector, false>;
  auto const immf  = &BaseVector::php_filter<c_ImmVector, false>;
  auto const fk    = &BaseVector::php_filter<c_Vector, true>;
  auto const immfk = &BaseVector::php_filter<c_ImmVector, true>;
  HHVM_NAMED_ME(HH\\Vector,    filter,        f);
  HHVM_NAMED_ME(HH\\ImmVector, filter,        immf);
  HHVM_NAMED_ME(HH\\Vector,    filterWithKey, fk);
  HHVM_NAMED_ME(HH\\ImmVector, filterWithKey, immfk);

  HHVM_NAMED_STATIC_ME(HH\\Vector,    fromItems,
                       HHVM_STATIC_MN(BaseVector, fromItems)<c_Vector>);
  HHVM_NAMED_STATIC_ME(HH\\Vector,    fromKeysOf,
                       HHVM_STATIC_MN(BaseVector, fromKeysOf)<c_Vector>);
  HHVM_NAMED_STATIC_ME(HH\\Vector,    fromArray,
                       c_Vector::fromArray);
  HHVM_NAMED_STATIC_ME(HH\\ImmVector, fromItems,
                       HHVM_STATIC_MN(BaseVector, fromItems)<c_ImmVector>);
  HHVM_NAMED_STATIC_ME(HH\\ImmVector, fromKeysOf,
                       HHVM_STATIC_MN(BaseVector, fromKeysOf)<c_ImmVector>);

  // Vector specific
  HHVM_NAMED_ME(HH\\Vector, add,          &c_Vector::php_add);
  HHVM_NAMED_ME(HH\\Vector, append,       &c_Vector::php_add);
  HHVM_NAMED_ME(HH\\Vector, addAll,       &c_Vector::php_addAll);
  HHVM_NAMED_ME(HH\\Vector, addAllKeysOf, &c_Vector::php_addAllKeysOf);
  HHVM_NAMED_ME(HH\\Vector, clear,        &c_Vector::php_clear);
  HHVM_NAMED_ME(HH\\Vector, pop,          &c_Vector::pop);
  HHVM_NAMED_ME(HH\\Vector, reverse,      &c_Vector::reverse);
  HHVM_NAMED_ME(HH\\Vector, shuffle,      &c_Vector::shuffle);
  HHVM_NAMED_ME(HH\\Vector, removeKey,    &c_Vector::php_removeKey);
  HHVM_NAMED_ME(HH\\Vector, reserve,      &c_Vector::php_reserve);
  HHVM_NAMED_ME(HH\\Vector, resize,       &c_Vector::php_resize);
  HHVM_NAMED_ME(HH\\Vector, set,          &c_Vector::php_set);
  HHVM_NAMED_ME(HH\\Vector, setAll,       &c_Vector::php_setAll);
  HHVM_NAMED_ME(HH\\Vector, splice,       &c_Vector::php_splice);

  // Materialization
  HHVM_NAMED_ME(HH\\Vector,      toMap,       materialize<c_Map>);
  HHVM_NAMED_ME(HH\\Vector,      toImmMap,    materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\Vector,      toSet,       materialize<c_Set>);
  HHVM_NAMED_ME(HH\\Vector,      toImmSet,    materialize<c_ImmSet>);
  HHVM_NAMED_ME(HH\\Vector,      toImmVector, &c_Vector::getImmutableCopy);

  HHVM_NAMED_ME(HH\\ImmVector,   toMap,       materialize<c_Map>);
  HHVM_NAMED_ME(HH\\ImmVector,   toImmMap,    materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\ImmVector,   toSet,       materialize<c_Set>);
  HHVM_NAMED_ME(HH\\ImmVector,   toImmSet,    materialize<c_ImmSet>);
  HHVM_NAMED_ME(HH\\ImmVector,   toVector,    materialize<c_Vector>);

  loadSystemlib("collections-vector");

  c_Vector::s_cls = Unit::lookupClass(s_HH_Vector.get());
  assertx(c_Vector::s_cls);
  finishClass<c_Vector>();

  c_ImmVector::s_cls = Unit::lookupClass(s_HH_ImmVector.get());
  assertx(c_ImmVector::s_cls);
  finishClass<c_ImmVector>();
}

/////////////////////////////////////////////////////////////////////////////
}}
