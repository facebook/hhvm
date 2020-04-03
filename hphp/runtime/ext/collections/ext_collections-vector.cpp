#include "hphp/runtime/ext/collections/ext_collections-vector.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/zend/zend-math.h"

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

namespace {

void copySlice(ArrayData* from, ArrayData* to,
               int64_t from_pos, int64_t to_pos, int64_t size) {
  assertx(0 < size && from_pos + size <= from->size());
  assertx(from->hasVanillaPackedLayout() && to->hasVanillaPackedLayout());
  int64_t offset = from_pos - to_pos;
  int64_t to_end = to_pos + size;
  do {
    auto from_elm = PackedArray::LvalUncheckedInt(from, to_pos + offset);
    auto to_elm = PackedArray::LvalUncheckedInt(to, to_pos);
    tvDup(*from_elm, to_elm);
  } while (++to_pos < to_end);
}

}  // namespace

/////////////////////////////////////////////////////////////////////////////
// BaseVector

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
  copySlice(m_arr, vec->m_arr, 0, 0, sz);
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
  copySlice(m_arr, vec->m_arr, skipAmt, 0, sz);
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
  copySlice(m_arr, vec->m_arr, skipAmt, 0, sz);
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
    copySlice(m_arr, vec->m_arr, 0, 0, m_size);
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
  uint32_t i = 0;
  do {
    Variant v = iter.second();
    auto pair = req::make<c_Pair>(*dataAt(i), *v.asTypedValue());
    vec->addRaw(make_tv<KindOfObject>(pair.get()));
    ++iter;
  } while (++i < m_size && iter);
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
      if (!m_size && adata->isVecArrayKind()) {
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
      addRaw(v);
    },
    [&, this](ObjectData* coll) {
      if (coll->collectionType() == CollectionType::Pair) {
        reserve(m_size + 2);
      }
    },
    [this](const TypedValue* item) {
      add(*item);
    });

  if (UNLIKELY(!ok)) {
    throw_invalid_collection_parameter();
  }
}

int64_t BaseVector::linearSearch(const Variant& search_value) {
  auto search_tv = *search_value.asTypedValue();
  for (uint32_t i = 0; i < m_size; ++i) {
    if (tvSame(search_tv, *dataAt(i))) {
      return i;
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
  uint32_t i = 0;
  do {
    tvCopy(make_tv<KindOfInt64>(i), vec->dataAt(i));
  } while (++i < m_size);
  return Object{std::move(vec)};
}

bool BaseVector::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  if (UNLIKELY(key->m_type != KindOfInt64)) {
    throwBadKeyType();
    return false;
  }
  const auto vec = static_cast<BaseVector*>(obj);
  const auto result = vec->get(key->m_data.num);
  return result ? !tvIsNull(*result) : false;
}

bool BaseVector::OffsetContains(ObjectData* obj, const TypedValue* key) {
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
  dropImmCopy();
  auto oldAd = arrayData();
  m_arr = PackedArray::PrependVec(oldAd, tv);
  if (m_arr != oldAd) {
    decRefArr(oldAd);
  }
  m_size = arrayData()->m_size;
}

Variant BaseVector::popFront() {
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Vector");
  }
  const auto tv = removeKeyImpl(0);
  return Variant(tvAsCVarRef(&tv), Variant::TVCopy());
}

TypedValue BaseVector::removeKeyImpl(int64_t k) {
  assertx(contains(k));
  mutate();
  const auto result = *dataAt(k);
  if (k+1 < m_size) {
    static_assert(PackedArray::stores_typed_values, "");
    size_t bytes = (m_size-(k+1)) * sizeof(TypedValue);
    std::memmove(&packedData(m_arr)[k], &packedData(m_arr)[k+1], bytes);
  }
  decSize();
  return result;
}

void BaseVector::reserveImpl(uint32_t newCap) {
  auto oldAd = arrayData();
  m_arr = PackedArray::MakeReserveVec(newCap);
  arrayData()->m_size = m_size;
  if (LIKELY(!oldAd->cowCheck())) {
    assertx(oldAd->isVecArrayKind());
    if (m_size > 0) {
      static_assert(PackedArray::stores_typed_values, "");
      size_t bytes = m_size * sizeof(TypedValue);
      std::memcpy(packedData(m_arr), packedData(oldAd), bytes);
      // Mark oldAd as having 0 elements so that the array release logic doesn't
      // decRef the elements (since we teleported the elements to a new array)
      oldAd->m_size = 0;
    }
    decRefArr(oldAd);
  } else {
    if (m_size > 0) {
      copySlice(oldAd, m_arr, 0, 0, m_size);
    }
    assertx(!oldAd->decWillRelease());
    oldAd->decRefCount();
  }
}

void BaseVector::reserve(uint32_t sz) {
  dropImmCopy();
  if (sz > PackedArray::capacity(arrayData())) {
    reserveImpl(sz);
  } else if (!canMutateBuffer()) {
    mutateImpl();
  }
  assertx(canMutateBuffer());
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
  assertx(!oldAd->decWillRelease());
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
  ArrayIter iter(container);
  assertx(iter);
  do {
    vec->addRaw(*iter.first().asTypedValue());
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
    m_immCopy = std::move(vec);
  }
  assertx(!m_immCopy.isNull());
  assertx(arrayData() ==
         static_cast<c_ImmVector*>(m_immCopy.get())->arrayData());
  assertx(!canMutateBuffer());
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
  dropImmCopy();
  decRefArr(arrayData());
  m_arr = ArrayData::CreateVec();
  m_size = 0;
}

void c_Vector::removeKey(int64_t k) {
  if (!contains(k)) return;
  tvDecRefGen(removeKeyImpl(k));
}

void c_Vector::addAllKeysOf(const Variant& container) {
  if (container.isNull()) return;
  auto const& containerCell = container_as_tv(container);

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
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Vector");
  }
  mutate();
  decSize();
  const auto tv = *dataAt(m_size);
  return Variant(tvAsCVarRef(&tv), Variant::TVCopy());
}

void c_Vector::resize(uint32_t sz, const TypedValue* val) {
  if (sz == m_size) {
    return;
  }
  if (sz == 0) {
    dropImmCopy();
    decRefArr(arrayData());
    m_arr = ArrayData::CreateVec();
    m_size = 0;
  } else if (m_size > sz) {
    // If there were any objects in the part that's being resized away, their
    // desctuctors may mutate this Vector (and need to see it in the fully
    // resized state). The easiest way to do this is to copy the part we're
    // keeping into a new vec, swap them, and decref the old one.
    dropImmCopy();
    auto oldAd = arrayData();
    m_arr = PackedArray::MakeReserveVec(sz);
    copySlice(oldAd, m_arr, 0, 0, sz);
    arrayData()->m_size = m_size = sz;
    decRefArr(oldAd);
  } else {
    reserve(sz);
    uint32_t i = m_size;
    do {
      tvDup(*val, dataAt(i));
    } while (++i < sz);
    setSize(sz);
  }
}

void c_Vector::reverse() {
  if (m_size < 2) return;
  mutate();
  uint32_t i = 0;
  uint32_t j = m_size - 1;
  do {
    tvSwap(dataAt(i), dataAt(j));
  } while (++i < --j);
}

void c_Vector::splice(int64_t startPos, int64_t endPos) {
  // If there were any objects in the part that's being spliced away, their
  // desctuctors may mutate this Vector (and need to see it in the fully
  // spliced state). The easiest way to do this is to copy the part we're
  // keeping into a new vec, swap them, and decref the old one.
  assertx(0 <= startPos && startPos < endPos && endPos <= m_size);
  uint32_t sz = m_size - (endPos - startPos);
  dropImmCopy();
  auto oldAd = arrayData();
  m_arr = PackedArray::MakeReserveVec(sz);
  if (startPos > 0) {
    copySlice(oldAd, m_arr, 0, 0, startPos);
  }
  if (sz > startPos) {
    copySlice(oldAd, m_arr, endPos, startPos, sz - startPos);
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
  mutate();
  uint32_t i = 1;
  do {
    const uint32_t j = math_mt_rand(0, i);
    tvSwap(dataAt(i), dataAt(j));
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
  uint32_t i = 0;
  ssize_t pos = ad->iter_begin();
  do {
    assertx(pos != ad->iter_end());
    tvDup(ad->nvGetVal(pos), target->dataAt(i));
    pos = ad->iter_advance(pos);
  } while (++i < sz);
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

  const auto& cellContainer = *container.asTypedValue();
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
  BASE_ME(toArray,      &BaseVector::toPHPArray);
  BASE_ME(toVArray,     &BaseVector::toVArray);
  BASE_ME(toDArray,     &BaseVector::toDArray);
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
  TMPL_ME(skip,           &BaseVector::php_skip);
  TMPL_ME(slice,          &BaseVector::php_slice);
  TMPL_ME(concat,         &BaseVector::php_concat);
  TMPL_ME(zip,            &BaseVector::php_zip);
#undef TMPL_ME

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
