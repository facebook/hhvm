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
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/server/source-root-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
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
  char intbuf[21];
  auto const s = conv_10(i, intbuf + sizeof(intbuf));

  g_context->write(s.data(), s.size());
}

void print_boolean(bool val) {
  if (val) {
    g_context->write("1", 1);
  }
}

/**
 * concat_ss will will incRef the output string
 * and decref its first argument
 */
StringData* concat_ss(StringData* v1, StringData* v2) {
  if (v1->cowCheck()) {
    StringData* ret = StringData::Make(v1, v2);
    // Because v1 was shared, we know this won't release the string.
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
  auto const s2 = v2->slice();
  return StringData::Make(s1, s2);
}

/**
 * concat_si will incRef the output string
 * and decref its first argument
 */
StringData* concat_si(StringData* v1, int64_t v2) {
  char intbuf[21];
  auto const s2 = conv_10(v2, intbuf + sizeof(intbuf));
  if (v1->cowCheck()) {
    auto const s1 = v1->slice();
    auto const ret = StringData::Make(s1, s2);
    // Because v1 was shared, we know this won't release it.
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
  if (v1->cowCheck()) {
    StringData* ret = StringData::Make(
      v1->slice(), v2->slice(), v3->slice());
    // Because v1 was shared, we know this won't release it.
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
  if (v1->cowCheck()) {
    StringData* ret = StringData::Make(
        v1->slice(), v2->slice(), v3->slice(), v4->slice());
    // Because v1 was shared, we know this won't release it.
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
                   const char* fname, Unit** releaseUnit) {
  return g_hphp_compiler_parse(s, sz, md5, fname, releaseUnit);
}

Unit* compile_string(const char* s,
                     size_t sz,
                     const char* fname,
                     Unit** releaseUnit) {
  auto const md5 = MD5{mangleUnitMd5(string_md5(folly::StringPiece{s, sz}))};
  if (auto u = Repo::get().loadUnit(fname ? fname : "", md5).release()) {
    return u;
  }
  // NB: fname needs to be long-lived if generating a bytecode repo because it
  // can be cached via a Location ultimately contained by ErrorInfo for printing
  // code errors.
  return g_hphp_compiler_parse(s, sz, md5, fname, releaseUnit);
}

Unit* compile_systemlib_string(const char* s, size_t sz, const char* fname) {
  if (RuntimeOption::RepoAuthoritative) {
    String systemName = String("/:") + fname;
    auto md5 = MD5{mangleUnitMd5(string_md5(folly::StringPiece{s,sz}))};
    if (Repo::get().findFile(systemName.data(),
                             SourceRootInfo::GetCurrentSourceRoot(),
                             md5) == RepoStatus::success) {
      if (auto u = Repo::get().loadUnit(fname, md5)) {
        return u.release();
      }
    }
  }
  return compile_string(s, sz, fname);
}

int init_closure(ActRec* ar, TypedValue* sp) {
  auto closure = c_Closure::fromObject(ar->getThis());

  // Put in the correct context
  ar->m_func = closure->getInvokeFunc();

  if (ar->func()->cls()) {
    // Swap in the $this or late bound class or null if it is ony from a plain
    // function or pseudomain
    ar->setThisOrClass(closure->getThisOrClass());

    if (ar->hasThis()) {
      ar->getThis()->incRefCount();
    }
  } else {
    ar->trashThis();
  }

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
  raise_notice("Undefined index: %s", key->data());
}

std::string formatParamRefMismatch(const char* fname, uint32_t index,
                                   bool funcByRef) {
  if (funcByRef) {
    return folly::sformat(
      "{}() expects parameter {} by reference, but the call was "
      "not annotated with '&'", fname, index + 1
    );
  } else {
    return folly::sformat(
      "{}() expects parameter {} by value, but the call was "
      "annotated with '&'", fname, index + 1
    );
  }
}

void raiseParamRefMismatch(uint32_t index, const Func* func) {
  raise_warning(
    formatParamRefMismatch(func->fullDisplayName()->data(),
                           index, func->byRef(index))
  );
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
