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
#include "hphp/runtime/base/request-info.h"
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

#include <folly/tracing/StaticTracepoint.h>

namespace HPHP {

TRACE_SET_MOD(runtime);

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
    FOLLY_SDT(hhvm, hhvm_cow_concat, v1->size(), v2->size());
    StringData* ret = StringData::Make(v1, v2);
    // Because v1 was shared, we know this won't release the string.
    v1->decRefCount();
    return ret;
  }

  auto const rhs = v2->slice();
  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize, rhs.size());
  auto const ret = v1->append(rhs);
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
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
    FOLLY_SDT(hhvm, hhvm_cow_concat, s1.size(), s2.size());
    auto const ret = StringData::Make(s1, s2);
    // Because v1 was shared, we know this won't release it.
    v1->decRefCount();
    return ret;
  }

  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize, s2.size());
  auto const ret = v1->append(s2);
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

StringData* concat_s3(StringData* v1, StringData* v2, StringData* v3) {
  if (v1->cowCheck()) {
    auto s1 = v1->slice();
    auto s2 = v2->slice();
    auto s3 = v3->slice();
    FOLLY_SDT(hhvm, hhvm_cow_concat, s1.size(), s2.size() + s3.size());
    StringData* ret = StringData::Make(s1, s2, s3);
    // Because v1 was shared, we know this won't release it.
    v1->decRefCount();
    return ret;
  }

  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize, v2->size() + v3->size());
  auto const ret = v1->append(v2->slice(), v3->slice());
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

StringData* concat_s4(StringData* v1, StringData* v2,
                      StringData* v3, StringData* v4) {
  if (v1->cowCheck()) {
    auto s1 = v1->slice();
    auto s2 = v2->slice();
    auto s3 = v3->slice();
    auto s4 = v4->slice();
    FOLLY_SDT(hhvm, hhvm_cow_concat, s1.size(),
              s2.size() + s3.size() + s4.size());
    StringData* ret = StringData::Make(s1, s2, s3, s4);
    // Because v1 was shared, we know this won't release it.
    v1->decRefCount();
    return ret;
  }

  UNUSED auto const lsize = v1->size();
  FOLLY_SDT(hhvm, hhvm_mut_concat, lsize,
            v2->size() + v3->size() + v4->size());
  auto const ret = v1->append(v2->slice(), v3->slice(), v4->slice());
  if (UNLIKELY(ret != v1)) {
    // had to realloc even though count==1
    assertx(v1->hasExactlyOneRef());
    v1->release();
  }
  return ret;
}

void raiseWarning(const StringData* sd) {
  raise_warning("%s", sd->data());
}

void raiseNotice(const StringData* sd) {
  raise_notice("%s", sd->data());
}

void throwArrayIndexException(const int64_t index, bool isInOut) {
  SystemLib::throwOutOfBoundsExceptionObject(folly::sformat(
    "Undefined index{}: {}", isInOut ? " on inout parameter" : "", index));
}

void throwArrayKeyException(const StringData* key, bool isInOut) {
  SystemLib::throwOutOfBoundsExceptionObject(folly::sformat(
    "Undefined index{}: {}", isInOut ? " on inout parameter" : "",
    key->data()));
}

std::string formatParamInOutMismatch(const char* fname, uint32_t index,
                                   bool funcByRef) {
  if (funcByRef) {
    return folly::sformat(
      "{}() expects parameter {} to be inout, but the call was "
      "not annotated with 'inout'", fname, index + 1
    );
  } else {
    return folly::sformat(
      "{}() does not expect parameter {} to be inout, but the call was "
      "annotated with 'inout'", fname, index + 1
    );
  }
}

void throwParamInOutMismatch(const Func* func, uint32_t index) {
  SystemLib::throwInvalidArgumentExceptionObject(formatParamInOutMismatch(
    func->fullName()->data(), index, func->isInOut(index)));
}

void throwParamInOutMismatchRange(const Func* func, unsigned firstVal,
                                  uint64_t mask, uint64_t vals) {
  for (auto i = 0; i < 64; ++i) {
    if (mask & (1UL << i)) {
      bool isInOut = vals & (1UL << i);
      if (func->isInOut(firstVal + i) != isInOut) {
        throwParamInOutMismatch(func, firstVal + i);
      }
    }
  }

  // Caller guarantees at least one parameter with inout-ness mismatch.
  not_reached();
}

void throwInvalidUnpackArgs() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only containers may be unpacked");
}

void raiseRxCallViolation(const ActRec* caller, const Func* callee) {
  assertx(RuntimeOption::EvalRxEnforceCalls > 0);
  auto const errMsg = folly::sformat(
    "Call to {} '{}' from {} '{}' violates reactivity constraints.",
    rxLevelToString(callee->rxLevel()),
    callee->fullName()->data(),
    rxLevelToString(caller->rxMinLevel()),
    caller->func()->fullName()->data()
  );

  if (RuntimeOption::EvalRxEnforceCalls >= 2) {
    SystemLib::throwBadMethodCallExceptionObject(errMsg);
  } else {
    raise_warning(errMsg);
  }
}

//////////////////////////////////////////////////////////////////////

int64_t zero_error_level() {
  auto& id = RequestInfo::s_requestInfo.getNoCheck()->m_reqInjectionData;
  auto level = id.getErrorReportingLevel();
  id.setErrorReportingLevel(0);
  return level;
}

void restore_error_level(int64_t oldLevel) {
  auto& id = RequestInfo::s_requestInfo.getNoCheck()->m_reqInjectionData;
  if (id.getErrorReportingLevel() == 0) {
    id.setErrorReportingLevel(oldLevel);
  }
}

//////////////////////////////////////////////////////////////////////

}
