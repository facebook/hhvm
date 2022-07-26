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
  switch (kind) {
#define X(Field, Type) \
    case Kind::T_##Field: return sizeof(Type);
TYPE_STRUCTURE_KINDS
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default: return 0;
  }
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

bool TypeStructure::isBespokeTypeStructure(const ArrayData* ad) {
  return !ad->isVanilla() && asBespoke(ad)->layoutIndex() == GetLayoutIndex();
}

void TypeStructure::InitializeLayouts() {
  new TypeStructureLayout();
}

template <bool Static>
TypeStructure* TypeStructure::MakeReserve(bool legacy, Kind kind) {
  auto const index = sizeIndex(kind);
  auto const size = MemoryManager::sizeIndex2Size(index);
  auto const alloc = [&]{
    if (!Static) return tl_heap->objMallocIndex(index);
    return RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size);
  }();

  auto const tad = static_cast<TypeStructure*>(alloc);
  auto const aux = packSizeIndexAndAuxBits(
    index, legacy ? ArrayData::kLegacyArray : 0);
  tad->initHeader_16(HeaderKind::BespokeDict,
    Static ? StaticValue : OneReference, aux);

  // need to initialize this to actual size in MakeFromVanilla
  tad->m_size = 0;

  tad->m_layout_index = GetLayoutIndex();
  tad->m_kind = uint8_t(kind);
  tad->m_extra_hi8 = TypeStructure::kFieldsByte;
  tad->m_extra_lo8 = 0;
  tad->m_alias = nullptr;
  tad->m_typevars = nullptr;
  tad->m_typevar_types = nullptr;

  switch (kind) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    tad->m_extra_hi8 = Type::kFieldsByte; \
    Type::initializeFields(reinterpret_cast<Type*>(tad)); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }

  return tad;
}

template <typename Type>
size_t setFields(Type* ts, ArrayData* ad) {
  auto fields = 0;
  VanillaDict::IterateKV(VanillaDict::as(ad), [&](auto k, auto v) {
    assertx(tvIsString(k));
    if (Type::setField(ts, val(k).pstr, v)) {
      fields++;
      tvIncRefGen(v);
    }
  });
  return fields;
}

void TypeStructure::setBitField(TypedValue v, BitFieldOffsets offset) {
  assertx(tvIsBool(v));
  uint8_t val = v.val().num;
  clearBitField(offset);
  m_extra_lo8 |= val << offset;
}

// caller must incref TypedValue
bool TypeStructure::setField(TypeStructure* tad, StringData* k, TypedValue v) {
  if (s_kind.same(k)) {
    assertx(tvIsInt(v));
    tad->m_kind = safe_cast<uint8_t>(val(v).num);
  } else if (s_nullable.same(k)) {
    tad->setBitField(v, kNullableOffset);
  } else if (s_soft.same(k)) {
    tad->setBitField(v, kSoftOffset);
  } else if (s_like.same(k)) {
    tad->setBitField(v, kLikeOffset);
  } else if (s_opaque.same(k)) {
    tad->setBitField(v, kOpaqueOffset);
  } else if (s_optional_shape_field.same(k)) {
    tad->setBitField(v, kOptionalShapeFieldOffset);
  } else if (s_alias.same(k)) {
    setStringField(tad->m_alias, v);
  } else if (s_typevars.same(k)) {
    setStringField(tad->m_typevars, v);
  } else if (s_typevar_types.same(k)) {
    setArrayField(tad->m_typevar_types, v);
  } else {
    return false;
  }
  return true;
}

TypeStructure* TypeStructure::MakeFromVanilla(
    ArrayData* ad, bool forceNonStatic) {
  if (!ad->isVanillaDict() || !ad->exists(s_kind)) return nullptr;
  auto const kind =
    static_cast<Kind>(Variant::wrap(ad->get(s_kind)).toInt64Val());

  auto const result = (!forceNonStatic && ad->isStatic())
    ? MakeReserve<true>(ad->isLegacyArray(), kind)
    : MakeReserve<false>(ad->isLegacyArray(), kind);

  auto size = setFields<TypeStructure>(result, ad);

  switch (kind) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    size += setFields<Type>(reinterpret_cast<Type*>(result), ad); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }

  assertx(ad->size() == size);
  result->m_size = size;

  assertx(result->checkInvariants());
  return result;
}

TypeStructure* TypeStructure::MakeFromVanillaStatic(ArrayData* ad) {
  // must create a non-static bespoke array first so that GetScalarArray won't
  // short-circuit on isStatic and will dedup properly
  ArrayData* ts = MakeFromVanilla(ad, true);
  if (ts) {
    GetScalarArray(&ts);
    return As(ts);
  }
  return nullptr;
}

bool TypeStructure::checkInvariants() const {
  assertx(0 <= kind() && kind() <= HPHP::TypeStructure::kMaxResolvedKind);
  assertx(m_alias == nullptr || m_alias->kindIsValid());
  assertx(m_typevars == nullptr || m_typevars->kindIsValid());
  assertx(m_typevar_types == nullptr || m_typevar_types->kindIsValid());

  switch (typeKind()) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    return reinterpret_cast<const Type*>(this)->checkInvariants();
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
#define X(Name, Type) \
  case Kind::T_##Name: \
    assertx(fieldsByte() == TypeStructure::kFieldsByte); \
    return true;
TYPE_STRUCTURE_KINDS
#undef X
    default:
      return false;
  }
}

bool TypeStructure::isValidTypeStructure(const ArrayData* ad) {
  if (!ad->isVanillaDict() || !ad->exists(s_kind)) return false;
  auto const kind =
    static_cast<Kind>(Variant::wrap(ad->get(s_kind)).toInt64Val());

  // check that the key exists and that the type matches
  auto fields = 0;
#define X(Field, FieldString, KindOfType)                             \
  if (ad->exists(s_##FieldString) &&                                  \
      equivDataTypes(ad->get(s_##FieldString).type(), KindOfType)) {  \
    fields++;                                                         \
  }
TYPE_STRUCTURE_FIELDS
#undef X

  switch (kind) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    fields += Type::countFields(ad); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
#define X(Name, Type) \
  case Kind::T_##Name: \
    break;
TYPE_STRUCTURE_KINDS
#undef X
  default: \
    return false;
  }

  // all elements in ad must exist in type structure
  return fields == ad->size();
}

ArrayData* TypeStructure::escalateWithCapacity(
    size_t capacity, const char* reason) const {
  assertx(capacity >= size());
  logEscalateToVanilla(this, reason);

  auto ad = VanillaDict::MakeReserveDict(capacity);
  ad->setLegacyArrayInPlace(isLegacyArray());

  // move base TypeStructure fields
#define X(Field, FieldString, KindOfType) {                           \
  auto const key = s_##FieldString.get();                             \
  auto const tv = TypeStructure::tsNvGetStr(this, key);               \
  moveFieldToVanilla(ad, key, tv);                                    \
}
TYPE_STRUCTURE_FIELDS
#undef X

  // move any remaining children fields
  switch (typeKind()) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    reinterpret_cast<const Type*>(this)->moveFieldsToVanilla(ad); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default: break;
  }

  assertx(isValidTypeStructure(ad));
  return ad;
}

template <bool Static>
TypeStructure* TypeStructure::copy() const {
  auto const sizeIdx = sizeIndex(typeKind());
  auto const heapSize = MemoryManager::sizeIndex2Size(sizeIdx);
  auto const mem = [&]{
    if (!Static) return tl_heap->objMallocIndex(sizeIdx);
    return RO::EvalLowStaticArrays ? low_malloc(heapSize) : uncounted_malloc(heapSize);
  }();
  auto const ad = static_cast<TypeStructure*>(mem);

  assertx(heapSize % 16 == 0);
  memcpy16_inline(ad, this, heapSize);

  auto const aux = packSizeIndexAndAuxBits(sizeIdx, auxBits());
  ad->initHeader_16(HeaderKind::BespokeDict, Static ? StaticValue : OneReference, aux);
  if (!Static) ad->incRefFields();

  return ad;
}

// explicitly declare to use in runtime tests
template TypeStructure* TypeStructure::copy<true>() const;
template TypeStructure* TypeStructure::copy<false>() const;

void TypeStructure::Scan(
    const TypeStructure* tad, type_scan::Scanner& scanner) {
  scanner.scan(tad->m_alias);
  scanner.scan(tad->m_typevars);
  scanner.scan(tad->m_typevar_types);

  switch (tad->typeKind()) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    Type::scan(reinterpret_cast<const Type*>(tad), scanner); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }
}

void TypeStructure::incRefFields() {
  if (m_alias) m_alias->incRefCount();
  if (m_typevars) m_typevars->incRefCount();
  if (m_typevar_types) m_typevar_types->incRefCount();

  switch (typeKind()) {
#define X(Name, Type) \
    case Kind::T_##Name: \
      reinterpret_cast<Type*>(this)->incRefFields(); \
      break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }
}

void TypeStructure::decRefFields() {
  if (m_alias) m_alias->decRefAndRelease();
  if (m_typevars) m_typevars->decRefAndRelease();
  if (m_typevar_types) m_typevar_types->decRefAndRelease();

  switch (typeKind()) {
#define X(Name, Type) \
    case Kind::T_##Name: \
      reinterpret_cast<Type*>(this)->decRefFields(); \
      break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }
}

bool TypeStructure::containsField(const StringData* k) const {
#define X(Field, FieldString, KindOfType) \
  if (s_##FieldString.same(k)) return true;
TYPE_STRUCTURE_FIELDS
#undef X

  switch (typeKind()) {
#define X(Name, Type) \
    case Kind::T_##Name: \
      return reinterpret_cast<const Type*>(this)->containsField(k); \
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      return false;
  }
}

std::pair<DataType, uint8_t> TypeStructure::getFieldPair(const StringData* k) {
  // fields in base TypeStructure always exist and are represented by offset 0
#define X(Field, FieldString, KindOfType) \
  if (s_##FieldString.same(k)) return {KindOfType, 0};
  TYPE_STRUCTURE_FIELDS
#undef X

#define X(Field, Type, KindOfType, Struct, FieldsByteOffset) \
  if (s_##Field.same(k)) return {KindOfType, FieldsByteOffset};
  TYPE_STRUCTURE_CHILDREN_FIELDS
#undef X

  return {KindOfUninit, 0};
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

#define X(Field, Type, KindOfType, Struct, ...) \
  if (s_##Field.same(key)) return Struct::getFieldOffset(key);
    TYPE_STRUCTURE_CHILDREN_FIELDS
#undef X
  always_assert(false);
}

uint8_t TypeStructure::getBooleanBitOffset(const StringData* key) {
  if (s_nullable.same(key)) return kNullableOffset;
  if (s_soft.same(key)) return kSoftOffset;
  if (s_like.same(key)) return kLikeOffset;
  if (s_opaque.same(key)) return kOpaqueOffset;
  if (s_optional_shape_field.same(key)) return kOptionalShapeFieldOffset;
  // otherwise the boolean field does not require an offset
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// ArrayData interface

namespace {

TypedValue make_tv_safe(uint8_t val) {
  return make_tv<KindOfInt64>(val);
}
TypedValue make_tv_safe(bool val) {
  // to match the current behaviour of type structures
  return val ? make_tv<KindOfBoolean>(true) : make_tv<KindOfUninit>();
}
TypedValue make_tv_safe(StringData* val) {
  if (val == nullptr) return make_tv<KindOfUninit>();
  return val->isStatic()
    ? make_tv<KindOfPersistentString>(val)
    : make_tv<KindOfString>(val);
}
TypedValue make_tv_safe(ArrayData* val) {
  if (val == nullptr) return make_tv<KindOfUninit>();
  if (val->isDictType()) {
    return val->isStatic()
      ? make_tv<KindOfPersistentDict>(val)
      : make_tv<KindOfDict>(val);
  }
  assertx(val->isVecType());
  return val->isStatic()
    ? make_tv<KindOfPersistentVec>(val)
    : make_tv<KindOfVec>(val);
}

}

ArrayData* TypeStructure::EscalateToVanilla(
    const TypeStructure* tad, const char* reason) {
  return tad->escalateWithCapacity(tad->size(), reason);
}

void TypeStructure::ConvertToUncounted(
    TypeStructure* tad, const MakeUncountedEnv& env) {
  if (tad->m_alias) MakeUncountedString(tad->m_alias, env);
  if (tad->m_typevars) MakeUncountedString(tad->m_typevars, env);
  if (tad->m_typevar_types) MakeUncountedArray(tad->m_typevar_types, env);

  switch (tad->typeKind()) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    Type::convertToUncounted(reinterpret_cast<Type*>(tad), env); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }
}

void TypeStructure::ReleaseUncounted(TypeStructure* tad) {
  if (tad->m_alias) DecRefUncountedString(tad->m_alias);
  if (tad->m_typevars) DecRefUncountedString(tad->m_typevars);
  if (tad->m_typevar_types) DecRefUncountedArray(tad->m_typevar_types);

  switch (tad->typeKind()) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    Type::releaseUncounted(reinterpret_cast<Type*>(tad)); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }
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

void TypeStructure::OnSetEvalScalar(TypeStructure* tad) {
  if (tad->m_alias && !tad->m_alias->isStatic()) {
    auto field = tad->m_alias;
    tad->m_alias = makeStaticString(field);
    decRefStr(field);
  }
  if (tad->m_typevars && !tad->m_typevars->isStatic()) {
    auto field = tad->m_typevars;
    tad->m_typevars = makeStaticString(field);
    decRefStr(field);
  }
  if (tad->m_typevar_types && !tad->m_typevar_types->isStatic()) {
    auto arr = Array(tad->m_typevar_types);
    arr.setEvalScalar();
    decRefArr(tad->m_typevar_types);
    tad->m_typevar_types = arr.get();
  }

  switch (tad->typeKind()) {
#define X(Name, Type) \
  case Kind::T_##Name: \
    Type::onSetEvalScalar(reinterpret_cast<Type*>(tad)); \
    break;
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default:
      break;
  }
}

ArrayData* TypeStructure::Copy(const TypeStructure* tad) {
  return tad->copy<false>();
}

ArrayData* TypeStructure::CopyStatic(const TypeStructure* tad) {
  return tad->copy<true>();
}

TypedValue TypeStructure::NvGetInt(const TypeStructure*, int64_t) {
  return make_tv<KindOfUninit>();
}

TypedValue TypeStructure::NvGetStr(
    const TypeStructure* tad, const StringData* k) {
  auto const tv = TypeStructure::tsNvGetStr(tad, k);
  if (tv.is_init()) return tv;

  // at this point, either k exists in a children struct or does not exist at all
  switch (tad->typeKind()) {
#define X(Name, Type) \
  case Kind::T_##Name: return Type::tsNvGetStr(reinterpret_cast<const Type*>(tad), k);
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default: break;
  }
  return make_tv<KindOfUninit>();
}

TypedValue TypeStructure::tsNvGetStr(
    const TypeStructure* tad, const StringData* k) {
#define X(Field, FieldString, KindOfType)                             \
  if (s_##FieldString.same(k)) {                                      \
    return make_tv_safe(tad->Field());                                \
  }
TYPE_STRUCTURE_FIELDS
#undef X
  return make_tv<KindOfUninit>();
}

TypedValue TypeStructure::GetPosKey(const TypeStructure* tad, ssize_t pos) {
#define X(Field, FieldString, KindOfType)                             \
  if ((isIntType(KindOfType) || tad->Field()) && pos-- == 0) {        \
    return make_tv<KindOfPersistentString>(s_##FieldString.get());    \
  }
TYPE_STRUCTURE_FIELDS
#undef X

  switch (tad->typeKind()) {
#define X(Name, Type) \
    case Kind::T_##Name: \
      return Type::tsGetPosKey(reinterpret_cast<const Type*>(tad), pos);
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default: break;
  }

  return make_tv<KindOfUninit>();
}

TypedValue TypeStructure::GetPosVal(const TypeStructure* tad, ssize_t pos) {
#define X(Field, FieldString, KindOfType)                             \
  if ((isIntType(KindOfType) || tad->Field()) && pos-- == 0) {        \
    return make_tv_safe(tad->Field());                                \
  }
TYPE_STRUCTURE_FIELDS
#undef X

  switch (tad->typeKind()) {
#define X(Name, Type) \
    case Kind::T_##Name: \
      return Type::tsGetPosVal(reinterpret_cast<const Type*>(tad), pos);
TYPE_STRUCTURE_CHILDREN_KINDS
#undef X
    default: break;
  }

  return make_tv<KindOfUninit>();
}

bool TypeStructure::PosIsValid(const TypeStructure* tad, ssize_t pos) {
  return 0 <= pos && pos < tad->size();
}

ssize_t TypeStructure::IterBegin(const TypeStructure*) {
  return 0;
}
ssize_t TypeStructure::IterLast(const TypeStructure* tad) {
  return tad->size() - 1;
}
ssize_t TypeStructure::IterEnd(const TypeStructure* tad) {
  return tad->size();
}
ssize_t TypeStructure::IterAdvance(const TypeStructure* tad, ssize_t pos) {
  return pos < tad->size() ? pos + 1 : pos;
}
ssize_t TypeStructure::IterRewind(const TypeStructure* tad, ssize_t pos) {
  return pos > 0 ? pos - 1 : tad->size();
}

arr_lval TypeStructure::LvalInt(TypeStructure* tad, int64_t k) {
  throwOOBArrayKeyException(k, tad);
}

// type structure shouldn't return a reference that's able to be mutated,
// so escalate
arr_lval TypeStructure::LvalStr(TypeStructure* tad, StringData* k) {
  auto const vad =
    tad->escalateWithCapacity(tad->size(), __func__);
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
  auto const capacity = tad->size() + 1;
  auto const vad = tad->escalateWithCapacity(capacity, __func__);
  auto const res = VanillaDict::SetIntMove(vad, k, v);
  assertx(vad == res);
  if (tad->decReleaseCheck()) Release(tad);
  return res;
}

ArrayData* TypeStructure::SetStrMove(
    TypeStructure* tad, StringData* k, TypedValue v) {
  auto const vad = tad->escalateWithCapacity(tad->size(), __func__);
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
  auto const vad = tad->escalateWithCapacity(tad->size(), __func__);
  if (tad->decReleaseCheck()) Release(tad);
  return VanillaDict::RemoveStrMove(vad, k);
}

ArrayData* TypeStructure::AppendMove(TypeStructure* tad, TypedValue v) {
  auto const vad = tad->escalateWithCapacity(tad->size(), __func__);
  auto const res = VanillaDict::AppendMove(vad, v);
  assertx(vad == res);
  if (tad->decReleaseCheck()) Release(tad);
  return res;
}

// type structure fields should not be removed, so escalate
ArrayData* TypeStructure::PopMove(TypeStructure* tad, Variant& value) {
  auto const vad = tad->escalateWithCapacity(tad->size(), __func__);
  if (tad->decReleaseCheck()) Release(tad);
  return VanillaDict::PopMove(vad, value);
}

ArrayData* TypeStructure::PreSort(TypeStructure* tad, SortFunction sf) {
  return tad->escalateWithCapacity(tad->size(), sortFunctionName(sf));
}

ArrayData* TypeStructure::PostSort(TypeStructure*, ArrayData* vad) {
  return vad;
}

ArrayData* TypeStructure::SetLegacyArray(
    TypeStructure* tadIn, bool copy, bool legacy) {
  auto const tad = copy ? tadIn->copy<false>() : tadIn;
  tad->setLegacyArrayInPlace(legacy);
  return tad;
}
//////////////////////////////////////////////////////////////////////////////
// specific TypeStructures

namespace {

template<class T> void tsMakeUncounted(T t, const MakeUncountedEnv& env) {}
template<>
void tsMakeUncounted<StringData*>(StringData* field, const MakeUncountedEnv& env) {
  if (field) MakeUncountedString(field, env);
}
template<>
void tsMakeUncounted<ArrayData*>(ArrayData* field, const MakeUncountedEnv& env) {
  if (field) MakeUncountedArray(field, env);
}

template<class T> void tsDecRefUncounted(T t) {}
template<>
void tsDecRefUncounted<StringData*>(StringData* field) {
  if (field) DecRefUncountedString(field);
}
template<>
void tsDecRefUncounted<ArrayData*>(ArrayData* field) {
  if (field) DecRefUncountedArray(field);
}

template<class T> void initializeField(T t) {}
template<>
void initializeField<bool&>(bool& field) {
  field = 0;
}
template<>
void initializeField<StringData*&>(StringData*& field) {
  field = nullptr;
}
template<>
void initializeField<ArrayData*&>(ArrayData*& field) {
  field = nullptr;
}

template<class T> void scanField(T t, type_scan::Scanner& scanner) {}
template<>
void scanField<StringData*>(StringData* field, type_scan::Scanner& scanner) {
  scanner.scan(field);
}
template<>
void scanField<ArrayData*>(ArrayData* field, type_scan::Scanner& scanner) {
  scanner.scan(field);
}

template<class T> void incRefField(T t) {}
template<>
void incRefField<StringData*>(StringData* field) {
  if (field) field->incRefCount();
}
template<>
void incRefField<ArrayData*>(ArrayData* field) {
  if (field) field->incRefCount();
}

template<class T> void decRefField(T t) {}
template<>
void decRefField<StringData*>(StringData* field) {
  if (field) field->decRefCount();
}
template<>
void decRefField<ArrayData*>(ArrayData* field) {
  if (field) field->decRefCount();
}

template<class T> void setEvalScalar(T t) {}
template<>
void setEvalScalar(StringData*& field) {
  if (field && !field->isStatic()) {
    auto staticField = field;
    field = makeStaticString(staticField);
    decRefStr(field);
  }
}
template<>
void setEvalScalar(ArrayData*& field) {
  if (field && !field->isStatic()) {
    auto arr = Array(field);
    arr.setEvalScalar();
    decRefArr(field);
    field = arr.get();
  }
}

}

#define MAKE_TV_FIELD(Field, ...)                                           \
  if (s_##Field.same(k)) return make_tv_safe(tad->m_##Field);

#define GET_POS_KEY(Field, ...)                                             \
  if (tad->m_##Field && pos-- == 0) {                                       \
    return make_tv<KindOfPersistentString>(s_##Field.get());                \
  }

#define GET_POS_VAL(Field, ...)                                             \
  if (tad->m_##Field && pos-- == 0) {                                       \
    return make_tv_safe(tad->m_##Field);                                    \
  }

#define MOVE_FIELD_TO_VANILLA(Field, Type, KindOfType, ...) {               \
  auto const tv = make_tv_safe(m_##Field);                                  \
  moveFieldToVanilla(ad, s_##Field.get(), tv);                              \
}
#define CONTAINS_FIELD(Field, Type, ...)                                    \
  if (s_##Field.same(k)) return true;

#define CONVERT_TO_UNCOUNTED(Field, Type, ...)                              \
  tsMakeUncounted<Type>(tad->m_##Field, env);

#define RELEASE_UNCOUNTED(Field, Type, ...)                                 \
  tsDecRefUncounted<Type>(tad->m_##Field);

#define INITIALIZE_FIELD(Field, Type, ...) initializeField<Type&>(tad->m_##Field);
#define SCAN_FIELD(Field, Type, ...) scanField<Type>(tad->m_##Field, scanner);
#define INC_REF_FIELD(Field, Type, ...) incRefField<Type>(m_##Field);
#define DEC_REF_FIELD(Field, Type, ...) decRefField<Type>(m_##Field);

#define GET_FIELD_OFFSET(Field, Type, KindOfType, Struct, ...)              \
  if (s_##Field.same(k)) return offsetof(Struct, m_##Field);

#define COUNT_FIELDS(Field, Type, KindOfType, ...)              \
  if (ad->exists(s_##Field) &&                                  \
      equivDataTypes(ad->get(s_##Field).type(), KindOfType)) {  \
    fields++;                                                         \
  }

#define SET_EVAL_SCALAR(Field, Type, KindOfType, ...) \
  setEvalScalar<Type&>(tad->m_##Field);

#define O(ChildStruct, FieldsMacro)                                         \
TypedValue ChildStruct::tsNvGetStr(const ChildStruct* tad, const StringData* k) { \
  FieldsMacro(MAKE_TV_FIELD)                                                \
  return make_tv<KindOfUninit>();                                           \
}                                                                           \
TypedValue ChildStruct::tsGetPosKey(const ChildStruct* tad, ssize_t pos) {  \
  FieldsMacro(GET_POS_KEY)                                                  \
  return make_tv<KindOfUninit>();                                           \
}                                                                           \
TypedValue ChildStruct::tsGetPosVal(const ChildStruct* tad, ssize_t pos) {  \
  FieldsMacro(GET_POS_VAL)                                                  \
  return make_tv<KindOfUninit>();                                           \
}                                                                           \
void ChildStruct::moveFieldsToVanilla(ArrayData* ad) const {                \
  FieldsMacro(MOVE_FIELD_TO_VANILLA)                                        \
}                                                                           \
void ChildStruct::convertToUncounted(ChildStruct* tad, const MakeUncountedEnv& env) { \
  FieldsMacro(CONVERT_TO_UNCOUNTED)                                         \
}                                                                           \
void ChildStruct::releaseUncounted(ChildStruct* tad) {                      \
  FieldsMacro(RELEASE_UNCOUNTED)                                            \
}                                                                           \
void ChildStruct::initializeFields(ChildStruct* tad) {                      \
  FieldsMacro(INITIALIZE_FIELD)                                             \
}                                                                           \
void ChildStruct::scan(const ChildStruct* tad, type_scan::Scanner& scanner) { \
  FieldsMacro(SCAN_FIELD)                                                   \
}                                                                           \
void ChildStruct::incRefFields() {                                          \
  FieldsMacro(INC_REF_FIELD)                                                \
}                                                                           \
void ChildStruct::decRefFields() {                                          \
  FieldsMacro(DEC_REF_FIELD)                                                \
}                                                                           \
bool ChildStruct::containsField(const StringData* k) const {                \
  FieldsMacro(CONTAINS_FIELD)                                               \
  return false;                                                             \
}                                                                           \
size_t ChildStruct::getFieldOffset(const StringData* k) {                   \
  FieldsMacro(GET_FIELD_OFFSET)                                             \
  return -1;                                                                \
}                                                                           \
int ChildStruct::countFields(const ArrayData* ad) {                         \
  auto fields = 0;                                                          \
  FieldsMacro(COUNT_FIELDS)                                                 \
  return fields;                                                            \
}                                                                           \
void ChildStruct::onSetEvalScalar(ChildStruct* tad) {                       \
  FieldsMacro(SET_EVAL_SCALAR)                                              \
}

O(TSShape, TSSHAPE_FIELDS)
O(TSTuple, TSTUPLE_FIELDS)
O(TSFun, TSFUN_FIELDS)
O(TSTypevar, TSTYPEVAR_FIELDS)
O(TSWithClassishTypes, TSCLASSISH_FIELDS)
O(TSWithGenericTypes, TSGENERIC_FIELDS)

#undef O

#undef MAKE_TV_FIELD
#undef GET_POS_KEY
#undef GET_POS_VAL
#undef MOVE_FIELD_TO_VANILLA
#undef CONTAINS_FIELD
#undef CONVERT_TO_UNCOUNTED
#undef RELEASE_UNCOUNTED
#undef INITIALIZE_FIELD
#undef SCAN_FIELD
#undef INC_REF_FIELD
#undef DEC_REF_FIELD
#undef SET_EVAL_SCALAR

bool TSShape::checkInvariants() const {
  assertx(typeKind() == Kind::T_shape);
  assertx(fieldsByte() == TSShape::kFieldsByte);
  assertx(m_fields == nullptr || m_fields->kindIsValid());
  return true;
}
bool TSTuple::checkInvariants() const {
  assertx(typeKind() == Kind::T_tuple);
  assertx(fieldsByte() == TSTuple::kFieldsByte);
  assertx(m_elem_types && m_elem_types->kindIsValid());
  return true;
}
bool TSFun::checkInvariants() const {
  assertx(typeKind() == Kind::T_fun);
  assertx(fieldsByte() == TSFun::kFieldsByte);
  assertx(m_return_type && m_return_type->kindIsValid());
  assertx(m_param_types == nullptr || m_param_types->kindIsValid());
  assertx(m_variadic_type == nullptr || m_variadic_type->kindIsValid());
  return true;
}
bool TSTypevar::checkInvariants() const {
  assertx(typeKind() == Kind::T_typevar);
  assertx(fieldsByte() == TSTypevar::kFieldsByte);
  assertx(m_name && m_name->kindIsValid());
  return true;
}
bool TSWithClassishTypes::checkInvariants() const {
  assertx(fieldsByte() == TSWithClassishTypes::kFieldsByte);
  assertx(m_generic_types == nullptr || m_generic_types->kindIsValid());
  assertx(m_classname && m_classname->kindIsValid());
  return true;
}
bool TSWithGenericTypes::checkInvariants() const {
  assertx(fieldsByte() == TSWithGenericTypes::kFieldsByte);
  assertx(m_generic_types == nullptr || m_generic_types->kindIsValid());
  return true;
}

bool TSShape::setField(TSShape* tad, StringData* k, TypedValue v) {
  if (s_fields.same(k)) {
    setArrayField(tad->m_fields, v);
  } else if (s_allows_unknown_fields.same(k)) {
    assertx(tvIsBool(v));
    tad->m_allows_unknown_fields = val(v).num;
  } else {
    return false;
  }
  return true;
}
bool TSTuple::setField(TSTuple* tad, StringData* k, TypedValue v) {
  if (s_elem_types.same(k)) {
    setArrayField(tad->m_elem_types, v);
    return true;
  }
  return false;
}
bool TSFun::setField(TSFun* tad, StringData* k, TypedValue v) {
  if (s_param_types.same(k)) {
    setArrayField(tad->m_param_types, v);
  } else if (s_return_type.same(k)) {
    setArrayField(tad->m_return_type, v);
  } else if (s_variadic_type.same(k)) {
    setArrayField(tad->m_variadic_type, v);
  } else {
    return false;
  }
  return true;
}
bool TSTypevar::setField(TSTypevar* tad, StringData* k, TypedValue v) {
  if (s_name.same(k)) {
    setStringField(tad->m_name, v);
    return true;
  }
  return false;
}
bool TSWithClassishTypes::setField(TSWithClassishTypes* tad, StringData* k, TypedValue v) {
  if (s_generic_types.same(k)) {
    setArrayField(tad->m_generic_types, v);
  } else if (s_classname.same(k)) {
    setStringField(tad->m_classname, v);
  } else if (s_exact.same(k)) {
    assertx(tvIsBool(v));
    tad->m_exact = val(v).num;
  } else {
    return false;
  }
  return true;
}
bool TSWithGenericTypes::setField(TSWithGenericTypes* tad, StringData* k, TypedValue v) {
  if (s_generic_types.same(k)) {
    setArrayField(tad->m_generic_types, v);
    return true;
  }
  return false;
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
    auto const dt = bespoke::TypeStructure::getFieldPair(key.strVal()).first;
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
