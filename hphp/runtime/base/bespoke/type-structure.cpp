/*
  +----------------------------------------------------------------------+
  | HipHop for PHP                                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/bespoke/type-structure.h"

#include "hphp/runtime/base/bespoke/escalation-logging.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/vm/jit/type.h"

namespace HPHP::bespoke {

struct StaticString;

///////////////////////////////////////////////////////////////////////////////

TypeStructure* TypeStructure::As(ArrayData* ad) {
  auto const result = reinterpret_cast<TypeStructure*>(ad);
  assertx(result->checkInvariants());
  return result;
}

const TypeStructure* TypeStructure::As(const ArrayData* ad) {
  return As(const_cast<ArrayData*>(ad));
}

namespace {

size_t getSizeOfTypeStruct(Kind kind) {
  // TODO: return different sizes for children structs
  return sizeof(TypeStructure);
}
void moveFieldToVanilla(ArrayData* vad, StringData* key, TypedValue tv) {
  auto const type = tv.m_type;
  if (type == KindOfUninit || (type == KindOfBoolean && !val(tv).num)) {
    return;
  }
  auto const res = VanillaDict::SetStrMove(vad, key, tv);
  assertx(vad == res);
  if (isRefcountedType(type)) tvIncRefGen(tv);
  vad = res;
}

void setStringField(StringData*& field, TypedValue v) {
  assertx(tvIsString(v));
  if (field) field->decRefAndRelease();
  field = val(v).pstr;
}
void setArrayField(ArrayData*& field, TypedValue v) {
  assertx(tvIsArrayLike(v));
  if (field) field->decRefAndRelease();
  field = val(v).parr;
}

}

size_t TypeStructure::sizeIndex(Kind kind) {
  return MemoryManager::size2Index(getSizeOfTypeStruct(kind));
}

LayoutIndex TypeStructure::GetLayoutIndex() {
  return LayoutIndex{uint16_t(kTypeStructureLayoutByte << 8)};
}

void TypeStructure::InitializeLayouts() {
  new TypeStructureLayout();
}

template <bool Static>
TypeStructure* TypeStructure::MakeReserve(bool legacy, Kind kind) {
  auto const index = MemoryManager::size2Index(getSizeOfTypeStruct(kind));
  auto const size = MemoryManager::sizeIndex2Size(index);
  auto const alloc = [&]{
    if (!Static) return tl_heap->objMallocIndex(index);
    return RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size);
  }();

  auto const tad = static_cast<TypeStructure*>(alloc);
  auto const aux = packSizeIndexAndAuxBits(
    sizeIndex(kind), legacy ? ArrayData::kLegacyArray : 0);
  tad->initHeader_16(HeaderKind::BespokeDict,
    Static ? StaticValue : OneReference, aux);

  tad->m_size = numFields(kind);
  tad->m_layout_index = GetLayoutIndex();
  tad->m_extra_lo8 = 0;
  tad->m_extra_hi8 = 0;
  tad->m_alias = nullptr;
  tad->m_typevars = nullptr;
  tad->m_typevar_types = nullptr;

  assertx(tad->checkInvariants());
  return tad;
}

template <typename Type>
void setFields(Type* result, ArrayData* ad) {
  VanillaDict::IterateKV(VanillaDict::as(ad), [&](auto k, auto v) {
    assertx(tvIsString(k));
    auto const res = Type::setField(result, val(k).pstr, v);
    if (res && isRefcountedType(v.m_type)) tvIncRefGen(v);
  });
}

void TypeStructure::setBitField(TypedValue v, BitFieldOffsets offset) {
  assertx(tvIsBool(v));
  uint8_t val = v.val().num;
  clearBitField(offset);
  m_extra_lo8 |= val << offset;
}

// caller must incref TypedValue
bool TypeStructure::setField(TypeStructure* tad, StringData* k, TypedValue v) {
  auto compare = k->isStatic()
    ? [](StringData* a, StringData* b) { return a == b; }
    : [](StringData* a, StringData* b) { return a->same(b); };

  if (compare(s_kind.get(), k)) {
    assertx(tvIsInt(v));
    tad->m_extra_hi8 = safe_cast<uint8_t>(val(v).num);
  } else if (compare(s_nullable.get(), k)) {
    tad->setBitField(v, kNullableOffset);
  } else if (compare(s_soft.get(), k)) {
    tad->setBitField(v, kSoftOffset);
  } else if (compare(s_like.get(), k)) {
    tad->setBitField(v, kLikeOffset);
  } else if (compare(s_opaque.get(), k)) {
    tad->setBitField(v, kOpaqueOffset);
  } else if (compare(s_optional_shape_field.get(), k)) {
    tad->setBitField(v, kOptionalShapeFieldOffset);
  } else if (compare(s_alias.get(), k)) {
    setStringField(tad->m_alias, v);
  } else if (compare(s_typevars.get(), k)) {
    setStringField(tad->m_typevars, v);
  } else if (compare(s_typevar_types.get(), k)) {
    setArrayField(tad->m_typevar_types, v);
  } else {
    return false;
  }
  return true;
}

TypeStructure* TypeStructure::MakeFromVanilla(ArrayData* ad) {
  if (!ad->isVanillaDict() || !ad->exists(s_kind)) return nullptr;
  auto const kind =
    static_cast<Kind>(Variant::wrap(ad->get(s_kind)).toInt64Val());

  auto const result = ad->isStatic()
    ? MakeReserve<true>(ad->isLegacyArray(), kind)
    : MakeReserve<false>(ad->isLegacyArray(), kind);

  setFields<TypeStructure>(result, ad);

  assertx(result->checkInvariants());
  return result;
}

bool TypeStructure::checkInvariants() const {
  assertx(0 <= kind() && kind() <= HPHP::TypeStructure::kMaxResolvedKind);
  assertx(m_alias == nullptr || m_alias->kindIsValid());
  assertx(m_typevars == nullptr || m_typevars->kindIsValid());
  assertx(m_typevar_types == nullptr || m_typevar_types->kindIsValid());
  return true;
}

bool TypeStructure::isValidTypeStructure(ArrayData* ad) {
  if (!ad->isVanillaDict() || !ad->exists(s_kind)) return false;

  // check that the key exists and that the type matches
  auto fields = 0;
#define X(Field, FieldString, KindOfType)                             \
  if (ad->exists(s_##FieldString) &&                                  \
      equivDataTypes(ad->get(s_##FieldString).type(), KindOfType)) {  \
    fields++;                                                         \
  }
TYPE_STRUCTURE_FIELDS
#undef X

  // all elements in ad must exist in type structure
  return fields == ad->size();
}

ArrayData* TypeStructure::escalateWithCapacity(
    size_t capacity, const char* reason) const {
  assertx(capacity >= numFields(typeKind()));
  logEscalateToVanilla(this, reason);

  auto ad = VanillaDict::MakeReserveDict(capacity);
  ad->setLegacyArrayInPlace(isLegacyArray());

#define X(Field, FieldString, KindOfType) {                           \
  auto const key = s_##FieldString.get();                             \
  auto const tv = TypeStructure::NvGetStr(this, key);                 \
  moveFieldToVanilla(ad, key, tv);                                    \
}
TYPE_STRUCTURE_FIELDS
#undef X

  assertx(isValidTypeStructure(ad));
  return ad;
}

TypeStructure* TypeStructure::copy() const {
  auto const sizeIdx = sizeIndex(typeKind());
  auto const mem = tl_heap->objMallocIndex(sizeIdx);
  auto const heapSize = MemoryManager::sizeIndex2Size(sizeIdx);
  auto const ad = static_cast<TypeStructure*>(mem);

  assertx(heapSize % 16 == 0);
  memcpy16_inline(ad, this, heapSize);

  auto const aux = packSizeIndexAndAuxBits(sizeIdx, auxBits());
  ad->initHeader_16(HeaderKind::BespokeDict, OneReference, aux);
  ad->incRefFields();

  return ad;
}

void TypeStructure::Scan(
    const TypeStructure* tad, type_scan::Scanner& scanner) {
  scanner.scan(tad->m_alias);
  scanner.scan(tad->m_typevars);
  scanner.scan(tad->m_typevar_types);
}

void TypeStructure::incRefFields() {
  if (m_alias) m_alias->incRefCount();
  if (m_typevars) m_typevars->incRefCount();
  if (m_typevar_types) m_typevar_types->incRefCount();
}

void TypeStructure::decRefFields() {
  if (m_alias) m_alias->decRefAndRelease();
  if (m_typevars) m_typevars->decRefAndRelease();
  if (m_typevar_types) m_typevar_types->decRefAndRelease();
}

ssize_t TypeStructure::numFields(Kind kind) {
  // TODO: return different field counts for children structs
  return kNumFields;
}

bool TypeStructure::containsField(const StringData* k) const {
  auto compare = k->isStatic()
    ? [](StringData* a, const StringData* b) { return a == b; }
    : [](StringData* a, const StringData* b) { return a->same(b); };

#define X(Field, FieldString, KindOfType) \
  if (compare(s_##FieldString.get(), k)) return true;
TYPE_STRUCTURE_FIELDS
#undef X
  return false;
}

DataType TypeStructure::getKindOfField(const StringData* k) {
  if (k->isStatic()) {
#define X(Field, FieldString, KindOfType) \
    if (k == s_##FieldString.get()) return KindOfType;
  TYPE_STRUCTURE_FIELDS
#undef X
  } else {
#define X(Field, FieldString, KindOfType) \
    if (s_##FieldString.same(k)) return KindOfType;
  TYPE_STRUCTURE_FIELDS
#undef X
  }
  return KindOfUninit;
}

size_t TypeStructure::getFieldOffset(const StringData* key) {
  if (s_kind.same(key)) {
    return kindOffset();
  } else if (s_nullable.same(key)) {
    return bitFieldOffset();
  } else if (s_soft.same(key)) {
    return bitFieldOffset();
  } else if (s_like.same(key)) {
    return bitFieldOffset();
  } else if (s_opaque.same(key)) {
    return bitFieldOffset();
  } else if (s_optional_shape_field.same(key)) {
    return bitFieldOffset();
  } else if (s_alias.same(key)) {
    return offsetof(TypeStructure, m_alias);
  } else if (s_typevars.same(key)) {
    return offsetof(TypeStructure, m_typevars);
  } else if (s_typevar_types.same(key)) {
    return offsetof(TypeStructure, m_typevar_types);
  }
  always_assert(false);
}

uint8_t TypeStructure::getBitOffset(const StringData* key) {
  if (s_nullable.same(key)) return kNullableOffset;
  if (s_soft.same(key)) return kSoftOffset;
  if (s_like.same(key)) return kLikeOffset;
  if (s_opaque.same(key)) return kOpaqueOffset;
  if (s_optional_shape_field.same(key)) return kOptionalShapeFieldOffset;
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////////////
// ArrayData interface

namespace {

TypedValue make_tv_safe(int8_t val) {
  return make_tv<KindOfInt64>(val);
}
TypedValue make_tv_safe(bool val) {
  // to match the current behaviour of type structures
  return val ? make_tv<KindOfBoolean>(true) : make_tv<KindOfUninit>();
}
TypedValue make_tv_safe(StringData* val) {
  return val ? make_tv<KindOfString>(val) : make_tv<KindOfUninit>();
}
TypedValue make_tv_safe(ArrayData* val) {
  if (val == nullptr) return make_tv<KindOfUninit>();
  return val->isDictType() ? make_tv<KindOfDict>(val) : make_tv<KindOfVec>(val);
}

}

ArrayData* TypeStructure::EscalateToVanilla(
    const TypeStructure* tad, const char* reason) {
  return tad->escalateWithCapacity(numFields(tad->typeKind()), reason);
}

void TypeStructure::ConvertToUncounted(
    TypeStructure* tad, const MakeUncountedEnv& env) {
  if (tad->m_alias) MakeUncountedString(tad->m_alias, env);
  if (tad->m_typevars) MakeUncountedString(tad->m_typevars, env);
  if (tad->m_typevar_types) MakeUncountedArray(tad->m_typevar_types, env);
}

void TypeStructure::ReleaseUncounted(TypeStructure* tad) {
  if (tad->m_alias) DecRefUncountedString(tad->m_alias);
  if (tad->m_typevars) DecRefUncountedString(tad->m_typevars);
  if (tad->m_typevar_types) DecRefUncountedArray(tad->m_typevar_types);
}

void TypeStructure::Release(TypeStructure* tad) {
  tad->fixCountForRelease();
  assertx(tad->isRefCounted());
  assertx(tad->hasExactlyOneRef());
  tad->decRefFields();
  tl_heap->objFreeIndex(tad, sizeIndex(tad->typeKind()));
}

bool TypeStructure::IsVectorData(const TypeStructure*) {
  return false;
}

ArrayData* TypeStructure::Copy(const TypeStructure* tad) {
  return tad->copy();
}

TypedValue TypeStructure::NvGetInt(const TypeStructure*, int64_t) {
  return make_tv<KindOfUninit>();
}

TypedValue TypeStructure::NvGetStr(
    const TypeStructure* tad, const StringData* k) {
  auto compare = k->isStatic()
    ? [](StringData* a, const StringData* b) { return a == b; }
    : [](StringData* a, const StringData* b) { return a->same(b); };

#define X(Field, FieldString, KindOfType)                             \
  if (compare(s_##FieldString.get(), k)) {                            \
    return make_tv_safe(tad->Field());                                \
  }
TYPE_STRUCTURE_FIELDS
#undef X
  return make_tv<KindOfUninit>();
}

TypedValue TypeStructure::GetPosKey(const TypeStructure* tad, ssize_t pos) {
#define X(Field, FieldString, KindOfType)                             \
  if (pos-- == 0) {                                                   \
    return make_tv<KindOfString>(s_##FieldString.get());              \
  }
TYPE_STRUCTURE_FIELDS
#undef X
  return make_tv<KindOfUninit>();
}

TypedValue TypeStructure::GetPosVal(const TypeStructure* tad, ssize_t pos) {
#define X(Field, FieldString, KindOfType)                             \
  if (pos-- == 0) {                                                   \
    return make_tv_safe(tad->Field());                                \
  }
TYPE_STRUCTURE_FIELDS
#undef X
  return make_tv<KindOfUninit>();
}

bool TypeStructure::PosIsValid(const TypeStructure* tad, ssize_t pos) {
  return 0 <= pos && pos < numFields(tad->typeKind());
}

ssize_t TypeStructure::IterBegin(const TypeStructure*) {
  return 0;
}
ssize_t TypeStructure::IterLast(const TypeStructure* tad) {
  return numFields(tad->typeKind()) - 1;
}
ssize_t TypeStructure::IterEnd(const TypeStructure* tad) {
  return numFields(tad->typeKind());
}
ssize_t TypeStructure::IterAdvance(const TypeStructure* tad, ssize_t pos) {
  return pos < numFields(tad->typeKind()) ? pos + 1 : pos;
}
ssize_t TypeStructure::IterRewind(const TypeStructure* tad, ssize_t pos) {
  return pos > 0 ? pos - 1 : numFields(tad->typeKind());
}

arr_lval TypeStructure::LvalInt(TypeStructure* tad, int64_t k) {
  throwOOBArrayKeyException(k, tad);
}

// type structure shouldn't return a reference that's able to be mutated,
// so escalate
arr_lval TypeStructure::LvalStr(TypeStructure* tad, StringData* k) {
  auto const vad =
    tad->escalateWithCapacity(numFields(tad->typeKind()), __func__);
  assert(vad != nullptr);
  auto const result = vad->lval(k);
  assertx(result.arr == vad);
  return result;
}

tv_lval TypeStructure::ElemInt(tv_lval lval, int64_t k, bool throwOnMissing) {
  if (throwOnMissing) throwOOBArrayKeyException(k, lval.val().parr);
  return const_cast<TypedValue*>(&immutable_null_base);
}

// type structure shouldn't return a reference that's able to be mutated,
// so escalate
tv_lval TypeStructure::ElemStr(
    tv_lval lvalIn, StringData* k, bool throwOnMissing) {
  auto tadIn = As(lvalIn.val().parr);
  auto const lval = TypeStructure::LvalStr(tadIn, k);
  if (lval.arr != tadIn) {
    lvalIn.type() = dt_with_rc(lvalIn.type());
    lvalIn.val().parr = lval.arr;
    if (tadIn->decReleaseCheck()) Release(tadIn);
  }
  return lval;
}

ArrayData* TypeStructure::SetIntMove(
    TypeStructure* tad, int64_t k, TypedValue v) {
  auto const capacity = numFields(tad->typeKind()) + 1;
  auto const vad = tad->escalateWithCapacity(capacity, __func__);
  auto const res = VanillaDict::SetIntMove(vad, k, v);
  assertx(vad == res);
  if (tad->decReleaseCheck()) Release(tad);
  return res;
}

ArrayData* TypeStructure::SetStrMove(
    TypeStructure* tad, StringData* k, TypedValue v) {
  auto const capacity = numFields(tad->typeKind()) + 1;
  auto const vad = tad->escalateWithCapacity(capacity, __func__);
  auto const res = VanillaDict::SetStrMove(vad, k, v);
  if (tad->decReleaseCheck()) Release(tad);
  return res;
}

ArrayData* TypeStructure::RemoveIntMove(TypeStructure* tad, int64_t) {
  return tad;
}

ArrayData* TypeStructure::RemoveStrMove(
    TypeStructure* tad, const StringData* k) {
  if (!tad->containsField(k)) return tad;

  // type structure fields should not be removed, so escalate
  auto const capacity = numFields(tad->typeKind());
  auto const vad = tad->escalateWithCapacity(capacity, __func__);
  if (tad->decReleaseCheck()) Release(tad);
  return VanillaDict::RemoveStrMove(vad, k);
}

ArrayData* TypeStructure::AppendMove(TypeStructure* tad, TypedValue v) {
  auto const capacity = numFields(tad->typeKind()) + 1;
  auto const vad = tad->escalateWithCapacity(capacity, __func__);
  auto const res = VanillaDict::AppendMove(vad, v);
  assertx(vad == res);
  if (tad->decReleaseCheck()) Release(tad);
  return res;
}

// type structure fields should not be removed, so escalate
ArrayData* TypeStructure::PopMove(TypeStructure* tad, Variant& value) {
  auto const capacity = numFields(tad->typeKind());
  auto const vad = tad->escalateWithCapacity(capacity, __func__);
  if (tad->decReleaseCheck()) Release(tad);
  return VanillaDict::PopMove(vad, value);
}

ArrayData* TypeStructure::PreSort(TypeStructure* tad, SortFunction sf) {
  auto const capacity = numFields(tad->typeKind());
  return tad->escalateWithCapacity(capacity, sortFunctionName(sf));
}

ArrayData* TypeStructure::PostSort(TypeStructure*, ArrayData* vad) {
  return vad;
}

ArrayData* TypeStructure::SetLegacyArray(
    TypeStructure* tadIn, bool copy, bool legacy) {
  auto const tad = copy ? tadIn->copy() : tadIn;
  tad->setLegacyArrayInPlace(legacy);
  return tad;
}

//////////////////////////////////////////////////////////////////////////////
// TypeStructureLayout

using namespace jit;

const LayoutFunctions* typeStructureVtable() {
  static auto const result = fromArray<TypeStructure>();
  return &result;
}

TypeStructureLayout::TypeStructureLayout()
  : ConcreteLayout(
      LayoutIndex{uint16_t(kTypeStructureLayoutByte << 8)},
      "TypeStructure",
      {AbstractLayout::GetBespokeTopIndex()},
      typeStructureVtable()
    )
{}

// TODO: currently these are placeholder values, change them

ArrayLayout TypeStructureLayout::appendType(Type val) const {
  return ArrayLayout::Vanilla();
}
ArrayLayout TypeStructureLayout::removeType(Type key) const {
  return ArrayLayout::Vanilla();
}
ArrayLayout TypeStructureLayout::setType(Type key, Type val) const {
  return ArrayLayout::Vanilla();
}

std::pair<Type, bool> TypeStructureLayout::elemType(Type key) const {
  if (key <= TInt) return {TBottom, false};
  if (key.hasConstVal(TStr)) {
    auto const dt = bespoke::TypeStructure::getKindOfField(key.strVal());
    if (dt == KindOfBoolean) return {TBool, false};
    // 'kind' field should always be present
    if (dt == KindOfInt64) return {TInt, true};
    if (dt == KindOfVec) return {TVec, false};
    if (dt == KindOfDict) return {TDict, false};
    if (isStringType(dt)) return {TStr, false};
  }

  return {TInitCell, false};
}
std::pair<Type, bool> TypeStructureLayout::firstLastType(
    bool isFirst, bool isKey) const {
  return {TBottom, false};
}
Type TypeStructureLayout::iterPosType(Type pos, bool isKey) const {
  return TBottom;
}

}
