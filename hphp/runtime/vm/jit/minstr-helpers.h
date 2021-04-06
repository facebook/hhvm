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
#pragma once

#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/member-operations.h"

#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"

namespace HPHP { namespace jit { namespace MInstrHelpers {

//////////////////////////////////////////////////////////////////////

template<MOpMode mode>
tv_lval baseGImpl(TypedValue key) {
  auto const name = prepareKey(key);
  SCOPE_EXIT { decRefStr(name); };

  auto const env = g_context->m_globalNVTable;
  assertx(env != nullptr);

  auto base = env->lookup(name);
  if (base == nullptr) {
    if (mode == MOpMode::Warn) {
      SystemLib::throwOutOfBoundsExceptionObject(
        folly::sformat("Undefined index: {}", name)
      );
    }
    if (mode == MOpMode::Define) {
      auto tv = make_tv<KindOfNull>();
      env->set(name, &tv);
      base = env->lookup(name);
    } else {
      return const_cast<TypedValue*>(&immutable_null_base);
    }
  }
  return base;
}

#define BASE_G_HELPER_TABLE(m)                  \
  /* name    mode                  */           \
  m(baseG,   MOpMode::None)                     \
  m(baseGW,  MOpMode::Warn)                     \
  m(baseGD,  MOpMode::Define)                   \

#define X(nm, mode)                             \
inline tv_lval nm(TypedValue key) {             \
  return baseGImpl<mode>(key);                  \
}
BASE_G_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define PROP_HELPER_TABLE(m)                        \
  /* name      mode                  keyType     */ \
  m(propC,     MOpMode::None,       KeyType::Any)   \
  m(propCS,    MOpMode::None,       KeyType::Str)   \
  m(propCU,    MOpMode::Unset,      KeyType::Any)   \
  m(propCUS,   MOpMode::Unset,      KeyType::Str)   \
  m(propCW,    MOpMode::Warn,       KeyType::Any)   \
  m(propCWS,   MOpMode::Warn,       KeyType::Str)

#define X(nm, mode, kt)                                       \
inline tv_lval nm(Class* ctx, tv_lval base, key_type<kt> key, \
                  TypedValue& tvRef, ReadOnlyOp op) {         \
  return Prop<mode,kt>(tvRef, ctx, base, key, op);            \
}
PROP_HELPER_TABLE(X)
#undef X

#define PROPD_HELPER_TABLE(m)                       \
  /* name      keyType     */                       \
  m(propCD,    KeyType::Any)                        \
  m(propCDS,   KeyType::Str)

#define X(nm, kt)                                             \
inline tv_lval nm(Class* ctx, tv_lval base, key_type<kt> key, \
                  TypedValue& tvRef, ReadOnlyOp op) {         \
  return Prop<MOpMode::Define,kt>(tvRef, ctx, base, key, op); \
}
PROPD_HELPER_TABLE(X)
#undef X

#define PROP_OBJ_HELPER_TABLE(m)                     \
  /* name      mode                  keyType     */  \
  m(propCO,    MOpMode::None,       KeyType::Any)    \
  m(propCOS,   MOpMode::None,       KeyType::Str)    \
  m(propCUO,   MOpMode::Unset,      KeyType::Any)    \
  m(propCUOS,  MOpMode::Unset,      KeyType::Str)    \
  m(propCWO,   MOpMode::Warn,       KeyType::Any)    \
  m(propCWOS,  MOpMode::Warn,       KeyType::Str)    \

#define X(nm, mode, kt)                                           \
inline tv_lval nm(Class* ctx, ObjectData* base, key_type<kt> key, \
                  TypedValue& tvRef, ReadOnlyOp op) {             \
  return PropObj<mode,kt>(tvRef, ctx, base, key, op);             \
}
PROP_OBJ_HELPER_TABLE(X)
#undef X

#define PROPD_OBJ_HELPER_TABLE(m)                    \
  /* name      keyType     */                        \
  m(propCDO,   KeyType::Any)                         \
  m(propCDOS,  KeyType::Str)                         \

#define X(nm, kt)                                                 \
inline tv_lval nm(Class* ctx, ObjectData* base, key_type<kt> key, \
                  TypedValue& tvRef, ReadOnlyOp op) {             \
  return PropObj<MOpMode::Define,kt>(tvRef, ctx, base, key, op);  \
}
PROPD_OBJ_HELPER_TABLE(X)
#undef X

// NullSafe prop.
inline tv_lval propCQ(Class* ctx, tv_rval base, StringData* key,
                      TypedValue& tvRef, ReadOnlyOp op) {
  return nullSafeProp(tvRef, ctx, base, key, op);
}

// NullSafe prop with object base.
inline tv_lval propCOQ(Class* ctx, ObjectData* base, StringData* key,
                       TypedValue& tvRef, ReadOnlyOp op) {
  return base->prop(&tvRef, ctx, key, op);
}

//////////////////////////////////////////////////////////////////////

inline TypedValue cGetRefShuffle(const TypedValue& localTvRef,
                                 tv_rval result) {
  if (LIKELY(&val(result) != &localTvRef.m_data)) {
    tvIncRefGen(*result);
  }

  return *result;
}

#define CGET_PROP_HELPER_TABLE(m)                      \
  /* name            keyType       mode  */            \
  m(cGetPropCQuiet,  KeyType::Any, MOpMode::None)      \
  m(cGetPropSQuiet,  KeyType::Str, MOpMode::None)      \
  m(cGetPropC,       KeyType::Any, MOpMode::Warn)      \
  m(cGetPropS,       KeyType::Str, MOpMode::Warn)      \

#define X(nm, kt, mode)                                               \
inline TypedValue nm(Class* ctx, tv_lval base, key_type<kt> key,      \
                     ReadOnlyOp op) {                                 \
  TypedValue localTvRef;                                              \
  auto result = Prop<mode,kt>(localTvRef, ctx, base, key, op);        \
  return cGetRefShuffle(localTvRef, result);                          \
}
CGET_PROP_HELPER_TABLE(X)
#undef X

#define CGET_OBJ_PROP_HELPER_TABLE(m)                  \
  /* name            keyType       mode */             \
  m(cGetPropCOQuiet, KeyType::Any, MOpMode::None)      \
  m(cGetPropSOQuiet, KeyType::Str, MOpMode::None)      \
  m(cGetPropCO,      KeyType::Any, MOpMode::Warn)      \
  m(cGetPropSO,      KeyType::Str, MOpMode::Warn)      \

#define X(nm, kt, mode)                                               \
inline TypedValue nm(Class* ctx, ObjectData* base, key_type<kt> key,  \
                     ReadOnlyOp op) {                                 \
  TypedValue localTvRef;                                              \
  auto result = PropObj<mode,kt>(localTvRef, ctx, base, key, op);     \
  return cGetRefShuffle(localTvRef, result);                          \
}
CGET_OBJ_PROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// NullSafe prop.
inline TypedValue cGetPropSQ(Class* ctx, tv_lval base, StringData* key,
                             ReadOnlyOp op) {
  TypedValue localTvRef;
  auto result = nullSafeProp(localTvRef, ctx, base, key, op);
  return cGetRefShuffle(localTvRef, result);
}

// NullSafe prop with object base.
inline TypedValue cGetPropSOQ(Class* ctx, ObjectData* base, StringData* key,
                              ReadOnlyOp op) {
  TypedValue localTvRef;
  auto result = base->prop(&localTvRef, ctx, key, op);
  return cGetRefShuffle(localTvRef, result);
}

//////////////////////////////////////////////////////////////////////

#define SETPROP_HELPER_TABLE(m)          \
  /* name        keyType      */         \
  m(setPropC,    KeyType::Any)           \
  m(setPropCS,   KeyType::Str)           \

#define X(nm, kt)                                                            \
inline void nm(Class* ctx, tv_lval base, key_type<kt> key, TypedValue val,   \
               ReadOnlyOp op) {                                              \
  HPHP::SetProp<false, kt>(ctx, base, key, &val, op);                        \
}
SETPROP_HELPER_TABLE(X)
#undef X

#define SETPROP_OBJ_HELPER_TABLE(m)     \
  /* name        keyType     */         \
  m(setPropCO,   KeyType::Any)          \
  m(setPropCOS,  KeyType::Str)          \

#define X(nm, kt)                                                              \
inline void nm(Class* ctx, ObjectData* base, key_type<kt> key, TypedValue val, \
               ReadOnlyOp op) {                                                \
  HPHP::SetPropObj<kt>(ctx, base, key, &val, op);                              \
}
SETPROP_OBJ_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline void unsetPropC(Class* ctx, tv_lval base, TypedValue key) {
  HPHP::UnsetProp(ctx, base, key);
}

inline void unsetPropCO(Class* ctx, ObjectData* base, TypedValue key) {
  HPHP::UnsetPropObj(ctx, base, key);
}

//////////////////////////////////////////////////////////////////////

inline TypedValue setOpPropC(Class* ctx, tv_lval base, TypedValue key,
                             TypedValue val, SetOpOp op) {
  TypedValue localTvRef;
  auto result = HPHP::SetOpProp(localTvRef, ctx, op, base, key, &val);
  return cGetRefShuffle(localTvRef, result);
}

inline TypedValue setOpPropCO(Class* ctx, ObjectData* base, TypedValue key,
                              TypedValue val, SetOpOp op) {
  TypedValue localTvRef;
  auto result = SetOpPropObj(localTvRef, ctx, op, base, key, &val);
  return cGetRefShuffle(localTvRef, result);
}

//////////////////////////////////////////////////////////////////////

inline TypedValue incDecPropC(Class* ctx, tv_lval base, TypedValue key,
                              IncDecOp op) {
  return HPHP::IncDecProp(ctx, op, base, key);
}

inline TypedValue incDecPropCO(Class* ctx, ObjectData* base, TypedValue key,
                               IncDecOp op) {
  return HPHP::IncDecPropObj(ctx, op, base, key);
}

//////////////////////////////////////////////////////////////////////

#define ISSET_PROP_HELPER_TABLE(m) \
  /* name        keyType */        \
  m(issetPropC,  KeyType::Any)     \
  m(issetPropCS, KeyType::Str)     \

#define X(nm, kt)                                                   \
/* This returns uint64_t to ensure all 64 bits of rax are valid. */ \
inline uint64_t nm(Class* ctx, tv_lval base, key_type<kt> key) {    \
  return HPHP::IssetProp<kt>(ctx, base, key);                       \
}
ISSET_PROP_HELPER_TABLE(X)
#undef X

#define ISSET_OBJ_PROP_HELPER_TABLE(m) \
  /* name         keyType */           \
  m(issetPropCO,  KeyType::Any)        \
  m(issetPropCOS, KeyType::Str)        \

#define X(nm, kt)                                                    \
/* This returns uint64_t to ensure all 64 bits of rax are valid. */  \
inline uint64_t nm(Class* ctx, ObjectData* base, key_type<kt> key) { \
  return IssetPropObj<kt>(ctx, base, key);                           \
}
ISSET_OBJ_PROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline void profileDictAccessHelper(const ArrayData* ad, int64_t i,
                                    ArrayAccessProfile* prof,
                                    bool cowCheck) {
  prof->update(ad, i, cowCheck);
}
inline void profileDictAccessHelper(const ArrayData* ad, const StringData* sd,
                                    ArrayAccessProfile* prof,
                                    bool cowCheck) {
  prof->update(ad, sd, cowCheck);
}

#define PROFILE_DICT_ACCESS_HELPER_TABLE(m)                 \
  /* name                keyType  */                        \
  m(profileDictAccessS,  KeyType::Str)                      \
  m(profileDictAccessI,  KeyType::Int)                      \

#define X(nm, keyType)                                      \
inline void nm(const ArrayData* a, key_type<keyType> k,     \
               ArrayAccessProfile* p, bool cowCheck) {      \
  profileDictAccessHelper(a, k, p, cowCheck);               \
}
PROFILE_DICT_ACCESS_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline void profileKeysetAccessHelper(const ArrayData* ad, int64_t i,
                                      ArrayAccessProfile* prof,
                                      bool cowCheck) {
  prof->update(ad, i, cowCheck);
}
inline void profileKeysetAccessHelper(const ArrayData* ad, const StringData* sd,
                                      ArrayAccessProfile* prof,
                                      bool cowCheck) {
  prof->update(ad, sd, cowCheck);
}

#define PROFILE_KEYSET_ACCESS_HELPER_TABLE(m)               \
  /* name                keyType  */                        \
  m(profileKeysetAccessS,  KeyType::Str)                    \
  m(profileKeysetAccessI,  KeyType::Int)                    \

#define X(nm, keyType)                                      \
inline void nm(const ArrayData* a, key_type<keyType> k,     \
               ArrayAccessProfile* p, bool cowCheck) {      \
  profileKeysetAccessHelper(a, k, p, cowCheck);             \
}
PROFILE_KEYSET_ACCESS_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_HELPER_TABLE(m)                    \
  /* name      keyType         mode */          \
  m(elemC,     KeyType::Any,   MOpMode::None)   \
  m(elemCW,    KeyType::Any,   MOpMode::Warn)   \
  m(elemCIO,   KeyType::Any,   MOpMode::InOut)  \
  m(elemI,     KeyType::Int,   MOpMode::None)   \
  m(elemIW,    KeyType::Int,   MOpMode::Warn)   \
  m(elemIIO,   KeyType::Int,   MOpMode::InOut)  \
  m(elemS,     KeyType::Str,   MOpMode::None)   \
  m(elemSW,    KeyType::Str,   MOpMode::Warn)   \
  m(elemSIO,   KeyType::Str,   MOpMode::InOut)  \

#define X(nm, keyType, mode)                                \
inline TypedValue nm(tv_lval base, key_type<keyType> key) { \
  return Elem<mode, keyType>(base, key);                    \
}
ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEMD_HELPER_TABLE(m) \
  /* name      keyType     */ \
  m(elemCD,    KeyType::Any)  \
  m(elemID,    KeyType::Int)  \
  m(elemSD,    KeyType::Str)  \

#define X(nm, keyType)                                   \
inline tv_lval nm(tv_lval base, key_type<keyType> key) { \
  return ElemD<keyType>(base, key);                      \
}
ELEMD_HELPER_TABLE(X)
#undef X

#define ELEMU_HELPER_TABLE(m) \
  /* name      keyType */     \
  m(elemCU,    KeyType::Any)  \
  m(elemIU,    KeyType::Int)  \
  m(elemSU,    KeyType::Str)  \

#define X(nm, keyType)                                   \
inline tv_lval nm(tv_lval base, key_type<keyType> key) { \
  return ElemU<keyType>(base, key);                      \
}
ELEMU_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_DICT_D_HELPER_TABLE(m) \
  /* name          keyType       */ \
  m(elemDictSD,    KeyType::Str)    \
  m(elemDictID,    KeyType::Int)    \

#define X(nm, keyType)                                   \
inline tv_lval nm(tv_lval base, key_type<keyType> key) { \
  assertx(tvIsDict(base));                               \
  return ElemDDict<keyType>(base, key);                  \
}
ELEM_DICT_D_HELPER_TABLE(X)
#undef X

#define ELEM_DICT_U_HELPER_TABLE(m)  \
  /* name        keyType */          \
  m(elemDictSU, KeyType::Str)        \
  m(elemDictIU, KeyType::Int)        \

#define X(nm, keyType)                                   \
inline tv_lval nm(tv_lval base, key_type<keyType> key) { \
  assertx(tvIsDict(base));                               \
  return ElemUDict<keyType>(base, key);                  \
}
ELEM_DICT_U_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_KEYSET_U_HELPER_TABLE(m)  \
  /* name         keyType */           \
  m(elemKeysetSU, KeyType::Str)        \
  m(elemKeysetIU, KeyType::Int)        \

#define X(nm, keyType)                                                \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {              \
  assertx(isKeysetType(type(base)));                                  \
  return ElemUKeyset<keyType>(base, key);                             \
}
ELEM_KEYSET_U_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define DICTGET_HELPER_TABLE(m)                                 \
  /* name          keyType        mode  */                      \
  m(dictGetS,      KeyType::Str,  MOpMode::Warn)                \
  m(dictGetI,      KeyType::Int,  MOpMode::Warn)                \
  m(dictGetSQuiet, KeyType::Str,  MOpMode::None)                \
  m(dictGetIQuiet, KeyType::Int,  MOpMode::None)                \

#define X(nm, keyType, mode)                                \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) { \
  return HPHP::ElemDict<mode, keyType>(a, key);             \
}
DICTGET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define KEYSETGET_HELPER_TABLE(m)                                 \
  /* name            keyType        mode */                       \
  m(keysetGetS,      KeyType::Str,  MOpMode::Warn)                \
  m(keysetGetI,      KeyType::Int,  MOpMode::Warn)                \
  m(keysetGetSQuiet, KeyType::Str,  MOpMode::None)                \
  m(keysetGetIQuiet, KeyType::Int,  MOpMode::None)                \

#define X(nm, keyType, mode)                                 \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) {  \
  return HPHP::ElemKeyset<mode, keyType>(a, key);            \
}
KEYSETGET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, MOpMode mode>
TypedValue cGetElemImpl(tv_lval base, key_type<keyType> key) {
  auto const result = Elem<mode, keyType>(base, key);
  tvIncRefGen(result);
  return result;
}

#define CGETELEM_HELPER_TABLE(m)                    \
  /* name            key           mode */          \
  m(cGetElemCQuiet,  KeyType::Any, MOpMode::None)   \
  m(cGetElemIQuiet,  KeyType::Int, MOpMode::None)   \
  m(cGetElemSQuiet,  KeyType::Str, MOpMode::None)   \
  m(cGetElemC,       KeyType::Any, MOpMode::Warn)   \
  m(cGetElemI,       KeyType::Int, MOpMode::Warn)   \
  m(cGetElemS,       KeyType::Str, MOpMode::Warn)   \
  m(cGetElemCIO,     KeyType::Any, MOpMode::InOut)  \
  m(cGetElemIIO,     KeyType::Int, MOpMode::InOut)  \
  m(cGetElemSIO,     KeyType::Str, MOpMode::InOut)  \

#define X(nm, kt, mode)                                 \
inline TypedValue nm(tv_lval base, key_type<kt> key) {  \
  return cGetElemImpl<kt, mode>(base, key);             \
}
CGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline ArrayData* dictSetImplPre(ArrayData* a, int64_t i, TypedValue val) {
  return MixedArray::SetIntMove(a, i, val);
}
inline ArrayData* dictSetImplPre(ArrayData* a, StringData* s, TypedValue val) {
  return MixedArray::SetStrMove(a, s, val);
}

template<KeyType keyType>
auto dictSetImpl(ArrayData* a, key_type<keyType> key, TypedValue value) {
  assertx(tvIsPlausible(value));
  assertx(a->isVanillaDict());
  return dictSetImplPre(a, key, value);
}

#define DICTSET_HELPER_TABLE(m) \
  /* name       keyType      */ \
  m(dictSetI,   KeyType::Int)   \
  m(dictSetS,   KeyType::Str)   \

#define X(nm, keyType)                                                      \
inline ArrayData* nm(ArrayData* a, key_type<keyType> key, TypedValue val) { \
  return dictSetImpl<keyType>(a, key, val);                                 \
}
DICTSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline ArrayData* keysetSetNewElemImplPre(ArrayData* a, int64_t i) {
  return SetArray::AddToSet(a, i);
}

inline ArrayData* keysetSetNewElemImplPre(ArrayData* a, StringData* s) {
  return SetArray::AddToSet(a, s);
}

template<KeyType keyType>
void keysetSetNewElemImpl(tv_lval base, key_type<keyType> key) {
  assertx(tvIsPlausible(*base));
  assertx(tvIsKeyset(base));
  auto oldArr = val(base).parr;
  auto newArr = keysetSetNewElemImplPre(oldArr, key);
  if (oldArr != newArr) {
    type(base) = KindOfKeyset;
    val(base).parr = newArr;
    assertx(tvIsPlausible(*base));
    decRefArr(oldArr);
  }
}

#define KEYSET_SETNEWELEM_HELPER_TABLE(m)       \
  /* name              keyType      */          \
  m(keysetSetNewElemI, KeyType::Int)            \
  m(keysetSetNewElemS, KeyType::Str)            \

#define X(nm, keyType)                                   \
inline void nm(tv_lval tv, key_type<keyType> key) {  \
  keysetSetNewElemImpl<keyType>(tv, key);                \
}
KEYSET_SETNEWELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType>
StringData* setElemImpl(tv_lval base, key_type<keyType> key, TypedValue val) {
  return HPHP::SetElem<false, keyType>(base, key, &val);
}

#define SETELEM_HELPER_TABLE(m) \
  /* name       keyType      */ \
  m(setElemC,  KeyType::Any)    \
  m(setElemI,  KeyType::Int)    \
  m(setElemS,  KeyType::Str)    \

#define X(nm, kt)                                                       \
inline StringData* nm(tv_lval base, key_type<kt> key, TypedValue val) { \
  return setElemImpl<kt>(base, key, val);                               \
}
SETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define DICT_ISSET_ELEM_HELPER_TABLE(m) \
  /* name           keyType */          \
  m(dictIssetElemS, KeyType::Str)       \
  m(dictIssetElemI, KeyType::Int)       \

#define X(nm, keyType)                                    \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) { \
  return IssetElemDict<keyType>(a, key);                  \
}
DICT_ISSET_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define KEYSET_ISSET_ELEM_HELPER_TABLE(m) \
  /* name             keyType */          \
  m(keysetIssetElemS, KeyType::Str)       \
  m(keysetIssetElemI, KeyType::Int)       \

#define X(nm, keyType)                                    \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) { \
  return IssetElemKeyset<keyType>(a, key);                \
}
KEYSET_ISSET_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<KeyType keyType>
void unsetElemImpl(tv_lval base, key_type<keyType> key) {
  HPHP::UnsetElem<keyType>(base, key);
}

#define UNSET_ELEM_HELPER_TABLE(m)  \
  /* name         keyType */        \
  m(unsetElemC,   KeyType::Any)     \
  m(unsetElemI,   KeyType::Int)     \
  m(unsetElemS,   KeyType::Str)     \

#define X(nm, kt)                                 \
inline void nm(tv_lval base, key_type<kt> key) {  \
  unsetElemImpl<kt>(base, key);                   \
}
UNSET_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ISSET_ELEM_HELPER_TABLE(m) \
  /* name       keyType */         \
  m(issetElemC, KeyType::Any)      \
  m(issetElemI, KeyType::Int)      \
  m(issetElemS, KeyType::Str)      \

#define X(nm, kt)                                    \
inline uint64_t nm(tv_lval base, key_type<kt> key) { \
  return HPHP::IssetElem<kt>(base, key);             \
}
ISSET_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<KeyType keyType>
TypedValue mapGetImpl(c_Map* map, key_type<keyType> key) {
  auto const ret = map->at(key);
  tvIncRefGen(*ret);
  return *ret;
}

template<KeyType keyType>
uint64_t mapIssetImpl(c_Map* map, key_type<keyType> key) {
  auto result = map->get(key);
  return result ? !tvIsNull(result) : false;
}

template<KeyType keyType>
void mapSetImpl(c_Map* map, key_type<keyType> key, TypedValue value) {
  map->setMove(key, value);
}

inline
void vectorSetImplI(c_Vector* vector, int64_t key, TypedValue value) {
  vector->setMove(key, value);
}

[[noreturn]] inline
void vectorSetImplS(c_Vector* vector, StringData* key, TypedValue value) {
  BaseVector::throwBadKeyType();
}

//////////////////////////////////////////////////////////////////////

}}}
