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
#include "hphp/runtime/base/execution-context.h"

#define __STDC_LIMIT_MACROS

#include <cstdint>
#include <algorithm>
#include <list>
#include <utility>

#include <folly/MapUtil.h>
#include <folly/Format.h>
#include <folly/Likely.h>

#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/text-color.h"
#include "hphp/util/service-data.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/apc-gc-manager.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/system-profiler.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/std/ext_std_output.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/jit/enter-tc.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/act-rec-defs.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/hh-utils.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/zend/zend-math.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(bcinterp);

rds::local::AliasedRDSLocal<ExecutionContext,
                            rds::local::Initialize::Explicitly,
                            &rds::local::detail::HotRDSLocals::g_context
                           > g_context;

ExecutionContext::ExecutionContext()
  : m_transport(nullptr)
  , m_sb(nullptr)
  , m_implicitFlush(false)
  , m_protectedLevel(0)
  , m_stdoutBytesWritten(0)
  , m_errorState(ExecutionContext::ErrorState::NoError)
  , m_lastErrorNum(0)
  , m_deferredErrors(nullptr)
  , m_throwAllErrors(false)
  , m_pageletTasksStarted(0)
  , m_vhost(nullptr)
  , m_globalVarEnv(nullptr)
  , m_lambdaCounter(0)
  , m_nesting(0)
  , m_dbgNoBreak(false)
  , m_lastErrorPath(staticEmptyString())
  , m_lastErrorLine(0)
  , m_executingSetprofileCallback(false)
  , m_logger_hook(*this)
{
  ARRPROV_USE_RUNTIME_LOCATION();
  m_deferredErrors = staticEmptyVecArray();
  resetCoverageCounters();
  // We don't want a new execution context to cause any request-heap
  // allocations (because it will cause us to hold a slab, even while idle).
  static auto s_cwd = makeStaticString(Process::CurrentWorkingDirectory);
  m_cwd = s_cwd;
  RID().setMemoryLimit(std::to_string(RuntimeOption::RequestMemoryMaxBytes));
  RID().setErrorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel);

  VariableSerializer::serializationSizeLimit->value =
    RuntimeOption::SerializationSizeLimit;
  tvWriteUninit(m_headerCallback);

  m_shutdowns[ShutdownType::ShutDown] = empty_vec_array();
  m_shutdowns[ShutdownType::PostSend] = empty_vec_array();
  m_shutdownsBackup[ShutdownType::ShutDown] = empty_vec_array();
  m_shutdownsBackup[ShutdownType::PostSend] = empty_vec_array();
}

namespace rds { namespace local {
// See header for why this is required.
#ifndef _MSC_VER
template<>
#endif
void rds::local::RDSLocal<ExecutionContext,
                          rds::local::Initialize::Explicitly>::destroy() {
  if (!isNull()) {
    getNoCheck()->sweep();
    nullOut();
  }
}
}}


void ExecutionContext::cleanup() {
  manageAPCHandle();
}

void ExecutionContext::sweep() {
  cleanup();
}

ExecutionContext::~ExecutionContext() {
  cleanup();
}

void ExecutionContext::backupSession() {
  m_shutdownsBackup = m_shutdowns;
  m_userErrorHandlersBackup = m_userErrorHandlers;
  m_userExceptionHandlersBackup = m_userExceptionHandlers;
}

void ExecutionContext::restoreSession() {
  m_shutdowns = m_shutdownsBackup;
  m_userErrorHandlers = m_userErrorHandlersBackup;
  m_userExceptionHandlers = m_userExceptionHandlersBackup;
}

///////////////////////////////////////////////////////////////////////////////
// system functions

String ExecutionContext::getMimeType() const {
  String mimetype;
  if (m_transport) {
    mimetype = m_transport->getMimeType();
  }

  if (strncasecmp(mimetype.data(), "text/", 5) == 0) {
    int pos = mimetype.find(';');
    if (pos != String::npos) {
      mimetype = mimetype.substr(0, pos);
    }
  } else if (m_transport && m_transport->getUseDefaultContentType()) {
    mimetype = RID().getDefaultMimeType();
  }
  return mimetype;
}

std::string ExecutionContext::getRequestUrl(size_t szLimit) {
  Transport* t = getTransport();
  std::string ret = t ? t->getUrl() : "";
  if (szLimit != std::string::npos) {
    ret = ret.substr(0, szLimit);
  }
  return ret;
}

void ExecutionContext::setContentType(const String& mimetype,
                                          const String& charset) {
  if (m_transport) {
    String contentType = mimetype;
    contentType += "; ";
    contentType += "charset=";
    contentType += charset;
    m_transport->addHeader("Content-Type", contentType.c_str());
    m_transport->setUseDefaultContentType(false);
  }
}

///////////////////////////////////////////////////////////////////////////////
// write()

void ExecutionContext::write(const String& s) {
  write(s.data(), s.size());
}

void ExecutionContext::addStdoutHook(StdoutHook* hook) {
  if (hook != nullptr) {
    m_stdoutHooks.insert(hook);
  }
}

bool ExecutionContext::removeStdoutHook(StdoutHook* hook) {
  if (hook == nullptr) {
    return false;
  }

  return m_stdoutHooks.erase(hook) != 0;
}

static void safe_stdout(const  void  *ptr,  size_t  size) {
  write(fileno(stdout), ptr, size);
}

void ExecutionContext::writeStdout(const char *s, int len) {
  fflush(stdout);
  if (m_stdoutHooks.empty()) {
    if (s_stdout_color) {
      safe_stdout(s_stdout_color, strlen(s_stdout_color));
      safe_stdout(s, len);
      safe_stdout(ANSI_COLOR_END, strlen(ANSI_COLOR_END));
    } else {
      safe_stdout(s, len);
    }
    m_stdoutBytesWritten += len;
  } else {
    for (auto const hook : m_stdoutHooks) {
      assertx(hook != nullptr);
      (*hook)(s, len);
    }
  }
}

void ExecutionContext::writeTransport(const char *s, int len) {
  if (m_transport) {
    m_transport->sendRaw(s, len, 200, false, true);
  } else {
    writeStdout(s, len);
  }
}

size_t ExecutionContext::getStdoutBytesWritten() const {
  return m_stdoutBytesWritten;
}

void ExecutionContext::write(const char *s, int len) {
  if (m_sb) {
    m_sb->append(s, len);
    if (m_out && m_out->chunk_size > 0) {
      if (m_sb->size() >= m_out->chunk_size) {
        obFlush();
      }
    }
    if (m_implicitFlush) flush();
  } else {
    writeTransport(s, len);
  }
}

///////////////////////////////////////////////////////////////////////////////
// output buffers

void ExecutionContext::obProtect(bool on) {
  m_protectedLevel = on ? m_buffers.size() : 0;
}

void ExecutionContext::obStart(const Variant& handler /* = null */,
                               int chunk_size /* = 0 */,
                               OBFlags flags /* = OBFlags::Default */) {
  if (m_insideOBHandler) {
    raise_error("ob_start(): Cannot use output buffering "
                "in output buffering display handlers");
  }
  m_buffers.emplace_back(Variant(handler), chunk_size, flags);
  resetCurrentBuffer();
}

String ExecutionContext::obCopyContents() {
  if (!m_buffers.empty()) {
    StringBuffer &oss = m_buffers.back().oss;
    if (!oss.empty()) {
      return oss.copy();
    }
  }
  return empty_string();
}

String ExecutionContext::obDetachContents() {
  if (!m_buffers.empty()) {
    StringBuffer &oss = m_buffers.back().oss;
    if (!oss.empty()) {
      return oss.detach();
    }
  }
  return empty_string();
}

int ExecutionContext::obGetContentLength() {
  if (m_buffers.empty()) {
    return 0;
  }
  return m_buffers.back().oss.size();
}

void ExecutionContext::obClean(int handler_flag) {
  if (!m_buffers.empty()) {
    OutputBuffer *last = &m_buffers.back();
    if (!last->handler.isNull()) {
      m_insideOBHandler = true;
      SCOPE_EXIT { m_insideOBHandler = false; };
      vm_call_user_func(last->handler,
                        make_vec_array(last->oss.detach(), handler_flag));
    }
    last->oss.clear();
  }
}

bool ExecutionContext::obFlush(bool force /*= false*/) {
  assertx(m_protectedLevel >= 0);

  if ((int)m_buffers.size() <= m_protectedLevel) {
    return false;
  }

  auto iter = m_buffers.end();
  OutputBuffer& last = *(--iter);
  if (!force && !(last.flags & OBFlags::Flushable)) {
    return false;
  }
  if (any(last.flags & OBFlags::OutputDisabled)) {
    return false;
  }

  const int flag = k_PHP_OUTPUT_HANDLER_START | k_PHP_OUTPUT_HANDLER_END;

  if (iter != m_buffers.begin()) {
    OutputBuffer& prev = *(--iter);
    if (last.handler.isNull()) {
      prev.oss.absorb(last.oss);
    } else {
      auto str = last.oss.detach();
      try {
        Variant tout;
        {
          ARRPROV_USE_RUNTIME_LOCATION();
          m_insideOBHandler = true;
          SCOPE_EXIT { m_insideOBHandler = false; };
          tout = vm_call_user_func(
            last.handler, make_vec_array(str, flag)
          );
        }
        prev.oss.append(tout.toString());
      } catch (...) {
        prev.oss.append(str);
        throw;
      }
    }
    return true;
  }

  auto str = last.oss.detach();
  if (!last.handler.isNull()) {
    try {
      Variant tout;
      {
        ARRPROV_USE_RUNTIME_LOCATION();
        m_insideOBHandler = true;
        SCOPE_EXIT { m_insideOBHandler = false; };
        tout = vm_call_user_func(
          last.handler, make_vec_array(str, flag)
        );
      }
      str = tout.toString();
    } catch (...) {
      writeTransport(str.data(), str.size());
      throw;
    }
  }

  writeTransport(str.data(), str.size());
  return true;
}

void ExecutionContext::obFlushAll() {
  do {
    obFlush(true);
  } while (obEnd());
}

bool ExecutionContext::obEnd() {
  assertx(m_protectedLevel >= 0);
  if ((int)m_buffers.size() > m_protectedLevel) {
    m_buffers.pop_back();
    resetCurrentBuffer();
    if (m_implicitFlush) flush();
    return true;
  }
  if (m_implicitFlush) flush();
  return false;
}

void ExecutionContext::obEndAll() {
  while (obEnd()) {}
}

int ExecutionContext::obGetLevel() {
  assertx((int)m_buffers.size() >= m_protectedLevel);
  return m_buffers.size() - m_protectedLevel;
}

const StaticString
  s_level("level"),
  s_type("type"),
  s_flags("flags"),
  s_name("name"),
  s_args("args"),
  s_chunk_size("chunk_size"),
  s_buffer_used("buffer_used"),
  s_default_output_handler("default output handler");

Array ExecutionContext::obGetStatus(bool full) {
  Array ret = empty_varray();
  int level = 0;
  for (auto& buffer : m_buffers) {
    Array status = empty_darray();
    if (level < m_protectedLevel || buffer.handler.isNull()) {
      status.set(s_name, s_default_output_handler);
      status.set(s_type, 0);
    } else {
      status.set(s_name, buffer.handler);
      status.set(s_type, 1);
    }

    int flags = 0;
    if (any(buffer.flags & OBFlags::Cleanable)) {
      flags |= k_PHP_OUTPUT_HANDLER_CLEANABLE;
    }
    if (any(buffer.flags & OBFlags::Flushable)) {
      flags |= k_PHP_OUTPUT_HANDLER_FLUSHABLE;
    }
    if (any(buffer.flags & OBFlags::Removable)) {
      flags |= k_PHP_OUTPUT_HANDLER_REMOVABLE;
    }
    status.set(s_flags, flags);

    status.set(s_level, level);
    status.set(s_chunk_size, buffer.chunk_size);
    status.set(s_buffer_used, static_cast<uint64_t>(buffer.oss.size()));

    if (full) {
      ret.append(status);
    } else {
      ret = std::move(status);
    }
    level++;
  }
  return ret;
}

String ExecutionContext::obGetBufferName() {
  if (m_buffers.empty()) {
    return String();
  } else if (m_buffers.size() <= m_protectedLevel) {
    return s_default_output_handler;
  } else {
    auto iter = m_buffers.end();
    OutputBuffer& buffer = *(--iter);
    if (buffer.handler.isNull()) {
      return s_default_output_handler;
    } else {
      return buffer.handler.toString();
    }
  }
}

void ExecutionContext::obSetImplicitFlush(bool on) {
  m_implicitFlush = on;
}

Array ExecutionContext::obGetHandlers() {
  Array ret = empty_varray();
  for (auto& ob : m_buffers) {
    auto& handler = ob.handler;
    ret.append(handler.isNull() ? s_default_output_handler : handler);
  }
  return ret;
}

void ExecutionContext::flush() {
  if (!m_buffers.empty() &&
      RuntimeOption::EnableEarlyFlush && m_protectedLevel &&
      !(m_buffers.front().flags & OBFlags::OutputDisabled)) {
    OutputBuffer &buffer = m_buffers.front();
    StringBuffer &oss = buffer.oss;
    if (!oss.empty()) {
      if (any(buffer.flags & OBFlags::WriteToStdout)) {
        writeStdout(oss.data(), oss.size());
      } else {
        writeTransport(oss.data(), oss.size());
      }
      oss.clear();
    }
  }
}

void ExecutionContext::resetCurrentBuffer() {
  if (m_buffers.empty()) {
    m_sb = nullptr;
    m_out = nullptr;
  } else {
    m_sb = &m_buffers.back().oss;
    m_out = &m_buffers.back();
  }
}

///////////////////////////////////////////////////////////////////////////////
// program executions

void ExecutionContext::registerShutdownFunction(const Variant& function,
                                                ShutdownType type) {
  auto& funcs = m_shutdowns[type];
  assertx(funcs.isVecArray());
  funcs.append(function);
}

Variant ExecutionContext::pushUserErrorHandler(const Variant& function,
                                               int error_types) {
  Variant ret;
  if (!m_userErrorHandlers.empty()) {
    ret = m_userErrorHandlers.back().first;
  }
  m_userErrorHandlers.push_back(std::pair<Variant,int>(function, error_types));
  return ret;
}

Variant ExecutionContext::pushUserExceptionHandler(const Variant& function) {
  Variant ret;
  if (!m_userExceptionHandlers.empty()) {
    ret = m_userExceptionHandlers.back();
  }
  m_userExceptionHandlers.push_back(function);
  return ret;
}

void ExecutionContext::popUserErrorHandler() {
  if (!m_userErrorHandlers.empty()) {
    m_userErrorHandlers.pop_back();
  }
}

void ExecutionContext::clearUserErrorHandlers() {
  while (!m_userErrorHandlers.empty()) m_userErrorHandlers.pop_back();
}

void ExecutionContext::popUserExceptionHandler() {
  if (!m_userExceptionHandlers.empty()) {
    m_userExceptionHandlers.pop_back();
  }
}

void ExecutionContext::acceptRequestEventHandlers(bool enable) {
  m_acceptRequestEventHandlers = enable;
}

std::size_t ExecutionContext::registerRequestEventHandler(
  RequestEventHandler *handler) {
  assertx(handler && handler->getInited());
  assertx(m_acceptRequestEventHandlers);
  m_requestEventHandlers.push_back(handler);
  return m_requestEventHandlers.size()-1;
}

void ExecutionContext::unregisterRequestEventHandler(
  RequestEventHandler* handler,
  std::size_t index) {
  assertx(index < m_requestEventHandlers.size() &&
         m_requestEventHandlers[index] == handler);
  assertx(!handler->getInited());
  if (index == m_requestEventHandlers.size()-1) {
    m_requestEventHandlers.pop_back();
  } else {
    m_requestEventHandlers[index] = nullptr;
  }
}

static bool requestEventHandlerPriorityComp(RequestEventHandler *a,
                                            RequestEventHandler *b) {
  if (!a) return b;
  else if (!b) return false;
  else return a->priority() < b->priority();
}

void ExecutionContext::onRequestShutdown() {
  while (!m_requestEventHandlers.empty()) {
    // handlers could cause other handlers to be registered,
    // so need to repeat until done
    decltype(m_requestEventHandlers) tmp;
    tmp.swap(m_requestEventHandlers);

    // Sort handlers by priority so that lower priority values get shutdown
    // first
    sort(tmp.begin(), tmp.end(),
         requestEventHandlerPriorityComp);
    for (auto* handler : tmp) {
      if (!handler) continue;
      assertx(handler->getInited());
      handler->requestShutdown();
      handler->setInited(false);
    }
  }
}

void ExecutionContext::executeFunctions(ShutdownType type) {
  ARRPROV_USE_RUNTIME_LOCATION();
  RID().resetTimers(
      RuntimeOption::PspTimeoutSeconds,
      RuntimeOption::PspCpuTimeoutSeconds
  );

  // We mustn't destroy any callbacks until we're done with all
  // of them. So hold them in tmp.
  // XXX still true in a world without destructors?
  auto tmp = empty_vec_array();
  while (true) {
    Array funcs = m_shutdowns[type];
    if (funcs.empty()) break;
    m_shutdowns[type] = empty_vec_array();
    IterateV(
      funcs.get(),
      [](TypedValue v) {
        vm_call_user_func(VarNR{v}, init_null_variant);
      }
    );
    tmp.append(funcs);
  }
}

void ExecutionContext::onShutdownPreSend() {
  // in case obStart was called without obFlush
  SCOPE_EXIT {
    try { obFlushAll(); } catch (...) {}
  };

  // When host is OOMing, abort abruptly.
  if (RID().shouldOOMAbort()) return;

  tracing::Block _{"shutdown-pre-send"};

  tl_heap->resetCouldOOM(isStandardRequest());
  executeFunctions(ShutDown);
}

void ExecutionContext::debuggerExecutePsps() {
  try {
    executeFunctions(PostSend);
  } catch (const ExitException& e) {
    // do nothing
  } catch (const Exception& e) {
    onFatalError(e);
  } catch (const Object& e) {
    onUnhandledException(e);
  } catch (...) {
  }
}

void ExecutionContext::onShutdownPostSend() {
  // When host is OOMing, abort abruptly.
  if (RID().shouldOOMAbort()) return;

  tracing::Block _{"shutdown-post-send"};

  ServerStats::SetThreadMode(ServerStats::ThreadMode::PostProcessing);
  tl_heap->resetCouldOOM(isStandardRequest());
  try {
    try {
      ServerStatsHelper ssh("psp", ServerStatsHelper::TRACK_HWINST);
      executeFunctions(PostSend);
    } catch (...) {
      try {
        bump_counter_and_rethrow(true /* isPsp */);
      } catch (const ExitException& e) {
        // do nothing
      } catch (const Exception& e) {
        onFatalError(e);
      } catch (const Object& e) {
        onUnhandledException(e);
      }
    }
  } catch (...) {
    Logger::Error("unknown exception was thrown from psp");
  }

  ServerStats::SetThreadMode(ServerStats::ThreadMode::Idling);
}

///////////////////////////////////////////////////////////////////////////////
// error handling

bool ExecutionContext::errorNeedsHandling(int errnum,
                                              bool callUserHandler,
                                              ErrorThrowMode mode) {
  if (UNLIKELY(m_throwAllErrors)) {
    throw Exception(folly::sformat("throwAllErrors: {}", errnum));
  }
  if (mode != ErrorThrowMode::Never || errorNeedsLogging(errnum)) {
    return true;
  }
  if (callUserHandler) {
    if (!m_userErrorHandlers.empty() &&
        (m_userErrorHandlers.back().second & errnum) != 0) {
      return true;
    }
  }
  return false;
}

bool ExecutionContext::errorNeedsLogging(int errnum) {
  auto level =
    RID().getErrorReportingLevel() |
    RuntimeOption::ForceErrorReportingLevel;
  return RuntimeOption::NoSilencer || (level & errnum) != 0;
}

struct ErrorStateHelper {
  ErrorStateHelper(ExecutionContext *context,
                   ExecutionContext::ErrorState state) {
    m_context = context;
    m_originalState = m_context->getErrorState();
    m_context->setErrorState(state);
  }
  ~ErrorStateHelper() {
    m_context->setErrorState(m_originalState);
  }
private:
  ExecutionContext *m_context;
  ExecutionContext::ErrorState m_originalState;
};

const StaticString
  s_class("class"),
  s_file("file"),
  s_function("function"),
  s_line("line"),
  s_error_num("error-num"),
  s_error_string("error-string"),
  s_error_file("error-file"),
  s_error_line("error-line"),
  s_error_backtrace("error-backtrace"),
  s_overflow("overflow");

void ExecutionContext::handleError(const std::string& msg,
                                   int errnum,
                                   bool callUserHandler,
                                   ErrorThrowMode mode,
                                   const std::string& prefix,
                                   bool skipFrame /* = false */) {
  SYNC_VM_REGS_SCOPED();

  auto newErrorState = ErrorState::ErrorRaised;
  switch (getErrorState()) {
  case ErrorState::ErrorRaised:
  case ErrorState::ErrorRaisedByUserHandler:
    return;
  case ErrorState::ExecutingUserHandler:
    newErrorState = ErrorState::ErrorRaisedByUserHandler;
    break;
  default:
    break;
  }

  // Potentially upgrade the error to E_USER_ERROR
  if (errnum & RuntimeOption::ErrorUpgradeLevel &
      static_cast<int>(ErrorMode::UPGRADEABLE_ERROR)) {
    errnum = static_cast<int>(ErrorMode::USER_ERROR);
    mode = ErrorThrowMode::IfUnhandled;
  }

  auto const ee = skipFrame ?
    ExtendedException(ExtendedException::SkipFrame{}, msg) :
    ExtendedException(msg);
  bool handled = false;
  {
    ErrorStateHelper esh(this, newErrorState);
    if (callUserHandler) {
      handled = callUserErrorHandler(ee, errnum, false);
    }

    if (!handled) {
      recordLastError(ee, errnum);
    }

    if (g_system_profiler) {
      g_system_profiler->errorCallBack(ee, errnum, msg);
    }
  }

  if (mode == ErrorThrowMode::Always ||
      (mode == ErrorThrowMode::IfUnhandled && !handled)) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerErrorHook(ee, errnum, msg));
    bool isRecoverable =
      errnum == static_cast<int>(ErrorMode::RECOVERABLE_ERROR);
    raise_fatal_error(msg.c_str(), ee.getBacktrace(), isRecoverable,
                      !errorNeedsLogging(errnum) /* silent */);
    not_reached();
  }
  if (!handled) {



    // If we're inside an error handler already, queue it up on the deferred
    // list.
    if (getErrorState() == ErrorState::ExecutingUserHandler) {
      auto& deferred = m_deferredErrors;
      if (deferred.size() < RuntimeOption::EvalMaxDeferredErrors) {
        auto fileAndLine = ee.getFileAndLine();
        deferred.append(
          make_dict_array(
            s_error_num, errnum,
            s_error_string, msg,
            s_error_file, std::move(fileAndLine.first),
            s_error_line, fileAndLine.second,
            s_error_backtrace, ee.getBacktrace()
          )
        );
      } else if (!deferred.empty()) {
        auto const last = deferred.lval(int64_t{deferred.size() - 1});
        if (isDictType(type(last))) {
          asArrRef(last).set(s_overflow, true);
        }
      }
    }

    if (errorNeedsLogging(errnum)) {
      DEBUGGER_ATTACHED_ONLY(phpDebuggerErrorHook(ee, errnum, ee.getMessage()));
      auto fileAndLine = ee.getFileAndLine();
      Logger::Log(Logger::LogError, prefix.c_str(), ee,
                  fileAndLine.first.c_str(), fileAndLine.second);
    }
  }
}

bool ExecutionContext::callUserErrorHandler(const Exception& e, int errnum,
                                                bool swallowExceptions) {
  switch (getErrorState()) {
  case ErrorState::ExecutingUserHandler:
  case ErrorState::ErrorRaisedByUserHandler:
    return false;
  default:
    break;
  }
  if (!m_userErrorHandlers.empty() &&
      (m_userErrorHandlers.back().second & errnum) != 0) {
    auto fileAndLine = std::make_pair(empty_string(), 0);
    Variant backtrace;
    if (auto const ee = dynamic_cast<const ExtendedException*>(&e)) {
      fileAndLine = ee->getFileAndLine();
      backtrace = ee->getBacktrace();
    }
    try {
      ARRPROV_USE_RUNTIME_LOCATION();
      ErrorStateHelper esh(this, ErrorState::ExecutingUserHandler);
      m_deferredErrors = empty_vec_array();
      SCOPE_EXIT { m_deferredErrors = empty_vec_array(); };
      if (!same(vm_call_user_func
                (m_userErrorHandlers.back().first,
                 make_vec_array(errnum, String(e.getMessage()),
                     fileAndLine.first, fileAndLine.second, empty_darray(),
                     backtrace)),
                false)) {
        return true;
      }
    } catch (const RequestTimeoutException&) {
      static auto requestErrorHandlerTimeoutCounter =
          ServiceData::createTimeSeries("requests_timed_out_error_handler",
                                        {ServiceData::StatsType::COUNT});
      requestErrorHandlerTimeoutCounter->addValue(1);
      ServerStats::Log("request.timed_out.error_handler", 1);

      if (!swallowExceptions) throw;
    } catch (const RequestCPUTimeoutException&) {
      static auto requestErrorHandlerCPUTimeoutCounter =
          ServiceData::createTimeSeries("requests_cpu_timed_out_error_handler",
                                        {ServiceData::StatsType::COUNT});
      requestErrorHandlerCPUTimeoutCounter->addValue(1);
      ServerStats::Log("request.cpu_timed_out.error_handler", 1);

      if (!swallowExceptions) throw;
    } catch (const RequestMemoryExceededException&) {
      static auto requestErrorHandlerMemoryExceededCounter =
          ServiceData::createTimeSeries(
              "requests_memory_exceeded_error_handler",
              {ServiceData::StatsType::COUNT});
      requestErrorHandlerMemoryExceededCounter->addValue(1);
      ServerStats::Log("request.memory_exceeded.error_handler", 1);

      if (!swallowExceptions) throw;
    } catch (...) {
      static auto requestErrorHandlerOtherExceptionCounter =
          ServiceData::createTimeSeries(
              "requests_other_exception_error_handler",
              {ServiceData::StatsType::COUNT});
      requestErrorHandlerOtherExceptionCounter->addValue(1);
      ServerStats::Log("request.other_exception.error_handler", 1);

      if (!swallowExceptions) throw;
    }
  }
  return false;
}

bool ExecutionContext::onFatalError(const Exception& e) {
  tl_heap->resetCouldOOM(isStandardRequest());
  RID().resetTimers();
  // need to restore the error reporting level, because the fault
  // handler for silencers won't be run on fatals, and we might be
  // about to run a user error handler (and psp/shutdown code).
  RID().setErrorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel);

  auto prefix = "\nFatal error: ";
  auto errnum = static_cast<int>(ErrorMode::FATAL_ERROR);
  auto const fatal = dynamic_cast<const FatalErrorException*>(&e);
  if (fatal && fatal->isRecoverable()) {
     prefix = "\nCatchable fatal error: ";
     errnum = static_cast<int>(ErrorMode::RECOVERABLE_ERROR);
  }

  recordLastError(e, errnum);

  bool silenced = false;
  auto fileAndLine = std::make_pair(empty_string(), 0);
  if (auto const ee = dynamic_cast<const ExtendedException*>(&e)) {
    silenced = ee->isSilent();
    fileAndLine = ee->getFileAndLine();
  }
  // need to silence even with the AlwaysLogUnhandledExceptions flag set
  if (!silenced && RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(Logger::LogError, prefix, e, fileAndLine.first.c_str(),
                fileAndLine.second);
  }
  bool handled = false;
  if (RuntimeOption::CallUserHandlerOnFatals) {
    handled = callUserErrorHandler(e, errnum, true);
  }
  if (!handled && !silenced && !RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(Logger::LogError, prefix, e, fileAndLine.first.c_str(),
                fileAndLine.second);
  }
  return handled;
}

bool ExecutionContext::onUnhandledException(Object e) {
  String err = throwable_to_string(e.get());
  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("\nFatal error: Uncaught %s", err.data());
  }

  if (e.instanceof(SystemLib::s_ThrowableClass)) {
    ARRPROV_USE_RUNTIME_LOCATION();
    // user thrown exception
    if (!m_userExceptionHandlers.empty()) {
      if (!same(vm_call_user_func
                (m_userExceptionHandlers.back(),
                 make_vec_array(e)),
                false)) {
        return true;
      }
    }
  } else {
    assertx(false);
  }
  m_lastError = err;

  if (!RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("\nFatal error: Uncaught %s", err.data());
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

void ExecutionContext::debuggerInfo(
    std::vector<std::pair<const char*,std::string>>& info) {
  int64_t newInt = convert_bytes_to_long(IniSetting::Get("memory_limit"));
  if (newInt <= 0) {
    newInt = std::numeric_limits<int64_t>::max();
  }
  if (newInt == std::numeric_limits<int64_t>::max()) {
    info.emplace_back("Max Memory", "(unlimited)");
  } else {
    info.emplace_back("Max Memory", IDebuggable::FormatSize(newInt));
  }
  info.emplace_back("Max Time",
                    IDebuggable::FormatTime(RID().getTimeout() * 1000));
}

void ExecutionContext::setenv(const String& name, const String& value) {
  m_envs.set(m_envs.convertKey<IntishCast::Cast>(name),
             make_tv<KindOfString>(value.get()));
}

void ExecutionContext::unsetenv(const String& name) {
  m_envs.remove(name);
}

String ExecutionContext::getenv(const String& name) const {
  if (m_envs.exists(name)) {
    return m_envs[name].toString();
  }
  if (is_cli_server_mode()) {
    auto envs = cli_env();
    if (envs.exists(name)) return envs[name].toString();
    return String();
  }
  if (auto value = ::getenv(name.data())) {
    return String(value, CopyString);
  }
  if (RuntimeOption::EnvVariables.find(name.c_str()) != RuntimeOption::EnvVariables.end()) {
    return String(RuntimeOption::EnvVariables[name.c_str()].data(), CopyString);
  }
  return String();
}

TypedValue ExecutionContext::lookupClsCns(const NamedEntity* ne,
                                      const StringData* cls,
                                      const StringData* cns) {
  Class* class_ = nullptr;
  try {
    class_ = Unit::loadClass(ne, cls);
  } catch (Object& ex) {
    // For compatibility with php, throwing through a constant lookup has
    // different behavior inside a property initializer (86pinit/86sinit).
    auto ar = getStackFrame();
    if (ar && ar->func() && Func::isSpecial(ar->func()->name())) {
      raise_warning("Uncaught %s", throwable_to_string(ex.get()).data());
      raise_error("Couldn't find constant %s::%s", cls->data(), cns->data());
    }
    throw;
  }
  if (class_ == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, cls->data());
  }
  TypedValue clsCns = class_->clsCnsGet(cns);
  if (clsCns.m_type == KindOfUninit) {
    raise_error("Couldn't find constant %s::%s", cls->data(), cns->data());
  }
  return clsCns;
}

static Class* loadClass(StringData* clsName) {
  Class* class_ = Unit::loadClass(clsName);
  if (class_ == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, clsName->data());
  }
  return class_;
}

ObjectData* ExecutionContext::createObject(StringData* clsName,
                                           const Variant& params,
                                           bool init /* = true */) {
  return createObject(loadClass(clsName), params, init);
}

ObjectData* ExecutionContext::createObject(const Class* class_,
                                           const Variant& params,
                                           bool init) {
  callerDynamicConstructChecks(class_);
  auto o = Object::attach(ObjectData::newInstance(const_cast<Class*>(class_)));
  if (init) {
    initObject(class_, params, o.get());
  }

  return o.detach();
}

ObjectData* ExecutionContext::createObjectOnly(StringData* clsName) {
  return createObject(clsName, init_null_variant, false);
}

ObjectData* ExecutionContext::initObject(StringData* clsName,
                                         const Variant& params,
                                         ObjectData* o) {
  return initObject(loadClass(clsName), params, o);
}

ObjectData* ExecutionContext::initObject(const Class* class_,
                                         const Variant& params,
                                         ObjectData* o) {
  auto ctor = class_->getCtor();
  if (!(ctor->attrs() & AttrPublic)) {
    std::string msg = "Access to non-public constructor of class ";
    msg += class_->name()->data();
    Reflection::ThrowReflectionExceptionObject(msg);
  }
  // call constructor
  if (!isContainerOrNull(params)) {
    throw_param_is_not_container();
  }
  tvDecRefGen(invokeFunc(ctor, params, o, nullptr, nullptr, true, false, true));
  return o;
}

ActRec* ExecutionContext::getStackFrame() {
  VMRegAnchor _;
  return vmfp();
}

ObjectData* ExecutionContext::getThis() {
  VMRegAnchor _;
  ActRec* fp = vmfp();
  if (fp->skipFrame()) fp = getPrevVMStateSkipFrame(fp);
  if (fp && fp->func()->cls() && fp->hasThis()) {
    return fp->getThis();
  }
  return nullptr;
}

const RepoOptions& ExecutionContext::getRepoOptionsForCurrentFrame() const {
  VMRegAnchor _;

  if (auto const ar = vmfp()) {
    auto const path = ar->func()->unit()->filepath();
    return RepoOptions::forFile(path->data());
  }
  return RepoOptions::defaults();
}

void ExecutionContext::onLoadWithOptions(
  const char* f, const RepoOptions& opts
) {
  if (!RuntimeOption::EvalFatalOnParserOptionMismatch) return;
  if (!m_requestOptions) {
    m_requestOptions.emplace(opts);
    return;
  }
  if (m_requestOptions != opts) {
    // The data buffer has to stay alive for the call to raise_error.
    auto const path_str = opts.path();
    auto const path = path_str.empty() ? "{default options}" : path_str.data();
    raise_error(
      "Attempting to load file %s with incompatible parser settings from %s, "
      "this request is using parser settings from %s",
      f, path, m_requestOptions->path().data()
    );
  }
}

StringData* ExecutionContext::getContainingFileName() {
  VMRegAnchor _;
  ActRec* ar = vmfp();
  if (ar == nullptr) return staticEmptyString();
  if (ar->skipFrame()) ar = getPrevVMStateSkipFrame(ar);
  if (ar == nullptr) return staticEmptyString();
  Unit* unit = ar->m_func->unit();
  auto const path = ar->m_func->originalFilename() ?
    ar->m_func->originalFilename() : unit->filepath();
  return const_cast<StringData*>(path);
}

int ExecutionContext::getLine() {
  VMRegAnchor _;
  ActRec* ar = vmfp();
  Unit* unit = ar ? ar->m_func->unit() : nullptr;
  Offset pc = unit ? pcOff() : 0;
  if (ar == nullptr) return -1;
  if (ar->skipFrame()) ar = getPrevVMStateSkipFrame(ar, &pc);
  if (ar == nullptr || (unit = ar->m_func->unit()) == nullptr) return -1;
  return unit->getLineNumber(pc);
}

ActRec* ExecutionContext::getFrameAtDepthForDebuggerUnsafe(int frameDepth) {
  ActRec* ret = nullptr;
  walkStack([&] (ActRec* fp, Offset) {
    if (frameDepth == 0) {
      if (fp && !fp->localsDecRefd()) {
        ret = fp;
      }
      return true;
    }

    frameDepth--;
    return false;
  });
  return ret;
}

void ExecutionContext::setVar(StringData* name, tv_rval v) {
  VMRegAnchor _;
  ActRec *fp = vmfp();
  if (!fp) return;
  if (fp->skipFrame()) fp = getPrevVMStateSkipFrame(fp);
  if (fp) fp->getVarEnv()->set(name, v);
}

Array ExecutionContext::getLocalDefinedVariablesDebugger(int frame) {
  const auto fp = getFrameAtDepthForDebuggerUnsafe(frame);
  return getDefinedVariables(fp);
}

bool ExecutionContext::setHeaderCallback(const Variant& callback) {
  if (tvAsVariant(g_context->m_headerCallback).toBoolean()) {
    // return false if a callback has already been set.
    return false;
  }
  tvAsVariant(g_context->m_headerCallback) = callback;
  return true;
}

const StaticString s___HasTopLevelCode("__HasTopLevelCode");
void checkPseudomain(const Unit* unit) {
  auto& attrs = unit->fileAttributes();
  if (attrs.find(s___HasTopLevelCode.get()) != attrs.end()) {
    if (RuntimeOption::EvalWarnOnPseudomain == 1) {
      raise_warning("Found top-level code in %s", unit->filepath()->data());
      return;
    } else if (RuntimeOption::EvalWarnOnPseudomain == 2) {
      throw TopLevelCodeBannedException(unit->filepath()->data());
    }
  }
}

const static StaticString
  s_enter_async_entry_point("__SystemLib\\enter_async_entry_point");

TypedValue ExecutionContext::invokeUnit(const Unit* unit,
                                        bool callByHPHPInvoke) {
  checkHHConfig(unit);

  if (!unit->isHHFile()) {
    throw PhpNotSupportedException(unit->filepath()->data());
  }

  auto ret = invokePseudoMain(unit->getMain(nullptr, false), m_globalVarEnv);

  auto it = unit->getCachedEntryPoint();
  if (callByHPHPInvoke && it != nullptr) {
    if (it->isAsync()) {
      invokeFunc(
        Unit::lookupFunc(s_enter_async_entry_point.get()),
        make_vec_array_tagged(ARRPROV_HERE(), Variant{it}),
        nullptr, nullptr, nullptr, false
      );
    } else {
      invokeFunc(it, init_null_variant, nullptr, nullptr,
                 nullptr, false);
    }
  }
  return ret;
}

void ExecutionContext::syncGdbState() {
  if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitNoGdb) {
    Debug::DebugInfo::Get()->debugSync();
  }
}

void ExecutionContext::pushVMState(TypedValue* savedSP) {
  if (UNLIKELY(!vmfp())) {
    // first entry
    assertx(m_nestedVMs.size() == 0);
    return;
  }

  TRACE(3, "savedVM: %p %p %p %p\n", vmpc(), vmfp(), vmFirstAR(), savedSP);

  auto& savedVM = m_nestedVMs.emplace_back(
    VMState {
      vmpc(),
      vmfp(),
      vmFirstAR(),
      savedSP,
      vmMInstrState(),
      vmJitCalledFrame(),
      vmJitReturnAddr(),
      jit::g_unwind_rds->exn,
      jit::g_unwind_rds->sideEnter
    }
  );
  jit::g_unwind_rds->exn = nullptr;
  jit::g_unwind_rds->sideEnter = false;

  m_nesting++;

  if (debug && savedVM.fp &&
      savedVM.fp->m_func &&
      savedVM.fp->m_func->unit()) {
    // Some asserts and tracing.
    const Func* func = savedVM.fp->m_func;
    /* bound-check asserts in offsetOf */
    func->unit()->offsetOf(savedVM.pc);
    TRACE(3, "pushVMState: saving frame %s pc %p off %d fp %p\n",
          func->name()->data(),
          savedVM.pc,
          func->unit()->offsetOf(savedVM.pc),
          savedVM.fp);
  }
}

void ExecutionContext::popVMState() {
  if (UNLIKELY(m_nestedVMs.empty())) {
    // last exit
    vmfp() = nullptr;
    vmpc() = nullptr;
    vmFirstAR() = nullptr;
    return;
  }

  assertx(m_nestedVMs.size() >= 1);

  VMState &savedVM = m_nestedVMs.back();
  vmpc() = savedVM.pc;
  vmfp() = savedVM.fp;
  vmFirstAR() = savedVM.firstAR;
  vmStack().top() = savedVM.sp;
  vmMInstrState() = savedVM.mInstrState;
  vmJitCalledFrame() = savedVM.jitCalledFrame;
  vmJitReturnAddr() = savedVM.jitReturnAddr;
  jit::g_unwind_rds->exn = savedVM.exn;
  jit::g_unwind_rds->sideEnter = savedVM.unwinderSideEnter;

  if (debug) {
    if (savedVM.fp &&
        savedVM.fp->m_func &&
        savedVM.fp->m_func->unit()) {
      const Func* func = savedVM.fp->m_func;
      (void) /* bound-check asserts in offsetOf */
        func->unit()->offsetOf(savedVM.pc);
      TRACE(3, "popVMState: restoring frame %s pc %p off %d fp %p\n",
            func->name()->data(),
            savedVM.pc,
            func->unit()->offsetOf(savedVM.pc),
            savedVM.fp);
    }
  }

  m_nestedVMs.pop_back();
  m_nesting--;

  TRACE(1, "Reentry: exit fp %p pc %p\n", vmfp(), vmpc());
}

void ExecutionContext::ExcLoggerHook::operator()(
    const char* header, const char* msg, const char* ending
) {
  ec.write(header);
  ec.write(msg);
  ec.write(ending);
  ec.flush();
}

StaticString
  s_php("<?php "),
  s_hh("<?hh "),
  s_namespace("namespace "),
  s_curly_start(" { "),
  s_curly_end(" }"),
  s_function_start("<<__DynamicallyCallable>> function "),
  s_evaluate_default_argument("evaluate_default_argument"),
  s_function_middle("() { return "),
  s_semicolon(";"),
  s_stdclass("stdclass");

void ExecutionContext::requestInit() {
  assertx(SystemLib::s_unit);

  initBlackHole();
  VarEnv::createGlobal();
  vmStack().requestInit();
  ResourceHdr::resetMaxId();
  jit::tc::requestInit();

  if (RuntimeOption::EvalJitEnableRenameFunction) {
    assertx(SystemLib::s_anyNonPersistentBuiltins);
  }

  /*
   * The normal case for production mode is that all builtins are
   * persistent, and every systemlib unit is accordingly going to be
   * merge only.
   *
   * However, if we have rename_function generally enabled, or if any
   * builtin functions were specified as interceptable at
   * repo-generation time, we'll actually need to merge systemlib on
   * every request because some of the builtins will not be marked
   * persistent.
   */
  if (UNLIKELY(SystemLib::s_anyNonPersistentBuiltins)) {
    SystemLib::s_unit->merge();
    SystemLib::mergePersistentUnits();
    if (SystemLib::s_hhas_unit) SystemLib::s_hhas_unit->merge();
  } else {
    // System units are merge only, and everything is persistent.
    assertx(SystemLib::s_unit->isEmpty());
    assertx(!SystemLib::s_hhas_unit || SystemLib::s_hhas_unit->isEmpty());
  }

  profileRequestStart();

  HHProf::Request::StartProfiling();

#ifndef NDEBUG
  Class* cls = NamedEntity::get(s_stdclass.get())->clsList();
  assertx(cls);
  assertx(cls == SystemLib::s_stdclassClass);
#endif

  if (Logger::UseRequestLog) Logger::SetThreadHook(&m_logger_hook);

  // Needs to be last (or nearly last): might cause unit merging to call an
  // extension function in the VM; this is bad if systemlib itself hasn't been
  // merged.
  autoTypecheckRequestInit();
}

void ExecutionContext::requestExit() {
  autoTypecheckRequestExit();
  HHProf::Request::FinishProfiling();

  manageAPCHandle();
  syncGdbState();
  vmStack().requestExit();
  profileRequestEnd();
  EventHook::Disable();
  zend_rand_unseed();
  clearBlackHole();

  if (m_globalVarEnv) {
    req::destroy_raw(m_globalVarEnv);
    m_globalVarEnv = nullptr;
  }

  if (!m_lastError.isNull()) {
    clearLastError();
  }

  {
    ARRPROV_USE_RUNTIME_LOCATION();
    m_deferredErrors = empty_vec_array();
  }

  if (Logger::UseRequestLog) Logger::SetThreadHook(nullptr);
  if (m_requestTrace) record_trace(std::move(*m_requestTrace));
}

template<class Action>
static inline void enterVM(ActRec* ar, Action action) {
  enterVMCustomHandler(ar, [&] { exception_handler(action); });
}

/*
 * Shared implementation for invokeFunc{,Few}().
 *
 * The `doEnterVM' callback take an ActRec* argument corresponding to
 * the reentry frame.
 */
template<class FEnterVM>
ALWAYS_INLINE
TypedValue ExecutionContext::invokeFuncImpl(const Func* f,
                                            ObjectData* thiz, Class* cls,
                                            uint32_t numArgsInclUnpack,
                                            FEnterVM doEnterVM) {
  assertx(f);
  // If `f' is a regular function, `thiz' and `cls' must be null.
  assertx(IMPLIES(!f->implCls(), (!thiz && !cls)));
  // If `f' is a method, either `thiz' or `cls' must be non-null.
  assertx(IMPLIES(f->preClass(), thiz || cls));
  // If `f' is a static method, thiz must be null.
  assertx(IMPLIES(f->isStaticInPrologue(), !thiz));

  if (thiz != nullptr) thiz->incRefCount();

  ActRec* ar = vmStack().indA(numArgsInclUnpack);
  ar->setReturnVMExit();
  ar->m_func = f;
  if (thiz) {
    ar->setThis(thiz);
  } else if (cls) {
    ar->setClass(cls);
  } else {
    ar->trashThis();
  }
  ar->setNumArgs(numArgsInclUnpack);
  ar->trashVarEnv();

#ifdef HPHP_TRACE
  if (vmfp() == nullptr) {
    TRACE(1, "Reentry: enter %s(%p) from top-level\n",
          f->name()->data(), ar);
  } else {
    TRACE(1, "Reentry: enter %s(pc %p ar %p) from %s(%p)\n",
          f->name()->data(), vmpc(), ar,
          vmfp()->m_func ? vmfp()->m_func->name()->data()
                         : "unknownBuiltin",
          vmfp());
  }
#endif

  auto const reentrySP =
    vmStack().top() + numArgsInclUnpack + kNumActRecCells + f->numInOutParams();
  pushVMState(reentrySP);
  SCOPE_EXIT {
    assert_flog(
      vmStack().top() == reentrySP,
      "vmsp() mismatch around reentry: before @ {}, after @ {}",
      reentrySP, vmStack().top()
    );
    popVMState();
  };

  enterVM(ar, [&] { doEnterVM(ar); });

  if (UNLIKELY(f->takesInOutParams())) {
    // This is OK (albeit ugly) since the return value should only be readable
    // from user code via e.g. call_user_func and we will tagTV the result from
    // the native wrapper and getting the correct frame pointer here (or from
    // the normal arrprov::tagFromPC) is awkward.
    ARRPROV_USE_RUNTIME_LOCATION();

    VArrayInit varr(f->numInOutParams() + 1);
    for (uint32_t i = 0; i < f->numInOutParams() + 1; ++i) {
      varr.append(*vmStack().topTV());
      vmStack().popC();
    }
    auto arr = varr.toArray();
    return make_array_like_tv(arr.detach());
  } else {
    auto const retval = *vmStack().topTV();
    vmStack().discard();
    return retval;
  }
}

/**
 * Enter VM by calling action(), which invokes a function or resumes
 * an async function. The 'ar' argument points to an ActRec of the
 * invoked/resumed function.
 */
template<class Action>
static inline void enterVMCustomHandler(ActRec* ar, Action action) {
  assertx(ar);
  assertx(!ar->sfp());
  assertx(isReturnHelper(reinterpret_cast<void*>(ar->m_savedRip)));
  assertx(ar->callOffset() == 0);

  vmFirstAR() = ar;
  vmJitCalledFrame() = nullptr;
  vmJitReturnAddr() = 0;

  action();

  while (vmpc()) {
    exception_handler(enterVMAtCurPC);
  }
}

TypedValue ExecutionContext::invokePseudoMain(const Func* f,
                                              VarEnv* varEnv /* = NULL */,
                                              ObjectData* thiz /* = NULL */,
                                              Class* cls /* = NULL */) {
  assertx(f->isPseudoMain());
  auto toMerge = f->unit();
  toMerge->merge();
  if (toMerge->isMergeOnly()) {
    Stats::inc(Stats::PseudoMain_Skipped);
    return *toMerge->getMainReturn();
  }

  checkPseudomain(toMerge);

  Stats::inc(Stats::PseudoMain_Executed);
  VMRegAnchor _;

  // We must do a stack overflow check for leaf functions on re-entry,
  // because we won't have checked that the stack is deep enough for a
  // leaf function /after/ re-entry, and the prologue for the leaf
  // function will not make a check.
  if (f->isPhpLeafFn()) {
    // Check both the native stack and VM stack for overflow.
    checkStack(vmStack(), f, kNumActRecCells);
  } else {
    // invokePseudoMain() must always check the native stack for overflow no
    // matter what.
    checkNativeStack();
  }

  // Reserve space for ActRec.
  for (auto i = kNumActRecCells; i > 0; --i) vmStack().pushUninit();

  auto const doEnterVM = [&] (ActRec* ar) {
    enterVMAtPseudoMain(ar, varEnv);
  };

  return invokeFuncImpl(f, thiz, cls, 0, doEnterVM);
}

TypedValue ExecutionContext::invokeFunc(const Func* f,
                                        const Variant& args_,
                                        ObjectData* thiz /* = NULL */,
                                        Class* cls /* = NULL */,
                                        StringData* invName /* = NULL */,
                                        bool dynamic /* = true */,
                                        bool checkRefAnnot /* = false */,
                                        bool allowDynCallNoPointer
                                                              /* = false */,
                                        Array&& reifiedGenerics
                                                              /* = Array() */) {
  VMRegAnchor _;

  // We must do a stack overflow check for leaf functions on re-entry,
  // because we won't have checked that the stack is deep enough for a
  // leaf function /after/ re-entry, and the prologue for the leaf
  // function will not make a check.
  if (f->isPhpLeafFn() ||
      !(f->numParams() <= kStackCheckReenterPadding - kNumActRecCells)) {
    // Check both the native stack and VM stack for overflow, numParams
    // is already included in f->maxStackCells().
    checkStack(vmStack(), f, f->numInOutParams() + kNumActRecCells);
  } else {
    // invokeFunc() must always check the native stack for overflow no
    // matter what.
    checkNativeStack();
  }

  // Reserve space for inout outputs and ActRec.
  for (auto i = f->numInOutParams(); i > 0; --i) vmStack().pushUninit();
  for (auto i = kNumActRecCells; i > 0; --i) vmStack().pushUninit();

  // Push arguments.
  auto const& args = *args_.asTypedValue();
  assertx(isContainerOrNull(args));
  auto numArgs = tvIsNull(args) ? 0 : getContainerSize(args);
  if (numArgs == 0) {
    if (UNLIKELY(invName != nullptr)) {
      shuffleMagicArgs(String::attach(invName), 0, false);
      numArgs = 2;
    }
  } else {
    assertx(isContainer(args));
    tvDup(args, *vmStack().allocC());
    if (LIKELY(invName == nullptr)) {
      numArgs = prepareUnpackArgs(f, 0, checkRefAnnot);
    } else {
      shuffleMagicArgs(String::attach(invName), 0, true);
      numArgs = 2;
    }
  }

  // Caller checks.
  if (dynamic) callerDynamicCallChecks(f, allowDynCallNoPointer);

  auto const doEnterVM = [&] (ActRec* ar) {
    enterVMAtFunc(ar, std::move(reifiedGenerics), f->takesInOutParams(),
                  dynamic, allowDynCallNoPointer);
  };

  return invokeFuncImpl(f, thiz, cls, numArgs, doEnterVM);
}

TypedValue ExecutionContext::invokeFuncFew(
  const Func* f,
  ExecutionContext::ThisOrClass thisOrCls,
  StringData* invName,
  uint32_t numArgs,
  const TypedValue* argv,
  bool dynamic /* = true */,
  bool allowDynCallNoPointer
  /* = false */
) {
  VMRegAnchor _;
  auto& stack = vmStack();

  // See comments in invokeFunc().
  if (f->isPhpLeafFn() ||
      !(numArgs <= kStackCheckReenterPadding - kNumActRecCells)) {
    checkStack(stack, f, f->numInOutParams() + kNumActRecCells);
  } else {
    checkNativeStack();
  }

  // Reserve space for inout outputs and ActRec.
  for (auto i = f->numInOutParams(); i > 0; --i) stack.pushUninit();
  for (auto i = kNumActRecCells; i > 0; --i) stack.pushUninit();

  auto const pushPackedArgs = [&](uint32_t from) {
    VArrayInit ai{numArgs - from};
    for (auto i = from; i < numArgs; ++i) ai.append(argv[i]);
    if (RuntimeOption::EvalHackArrDVArrs) {
      stack.pushVecNoRc(ai.create());
    } else {
      stack.pushArrayNoRc(ai.create());
    }
  };

  if (LIKELY(invName == nullptr)) {
    // Push non-variadic arguments.
    auto const numParams = f->numNonVariadicParams();
    for (auto i = 0; i < numArgs && i < numParams; ++i) {
      const TypedValue *from = &argv[i];
      TypedValue* to = stack.allocTV();
      tvDup(*from, *to);
    }

    // Push variadic arguments.
    if (UNLIKELY(numParams < numArgs)) {
      pushPackedArgs(numParams);
      numArgs = numParams + 1;
    }
  } else {
    // Push method name plus varray of args for magic __call().
    stack.pushStringNoRc(invName);
    pushPackedArgs(0);
    numArgs = 2;
  }

  // Caller checks.
  if (dynamic) callerDynamicCallChecks(f, allowDynCallNoPointer);

  auto const doEnterVM = [&] (ActRec* ar) {
    enterVMAtFunc(ar, Array(), f->takesInOutParams(), dynamic, false);
  };

  return invokeFuncImpl(f,
                        thisOrCls.left(),
                        thisOrCls.right(),
                        numArgs, doEnterVM);
}

static void prepareAsyncFuncEntry(ActRec* enterFnAr,
                                  Resumable* resumable,
                                  bool willUnwind) {
  assertx(enterFnAr);
  assertx(enterFnAr->func()->isAsync());
  assertx(isResumed(enterFnAr));
  assertx(resumable);

  vmfp() = enterFnAr;
  // If we're going to unwind, we need to resume at the offset we
  // suspended at, so we look up the correct catch trace (not the one
  // for the next op).
  vmpc() = enterFnAr->func()->unit()->at(
    willUnwind
      ? resumable->suspendOffset()
      : resumable->resumeFromAwaitOffset()
  );
  EventHook::FunctionResumeAwait(enterFnAr);
}

void ExecutionContext::resumeAsyncFunc(Resumable* resumable,
                                       ObjectData* freeObj,
                                       const TypedValue awaitResult) {
  assertx(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assertx(tl_regState == VMRegState::CLEAN); };

  auto fp = resumable->actRec();
  // We don't need to check for space for the ActRec (unlike generally
  // in normal re-entry), because the ActRec isn't on the stack.
  checkStack(vmStack(), fp->func(), 0);

  TypedValue* savedSP = vmStack().top();
  tvDup(awaitResult, *vmStack().allocC());

  // decref after awaitResult is on the stack
  decRefObj(freeObj);

  pushVMState(savedSP);
  SCOPE_EXIT { popVMState(); };

  enterVM(fp, [&] {
    prepareAsyncFuncEntry(fp, resumable, false);

    const bool useJit = RID().getJit();
    if (LIKELY(useJit && resumable->resumeAddr())) {
      Stats::inc(Stats::VMEnter);
      jit::enterTC(resumable->resumeAddr());
    } else {
      enterVMAtCurPC();
    }
  });
}

void ExecutionContext::resumeAsyncFuncThrow(Resumable* resumable,
                                            ObjectData* freeObj,
                                            ObjectData* exception) {
  assertx(exception);
  assertx(exception->instanceof(SystemLib::s_ThrowableClass));
  assertx(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assertx(tl_regState == VMRegState::CLEAN); };

  auto fp = resumable->actRec();
  checkStack(vmStack(), fp->func(), 0);

  // decref after we hold reference to the exception
  Object e(exception);
  decRefObj(freeObj);

  pushVMState(vmStack().top());
  SCOPE_EXIT { popVMState(); };

  enterVMCustomHandler(fp, [&] {
    prepareAsyncFuncEntry(fp, resumable, true);
    unwindVM(exception);
    e.reset();
  });
}

ActRec* ExecutionContext::getPrevVMState(const ActRec* fp,
                                         Offset* prevPc /* = NULL */,
                                         TypedValue** prevSp /* = NULL */,
                                         bool* fromVMEntry /* = NULL */,
                                         uint64_t* jitReturnAddr /* = NULL */) {
  if (fp == nullptr) {
    return nullptr;
  }
  ActRec* prevFp = fp->sfp();
  if (LIKELY(prevFp != nullptr)) {
    if (prevSp) {
      if (UNLIKELY(isResumed(fp))) {
        assertx(fp->func()->isGenerator());
        *prevSp = (TypedValue*)prevFp - prevFp->func()->numSlotsInFrame();
      } else {
        *prevSp = (TypedValue*)(fp + 1);
      }
    }
    if (prevPc) *prevPc = prevFp->func()->base() + fp->callOffset();
    if (fromVMEntry) *fromVMEntry = false;
    return prevFp;
  }
  // Linear search from end of m_nestedVMs. In practice, we're probably
  // looking for something recently pushed.
  int i = m_nestedVMs.size() - 1;
  ActRec* firstAR = vmFirstAR();
  while (i >= 0 && firstAR != fp) {
    firstAR = m_nestedVMs[i--].firstAR;
  }
  if (i == -1) return nullptr;
  const VMState& vmstate = m_nestedVMs[i];
  prevFp = vmstate.fp;
  assertx(prevFp);
  assertx(prevFp->func()->unit());
  if (prevSp) *prevSp = vmstate.sp;
  if (prevPc) {
    *prevPc = prevFp->func()->unit()->offsetOf(vmstate.pc);
  }
  if (fromVMEntry) *fromVMEntry = true;
  if (jitReturnAddr) *jitReturnAddr = (uint64_t)vmstate.jitReturnAddr;
  return prevFp;
}

/*
  Instantiate hoistable classes and functions.
  If there is any more work left to do, setup a
  new frame ready to execute the pseudomain.

  return true iff the pseudomain needs to be executed.
*/
bool ExecutionContext::evalUnit(Unit* unit, PC callPC, PC& pc, int funcType) {
  vmpc() = callPC;
  unit->merge();
  if (unit->isMergeOnly()) {
    Stats::inc(Stats::PseudoMain_Skipped);
    *vmStack().allocTV() = *unit->getMainReturn();
    return false;
  }

  checkPseudomain(unit);

  Stats::inc(Stats::PseudoMain_Executed);

  ActRec* ar = vmStack().allocA();
  auto const cls = vmfp()->func()->cls();
  auto const hasThis = cls && !vmfp()->func()->isStatic();
  auto const func = unit->getMain(cls, hasThis);
  assertx(!func->isCPPBuiltin());
  ar->m_func = func;
  if (cls) {
    ar->setThisOrClass(vmfp()->getThisOrClass());
    if (hasThis) ar->getThis()->incRefCount();
  } else {
    ar->trashThis();
  }
  ar->setNumArgs(0);
  assertx(vmfp());
  ar->setReturn(vmfp(), callPC, jit::tc::ustubs().retHelper, false);
  pushFrameSlots(func);

  auto prevFp = vmfp();
  if (UNLIKELY(prevFp->skipFrame())) {
    prevFp = g_context->getPrevVMStateSkipFrame(prevFp);
  }
  assertx(prevFp);
  assertx(prevFp->func()->attrs() & AttrMayUseVV);
  if (!prevFp->hasVarEnv()) {
    prevFp->setVarEnv(VarEnv::createLocal(prevFp));
  }
  ar->m_varEnv = prevFp->m_varEnv;
  ar->m_varEnv->enterFP(prevFp, ar);

  vmfp() = ar;
  pc = func->getEntry();
  vmpc() = pc;
  bool ret = EventHook::FunctionCall(vmfp(), funcType);
  pc = vmpc();
  checkStack(vmStack(), func, 0);
  return ret;
}

Variant ExecutionContext::getEvaledArg(const StringData* val,
                                       const String& namespacedName,
                                       const Unit* funcUnit) {
  auto key = StrNR(val);

  if (m_evaledArgs.get()) {
    auto const arg = m_evaledArgs.get()->get(key);
    if (!arg.is_dummy()) return Variant::wrap(arg.tv());
  }

  String code;
  String funcName;
  int pos = namespacedName.rfind('\\');
  if (pos != -1) {
    auto ns = namespacedName.substr(0, pos);
    code = (funcUnit->isHHFile() ? s_hh : s_php) +
      s_namespace + ns + s_curly_start + s_function_start +
      s_evaluate_default_argument + s_function_middle + key +
      s_semicolon + s_curly_end + s_curly_end;
    funcName = ns + "\\" + s_evaluate_default_argument;
  } else {
    code = (funcUnit->isHHFile() ? s_hh : s_php) +
      s_function_start + s_evaluate_default_argument + s_function_middle +
      key + s_semicolon + s_curly_end;
    funcName = s_evaluate_default_argument;
  }

  // This unit needs to have a name, so that we have provenance for its arrays.
  auto const name = folly::to<std::string>(
    funcUnit->filepath()->data(), " @ getEvaledArg @ ", namespacedName.data());
  Unit* unit = compileEvalString(code.get(), name.data());
  assertx(unit != nullptr);
  unit->setInterpretOnly();

  // The evaluate_default_argument function should be the last one
  auto func = *(unit->funcs().end() - 1);
  assertx(func->name()->equal(funcName.get()) &&
          "We expecting the evaluate_default_argument func");

  // Default arg values are not currently allowed to depend on class context.
  auto v = Variant::attach(
    g_context->invokeFuncFew(func, nullptr)
  );
  auto const lv = m_evaledArgs.lvalForce(key, AccessFlags::Key);
  tvSet(*v.asTypedValue(), lv);
  return Variant::wrap(lv.tv());
}

void ExecutionContext::recordLastError(const Exception& e, int errnum) {
  m_lastError = String(e.getMessage());
  m_lastErrorNum = errnum;
  m_lastErrorPath = String::attach(getContainingFileName());
  m_lastErrorLine = getLine();
  if (auto const ee = dynamic_cast<const ExtendedException*>(&e)) {
    m_lastErrorPath = ee->getFileAndLine().first;
    m_lastErrorLine = ee->getFileAndLine().second;
  }
}

void ExecutionContext::clearLastError() {
  m_lastError = String();
  m_lastErrorNum = 0;
  m_lastErrorPath = staticEmptyString();
  m_lastErrorLine = 0;
}

void ExecutionContext::enqueueAPCHandle(APCHandle* handle, size_t size) {
  assertx(handle->isUncounted());
  if (RuntimeOption::EvalGCForAPC) {
    // Register handle with APCGCManager
    // And resursively find all allocations belong to handle, register them too
    APCGCManager::getInstance().registerPendingDeletion(handle, size);
  }
  m_apcHandles.push_back(handle);
  m_apcMemSize += size;
}

// Treadmill solution for the SharedVariant memory management
namespace {
struct FreedAPCHandle {
  explicit FreedAPCHandle(std::vector<APCHandle*>&& shandles, size_t size)
    : m_memSize(size), m_apcHandles(std::move(shandles))
  {}
  void operator()() {
    if (RuntimeOption::EvalGCForAPC) {
      // Treadmill ask APCGCManager to free the handles
      APCGCManager::getInstance().freeAPCHandles(m_apcHandles);
    } else {
      for (auto handle : m_apcHandles) {
        APCTypedValue::fromHandle(handle)->deleteUncounted();
      }
    }
    APCStats::getAPCStats().removePendingDelete(m_memSize);
  }
private:
  size_t m_memSize;
  std::vector<APCHandle*> m_apcHandles;
};
}

void ExecutionContext::manageAPCHandle() {
  assertx(apcExtension::UseUncounted || m_apcHandles.size() == 0);
  if (m_apcHandles.size() > 0) {
    std::vector<APCHandle*> handles;
    handles.swap(m_apcHandles);
    Treadmill::enqueue(
      FreedAPCHandle(std::move(handles), m_apcMemSize)
    );
    APCStats::getAPCStats().addPendingDelete(m_apcMemSize);
  }
}

// Evaled units have a footprint in the TC and translation metadata. The
// applications we care about tend to have few, short, stereotyped evals,
// where the same code keeps getting eval'ed over and over again; so we
// keep around units for each eval'ed string, so that the TC space isn't
// wasted on each eval.
typedef RankedCHM<StringData*, HPHP::Unit*,
        StringDataHashCompare,
        RankEvaledUnits> EvaledUnitsMap;
static EvaledUnitsMap s_evaledUnits;
Unit* ExecutionContext::compileEvalString(
    StringData* code,
    const char* evalFilename /* = nullptr */) {
  EvaledUnitsMap::accessor acc;
  // Promote this to a static string; otherwise it may get swept
  // across requests.
  code = makeStaticString(code);
  if (s_evaledUnits.insert(acc, code)) {
    acc->second = compile_string(
      code->data(),
      code->size(),
      evalFilename,
      Native::s_noNativeFuncs,
      getRepoOptionsForCurrentFrame()
    );
  }
  return acc->second;
}

ExecutionContext::EvaluationResult
ExecutionContext::evalPHPDebugger(StringData* code, int frame) {
  // The code has "<?php" prepended already
  auto unit = compile_debugger_string(code->data(), code->size(),
    getRepoOptionsForCurrentFrame());
  if (unit == nullptr) {
    raise_error("Syntax error");
    return {true, init_null_variant, "Syntax error"};
  }

  return evalPHPDebugger(unit, frame);
}

const StaticString
  s_DebuggerMainAttr("__DebuggerMain"),
  s_uninitClsName("__uninitSentinel");

ExecutionContext::EvaluationResult
ExecutionContext::evalPHPDebugger(Unit* unit, int frame) {
  always_assert(!RuntimeOption::RepoAuthoritative);

  // Do not JIT this unit, we are using it exactly once.
  unit->setInterpretOnly();

  VMRegAnchor _;

  auto fp = getFrameAtDepthForDebuggerUnsafe(frame);

  // Continue walking up the stack until we find a frame that can have
  // a variable environment context attached to it, or we run out out frames.
  while (fp && (fp->skipFrame() || fp->isInlined())) {
    fp = getPrevVMStateSkipFrame(fp);
  }

  if (fp && !fp->hasVarEnv()) {
    fp->setVarEnv(VarEnv::createLocal(fp));
  }
  ObjectData *this_ = nullptr;
  // NB: the ActRec and function within the AR may have different classes. The
  // class in the ActRec is the type used when invoking the function (i.e.,
  // Derived in Derived::Foo()) while the class obtained from the function is
  // the type that declared the function Foo, which may be Base. We need both
  // the class to match any object that this function may have been invoked on,
  // and we need the class from the function execution is stopped in.
  Class *frameClass = nullptr;
  Class *functionClass = nullptr;
  if (fp) {
    functionClass = fp->m_func->cls();
    if (functionClass) {
      if (fp->hasThis()) {
        this_ = fp->getThis();
      } else if (fp->hasClass()) {
        frameClass = fp->getClass();
      }
    }
    phpDebuggerEvalHook(fp->m_func);
  }

  const static StaticString s_cppException("Hit an exception");
  const static StaticString s_phpException("Hit a php exception");
  const static StaticString s_exit("Hit exit");
  const static StaticString s_fatal("Hit fatal");
  std::ostringstream errorString;
  std::string stack;

  // Find a suitable PC to use when switching to the target frame. If the target
  // is the current frame, this is just vmpc(). For other cases, this will
  // generally be the address of a call from that frame's function. If we can't
  // find the target frame (because it lies deeper in the stack), then just use
  // the target frame's func's entry point.
  auto const findSuitablePC = [this](const ActRec* target){
    if (auto fp = vmfp()) {
      if (fp == target) return vmpc();
      while (true) {
        auto prevFp = getPrevVMState(fp);
        if (!prevFp) break;
        if (prevFp == target) {
          return prevFp->func()->getEntry() + fp->callOffset();
        }
        fp = prevFp;
      }
    }
    return target->func()->getEntry();
  };

  try {
    // Start with the correct parent FP so that VarEnv can properly exitFP().
    // Note that if the same VarEnv is used across multiple frames, the most
    // recent FP must be used. This can happen if we are trying to debug
    // an eval() call or a call issued by debugger itself.
    //
    // We also need to change vmpc() to match, since we assert in a few places
    // that the vmpc() lies within vmfp()'s code.
    auto savedFP = vmfp();
    auto savedPC = vmpc();
    if (fp) {
      auto newFp = fp->m_varEnv->getFP();
      assertx(!newFp->skipFrame());
      vmpc() = findSuitablePC(newFp);
      vmfp() = newFp;
    }
    SCOPE_EXIT { vmpc() = savedPC; vmfp() = savedFP; };

    invokePseudoMain(
      unit->getMain(functionClass, this_),
      fp ? fp->m_varEnv : nullptr,
      this_,
      frameClass
    );

    enum VarAction { StoreFrame, StoreVV, StoreEnv };

    auto const uninit_cls = Unit::loadClass(s_uninitClsName.get());

    Array globals{get_global_variables()};
    auto& env = [&] () -> Array& {
      if (fp->m_varEnv && fp->m_varEnv->isGlobalScope()) return globals;
      if (m_debuggerEnv.isNull()) m_debuggerEnv = Array::CreateDArray();
      return m_debuggerEnv;
    }();

    auto const ctx = fp ? fp->func()->cls() : nullptr;
    auto const f = [&] () -> Func* {
      for (auto orig_f : unit->funcs()) {
        if (orig_f->userAttributes().count(s_DebuggerMainAttr.get())) {
          auto const f = ctx ? orig_f->clone(ctx) : orig_f;
          if (ctx) {
            f->setBaseCls(ctx);
            if (fp && fp->hasThis()) {
              f->setAttrs(Attr(f->attrs() & ~AttrStatic));
            } else if (fp && fp->hasClass()) {
              f->setAttrs(Attr(f->attrs() | AttrStatic));
            }
          }
          assertx(f->numParams() >= 1 && f->numParams() == f->numInOutParams());
          return f;
        }
      }
      return nullptr;
    }();
    always_assert(f);

    VArrayInit args{f->numParams()};
    std::vector<VarAction> actions{f->numParams(), StoreEnv};
    std::vector<Id> frameIds;
    frameIds.resize(f->numParams(), 0);
    auto const appendUninit = [&] {
      args.append(make_tv<KindOfObject>(Object{uninit_cls}.detach()));
    };
    for (Id id = 0; id < f->numParams() - 1; id++) {
      assertx(id < f->numNamedLocals());
      assertx(f->params()[id].isInOut());
      if (fp) {
        auto const idx = fp->func()->lookupVarId(f->localVarName(id));
        if (idx != kInvalidId) {
          auto const var = tvAsVariant(frame_local(fp, idx));
          if (var.isInitialized()) args.append(var);
          else appendUninit();
          actions[id] = StoreFrame;
          frameIds[id] = idx;
          continue;
        }
        if (fp->hasVarEnv()) {
          auto const tv = fp->getVarEnv()->lookup(f->localVarName(id));
          if (tv) {
            if (type(tv) != KindOfUninit) args.append(tvAsVariant(*tv));
            else appendUninit();
            actions[id] = StoreVV;
            continue;
          }
        }
      }
      auto const val = env.rval(StrNR{f->localVarName(id)});
      if (val && type(val) != KindOfUninit) args.append(val.tv());
      else appendUninit();
    }
    args.append(make_tv<KindOfNull>()); // $__debugger_exn$output

    auto const obj = ctx && fp->hasThis() ? fp->getThis() : nullptr;
    auto const cls = ctx && fp->hasClass() ? fp->getClass() : nullptr;
    auto const arr_tv = invokeFunc(f, args.toArray(), obj, cls,
                                   nullptr, false, false, false, Array());
    assertx(isArrayLikeType(type(arr_tv)));
    assertx(val(arr_tv).parr->size() == f->numParams() + 1);
    Array arr = Array::attach(val(arr_tv).parr);
    for (Id id = 0; id < f->numParams() - 1; id++) {
      auto const rval = arr.rval(id + 1);
      if (isObjectType(type(rval)) &&
          val(rval).pobj->instanceof(uninit_cls)) {
        switch (actions[id]) {
        case StoreFrame:
          tvAsVariant(frame_local(fp, frameIds[id])).unset();
          break;
        case StoreVV:
          fp->m_varEnv->unset(f->localVarName(id));
          break;
        case StoreEnv:
          env.remove(StrNR{f->localVarName(id)});
          break;
        }
        continue;
      }
      switch (actions[id]) {
      case StoreFrame:
        tvAsVariant(frame_local(fp, frameIds[id])) = arr[id + 1];
        break;
      case StoreVV:
        fp->m_varEnv->set(f->localVarName(id), rval);
        break;
      case StoreEnv:
        env.set(StrNR{f->localVarName(id)}, rval.tv());
        break;
      }
    }
    auto const exn = arr[int64_t(f->numParams())];
    if (exn.isObject()) {
      assertx(exn.toObject().instanceof("Throwable"));
      throw_object(exn.toObject());
    }
    assertx(exn.isNull());

    return {false, arr[0], ""};
  } catch (FatalErrorException& e) {
    errorString << s_fatal.data();
    errorString << " : ";
    errorString << e.getMessage().c_str();
    errorString << "\n";
    stack = ExtendedLogger::StringOfStackTrace(e.getBacktrace());
  } catch (ExitException& e) {
    errorString << s_exit.data();
    errorString << " : ";
    errorString << *rl_exit_code;
  } catch (Eval::DebuggerException& e) {
  } catch (Exception& e) {
    errorString << s_cppException.data();
    errorString << " : ";
    errorString << e.getMessage().c_str();
    ExtendedException* ee = dynamic_cast<ExtendedException*>(&e);
    if (ee) {
      errorString << "\n";
      stack = ExtendedLogger::StringOfStackTrace(ee->getBacktrace());
    }
  } catch (Object &e) {
    errorString << s_phpException.data();
    errorString << " : ";
    try {
      errorString << throwable_to_string(e.get()).data();
    } catch (...) {
      errorString << e->getVMClass()->name()->data();
    }
  } catch (...) {
    errorString << s_cppException.data();
  }

  auto errorStr = errorString.str();
  g_context->write(errorStr);
  if (!stack.empty()) {
    g_context->write(stack.c_str());
  }

  return {true, init_null_variant, errorStr};
}

void ExecutionContext::enterDebuggerDummyEnv() {
  static Unit* s_debuggerDummy = compile_debugger_string(
    "<?php?>", 7, RepoOptions::defaults()
  );
  // Ensure that the VM stack is completely empty (vmfp() should be null)
  // and that we're not in a nested VM (reentrancy)
  assertx(vmfp() == nullptr);
  assertx(m_nestedVMs.size() == 0);
  assertx(m_nesting == 0);
  assertx(vmStack().count() == 0);
  ActRec* ar = vmStack().allocA();
  ar->m_func = s_debuggerDummy->getMain(nullptr, false);
  ar->setNumArgs(0);
  ar->trashThis();
  ar->setReturnVMExit();
  vmfp() = ar;
  vmpc() = s_debuggerDummy->entry();
  vmFirstAR() = ar;
  vmfp()->setVarEnv(m_globalVarEnv);
  m_globalVarEnv->enterFP(nullptr, vmfp());
}

void ExecutionContext::exitDebuggerDummyEnv() {
  assertx(m_globalVarEnv);
  // Ensure that vmfp() is valid
  assertx(vmfp() != nullptr);
  // Ensure that vmfp() points to the only frame on the call stack.
  // In other words, make sure there are no VM frames directly below
  // this one and that we are not in a nested VM (reentrancy)
  assertx(!vmfp()->sfp());
  assertx(m_nestedVMs.size() == 0);
  assertx(m_nesting == 0);
  // Teardown the frame we erected by enterDebuggerDummyEnv()
  const Func* func = vmfp()->m_func;
  try {
    vmfp()->setLocalsDecRefd();
    frame_free_locals_no_hook(vmfp());
  } catch (...) {}
  vmStack().ndiscard(func->numSlotsInFrame());
  vmStack().discardAR();
  // After tearing down this frame, the VM stack should be completely empty
  assertx(vmStack().count() == 0);
  vmfp() = nullptr;
  vmpc() = nullptr;
}

ThrowAllErrorsSetter::ThrowAllErrorsSetter() {
  m_throwAllErrors = g_context->getThrowAllErrors();
  g_context->setThrowAllErrors(true);
}

ThrowAllErrorsSetter::~ThrowAllErrorsSetter() {
  g_context->setThrowAllErrors(m_throwAllErrors);
}

}
