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
#include "runtime/vm/core_types.h"
#include "runtime/vm/bytecode.h"
#include "util/trace.h"
#include "runtime.h"
#include "runtime/vm/translator/translator-inline.h"

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
 * new_iter creates an iterator for the specified array iff the array is not
 * empty. If new_iter creates an iterator, it does not increment the refcount
 * of the specified array. If new_iter does not create an iterator, it decRefs
 * the array.
 */
int64 new_iter(Iter* dest, HphpArray* arr) {
  TRACE(2, "new_iter: I %p, arr %p\n", dest, arr);
  bool empty = IsHphpArray(arr) ? (arr->nvSize() == 0)
    : arr->empty();
  if (!empty) {
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
 * iter_next_array will advance the iterator to point to the next element.
 * If the iterator reaches the end, iter_next_array will free the iterator
 * and will decRef the array.
 */
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
 * iter_value_cell will store a copy of the current value at the address
 * given by 'out'. iter_value_cell will increment the refcount of the current
 * value if appropriate.
 */
void iter_value_cell(Iter* iter, TypedValue* out) {
  TRACE(2, "iter_value_cell: I %p, out %p\n", iter, out);
  ASSERT(iter->m_itype == Iter::TypeArray ||
         iter->m_itype == Iter::TypeIterator);
  ArrayIter& arr = iter->arr();
  if (LIKELY(arr.isHphpArray())) {
    TypedValue* cur = arr.nvSecond();
    TV_READ_CELL(cur, out);
    return;
  }
  Variant val = arr.second();
  TV_READ_CELL((TypedValue*)&val, out);
}

void iter_key_cell(Iter* iter, TypedValue* out) {
  TRACE(2, "iter_key_cell: I %p, out %p\n", iter, out);
  ASSERT(iter->m_itype == Iter::TypeArray ||
         iter->m_itype == Iter::TypeIterator);
  ArrayIter& arr = iter->arr();
  if (LIKELY(arr.isHphpArray())) {
    arr.nvFirst(out);
    return;
  }
  Variant key = arr.first();
  ASSERT(key.getRawType() == KindOfInt64 || IS_STRING_TYPE(key.getRawType()));
  TV_READ_CELL((TypedValue*)&key, out);
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
    int len1 = v1->size();
    int len2 = v2->size();
    int len = len1 + len2;
    char *buf = (char *)malloc(len + 1);
    if (buf == NULL) {
      throw FatalErrorException(0, "malloc failed: %d", len);
    }
    memcpy(buf, v1->data(), len1);
    // memcpy will copy the NULL-terminator for us
    memcpy(buf + len1, v2->data(), len2+1);
    StringData* ret = NEW(StringData)(buf, len, AttachString);
    ret->incRefCount();
    if (v2->decRefCount() == 0) v2->release();
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  } else {
    v1->append(v2->data(), v2->size());
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
  int len2 = v2->size();
  int len = len1 + len2;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException(0, "malloc failed: %d", len);
  }
  memcpy(buf, intstart, len1);
  // memcpy will copy the NULL-terminator for us
  memcpy(buf + len1, v2->data(), len2+1);
  StringData* ret = NEW(StringData)(buf, len, AttachString);
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
  int len1 = v1->size();
  int len = len1 + len2;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException(0, "malloc failed: %d", len);
  }
  memcpy(buf, v1->data(), len1);
  memcpy(buf + len1, intstart, len2);
  buf[len] = '\0';
  StringData* ret = NEW(StringData)(buf, len, AttachString);
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
  // XXX For now we just use the string_concat defined in
  // "runtime/base/zend/zend_string.h". We may want to
  // replace this with something more efficient in the
  // future
  char *dest;
  int outputLen;
  dest = HPHP::string_concat(s1, s1len, s2, s2len, outputLen);

  StringData* retval = NEW(StringData)(dest, outputLen, AttachString);
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
  int64 retval = v1->empty() ? 1 : 0;
  // decRef the string
  if (v1->decRefCount() == 0) v1->release();
  return retval;
}

int64 eq_bool_str(int64 v1, StringData* v2) {
  ASSERT(v1 == 0 || v1 == 1);
  int64 retval = v2->toBoolean() ? v1 : (v1^1);
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
    return v1 == lval ? 1 : 0;
  } else if (ret == KindOfDouble) {
    return (double)v1 == dval ? 1 : 0;
  } else {
    return v1 == 0 ? 1 : 0;
  }
}

int64 eq_str_str(StringData* v1, StringData* v2) {
  int64 retval = (v1->compare(v2) == 0) ? 1 : 0;
  if (v2->decRefCount() == 0) v2->release();
  if (v1->decRefCount() == 0) v1->release();
  return retval;
}

int64 same_str_str(StringData* v1, StringData* v2) {
  int64 retval;
  ASSERT(v1);
  ASSERT(v2);
  int len = v1->size();
  if (v2->size() != len) {
    retval = false;
  } else if (v1->data() == v2->data()) {
    retval = true;
  } else {
    retval = !memcmp(v1->data(), v2->data(), len);
  }
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
  if (LIKELY(IsHphpArray(ad))) {
    HphpArray* ha = (HphpArray*)ad;
    return (ha->nvSize() != 0);
  } else {
    return (ad->size() != 0);
  }
}

int64 arr_to_bool(ArrayData* ad) {
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
  datum->release();
}

void
tv_release_obj(ObjectData* datum) {
  datum->release();
}

void
tv_release_var(Variant* datum) {
  datum->release();
}

void
tv_release_generic(TypedValue* tv) {
  tvReleaseHelper(tv->m_type, tv->m_data.num);
}

void
frame_free_locals(ActRec* fp) {
  using namespace Transl;
  vmfp() = (Cell*)fp; // VM can be re-entered here.
  // At return-time, we know that the eval stack is empty except
  // for the return value.
  vmsp() = vmfp() - fp->m_func->numSlotsInFrame();
  if (debug) {
    g_context->m_isValid = 1;
  }
  TRACE(1, "frame_free_locals: updated fp to %p\n", fp);
  frame_free_locals_inl(fp);
  if (debug) {
    g_context->m_isValid = 0;
  }
}

Unit* compile_string(const char* s, size_t sz, const char* fname) {
  static CompileStringFn compileString =
    (CompileStringFn)dlsym(NULL, "hphp_compiler_parse");
  Unit* retval = compileString(s, sz, fname);
  retval->setMd5(HPHP::f_md5(String(s)));
  return retval;
}

// Returned array has refcount zero! Caller must refcount.
HphpArray* pack_args_into_array(ActRec* ar, int nargs) {
  HphpArray* argArray = NEW(HphpArray)(nargs);
  for (int i = 0; i < nargs; ++i) {
    TypedValue* tv = (TypedValue*)(ar) - (i+1);
    argArray->nvAppend(tv, false);
  }
  return argArray;
}

// !!!
// The translator relies on the fact that this function can't reenter!
FuncDict::InterceptData* intercept_data(ActRec* ar) {
  if (UNLIKELY(g_context->m_funcDict.hasAnyIntercepts())) {
    return g_context->m_funcDict.getInterceptData(ar->m_func).get();
  }
  return NULL;
}

bool run_intercept_handler(ActRec* ar, FuncDict::InterceptData* data) {
  ASSERT(data);

  Variant doneFlag = true;
  Array args =
    CREATE_VECTOR5(data->m_name,
                   (ar->hasThis() ? Variant(Object(ar->getThis())) : null),
                   Array(pack_args_into_array(ar, ar->m_numArgs)),
                   data->m_data,
                   ref(doneFlag));

  ObjectData* intThis = NULL;
  Class* intCls = NULL;
  StringData* intInvName = NULL;
  const Func* handler = vm_decode_function(data->m_handler, g_context->m_fp,
                                           false, intThis, intCls, intInvName);
  TypedValue retval;
  g_context->invokeFunc(&retval, handler, args,
                        intThis, intCls, NULL, intInvName);

  if (doneFlag.toBoolean()) {
    // $done is true, meaning don't enter the intercepted function. Clean up the
    // args and AR, move the intercept handler's return value to the right
    // place, and get out. This code runs during the frame setup phase for a
    // call; specifically, we haven't pushed iterators, written uninit-null for
    // unpassed args, or moved excess args aside.
    Stack& stack = g_context->m_stack;
    ASSERT((TypedValue*)ar - stack.top() == ar->m_numArgs);

    while (uintptr_t(stack.top()) < uintptr_t(ar)) {
      stack.popTV();
    }
    stack.popAR();
    stack.allocTV();
    memcpy(stack.top(), &retval, sizeof(TypedValue));
    return false;
  }

  return true;
}

} } // HPHP::VM

