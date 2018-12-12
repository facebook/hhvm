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
      auto inner = *localTvRef.m_data.pref->cell();
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

template <KeyType keyType, MOpMode mode, ICMode intishCast>
tv_lval elemImpl(tv_lval base, key_type<keyType> key, TypedValue& tvRef) {
  static_assert(mode != MOpMode::Define, "");
  if (mode == MOpMode::Unset) {
    return ElemU<intishCast, keyType>(tvRef, base, key);
  }
  // We won't really modify the TypedValue in the non-D case, so
  // this lval cast is safe.
  return Elem<mode, intishCast, keyType>(tvRef, base, key).as_lval();
}

#define ELEM_HELPER_TABLE(m)                                    \
  /* name      keyType         mode             intishCast */   \
  m(elemC,     KeyType::Any,   MOpMode::None,   ICMode::Ignore) \
  m(elemCU,    KeyType::Any,   MOpMode::Unset,  ICMode::Ignore) \
  m(elemCW,    KeyType::Any,   MOpMode::Warn,   ICMode::Ignore) \
  m(elemCIO,   KeyType::Any,   MOpMode::InOut,  ICMode::Ignore) \
  m(elemI,     KeyType::Int,   MOpMode::None,   ICMode::Ignore) \
  m(elemIU,    KeyType::Int,   MOpMode::Unset,  ICMode::Ignore) \
  m(elemIW,    KeyType::Int,   MOpMode::Warn,   ICMode::Ignore) \
  m(elemIIO,   KeyType::Int,   MOpMode::InOut,  ICMode::Ignore) \
  m(elemS,     KeyType::Str,   MOpMode::None,   ICMode::Ignore) \
  m(elemSU,    KeyType::Str,   MOpMode::Unset,  ICMode::Ignore) \
  m(elemSW,    KeyType::Str,   MOpMode::Warn,   ICMode::Ignore) \
  m(elemSIO,   KeyType::Str,   MOpMode::InOut,  ICMode::Ignore) \
  m(elemC_C,   KeyType::Any,   MOpMode::None,   ICMode::Cast)   \
  m(elemCU_C,  KeyType::Any,   MOpMode::Unset,  ICMode::Cast)   \
  m(elemCW_C,  KeyType::Any,   MOpMode::Warn,   ICMode::Cast)   \
  m(elemCIO_C, KeyType::Any,   MOpMode::InOut,  ICMode::Cast)   \
  m(elemI_C,   KeyType::Int,   MOpMode::None,   ICMode::Cast)   \
  m(elemIU_C,  KeyType::Int,   MOpMode::Unset,  ICMode::Cast)   \
  m(elemIW_C,  KeyType::Int,   MOpMode::Warn,   ICMode::Cast)   \
  m(elemIIO_C, KeyType::Int,   MOpMode::InOut,  ICMode::Cast)   \
  m(elemS_C,   KeyType::Str,   MOpMode::None,   ICMode::Cast)   \
  m(elemSU_C,  KeyType::Str,   MOpMode::Unset,  ICMode::Cast)   \
  m(elemSW_C,  KeyType::Str,   MOpMode::Warn,   ICMode::Cast)   \
  m(elemSIO_C, KeyType::Str,   MOpMode::InOut,  ICMode::Cast)   \
  m(elemC_W,   KeyType::Any,   MOpMode::None,   ICMode::Warn)   \
  m(elemCU_W,  KeyType::Any,   MOpMode::Unset,  ICMode::Warn)   \
  m(elemCW_W,  KeyType::Any,   MOpMode::Warn,   ICMode::Warn)   \
  m(elemCIO_W, KeyType::Any,   MOpMode::InOut,  ICMode::Warn)   \
  m(elemI_W,   KeyType::Int,   MOpMode::None,   ICMode::Warn)   \
  m(elemIU_W,  KeyType::Int,   MOpMode::Unset,  ICMode::Warn)   \
  m(elemIW_W,  KeyType::Int,   MOpMode::Warn,   ICMode::Warn)   \
  m(elemIIO_W, KeyType::Int,   MOpMode::InOut,  ICMode::Warn)   \
  m(elemS_W,   KeyType::Str,   MOpMode::None,   ICMode::Warn)   \
  m(elemSU_W,  KeyType::Str,   MOpMode::Unset,  ICMode::Warn)   \
  m(elemSW_W,  KeyType::Str,   MOpMode::Warn,   ICMode::Warn)   \
  m(elemSIO_W, KeyType::Str,   MOpMode::InOut,  ICMode::Warn)   \

#define X(nm, keyType, mode, intishCast)                      \
inline tv_lval nm(tv_lval base,                               \
                  key_type<keyType> key,                      \
                  TypedValue& tvRef) {                        \
  return elemImpl<keyType,mode,intishCast>(base, key, tvRef); \
}
ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

#define ELEMD_HELPER_TABLE(m)                 \
  /* name      keyType         intishCast */  \
  m(elemCD,    KeyType::Any,   ICMode::Ignore) \
  m(elemID,    KeyType::Int,   ICMode::Ignore) \
  m(elemSD,    KeyType::Str,   ICMode::Ignore) \
  m(elemCD_C,  KeyType::Any,   ICMode::Cast)   \
  m(elemID_C,  KeyType::Int,   ICMode::Cast)   \
  m(elemSD_C,  KeyType::Str,   ICMode::Cast)   \
  m(elemCD_W,  KeyType::Any,   ICMode::Warn)   \
  m(elemID_W,  KeyType::Int,   ICMode::Warn)   \
  m(elemSD_W,  KeyType::Str,   ICMode::Warn)   \

#define X(nm, keyType, intishCast)                            \
inline tv_lval nm(tv_lval base,                               \
                  key_type<keyType> key,                      \
                  TypedValue& tvRef,                          \
                  const MInstrPropState* pState) {            \
  return ElemD<MOpMode::Define, false, intishCast, keyType>(  \
    tvRef, base, key, pState                                  \
  );                                                          \
}
ELEMD_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<bool warn, ICMode intishCast>
inline tv_rval checkedGet(ArrayData* a, StringData* key) {
  assertx(a->isPHPArray());
  if (auto const intish = tryIntishCast<intishCast>(key)) {
    return warn ? a->rvalStrict(*intish) : a->rval(*intish);
  } else {
    return warn ? a->rvalStrict(key) : a->rval(key);
  }
}

template <bool warn, ICMode intishCast>
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

template<KeyType keyType, bool checkForInt, MOpMode mode, ICMode intishCast>
inline tv_rval elemArrayImpl(ArrayData* ad, key_type<keyType> key) {
  auto constexpr warn = mode == MOpMode::Warn;
  auto const ret = checkForInt ?
    checkedGet<warn, intishCast>(ad, key) :
    (warn ? ad->rvalStrict(key) : ad->rval(key));
  if (!ret) return elemArrayNotFound<mode>(key);
  return ret;
}

#define ELEM_ARRAY_D_HELPER_TABLE(m)               \
  /* name                keyType   intishCast */   \
  m(elemArraySD,    KeyType::Str,  ICMode::Ignore) \
  m(elemArrayID,    KeyType::Int,  ICMode::Ignore) \
  m(elemArraySDC,   KeyType::Str,  ICMode::Cast)   \
  m(elemArrayIDC,   KeyType::Int,  ICMode::Cast)   \
  m(elemArraySDW,   KeyType::Str,  ICMode::Warn)   \
  m(elemArrayIDW,   KeyType::Int,  ICMode::Warn)   \

#define X(nm, keyType, intishCast)                                          \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {                    \
  auto cbase = tvToCell(base);                                              \
  assertx(isArrayType(type(cbase)));                                        \
  return ElemDArray<MOpMode::None, false, intishCast, keyType>(cbase, key); \
}
ELEM_ARRAY_D_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_U_HELPER_TABLE(m)            \
  /* name         keyType        intishCast */  \
  m(elemArraySU,  KeyType::Str,  ICMode::Ignore)         \
  m(elemArrayIU,  KeyType::Int,  ICMode::Ignore)         \
  m(elemArraySUC, KeyType::Str,  ICMode::Cast)          \
  m(elemArrayIUC, KeyType::Int,  ICMode::Cast)          \
  m(elemArraySUW, KeyType::Str,  ICMode::Warn)          \
  m(elemArrayIUW, KeyType::Int,  ICMode::Warn)          \

#define X(nm, keyType, intishCast)                                     \
inline tv_lval nm(tv_lval base, key_type<keyType> key) {               \
  auto cbase = tvToCell(base);                                         \
  assertx(isArrayType(type(cbase)));                                   \
  return ElemUArray<intishCast, keyType>(cbase, key);                  \
}
ELEM_ARRAY_U_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_HELPER_TABLE(m)                                             \
  /*                                 checkForInt                            */ \
  /* name             keyType       /            mode      intishCast   */ \
  m(elemArrayS,       KeyType::Str, false, MOpMode::None,  ICMode::Ignore) \
  m(elemArraySi,      KeyType::Str, true,  MOpMode::None,  ICMode::Ignore) \
  m(elemArrayI,       KeyType::Int, false, MOpMode::None,  ICMode::Ignore) \
  m(elemArraySW,      KeyType::Str, false, MOpMode::Warn,  ICMode::Ignore) \
  m(elemArraySiW,     KeyType::Str, true,  MOpMode::Warn,  ICMode::Ignore) \
  m(elemArrayIW,      KeyType::Int, false, MOpMode::Warn,  ICMode::Ignore) \
  m(elemArraySW_IO,   KeyType::Str, false, MOpMode::InOut, ICMode::Ignore) \
  m(elemArraySiW_IO,  KeyType::Str, true,  MOpMode::InOut, ICMode::Ignore) \
  m(elemArrayIW_IO,   KeyType::Int, false, MOpMode::InOut, ICMode::Ignore) \
  m(elemArrayS_C,     KeyType::Str, false, MOpMode::None,  ICMode::Cast)   \
  m(elemArraySi_C,    KeyType::Str, true,  MOpMode::None,  ICMode::Cast)   \
  m(elemArrayI_C,     KeyType::Int, false, MOpMode::None,  ICMode::Cast)   \
  m(elemArraySW_C,    KeyType::Str, false, MOpMode::Warn,  ICMode::Cast)   \
  m(elemArraySiW_C,   KeyType::Str, true,  MOpMode::Warn,  ICMode::Cast)   \
  m(elemArrayIW_C,    KeyType::Int, false, MOpMode::Warn,  ICMode::Cast)   \
  m(elemArraySW_CIO,  KeyType::Str, false, MOpMode::InOut, ICMode::Cast)   \
  m(elemArraySiW_CIO, KeyType::Str, true,  MOpMode::InOut, ICMode::Cast)   \
  m(elemArrayIW_CIO,  KeyType::Int, false, MOpMode::InOut, ICMode::Cast)   \
  m(elemArrayS_W,     KeyType::Str, false, MOpMode::None,  ICMode::Warn)   \
  m(elemArraySi_W,    KeyType::Str, true,  MOpMode::None,  ICMode::Warn)   \
  m(elemArrayI_W,     KeyType::Int, false, MOpMode::None,  ICMode::Warn)   \
  m(elemArraySW_W,    KeyType::Str, false, MOpMode::Warn,  ICMode::Warn)   \
  m(elemArraySiW_W,   KeyType::Str, true,  MOpMode::Warn,  ICMode::Warn)   \
  m(elemArrayIW_W,    KeyType::Int, false, MOpMode::Warn,  ICMode::Warn)   \
  m(elemArraySW_WIO,  KeyType::Str, false, MOpMode::InOut, ICMode::Warn)   \
  m(elemArraySiW_WIO, KeyType::Str, true,  MOpMode::InOut, ICMode::Warn)   \
  m(elemArrayIW_WIO,  KeyType::Int, false, MOpMode::InOut, ICMode::Warn)   \

#define X(nm, keyType, checkForInt, mode, intishCast)                     \
inline tv_rval nm(ArrayData* ad, key_type<keyType> key) {       \
  return elemArrayImpl<keyType, checkForInt, mode, intishCast>(ad, key);  \
}
ELEM_ARRAY_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// Keep these error handlers in sync with ArrayData::getNotFound();
TypedValue arrayGetNotFound(int64_t k);
TypedValue arrayGetNotFound(const StringData* k);

template<KeyType keyType, bool checkForInt, MOpMode mode, ICMode intishCast>
TypedValue arrayGetImpl(ArrayData* a, key_type<keyType> key) {
  auto ret = checkForInt
    ? checkedGet<true, intishCast>(a, key)
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
  /* name           keyType        checkForInt mode            intishCast */  \
  m(arrayGetS,      KeyType::Str,  false,      MOpMode::None,  ICMode::Ignore) \
  m(arrayGetSi,     KeyType::Str,  true,       MOpMode::None,  ICMode::Ignore) \
  m(arrayGetI,      KeyType::Int,  false,      MOpMode::None,  ICMode::Ignore) \
  m(arrayGetSC,     KeyType::Str,  false,      MOpMode::None,  ICMode::Cast)   \
  m(arrayGetSiC,    KeyType::Str,  true,       MOpMode::None,  ICMode::Cast)   \
  m(arrayGetIC,     KeyType::Int,  false,      MOpMode::None,  ICMode::Cast)   \
  m(arrayGetSW,     KeyType::Str,  false,      MOpMode::None,  ICMode::Warn)   \
  m(arrayGetSiW,    KeyType::Str,  true,       MOpMode::None,  ICMode::Warn)   \
  m(arrayGetIW,     KeyType::Int,  false,      MOpMode::None,  ICMode::Warn)   \
  m(arrayGetS_W,    KeyType::Str,  false,      MOpMode::Warn,  ICMode::Ignore) \
  m(arrayGetSi_W,   KeyType::Str,  true,       MOpMode::Warn,  ICMode::Ignore) \
  m(arrayGetI_W,    KeyType::Int,  false,      MOpMode::Warn,  ICMode::Ignore) \
  m(arrayGetSC_W,   KeyType::Str,  false,      MOpMode::Warn,  ICMode::Cast)   \
  m(arrayGetSiC_W,  KeyType::Str,  true,       MOpMode::Warn,  ICMode::Cast)   \
  m(arrayGetIC_W,   KeyType::Int,  false,      MOpMode::Warn,  ICMode::Cast)   \
  m(arrayGetSW_W,   KeyType::Str,  false,      MOpMode::Warn,  ICMode::Warn)   \
  m(arrayGetSiW_W,  KeyType::Str,  true,       MOpMode::Warn,  ICMode::Warn)   \
  m(arrayGetIW_W,   KeyType::Int,  false,      MOpMode::Warn,  ICMode::Warn)   \
  m(arrayGetS_IO,   KeyType::Str,  false,      MOpMode::InOut, ICMode::Ignore) \
  m(arrayGetSi_IO,  KeyType::Str,  true,       MOpMode::InOut, ICMode::Ignore) \
  m(arrayGetI_IO,   KeyType::Int,  false,      MOpMode::InOut, ICMode::Ignore) \
  m(arrayGetSC_IO,  KeyType::Str,  false,      MOpMode::InOut, ICMode::Cast)   \
  m(arrayGetSiC_IO, KeyType::Str,  true,       MOpMode::InOut, ICMode::Cast)   \
  m(arrayGetIC_IO,  KeyType::Int,  false,      MOpMode::InOut, ICMode::Cast)   \
  m(arrayGetSW_IO,  KeyType::Str,  false,      MOpMode::InOut, ICMode::Warn)   \
  m(arrayGetSiW_IO, KeyType::Str,  true,       MOpMode::InOut, ICMode::Warn)   \
  m(arrayGetIW_IO,  KeyType::Int,  false,      MOpMode::InOut, ICMode::Warn)   \

#define X(nm, keyType, checkForInt, mode, intishCast)                   \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) {             \
  return arrayGetImpl<keyType, checkForInt, mode, intishCast>(a, key);  \
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

template <KeyType keyType, MOpMode mode, ICMode intishCast>
TypedValue cGetElemImpl(tv_lval base, key_type<keyType> key) {
  TypedValue localTvRef;
  auto result = Elem<mode, intishCast, keyType>(localTvRef, base, key);
  return cGetRefShuffle(localTvRef, result);
}

#define CGETELEM_HELPER_TABLE(m)                                    \
  /* name            key           mode           intishCast */     \
  m(cGetElemCQuiet,  KeyType::Any, MOpMode::None,  ICMode::Ignore)  \
  m(cGetElemIQuiet,  KeyType::Int, MOpMode::None,  ICMode::Ignore)  \
  m(cGetElemSQuiet,  KeyType::Str, MOpMode::None,  ICMode::Ignore)  \
  m(cGetElemC,       KeyType::Any, MOpMode::Warn,  ICMode::Ignore)  \
  m(cGetElemI,       KeyType::Int, MOpMode::Warn,  ICMode::Ignore)  \
  m(cGetElemS,       KeyType::Str, MOpMode::Warn,  ICMode::Ignore)  \
  m(cGetElemCIO,     KeyType::Any, MOpMode::InOut, ICMode::Ignore)  \
  m(cGetElemIIO,     KeyType::Int, MOpMode::InOut, ICMode::Ignore)  \
  m(cGetElemSIO,     KeyType::Str, MOpMode::InOut, ICMode::Ignore)  \
  m(cGetElemCQuietC, KeyType::Any, MOpMode::None,  ICMode::Cast)    \
  m(cGetElemIQuietC, KeyType::Int, MOpMode::None,  ICMode::Cast)    \
  m(cGetElemSQuietC, KeyType::Str, MOpMode::None,  ICMode::Cast)    \
  m(cGetElemCC,      KeyType::Any, MOpMode::Warn,  ICMode::Cast)    \
  m(cGetElemIC,      KeyType::Int, MOpMode::Warn,  ICMode::Cast)    \
  m(cGetElemSC,      KeyType::Str, MOpMode::Warn,  ICMode::Cast)    \
  m(cGetElemCIOC,    KeyType::Any, MOpMode::InOut, ICMode::Cast)    \
  m(cGetElemIIOC,    KeyType::Int, MOpMode::InOut, ICMode::Cast)    \
  m(cGetElemSIOC,    KeyType::Str, MOpMode::InOut, ICMode::Cast)    \
  m(cGetElemCQuietW, KeyType::Any, MOpMode::None,  ICMode::Warn)    \
  m(cGetElemIQuietW, KeyType::Int, MOpMode::None,  ICMode::Warn)    \
  m(cGetElemSQuietW, KeyType::Str, MOpMode::None,  ICMode::Warn)    \
  m(cGetElemCW,      KeyType::Any, MOpMode::Warn,  ICMode::Warn)    \
  m(cGetElemIW,      KeyType::Int, MOpMode::Warn,  ICMode::Warn)    \
  m(cGetElemSW,      KeyType::Str, MOpMode::Warn,  ICMode::Warn)    \
  m(cGetElemCIOW,    KeyType::Any, MOpMode::InOut, ICMode::Warn)    \
  m(cGetElemIIOW,    KeyType::Int, MOpMode::InOut, ICMode::Warn)    \
  m(cGetElemSIOW,    KeyType::Str, MOpMode::InOut, ICMode::Warn)    \

#define X(nm, kt, mode, intishCast)                                     \
inline TypedValue nm(tv_lval base, key_type<kt> key) {              \
  return cGetElemImpl<kt, mode, intishCast>(base, key);                 \
}
CGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, ICMode intishCast>
RefData* vGetElemImpl(tv_lval base, key_type<keyType> key,
                      const MInstrPropState* pState) {
  TypedValue localTvRef;
  auto result = ElemD<MOpMode::Define, true, intishCast, keyType>(
    localTvRef, base, key, pState
  );
  return vGetRefShuffle(localTvRef, result);
}

#define VGETELEM_HELPER_TABLE(m)                 \
  /* name         keyType        intishCast */   \
  m(vGetElemC,    KeyType::Any,  ICMode::Ignore) \
  m(vGetElemI,    KeyType::Int,  ICMode::Ignore) \
  m(vGetElemS,    KeyType::Str,  ICMode::Ignore) \
  m(vGetElemCC,   KeyType::Any,  ICMode::Cast)   \
  m(vGetElemIC,   KeyType::Int,  ICMode::Cast)   \
  m(vGetElemSC,   KeyType::Str,  ICMode::Cast)   \
  m(vGetElemCW,   KeyType::Any,  ICMode::Warn)   \
  m(vGetElemIW,   KeyType::Int,  ICMode::Warn)   \
  m(vGetElemSW,   KeyType::Str,  ICMode::Warn)   \

#define X(nm, kt, intishCast)                                           \
inline RefData* nm(tv_lval base, key_type<kt> key,                      \
                   const MInstrPropState* pState) {                     \
  return vGetElemImpl<kt, intishCast>(base, key, pState);               \
}
VGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <ICMode intishCast>
inline ArrayData* checkedSet(ArrayData* a,
                             StringData* key,
                             Cell value) {
  assertx(a->isPHPArray());
  if (auto const intish = tryIntishCast<intishCast>(key)) {
    return a->set(*intish, value);
  } else {
    return a->set(key, value);
  }
}

template <ICMode intishCast>
inline ArrayData* checkedSet(ArrayData*, int64_t, Cell) {
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<KeyType keyType, bool checkForInt, bool setRef, ICMode intishCast>
auto
arraySetImpl(ArrayData* a, key_type<keyType> key, Cell value, TypedValue* ref) {
  static_assert(keyType != KeyType::Any,
                "KeyType::Any is not supported in arraySetMImpl");
  assertx(cellIsPlausible(value));
  assertx(a->isPHPArray());
  ArrayData* ret = checkForInt ? checkedSet<intishCast>(a, key, value)
                               : a->set(key, value);
  return arrayRefShuffle<setRef, KindOfArray>(a, ret, ref);
}

#define ARRAYSET_HELPER_TABLE(m)                        \
  /* name         keyType     checkForInt intishCast */    \
  m(arraySetS,    KeyType::Str,   false,  ICMode::Ignore)  \
  m(arraySetSi,   KeyType::Str,    true,  ICMode::Ignore)  \
  m(arraySetI,    KeyType::Int,   false,  ICMode::Ignore)  \
  m(arraySetSC,   KeyType::Str,   false,  ICMode::Cast)    \
  m(arraySetSiC,  KeyType::Str,    true,  ICMode::Cast)    \
  m(arraySetIC,   KeyType::Int,   false,  ICMode::Cast)    \
  m(arraySetSW,   KeyType::Str,   false,  ICMode::Warn)    \
  m(arraySetSiW,  KeyType::Str,    true,  ICMode::Warn)    \
  m(arraySetIW,   KeyType::Int,   false,  ICMode::Warn)    \

#define X(nm, keyType, checkForInt, intishCast)                         \
inline ArrayData* nm(ArrayData* a, key_type<keyType> key, Cell value) { \
  return arraySetImpl<keyType, checkForInt, false, intishCast>(         \
    a, key, value, nullptr                                              \
  );                                                                    \
}
ARRAYSET_HELPER_TABLE(X)
#undef X

#define ARRAYSET_REF_HELPER_TABLE(m)                     \
  /* name         keyType     checkForInt intishCast */  \
  m(arraySetSR,   KeyType::Str,   false,   ICMode::Ignore) \
  m(arraySetSiR,  KeyType::Str,    true,   ICMode::Ignore) \
  m(arraySetIR,   KeyType::Int,   false,   ICMode::Ignore) \
  m(arraySetSRC,  KeyType::Str,   false,   ICMode::Cast)   \
  m(arraySetSiRC, KeyType::Str,    true,   ICMode::Cast)   \
  m(arraySetIRC,  KeyType::Int,   false,   ICMode::Cast)   \
  m(arraySetSRW,  KeyType::Str,   false,   ICMode::Warn)   \
  m(arraySetSiRW, KeyType::Str,    true,   ICMode::Warn)   \
  m(arraySetIRW,  KeyType::Int,   false,   ICMode::Warn)   \

#define X(nm, keyType, checkForInt, intishCast)                          \
inline                                                                   \
void nm(ArrayData* a, key_type<keyType> key, Cell value, RefData* ref) { \
  arraySetImpl<keyType, checkForInt, true, intishCast>(                  \
    a, key, value, ref->cell()                                           \
  );                                                                     \
}
ARRAYSET_REF_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<bool setRef>
auto vecSetImpl(ArrayData* a, int64_t key, Cell value, TypedValue* ref) {
  assertx(cellIsPlausible(value));
  assertx(a->isVecArray());
  ArrayData* ret = PackedArray::SetIntVec(a, key, value);
  return arrayRefShuffle<setRef, KindOfVec>(a, ret, ref);
}

inline ArrayData* vecSetI(ArrayData* a, int64_t key, Cell value) {
  return vecSetImpl<false>(a, key, value, nullptr);
}

inline void vecSetIR(ArrayData* a, int64_t key, Cell value,
                     RefData* ref) {
  vecSetImpl<true>(a, key, value, ref->cell());
}

//////////////////////////////////////////////////////////////////////

inline ArrayData* dictSetImplPre(ArrayData* a, int64_t i, Cell val) {
  return MixedArray::SetIntDict(a, i, val);
}
inline ArrayData* dictSetImplPre(ArrayData* a, StringData* s, Cell val) {
  return MixedArray::SetStrDict(a, s, val);
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
  dictSetImpl<keyType, true>(a, key, val, ref->cell());                 \
}
DICTSET_REF_HELPER_TABLE(X)
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
template <KeyType keyType, ICMode intishCast>
StringData* setElemImpl(tv_lval base, key_type<keyType> key,
                        Cell val, const MInstrPropState* pState) {
  return HPHP::SetElem<false, intishCast, keyType>(base, key, &val, pState);
}

#define SETELEM_HELPER_TABLE(m)               \
  /* name       keyType       intishCast */   \
  m(setElemC,   KeyType::Any, ICMode::Ignore) \
  m(setElemI,   KeyType::Int, ICMode::Ignore) \
  m(setElemS,   KeyType::Str, ICMode::Ignore) \
  m(setElemCC,  KeyType::Any, ICMode::Cast)   \
  m(setElemIC,  KeyType::Int, ICMode::Cast)   \
  m(setElemSC,  KeyType::Str, ICMode::Cast)   \
  m(setElemCW,  KeyType::Any, ICMode::Warn)   \
  m(setElemIW,  KeyType::Int, ICMode::Warn)   \
  m(setElemSW,  KeyType::Str, ICMode::Warn)   \

#define X(nm, kt, intishCast)                                           \
inline StringData* nm(tv_lval base, key_type<kt> key,                   \
                      Cell val, const MInstrPropState* pState) {        \
  return setElemImpl<kt, intishCast>(base, key, val, pState);           \
}
SETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////


template<KeyType keyType, bool checkForInt, ICMode intishCast>
uint64_t arrayIssetImpl(ArrayData* a, key_type<keyType> key) {
  auto const rval = checkForInt
    ? checkedGet<false, intishCast>(a, key)
    : a->rval(key);
  return !rval
    ? 0
    : !isNullType(rval.unboxed().type());
}

#define ARRAY_ISSET_HELPER_TABLE(m)                             \
  /* name           keyType         checkForInt  intishCast */  \
  m(arrayIssetS,    KeyType::Str,   false,       ICMode::Ignore) \
  m(arrayIssetSi,   KeyType::Str,    true,       ICMode::Ignore) \
  m(arrayIssetI,    KeyType::Int,   false,       ICMode::Ignore) \
  m(arrayIssetSC,   KeyType::Str,   false,       ICMode::Cast)   \
  m(arrayIssetSiC,  KeyType::Str,    true,       ICMode::Cast)   \
  m(arrayIssetIC,   KeyType::Int,   false,       ICMode::Cast)   \
  m(arrayIssetSW,   KeyType::Str,   false,       ICMode::Warn)   \
  m(arrayIssetSiW,  KeyType::Str,    true,       ICMode::Warn)   \
  m(arrayIssetIW,   KeyType::Int,   false,       ICMode::Warn)   \

#define X(nm, keyType, checkForInt, intishCast)                     \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) {           \
  return arrayIssetImpl<keyType, checkForInt, intishCast>(a, key);  \
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

template<KeyType keyType, ICMode intishCast>
void unsetElemImpl(tv_lval base, key_type<keyType> key) {
  HPHP::UnsetElem<intishCast, keyType>(base, key);
}

#define UNSET_ELEM_HELPER_TABLE(m)              \
  /* name         keyType       intishCast */   \
  m(unsetElemC,   KeyType::Any, ICMode::Ignore) \
  m(unsetElemI,   KeyType::Int, ICMode::Ignore) \
  m(unsetElemS,   KeyType::Str, ICMode::Ignore) \
  m(unsetElemCC,  KeyType::Any, ICMode::Cast)   \
  m(unsetElemIC,  KeyType::Int, ICMode::Cast)   \
  m(unsetElemSC,  KeyType::Str, ICMode::Cast)   \
  m(unsetElemCW,  KeyType::Any, ICMode::Warn)   \
  m(unsetElemIW,  KeyType::Int, ICMode::Warn)   \
  m(unsetElemSW,  KeyType::Str, ICMode::Warn)   \

#define X(nm, kt, intishCast)                         \
inline void nm(tv_lval base, key_type<kt> key) {  \
  unsetElemImpl<kt, intishCast>(base, key);           \
}
UNSET_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////
template <KeyType keyType, bool isEmpty, ICMode intishCast>
bool issetEmptyElemImpl(tv_lval base, key_type<keyType> key) {
  return HPHP::IssetEmptyElem<isEmpty, intishCast, keyType>(base, key);
}

#define ISSET_EMPTY_ELEM_HELPER_TABLE(m)                  \
  /* name         keyType       isEmpty  intishCast */    \
  m(issetElemC,   KeyType::Any, false,   ICMode::Ignore) \
  m(issetElemCE,  KeyType::Any,  true,   ICMode::Ignore) \
  m(issetElemI,   KeyType::Int, false,   ICMode::Ignore) \
  m(issetElemIE,  KeyType::Int,  true,   ICMode::Ignore) \
  m(issetElemS,   KeyType::Str, false,   ICMode::Ignore) \
  m(issetElemSE,  KeyType::Str,  true,   ICMode::Ignore) \
  m(issetElemCC,  KeyType::Any, false,   ICMode::Cast)   \
  m(issetElemCEC, KeyType::Any,  true,   ICMode::Cast)   \
  m(issetElemIC,  KeyType::Int, false,   ICMode::Cast)   \
  m(issetElemIEC, KeyType::Int,  true,   ICMode::Cast)   \
  m(issetElemSC,  KeyType::Str, false,   ICMode::Cast)   \
  m(issetElemSEC, KeyType::Str,  true,   ICMode::Cast)   \
  m(issetElemCW,  KeyType::Any, false,   ICMode::Warn)   \
  m(issetElemCEW, KeyType::Any,  true,   ICMode::Warn)   \
  m(issetElemIW,  KeyType::Int, false,   ICMode::Warn)   \
  m(issetElemIEW, KeyType::Int,  true,   ICMode::Warn)   \
  m(issetElemSW,  KeyType::Str, false,   ICMode::Warn)   \
  m(issetElemSEW, KeyType::Str,  true,   ICMode::Warn)   \

#define X(nm, kt, isEmpty, intishCast)                               \
inline uint64_t nm(tv_lval base, key_type<kt> key) {             \
  return issetEmptyElemImpl<kt, isEmpty, intishCast>(base, key);     \
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

inline
void vectorSetImplI(c_Vector* vector, int64_t key, Cell value) {
  vector->set(key, value);
}

[[noreturn]] inline
void vectorSetImplS(c_Vector* vector, StringData* key, Cell value) {
  BaseVector::throwBadKeyType();
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
