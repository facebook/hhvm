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
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/util/trace.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/ext/string/ext_string.h"

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
        escapeStringForCPP(s->data(), s->size()).data());
  decRefStr(s);
}

void print_int(int64_t i) {
  char buf[256];
  snprintf(buf, 256, "%" PRId64, i);
  g_context->write(buf);
  TRACE(1, "t-x64 output(int): %" PRId64 "\n", i);
}

void print_boolean(bool val) {
  if (val) {
    g_context->write("1");
  }
}

/**
 * concat_ss will will incRef the output string
 * and decref its first argument
 */
StringData* concat_ss(StringData* v1, StringData* v2) {
  if (v1->hasMultipleRefs()) {
    StringData* ret = StringData::Make(v1, v2);
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  }

  auto const ret = v1->append(v2->slice());
  if (UNLIKELY(ret != v1)) {
    assert(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

/**
 * concat_is will incRef the output string
 */
StringData* concat_is(int64_t v1, StringData* v2) {
  char intbuf[21];
  // Convert the int to a string
  auto const s1 = conv_10(v1, intbuf + sizeof(intbuf));
  StringSlice s2 = v2->slice();
  return StringData::Make(s1, s2);
}

/**
 * concat_si will incRef the output string
 * and decref its first argument
 */
StringData* concat_si(StringData* v1, int64_t v2) {
  char intbuf[21];
  auto const s2 = conv_10(v2, intbuf + sizeof(intbuf));
  if (v1->hasMultipleRefs()) {
    auto const s1 = v1->slice();
    auto const ret = StringData::Make(s1, s2);
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  }

  auto const ret = v1->append(s2);
  if (UNLIKELY(ret != v1)) {
    assert(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

StringData* concat_s3(StringData* v1, StringData* v2, StringData* v3) {
  if (v1->hasMultipleRefs()) {
    StringData* ret = StringData::Make(
      v1->slice(), v2->slice(), v3->slice());
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  }

  auto const ret = v1->append(v2->slice(), v3->slice());

  if (UNLIKELY(ret != v1)) {
    assert(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

StringData* concat_s4(StringData* v1, StringData* v2,
                      StringData* v3, StringData* v4) {
  if (v1->hasMultipleRefs()) {
    StringData* ret = StringData::Make(
        v1->slice(), v2->slice(), v3->slice(), v4->slice());
    // Because v1->getCount() is greater than 1, we know we will never
    // have to release the string here
    v1->decRefCount();
    return ret;
  }

  auto const ret = v1->append(v2->slice(), v3->slice(), v4->slice());

  if (UNLIKELY(ret != v1)) {
    assert(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
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

Unit* compile_string(const char* s,
                     size_t sz,
                     const char* fname /* = nullptr */) {
  auto md5string = string_md5(s, sz);
  MD5 md5(md5string.c_str());
  Unit* u = Repo::get().loadUnit(fname ? fname : "", md5).release();
  if (u != nullptr) {
    return u;
  }
  // NB: fname needs to be long-lived if generating a bytecode repo because it
  // can be cached via a Location ultimately contained by ErrorInfo for printing
  // code errors.
  return g_hphp_compiler_parse(s, sz, md5, fname);
}

Unit* compile_systemlib_string(const char* s, size_t sz,
                               const char* fname) {
  if (RuntimeOption::RepoAuthoritative) {
    String systemName = String("/:") + String(fname);
    MD5 md5;
    if (Repo::get().findFile(systemName.data(),
                             SourceRootInfo::GetCurrentSourceRoot(),
                             md5)) {
      if (auto u = Repo::get().loadUnit(fname, md5)) {
        return u.release();
      }
    }
  }
  return compile_string(s, sz, fname);
}

void assertTv(const TypedValue* tv) {
  always_assert(tvIsPlausible(*tv));
}

int init_closure(ActRec* ar, TypedValue* sp) {
  c_Closure* closure = static_cast<c_Closure*>(ar->getThis());

  // Swap in the $this or late bound class or null if it is ony from a plain
  // function or pseudomain
  ar->setThisOrClassAllowNull(closure->getThisOrClass());

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

void raiseNotice(const StringData* sd) {
  raise_notice("%s", sd->data());
}

void raiseArrayIndexNotice(const int64_t index) {
  raise_notice("Undefined index: %" PRId64, index);
}

void raiseArrayKeyNotice(const StringData* key) {
  raise_notice("Undefined key: %s", key->data());
}

//////////////////////////////////////////////////////////////////////

int64_t zero_error_level() {
  auto& id = ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData;
  auto level = id.getErrorReportingLevel();
  id.setErrorReportingLevel(0);
  return level;
}

void restore_error_level(int64_t oldLevel) {
  auto& id = ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData;
  if (id.getErrorReportingLevel() == 0) {
    id.setErrorReportingLevel(oldLevel);
  }
}

//////////////////////////////////////////////////////////////////////

}
