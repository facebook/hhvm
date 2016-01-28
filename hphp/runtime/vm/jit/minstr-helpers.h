/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

// This header does fun things with macros; keep it last.
#include "hphp/runtime/vm/jit/minstr-translator-internal.h"

namespace HPHP { namespace jit { namespace MInstrHelpers {

//////////////////////////////////////////////////////////////////////

template <bool warn, bool define>
TypedValue* baseGImpl(TypedValue key) {
  TypedValue* base;
  StringData* name = prepareKey(key);
  SCOPE_EXIT { decRefStr(name); };
  VarEnv* varEnv = g_context->m_globalVarEnv;
  assertx(varEnv != nullptr);
  base = varEnv->lookup(name);
  if (base == nullptr) {
    if (warn) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    if (define) {
      TypedValue tv;
      tvWriteNull(&tv);
      varEnv->set(name, &tv);
      base = varEnv->lookup(name);
    } else {
      return const_cast<TypedValue*>(init_null_variant.asTypedValue());
    }
  }
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

inline TypedValue* baseG(TypedValue key) {
  return baseGImpl<false, false>(key);
}

inline TypedValue* baseGW(TypedValue key) {
  return baseGImpl<true, false>(key);
}

inline TypedValue* baseGD(TypedValue key) {
  return baseGImpl<false, true>(key);
}

inline TypedValue* baseGWD(TypedValue key) {
  return baseGImpl<true, true>(key);
}

//////////////////////////////////////////////////////////////////////

template <MInstrAttr attrs, KeyType kt, bool isObj>
TypedValue* propImpl(Class* ctx, TypedValue* base,
                     key_type<kt> key, TypedValue& tvRef) {
  return Prop<WDU(attrs), isObj, kt>(tvRef, ctx, base, key);
}

#define PROP_HELPER_TABLE(m)                      \
  /* name      attrs       keyType       isObj */ \
  m(propC,     None,       KeyType::Any, false)   \
  m(propCS,    None,       KeyType::Str, false)   \
  m(propCD,    Define,     KeyType::Any, false)   \
  m(propCDS,   Define,     KeyType::Str, false)   \
  m(propCDO,   Define,     KeyType::Any, true)    \
  m(propCDOS,  Define,     KeyType::Str, true)    \
  m(propCO,    None,       KeyType::Any, true)    \
  m(propCOS,   None,       KeyType::Str, true)    \
  m(propCU,    Unset,      KeyType::Any, false)   \
  m(propCUS,   Unset,      KeyType::Str, false)   \
  m(propCUO,   Unset,      KeyType::Any, true)    \
  m(propCUOS,  Unset,      KeyType::Str, true)    \
  m(propCW,    Warn,       KeyType::Any, false)   \
  m(propCWS,   Warn,       KeyType::Str, false)   \
  m(propCWD,   WarnDefine, KeyType::Any, false)   \
  m(propCWDS,  WarnDefine, KeyType::Str, false)   \
  m(propCWDO,  WarnDefine, KeyType::Any, true)    \
  m(propCWDOS, WarnDefine, KeyType::Str, true)    \
  m(propCWO,   Warn,       KeyType::Any, true)    \
  m(propCWOS,  Warn,       KeyType::Str, true)    \

#define X(nm, attrs, kt, isObj)                                       \
inline TypedValue* nm(Class* ctx, TypedValue* base, key_type<kt> key, \
                      TypedValue& tvRef) {                            \
  return propImpl<attrs, kt, isObj>(ctx, base, key, tvRef);           \
}
PROP_HELPER_TABLE(X)
#undef X

// NullSafe prop.
inline TypedValue* propCQ(Class* ctx, TypedValue* base, StringData* key,
                          TypedValue& tvRef) {
  return nullSafeProp(tvRef, ctx, base, key);
}

// NullSafe prop with object base.
inline TypedValue* propCOQ(Class* ctx, ObjectData* base, StringData* key,
                           TypedValue& tvRef) {
  return base->prop(&tvRef, ctx, key);
}

//////////////////////////////////////////////////////////////////////

inline TypedValue cGetRefShuffle(const TypedValue& localTvRef,
                                 const TypedValue* result) {
  if (LIKELY(result != &localTvRef)) {
    result = tvToCell(result);
    tvRefcountedIncRef(result);
  } else {
    // If a magic getter or array access method returned by reference, we have
    // to incref the inner cell and drop our reference to the RefData.
    // Otherwise we do nothing, since we already own a reference to result.
    if (UNLIKELY(localTvRef.m_type == KindOfRef)) {
      auto inner = *localTvRef.m_data.pref->tv();
      tvRefcountedIncRef(&inner);
      decRefRef(localTvRef.m_data.pref);
      return inner;
    }
  }

  return *result;
}

template <KeyType keyType, bool isObj, bool warn>
TypedValue cGetPropImpl(Class* ctx, TypedValue* base, key_type<keyType> key) {
  TypedValue localTvRef;
  auto result = Prop<warn, false, false, isObj, keyType>(
    localTvRef,
    ctx,
    base,
    key
  );
  return cGetRefShuffle(localTvRef, result);
}

#define CGETPROP_HELPER_TABLE(m)                      \
  /* name             keyType       isObj   attrs */  \
  m(cGetPropCQuiet,  KeyType::Any, false,  None)      \
  m(cGetPropCOQuiet, KeyType::Any,  true,  None)      \
  m(cGetPropSQuiet,  KeyType::Str, false,  None)      \
  m(cGetPropSOQuiet, KeyType::Str,  true,  None)      \
  m(cGetPropC,       KeyType::Any, false,  Warn)      \
  m(cGetPropCO,      KeyType::Any,  true,  Warn)      \
  m(cGetPropS,       KeyType::Str, false,  Warn)      \
  m(cGetPropSO,      KeyType::Str,  true,  Warn)

#define X(nm, kt, isObj, attrs)                                        \
inline TypedValue nm(Class* ctx, TypedValue* base, key_type<kt> key) { \
  return cGetPropImpl<kt, isObj, (attrs) & MIA_warn>(ctx, base, key);  \
}
CGETPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// NullSafe prop.
inline TypedValue cGetPropSQ(Class* ctx, TypedValue* base, StringData* key) {
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
                               TypedValue* result) {
  if (LIKELY(result != &localTvRef)) {
    if (result->m_type != KindOfRef) tvBox(result);
    auto ref = result->m_data.pref;
    ref->incRefCount();
    return ref;
  }

  if (localTvRef.m_type != KindOfRef) {
    // RefData::Make takes ownership of the reference that lives in localTvRef
    // so no refcounting is necessary.
    return RefData::Make(localTvRef);
  }

  return localTvRef.m_data.pref;
}

template <KeyType keyType, bool isObj>
RefData* vGetPropImpl(Class* ctx, TypedValue* base, key_type<keyType> key) {
  TypedValue localTvRef;
  auto result = Prop<false, true, false, isObj, keyType>(
    localTvRef, ctx, base, key
  );

  return vGetRefShuffle(localTvRef, result);
}

#define VGETPROP_HELPER_TABLE(m)       \
  /* name        keyType       isObj */\
  m(vGetPropC,   KeyType::Any, false)  \
  m(vGetPropCO,  KeyType::Any,  true)  \
  m(vGetPropS,   KeyType::Str, false)  \
  m(vGetPropSO,  KeyType::Str,  true)

#define X(nm, kt, isObj)                                              \
inline RefData* nm(Class* ctx, TypedValue* base, key_type<kt> key) {  \
  return vGetPropImpl<kt, isObj>(ctx, base, key);                     \
}
VGETPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool isObj>
void bindPropImpl(Class* ctx, TypedValue* base, TypedValue key,
                  RefData* val) {
  TypedValue localTvRef;
  auto prop = Prop<false, true, false, isObj>(localTvRef, ctx, base, key);

  if (UNLIKELY(prop == &localTvRef)) {
    // Skip binding a TypedValue that's about to be destroyed and just destroy
    // it now.
    tvRefcountedDecRef(localTvRef);
    return;
  }

  tvBindRef(val, prop);
}

#define BINDPROP_HELPER_TABLE(m)   \
  /* name        isObj */          \
  m(bindPropC,   false)            \
  m(bindPropCO,   true)

#define X(nm, ...)                                                      \
inline void nm(Class* ctx, TypedValue* base, TypedValue key,            \
               RefData* val) {                                          \
  bindPropImpl<__VA_ARGS__>(ctx, base, key, val);                       \
}
BINDPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType kt, bool isObj>
void setPropImpl(Class* ctx, TypedValue* base, key_type<kt> key, Cell val) {
  HPHP::SetProp<false, isObj, kt>(ctx, base, key, &val);
}

#define SETPROP_HELPER_TABLE(m)                 \
  /* name        keyType       isObj */         \
  m(setPropC,    KeyType::Any, false)           \
  m(setPropCS,   KeyType::Str, false)           \
  m(setPropCO,   KeyType::Any, true)            \
  m(setPropCOS,  KeyType::Str, true)            \

#define X(nm, kt, isObj)                                                   \
inline void nm(Class* ctx, TypedValue* base, key_type<kt> key, Cell val) { \
  setPropImpl<kt, isObj>(ctx, base, key, val);                             \
}
SETPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool isObj>
void unsetPropImpl(Class* ctx, TypedValue* base, TypedValue key) {
  HPHP::UnsetProp<isObj>(ctx, base, key);
}

#define UNSETPROP_HELPER_TABLE(m)  \
  /* name        isObj */          \
  m(unsetPropC,  false)            \
  m(unsetPropCO,  true)

#define X(nm, ...)                                              \
inline void nm(Class* ctx, TypedValue* base, TypedValue key) {  \
  unsetPropImpl<__VA_ARGS__>(ctx, base, key);                   \
}
UNSETPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool isObj>
TypedValue setOpPropImpl(Class* ctx, TypedValue* base,
                         TypedValue key, Cell val, SetOpOp op) {
  TypedValue localTvRef;
  auto result = HPHP::SetOpProp<isObj>(localTvRef, ctx, op, base, key, &val);

  return cGetRefShuffle(localTvRef, result);
}

#define SETOPPROP_HELPER_TABLE(m)               \
  /* name         isObj */                      \
  m(setOpPropC,    false)                       \
  m(setOpPropCO,    true)

#define X(nm, ...)                                                      \
inline TypedValue nm(Class* ctx, TypedValue* base, TypedValue key,      \
                     Cell val, SetOpOp op) {                            \
  return setOpPropImpl<__VA_ARGS__>(ctx, base, key, val, op);           \
}
SETOPPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool isObj>
TypedValue incDecPropImpl(
  Class* ctx,
  TypedValue* base,
  TypedValue key,
  IncDecOp op
) {
  auto result = make_tv<KindOfUninit>();
  HPHP::IncDecProp<isObj>(ctx, op, base, key, result);
  assertx(result.m_type != KindOfRef);
  return result;
}

#define INCDECPROP_HELPER_TABLE(m)              \
  /* name         isObj */                      \
  m(incDecPropC,   false)                       \
  m(incDecPropCO,   true)

#define X(nm, ...)                                                      \
inline TypedValue nm(                                                   \
  Class* ctx,                                                           \
  TypedValue* base,                                                     \
  TypedValue key,                                                       \
  IncDecOp op                                                           \
) {                                                                     \
  return incDecPropImpl<__VA_ARGS__>(ctx, base, key, op);               \
}
INCDECPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType kt, bool useEmpty, bool isObj>
bool issetEmptyPropImpl(Class* ctx, TypedValue* base, key_type<kt> key) {
  return HPHP::IssetEmptyProp<useEmpty, isObj, kt>(ctx, base, key);
}

#define ISSET_EMPTY_PROP_HELPER_TABLE(m)                          \
  /* name          keyType       useEmpty isObj */                \
  m(issetPropC,    KeyType::Any, false,   false)                  \
  m(issetPropCS,   KeyType::Str, false,   false)                  \
  m(issetPropCE,   KeyType::Any, true,    false)                  \
  m(issetPropCES,  KeyType::Str, true,    false)                  \
  m(issetPropCEO,  KeyType::Any, true,    true)                   \
  m(issetPropCEOS, KeyType::Str, true,    true)                   \
  m(issetPropCO,   KeyType::Any, false,   true)                   \
  m(issetPropCOS,  KeyType::Str, false,   true)                   \

#define X(nm, kt, useEmpty, isObj)                                      \
/* This returns uint64_t to ensure all 64 bits of rax are valid. */     \
inline uint64_t nm(Class* ctx, TypedValue* base, key_type<kt> key) {    \
  return issetEmptyPropImpl<kt, useEmpty, isObj>(ctx, base, key);       \
}
ISSET_EMPTY_PROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, bool warn, bool define, bool reffy, bool unset>
TypedValue* elemImpl(TypedValue* base,
                     key_type<keyType> key,
                     TypedValue& tvRef) {
  if (unset) {
    return ElemU<keyType>(tvRef, base, key);
  }
  if (define) {
    return ElemD<warn, reffy, keyType>(tvRef, base, key);
  }
  // We won't really modify the TypedValue in the non-D case, so
  // this const_cast is safe.
  return const_cast<TypedValue*>(Elem<warn, keyType>(tvRef, base, key));
}

#define ELEM_HELPER_TABLE(m)                     \
  /* name      keyType         attrs  */         \
  m(elemC,     KeyType::Any,   None)             \
  m(elemCD,    KeyType::Any,   Define)           \
  m(elemCDR,   KeyType::Any,   DefineReffy)      \
  m(elemCU,    KeyType::Any,   Unset)            \
  m(elemCW,    KeyType::Any,   Warn)             \
  m(elemCWD,   KeyType::Any,   WarnDefine)       \
  m(elemCWDR,  KeyType::Any,   WarnDefineReffy)  \
  m(elemI,     KeyType::Int,   None)             \
  m(elemID,    KeyType::Int,   Define)           \
  m(elemIDR,   KeyType::Int,   DefineReffy)      \
  m(elemIU,    KeyType::Int,   Unset)            \
  m(elemIW,    KeyType::Int,   Warn)             \
  m(elemIWD,   KeyType::Int,   WarnDefine)       \
  m(elemIWDR,  KeyType::Int,   WarnDefineReffy)  \
  m(elemS,     KeyType::Str,   None)             \
  m(elemSD,    KeyType::Str,   Define)           \
  m(elemSDR,   KeyType::Str,   DefineReffy)      \
  m(elemSU,    KeyType::Str,   Unset)            \
  m(elemSW,    KeyType::Str,   Warn)             \
  m(elemSWD,   KeyType::Str,   WarnDefine)       \
  m(elemSWDR,  KeyType::Str,   WarnDefineReffy)

#define X(nm, keyType, attrs)                             \
inline TypedValue* nm(TypedValue* base,                   \
                      key_type<keyType> key,              \
                      TypedValue& tvRef) {                 \
  return elemImpl<keyType, WDRU(attrs)>(base, key, tvRef);  \
}
ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline const TypedValue* checkedGet(ArrayData* a, StringData* key) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ? a->nvGet(i) :
         a->nvGet(key);
}

inline const TypedValue* checkedGet(ArrayData* a, int64_t key) {
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<bool warn>
NEVER_INLINE
const TypedValue* elemArrayNotFound(int64_t k) {
  if (warn) {
    raise_notice("Undefined index: %" PRId64, k);
  }
  return null_variant.asTypedValue();
}

template<bool warn>
NEVER_INLINE
const TypedValue* elemArrayNotFound(const StringData* k) {
  if (warn) {
    raise_notice("Undefined index: %s", k->data());
  }
  return null_variant.asTypedValue();
}

template<KeyType keyType, bool checkForInt, bool warn>
inline const TypedValue* elemArrayImpl(ArrayData* ad, key_type<keyType> key) {
  auto const ret = checkForInt ? checkedGet(ad, key) : ad->nvGet(key);
  return ret ? ret : elemArrayNotFound<warn>(key);
}

#define ELEM_ARRAY_D_HELPER_TABLE(m) \
  /* name                keyType */  \
  m(elemArraySD,    KeyType::Str)    \
  m(elemArrayID,    KeyType::Int)    \

#define X(nm, keyType)                                                 \
inline TypedValue* nm(TypedValue* base, key_type<keyType> key) {       \
  auto cbase = tvToCell(base);                                         \
  assertx(isArrayType(cbase->m_type));                                 \
  auto constexpr warn  = false;                                        \
  auto constexpr reffy = false;                                        \
  return ElemDArray<warn, reffy, keyType>(cbase, key);                 \
}
ELEM_ARRAY_D_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_U_HELPER_TABLE(m) \
  /* name        keyType */          \
  m(elemArraySU, KeyType::Str)       \
  m(elemArrayIU, KeyType::Int)       \

#define X(nm, keyType)                                                 \
inline TypedValue* nm(TypedValue* base, key_type<keyType> key) {       \
  auto cbase = tvToCell(base);                                         \
  assertx(isArrayType(cbase->m_type));                                 \
  return ElemUArray<keyType>(cbase, key);                              \
}
ELEM_ARRAY_U_HELPER_TABLE(X)
#undef X

#define ELEM_ARRAY_HELPER_TABLE(m)                      \
  /* name               keyType  checkForInt   warn */  \
  m(elemArrayS,    KeyType::Str,       false,  false)   \
  m(elemArraySi,   KeyType::Str,        true,  false)   \
  m(elemArrayI,    KeyType::Int,       false,  false)   \
  m(elemArraySW,   KeyType::Str,       false,  true)    \
  m(elemArraySiW,  KeyType::Str,        true,  true)    \
  m(elemArrayIW,   KeyType::Int,       false,  true)    \

#define X(nm, keyType, checkForInt, warn)               \
inline const TypedValue* nm(ArrayData* ad, key_type<keyType> key) { \
  return elemArrayImpl<keyType, checkForInt, warn>(ad, key);\
}
ELEM_ARRAY_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// Keep these error handlers in sync with ArrayData::getNotFound();
TypedValue arrayGetNotFound(int64_t k);
TypedValue arrayGetNotFound(const StringData* k);

template<KeyType keyType, bool checkForInt>
TypedValue arrayGetImpl(ArrayData* a, key_type<keyType> key) {
  auto ret = checkForInt ? checkedGet(a, key) : a->nvGet(key);
  if (ret) return *ret;
  return arrayGetNotFound(key);
}

#define ARRAYGET_HELPER_TABLE(m)                           \
  /* name        keyType     checkForInt */                \
  m(arrayGetS,  KeyType::Str,   false)                     \
  m(arrayGetSi, KeyType::Str,    true)                     \
  m(arrayGetI,  KeyType::Int,   false)                     \

#define X(nm, keyType, checkForInt)                          \
inline TypedValue nm(ArrayData* a, key_type<keyType> key) {  \
  return arrayGetImpl<keyType, checkForInt>(a, key);         \
}
ARRAYGET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, bool warn>
TypedValue cGetElemImpl(TypedValue* base, key_type<keyType> key) {
  TypedValue localTvRef;
  auto result = Elem<warn, keyType>(localTvRef, base, key);
  return cGetRefShuffle(localTvRef, result);
}

#define CGETELEM_HELPER_TABLE(m)               \
  /* name            key          attrs  */    \
  m(cGetElemCQuiet, KeyType::Any, None)        \
  m(cGetElemIQuiet, KeyType::Int, None)        \
  m(cGetElemSQuiet, KeyType::Str, None)        \
  m(cGetElemC,      KeyType::Any, Warn)        \
  m(cGetElemI,      KeyType::Int, Warn)        \
  m(cGetElemS,      KeyType::Str, Warn)

#define X(nm, kt, attrs)                                                \
inline TypedValue nm(TypedValue* base, key_type<kt> key) {              \
  return cGetElemImpl<kt, (attrs) & MIA_warn>(base, key);               \
}
CGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType>
RefData* vGetElemImpl(TypedValue* base, key_type<keyType> key) {
  TypedValue localTvRef;
  auto result = HPHP::ElemD<false, true, keyType>(localTvRef, base, key);

  return vGetRefShuffle(localTvRef, result);
}

#define VGETELEM_HELPER_TABLE(m)                \
  /* name         keyType */                    \
  m(vGetElemC,    KeyType::Any)                 \
  m(vGetElemI,    KeyType::Int)                 \
  m(vGetElemS,    KeyType::Str)

#define X(nm, kt)                                                       \
  inline RefData* nm(TypedValue* base, key_type<kt> key) {              \
    return vGetElemImpl<kt>(base, key);                                 \
}
VGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

inline ArrayData* uncheckedSet(ArrayData* a,
                               StringData* key,
                               Cell value,
                               bool copy) {
  return g_array_funcs.setStr[a->kind()](a, key, value, copy);
}

inline ArrayData* uncheckedSet(ArrayData* a,
                               int64_t key,
                               Cell value,
                               bool copy) {
  return g_array_funcs.setInt[a->kind()](a, key, value, copy);
}


inline ArrayData* checkedSet(ArrayData* a,
                             StringData* key,
                             Cell value,
                             bool copy) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ? uncheckedSet(a, i, value, copy) :
         uncheckedSet(a, key, value, copy);
}

inline ArrayData* checkedSet(ArrayData*, int64_t, Cell, bool) {
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<KeyType keyType, bool checkForInt, bool setRef>
typename ShuffleReturn<setRef>::return_type
arraySetImpl(ArrayData* a, key_type<keyType> key, Cell value, RefData* ref) {
  static_assert(keyType != KeyType::Any,
                "KeyType::Any is not supported in arraySetMImpl");
  assertx(cellIsPlausible(value));
  const bool copy = a->cowCheck();
  ArrayData* ret = checkForInt ? checkedSet(a, key, value, copy)
                               : uncheckedSet(a, key, value, copy);
  return arrayRefShuffle<setRef>(a, ret, setRef ? ref->tv() : nullptr);
}

#define ARRAYSET_HELPER_TABLE(m)                             \
  /* name        keyType        checkForInt  setRef */       \
  m(arraySetS,   KeyType::Str,   false,      false)          \
  m(arraySetSi,  KeyType::Str,    true,      false)          \
  m(arraySetI,   KeyType::Int,   false,      false)          \
  m(arraySetSR,  KeyType::Str,   false,      true)           \
  m(arraySetSiR, KeyType::Str,    true,      true)           \
  m(arraySetIR,  KeyType::Int,   false,      true)           \

#define X(nm, keyType, checkForInt, setRef)                  \
ShuffleReturn<setRef>::return_type                           \
inline nm(ArrayData* a, key_type<keyType> key, Cell value, RefData* ref) { \
  return arraySetImpl<keyType, checkForInt, setRef>(a, key, value, ref);\
}
ARRAYSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType>
StringData* setElemImpl(TypedValue* base, key_type<keyType> key, Cell val) {
  return HPHP::SetElem<false, keyType>(base, key, &val);
}

#define SETELEM_HELPER_TABLE(m)                 \
  /* name       keyType    */                   \
  m(setElemC,   KeyType::Any)                   \
  m(setElemI,   KeyType::Int)                   \
  m(setElemS,   KeyType::Str)

#define X(nm, kt)                                                     \
inline StringData* nm(TypedValue* base, key_type<kt> key, Cell val) { \
  return setElemImpl<kt>(base, key, val);                             \
}
SETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////


template<KeyType keyType, bool checkForInt>
uint64_t arrayIssetImpl(ArrayData* a, key_type<keyType> key) {
  auto const value = checkForInt ? checkedGet(a, key) : a->nvGet(key);
  return !value ? 0 :
         !tvAsCVarRef(value).isNull();
}

#define ARRAY_ISSET_HELPER_TABLE(m)                  \
  /* name           keyType         checkForInt  */  \
  m(arrayIssetS,    KeyType::Str,   false)           \
  m(arrayIssetSi,   KeyType::Str,    true)           \
  m(arrayIssetI,    KeyType::Int,   false)           \

#define X(nm, keyType, checkForInt)                    \
inline uint64_t nm(ArrayData* a, key_type<keyType> key) {\
  return arrayIssetImpl<keyType, checkForInt>(a, key);\
}
ARRAY_ISSET_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<KeyType keyType>
void unsetElemImpl(TypedValue* base, key_type<keyType> key) {
  HPHP::UnsetElem<keyType>(base, key);
}

#define UNSET_ELEM_HELPER_TABLE(m)              \
  /* name         keyType */                    \
  m(unsetElemC,   KeyType::Any)                 \
  m(unsetElemI,   KeyType::Int)                 \
  m(unsetElemS,   KeyType::Str)

#define X(nm, kt)                                     \
inline void nm(TypedValue* base, key_type<kt> key) {  \
  unsetElemImpl<kt>(base, key);                       \
}
UNSET_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////
template <KeyType keyType, bool isEmpty>
bool issetEmptyElemImpl(TypedValue* base, key_type<keyType> key) {
  return HPHP::IssetEmptyElem<isEmpty, keyType>(base, key);
}

#define ISSET_EMPTY_ELEM_HELPER_TABLE(m)        \
  /* name         keyType     isEmpty */        \
  m(issetElemC,   KeyType::Any, false)          \
  m(issetElemCE,  KeyType::Any,  true)          \
  m(issetElemI,   KeyType::Int, false)          \
  m(issetElemIE,  KeyType::Int,  true)          \
  m(issetElemS,   KeyType::Str, false)          \
  m(issetElemSE,  KeyType::Str,  true)          \

#define X(nm, kt, isEmpty)                                \
inline uint64_t nm(TypedValue* base, key_type<kt> key) {  \
  return issetEmptyElemImpl<kt, isEmpty>(base, key);      \
}
ISSET_EMPTY_ELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template<KeyType keyType>
TypedValue mapGetImpl(c_Map* map, key_type<keyType> key) {
  TypedValue* ret = map->at(key);
  tvRefcountedIncRef(ret);
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
  map->set(key, &value);
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
