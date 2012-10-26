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
#include "runtime/base/tv_macros.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/zend/zend_string.h"
#include "runtime/base/array/hphp_array.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/ext/ext_continuation.h"
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

/**
 * print_string will decRef the string
 */
void print_string(StringData* s) {
  g_context->write(s->data(), s->size());
  TRACE(1, "t-x64 output(str): (%p) %43s\n", s->data(),
        Util::escapeStringForCPP(s->data(), s->size()).data());
  // decRef the string
  if (s->decRefCount() == 0) s->release();
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

/**
 * new_iter_array creates an iterator for the specified array iff the array is
 * not empty. If new_iter_array creates an iterator, it does not increment the
 * refcount of the specified array. If new_iter_array does not create an
 * iterator, it decRefs the array.
 */
HOT_FUNC
int64 new_iter_array(Iter* dest, HphpArray* arr) {
  TRACE(2, "%s: I %p, arr %p\n", __func__, dest, arr);
  if (!arr->empty()) {
    // We are transferring ownership of the array to the iterator, therefore
    // we do not need to adjust the refcount.
    (void) new (&dest->arr()) ArrayIter(arr, 0);
    dest->m_itype = Iter::TypeArray;
    return 1LL;
  }
  // We did not transfer ownership of the array to an iterator, so we need
  // to decRef the array.
  if (arr->decRefCount() == 0) arr->release();
  return 0LL;
}

/**
 * new_iter_object creates an iterator for the specified object if the object
 * is iterable and it is non-empty (has properties). If new_iter_object creates
 * an iterator, it does not increment the refcount of the specified object. If
 * new_iter_object does not create an iterator, it decRefs the object.
 */
HOT_FUNC
int64 new_iter_object(Iter* dest, ObjectData* obj, Class* ctx) {
  Iter::Type itType;
  if (obj->isCollection() || obj->implementsIterator()) {
    TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator\n",
             __func__, dest, obj, ctx);
    (void) new (&dest->arr()) ArrayIter(obj, 0);
    itType = Iter::TypeIterator;
  } else {
    bool isIteratorAggregate;
    Object itObj = obj->iterableObject(isIteratorAggregate, false);
    if (isIteratorAggregate) {
      TRACE(2, "%s: I %p, obj %p, ctx %p, IteratorAggregate\n",
               __func__, dest, obj, ctx);
      (void) new (&dest->arr()) ArrayIter(itObj.get());
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
    // We did not transfer ownership of the object to an iterator, so we need
    // to decRef the object.
    if (obj->decRefCount() == 0) obj->release();
  }
  if (!dest->arr().end()) {
    dest->m_itype = itType;
    return 1LL;
  }
  // Iterator was empty; call the destructor on the iterator we just
  // constructed.
  dest->arr().~ArrayIter();
  return 0LL;
}

/**
 * iter_next_array will advance the iterator to point to the next element.
 * If the iterator reaches the end, iter_next_array will free the iterator
 * and will decRef the array.
 */
HOT_FUNC
int64 iter_next_array(Iter* iter) {
  TRACE(2, "iter_next_array: I %p\n", iter);
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
  return 1;
}

/**
 * iter_value_cell* will store a copy of the current value at the address
 * given by 'out'. iter_value_cell* will increment the refcount of the current
 * value if appropriate.
 */
template <bool typeArray>
static inline void iter_value_cell_impl(Iter* iter, TypedValue* out) {
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  ASSERT((typeArray && iter->m_itype == Iter::TypeArray) ||
         (!typeArray && iter->m_itype == Iter::TypeIterator));
  ArrayIter& arr = iter->arr();
  if (typeArray) {
    TypedValue* cur = arr.nvSecond();
    if (UNLIKELY(cur->m_type == KindOfRef)) cur = cur->m_data.pref->tv();
    TV_DUP_CELL_NC(cur, out);
    return;
  }
  Variant val = arr.second();
  ASSERT(val.getRawType() != KindOfRef);
  TV_DUP_CELL_NC((TypedValue*)&val, out);
}

HOT_FUNC
void iter_value_cell_array(Iter* iter, TypedValue* out) {
  iter_value_cell_impl<true>(iter, out);
}

HOT_FUNC
void iter_value_cell_iterator(Iter* iter, TypedValue* out) {
  iter_value_cell_impl<false>(iter, out);
}

template <bool typeArray>
static inline void iter_value_cell_local_impl(Iter* iter, TypedValue* out) {
  DataType oldType = out->m_type;
  if (UNLIKELY(oldType == KindOfRef)) {
    out = out->m_data.pref->tv();
    oldType = out->m_type;
  }
  uint64_t oldDatum = out->m_data.num;
  iter_value_cell_impl<typeArray>(iter, out);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

HOT_FUNC
void iter_value_cell_local_array(Iter* iter, TypedValue* out) {
  iter_value_cell_local_impl<true>(iter, out);
}

HOT_FUNC
void iter_value_cell_local_iterator(Iter* iter, TypedValue* out) {
  iter_value_cell_local_impl<false>(iter, out);
}

template <bool typeArray>
static inline void iter_key_cell_impl(Iter* iter, TypedValue* out) {
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  ASSERT((typeArray && iter->m_itype == Iter::TypeArray) ||
         (!typeArray && iter->m_itype == Iter::TypeIterator));
  ArrayIter& arr = iter->arr();
  if (typeArray) {
    arr.nvFirst(out);
    return;
  }
  Variant key = arr.first();
  ASSERT(key.getRawType() == KindOfInt64 || IS_STRING_TYPE(key.getRawType()));
  TV_DUP_CELL_NC((TypedValue*)&key, out);
}

HOT_FUNC
void iter_key_cell_array(Iter* iter, TypedValue* out) {
  iter_key_cell_impl<true>(iter, out);
}

HOT_FUNC
void iter_key_cell_iterator(Iter* iter, TypedValue* out) {
  iter_key_cell_impl<false>(iter, out);
}

template <bool typeArray>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  DataType oldType = out->m_type;
  if (UNLIKELY(oldType == KindOfRef)) {
    out = out->m_data.pref->tv();
    oldType = out->m_type;
  }
  uint64_t oldDatum = out->m_data.num;
  iter_key_cell_impl<typeArray>(iter, out);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

HOT_FUNC
void iter_key_cell_local_array(Iter* iter, TypedValue* out) {
  iter_key_cell_local_impl<true>(iter, out);
}

HOT_FUNC
void iter_key_cell_local_iterator(Iter* iter, TypedValue* out) {
  iter_key_cell_local_impl<false>(iter, out);
}

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
    if (v2->decRefCount() == 0) v2->release();
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  } else {
    v1->append(v2->slice());
    if (v2->decRefCount() == 0) v2->release();
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
  if (v2->decRefCount() == 0) v2->release();
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
  if (v1->decRefCount() == 0) v1->release();
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
  // decRef the string
  if (v1->decRefCount() == 0) v1->release();
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
  // decRef the string
  if (v2->decRefCount() == 0) v2->release();
  return retval;
}

int64 eq_int_str(int64 v1, StringData* v2) {
  int64 lval; double dval;
  DataType ret = is_numeric_string(v2->data(), v2->size(), &lval, &dval, 1);
  // decRef the string
  if (v2->decRefCount() == 0) v2->release();
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
  if (v2->decRefCount() == 0) v2->release();
  if (v1->decRefCount() == 0) v1->release();
  return retval;
}

int64 same_str_str(StringData* v1, StringData* v2) {
  int64 retval = v1 == v2 || v1->same(v2);
  if (v2->decRefCount() == 0) v2->release();
  if (v1->decRefCount() == 0) v1->release();
  return retval;
}

int64 str0_to_bool(StringData* sd) {
  int64 retval = sd->toBoolean();
  return retval;
}

int64 str_to_bool(StringData* sd) {
  int64 retval = str0_to_bool(sd);
  if (sd->decRefCount() == 0) sd->release();
  return retval;
}

int64 arr0_to_bool(ArrayData* ad) {
  return ad->size() != 0;
}

int64 arr_to_bool(ArrayData* ad) {
  ASSERT(Transl::tx64->stateIsDirty());
  int64 retval = arr0_to_bool(ad);
  if (ad->decRefCount() == 0) ad->release();
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

void
frame_free_locals(ActRec* fp, int numLocals) {
  ASSERT(Transl::tx64->stateIsDirty());
  using namespace Transl;
#ifdef DEBUG
  VMRegAnchor _;
  ASSERT(vmfp() == (Cell*)fp);
#endif
  // At return-time, we know that the eval stack is empty except
  // for the return value.
  TRACE(1, "frame_free_locals: updated fp to %p\n", fp);
  frame_free_locals_inl(fp, numLocals);
}

void
frame_free_locals_no_this(ActRec* fp, int numLocals) {
  ASSERT(Transl::tx64->stateIsDirty());
  using namespace Transl;
#ifdef DEBUG
  VMRegAnchor _;
  ASSERT(vmfp() == (Cell*)fp);
#endif
  // At return-time, we know that the eval stack is empty except
  // for the return value.
  TRACE(1, "frame_free_locals_no_this: updated fp to %p\n", fp);
  frame_free_locals_no_this_inl(fp, numLocals);
}

Unit* compile_file(const char* s, size_t sz, const MD5& md5,
                   const char* fname) {
  static CompileStringFn compileString =
    (CompileStringFn)dlsym(NULL, "hphp_compiler_parse");
  Unit* retval = compileString(s, sz, md5, fname);
  return retval;
}

Unit* build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                             ssize_t numBuiltinFuncs) {
  static Unit*(*func)(const HhbcExtFuncInfo*, ssize_t) =
    (Unit*(*)(const HhbcExtFuncInfo*, ssize_t))
      dlsym(NULL, "hphp_build_native_func_unit");
  Unit* u = func(builtinFuncs, numBuiltinFuncs);
  return u;
}

Unit* build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                              ssize_t numBuiltinClasses) {
  static Unit*(*func)(const HhbcExtClassInfo*, ssize_t) =
    (Unit*(*)(const HhbcExtClassInfo*, ssize_t))
      dlsym(NULL, "hphp_build_native_class_unit");
  Unit* u = func(builtinClasses, numBuiltinClasses);
  return u;
}

Unit* compile_string(const char* s, size_t sz) {
  MD5 md5;
  int out_len;
  md5 = MD5(string_md5(s, sz, false, out_len));

  VM::Unit* u = Repo::get().loadUnit("", md5);
  if (u != NULL) {
    return u;
  }
  static CompileStringFn compileString =
    (CompileStringFn)dlsym(NULL, "hphp_compiler_parse");
  u = compileString(s, sz, md5, NULL);
  return u;
}

// Returned array has refcount zero! Caller must refcount.
HphpArray* pack_args_into_array(ActRec* ar, int nargs) {
  HphpArray* argArray = NEW(HphpArray)(nargs);
  for (int i = 0; i < nargs; ++i) {
    TypedValue* tv = (TypedValue*)(ar) - (i+1);
    argArray->nvAppendWithRef(tv);
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

} } // HPHP::VM

