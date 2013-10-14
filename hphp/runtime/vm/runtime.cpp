/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/ext/ext_string.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

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

void print_int(int64_t i) {
  char buf[256];
  snprintf(buf, 256, "%" PRId64, i);
  echo(buf);
  TRACE(1, "t-x64 output(int): %" PRId64 "\n", i);
}

void print_boolean(bool val) {
  if (val) {
    echo("1");
  }
}

#define NEW_COLLECTION_HELPER(name) \
  ObjectData* \
  new##name##Helper(int nElms) { \
    ObjectData *obj = NEWOBJ(c_##name)(); \
    obj->incRefCount(); \
    if (nElms) { \
      collectionReserve(obj, nElms); \
    } \
    TRACE(2, "new" #name "Helper: capacity %d\n", nElms); \
    return obj; \
  }

NEW_COLLECTION_HELPER(Vector)
NEW_COLLECTION_HELPER(Map)
NEW_COLLECTION_HELPER(StableMap)
NEW_COLLECTION_HELPER(Set)

ObjectData* newPairHelper() {
  ObjectData *obj = NEWOBJ(c_Pair)();
  obj->incRefCount();
  TRACE(2, "newPairHelper: capacity 2\n");
  return obj;
}

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
 * concat_ss will will incRef the output string
 * and decref its first argument
 */
StringData*
concat_ss(StringData* v1, StringData* v2) {
  if (v1->hasMultipleRefs()) {
    StringData* ret = StringData::Make(v1, v2);
    ret->setRefCount(1);
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  }

  auto const newV1 = v1->append(v2->slice());
  if (UNLIKELY(newV1 != v1)) {
    assert(v1->getCount() == 1);
    v1->release();
    newV1->incRefCount();
    return newV1;
  }
  return v1;
}

/**
 * concat_is will incRef the output string
 */
StringData*
concat_is(int64_t v1, StringData* v2) {
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
  StringData* ret = StringData::Make(s1, s2);
  ret->incRefCount();
  return ret;
}

/**
 * concat_si will incRef the output string
 * and decref its first argument
 */
StringData*
concat_si(StringData* v1, int64_t v2) {
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
  StringData* ret = StringData::Make(s1, s2);
  ret->incRefCount();
  decRefStr(v1);
  return ret;
}

int64_t eq_null_str(StringData* v1) {
  int64_t retval = v1->empty();
  decRefStr(v1);
  return retval;
}

int64_t eq_bool_str(int64_t v1, StringData* v2) {
  // The truth table for v2->toBoolean() ? v1 : !v1
  //   looks like:
  //      \ v2:0 | v2:1
  // v1:0 |   1  |   0
  // v1:1 |   0  |   1
  //
  // which is nothing but nxor.
  int64_t v2i = int64_t(v2->toBoolean());
  assert(v2i == 0ll || v2i == 1ll);
  assert(v1  == 0ll || v1  == 1ll);
  int64_t retval = (v2i ^ v1) ^ 1;
  assert(retval == 0ll || retval == 1ll);
  decRefStr(v2);
  return retval;
}

int64_t eq_int_str(int64_t v1, StringData* v2) {
  int64_t lval; double dval;
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

int64_t eq_str_str(StringData* v1, StringData* v2) {
  int64_t retval = v1->equal(v2);
  decRefStr(v2);
  decRefStr(v1);
  return retval;
}

int64_t same_str_str(StringData* v1, StringData* v2) {
  int64_t retval = v1 == v2 || v1->same(v2);
  decRefStr(v2);
  decRefStr(v1);
  return retval;
}

int64_t str0_to_bool(StringData* sd) {
  int64_t retval = sd->toBoolean();
  return retval;
}

int64_t str_to_bool(StringData* sd) {
  int64_t retval = str0_to_bool(sd);
  decRefStr(sd);
  return retval;
}

int64_t arr0_to_bool(ArrayData* ad) {
  return ad->size() != 0;
}

int64_t arr_to_bool(ArrayData* ad) {
  assert(Transl::Translator::Get()->stateIsDirty());
  int64_t retval = arr0_to_bool(ad);
  decRefArr(ad);
  return retval;
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

Unit* compile_string(const char* s, size_t sz, const char* fname) {
  MD5 md5;
  int out_len;

  char * md5str = string_md5(s, sz, false, out_len);
  md5 = MD5(md5str);
  free(md5str);

  Unit* u = Repo::get().loadUnit(fname ? fname : "", md5);
  if (u != nullptr) {
    return u;
  }
  return g_hphp_compiler_parse(s, sz, md5, fname);
}

void assertTv(const TypedValue* tv) {
  always_assert(tvIsPlausible(*tv));
}

int init_closure(ActRec* ar, TypedValue* sp) {
  c_Closure* closure = static_cast<c_Closure*>(ar->getThis());

  // Swap in the $this or late bound class
  ar->setThis(closure->getThisOrClass());

  if (ar->hasThis()) {
    ar->getThis()->incRefCount();
  }

  // Put in the correct context
  ar->m_func = closure->getInvokeFunc();

  // The closure is the first local.
  // Similar to tvWriteObject() but we don't incref because it used to be $this
  // and now it is a local, so they cancel out
  TypedValue* firstLocal = --sp;
  firstLocal->m_type = KindOfObject;
  firstLocal->m_data.pobj = closure;

  // Copy in all the use vars
  TypedValue* prop = closure->getUseVars();
  int n = closure->getNumUseVars();
  for (int i=0; i < n; i++) {
    tvDup(*prop++, *--sp);
  }

  return n + 1;
}

void raiseWarning(const StringData* sd) {
  raise_warning("%s", sd->data());
}

HOT_FUNC int64_t modHelper(int64_t left, int64_t right) {
  // We already dealt with divide-by-zero up in hhbctranslator.
  assert(right != 0);
  return left % right;
}

void defClsHelper(PreClass* preClass) {
  using namespace Transl;

  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;
  Unit::defClass(preClass);

  /*
   * UniqueStubs::defClsHelper sync'd the registers for us already.
   * This means if an exception propagates we want to leave things as
   * VMRegState::CLEAN, since we're still in sync.  Only set it to
   * dirty if we are actually returning to run in the TC again.
   */
  tl_regState = VMRegState::DIRTY;
}

} // HPHP::VM

