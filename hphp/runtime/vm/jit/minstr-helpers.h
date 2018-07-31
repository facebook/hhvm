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

#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/member-operations.h"

#include "hphp/runtime/vm/jit/array-offset-profile.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"

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
    if (mode == MOpMode::Warn) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    if (mode == MOpMode::Define) {
      auto tv = make_tv<KindOfNull>();
      varEnv->set(name, &tv);
      base = varEnv->lookup(name);
    } else {
      return const_cast<TypedValue*>(&immutable_null_base);
    }
  }
  return tvToCell(base);
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
    result = tvToCell(result);
    tvIncRefGen(*result);
  } else {
    // If a magic getter or array access method returned by reference, we have
    // to incref the inner cell and drop our reference to the RefData.
    // Otherwise we do nothing, since we already own a reference to result.
    if (UNLIKELY(isRefType(localTvRef.m_type))) {
      auto inner = *localTvRef.m_data.pref->tv();
      tvIncRefGen(inner);
      decRefRef(localTvRef.m_data.pref);
      return inner;
    }
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

inline RefData* vGetRefShuffle(const TypedValue& localTvRef,
                               tv_lval result) {
  if (LIKELY(&val(result) != &localTvRef.m_data)) {
    tvBoxIfNeeded(result);
    auto ref = val(result).pref;
    ref->incRefCount();
    return ref;
  }

  if (!isRefType(localTvRef.m_type)) {
    // RefData::Make takes ownership of the reference that lives in localTvRef
    // so no refcounting is necessary.
    return RefData::Make(localTvRef);
  }

  return localTvRef.m_data.pref;
}

#define VGET_PROP_HELPER_TABLE(m) \
  /* name        keyType     */   \
  m(vGetPropC,   KeyType::Any)    \
  m(vGetPropS,   KeyType::Str)

#define X(nm, kt)                                                       \
inline RefData* nm(Class* ctx, tv_lval base,                            \
                   key_type<kt> key, MInstrPropState* pState) {         \
  TypedValue localTvRef;                                                \
  auto result =                                                         \
    Prop<MOpMode::Define,kt,true>(localTvRef, ctx, base, key, pState);  \
  return vGetRefShuffle(localTvRef, result);                            \
}
VGET_PROP_HELPER_TABLE(X)
#undef X

#define VGET_OBJ_PROP_HELPER_TABLE(m) \
  /* name        keyType      */      \
  m(vGetPropCO,  KeyType::Any)        \
  m(vGetPropSO,  KeyType::Str)

#define X(nm, kt)                                                       \
inline RefData* nm(Class* ctx, ObjectData* base,                        \
                   key_type<kt> key, MInstrPropState* pState) {         \
  TypedValue localTvRef;                                                \
  auto result =                                                         \
    PropObj<MOpMode::Define,kt,true>(localTvRef, ctx, base, key, pState); \
  return vGetRefShuffle(localTvRef, result);                            \
}
VGET_OBJ_PROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<class PropImpl>
void bindPropImpl(RefData* val, PropImpl prop_impl) {
  TypedValue localTvRef;
  auto prop = prop_impl(localTvRef);
  if (UNLIKELY(prop == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvDecRefGen(localTvRef);
  } else {
    tvBindRef(val, prop);
  }
}

inline void bindPropC(Class* ctx, tv_lval base, TypedValue key,
                      RefData* val, MInstrPropState* pState) {
  bindPropImpl(val, [&](TypedValue& tvref) {
    return Prop<MOpMode::Define,KeyType::Any,true>(
      tvref, ctx, base, key, pState
    );
  });
}

inline void bindPropCO(Class* ctx, ObjectData* base, TypedValue key,
                       RefData* val, MInstrPropState* pState) {
  bindPropImpl(val, [&](TypedValue& tvref) {
    return PropObj<MOpMode::Define,KeyType::Any,true>(
      tvref, ctx, base, key, pState
    );
  });
}

//////////////////////////////////////////////////////////////////////

#define SETPROP_HELPER_TABLE(m)          \
  /* name        keyType      */         \
  m(setPropC,    KeyType::Any)           \
  m(setPropCS,   KeyType::Str)           \

#define X(nm, kt)                                                       \
inline void nm(Class* ctx, tv_lval base, key_type<kt> key,              \
               Cell val, const MInstrPropState* pState) {               \
  HPHP::SetProp<false, kt>(ctx, base, key, &val, pState);               \
}
SETPROP_HELPER_TABLE(X)
#undef X

#define SETPROP_OBJ_HELPER_TABLE(m)     \
  /* name        keyType     */         \
  m(setPropCO,   KeyType::Any)          \
  m(setPropCOS,  KeyType::Str)          \

#define X(nm, kt)                                                          \
inline void nm(Class* ctx, ObjectData* base, key_type<kt> key, Cell val) { \
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
                             Cell val, SetOpOp op,
                             const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto result = HPHP::SetOpProp(localTvRef, ctx, op, base, key, &val, pState);
  return cGetRefShuffle(localTvRef, result);
}

inline TypedValue setOpPropCO(Class* ctx, ObjectData* base, TypedValue key,
                              Cell val, SetOpOp op) {
  TypedValue localTvRef;
  auto result = SetOpPropObj(localTvRef, ctx, op, base, key, &val);
  return cGetRefShuffle(localTvRef, result);
}

//////////////////////////////////////////////////////////////////////

inline TypedValue incDecPropC(Class* ctx, tv_lval base, TypedValue key,
                              IncDecOp op, const MInstrPropState* pState) {
  auto const result = HPHP::IncDecProp(ctx, op, base, key, pState);
  assertx(!isRefType(result.m_type));
  return result;
}

inline TypedValue incDecPropCO(Class* ctx, ObjectData* base, TypedValue key,
                               IncDecOp op) {
  auto const result = HPHP::IncDecPropObj(ctx, op, base, key);
  assertx(!isRefType(result.m_type));
  return result;
}

//////////////////////////////////////////////////////////////////////

#define ISSET_EMPTY_PROP_HELPER_TABLE(m)                \
  /* name          keyType       useEmpty */            \
  m(issetPropC,    KeyType::Any, false)                 \
  m(issetPropCS,   KeyType::Str, false)                 \
  m(issetPropCE,   KeyType::Any, true)                  \
  m(issetPropCES,  KeyType::Str, true)                  \

#define X(nm, kt, useEmpty)                                             \
/* This returns uint64_t to ensure all 64 bits of rax are valid. */     \
inline uint64_t nm(Class* ctx, tv_lval base, key_type<kt> key) {    \
  return HPHP::IssetEmptyProp<useEmpty, kt>(ctx, base, key);            \
}
ISSET_EMPTY_PROP_HELPER_TABLE(X)
#undef X

#define ISSET_EMPTY_OBJ_PROP_HELPER_TABLE(m)       \
  /* name          keyType       useEmpty */       \
  m(issetPropCEO,  KeyType::Any, true)             \
  m(issetPropCEOS, KeyType::Str, true)             \
  m(issetPropCO,   KeyType::Any, false)            \
  m(issetPropCOS,  KeyType::Str, false)            \

#define X(nm, kt, useEmpty)                                             \
/* This returns uint64_t to ensure all 64 bits of rax are valid. */     \
inline uint64_t nm(Class* ctx, ObjectData* base, key_type<kt> key) {    \
  return IssetEmptyPropObj<useEmpty, kt>(ctx, base, key);               \
}
ISSET_EMPTY_OBJ_PROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<bool checkForInt>
void profileMixedArrayOffsetHelper(const ArrayData* ad, int64_t i,
                                   ArrayOffsetProfile* prof) {
  prof->update(ad, i);
}
template<bool checkForInt>
void profileMixedArrayOffsetHelper(const ArrayData* ad, const StringData* sd,
                                   ArrayOffsetProfile* prof) {
  prof->update(ad, sd, checkForInt);
}

#define PROFILE_MIXED_ARRAY_OFFSET_HELPER_TABLE(m)          \
  /* name                       keyType     checkForInt */  \
  m(profileMixedArrayOffsetS,  KeyType::Str,   false)       \
  m(profileMixedArrayOffsetSi, KeyType::Str,    true)       \
  m(profileMixedArrayOffsetI,  KeyType::Int,   false)       \

#define X(nm, keyType, checkForInt)                     \
inline void nm(const ArrayData* a, key_type<keyType> k, \
               ArrayOffsetProfile* p) {                 \
  profileMixedArrayOffsetHelper<checkForInt>(a, k, p);  \
}
PROFILE_MIXED_ARRAY_OFFSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline void profileDictOffsetHelper(const ArrayData* ad, int64_t i,
                                    ArrayOffsetProfile* prof) {
  prof->update(ad, i);
}
inline void profileDictOffsetHelper(const ArrayData* ad, const StringData* sd,
                                    ArrayOffsetProfile* prof) {
  prof->update(ad, sd, false);
}

#define PROFILE_DICT_OFFSET_HELPER_TABLE(m)                 \
  /* name                keyType  */                        \
  m(profileDictOffsetS,  KeyType::Str)                      \
  m(profileDictOffsetI,  KeyType::Int)                      \

#define X(nm, keyType)                                      \
inline void nm(const ArrayData* a, key_type<keyType> k,     \
               ArrayOffsetProfile* p) {                     \
  profileDictOffsetHelper(a, k, p);                         \
}
PROFILE_DICT_OFFSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline void profileKeysetOffsetHelper(const ArrayData* ad, int64_t i,
                                      ArrayOffsetProfile* prof) {
  prof->update(ad, i);
}
inline void profileKeysetOffsetHelper(const ArrayData* ad, const StringData* sd,
                                      ArrayOffsetProfile* prof) {
  prof->update(ad, sd, false);
}

#define PROFILE_KEYSET_OFFSET_HELPER_TABLE(m)               \
  /* name                keyType  */                        \
  m(profileKeysetOffsetS,  KeyType::Str)                    \
  m(profileKeysetOffsetI,  KeyType::Int)                    \

#define X(nm, keyType)                                      \
inline void nm(const ArrayData* a, key_type<keyType> k,     \
               ArrayOffsetProfile* p) {                     \
  profileKeysetOffsetHelper(a, k, p);                       \
}
PROFILE_KEYSET_OFFSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, MOpMode mode, bool intishWarn>
tv_lval elemImpl(tv_lval base, key_type<keyType> key, TypedValue& tvRef) {
  static_assert(mode != MOpMode::Define, "");
  if (mode == MOpMode::Unset) {
    return ElemU<intishWarn, keyType>(tvRef, base, key);
  }
  // We won't really modify the TypedValue in the non-D case, so
  // this lval cast is safe.
  return Elem<mode, intishWarn, keyType>(tvRef, base, key).as_lval();
}

#define ELEM_HELPER_TABLE(m)                                   \
  /* name      keyType         mode             intishWarn */  \
  m(elemC,     KeyType::Any,   MOpMode::None,   false)         \
  m(elemCU,    KeyType::Any,   MOpMode::Unset,  false)         \
  m(elemCW,    KeyType::Any,   MOpMode::Warn,   false)         \
  m(elemCIO,   KeyType::Any,   MOpMode::InOut,  false)         \
  m(elemI,     KeyType::Int,   MOpMode::None,   false)         \
  m(elemIU,    KeyType::Int,   MOpMode::Unset,  false)         \
  m(elemIW,    KeyType::Int,   MOpMode::Warn,   false)         \
  m(elemIIO,   KeyType::Int,   MOpMode::InOut,  false)         \
  m(elemS,     KeyType::Str,   MOpMode::None,   false)         \
  m(elemSU,    KeyType::Str,   MOpMode::Unset,  false)         \
  m(elemSW,    KeyType::Str,   MOpMode::Warn,   false)         \
  m(elemSIO,   KeyType::Str,   MOpMode::InOut,  false)         \
  m(elemC_W,   KeyType::Any,   MOpMode::None,   true)          \
  m(elemCU_W,  KeyType::Any,   MOpMode::Unset,  true)          \
  m(elemCW_W,  KeyType::Any,   MOpMode::Warn,   true)          \
  m(elemCIO_W, KeyType::Any,   MOpMode::InOut,  true)          \
  m(elemI_W,   KeyType::Int,   MOpMode::None,   true)          \
  m(elemIU_W,  KeyType::Int,   MOpMode::Unset,  true)          \
  m(elemIW_W,  KeyType::Int,   MOpMode::Warn,   true)          \
  m(elemIIO_W, KeyType::Int,   MOpMode::InOut,  true)          \
  m(elemS_W,   KeyType::Str,   MOpMode::None,   true)          \
  m(elemSU_W,  KeyType::Str,   MOpMode::Unset,  true)          \
  m(elemSW_W,  KeyType::Str,   MOpMode::Warn,   true)          \
  m(elemSIO_W, KeyType::Str,   MOpMode::InOut,  true)          \

#define X(nm, keyType, mode, intishWarn)                      \
inline tv_lval nm(tv_lval base,                               \
                  key_type<keyType> key,                      \
                  TypedValue& tvRef) {                        \
  return elemImpl<keyType,mode,intishWarn>(base, key, tvRef); \
}
ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEMD_HELPER_TABLE(m)                 \
  /* name      keyType         intishWarn */  \
  m(elemCD,    KeyType::Any,   false)         \
  m(elemID,    KeyType::Int,   false)         \
  m(elemSD,    KeyType::Str,   false)         \
  m(elemCD_W,  KeyType::Any,   true)          \
  m(elemID_W,  KeyType::Int,   true)          \
  m(elemSD_W,  KeyType::Str,   true)          \

#define X(nm, keyType, intishWarn)                            \
inline tv_lval nm(tv_lval base,                               \
                  key_type<keyType> key,                      \
                  TypedValue& tvRef,                          \
                  const MInstrPropState* pState) {            \
  return ElemD<MOpMode::Define, false, intishWarn, keyType>(  \
    tvRef, base, key, pState                                  \
  );                                                          \
}
ELEMD_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<bool warn, bool intishWarn>
inline tv_rval checkedGet(ArrayData* a, StringData* key) {
  int64_t i;
  assertx(a->isPHPArray());
  if (UNLIKELY(key->isStrictlyInteger(i))) {
    if (intishWarn) raise_intish_index_cast();
    return warn ? a->rvalStrict(i) : a->rval(i);
  } else {
    return warn ? a->rvalStrict(key) : a->rval(key);
  }
}

template <bool warn, bool intishWarn>
inline tv_rval checkedGet(ArrayData* /*a*/, int64_t /*key*/) {
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<MOpMode mode>
NEVER_INLINE
tv_rval elemArrayNotFound(int64_t k) {
  if (mode == MOpMode::Warn) {
    raise_notice("Undefined index: %" PRId64, k);
  }
  if (mode == MOpMode::InOut) {
    raise_inout_undefined_index(k);
  }
  return &immutable_uninit_base;
}

template<MOpMode mode>
NEVER_INLINE
tv_rval elemArrayNotFound(const StringData* k) {
  if (mode == MOpMode::Warn) {
    raise_notice("Undefined index: %s", k->data());
  }
  if (mode == MOpMode::InOut) {
    raise_inout_undefined_index(k);
  }
  return &immutable_uninit_base;
}

template<KeyType keyType, bool checkForInt, MOpMode mode, bool intishWarn>
inline tv_rval elemArrayImpl(ArrayData* ad, key_type<keyType> key) {
  auto constexpr warn = mode == MOpMode::Warn;
  auto const ret = checkForInt ?
    checkedGet<warn, intishWarn>(ad, key) :
    (warn ? ad->rvalStrict(key) : ad->rval(key));
  if (!ret) return elemArrayNotFound<mode>(key);
  return ret;
}

#define ELEM_ARRAY_D_HELPER_TABLE(m)              \
  /* name                keyType   intishWarn */  \
  m(elemArraySD,    KeyType::Str,  false)         \
  m(elemArrayID,    KeyType::Int,  false)         \
  m(elemArraySDW,   KeyType::Str,  true)          \
  m(elemArrayIDW,   KeyType::Int,  true)          \

#define X(nm, keyType, intishWarn)                                          \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {                    \
  auto cbase = tvToCell(base);                                              \
  assertx(isArrayType(type(cbase)));                                        \
  return ElemDArray<MOpMode::None, false, intishWarn, keyType>(cbase, key); \
}
ELEM_ARRAY_D_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_U_HELPER_TABLE(m)            \
  /* name         keyType        intishWarn */  \
  m(elemArraySU,  KeyType::Str,  false)         \
  m(elemArrayIU,  KeyType::Int,  false)         \
  m(elemArraySUW, KeyType::Str,  true)          \
  m(elemArrayIUW, KeyType::Int,  true)          \

#define X(nm, keyType, intishWarn)                                     \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {               \
  auto cbase = tvToCell(base);                                         \
  assertx(isArrayType(type(cbase)));                                   \
  return ElemUArray<intishWarn, keyType>(cbase, key);                  \
}
ELEM_ARRAY_U_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_HELPER_TABLE(m)                                             \
  /* name             keyType       checkForInt  mode            intishWarn */ \
  m(elemArrayS,       KeyType::Str, false,       MOpMode::None,  false)        \
  m(elemArraySi,      KeyType::Str, true,        MOpMode::None,  false)        \
  m(elemArrayI,       KeyType::Int, false,       MOpMode::None,  false)        \
  m(elemArraySW,      KeyType::Str, false,       MOpMode::Warn,  false)        \
  m(elemArraySiW,     KeyType::Str, true,        MOpMode::Warn,  false)        \
  m(elemArrayIW,      KeyType::Int, false,       MOpMode::Warn,  false)        \
  m(elemArraySW_IO,   KeyType::Str, false,       MOpMode::InOut, false)        \
  m(elemArraySiW_IO,  KeyType::Str, true,        MOpMode::InOut, false)        \
  m(elemArrayIW_IO,   KeyType::Int, false,       MOpMode::InOut, false)        \
  m(elemArrayS_W,     KeyType::Str, false,       MOpMode::None,  true)         \
  m(elemArraySi_W,    KeyType::Str, true,        MOpMode::None,  true)         \
  m(elemArrayI_W,     KeyType::Int, false,       MOpMode::None,  true)         \
  m(elemArraySW_W,    KeyType::Str, false,       MOpMode::Warn,  true)         \
  m(elemArraySiW_W,   KeyType::Str, true,        MOpMode::Warn,  true)         \
  m(elemArrayIW_W,    KeyType::Int, false,       MOpMode::Warn,  true)         \
  m(elemArraySW_WIO,  KeyType::Str, false,       MOpMode::InOut, true)         \
  m(elemArraySiW_WIO, KeyType::Str, true,        MOpMode::InOut, true)         \
  m(elemArrayIW_WIO,  KeyType::Int, false,       MOpMode::InOut, true)         \

#define X(nm, keyType, checkForInt, mode, intishWarn)                     \
inline tv_rval nm(ArrayData* ad, key_type<keyType> key) {       \
  return elemArrayImpl<keyType, checkForInt, mode, intishWarn>(ad, key);  \
}
ELEM_ARRAY_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// Keep these error handlers in sync with ArrayData::getNotFound();
TypedValue arrayGetNotFound(int64_t k);
TypedValue arrayGetNotFound(const StringData* k);

template<KeyType keyType, bool checkForInt, MOpMode mode, bool intishWarn>
TypedValue arrayGetImpl(ArrayData* a, key_type<keyType> key) {
  auto ret = checkForInt
    ? checkedGet<true, intishWarn>(a, key)
    : a->rvalStrict(key);
  if (ret) return ret.tv();
  if (mode == MOpMode::None) return make_tv<KindOfNull>();
  if (mode == MOpMode::InOut) {
    raise_inout_undefined_index(key);
    return make_tv<KindOfNull>();
  }
  assertx(mode == MOpMode::Warn);
  return arrayGetNotFound(key);
}

#define ARRAYGET_HELPER_TABLE(m)                                              \
  /* name           keyType        checkForInt mode            intishWarn */  \
  m(arrayGetS,      KeyType::Str,  false,      MOpMode::None,  false)         \
  m(arrayGetSi,     KeyType::Str,  true,       MOpMode::None,  false)         \
  m(arrayGetI,      KeyType::Int,  false,      MOpMode::None,  false)         \
  m(arrayGetSW,     KeyType::Str,  false,      MOpMode::None,  true)          \
  m(arrayGetSiW,    KeyType::Str,  true,       MOpMode::None,  true)          \
  m(arrayGetIW,     KeyType::Int,  false,      MOpMode::None,  true)          \
  m(arrayGetS_W,    KeyType::Str,  false,      MOpMode::Warn,  false)         \
  m(arrayGetSi_W,   KeyType::Str,  true,       MOpMode::Warn,  false)         \
  m(arrayGetI_W,    KeyType::Int,  false,      MOpMode::Warn,  false)         \
  m(arrayGetSW_W,   KeyType::Str,  false,      MOpMode::Warn,  true)          \
  m(arrayGetSiW_W,  KeyType::Str,  true,       MOpMode::Warn,  true)          \
  m(arrayGetIW_W,   KeyType::Int,  false,      MOpMode::Warn,  true)          \
  m(arrayGetS_IO,   KeyType::Str,  false,      MOpMode::InOut, false)         \
  m(arrayGetSi_IO,  KeyType::Str,  true,       MOpMode::InOut, false)         \
  m(arrayGetI_IO,   KeyType::Int,  false,      MOpMode::InOut, false)         \
  m(arrayGetSW_IO,  KeyType::Str,  false,      MOpMode::InOut, true)          \
  m(arrayGetSiW_IO, KeyType::Str,  true,       MOpMode::InOut, true)          \
  m(arrayGetIW_IO,  KeyType::Int,  false,      MOpMode::InOut, true)          \

#define X(nm, keyType, checkForInt, mode, intishWarn)                   \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) {             \
  return arrayGetImpl<keyType, checkForInt, mode, intishWarn>(a, key);  \
}
ARRAYGET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEM_DICT_D_HELPER_TABLE(m)  \
  /* name           keyType */       \
  m(elemDictSD,    KeyType::Str)     \
  m(elemDictID,    KeyType::Int)     \

#define X(nm, keyType)                                                 \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {               \
  auto cbase = tvToCell(base);                                         \
  assertx(isDictType(type(cbase)));                                    \
  return ElemDDict<false, keyType>(cbase, key);                        \
}
ELEM_DICT_D_HELPER_TABLE(X)
#undef X

#define ELEM_DICT_U_HELPER_TABLE(m)  \
  /* name        keyType */          \
  m(elemDictSU, KeyType::Str)        \
  m(elemDictIU, KeyType::Int)        \

#define X(nm, keyType)                                                 \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {               \
  auto cbase = tvToCell(base);                                         \
  assertx(isDictType(type(cbase)));                                    \
  return ElemUDict<keyType>(cbase, key);                               \
}
ELEM_DICT_U_HELPER_TABLE(X)
#undef X

#define ELEM_DICT_HELPER_TABLE(m)                               \
  /* name               keyType        mode */                  \
  m(elemDictS,     KeyType::Str,       MOpMode::None)          \
  m(elemDictI,     KeyType::Int,       MOpMode::None)          \
  m(elemDictSW,    KeyType::Str,       MOpMode::Warn)          \
  m(elemDictIW,    KeyType::Int,       MOpMode::Warn)          \
  m(elemDictSIO,   KeyType::Str,       MOpMode::InOut)         \
  m(elemDictIIO,   KeyType::Int,       MOpMode::InOut)         \

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
  auto cbase = tvToCell(base);                                         \
  assertx(isKeysetType(type(cbase)));                                  \
  return ElemUKeyset<keyType>(cbase, key);                             \
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

template <KeyType keyType, MOpMode mode, bool intishWarn>
TypedValue cGetElemImpl(tv_lval base, key_type<keyType> key) {
  TypedValue localTvRef;
  auto result = Elem<mode, intishWarn, keyType>(localTvRef, base, key);
  return cGetRefShuffle(localTvRef, result);
}

#define CGETELEM_HELPER_TABLE(m)                                    \
  /* name            key           mode           intishWarn */     \
  m(cGetElemCQuiet,  KeyType::Any, MOpMode::None,  false)           \
  m(cGetElemIQuiet,  KeyType::Int, MOpMode::None,  false)           \
  m(cGetElemSQuiet,  KeyType::Str, MOpMode::None,  false)           \
  m(cGetElemC,       KeyType::Any, MOpMode::Warn,  false)           \
  m(cGetElemI,       KeyType::Int, MOpMode::Warn,  false)           \
  m(cGetElemS,       KeyType::Str, MOpMode::Warn,  false)           \
  m(cGetElemCIO,     KeyType::Any, MOpMode::InOut, false)           \
  m(cGetElemIIO,     KeyType::Int, MOpMode::InOut, false)           \
  m(cGetElemSIO,     KeyType::Str, MOpMode::InOut, false)           \
  m(cGetElemCQuietW, KeyType::Any, MOpMode::None,  true)            \
  m(cGetElemIQuietW, KeyType::Int, MOpMode::None,  true)            \
  m(cGetElemSQuietW, KeyType::Str, MOpMode::None,  true)            \
  m(cGetElemCW,      KeyType::Any, MOpMode::Warn,  true)            \
  m(cGetElemIW,      KeyType::Int, MOpMode::Warn,  true)            \
  m(cGetElemSW,      KeyType::Str, MOpMode::Warn,  true)            \
  m(cGetElemCIOW,    KeyType::Any, MOpMode::InOut, true)            \
  m(cGetElemIIOW,    KeyType::Int, MOpMode::InOut, true)            \
  m(cGetElemSIOW,    KeyType::Str, MOpMode::InOut, true)            \

#define X(nm, kt, mode, intishWarn)                                     \
inline TypedValue nm(tv_lval base, key_type<kt> key) {              \
  return cGetElemImpl<kt, mode, intishWarn>(base, key);                 \
}
CGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, bool intishWarn>
RefData* vGetElemImpl(tv_lval base, key_type<keyType> key,
                      const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto result = ElemD<MOpMode::Define, true, intishWarn, keyType>(
    localTvRef, base, key, pState
  );
  return vGetRefShuffle(localTvRef, result);
}

#define VGETELEM_HELPER_TABLE(m)                \
  /* name         keyType        intishWarn */  \
  m(vGetElemC,    KeyType::Any,  false)         \
  m(vGetElemI,    KeyType::Int,  false)         \
  m(vGetElemS,    KeyType::Str,  false)         \
  m(vGetElemCW,   KeyType::Any,  true)          \
  m(vGetElemIW,   KeyType::Int,  true)          \
  m(vGetElemSW,   KeyType::Str,  true)

#define X(nm, kt, intishWarn)                                           \
inline RefData* nm(tv_lval base, key_type<kt> key,                      \
                   const MInstrPropState* pState) {                     \
  return vGetElemImpl<kt, intishWarn>(base, key, pState);               \
}
VGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool intishWarn>
inline ArrayData* checkedSet(ArrayData* a,
                             StringData* key,
                             Cell value,
                             bool copy) {
  int64_t i;
  assertx(a->isPHPArray());
  if (UNLIKELY(key->isStrictlyInteger(i))) {
    if (intishWarn) raise_intish_index_cast();
    return a->set(i, value, copy);
  } else {
    return a->set(key, value, copy);
  }
}

template <bool intishWarn>
inline ArrayData* checkedSet(ArrayData*, int64_t, Cell, bool) {
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<KeyType keyType, bool checkForInt, bool setRef, bool intishWarn>
auto
arraySetImpl(ArrayData* a, key_type<keyType> key, Cell value, TypedValue* ref) {
  static_assert(keyType != KeyType::Any,
                "KeyType::Any is not supported in arraySetMImpl");
  assertx(cellIsPlausible(value));
  assertx(a->isPHPArray());
  const bool copy = a->cowCheck();
  ArrayData* ret = checkForInt ? checkedSet<intishWarn>(a, key, value, copy)
                               : a->set(key, value, copy);
  return arrayRefShuffle<setRef, KindOfArray>(a, ret, ref);
}

#define ARRAYSET_HELPER_TABLE(m)                        \
  /* name         keyType     checkForInt intishWarn */ \
  m(arraySetS,    KeyType::Str,   false,  false)        \
  m(arraySetSi,   KeyType::Str,    true,  false)        \
  m(arraySetI,    KeyType::Int,   false,  false)        \
  m(arraySetSW,   KeyType::Str,   false,  true)         \
  m(arraySetSiW,  KeyType::Str,    true,  true)         \
  m(arraySetIW,   KeyType::Int,   false,  true)

#define X(nm, keyType, checkForInt, intishWarn)                         \
inline ArrayData* nm(ArrayData* a, key_type<keyType> key, Cell value) { \
  return arraySetImpl<keyType, checkForInt, false, intishWarn>(         \
    a, key, value, nullptr                                              \
  );                                                                    \
}
ARRAYSET_HELPER_TABLE(X)
#undef X

#define ARRAYSET_REF_HELPER_TABLE(m)                     \
  /* name         keyType     checkForInt intishWarn */  \
  m(arraySetSR,   KeyType::Str,   false,   false)        \
  m(arraySetSiR,  KeyType::Str,    true,   false)        \
  m(arraySetIR,   KeyType::Int,   false,   false)        \
  m(arraySetSRW,  KeyType::Str,   false,   true)         \
  m(arraySetSiRW, KeyType::Str,    true,   true)         \
  m(arraySetIRW,  KeyType::Int,   false,   true)

#define X(nm, keyType, checkForInt, intishWarn)                          \
inline                                                                   \
void nm(ArrayData* a, key_type<keyType> key, Cell value, RefData* ref) { \
  arraySetImpl<keyType, checkForInt, true, intishWarn>(                  \
    a, key, value, ref->tv()                                             \
  );                                                                     \
}
ARRAYSET_REF_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<bool setRef>
auto vecSetImpl(ArrayData* a, int64_t key, Cell value, TypedValue* ref) {
  assertx(cellIsPlausible(value));
  assertx(a->isVecArray());
  const bool copy = a->cowCheck();
  ArrayData* ret = PackedArray::SetIntVec(a, key, value, copy);
  return arrayRefShuffle<setRef, KindOfVec>(a, ret, ref);
}

inline ArrayData* vecSetI(ArrayData* a, int64_t key, Cell value) {
  return vecSetImpl<false>(a, key, value, nullptr);
}

inline void vecSetIR(ArrayData* a, int64_t key, Cell value,
                     RefData* ref) {
  vecSetImpl<true>(a, key, value, ref->tv());
}

//////////////////////////////////////////////////////////////////////

inline ArrayData* dictSetImplPre(ArrayData* a, int64_t i, Cell val) {
  return MixedArray::SetIntDict(a, i, val, a->cowCheck());
}
inline ArrayData* dictSetImplPre(ArrayData* a, StringData* s, Cell val) {
  return MixedArray::SetStrDict(a, s, val, a->cowCheck());
}

template<KeyType keyType, bool setRef>
auto
dictSetImpl(ArrayData* a, key_type<keyType> key, Cell value, TypedValue* ref) {
  assertx(cellIsPlausible(value));
  assertx(a->isDict());
  auto ret = dictSetImplPre(a, key, value);
  return arrayRefShuffle<setRef, KindOfDict>(a, ret, ref);
}

#define DICTSET_HELPER_TABLE(m)    \
  /* name       keyType     */     \
  m(dictSetI,   KeyType::Int)      \
  m(dictSetS,   KeyType::Str)

#define X(nm, keyType)                                                   \
inline ArrayData* nm(ArrayData* a, key_type<keyType> key, Cell val) {    \
  return dictSetImpl<keyType, false>(a, key, val, nullptr);              \
}
DICTSET_HELPER_TABLE(X)
#undef X

#define DICTSET_REF_HELPER_TABLE(m) \
  /* name       keyType    */       \
  m(dictSetIR,  KeyType::Int)       \
  m(dictSetSR,  KeyType::Str)

#define X(nm, keyType)                                                  \
inline                                                                  \
void nm(ArrayData* a, key_type<keyType> key, Cell val, RefData* ref) {  \
  dictSetImpl<keyType, true>(a, key, val, ref->tv());                   \
}
DICTSET_REF_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline ArrayData* keysetSetNewElemImplPre(ArrayData* a, int64_t i) {
  return SetArray::AddToSet(a, i, a->cowCheck());
}

inline ArrayData* keysetSetNewElemImplPre(ArrayData* a, StringData* s) {
  return SetArray::AddToSet(a, s, a->cowCheck());
}

template<KeyType keyType>
void keysetSetNewElemImpl(tv_lval base, key_type<keyType> key) {
  base = tvToCell(base);
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
template <KeyType keyType, bool intishWarn>
StringData* setElemImpl(tv_lval base, key_type<keyType> key,
                        Cell val, const MInstrPropState* pState) {
  return HPHP::SetElem<false, intishWarn, keyType>(base, key, &val, pState);
}

#define SETELEM_HELPER_TABLE(m)                 \
  /* name       keyType       intishWarn */     \
  m(setElemC,   KeyType::Any, false)            \
  m(setElemI,   KeyType::Int, false)            \
  m(setElemS,   KeyType::Str, false)            \
  m(setElemCW,  KeyType::Any, true)             \
  m(setElemIW,  KeyType::Int, true)             \
  m(setElemSW,  KeyType::Str, true)             \

#define X(nm, kt, intishWarn)                                           \
inline StringData* nm(tv_lval base, key_type<kt> key,                   \
                      Cell val, const MInstrPropState* pState) {        \
  return setElemImpl<kt, intishWarn>(base, key, val, pState);           \
}
SETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////


template<KeyType keyType, bool checkForInt, bool intishWarn>
uint64_t arrayIssetImpl(ArrayData* a, key_type<keyType> key) {
  auto const rval = checkForInt
    ? checkedGet<false, intishWarn>(a, key)
    : a->rval(key);
  return !rval
    ? 0
    : !isNullType(rval.unboxed().type());
}

#define ARRAY_ISSET_HELPER_TABLE(m)                             \
  /* name           keyType         checkForInt  intishWarn */  \
  m(arrayIssetS,    KeyType::Str,   false,       false)         \
  m(arrayIssetSi,   KeyType::Str,    true,       false)         \
  m(arrayIssetI,    KeyType::Int,   false,       false)         \
  m(arrayIssetSW,   KeyType::Str,   false,       true)          \
  m(arrayIssetSiW,  KeyType::Str,    true,       true)          \
  m(arrayIssetIW,   KeyType::Int,   false,       true)          \

#define X(nm, keyType, checkForInt, intishWarn)                     \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) {           \
  return arrayIssetImpl<keyType, checkForInt, intishWarn>(a, key);  \
}
ARRAY_ISSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<KeyType keyType, bool isEmpty>
uint64_t dictIssetImpl(ArrayData* a, key_type<keyType> key) {
  return IssetEmptyElemDict<isEmpty, keyType>(a, key);
}

#define DICT_ISSET_EMPTY_ELEM_HELPER_TABLE(m)         \
  /* name              keyType      isEmpty */        \
  m(dictIssetElemS,    KeyType::Str,  false)          \
  m(dictIssetElemSE,   KeyType::Str,  true)           \
  m(dictIssetElemI,    KeyType::Int,  false)          \
  m(dictIssetElemIE,   KeyType::Int,  true)           \

#define X(nm, keyType, isEmpty)                               \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) {     \
  return dictIssetImpl<keyType, isEmpty>(a, key);             \
}
DICT_ISSET_EMPTY_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<KeyType keyType, bool isEmpty>
uint64_t keysetIssetImpl(ArrayData* a, key_type<keyType> key) {
  return IssetEmptyElemKeyset<isEmpty, keyType>(a, key);
}

#define KEYSET_ISSET_EMPTY_ELEM_HELPER_TABLE(m)         \
  /* name                keyType      isEmpty */        \
  m(keysetIssetElemS,    KeyType::Str,  false)          \
  m(keysetIssetElemSE,   KeyType::Str,  true)           \
  m(keysetIssetElemI,    KeyType::Int,  false)          \
  m(keysetIssetElemIE,   KeyType::Int,  true)           \

#define X(nm, keyType, isEmpty)                               \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) {     \
  return keysetIssetImpl<keyType, isEmpty>(a, key);           \
}
KEYSET_ISSET_EMPTY_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<KeyType keyType, bool intishWarn>
void unsetElemImpl(tv_lval base, key_type<keyType> key) {
  HPHP::UnsetElem<intishWarn, keyType>(base, key);
}

#define UNSET_ELEM_HELPER_TABLE(m)              \
  /* name         keyType       intishWarn */   \
  m(unsetElemC,   KeyType::Any, false)          \
  m(unsetElemI,   KeyType::Int, false)          \
  m(unsetElemS,   KeyType::Str, false)          \
  m(unsetElemCW,  KeyType::Any, true)           \
  m(unsetElemIW,  KeyType::Int, true)           \
  m(unsetElemSW,  KeyType::Str, true)           \

#define X(nm, kt, intishWarn)                         \
inline void nm(tv_lval base, key_type<kt> key) {  \
  unsetElemImpl<kt, intishWarn>(base, key);           \
}
UNSET_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////
template <KeyType keyType, bool isEmpty, bool intishWarn>
bool issetEmptyElemImpl(tv_lval base, key_type<keyType> key) {
  return HPHP::IssetEmptyElem<isEmpty, intishWarn, keyType>(base, key);
}

#define ISSET_EMPTY_ELEM_HELPER_TABLE(m)                  \
  /* name         keyType       isEmpty  intishWarn */    \
  m(issetElemC,   KeyType::Any, false,   false)           \
  m(issetElemCE,  KeyType::Any,  true,   false)           \
  m(issetElemI,   KeyType::Int, false,   false)           \
  m(issetElemIE,  KeyType::Int,  true,   false)           \
  m(issetElemS,   KeyType::Str, false,   false)           \
  m(issetElemSE,  KeyType::Str,  true,   false)           \
  m(issetElemCW,  KeyType::Any, false,   true)            \
  m(issetElemCEW, KeyType::Any,  true,   true)            \
  m(issetElemIW,  KeyType::Int, false,   true)            \
  m(issetElemIEW, KeyType::Int,  true,   true)            \
  m(issetElemSW,  KeyType::Str, false,   true)            \
  m(issetElemSEW, KeyType::Str,  true,   true)            \

#define X(nm, kt, isEmpty, intishWarn)                               \
inline uint64_t nm(tv_lval base, key_type<kt> key) {             \
  return issetEmptyElemImpl<kt, isEmpty, intishWarn>(base, key);     \
}
ISSET_EMPTY_ELEM_HELPER_TABLE(X)
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
  return result ? !cellIsNull(result) : false;
}

template<KeyType keyType>
void mapSetImpl(c_Map* map, key_type<keyType> key, Cell value) {
  // XXX: we should call this directly from the TC.
  map->set(key, value);
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
