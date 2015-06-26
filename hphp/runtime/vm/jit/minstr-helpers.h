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

template <MInstrAttr attrs, bool isObj>
TypedValue* propImpl(Class* ctx, TypedValue* base,
                     TypedValue key, MInstrState* mis) {
  return Prop<WDU(attrs), isObj>(
    mis->tvScratch, mis->tvRef, ctx, base, key);
}

#define PROP_HELPER_TABLE(m)                        \
  /* name     attrs        isObj */                 \
  m(propC,    None,        false)                   \
  m(propCD,   Define,      false)                   \
  m(propCDO,  Define,       true)                   \
  m(propCO,   None,         true)                   \
  m(propCU,   Unset,       false)                   \
  m(propCUO,  Unset,        true)                   \
  m(propCW,   Warn,        false)                   \
  m(propCWD,  WarnDefine,  false)                   \
  m(propCWDO, WarnDefine,   true)                   \
  m(propCWO,  Warn,         true)

#define X(nm, ...)                                                      \
inline TypedValue* nm(Class* ctx, TypedValue* base, TypedValue key,     \
               MInstrState* mis) {                                      \
  return propImpl<__VA_ARGS__>(ctx, base, key, mis);                    \
}
PROP_HELPER_TABLE(X)
#undef X

// NullSafe prop.
inline TypedValue* propCQ(Class* ctx, TypedValue* base, StringData* key,
                          MInstrState* mis) {
  return nullSafeProp(mis->tvScratch, mis->tvRef, ctx, base, key);
}

// NullSafe prop with object base.
inline TypedValue* propCOQ(Class* ctx, ObjectData* base, StringData* key,
                           MInstrState* mis) {
  return base->prop(&mis->tvScratch, &mis->tvRef, ctx, key);
}

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, bool isObj>
TypedValue cGetPropImpl(Class* ctx, TypedValue* base,
                        key_type<keyType> key, MInstrState* mis) {
  TypedValue scratch;
  TypedValue* result = Prop<true, false, false, isObj, keyType>(
    scratch, mis->tvRef, ctx, base, key);
  result = tvToCell(result);
  tvRefcountedIncRef(result);
  return *result;
}

#define CGETPROP_HELPER_TABLE(m)          \
  /* name         keyType       isObj */  \
  m(cGetPropC,    KeyType::Any, false)    \
  m(cGetPropCO,   KeyType::Any,  true)    \
  m(cGetPropS,    KeyType::Str, false)    \
  m(cGetPropSO,   KeyType::Str,  true)

#define X(nm, kt, isObj)                                          \
inline TypedValue nm(Class* ctx, TypedValue* base, key_type<kt> key, \
                     MInstrState* mis) {                           \
  return cGetPropImpl<kt, isObj>(ctx, base, key, mis);             \
}
CGETPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

// NullSafe prop.
inline TypedValue cGetPropSQ(Class* ctx, TypedValue* base,
                             StringData* key, MInstrState* mis) {
  TypedValue scratch;
  auto result = nullSafeProp(scratch, mis->tvRef, ctx, base, key);
  result = tvToCell(result);
  tvRefcountedIncRef(result);
  return *result;
}

// NullSafe prop with object base.
inline TypedValue cGetPropSOQ(Class* ctx, ObjectData* base,
                             StringData* key, MInstrState* mis) {
  TypedValue scratch;
  auto result = base->prop(&scratch, &mis->tvRef, ctx, key);
  result = tvToCell(result);
  tvRefcountedIncRef(result);
  return *result;
}

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, bool isObj>
RefData* vGetPropImpl(Class* ctx, TypedValue* base,
                      key_type<keyType> key, MInstrState* mis) {
  TypedValue* result = Prop<false, true, false, isObj, keyType>(
    mis->tvScratch, mis->tvRef, ctx, base, key);

  if (result->m_type != KindOfRef) {
    tvBox(result);
  }
  RefData* ref = result->m_data.pref;
  ref->incRefCount();
  return ref;
}

#define VGETPROP_HELPER_TABLE(m)       \
  /* name        keyType       isObj */\
  m(vGetPropC,   KeyType::Any, false)  \
  m(vGetPropCO,  KeyType::Any,  true)  \
  m(vGetPropS,   KeyType::Str, false)  \
  m(vGetPropSO,  KeyType::Str,  true)

#define X(nm, kt, isObj)                                           \
inline RefData* nm(Class* ctx, TypedValue* base, key_type<kt> key, \
                   MInstrState* mis) {                             \
  return vGetPropImpl<kt, isObj>(ctx, base, key, mis);             \
}
VGETPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool isObj>
void bindPropImpl(Class* ctx, TypedValue* base, TypedValue key,
                  RefData* val, MInstrState* mis) {
  TypedValue* prop = Prop<false, true, false, isObj>(
    mis->tvScratch, mis->tvRef, ctx, base, key);
  if (!(prop == &mis->tvScratch && prop->m_type == KindOfUninit)) {
    tvBindRef(val, prop);
  }
}

#define BINDPROP_HELPER_TABLE(m)   \
  /* name        isObj */          \
  m(bindPropC,   false)            \
  m(bindPropCO,   true)

#define X(nm, ...)                                                      \
inline void nm(Class* ctx, TypedValue* base, TypedValue key,            \
               RefData* val, MInstrState* mis) {                        \
  bindPropImpl<__VA_ARGS__>(ctx, base, key, val, mis);                  \
}
BINDPROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <bool isObj>
void setPropImpl(Class* ctx, TypedValue* base,
                 TypedValue key, Cell val) {
  HPHP::SetProp<false, isObj>(ctx, base, key, &val);
}

#define SETPROP_HELPER_TABLE(m)             \
  /* name        isObj */                   \
  m(setPropC,    false)                     \
  m(setPropCO,    true)

#define X(nm, ...)                                                      \
inline void nm(Class* ctx, TypedValue* base, TypedValue key,            \
               Cell val) {                                              \
  setPropImpl<__VA_ARGS__>(ctx, base, key, val);                        \
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
                         TypedValue key,
                         Cell val, MInstrState* mis, SetOpOp op) {
  TypedValue* result = HPHP::SetOpProp<isObj>(
    mis->tvScratch, mis->tvRef, ctx, op, base, key, &val);

  Cell ret;
  cellDup(*tvToCell(result), ret);
  return ret;
}

#define SETOPPROP_HELPER_TABLE(m)               \
  /* name         isObj */                      \
  m(setOpPropC,    false)                       \
  m(setOpPropCO,    true)

#define X(nm, ...)                                                     \
inline TypedValue nm(Class* ctx, TypedValue* base, TypedValue key,     \
              Cell val, MInstrState* mis, SetOpOp op) {                \
  return setOpPropImpl<__VA_ARGS__>(ctx, base, key, val, mis, op);     \
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
  HPHP::IncDecProp<true, isObj>(ctx, op, base, key, result);
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

template <bool useEmpty, bool isObj>
bool issetEmptyPropImpl(Class* ctx, TypedValue* base,
                        TypedValue key) {
  return HPHP::IssetEmptyProp<useEmpty, isObj>(ctx, base, key);
}

#define ISSET_EMPTY_PROP_HELPER_TABLE(m)        \
  /* name         useEmpty isObj */             \
  m(issetPropC,   false,   false)               \
  m(issetPropCE,   true,   false)               \
  m(issetPropCEO,  true,    true)               \
  m(issetPropCO,  false,    true)

#define X(nm, ...)                                                      \
/* This returns int64_t to ensure all 64 bits of rax are valid */       \
inline uint64_t nm(Class* ctx, TypedValue* base, TypedValue key) {      \
  return issetEmptyPropImpl<__VA_ARGS__>(ctx, base, key);               \
}
ISSET_EMPTY_PROP_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType, bool warn, bool define, bool reffy,
          bool unset>
TypedValue* elemImpl(TypedValue* base,
                     key_type<keyType> key,
                     MInstrState* mis) {
  if (unset) {
    return ElemU<keyType>(mis->tvScratch, mis->tvRef, base, key);
  }
  if (define) {
    return ElemD<warn, reffy, keyType>(mis->tvScratch, mis->tvRef, base, key);
  }
  // We won't really modify the TypedValue in the non-D case, so
  // this const_cast is safe.
  return const_cast<TypedValue*>(
    Elem<warn, keyType>(mis->tvScratch, mis->tvRef, base, key)
  );
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
                      MInstrState* mis) {                 \
  return elemImpl<keyType, WDRU(attrs)>(base, key, mis);  \
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
inline const TypedValue* elemArrayImpl(TypedValue* a, key_type<keyType> key) {
  assertx(a->m_type == KindOfArray);
  auto const ad = a->m_data.parr;
  auto const ret = checkForInt ? checkedGet(ad, key) : ad->nvGet(key);
  return ret ? ret : elemArrayNotFound<warn>(key);
}

#define ELEM_ARRAY_HELPER_TABLE(m)                                  \
  /* name               keyType  checkForInt   warn */  \
  m(elemArrayS,    KeyType::Str,       false,  false)   \
  m(elemArraySi,   KeyType::Str,        true,  false)   \
  m(elemArrayI,    KeyType::Int,       false,  false)   \
  m(elemArraySW,   KeyType::Str,       false,  true)    \
  m(elemArraySiW,  KeyType::Str,        true,  true)    \
  m(elemArrayIW,   KeyType::Int,       false,  true)    \

#define X(nm, keyType, checkForInt, warn)               \
inline const TypedValue* nm(TypedValue* a, key_type<keyType> key) { \
  return elemArrayImpl<keyType, checkForInt, warn>(a, key);\
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

template <KeyType keyType>
TypedValue cGetElemImpl(TypedValue* base, key_type<keyType> key,
                        MInstrState* mis) {
  TypedValue scratch;
  auto result = Elem<true, keyType>(scratch, mis->tvRef, base, key);
  result = tvToCell(result);
  tvRefcountedIncRef(result);
  return *result;
}

#define CGETELEM_HELPER_TABLE(m)                \
  /* name       key  */                         \
  m(cGetElemC,  KeyType::Any)                   \
  m(cGetElemI,  KeyType::Int)                   \
  m(cGetElemS,  KeyType::Str)

#define X(nm, kt)                                                       \
inline TypedValue nm(TypedValue* base, key_type<kt> key, MInstrState* mis) { \
  return cGetElemImpl<kt>(base, key, mis);                              \
}
CGETELEM_HELPER_TABLE(X)
#undef X

//////////////////////////////////////////////////////////////////////

template <KeyType keyType>
RefData* vGetElemImpl(TypedValue* base, key_type<keyType> key,
                      MInstrState* mis) {
  TypedValue* result = HPHP::ElemD<false, true, keyType>(
    mis->tvScratch, mis->tvRef, base, key);

  if (result->m_type != KindOfRef) {
    tvBox(result);
  }
  RefData* ref = result->m_data.pref;
  ref->incRefCount();
  return ref;
}

#define VGETELEM_HELPER_TABLE(m)                \
  /* name         keyType */                    \
  m(vGetElemC,    KeyType::Any)                 \
  m(vGetElemI,    KeyType::Int)                 \
  m(vGetElemS,    KeyType::Str)

#define X(nm, kt)                                                       \
inline RefData* nm(TypedValue* base, key_type<kt> key, MInstrState* mis) { \
  return vGetElemImpl<kt>(base, key,  mis);                             \
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
  const bool copy = a->hasMultipleRefs();
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
typename ShuffleReturn<setRef>::return_type                  \
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
bool issetEmptyElemImpl(TypedValue* base,
                        key_type<keyType> key,
                        MInstrState* mis) {
  // mis == nullptr if we proved that it won't be used.  mis->tvRef is ok
  // because it gets passed by reference.
  return HPHP::IssetEmptyElem<isEmpty, keyType>(mis->tvRef, base, key);
}

#define ISSET_EMPTY_ELEM_HELPER_TABLE(m)        \
  /* name         keyType     isEmpty */        \
  m(issetElemC,   KeyType::Any, false)          \
  m(issetElemCE,  KeyType::Any,  true)          \
  m(issetElemI,   KeyType::Int, false)          \
  m(issetElemIE,  KeyType::Int,  true)          \
  m(issetElemS,   KeyType::Str, false)          \
  m(issetElemSE,  KeyType::Str,  true)

#define X(nm, kt, isEmpty)                                              \
inline uint64_t nm(TypedValue* base, key_type<kt> key, MInstrState* mis) { \
  return issetEmptyElemImpl<kt, isEmpty>(base, key, mis);               \
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
