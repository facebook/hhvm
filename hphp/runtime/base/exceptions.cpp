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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
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
const StaticString s_throwableToStringFailed("(throwable_to_string failed)");
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
    Array top = tvCastToArrayLike(bt.lookup(0));
    if (top.exists(s_file)) file = tvCastToString(top.lookup(s_file));
    if (top.exists(s_line)) line = tvCastToInt64(top.lookup(s_line));
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

void throw_stack_overflow() {
  throw FatalErrorException("Stack overflow");
}

///////////////////////////////////////////////////////////////////////////////

[[noreturn]]
void raise_fatal_error(const char* msg,
                       const Array& bt /* = null_array */,
                       bool recoverable /* = false */,
                       bool silent /* = false */,
                       bool throws /* = true */) {
  auto ex = bt.isNull() && !recoverable
    ? FatalErrorException(msg)
    : FatalErrorException(msg, bt, recoverable);
  ex.setSilent(silent);
  throw ex;
}

[[noreturn]]
void raise_parse_error(const StringData* filename,
                       const char* msg,
                       const Location::Range& loc) {
  VMParserFrame parserFrame;
  parserFrame.filename = filename;
  parserFrame.lineNumber = loc.line1;
  Array bt = createBacktrace(BacktraceArgs()
    .withSelf()
    .setParserFrame(&parserFrame));
  raise_fatal_error(msg, bt);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
  DEBUG_ONLY bool vmfp_is_builtin() {
    VMRegAnchor _;
    auto const fp = vmfp();
    return fp && fp->func()->isBuiltin();
  }

  DEBUG_ONLY bool is_throwable(const ObjectData* throwable) {
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
    return type(traceOptsTV) == KindOfInt64 ? val(traceOptsTV).num : 0;
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

        auto const ln = f.func->getLineNumber(f.prevPc);
        tvSet(
          make_tv<KindOfInt64>(ln),
          throwable->propLvalAtOffset(s_lineSlot)
        );

        if (auto fn = f.func->originalFilename()) {
          tvSet(
            make_tv<KindOfPersistentString>(fn),
            throwable->propLvalAtOffset(s_fileSlot)
          );
        } else {
          tvSet(
            make_tv<KindOfPersistentString>(f.func->unit()->filepath()),
            throwable->propLvalAtOffset(s_fileSlot)
          );
        }
        return;
      }
      return;
    }

    assertx(isArrayLikeType(trace_rval.type()));
    auto const trace = trace_rval.val().parr;
    for (ArrayIter iter(trace); iter; ++iter) {
      auto const frame_tv = iter.secondVal();
      assertx(isArrayLikeType(type(frame_tv)));
      auto const frame = val(frame_tv).parr;
      auto const file = frame->get(s_file.get());
      auto const line = frame->get(s_line.get());
      if (file.is_init() || line.is_init()) {
        if (file.is_init()) {
          tvSet(
            tvAssertPlausible(file),
            throwable->propLvalAtOffset(s_fileSlot)
          );
        }
        if (line.is_init()) {
          tvSet(
            tvAssertPlausible(line),
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
  auto const opts = exception_get_trace_options();
  auto trace = HHVM_FN(debug_backtrace)(opts);
  auto tv = make_array_like_tv(trace.detach());
  tvMove(tv, trace_lval);

  VMRegAnchor _;
  auto const fp = vmfp();
  if (UNLIKELY(!fp)) return;
  if (UNLIKELY(fp->func()->isBuiltin())) {
    throwable_init_file_and_line_from_builtin(throwable);
  } else {
    // Get the current function and offset in an inline-aware way. It must
    // always exist, as vmfp() is a non-builtin frame pointer.
    auto const funcAndOffset = getCurrentFuncAndOffset();
    assertx(funcAndOffset.first != nullptr);
    auto const func = funcAndOffset.first;
    auto const file = const_cast<StringData*>(funcAndOffset.first->filename());
    auto const line = func->getLineNumber(funcAndOffset.second);
    tvSet(
      make_tv<KindOfString>(file),
      throwable->propLvalAtOffset(s_fileSlot)
    );
    tvSet(
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
  tvMove(tv, trace_lval);
  throwable_init_file_and_line_from_trace(throwable);
}

void throwable_mark_array(const ObjectData* throwable, Array& props) {
  assertx(is_throwable(throwable));
  assertx(throwable_has_expected_props());

  auto const prop = throwable->getVMClass()->declProperties()[s_traceSlot];
  auto const name = StrNR(prop.mangledName).asString();
  auto const base = props.lookup(name);
  if (!tvIsArrayLike(base)) return;

  auto const incref = Variant::wrap(base);
  // We use a depth of 2 because backtrace arrays are varrays-of-darrays.
  auto const marked = Variant::attach(arrprov::markTvToDepth(base, true, 2));
  props.set(name, marked);
}

String throwable_to_string(ObjectData* throwable) {
  if (throwable->instanceof(SystemLib::s_ThrowableClass)) {
    try {
      auto result = ObjectData::InvokeSimple(throwable, s___toString,
                                             RuntimeCoeffects::fixme());
      if (result.isString()) {
        return result.asCStrRef();
      }
    } catch (const Object&) {
      // Ignore PHP exceptions from Throwable::__toString
    }
  }
  return s_throwableToStringFailed;
}
///////////////////////////////////////////////////////////////////////////////
}
