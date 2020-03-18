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
#ifndef incl_HPHP_MINSTR_HELPERS_H_
#define incl_HPHP_MINSTR_HELPERS_H_

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

  auto const varEnv = g_context->m_globalVarEnv;
  assertx(varEnv != nullptr);

  auto base = varEnv->lookup(name);
  if (base == nullptr) {
    if (mode == MOpMode::Warn) throwArrayKeyException(name, false);
    if (mode == MOpMode::Define) {
      auto tv = make_tv<KindOfNull>();
      varEnv->set(name, &tv);
      base = varEnv->lookup(name);
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
inline tv_lval nm(TypedValue key) {         \
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

#define X(nm, mode, kt)                                               \
inline tv_lval nm(Class* ctx, tv_lval base, key_type<kt> key,         \
                  TypedValue& tvRef) {                                \
  return Prop<mode,kt>(tvRef, ctx, base, key, nullptr);               \
}
PROP_HELPER_TABLE(X)
#undef X

#define PROPD_HELPER_TABLE(m)                       \
  /* name      keyType     */                       \
  m(propCD,    KeyType::Any)                        \
  m(propCDS,   KeyType::Str)

#define X(nm, kt)                                                     \
inline tv_lval nm(Class* ctx, tv_lval base, key_type<kt> key,         \
                  TypedValue& tvRef, MInstrPropState* pState) {       \
  return Prop<MOpMode::Define,kt>(tvRef, ctx, base, key, pState);     \
}
PROPD_HELPER_TABLE(X)
#undef X

#define PROP_OBJ_HELPER_TABLE(m)                      \
  /* name      mode                  keyType     */   \
  m(propCO,    MOpMode::None,       KeyType::Any)    \
  m(propCOS,   MOpMode::None,       KeyType::Str)    \
  m(propCUO,   MOpMode::Unset,      KeyType::Any)    \
  m(propCUOS,  MOpMode::Unset,      KeyType::Str)    \
  m(propCWO,   MOpMode::Warn,       KeyType::Any)    \
  m(propCWOS,  MOpMode::Warn,       KeyType::Str)    \

#define X(nm, mode, kt)                                               \
inline tv_lval nm(Class* ctx, ObjectData* base, key_type<kt> key,     \
                  TypedValue& tvRef) {                                \
  return PropObj<mode,kt>(tvRef, ctx, base, key, nullptr);            \
}
PROP_OBJ_HELPER_TABLE(X)
#undef X

#define PROPD_OBJ_HELPER_TABLE(m)                    \
  /* name      keyType     */                        \
  m(propCDO,   KeyType::Any)                         \
  m(propCDOS,  KeyType::Str)                         \

#define X(nm, kt)                                                     \
inline tv_lval nm(Class* ctx, ObjectData* base, key_type<kt> key,     \
                  TypedValue& tvRef, MInstrPropState* pState) {       \
  return PropObj<MOpMode::Define,kt>(tvRef, ctx, base, key, pState);  \
}
PROPD_OBJ_HELPER_TABLE(X)
#undef X

// NullSafe prop.
inline tv_lval propCQ(Class* ctx, tv_rval base, StringData* key,
                      TypedValue& tvRef) {
  return nullSafeProp(tvRef, ctx, base, key);
}

// NullSafe prop with object base.
inline tv_lval propCOQ(Class* ctx, ObjectData* base, StringData* key,
                       TypedValue& tvRef) {
  return base->prop(&tvRef, ctx, key);
}

//////////////////////////////////////////////////////////////////////

inline TypedValue cGetRefShuffle(const TypedValue& localTvRef,
                                 tv_rval result) {
  if (LIKELY(&val(result) != &localTvRef.m_data)) {
    tvIncRefGen(*result);
  }

  return *result;
}

#define CGET_PROP_HELPER_TABLE(m)                       \
  /* name            keyType       mode  */             \
  m(cGetPropCQuiet,  KeyType::Any, MOpMode::None)      \
  m(cGetPropSQuiet,  KeyType::Str, MOpMode::None)      \
  m(cGetPropC,       KeyType::Any, MOpMode::Warn)      \
  m(cGetPropS,       KeyType::Str, MOpMode::Warn)      \

#define X(nm, kt, mode)                                                \
inline TypedValue nm(Class* ctx, tv_lval base, key_type<kt> key) {     \
  TypedValue localTvRef;                                               \
  auto result = Prop<mode,kt>(localTvRef, ctx, base, key, nullptr);    \
  return cGetRefShuffle(localTvRef, result);                           \
}
CGET_PROP_HELPER_TABLE(X)
#undef X

#define CGET_OBJ_PROP_HELPER_TABLE(m)                   \
  /* name            keyType       mode */             \
  m(cGetPropCOQuiet, KeyType::Any, MOpMode::None)      \
  m(cGetPropSOQuiet, KeyType::Str, MOpMode::None)      \
  m(cGetPropCO,      KeyType::Any, MOpMode::Warn)      \
  m(cGetPropSO,      KeyType::Str, MOpMode::Warn)      \

#define X(nm, kt, mode)                                                \
inline TypedValue nm(Class* ctx, ObjectData* base, key_type<kt> key) { \
  TypedValue localTvRef;                                               \
  auto result = PropObj<mode,kt>(localTvRef, ctx, base, key, nullptr); \
  return cGetRefShuffle(localTvRef, result);                           \
}
CGET_OBJ_PROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// NullSafe prop.
inline TypedValue cGetPropSQ(Class* ctx, tv_lval base, StringData* key) {
  TypedValue localTvRef;
  auto result = nullSafeProp(localTvRef, ctx, base, key);
  return cGetRefShuffle(localTvRef, result);
}

// NullSafe prop with object base.
inline TypedValue cGetPropSOQ(Class* ctx, ObjectData* base, StringData* key) {
  TypedValue localTvRef;
  auto result = base->prop(&localTvRef, ctx, key);
  return cGetRefShuffle(localTvRef, result);
}

//////////////////////////////////////////////////////////////////////

#define SETPROP_HELPER_TABLE(m)          \
  /* name        keyType      */         \
  m(setPropC,    KeyType::Any)           \
  m(setPropCS,   KeyType::Str)           \

#define X(nm, kt)                                                       \
inline void nm(Class* ctx, tv_lval base, key_type<kt> key,              \
               TypedValue val, const MInstrPropState* pState) {               \
  HPHP::SetProp<false, kt>(ctx, base, key, &val, pState);               \
}
SETPROP_HELPER_TABLE(X)
#undef X

#define SETPROP_OBJ_HELPER_TABLE(m)     \
  /* name        keyType     */         \
  m(setPropCO,   KeyType::Any)          \
  m(setPropCOS,  KeyType::Str)          \

#define X(nm, kt)                                                          \
inline void nm(Class* ctx, ObjectData* base, key_type<kt> key, TypedValue val) { \
  HPHP::SetPropObj<kt>(ctx, base, key, &val);                              \
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
                             TypedValue val, SetOpOp op,
                             const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto result = HPHP::SetOpProp(localTvRef, ctx, op, base, key, &val, pState);
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
                              IncDecOp op, const MInstrPropState* pState) {
  return HPHP::IncDecProp(ctx, op, base, key, pState);
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

inline void profileMixedArrayAccessHelper(const ArrayData* ad, int64_t i,
                                          ArrayAccessProfile* prof,
                                          bool cowCheck) {
  prof->update(ad, i, cowCheck);
}
inline void profileMixedArrayAccessHelper(const ArrayData* ad,
                                          const StringData* sd,
                                          ArrayAccessProfile* prof,
                                          bool cowCheck) {
  prof->update(ad, sd, cowCheck);
}

#define PROFILE_MIXED_ARRAY_ACCESS_HELPER_TABLE(m)  \
  /* name                      keyType */           \
  m(profileMixedArrayAccessS,  KeyType::Str)        \
  m(profileMixedArrayAccessI,  KeyType::Int)        \

#define X(nm, keyType)                              \
inline void nm(const ArrayData* a,                  \
               key_type<keyType> k,                 \
               ArrayAccessProfile* p,               \
               bool cowCheck) {                     \
  profileMixedArrayAccessHelper(a, k, p, cowCheck); \
}
PROFILE_MIXED_ARRAY_ACCESS_HELPER_TABLE(X)
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

template <KeyType keyType, MOpMode mode>
tv_lval elemImpl(tv_lval base, key_type<keyType> key, TypedValue& tvRef) {
  static_assert(mode != MOpMode::Define, "");
  if (mode == MOpMode::Unset) {
    return ElemU<keyType>(tvRef, base, key);
  }
  // We won't really modify the TypedValue in the non-D case, so
  // this lval cast is safe.
  return Elem<mode, keyType>(tvRef, base, key).as_lval();
}

#define ELEM_HELPER_TABLE(m)                    \
  /* name      keyType         mode */          \
  m(elemC,     KeyType::Any,   MOpMode::None)   \
  m(elemCU,    KeyType::Any,   MOpMode::Unset)  \
  m(elemCW,    KeyType::Any,   MOpMode::Warn)   \
  m(elemCIO,   KeyType::Any,   MOpMode::InOut)  \
  m(elemI,     KeyType::Int,   MOpMode::None)   \
  m(elemIU,    KeyType::Int,   MOpMode::Unset)  \
  m(elemIW,    KeyType::Int,   MOpMode::Warn)   \
  m(elemIIO,   KeyType::Int,   MOpMode::InOut)  \
  m(elemS,     KeyType::Str,   MOpMode::None)   \
  m(elemSU,    KeyType::Str,   MOpMode::Unset)  \
  m(elemSW,    KeyType::Str,   MOpMode::Warn)   \
  m(elemSIO,   KeyType::Str,   MOpMode::InOut)  \

#define X(nm, keyType, mode)                        \
inline tv_lval nm(tv_lval base,                     \
                  key_type<keyType> key,            \
                  TypedValue& tvRef) {              \
  return elemImpl<keyType, mode>(base, key, tvRef); \
}
ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEMD_HELPER_TABLE(m) \
  /* name      keyType        copyProv */ \
  m(elemCDN,    KeyType::Any, false)      \
  m(elemIDN,    KeyType::Int, false)      \
  m(elemSDN,    KeyType::Str, false)      \
  m(elemCDP,    KeyType::Any, true)       \
  m(elemIDP,    KeyType::Int, true)       \
  m(elemSDP,    KeyType::Str, true)       \

#define X(nm, keyType, copyProv)                           \
inline tv_lval nm(tv_lval base,                            \
                  key_type<keyType> key,                   \
                  TypedValue& tvRef,                       \
                  const MInstrPropState* pState) {         \
  return ElemD<MOpMode::Define, keyType, copyProv>( \
    tvRef, base, key, pState                               \
  );                                                       \
}
ELEMD_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_ARRAY_D_HELPER_TABLE(m)  \
  /* name           keyType */        \
  m(elemArraySD,    KeyType::Str)     \
  m(elemArrayID,    KeyType::Int)     \

#define X(nm, keyType)                                          \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {        \
  assertx(isArrayType(type(base)));                            \
  return ElemDArray<MOpMode::None, keyType>(base, key); \
}
ELEM_ARRAY_D_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_U_HELPER_TABLE(m)  \
  /* name         keyType */          \
  m(elemArraySU,  KeyType::Str)       \
  m(elemArrayIU,  KeyType::Int)       \

#define X(nm, keyType)                                     \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {   \
  assertx(isArrayType(type(base)));                       \
  return ElemUArray<keyType>(base, key);                  \
}
ELEM_ARRAY_U_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_HELPER_TABLE(m)                    \
  /* name             keyType       mode */           \
  m(elemArrayS,       KeyType::Str, MOpMode::None)    \
  m(elemArrayI,       KeyType::Int, MOpMode::None)    \
  m(elemArraySW,      KeyType::Str, MOpMode::Warn)    \
  m(elemArrayIW,      KeyType::Int, MOpMode::Warn)    \
  m(elemArraySW_IO,   KeyType::Str, MOpMode::InOut)   \
  m(elemArrayIW_IO,   KeyType::Int, MOpMode::InOut)   \

#define X(nm, keyType, mode)                              \
inline tv_rval nm(ArrayData* ad, key_type<keyType> key) { \
  return ElemArray<mode, keyType>(ad, key);               \
}
ELEM_ARRAY_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// Keep these error handlers in sync with ArrayData::getNotFound();
[[noreturn]]
void arrayGetNotFound(int64_t k);
[[noreturn]]
void arrayGetNotFound(const StringData* k);

template<KeyType keyType, MOpMode mode>
TypedValue arrayGetImpl(ArrayData* a, key_type<keyType> key) {
  auto ret = a->rval(key);
  if (ret) return ret.tv();
  if (mode == MOpMode::None) return make_tv<KindOfNull>();
  if (mode == MOpMode::InOut) {
    throw_inout_undefined_index(key);
    return make_tv<KindOfNull>();
  }
  assertx(mode == MOpMode::Warn);
  arrayGetNotFound(key);
}

#define ARRAYGET_HELPER_TABLE(m)                  \
  /* name           keyType       mode */         \
  m(arrayGetS,      KeyType::Str, MOpMode::None)  \
  m(arrayGetI,      KeyType::Int, MOpMode::None)  \
  m(arrayGetS_W,    KeyType::Str, MOpMode::Warn)  \
  m(arrayGetI_W,    KeyType::Int, MOpMode::Warn)  \
  m(arrayGetS_IO,   KeyType::Str, MOpMode::InOut) \
  m(arrayGetI_IO,   KeyType::Int, MOpMode::InOut) \

#define X(nm, keyType, mode)                                \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) { \
  return arrayGetImpl<keyType, mode>(a, key);               \
}
ARRAYGET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_VEC_D_HELPER_TABLE(m) \
  /* name          copyProv*/ \
  m(elemVecIDN,    false)     \
  m(elemVecIDP,    true)      \

#define X(nm, copyProv) \
inline tv_lval nm(tv_lval base, int64_t key) {                 \
  assertx(isVecType(type(base)));                             \
  return ElemDVec<KeyType::Int, copyProv>(base, key);  \
}
ELEM_VEC_D_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_DICT_D_HELPER_TABLE(m) \
  /* name          keyType       copyProv*/ \
  m(elemDictSDN,    KeyType::Str, false)    \
  m(elemDictIDN,    KeyType::Int, false)    \
  m(elemDictSDP,    KeyType::Str, true)     \
  m(elemDictIDP,    KeyType::Int, true)     \

#define X(nm, keyType, copyProv)                          \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {  \
  assertx(isDictType(type(base)));                       \
  return ElemDDict<keyType, copyProv>(base, key); \
}
ELEM_DICT_D_HELPER_TABLE(X)
#undef X

#define ELEM_DICT_U_HELPER_TABLE(m)  \
  /* name        keyType */          \
  m(elemDictSU, KeyType::Str)        \
  m(elemDictIU, KeyType::Int)        \

#define X(nm, keyType)                                                 \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {               \
  assertx(isDictType(type(base)));                                    \
  return ElemUDict<keyType>(base, key);                               \
}
ELEM_DICT_U_HELPER_TABLE(X)
#undef X

#define ELEM_DICT_HELPER_TABLE(m)                      \
  /* name               keyType        mode */         \
  m(elemDictS,     KeyType::Str,       MOpMode::None)  \
  m(elemDictI,     KeyType::Int,       MOpMode::None)  \
  m(elemDictSW,    KeyType::Str,       MOpMode::Warn)  \
  m(elemDictIW,    KeyType::Int,       MOpMode::Warn)  \
  m(elemDictSIO,   KeyType::Str,       MOpMode::InOut) \
  m(elemDictIIO,   KeyType::Int,       MOpMode::InOut) \

#define X(nm, keyType, mode) \
inline tv_rval nm(ArrayData* ad, key_type<keyType> key) { \
  return HPHP::ElemDict<mode, keyType>(ad, key); \
}
ELEM_DICT_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_KEYSET_U_HELPER_TABLE(m)  \
  /* name         keyType */          \
  m(elemKeysetSU, KeyType::Str)        \
  m(elemKeysetIU, KeyType::Int)        \

#define X(nm, keyType)                                                 \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {               \
  assertx(isKeysetType(type(base)));                                  \
  return ElemUKeyset<keyType>(base, key);                             \
}
ELEM_KEYSET_U_HELPER_TABLE(X)
#undef X

#define ELEM_KEYSET_HELPER_TABLE(m)                               \
  /* name            keyType             mode */                  \
  m(elemKeysetS,     KeyType::Str,       MOpMode::None)          \
  m(elemKeysetI,     KeyType::Int,       MOpMode::None)          \
  m(elemKeysetSW,    KeyType::Str,       MOpMode::Warn)          \
  m(elemKeysetIW,    KeyType::Int,       MOpMode::Warn)          \
  m(elemKeysetSIO,   KeyType::Str,       MOpMode::InOut)         \
  m(elemKeysetIIO,   KeyType::Int,       MOpMode::InOut)         \

#define X(nm, keyType, mode) \
inline tv_rval nm(ArrayData* ad, key_type<keyType> key) { \
  return HPHP::ElemKeyset<mode, keyType>(ad, key); \
}
ELEM_KEYSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define DICTGET_HELPER_TABLE(m)                                  \
  /* name          keyType        mode  */                       \
  m(dictGetS,      KeyType::Str,  MOpMode::Warn)                \
  m(dictGetI,      KeyType::Int,  MOpMode::Warn)                \
  m(dictGetSQuiet, KeyType::Str,  MOpMode::None)                \
  m(dictGetIQuiet, KeyType::Int,  MOpMode::None)                \

#define X(nm, keyType, mode) \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) { \
  return HPHP::ElemDict<mode, keyType>(a, key).tv(); \
}
DICTGET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define KEYSETGET_HELPER_TABLE(m)                                  \
  /* name            keyType        mode */                       \
  m(keysetGetS,      KeyType::Str,  MOpMode::Warn)                \
  m(keysetGetI,      KeyType::Int,  MOpMode::Warn)                \
  m(keysetGetSQuiet, KeyType::Str,  MOpMode::None)                \
  m(keysetGetIQuiet, KeyType::Int,  MOpMode::None)                \

#define X(nm, keyType, mode) \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) {  \
  return HPHP::ElemKeyset<mode, keyType>(a, key).tv(); \
}
KEYSETGET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, MOpMode mode>
TypedValue cGetElemImpl(tv_lval base, key_type<keyType> key) {
  TypedValue localTvRef;
  auto result = Elem<mode, keyType>(localTvRef, base, key);
  return cGetRefShuffle(localTvRef, result);
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

template<bool copyProv>
auto vecSetImpl(ArrayData* a, int64_t key, TypedValue value) {
  assertx(tvIsPlausible(value));
  assertx(a->isVecArrayKind());
  return PackedArray::SetIntMoveVec(a, key, value);
}

#define VECSET_HELPER_TABLE(m) \
  /* name     copyProv */      \
  m(vecSetIN, false)           \
  m(vecSetIP, true)

#define X(nm, copyProv)                                     \
inline ArrayData* nm(ArrayData* a, int64_t key, TypedValue val) { \
  return vecSetImpl<copyProv>(a, key, val); \
}
VECSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline ArrayData* dictSetImplPre(ArrayData* a, int64_t i, TypedValue val) {
  return MixedArray::SetIntMoveDict(a, i, val);
}
inline ArrayData* dictSetImplPre(ArrayData* a, StringData* s, TypedValue val) {
  return MixedArray::SetStrMoveDict(a, s, val);
}

template<KeyType keyType, bool copyProv>
auto dictSetImpl(ArrayData* a, key_type<keyType> key, TypedValue value) {
  assertx(tvIsPlausible(value));
  assertx(a->isDictKind());
  return dictSetImplPre(a, key, value);
}

#define DICTSET_HELPER_TABLE(m) \
  /* name       keyType        copyProv */ \
  m(dictSetIN,   KeyType::Int, false)      \
  m(dictSetIP,   KeyType::Int, true)       \
  m(dictSetSN,   KeyType::Str, false)      \
  m(dictSetSP,   KeyType::Str, true)

#define X(nm, keyType, copyProv)                                      \
inline ArrayData* nm(ArrayData* a, key_type<keyType> key, TypedValue val) { \
  return dictSetImpl<keyType, copyProv>(a, key, val); \
}
DICTSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool copyProv>
void setNewElem(tv_lval base, TypedValue val, const MInstrPropState* pState) {
  HPHP::SetNewElem<false, copyProv>(base, &val, pState);
}

template <bool copyProv>
void setNewElemVec(tv_lval base, TypedValue val) {
  HPHP::SetNewElemVec<copyProv>(base, &val);
}


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

template <KeyType keyType, bool copyProv>
StringData* setElemImpl(tv_lval base, key_type<keyType> key,
                        TypedValue val, const MInstrPropState* pState) {
  return HPHP::SetElem<false, copyProv, keyType>(base, key, &val, pState);
}

#define SETELEM_HELPER_TABLE(m) \
  /* name       keyType       copyProv*/ \
  m(setElemCN,  KeyType::Any, false)     \
  m(setElemIN,  KeyType::Int, false)     \
  m(setElemSN,  KeyType::Str, false)     \
  m(setElemCP,  KeyType::Any, true)      \
  m(setElemIP,  KeyType::Int, true)      \
  m(setElemSP,  KeyType::Str, true)      \

#define X(nm, kt, copyProv)                                      \
inline StringData* nm(tv_lval base, key_type<kt> key,            \
                      TypedValue val, const MInstrPropState* pState) { \
  return setElemImpl<kt, copyProv>(base, key, val, pState);      \
}
SETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////


template<KeyType keyType>
uint64_t arrayIssetImpl(ArrayData* a, key_type<keyType> key) {
  auto const rval = a->rval(key);
  return !rval
    ? 0
    : !isNullType(rval.type());
}

#define ARRAY_ISSET_HELPER_TABLE(m) \
  /* name           keyType */      \
  m(arrayIssetS,    KeyType::Str)   \
  m(arrayIssetI,    KeyType::Int)   \

#define X(nm, keyType)                                    \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) { \
  return arrayIssetImpl<keyType>(a, key);                 \
}
ARRAY_ISSET_HELPER_TABLE(X)
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

#endif
