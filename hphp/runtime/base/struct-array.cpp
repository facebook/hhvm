/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/struct-array-defs.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/shape.h"
#include "hphp/runtime/base/tv-helpers.h"

namespace HPHP {

StructArray::StructArray(
  uint32_t size,
  uint32_t pos,
  Shape* shape
)
  : ArrayData(kStructKind)
  , m_shape(shape)
{
  m_sizeAndPos = size | uint64_t{pos} << 32;
  assert(m_pos == pos);
  assert(m_size == size);
  assert(kind() == kStructKind);
  assert(hasExactlyOneRef());
}

StructArray* StructArray::create(
  Shape* shape,
  const TypedValue* values,
  size_t length
) {
  auto ptr = MM().objMalloc(bytesForCapacity(shape->capacity()));
  auto result = new (NotNull{}, ptr) StructArray(length, 0, shape);

  auto data = result->data();
  for (auto i = 0; i < length; ++i) {
    const auto& v = values[i];
    data[i].m_type = v.m_type;
    data[i].m_data = v.m_data;
  }
  return result;
}

StructArray* StructArray::createReversedValues(
  Shape* shape,
  const TypedValue* values,
  size_t length
) {
  auto ptr = MM().objMalloc(bytesForCapacity(shape->capacity()));
  auto result = new (NotNull{}, ptr) StructArray(length, 0, shape);

  auto data = result->data();
  for (auto i = 0; i < length; ++i) {
    const auto& v = values[length - i - 1];
    data[i].m_type = v.m_type;
    data[i].m_data = v.m_data;
  }
  return result;
}

StructArray* StructArray::createNoCopy(Shape* shape, size_t length) {
  auto ptr = MM().objMalloc(bytesForCapacity(shape->capacity()));
  return new (NotNull{}, ptr) StructArray(length, 0, shape);
}

StructArray* StructArray::createStatic(Shape* shape, size_t length) {
  auto ptr = malloc(bytesForCapacity(shape->capacity()));
  return new (NotNull{}, ptr) StructArray(length, 0, shape);
}

StructArray* StructArray::createUncounted(Shape* shape, size_t length) {
  auto ptr = malloc(bytesForCapacity(length));
  return new (NotNull{}, ptr) StructArray(length, 0, shape);
}

ArrayData* StructArray::MakeUncounted(ArrayData* array) {
  auto structArray = asStructArray(array);
  // We don't need to copy the full capacity, since the array won't
  // change once it's uncounted.
  auto size = structArray->size();
  StructArray* result = createUncounted(structArray->shape(), size);
  result->m_hdr.init(HeaderKind::Struct, UncountedValue);
  result->m_sizeAndPos = array->m_sizeAndPos;
  auto const srcData = structArray->data();
  auto const stop    = srcData + size;
  auto targetData    = result->data();
  for (auto ptr = srcData; ptr != stop; ++ptr, ++targetData) {
    auto srcVariant = MixedArray::CreateVarForUncountedArray(tvAsCVarRef(ptr));
    tvCopy(*srcVariant.asTypedValue(),
           *targetData);
  }
  assert(result->m_pos == structArray->m_pos);
  assert(result->isUncounted());
  return result;
}

void StructArray::Release(ArrayData* ad) {
  assert(ad->isRefCounted());
  assert(ad->hasExactlyOneRef());
  auto array = asStructArray(ad);

  auto const size = array->size();
  auto const data = array->data();
  auto const stop = data + size;
  for (auto ptr = data; ptr != stop; ++ptr) {
    tvRefcountedDecRef(*ptr);
  }
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }

  auto const cap = array->capacity();
  MM().objFree(array, sizeof(StructArray) + sizeof(TypedValue) * cap);
}

void StructArray::ReleaseUncounted(ArrayData* ad) {
  assert(ad->isUncounted());
  auto structArray = asStructArray(ad);

  auto const data = structArray->data();
  auto const stop = data + structArray->size();
  for (auto ptr = data; ptr != stop; ++ptr) {
    MixedArray::ReleaseUncountedTypedValue(*ptr);
  }

  // We better not have strong iterators associated with uncounted
  // arrays.
  if (debug && UNLIKELY(strong_iterators_exist())) {
    for_each_strong_iterator([&] (const MIterTable::Ent& miEnt) {
      assert(miEnt.array != structArray);
    });
  }

  std::free(structArray);
}

const TypedValue* StructArray::NvGetInt(const ArrayData*, int64_t) {
  return nullptr;
}

const TypedValue* StructArray::NvGetStr(
  const ArrayData* ad,
  const StringData* property
) {
  const auto structArray = asStructArray(ad);
  auto offset = structArray->shape()->offsetFor(property);
  if (offset == PropertyTable::kInvalidOffset) return nullptr;
  return &structArray->data()[offset];
}

void StructArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  const auto structArray = asStructArray(ad);

  auto str = const_cast<StringData*>(structArray->shape()->keyForOffset(pos));
  out->m_type = KindOfStaticString;
  out->m_data.pstr = str;
}

ArrayData* StructArray::SetInt(ArrayData* ad, int64_t k, Cell v, bool copy) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return mixedArray->addVal(k, v);
}

ArrayData* StructArray::SetStr(
  ArrayData* ad,
  StringData* k,
  Cell v,
  bool copy
) {
  auto structArray = asStructArray(ad);
  auto shape = structArray->shape();
  auto result = structArray;

  auto offset = shape->offsetFor(k);
  bool isNewProperty = offset == PropertyTable::kInvalidOffset;

  auto convertToMixedAndAdd = [&]() {
    auto mixed = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
    return mixed->addValNoAsserts(k, v);
  };

  if (isNewProperty) {
    StringData* staticKey;
    // We don't support adding non-static strings yet.
    if (k->isStatic()) {
      staticKey = k;
    } else {
      staticKey = lookupStaticString(k);
      if (!staticKey) return convertToMixedAndAdd();
    }

    auto newShape = shape->transition(staticKey);
    if (!newShape) return convertToMixedAndAdd();
    result = copy ? CopyAndResizeIfNeeded(structArray, newShape)
                  : ResizeIfNeeded(structArray, newShape);
    offset = result->shape()->offsetFor(staticKey);
    assert(offset != PropertyTable::kInvalidOffset);
    TypedValue* dst = &result->data()[offset];
    // TODO(#3888164): we should restructure things so we don't have to
    // check KindOfUninit here.
    if (UNLIKELY(v.m_type == KindOfUninit)) v = make_tv<KindOfNull>();
    cellDup(v, *dst);
    return result;
  }

  if (copy) {
    result = asStructArray(Copy(structArray));
  }

  assert(offset != PropertyTable::kInvalidOffset);
  TypedValue* dst = &result->data()[offset];
  if (UNLIKELY(v.m_type == KindOfUninit)) v = make_tv<KindOfNull>();
  cellSet(v, *tvToCell(dst));
  return result;
}

size_t StructArray::Vsize(const ArrayData*) {
  not_reached();
}

const Variant& StructArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  const auto array = asStructArray(ad);
  assert(array->size() == array->shape()->size());
  assert(pos != array->size());
  return tvAsCVarRef(&array->data()[pos]);
}

bool StructArray::IsVectorData(const ArrayData*) {
  return false;
}

bool StructArray::ExistsInt(const ArrayData* ad, int64_t k) {
  return false;
}

bool StructArray::ExistsStr(const ArrayData* ad, const StringData* property) {
  auto structArray = asStructArray(ad);
  return structArray->shape()->hasOffsetFor(property);
}

ArrayData* StructArray::LvalInt(
  ArrayData* ad,
  int64_t k,
  Variant*& ret,
  bool copy
) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return mixedArray->addLvalImpl(k, ret);
}

ArrayData* StructArray::LvalStr(
  ArrayData* ad,
  StringData* property,
  Variant*& ret,
  bool copy
) {
  auto structArray = asStructArray(ad);
  auto shape = structArray->shape();
  auto offset = shape->offsetFor(property);
  if (offset != PropertyTable::kInvalidOffset) {
    auto const result = asStructArray(
      copy ? Copy(structArray) : structArray);
    ret = &tvAsVariant(&result->data()[offset]);
    return result;
  }

  auto convertToMixedAndAdd = [&]() {
    auto mixed = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
    return mixed->addLvalImpl(property, ret);
  };

  // We don't support adding non-static strings yet.
  StringData* staticKey;
  if (property->isStatic()) {
    staticKey = property;
  } else {
    staticKey = lookupStaticString(property);
    if (!staticKey) return convertToMixedAndAdd();
  }

  auto newShape = shape->transition(staticKey);
  if (!newShape) return convertToMixedAndAdd();
  auto result = copy ? CopyAndResizeIfNeeded(structArray, newShape)
                     : ResizeIfNeeded(structArray, newShape);

  assert(newShape->hasOffsetFor(staticKey));
  offset = newShape->offsetFor(staticKey);
  tvWriteNull(&result->data()[offset]);
  ret = &tvAsVariant(&result->data()[offset]);
  return result;
}

ArrayData* StructArray::LvalNew(ArrayData* ad, Variant*& ret, bool copy) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return MixedArray::LvalNew(mixedArray->asArrayData(), ret, false);
}

ArrayData* StructArray::SetRefInt(
  ArrayData* ad,
  int64_t k,
  Variant& v,
  bool copy
) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return MixedArray::SetRefInt(mixedArray->asArrayData(), k, v, false);
}

ArrayData* StructArray::SetRefStr(
  ArrayData* ad,
  StringData* k,
  Variant& v,
  bool copy
) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return MixedArray::SetRefStr(mixedArray->asArrayData(), k, v, false);
}

ArrayData* StructArray::RemoveInt(ArrayData* ad, int64_t, bool) {
  return ad;
}

ArrayData* StructArray::RemoveStr(
  ArrayData* ad,
  const StringData* k,
  bool copy
) {
  auto structArray = asStructArray(ad);
  if (structArray->shape()->hasOffsetFor(k)) {
    auto const mixed = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
    auto pos = mixed->findForRemove(k, k->hash());
    if (validPos(pos)) mixed->erase(pos);
    return mixed;
  }
  return copy ? Copy(structArray) : structArray;
}

ssize_t StructArray::IterBegin(const ArrayData*) {
  return 0;
}

ssize_t StructArray::IterLast(const ArrayData* ad) {
  const auto structArray = asStructArray(ad);
  return structArray->size() ? structArray->size() - 1 : 0;
}

ssize_t StructArray::IterEnd(const ArrayData* ad) {
  return asStructArray(ad)->size();
}

ssize_t StructArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  if (pos < asStructArray(ad)->size()) {
    ++pos;
  }
  return pos;
}

ssize_t StructArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  if (pos > 0) {
    return pos - 1;
  }
  return asStructArray(ad)->size();
}

bool StructArray::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  auto structArray = asStructArray(ad);
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = 0;
  } else if (fp.m_pos == structArray->size()) {
    return false;
  } else {
    fp.m_pos = IterAdvance(structArray, fp.m_pos);
  }
  if (fp.m_pos == structArray->size()) {
    return false;
  }
  // We set ad's internal cursor to point to the next element
  // to conform with PHP5 behavior
  structArray->m_pos = IterAdvance(structArray, fp.m_pos);
  return true;
}

ArrayData* StructArray::Copy(const ArrayData* ad) {
  auto old = asStructArray(ad);
  auto shape = old->shape();

  auto result = StructArray::createNoCopy(shape, shape->size());
  result->m_pos = old->m_pos;

  assert(result->m_size == result->shape()->size());
  assert(result->size() == old->size());
  auto const srcData = old->data();
  auto const stop = srcData + old->size();
  auto targetData = result->data();
  for (auto ptr = srcData; ptr != stop; ++ptr, ++targetData) {
    tvDupFlattenVars(ptr, targetData, old);
  }

  assert(result->m_size == result->shape()->size());
  assert(result->hasExactlyOneRef());
  return result;
}

ArrayData* StructArray::CopyWithStrongIterators(const ArrayData* ad) {
  auto const cpy = Copy(ad);
  if (LIKELY(strong_iterators_exist())) {
    // This returns its first argument just so we can tail call it.
    return move_strong_iterators(cpy, const_cast<ArrayData*>(ad));
  }
  return cpy;
}

ArrayData* StructArray::CopyStatic(const ArrayData* ad) {
  auto structArray = asStructArray(ad);
  auto shape = structArray->shape();
  auto ret = StructArray::createStatic(shape, structArray->size());

  ret->m_pos = structArray->m_pos;

  auto const srcData = structArray->data();
  auto const size    = structArray->size();
  auto const stop    = srcData + size;
  auto targetData    = ret->data();
  for (auto ptr = srcData; ptr != stop; ++ptr, ++targetData) {
    tvDupFlattenVars(ptr, targetData, structArray);
  }

  assert(ret->hasExactlyOneRef());
  return ret;
}

ArrayData* StructArray::EscalateForSort(ArrayData* ad, SortFunction) {
  return ToMixedCopy(asStructArray(ad));
}

void StructArray::Ksort(ArrayData*, int, bool) {
  not_reached();
}

void StructArray::Sort(ArrayData*, int, bool) {
  not_reached();
}

void StructArray::Asort(ArrayData*, int, bool) {
  not_reached();
}

bool StructArray::Uksort(ArrayData*, const Variant&) {
  not_reached();
}

bool StructArray::Usort(ArrayData*, const Variant&) {
  not_reached();
}

bool StructArray::Uasort(ArrayData*, const Variant&) {
  not_reached();
}

ArrayData* StructArray::ZSetInt(ArrayData* ad, int64_t k, RefData* v) {
  return MixedArray::ZSetInt(ToMixedCopy(asStructArray(ad)), k, v);
}

ArrayData* StructArray::ZSetStr(ArrayData* ad, StringData* k, RefData* v) {
  return MixedArray::ZSetStr(ToMixedCopy(asStructArray(ad)), k, v);
}

ArrayData* StructArray::ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  return MixedArray::ZAppend(ToMixedCopy(asStructArray(ad)), v, key_ptr);
}

ArrayData* StructArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return MixedArray::Append(mixedArray->asArrayData(), v, false);
}

ArrayData* StructArray::AppendRef(ArrayData* ad, Variant& v, bool copy) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return MixedArray::AppendRef(mixedArray->asArrayData(), v, false);
}

ArrayData* StructArray::AppendWithRef(
  ArrayData* ad,
  const Variant& v,
  bool copy
) {
  auto structArray = asStructArray(ad);
  auto mixedArray = copy ? ToMixedCopy(structArray) : ToMixed(structArray);
  return MixedArray::AppendWithRef(mixedArray->asArrayData(), v, false);
}

ArrayData* StructArray::PlusEq(ArrayData* ad, const ArrayData* elems) {
  auto structArray = asStructArray(ad);
  auto const neededSize = structArray->size() + elems->size();
  auto const mixedArray = ToMixedCopyReserve(structArray, neededSize);
  try {
    auto const ret = MixedArray::PlusEq(mixedArray, elems);
    assert(ret == mixedArray);
    assert(mixedArray->hasExactlyOneRef());
    return ret;
  } catch (...) {
    MixedArray::Release(mixedArray);
    throw;
  }
}

ArrayData* StructArray::Merge(ArrayData* ad, const ArrayData* elems) {
  auto structArray = asStructArray(ad);
  auto const neededSize = structArray->m_size + elems->size();
  auto const mixedArray = ToMixedCopyReserve(structArray, neededSize);
  return MixedArray::ArrayMergeGeneric(mixedArray, elems);
}

ArrayData* StructArray::Pop(ArrayData* ad, Variant& value) {
  return MixedArray::Pop(ToMixed(asStructArray(ad))->asArrayData(), value);
}

ArrayData* StructArray::Dequeue(ArrayData* ad, Variant& value) {
  return MixedArray::Dequeue(ToMixed(asStructArray(ad))->asArrayData(), value);
}

ArrayData* StructArray::Prepend(ArrayData* ad, const Variant& v, bool copy) {
  return MixedArray::Prepend(ToMixed(asStructArray(ad)), v, copy);
}

void StructArray::Renumber(ArrayData* ad) {
  // No integer keys so nothing to do.
}

void StructArray::OnSetEvalScalar(ArrayData* ad) {
  auto structArray = asStructArray(ad);
  auto ptr = structArray->data();
  auto const stop = ptr + structArray->size();
  for (; ptr != stop; ++ptr) {
    tvAsVariant(ptr).setEvalScalar();
  }
  // All keys are already static strings.
}

ArrayData* StructArray::Escalate(const ArrayData* ad) {
  return const_cast<ArrayData*>(ad);
}

bool StructArray::checkInvariants(const ArrayData* ad) {
  assert(ad->getCount() != 0);
  return true;
}

StructArray* StructArray::Grow(StructArray* old, Shape* newShape) {
  assert(old->shape()->transitionRequiresGrowth());

  auto result = StructArray::create(newShape, old->data(),
    old->shape()->size());
  result->m_size = newShape->size();

  if (UNLIKELY(strong_iterators_exist())) {
    move_strong_iterators(result, old);
  }

  old->m_size = 0;
  if (debug) {
    // For debug builds, set m_pos to 0 as well to make the
    // asserts in checkInvariants() happy.
    old->m_pos = 0;
  }

  assert(result->hasExactlyOneRef());
  return result;
}

MixedArray* StructArray::ToMixedHeader(size_t neededSize) {
  auto const scale   = computeScaleFromSize(neededSize);
  auto const ad      = reqAllocArray(scale);

  ad->m_sizeAndPos       = 0; // We'll set size and pos later.
  ad->m_hdr.init(HeaderKind::Mixed, 1);
  ad->m_scale_used       = scale; // used=0
  ad->m_nextKI           = 0; // There were never any numeric indices.

  assert(ad->kind() == ArrayData::kMixedKind);
  assert(ad->m_size == 0);
  assert(ad->m_pos == 0);
  assert(ad->hasExactlyOneRef());
  assert(ad->m_used == 0);
  assert(ad->m_scale == scale);
  return ad;
}

MixedArray* StructArray::ToMixed(StructArray* old) {
  auto const oldSize = old->size();
  auto const ad      = ToMixedHeader(oldSize + 1);
  auto const srcData = old->data();
  auto shape         = old->shape();

  memset(ad->hashTab(), static_cast<uint8_t>(MixedArray::Empty),
    sizeof(int32_t) * ad->hashSize());

  for (auto i = 0; i < oldSize; ++i) {
    auto key = const_cast<StringData*>(shape->keyForOffset(i));
    auto& e = ad->addKeyAndGetElem(key);
    tvCopy(srcData[i], e.data);
  }

  old->m_size = 0;
  ad->m_pos = old->m_pos;
  if (debug) {
    // For debug builds, set m_pos to 0 as well to make the
    // asserts in checkInvariants() happy.
    old->m_pos = 0;
  }
  assert(ad->checkInvariants());
  assert(!ad->isFull());
  assert(ad->hasExactlyOneRef());
  return ad;
}

MixedArray* StructArray::ToMixedCopy(const StructArray* old) {
  auto const oldSize = old->size();
  auto const ad      = ToMixedHeader(oldSize + 1);
  auto const srcData = old->data();
  auto shape         = old->shape();

  memset(ad->hashTab(), static_cast<uint8_t>(MixedArray::Empty),
    sizeof(int32_t) * ad->hashSize());

  for (auto i = 0; i < oldSize; ++i) {
    auto key = const_cast<StringData*>(shape->keyForOffset(i));
    auto& e = ad->addKeyAndGetElem(key);
    tvDupFlattenVars(&srcData[i], &e.data, old);
  }

  ad->m_pos = old->m_pos;
  assert(ad->checkInvariants());
  assert(!ad->isFull());
  assert(ad->hasExactlyOneRef());
  return ad;
}

MixedArray* StructArray::ToMixedCopyReserve(
  const StructArray* old,
  size_t neededSize
) {
  assert(neededSize >= old->size());
  auto const ad      = ToMixedHeader(neededSize);
  auto const oldSize = old->size();
  auto const srcData = old->data();
  auto shape         = old->shape();

  memset(ad->hashTab(), static_cast<uint8_t>(MixedArray::Empty),
    sizeof(int32_t) * ad->hashSize());

  for (auto i = 0; i < oldSize; ++i) {
    auto key = const_cast<StringData*>(shape->keyForOffset(i));
    auto& e = ad->addKeyAndGetElem(key);
    tvDupFlattenVars(&srcData[i], &e.data, old);
  }

  ad->m_pos = old->m_pos;
  assert(ad->checkInvariants());
  assert(ad->hasExactlyOneRef());
  return ad;
}

StructArray* StructArray::CopyAndResizeIfNeeded(
  const StructArray* array,
  Shape* newShape
) {
  if (!array->shape()->transitionRequiresGrowth()) {
    auto ret = asStructArray(Copy(array));
    ret->setShape(newShape);
    return ret;
  }
  auto const copy = asStructArray(Copy(array));
  auto const ret = Grow(copy, newShape);
  Release(copy);
  return ret;
}

StructArray* StructArray::ResizeIfNeeded(StructArray* array, Shape* newShape) {
  if (array->shape()->transitionRequiresGrowth()) {
    return Grow(array, newShape);
  }
  array->setShape(newShape);
  return array;
}

}
