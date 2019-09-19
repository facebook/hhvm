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
const StaticString s___toString("__toString");
const Slot s_fileSlot{3};
const Slot s_lineSlot{4};
const Slot s_traceSlot{5};
const Slot s_traceOptsSlot{0};

RDS_LOCAL(int, rl_exit_code);

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
                          .withSelf()
                          .withMetadata());
  m_btp = bt.get();
}

void ExtendedException::recomputeBacktraceFromWH(c_WaitableWaitHandle* wh) {
  assertx(wh);
  Array bt = createBacktrace(BacktraceArgs()
                             .fromWaitHandle(wh)
                             .withSelf()
                             .withMetadata());
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
    if (erCls->lookupDeclProp(s_file.get()) != s_fileSlot ||
        exCls->lookupDeclProp(s_file.get()) != s_fileSlot ||
        erCls->lookupDeclProp(s_line.get()) != s_lineSlot ||
        exCls->lookupDeclProp(s_line.get()) != s_lineSlot ||
        erCls->lookupDeclProp(s_trace.get()) != s_traceSlot ||
        exCls->lookupDeclProp(s_trace.get()) != s_traceSlot) {
      return false;
    }
    // Check that we don't have the expected type-hints on these props so we
    // don't need to verify anything.
    return
      erCls->declPropTypeConstraint(s_fileSlot).isString() &&
      exCls->declPropTypeConstraint(s_fileSlot).isString() &&
      erCls->declPropTypeConstraint(s_lineSlot).isInt() &&
      exCls->declPropTypeConstraint(s_lineSlot).isInt() &&
      !erCls->declPropTypeConstraint(s_traceSlot).isCheckable() &&
      !exCls->declPropTypeConstraint(s_traceSlot).isCheckable();
  }

  int64_t exception_get_trace_options() {
    auto const exCls = SystemLib::s_ExceptionClass;
    assertx(exCls->lookupSProp(s_traceOpts.get()) == s_traceOptsSlot);
    assertx(exCls->needInitialization());

    exCls->initialize();
    auto const traceOptsTV = exCls->getSPropData(s_traceOptsSlot);
    return traceOptsTV->m_type == KindOfInt64
      ? traceOptsTV->m_data.num : 0;
  }

  void throwable_init_file_and_line_from_trace(ObjectData* throwable) {
    assertx(is_throwable(throwable));
    assertx(throwable_has_expected_props());

    auto const trace_rval = throwable->propRvalAtOffset(s_traceSlot);

    if (trace_rval.type() == KindOfResource) {
      auto bt = dyn_cast<CompactTrace>(Resource(trace_rval.val().pres));
      assertx(bt);

      for (auto& f : bt->frames()) {
        if (!f.func || f.func->isBuiltin()) continue;

        auto const ln = f.func->unit()->getLineNumber(f.prevPc);
        tvSetIgnoreRef(
          make_tv<KindOfInt64>(ln),
          throwable->propLvalAtOffset(s_lineSlot)
        );

        if (auto fn = f.func->originalFilename()) {
          tvSetIgnoreRef(
            make_tv<KindOfPersistentString>(fn),
            throwable->propLvalAtOffset(s_fileSlot)
          );
        } else {
          tvSetIgnoreRef(
            make_tv<KindOfPersistentString>(f.func->unit()->filepath()),
            throwable->propLvalAtOffset(s_fileSlot)
          );
        }
        return;
      }
      return;
    }

    assertx(RuntimeOption::EvalHackArrDVArrs ?
      isVecType(trace_rval.type()) :
      isArrayType(trace_rval.type())
    );
    auto const trace = trace_rval.val().parr;
    for (ArrayIter iter(trace); iter; ++iter) {
      assertx(iter.second().asTypedValue()->m_type == (
        RuntimeOption::EvalHackArrDVArrs ? KindOfDict : KindOfArray
      ));
      auto const frame = iter.second().asTypedValue()->m_data.parr;
      auto const file = frame->rval(s_file.get());
      auto const line = frame->rval(s_line.get());
      if (file || line) {
        if (file) {
          auto const tv = file.tv();
          tvSetIgnoreRef(
            tvAssertCell(tv),
            throwable->propLvalAtOffset(s_fileSlot)
          );
        }
        if (line) {
          auto const tv = line.tv();
          tvSetIgnoreRef(
            tvAssertCell(tv),
            throwable->propLvalAtOffset(s_lineSlot)
          );
        }
        return;
      }
    }
  }
}

void throwable_init(ObjectData* throwable) {
  assertx(is_throwable(throwable));
  assertx(throwable_has_expected_props());

  auto const trace_lval = throwable->propLvalAtOffset(s_traceSlot);
  auto opts = exception_get_trace_options();
  auto const filterOpts = opts & ~k_DEBUG_BACKTRACE_IGNORE_ARGS;
  if (
    !RuntimeOption::EvalEnableCompactBacktrace || filterOpts ||
    (RuntimeOption::EnableArgsInBacktraces &&
     opts != k_DEBUG_BACKTRACE_IGNORE_ARGS)
    ) {
    auto trace = HHVM_FN(debug_backtrace)(opts);
    auto tv = RuntimeOption::EvalHackArrDVArrs ?
      make_tv<KindOfVec>(trace.detach()) :
      make_tv<KindOfArray>(trace.detach());
    cellMove(tv, trace_lval);
  } else {
    cellMove(
      make_tv<KindOfResource>(createCompactBacktrace().detach()->hdr()),
      trace_lval
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
    tvSetIgnoreRef(
      make_tv<KindOfString>(file),
      throwable->propLvalAtOffset(s_fileSlot)
    );
    tvSetIgnoreRef(
      make_tv<KindOfInt64>(line),
      throwable->propLvalAtOffset(s_lineSlot)
    );
  }
}

void throwable_init_file_and_line_from_builtin(ObjectData* throwable) {
  assertx(vmfp_is_builtin());
  throwable_init_file_and_line_from_trace(throwable);
}

void throwable_recompute_backtrace_from_wh(ObjectData* throwable,
                                           c_WaitableWaitHandle* wh) {
  assertx(is_throwable(throwable));
  assertx(throwable_has_expected_props());
  assertx(wh);

  auto const trace_lval = throwable->propLvalAtOffset(s_traceSlot);
  auto opts = exception_get_trace_options();
  bool provide_object = opts & k_DEBUG_BACKTRACE_PROVIDE_OBJECT;
  bool provide_metadata = opts & k_DEBUG_BACKTRACE_PROVIDE_METADATA;
  bool ignore_args = opts & k_DEBUG_BACKTRACE_IGNORE_ARGS;

  auto trace = createBacktrace(BacktraceArgs()
                               .fromWaitHandle(wh)
                               .withSelf()
                               .withThis(provide_object)
                               .withMetadata(provide_metadata)
                               .ignoreArgs(ignore_args));
  auto tv = make_array_like_tv(trace.detach());
  cellMove(tv, trace_lval);
  throwable_init_file_and_line_from_trace(throwable);
}

String throwable_to_string(ObjectData* throwable) {
  auto result = ObjectData::InvokeSimple(throwable, s___toString);
  return result.isString()
    ? result.toString()
    : empty_string();
}
///////////////////////////////////////////////////////////////////////////////
}
