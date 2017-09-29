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
#include "hphp/runtime/base/exceptions.h"

#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const StaticString s_file("file");
const StaticString s_line("line");
const StaticString s_trace("trace");
const StaticString s_traceOpts("traceOpts");
const Slot s_fileIdx{3};
const Slot s_lineIdx{4};
const Slot s_traceIdx{5};
const Slot s_traceOptsIdx{0};

std::atomic<int> ExitException::ExitCode{0};  // XXX: this should not be static

ExtendedException::ExtendedException() : Exception() {
  computeBacktrace();
}

ExtendedException::ExtendedException(const std::string &msg) {
  m_msg = msg;
  computeBacktrace();
}

ExtendedException::ExtendedException(SkipFrame, const std::string &msg) {
  m_msg = msg;
  computeBacktrace(true);
}

ExtendedException::ExtendedException(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  format(fmt, ap);
  va_end(ap);
  computeBacktrace();
}

ExtendedException::ExtendedException(const std::string& msg,
                                     ArrayData* backTrace)
  : m_btp(backTrace)
{
  m_msg = msg;
}

/*
 * Normally we wouldn't need an explicit copy/move-constructors, or
 * copy/move-assignment operators, but we have to make sure m_key isn't copied.
 */

ExtendedException::ExtendedException(const ExtendedException& other)
  : Exception(other),
    m_btp(other.m_btp),
    m_silent(other.m_silent)
{}

ExtendedException::ExtendedException(ExtendedException&& other) noexcept
  : Exception(std::move(other)),
    m_btp(std::move(other.m_btp)),
    m_silent(other.m_silent)
{}

ExtendedException&
ExtendedException::operator=(const ExtendedException& other) {
  Exception::operator=(other);
  m_btp = other.m_btp;
  m_silent = other.m_silent;
  return *this;
}

ExtendedException&
ExtendedException::operator=(ExtendedException&& other) noexcept {
  Exception::operator=(std::move(other));
  m_btp = std::move(other.m_btp);
  m_silent = other.m_silent;
  return *this;
}

Array ExtendedException::getBacktrace() const {
  return Array(m_btp.get());
}

std::pair<String, int> ExtendedException::getFileAndLine() const {
  String file = empty_string();
  int line = 0;
  Array bt = getBacktrace();
  if (!bt.empty()) {
    Array top = tvCastToArrayLike(bt.rvalAt(0).tv());
    if (top.exists(s_file)) file = tvCastToString(top.rvalAt(s_file).tv());
    if (top.exists(s_line)) line = tvCastToInt64(top.rvalAt(s_line).tv());
  }
  return std::make_pair(file, line);
}

void ExtendedException::computeBacktrace(bool skipFrame /* = false */) {
  Array bt = createBacktrace(BacktraceArgs()
                          .skipTop(skipFrame)
                          .withSelf());
  m_btp = bt.get();
}

//////////////////////////////////////////////////////////////////////

FatalErrorException::FatalErrorException(int, const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  format(msg, ap);
  va_end(ap);
  computeBacktrace();
}

FatalErrorException::FatalErrorException(const std::string& msg,
                                         const Array& backtrace,
                                         bool isRecoverable /* = false */)
  : ExtendedException(msg, backtrace.get()), m_recoverable(isRecoverable)
{}

//////////////////////////////////////////////////////////////////////

void throw_null_pointer_exception() {
  throw ExtendedException("A null object pointer was used.");
}

void throw_invalid_object_type(const char* clsName) {
  throw ExtendedException("Unexpected object type %s.", clsName);
}

void throw_not_implemented(const char* feature) {
  throw ExtendedException("%s is not implemented yet.", feature);
}

void throw_not_supported(const char* feature, const char* reason) {
  throw ExtendedException("%s is not supported: %s", feature, reason);
}

///////////////////////////////////////////////////////////////////////////////

[[noreturn]]
void raise_fatal_error(const char* msg,
                       const Array& bt /* = null_array */,
                       bool recoverable /* = false */,
                       bool silent /* = false */,
                       bool throws /* = true */) {
  if (RuntimeOption::PHP7_EngineExceptions && throws) {
    VMRegAnchor _;
    SystemLib::throwErrorObject(Variant(msg));
  }
  auto ex = bt.isNull() && !recoverable
    ? FatalErrorException(msg)
    : FatalErrorException(msg, bt, recoverable);
  ex.setSilent(silent);
  throw ex;
}

///////////////////////////////////////////////////////////////////////////////

namespace {
  DEBUG_ONLY bool vmfp_is_builtin() {
    VMRegAnchor _;
    auto const fp = vmfp();
    return fp && fp->func()->isBuiltin();
  }

  DEBUG_ONLY bool is_throwable(ObjectData* throwable) {
    auto const erCls = SystemLib::s_ErrorClass;
    auto const exCls = SystemLib::s_ExceptionClass;
    return throwable->instanceof(erCls) || throwable->instanceof(exCls);
  }

  DEBUG_ONLY bool throwable_has_expected_props() {
    auto const erCls = SystemLib::s_ErrorClass;
    auto const exCls = SystemLib::s_ExceptionClass;
    return
      erCls->lookupDeclProp(s_file.get()) == s_fileIdx &&
      exCls->lookupDeclProp(s_file.get()) == s_fileIdx &&
      erCls->lookupDeclProp(s_line.get()) == s_lineIdx &&
      exCls->lookupDeclProp(s_line.get()) == s_lineIdx &&
      erCls->lookupDeclProp(s_trace.get()) == s_traceIdx &&
      exCls->lookupDeclProp(s_trace.get()) == s_traceIdx;
  }

  int64_t exception_get_trace_options() {
    auto const exCls = SystemLib::s_ExceptionClass;
    assertx(exCls->lookupSProp(s_traceOpts.get()) == s_traceOptsIdx);
    assertx(exCls->needInitialization());

    exCls->initialize();
    auto const traceOptsTV = exCls->getSPropData(s_traceOptsIdx);
    return traceOptsTV->m_type == KindOfInt64
      ? traceOptsTV->m_data.num : 0;
  }
}

void throwable_init_file_and_line_from_builtin(ObjectData* throwable) {
  assertx(vmfp_is_builtin());
  assertx(is_throwable(throwable));
  assertx(throwable_has_expected_props());

  auto const traceTv = throwable->propVec()[s_traceIdx];

  if (traceTv.m_type == KindOfResource) {
    auto bt = dyn_cast<CompactTrace>(Resource(traceTv.m_data.pres));
    assertx(bt);

    for (auto& f : bt->frames()) {
      if (!f.func || f.func->isBuiltin()) continue;

      auto const opAtPrevPc = f.func->unit()->getOp(f.prevPc);
      Offset pcAdjust = 0;
      if (opAtPrevPc == Op::PopR ||
          opAtPrevPc == Op::UnboxR ||
          opAtPrevPc == Op::UnboxRNop) {
        pcAdjust = 1;
      }

      auto const ln = f.func->unit()->getLineNumber(f.prevPc - pcAdjust);
      tvSetIgnoreRef(make_tv<KindOfInt64>(ln), throwable->propVec()[s_lineIdx]);

      if (auto fn = f.func->originalFilename()) {
        tvSetIgnoreRef(
          make_tv<KindOfPersistentString>(fn), throwable->propVec()[s_fileIdx]
        );
      } else {
        tvSetIgnoreRef(
          make_tv<KindOfPersistentString>(f.func->unit()->filepath()),
          throwable->propVec()[s_fileIdx]
        );
      }
      return;
    }
    return;
  }

  assertx(isArrayType(traceTv.m_type));
  auto const trace = traceTv.m_data.parr;
  for (ArrayIter iter(trace); iter; ++iter) {
    assertx(iter.second().asTypedValue()->m_type == KindOfArray);
    auto const frame = iter.second().asTypedValue()->m_data.parr;
    auto const file = frame->rval(s_file.get());
    auto const line = frame->rval(s_line.get());
    if (file || line) {
      if (file) {
        auto const tv = file.tv();
        tvSetIgnoreRef(tvAssertCell(tv), throwable->propVec()[s_fileIdx]);
      }
      if (line) {
        auto const tv = line.tv();
        tvSetIgnoreRef(tvAssertCell(tv), throwable->propVec()[s_lineIdx]);
      }
      return;
    }
  }
}

void throwable_init(ObjectData* throwable) {
  assertx(is_throwable(throwable));
  assertx(throwable_has_expected_props());

  auto& prop = throwable->propVec()[s_traceIdx];
  auto opts = exception_get_trace_options();
  auto const filterOpts = opts & ~k_DEBUG_BACKTRACE_IGNORE_ARGS;
  if (
    !RuntimeOption::EvalEnableCompactBacktrace || filterOpts ||
    (RuntimeOption::EnableArgsInBacktraces &&
     opts != k_DEBUG_BACKTRACE_IGNORE_ARGS)
    ) {
    auto trace = HHVM_FN(debug_backtrace)(opts);
    cellMove(make_tv<KindOfArray>(trace.detach()), prop);
  } else {
    cellMove(
      make_tv<KindOfResource>(createCompactBacktrace().detach()->hdr()), prop
    );
  }

  VMRegAnchor _;
  auto const fp = vmfp();
  if (UNLIKELY(!fp)) return;
  if (UNLIKELY(fp->func()->isBuiltin())) {
    throwable_init_file_and_line_from_builtin(throwable);
  } else {
    auto const unit = fp->func()->unit();
    auto const file = const_cast<StringData*>(unit->filepath());
    auto const line = unit->getLineNumber(unit->offsetOf(vmpc()));
    tvSetIgnoreRef(make_tv<KindOfString>(file), throwable->propVec()[s_fileIdx]);
    tvSetIgnoreRef(make_tv<KindOfInt64>(line), throwable->propVec()[s_lineIdx]);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
