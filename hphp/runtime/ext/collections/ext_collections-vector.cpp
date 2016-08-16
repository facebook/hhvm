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
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), ctx, argc, argv);
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

  auto nv = req::make<TVector>();
  uint32_t sz = m_size;
  nv->reserve(sz);
  assert(nv->canMutateBuffer());
  int32_t version = m_version;
  constexpr int64_t argc = useKey ? 2 : 1;
  TypedValue argv[argc];
  if (useKey) {
    argv[0].m_type = KindOfInt64;
  }
  for (uint32_t i = 0; i < sz; ++i) {
    TypedValue* tv = &nv->data()[i];
    if (useKey) {
      argv[0].m_data.num = i;
    }
    argv[argc-1] = data()[i];
    g_context->invokeFuncFew(tv, ctx, argc, argv);
    if (UNLIKELY(version != m_version)) {
      tvRefcountedDecRef(tv);
      throw_collection_modified();
    }
    nv->incSize();
  }
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
  auto nv = req::make<TVector>();
  uint32_t sz = m_size;
  int32_t version = m_version;
  assert(nv->canMutateBuffer());
  constexpr int64_t argc = useKey ? 2 : 1;
  TypedValue argv[argc];
  if (useKey) {
    argv[0].m_type = KindOfInt64;
  }
  for (uint32_t i = 0; i < sz; ++i) {
    if (useKey) {
      argv[0].m_data.num = i;
    }
    argv[argc-1] = data()[i];
    bool b = invokeAndCastToBool(ctx, argc, argv);
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
  auto vec = req::make<TVector>();
  assert(vec->m_size == 0);
  int32_t version UNUSED;
  if (std::is_same<c_Vector, TVector>::value) {
    version = m_version;
  }
  for (uint32_t i = 0; i < m_size; ++i) {
    bool b = invokeAndCastToBool(ctx, 1, &data()[i]);
    if (std::is_same<c_Vector, TVector>::value) {
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
  auto vec = req::make<TVector>();
  assert(vec->canMutateBuffer());
  uint32_t i = 0;
  int32_t version UNUSED;
  if (std::is_same<c_Vector, TVector>::value) {
    version = m_version;
  }
  for (; i < m_size; ++i) {
    bool b = invokeAndCastToBool(ctx, 1, &data()[i]);
    if (std::is_same<c_Vector, TVector>::value) {
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

template<class TVector>
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

// Used by __construct for Vector and ImmVector
// Used by addAll for Vector only
void BaseVector::addAllImpl(const Variant& t) {
  if (t.isNull()) return;

  bool skip_ref_check = false;
  auto ok = IterateV(
    *t.asTypedValue(),
    [&, this](ArrayData* adata) {
      if (adata->empty()) return true;
      if (!m_size) {

        // Not isPackedLayout(), we don't want to take ownership of Hack arrays.
        if (adata->isPacked()) {
          // Collections can't contains refs, so we need to check for their
          // presence before taking ownership of this array.
          bool contains_ref = false;
          if (!skip_ref_check) {
            PackedArray::IterateV(
              adata,
              [&](const TypedValue* value) {
                if (value->m_type == KindOfRef) {
                  contains_ref = true;
                  return true;
                }
                return false;
              }
            );
          }

          if (!contains_ref) {
            auto* oldAd = arrayData();
            m_arr = adata;
            adata->incRefCount();
            m_capacity = arrayData()->cap();
            m_size = arrayData()->m_size;
            decRefArr(oldAd);
            ++m_version;
            return true;
          }
        }
      }
      reserve(m_size + adata->size());
      mutateAndBump();
      return false;
    },
    [this](const TypedValue* item) {
      addRaw(tvAsCVarRef(item));
    },
    [&, this](ObjectData* coll) {
      if (coll->collectionType() == CollectionType::Pair) {
        mutateAndBump();
      } else if (coll->collectionType() == CollectionType::Vector) {
        // If its a vector collection we don't need to worry about refs in the
        // above fast-path.
        skip_ref_check = true;
      }
    },
    [this](const TypedValue* item) {
      add(tvToCell(item));
    });

  if (UNLIKELY(!ok)) {
    throw_invalid_collection_parameter();
  }
}

int64_t BaseVector::linearSearch(const Variant& search_value) {
  uint32_t sz = m_size;
  for (uint32_t i = 0; i < sz; ++i) {
    if (same(search_value, tvAsCVarRef(&data()[i]))) {
      return i;
    }
  }
  return -1;
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
  : ObjectData(cls, collections::objectFlags, kind)
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

void BaseVector::mutateImpl() {
  assert(arrayData()->cowCheck());
  dropImmCopy();
  if (canMutateBuffer()) {
    return;
  }
  assert(arrayData()->cowCheck());
  if (!m_size) {
    arrayData()->decRefCount();
    m_arr = staticEmptyArray();
    m_capacity = 0;
    return;
  }
  auto* oldAd = arrayData();
  m_arr = PackedArray::Copy(oldAd);
  assert(oldAd->cowCheck());
  oldAd->decRefCount();
}

template<class TVector>
ALWAYS_INLINE typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, TVector*>::type
BaseVector::Clone(ObjectData* obj) {
  auto thiz = static_cast<TVector*>(obj);
  auto target = static_cast<TVector*>(
    TVector::instanceCtor(TVector::classof()));
  if (!thiz->m_size) {
    return target;
  }
  thiz->arrayData()->incRefCount();
  target->m_arr = thiz->m_arr;
  target->m_size = thiz->m_size;
  target->m_capacity = thiz->m_capacity;
  return target;
}

template<class TVector> typename
  std::enable_if<std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseVector::fromKeysOf(const TypedValue& container) {
  ArrayIter iter(container);
  auto target = req::make<TVector>();
  target->reserve(getContainerSize(container));
  assert(target->canMutateBuffer());
  for (; iter; ++iter) { target->addRaw(iter.first()); }
  return Object{std::move(target)};
}

// This function will create a immutable copy of this Vector (if it doesn't
// already exist) and then return it
Object BaseVector::getImmutableCopy() {
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
  assert(arrayData()->cowCheck());
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
  m_arr = staticEmptyArray();
  m_size = 0;
  m_capacity = 0;
}

void c_Vector::removeKey(int64_t k) {
  if (!contains(k)) return;
  mutateAndBump();
  uint64_t datum = data()[k].m_data.num;
  DataType t = data()[k].m_type;
  if (k+1 < m_size) {
    memmove(&data()[k], &data()[k+1],
            (m_size-(k+1)) * sizeof(TypedValue));
  }
  decSize();
  tvRefcountedDecRefHelper(t, datum);
}

void c_Vector::addAllKeysOf(const Variant& container) {
  if (container.isNull()) return;
  const auto& containerCell = container_as_cell(container);

  auto sz = getContainerSize(containerCell);
  ArrayIter iter(containerCell);
  if (!sz || !iter) return;
  reserve(m_size + sz);

  mutateAndBump();
  assert(canMutateBuffer());
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

void c_Vector::resize(int64_t sz, const Cell* val) {
  assert(sz >= 0);
  if (sz == m_size) {
    return;
  }
  if (sz > (int64_t)m_capacity) {
    reserve(sz);
  } else {
    mutate();
  }
  assert(canMutateBuffer());
  ++m_version;
  uint32_t requestedSize = (uint32_t)sz;
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

void c_Vector::reverse() {
  if (m_size <= 1) return;
  mutateAndBump();
  TypedValue* start = data();
  TypedValue* end = start + m_size - 1;
  for (; start < end; ++start, --end) {
    std::swap(start->m_data.num, end->m_data.num);
    std::swap(start->m_type, end->m_type);
  }
}

void c_Vector::splice(int64_t startPos, int64_t endPos) {
  assert(startPos >= 0 && endPos <= m_size);
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
  if (endPos < m_size) {
    memmove(&data()[startPos], &data()[endPos],
            (m_size - endPos) * sizeof(TypedValue));
  }
  setSize(m_size - (endPos - startPos));
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
  if (UNLIKELY(start > sz)) return;
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
      if (end < start) return;
    }
  } else {
    end = sz;
  }
  splice(start, end);
}

void c_Vector::shuffle() {
  if (m_size <= 1) {
    return;
  }
  mutateAndBump();
  for (uint32_t i = 1; i < m_size; ++i) {
    uint32_t j = math_mt_rand(0, i);
    std::swap(data()[i], data()[j]);
  }
}

c_Vector* c_Vector::Clone(ObjectData* obj) {
  return BaseVector::Clone<c_Vector>(obj);
}

Object c_Vector::fromArray(const Class*, const Variant& arr) {
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

void c_Vector::OffsetSet(ObjectData* obj, const TypedValue* key,
                         const TypedValue* val) {
  static_cast<c_Vector*>(obj)->set(key, val);
}

void c_Vector::OffsetUnset(ObjectData* obj, const TypedValue* key) {
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
