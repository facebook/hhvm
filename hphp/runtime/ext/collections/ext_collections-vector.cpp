#include "hphp/runtime/ext/collections/ext_collections-vector.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/zend/zend-math.h"

namespace HPHP { namespace collections {

/////////////////////////////////////////////////////////////////////////////
// VectorIterator

static Variant HHVM_METHOD(VectorIterator, current) {
  return Native::data<VectorIterator>(this_)->current();
}

static int64_t HHVM_METHOD(VectorIterator, key) {
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
  assertx(from->isVanillaVec() && to->isVanillaVec());
  int64_t offset = from_pos - to_pos;
  int64_t to_end = to_pos + size;
  do {
    auto from_elm = VanillaVec::LvalUncheckedInt(from, to_pos + offset);
    auto to_elm = VanillaVec::LvalUncheckedInt(to, to_pos);
    tvDup(*from_elm, to_elm);
  } while (++to_pos < to_end);
}

}  // namespace

/////////////////////////////////////////////////////////////////////////////
// BaseVector

// Used by __construct for Vector and ImmVector
// Used by addAll for Vector only
void BaseVector::addAllImpl(const Variant& t) {
  if (t.isNull()) return;
  CoeffectsAutoGuard _;

  auto ok = IterateV(
    *t.asTypedValue(),
    [&, this](ArrayData* adata) {
      if (adata->empty()) return true;
      if (m_size) {
        reserve(m_size + adata->size());
        return false;
      }
      // The ArrayData backing a Vector must be a vanilla, unmarked vec.
      // Do all three escalations here. Dec-ref any intermediate values we
      // create along the way, but do not dec-ref the original adata.
      auto array = adata;
      if (!array->isVanilla()) {
        array = BespokeArray::ToVanilla(array, "BaseVector::addAllImpl");
      }
      if (array->isVanillaVec() && array->isLegacyArray()) {
        auto const tmp = array->setLegacyArray(array->cowCheck(), false);
        if (array != adata && array != tmp) decRefArr(array);
        array = tmp;
      }
      if (!array->isVanillaVec()) {
        auto const vec = array->toVec(array->cowCheck());
        if (array != adata && array != vec) decRefArr(array);
        array = vec;
      }
      if (array == adata) {
        array->incRefCount();
      }
      dropImmCopy();
      decRefArr(arrayData());
      setArrayData(array);
      m_size = array->size();
      return true;
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
  return VanillaVec::VecEqual(bv1->arrayData(), bv2->arrayData());
}

void BaseVector::addFront(TypedValue tv) {
  dropImmCopy();
  auto oldAd = arrayData();
  setArrayData(VanillaVec::Prepend(oldAd, tv));
  if (arrayData() != oldAd) {
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
  const auto result = *lvalAt(k);
  if (k+1 < m_size) {
    if constexpr (VanillaVec::stores_unaligned_typed_values) {
      auto* data = VanillaVec::entries(arrayData());
      size_t bytes = (m_size-(k+1)) * sizeof(UnalignedTypedValue);
      std::memmove(&data[k], &data[k+1], bytes);
    } else {
      uint32_t i = k, end = m_size - 1;
      do {
        tvCopy(*lvalAt(i + 1), lvalAt(i));
      } while (++i < end);
    }
  }
  decSize();
  return result;
}

void BaseVector::reserveImpl(uint32_t newCap) {
  auto oldAd = arrayData();
  auto const arr = VanillaVec::MakeReserveVec(newCap);
  setArrayData(arr);
  arr->m_size = m_size;
  if (LIKELY(!oldAd->cowCheck())) {
    assertx(oldAd->isVanillaVec());
    if (m_size > 0) {
      const size_t bytes = VanillaVec::capacityToSizeBytes(m_size);
      std::memcpy(arrayData() + 1, oldAd + 1, bytes - sizeof(ArrayData));
      // Mark oldAd as having 0 elements so that the array release logic doesn't
      // decRef the elements (since we teleported the elements to a new array)
      oldAd->m_size = 0;
    }
    decRefArr(oldAd);
  } else {
    if (m_size > 0) {
      copySlice(oldAd, arr, 0, 0, m_size);
    }
    assertx(!oldAd->decWillRelease());
    oldAd->decRefCount();
  }
}

void BaseVector::reserve(uint32_t sz) {
  dropImmCopy();
  if (sz > VanillaVec::capacity(arrayData())) {
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
  if (vec->decReleaseCheck()) VanillaVec::Release(vec);
}

void BaseVector::mutateImpl() {
  auto oldAd = arrayData();
  setArrayData(VanillaVec::Copy(oldAd));
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

void c_Vector::clear() {
  dropImmCopy();
  decRefArr(arrayData());
  setArrayData(ArrayData::CreateVec());
  m_size = 0;
}

void c_Vector::removeKey(int64_t k) {
  if (!contains(k)) return;
  tvDecRefGen(removeKeyImpl(k));
}

Variant c_Vector::pop() {
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Vector");
  }
  mutate();
  decSize();
  const auto tv = *lvalAt(m_size);
  return Variant(tvAsCVarRef(&tv), Variant::TVCopy());
}

void c_Vector::resize(uint32_t sz, const TypedValue* val) {
  if (sz == m_size) {
    return;
  }
  if (sz == 0) {
    dropImmCopy();
    decRefArr(arrayData());
    setArrayData(ArrayData::CreateVec());
    m_size = 0;
  } else if (m_size > sz) {
    // If there were any objects in the part that's being resized away, their
    // desctuctors may mutate this Vector (and need to see it in the fully
    // resized state). The easiest way to do this is to copy the part we're
    // keeping into a new vec, swap them, and decref the old one.
    dropImmCopy();
    auto oldAd = arrayData();
    auto const arr = VanillaVec::MakeReserveVec(sz);
    setArrayData(arr);
    copySlice(oldAd, arr, 0, 0, sz);
    arrayData()->m_size = m_size = sz;
    decRefArr(oldAd);
  } else {
    reserve(sz);
    uint32_t i = m_size;
    do {
      tvDup(*val, lvalAt(i));
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
    tvSwap(lvalAt(i), lvalAt(j));
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
  auto const arr = VanillaVec::MakeReserveVec(sz);
  setArrayData(arr);
  if (startPos > 0) {
    copySlice(oldAd, arr, 0, 0, startPos);
  }
  if (sz > startPos) {
    copySlice(oldAd, arr, endPos, startPos, sz - startPos);
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
      if ((uint64_t)end > (uint64_t)sz) end = sz;
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
    tvSwap(lvalAt(i), lvalAt(j));
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
    tvDup(ad->nvGetVal(pos), target->lvalAt(i));
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

c_ImmVector* c_ImmVector::Clone(ObjectData* obj) {
  return BaseVector::Clone<c_ImmVector>(obj);
}

namespace collections {

void CollectionsExtension::initVector() {
  HHVM_ME(VectorIterator, current);
  HHVM_ME(VectorIterator, key);
  HHVM_ME(VectorIterator, valid);
  HHVM_ME(VectorIterator, next);
  HHVM_ME(VectorIterator, rewind);

  Native::registerNativeDataInfo<VectorIterator>(Native::NDIFlags::NO_SWEEP);

  // Common Vector/ImmVector

#define BASE_ME(mn, impl) \
  HHVM_NAMED_ME(HH\\Vector,    mn, impl); \
  HHVM_NAMED_ME(HH\\ImmVector, mn, impl);
  BASE_ME(__construct,  &BaseVector::init);
  BASE_ME(getIterator,  &BaseVector::getIterator);
#undef BASE_ME

  // Vector specific
  HHVM_NAMED_ME(HH\\Vector, pop,      &c_Vector::pop);
  HHVM_NAMED_ME(HH\\Vector, reverse,  &c_Vector::reverse);
  HHVM_NAMED_ME(HH\\Vector, shuffle,  &c_Vector::shuffle);
  HHVM_NAMED_ME(HH\\Vector, reserve,  &c_Vector::php_reserve);
  HHVM_NAMED_ME(HH\\Vector, resize,   &c_Vector::php_resize);
  HHVM_NAMED_ME(HH\\Vector, splice,   &c_Vector::php_splice);

  // Vector specific functions that return `$this` in userland. These mutate
  // the underlying vector *then* return, with the mutation contained within
  // a `void` C++ function.
  HHVM_NAMED_ME(HH\\Vector, removeKeyNative,  &c_Vector::php_removeKey);
  HHVM_NAMED_ME(HH\\Vector, clearNative,      &c_Vector::clear);

  Native::registerNativePropHandler<CollectionPropHandler>(c_Vector::className());
  Native::registerNativePropHandler<CollectionPropHandler>(c_ImmVector::className());

  Native::registerClassExtraDataHandler(c_Vector::className(), finish_class<c_Vector>);
  Native::registerClassExtraDataHandler(c_ImmVector::className(), finish_class<c_ImmVector>);
}

/////////////////////////////////////////////////////////////////////////////
}}
