/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "runtime/base/execution_context.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/zend/zend_string.h"
#include "runtime/base/array/hphp_array.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/ext/ext_continuation.h"
#include "runtime/ext/ext_collection.h"
#include "runtime/vm/core_types.h"
#include "runtime/vm/bytecode.h"
#include "runtime/vm/repo.h"
#include "util/trace.h"
#include "runtime.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/vm/translator/translator-x64.h"

#include "runtime/base/zend/zend_functions.h"
#include "runtime/ext/profile/extprofile_string.h"

namespace HPHP {
namespace VM {

static const Trace::Module TRACEMOD = Trace::runtime;

CompileStringFn g_hphp_compiler_parse;
BuildNativeFuncUnitFn g_hphp_build_native_func_unit;
BuildNativeClassUnitFn g_hphp_build_native_class_unit;

/**
 * print_string will decRef the string
 */
void print_string(StringData* s) {
  g_context->write(s->data(), s->size());
  TRACE(1, "t-x64 output(str): (%p) %43s\n", s->data(),
        Util::escapeStringForCPP(s->data(), s->size()).data());
  decRefStr(s);
}

void print_int(int64 i) {
  char buf[256];
  snprintf(buf, 256, "%lld", i);
  echo(buf);
  TRACE(1, "t-x64 output(int): %lld\n", i);
}

void print_boolean(int64 val) {
  if (val) {
    echo("1");
  }
}

/*
 * iter_value_cell* will store a copy of the current value at the address
 * given by 'out'. iter_value_cell* will increment the refcount of the current
 * value if appropriate.
 *
 * This function has been split into hot and cold parts. The hot part has
 * been carefully crafted so that it's a leaf function (after all functions
 * it calls have been trivially inlined) that then tail calls a cold
 * version of itself (new_value_cell_cold). The hot part should cover the
 * common case, which occurs when the array parameter is an HphpArray.
 * If you make any changes to this function, please keep the hot/cold
 * splitting in mind, and disasemble the optimized version of the binary
 * to make sure the hot part is a good-looking leaf function; otherwise,
 * you're likely to get a performance regression.
 */
template <bool typeArray>
static inline void iter_value_cell_impl(Iter* iter, TypedValue* out) {
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  ASSERT((typeArray && iter->m_itype == Iter::TypeArray) ||
         (!typeArray && iter->m_itype == Iter::TypeIterator));
  ArrayIter& arrIter = iter->arr();
  if (typeArray) {
    TypedValue* cur = tvToCell(arrIter.nvSecond());
    // Old value is decref'd in iter_value_cell_local_impl.
    tvDupCell(cur, out);
    return;
  }
  Variant val = arrIter.second();
  ASSERT(val.getRawType() != KindOfRef);
  // Old value is decref'd in iter_value_cell_local_impl.
  tvDupCell((TypedValue*)&val, out);
}

template <bool typeArray>
static inline void iter_value_cell_local_impl(Iter* iter, TypedValue* out) {
  DataType oldType = out->m_type;
  ASSERT(oldType != KindOfRef);
  uint64_t oldDatum = out->m_data.num;
  iter_value_cell_impl<typeArray>(iter, out);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

template <bool typeArray>
static inline void iter_key_cell_impl(Iter* iter, TypedValue* out) {
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  ASSERT((typeArray && iter->m_itype == Iter::TypeArray) ||
         (!typeArray && iter->m_itype == Iter::TypeIterator));
  ArrayIter& arr = iter->arr();
  if (typeArray) {
    // Old value is decref'd in iter_key_cell_local_impl
    arr.nvFirst(out);
    return;
  }
  Variant key = arr.first();
  tvDupCell((TypedValue*)&key, out);
}

template <bool typeArray>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  DataType oldType = out->m_type;
  ASSERT(oldType != KindOfRef);
  uint64_t oldDatum = out->m_data.num;
  iter_key_cell_impl<typeArray>(iter, out);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/**
 * new_iter_array creates an iterator for the specified array iff the array is
 * not empty. If new_iter_array creates an iterator, it does not increment the
 * refcount of the specified array. If new_iter_array does not create an
 * iterator, it decRefs the array.
 */
static NEVER_INLINE
int64 new_iter_array_cold(Iter* dest, ArrayData* arr, TypedValue* valOut,
                          TypedValue* keyOut) {
  TRACE(2, "%s: I %p, arr %p\n", __func__, dest, arr);
  if (!arr->empty()) {
    // We are transferring ownership of the array to the iterator, therefore
    // we do not need to adjust the refcount.
    (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noInc);
    dest->m_itype = Iter::TypeArray;
    iter_value_cell_local_impl<true>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(dest, keyOut);
    }
    return 1LL;
  }
  // We did not transfer ownership of the array to an iterator, so we need
  // to decRef the array.
  decRefArr(arr);
  return 0LL;
}

static inline void getHphpArrayElm(HphpArray::Elm* elm, TypedValue* valOut,
                                   TypedValue* keyOut) {
  TypedValue* cur = tvToCell(&elm->data);
  tvDupCell(cur, valOut);
  if (keyOut) {
    HphpArray::getElmKey(elm, keyOut);
  }
}

static inline bool willBeReleased(TypedValue* tv) {
  if (!IS_REFCOUNTED_TYPE(tv->m_type)) return false;
  if (tv->m_data.pind->_count > 1) return false;
  return true;
}

HOT_FUNC
int64 new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  valOut = tvToCell(valOut);
  if (UNLIKELY(!IsHphpArray(ad))) {
    goto cold;
  }
  {
    HphpArray* arr = (HphpArray*)ad;
    if (LIKELY(arr->getSize() != 0)) {
      if (IS_REFCOUNTED_TYPE(valOut->m_type)) {
        if (UNLIKELY(valOut->m_data.pind->_count == 1)) {
          goto cold;
        }
        valOut->m_data.pstr->decRefCount();
      }
      // We are transferring ownership of the array to the iterator, therefore
      // we do not need to adjust the refcount.
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noIncNonNull);
      dest->m_itype = Iter::TypeArray;
      HphpArray::Elm* elm = arr->getElm(dest->arr().m_pos);
      getHphpArrayElm(elm, valOut, NULL);
      return 1LL;
    }
    // We did not transfer ownership of the array to an iterator, so we need
    // to decRef the array.
    if (UNLIKELY(arr->getCount() == 1)) {
      goto cold;
    }
    arr->decRefCount();
    return 0LL;
  }
cold:
  return new_iter_array_cold(dest, ad, valOut, NULL);
}

HOT_FUNC
int64 new_iter_array_key(Iter* dest, ArrayData* ad, TypedValue* valOut,
                         TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  valOut = tvToCell(valOut);
  keyOut = tvToCell(keyOut);
  if (UNLIKELY(!IsHphpArray(ad))) {
    goto cold;
  }
  {
    HphpArray* arr = (HphpArray*)ad;
    if (LIKELY(arr->getSize() != 0)) {
      if (UNLIKELY(willBeReleased(valOut)) || willBeReleased(keyOut)) {
        goto cold;
      }
      if (IS_REFCOUNTED_TYPE(valOut->m_type)) {
        valOut->m_data.pstr->decRefCount();
      }
      if (IS_REFCOUNTED_TYPE(keyOut->m_type)) {
        keyOut->m_data.pstr->decRefCount();
      }
      // We are transferring ownership of the array to the iterator, therefore
      // we do not need to adjust the refcount.
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noIncNonNull);
      dest->m_itype = Iter::TypeArray;
      HphpArray::Elm* elm = arr->getElm(dest->arr().m_pos);
      getHphpArrayElm(elm, valOut, keyOut);
      return 1LL;
    }
    // We did not transfer ownership of the array to an iterator, so we need
    // to decRef the array.
    if (UNLIKELY(arr->getCount() == 1)) {
      goto cold;
    }
    arr->decRefCount();
    return 0LL;
  }
cold:
  return new_iter_array_cold(dest, ad, valOut, keyOut);
}

class FreeObj {
 public:
  FreeObj() : m_obj(0) {}
  void operator=(ObjectData* obj) { m_obj = obj; }
  ~FreeObj() { if (UNLIKELY(m_obj != NULL)) decRefObj(m_obj); }
 private:
  ObjectData* m_obj;
};

/**
 * new_iter_object creates an iterator for the specified object if the object
 * is iterable and it is non-empty (has properties). If new_iter_object creates
 * an iterator, it does not increment the refcount of the specified object. If
 * new_iter_object does not create an iterator, it decRefs the object.
 *
 * If exceptions are thrown, new_iter_object takes care of decRefing the object.
 */
HOT_FUNC
int64 new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                      TypedValue* valOut, TypedValue* keyOut) {
  valOut = tvToCell(valOut);
  if (keyOut) {
    keyOut = tvToCell(keyOut);
  }
  Iter::Type itType;
  {
    FreeObj fo;
    if (obj->isCollection() || obj->implementsIterator()) {
      TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator\n",
            __func__, dest, obj, ctx);
      try {
        (void) new (&dest->arr()) ArrayIter(obj, ArrayIter::noInc);
      } catch (...) {
        decRefObj(obj);
        throw;
      }
      itType = Iter::TypeIterator;
    } else {
      bool isIteratorAggregate;
      /*
       * We are not going to transfer ownership of obj to the iterator,
       * so arrange to decRef it later. The actual decRef has to happen
       * after the call to arr().end() below, because both can have visible side
       * effects (calls to __destruct() and valid()). Similarly it has to
       * happen before the iter_*_cell_local_impl calls below, because they call
       * current() and key() (hence the explicit scope around FreeObj fo;)
       */
      fo = obj;

      Object itObj = obj->iterableObject(isIteratorAggregate, false);
      if (isIteratorAggregate) {
        TRACE(2, "%s: I %p, obj %p, ctx %p, IteratorAggregate\n",
              __func__, dest, obj, ctx);
        (void) new (&dest->arr()) ArrayIter(itObj, ArrayIter::transferOwner);
        itType = Iter::TypeIterator;
      } else {
        TRACE(2, "%s: I %p, obj %p, ctx %p, iterate as array\n",
              __func__, dest, obj, ctx);
        CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
        Array iterArray(itObj->o_toIterArray(ctxStr));
        ArrayData* ad = iterArray.getArrayData();
        (void) new (&dest->arr()) ArrayIter(ad);
        itType = Iter::TypeArray;
      }
    }
    try {
      if (dest->arr().end()) {
        // Iterator was empty; call the destructor on the iterator we just
        // constructed.
        dest->arr().~ArrayIter();
        return 0LL;
      }
    } catch (...) {
      dest->arr().~ArrayIter();
      throw;
    }
  }

  dest->m_itype = itType;
  if (itType == Iter::TypeIterator) {
    iter_value_cell_local_impl<false>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false>(dest, keyOut);
    }
  } else {
    iter_value_cell_local_impl<true>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(dest, keyOut);
    }
  }
  return 1LL;
}

/**
 * iter_next will advance the iterator to point to the next element.
 * If the iterator reaches the end, iter_next will free the iterator
 * and will decRef the array.
 * This function has been split into hot and cold parts. The hot part has
 * been carefully crafted so that it's a leaf function (after all functions
 * it calls have been trivially inlined) that then tail calls a cold
 * version of itself (iter_next_array_cold). The hot part should cover the
 * common case, which occurs when the array parameter is an HphpArray.
 * If you make any changes to this function, please keep the hot/cold
 * splitting in mind, and disasemble the optimized version of the binary
 * to make sure the hot part is a good-looking leaf function; otherwise,
 * you're likely to get a performance regression.
 */
static NEVER_INLINE
int64 iter_next_cold(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_cold: I %p\n", iter);
  ASSERT(iter->m_itype == Iter::TypeArray ||
         iter->m_itype == Iter::TypeIterator);
  ArrayIter* ai = &iter->arr();
  ai->next();
  if (ai->end()) {
    // The ArrayIter destructor will decRef the array
    ai->~ArrayIter();
    iter->m_itype = Iter::TypeUndefined;
    return 0;
  }
  if (iter->m_itype == Iter::TypeArray) {
    iter_value_cell_local_impl<true>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(iter, keyOut);
    }
  } else {
    iter_value_cell_local_impl<false>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false>(iter, keyOut);
    }
  }
  return 1;
}

HOT_FUNC
int64 iter_next(Iter* iter, TypedValue* valOut) {
  TRACE(2, "iter_next: I %p\n", iter);
  ASSERT(iter->m_itype == Iter::TypeArray ||
         iter->m_itype == Iter::TypeIterator);
  ArrayIter* arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  if (UNLIKELY(!arrIter->hasArrayData())) {
    goto cold;
  }
  {
    const ArrayData* ad = arrIter->getArrayData();
    if (UNLIKELY(!IsHphpArray(ad))) {
      goto cold;
    }
    const HphpArray* arr = (HphpArray*)ad;
    ssize_t pos = arrIter->getPos();
    if (size_t(pos) >= size_t(arr->getLastE())) {
      if (UNLIKELY(arr->getCount() == 1)) {
        goto cold;
      }
      arr->decRefCount();
      iter->m_itype = Iter::TypeUndefined;
      return 0;
    }
    pos = pos + 1;
    HphpArray::Elm* elm = arr->getElm(pos);
    if (UNLIKELY(elm->data.m_type >= HphpArray::KindOfTombstone)) {
      goto cold;
    }
    if (IS_REFCOUNTED_TYPE(valOut->m_type)) {
      if (UNLIKELY(valOut->m_data.pind->_count == 1)) {
        goto cold;
      }
      valOut->m_data.pstr->decRefCount();
    }
    arrIter->setPos(pos);
    getHphpArrayElm(elm, valOut, NULL);
    return 1;
  }
cold:
  return iter_next_cold(iter, valOut, NULL);
}

HOT_FUNC
int64 iter_next_key(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next: I %p\n", iter);
  ASSERT(iter->m_itype == Iter::TypeArray ||
         iter->m_itype == Iter::TypeIterator);
  ArrayIter* arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  keyOut = tvToCell(keyOut);
  if (UNLIKELY(!arrIter->hasArrayData())) {
    goto cold;
  }
  {
    const ArrayData* ad = arrIter->getArrayData();
    if (UNLIKELY(!IsHphpArray(ad))) {
      goto cold;
    }
    const HphpArray* arr = (HphpArray*)ad;
    ssize_t pos = arrIter->getPos();
    if (size_t(pos) >= size_t(arr->getLastE())) {
      if (UNLIKELY(arr->getCount() == 1)) {
        goto cold;
      }
      arr->decRefCount();
      iter->m_itype = Iter::TypeUndefined;
      return 0;
    }
    pos = pos + 1;
    HphpArray::Elm* elm = arr->getElm(pos);
    if (UNLIKELY(elm->data.m_type >= HphpArray::KindOfTombstone)) {
      goto cold;
    }
    if (IS_REFCOUNTED_TYPE(valOut->m_type)) {
      if (UNLIKELY(valOut->m_data.pind->_count == 1)) {
        goto cold;
      }
      valOut->m_data.pstr->decRefCount();
    }
    if (IS_REFCOUNTED_TYPE(keyOut->m_type)) {
      if (UNLIKELY(keyOut->m_data.pind->_count == 1)) {
        if (IS_REFCOUNTED_TYPE(valOut->m_type)) {
          valOut->m_data.pstr->incRefCount();
        }
        goto cold;
      }
      keyOut->m_data.pstr->decRefCount();
    }
    arrIter->setPos(pos);
    getHphpArrayElm(elm, valOut, keyOut);
    return 1;
  }
cold:
  return iter_next_cold(iter, valOut, keyOut);
}

HOT_FUNC_VM
ArrayData* new_array(int capacity) {
  ArrayData *a = NEW(HphpArray)(capacity);
  a->incRefCount();
  TRACE(2, "newArrayHelper: capacity %d\n", capacity);
  return a;
}

ArrayData* new_tuple(int n, const TypedValue* values) {
  HphpArray* a = NEW(HphpArray)(n, values);
  a->incRefCount();
  TRACE(2, "newTupleHelper: size %d\n", n);
  return a;
}

#define NEW_COLLECTION_HELPER(name) \
  ObjectData* \
  new##name##Helper(int nElems) { \
    ObjectData *obj = NEWOBJ(c_##name)(); \
    obj->incRefCount(); \
    if (nElems) { \
      collectionReserve(obj, nElems); \
    } \
    TRACE(2, "new" #name "Helper: capacity %d\n", nElems); \
    return obj; \
  }

NEW_COLLECTION_HELPER(Vector)
NEW_COLLECTION_HELPER(Map)
NEW_COLLECTION_HELPER(StableMap)

#undef NEW_COLLECTION_HELPER

static inline void
tvPairToCString(DataType t, uint64_t v,
                const char** outStr,
                size_t* outSz,
                bool* outMustFree) {
  if (IS_STRING_TYPE(t)) {
    StringData *strd = (StringData*)v;
    *outStr = strd->data();
    *outSz = strd->size();
    *outMustFree = false;
    return;
  }
  Cell c;
  c.m_type = t;
  c.m_data.num = v;
  String s = tvAsVariant(&c).toString();
  *outStr = (const char*)malloc(s.size());
  TRACE(1, "t-x64: stringified: %s -> %s\n", s.data(), *outStr);
  memcpy((char*)*outStr, s.data(), s.size());
  *outSz = s.size();
  *outMustFree = true;
}

/**
 * concat_ss will decRef the values passed in as appropriate, and it will
 * incRef the output string
 */
StringData*
concat_ss(StringData* v1, StringData* v2) {
  if (v1->getCount() > 1) {
    StringData* ret = NEW(StringData)(v1, v2);
    ret->setRefCount(1);
    decRefStr(v2);
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  } else {
    v1->append(v2->slice());
    decRefStr(v2);
    return v1;
  }
}

/**
 * concat_is will decRef the string passed in as appropriate, and it will
 * incRef the output string
 */
StringData*
concat_is(int64 v1, StringData* v2) {
  int len1;
  char intbuf[21];
  char* intstart;
  // Convert the int to a string
  {
    int is_negative;
    intstart = conv_10(v1, &is_negative, intbuf + sizeof(intbuf), &len1);
  }
  StringSlice s1(intstart, len1);
  StringSlice s2 = v2->slice();
  StringData* ret = NEW(StringData)(s1, s2);
  ret->incRefCount();
  decRefStr(v2);
  return ret;
}

/**
 * concat_si will decRef the string passed in as appropriate, and it will
 * incRef the output string
 */
StringData*
concat_si(StringData* v1, int64 v2) {
  int len2;
  char intbuf[21];
  char* intstart;
  // Convert the int to a string
  {
    int is_negative;
    intstart = conv_10(v2, &is_negative, intbuf + sizeof(intbuf), &len2);
  }
  StringSlice s1 = v1->slice();
  StringSlice s2(intstart, len2);
  StringData* ret = NEW(StringData)(s1, s2);
  ret->incRefCount();
  decRefStr(v1);
  return ret;
}

/**
 * concat will decRef the values passed in as appropriate, and it will
 * incRef the output string
 */
StringData*
concat(DataType t1, uint64 v1, DataType t2, uint64 v2) {
  const char *s1, *s2;
  size_t s1len, s2len;
  bool free1, free2;
  tvPairToCString(t1, v1, &s1, &s1len, &free1);
  tvPairToCString(t2, v2, &s2, &s2len, &free2);
  StringSlice r1(s1, s1len);
  StringSlice r2(s2, s2len);
  StringData* retval = NEW(StringData)(r1, r2);
  retval->incRefCount();
  // If tvPairToCString allocated temporary buffers, free them now
  if (free1) free((void*)s1);
  if (free2) free((void*)s2);
  // decRef the parameters as appropriate
  tvRefcountedDecRefHelper(t2, v2);
  tvRefcountedDecRefHelper(t1, v1);

  return retval;
}

int64 eq_null_str(StringData* v1) {
  int64 retval = v1->empty();
  decRefStr(v1);
  return retval;
}

int64 eq_bool_str(int64 v1, StringData* v2) {
  // The truth table for v2->toBoolean() ? v1 : !v1
  //   looks like:
  //      \ v2:0 | v2:1
  // v1:0 |   1  |   0
  // v1:1 |   0  |   1
  //
  // which is nothing but nxor.
  int64 v2i = int64(v2->toBoolean());
  ASSERT(v2i == 0ll || v2i == 1ll);
  ASSERT(v1  == 0ll || v1  == 1ll);
  int64 retval = (v2i ^ v1) ^ 1;
  ASSERT(retval == 0ll || retval == 1ll);
  decRefStr(v2);
  return retval;
}

int64 eq_int_str(int64 v1, StringData* v2) {
  int64 lval; double dval;
  DataType ret = is_numeric_string(v2->data(), v2->size(), &lval, &dval, 1);
  decRefStr(v2);
  if (ret == KindOfInt64) {
    return v1 == lval;
  } else if (ret == KindOfDouble) {
    return (double)v1 == dval;
  } else {
    return v1 == 0;
  }
}

int64 eq_str_str(StringData* v1, StringData* v2) {
  int64 retval = v1->equal(v2);
  decRefStr(v2);
  decRefStr(v1);
  return retval;
}

int64 same_str_str(StringData* v1, StringData* v2) {
  int64 retval = v1 == v2 || v1->same(v2);
  decRefStr(v2);
  decRefStr(v1);
  return retval;
}

int64 str0_to_bool(StringData* sd) {
  int64 retval = sd->toBoolean();
  return retval;
}

int64 str_to_bool(StringData* sd) {
  int64 retval = str0_to_bool(sd);
  decRefStr(sd);
  return retval;
}

int64 arr0_to_bool(ArrayData* ad) {
  return ad->size() != 0;
}

int64 arr_to_bool(ArrayData* ad) {
  ASSERT(Transl::tx64->stateIsDirty());
  int64 retval = arr0_to_bool(ad);
  decRefArr(ad);
  return retval;
}

/**
 * tv_to_bool will decrement tv's refcount if tv is a refcounted type
 */
int64
tv_to_bool(TypedValue* tv) {
  using std::string;
  bool retval;
  if (IS_STRING_TYPE(tv->m_type)) {
    StringData* sd = tv->m_data.pstr;
    retval = bool(str0_to_bool(sd));
  } else if (tv->m_type == KindOfArray) {
    ArrayData* ad = tv->m_data.parr;
    retval = bool(arr0_to_bool(ad));
  } else {
    retval = bool(tvAsCVarRef(tv));
  }
  TRACE(2, Trace::prettyNode("TvToBool", *tv) + string(" -> ") +
        string(retval ? "t" : "f") + string("\n"));
  tvRefcountedDecRef(tv);
  return int64(retval);
}

void
tv_release_str(StringData* datum) {
  datum->release();
}

void
tv_release_arr(ArrayData* datum) {
  ASSERT(Transl::tx64->stateIsDirty());
  datum->release();
}

void
tv_release_obj(ObjectData* datum) {
  ASSERT(Transl::tx64->stateIsDirty());
  datum->release();
}

void
tv_release_ref(RefData* datum) {
  ASSERT(Transl::tx64->stateIsDirty());
  datum->release();
}

Unit* compile_file(const char* s, size_t sz, const MD5& md5,
                   const char* fname) {
  return g_hphp_compiler_parse(s, sz, md5, fname);
}

Unit* build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                             ssize_t numBuiltinFuncs) {
  return g_hphp_build_native_func_unit(builtinFuncs, numBuiltinFuncs);
}

Unit* build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                              ssize_t numBuiltinClasses) {
  return g_hphp_build_native_class_unit(builtinClasses, numBuiltinClasses);
}

Unit* compile_string(const char* s, size_t sz) {
  MD5 md5;
  int out_len;
  md5 = MD5(string_md5(s, sz, false, out_len));

  VM::Unit* u = Repo::get().loadUnit("", md5);
  if (u != NULL) {
    return u;
  }
  return g_hphp_compiler_parse(s, sz, md5, NULL);
}

// Returned array has refcount zero! Caller must refcount.
HphpArray* pack_args_into_array(ActRec* ar, int nargs) {
  HphpArray* argArray = NEW(HphpArray)(nargs);
  for (int i = 0; i < nargs; ++i) {
    TypedValue* tv = (TypedValue*)(ar) - (i+1);
    argArray->HphpArray::appendWithRef(tvAsCVarRef(tv), false);
  }
  if (!ar->hasInvName()) {
    // If this is not a magic call, we're done
    return argArray;
  }
  // This is a magic call, so we need to shuffle the args
  HphpArray* magicArgs = NEW(HphpArray)(2);
  magicArgs->append(ar->getInvName(), false);
  magicArgs->append(argArray, false);
  return magicArgs;
}

bool run_intercept_handler_for_invokefunc(TypedValue* retval,
                                          const Func* f,
                                          CArrRef params,
                                          ObjectData* this_,
                                          StringData* invName,
                                          Variant* ihandler) {
  using namespace HPHP::VM::Transl;
  ASSERT(ihandler);
  ASSERT(retval);
  Variant doneFlag = true;
  Array args = params;
  if (invName) {
    // This is a magic call, so we need to shuffle the args
    HphpArray* magicArgs = NEW(HphpArray)(2);
    magicArgs->append(invName, false);
    magicArgs->append(params, false);
    args = magicArgs;
  }
  Array intArgs =
    CREATE_VECTOR5(f->fullNameRef(), (this_ ? Variant(Object(this_)) : null),
                   args, ihandler->asCArrRef()[1], ref(doneFlag));
  call_intercept_handler<false>(retval, intArgs, NULL, ihandler);
  // $done is true, meaning don't enter the intercepted function.
  return !doneFlag.toBoolean();
}

HphpArray* get_static_locals(const ActRec* ar) {
  if (ar->m_func->isClosureBody()) {
    static const StringData* s___static_locals =
      StringData::GetStaticString("__static_locals");
    ASSERT(ar->hasThis());
    ObjectData* closureObj = ar->getThis();
    ASSERT(closureObj);
    TypedValue* prop;
    TypedValue ref;
    tvWriteUninit(&ref);
    static_cast<Instance*>(closureObj)->prop(
      prop,
      ref,
      closureObj->getVMClass(),
      s___static_locals);
    if (prop->m_type == KindOfNull) {
      prop->m_data.parr = NEW(HphpArray)(1);
      prop->m_data.parr->incRefCount();
      prop->m_type = KindOfArray;
    }
    ASSERT(prop->m_type == KindOfArray);
    ASSERT(IsHphpArray(prop->m_data.parr));
    ASSERT(ref.m_type == KindOfUninit);
    return static_cast<HphpArray*>(prop->m_data.parr);
  } else if (ar->m_func->isGeneratorFromClosure()) {
    TypedValue* contLoc = frame_local(ar, 0);
    c_Continuation* cont = static_cast<c_Continuation*>(contLoc->m_data.pobj);
    ASSERT(cont != NULL);
    return cont->getStaticLocals();
  } else {
    return ar->m_func->getStaticLocals();
  }
}

void collection_setm_wk1_v0(ObjectData* obj, TypedValue* value) {
  ASSERT(obj);
  collectionAppend(obj, value);
  // TODO Task #1970153: It would be great if we had a version of
  // collectionAppend() that didn't incRef the value so that we
  // wouldn't have to decRef it here
  tvRefcountedDecRef(value);
}

void collection_setm_ik1_v0(ObjectData* obj, int64 key, TypedValue* value) {
  ASSERT(obj);
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    c_Vector* vec = static_cast<c_Vector*>(obj);
    vec->put(key, value);
  } else if (ct == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    mp->put(key, value);
  } else if (ct == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    smp->put(key, value);
  } else {
    ASSERT(false);
  }
  tvRefcountedDecRef(value);
}

void collection_setm_sk1_v0(ObjectData* obj, StringData* key,
                            TypedValue* value) {
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Only integer keys may be used with Vectors"));
    throw e;
  } else if (ct == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    mp->put(key, value);
  } else if (ct == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    smp->put(key, value);
  } else {
    ASSERT(false);
  }
  tvRefcountedDecRef(value);
}

} } // HPHP::VM

