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
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/apc-gc-manager.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/system-profiler.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/ext/std/ext_std_output.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/reflection/ext_reflection.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/enter-tc.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/hh-utils.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/act-rec-defs.h"
#include "hphp/runtime/vm/interp-helpers.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(bcinterp);

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ExecutionContext, g_context);

ExecutionContext::ExecutionContext()
  : m_transport(nullptr)
  , m_sb(nullptr)
  , m_implicitFlush(false)
  , m_protectedLevel(0)
  , m_stdout(nullptr)
  , m_stdoutData(nullptr)
  , m_stdoutBytesWritten(0)
  , m_errorState(ExecutionContext::ErrorState::NoError)
  , m_lastErrorNum(0)
  , m_deferredErrors(staticEmptyVecArray())
  , m_throwAllErrors(false)
  , m_pageletTasksStarted(0)
  , m_vhost(nullptr)
  , m_globalVarEnv(nullptr)
  , m_lambdaCounter(0)
  , m_nesting(0)
  , m_dbgNoBreak(false)
  , m_unwindingCppException(false)
  , m_lastErrorPath(staticEmptyString())
  , m_lastErrorLine(0)
  , m_executingSetprofileCallback(false)
{
  resetCoverageCounters();
  // We don't want a new execution context to cause any request-heap
  // allocations (because it will cause us to hold a slab, even while idle).
  static auto s_cwd = makeStaticString(Process::CurrentWorkingDirectory);
  m_cwd = s_cwd;
  RID().setMemoryLimit(std::to_string(RuntimeOption::RequestMemoryMaxBytes));
  RID().setErrorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel);

  VariableSerializer::serializationSizeLimit =
    RuntimeOption::SerializationSizeLimit;
  tvWriteUninit(&m_headerCallback);
}

// See header for why this is required.
#ifndef _MSC_VER
template<>
#endif
void ThreadLocalNoCheck<ExecutionContext>::destroy() {
  if (!isNull()) {
    getNoCheck()->sweep();
    setNull();
  }
}


void ExecutionContext::cleanup() {
  manageAPCHandle();

  // Discard all units that were created via create_function().
  for (auto& v : m_createdFuncs) delete v;
  m_createdFuncs.clear();
}

void ExecutionContext::sweep() {
  cleanup();
}

ExecutionContext::~ExecutionContext() {
  // When we destroy the execution context will call destructors on any objects
  // in the userErrorHandlers and userExceptionHandlers vectors. If these
  // destructors call restore_*_handler() they can trigger a pop_back() on the
  // vector resulting in double destruction. There's no reason for code to do
  // this but we should still avoid crashing.
  // N.B.: This is already taken care of for us if EnableObjDestructCall is on
  if (!RuntimeOption::EnableObjDestructCall) {
    while (!m_userErrorHandlers.empty()) m_userErrorHandlers.pop_back();
    while (!m_userExceptionHandlers.empty()) m_userExceptionHandlers.pop_back();
  }
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

void ExecutionContext::setStdout(PFUNC_STDOUT func, void *data) {
  m_stdout = func;
  m_stdoutData = data;
}

static void safe_stdout(const  void  *ptr,  size_t  size) {
  write(fileno(stdout), ptr, size);
}

void ExecutionContext::writeStdout(const char *s, int len) {
  fflush(stdout);
  if (m_stdout == nullptr) {
    if (s_stdout_color) {
      safe_stdout(s_stdout_color, strlen(s_stdout_color));
      safe_stdout(s, len);
      safe_stdout(ANSI_COLOR_END, strlen(ANSI_COLOR_END));
    } else {
      safe_stdout(s, len);
    }
    m_stdoutBytesWritten += len;
  } else {
    m_stdout(s, len, m_stdoutData);
  }
}

void ExecutionContext::writeTransport(const char *s, int len) {
  if (m_transport) {
    m_transport->sendRaw((void*)s, len, 200, false, true);
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
                        make_packed_array(last->oss.detach(), handler_flag));
    }
    last->oss.clear();
  }
}

bool ExecutionContext::obFlush(bool force /*= false*/) {
  assert(m_protectedLevel >= 0);

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
          m_insideOBHandler = true;
          SCOPE_EXIT { m_insideOBHandler = false; };
          tout = vm_call_user_func(
            last.handler, make_packed_array(str, flag)
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
        m_insideOBHandler = true;
        SCOPE_EXIT { m_insideOBHandler = false; };
        tout = vm_call_user_func(
          last.handler, make_packed_array(str, flag)
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
  assert(m_protectedLevel >= 0);
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
  assert((int)m_buffers.size() >= m_protectedLevel);
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
  Array ret = Array::Create();
  int level = 0;
  for (auto& buffer : m_buffers) {
    Array status;
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
  Array ret;
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
                                                Array arguments,
                                                ShutdownType type) {
  Array callback = make_map_array(s_name, function, s_args, arguments);
  Variant& funcs = m_shutdowns.lvalAt(type);
  forceToArray(funcs).append(callback);
}

bool ExecutionContext::removeShutdownFunction(const Variant& function,
                                              ShutdownType type) {
  bool ret = false;
  auto& funcs = forceToArray(m_shutdowns.lvalAt(type));
  PackedArrayInit newFuncs(funcs.size());

  for (ArrayIter iter(funcs); iter; ++iter) {
    if (!same(iter.second().toArray()[s_name], function)) {
      newFuncs.appendWithRef(iter.secondVal());
    } else {
      ret = true;
    }
  }
  funcs = newFuncs.toArray();
  return ret;
}

bool ExecutionContext::hasShutdownFunctions(ShutdownType type) {
  return !m_shutdowns.isNull() && m_shutdowns.exists(type) &&
    m_shutdowns[type].toArray().size() >= 1;
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
  assert(handler && handler->getInited());
  assert(m_acceptRequestEventHandlers);
  m_requestEventHandlers.push_back(handler);
  return m_requestEventHandlers.size()-1;
}

void ExecutionContext::unregisterRequestEventHandler(
  RequestEventHandler* handler,
  std::size_t index) {
  assert(index < m_requestEventHandlers.size() &&
         m_requestEventHandlers[index] == handler);
  assert(!handler->getInited());
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
      assert(handler->getInited());
      handler->requestShutdown();
      handler->setInited(false);
    }
  }
}

void ExecutionContext::executeFunctions(ShutdownType type) {
  RID().resetTimer(RuntimeOption::PspTimeoutSeconds);
  RID().resetCPUTimer(RuntimeOption::PspCpuTimeoutSeconds);

  if (!m_shutdowns.isNull() && m_shutdowns.exists(type)) {
    SCOPE_EXIT {
      try { m_shutdowns.remove(type); } catch (...) {}
    };
    // We mustn't destroy any callbacks until we're done with all
    // of them. So hold them in tmp.
    Array tmp;
    while (true) {
      auto& var = m_shutdowns.lvalAt(type);
      if (!var.isArray()) break;
      auto funcs = var.toArray();
      var.unset();
      for (int pos = 0; pos < funcs.size(); ++pos) {
        Array callback = funcs[pos].toArray();
        vm_call_user_func(callback[s_name], callback[s_args].toArray());
      }
      tmp.append(funcs);
    }
  }
}

void ExecutionContext::onShutdownPreSend() {
  // in case obStart was called without obFlush
  SCOPE_EXIT {
    try { obFlushAll(); } catch (...) {}
  };

  MM().resetCouldOOM(isStandardRequest());
  executeFunctions(ShutDown);
}

extern void ext_session_request_shutdown();

void ExecutionContext::onShutdownPostSend() {
  ServerStats::SetThreadMode(ServerStats::ThreadMode::PostProcessing);
  MM().resetCouldOOM(isStandardRequest());
  try {
    try {
      ServerStatsHelper ssh("psp", ServerStatsHelper::TRACK_HWINST);
      executeFunctions(PostSend);
    } catch (...) {
      try {
        bump_counter_and_rethrow(true /* isPsp */);
      } catch (const ExitException &e) {
        // do nothing
      } catch (const Exception &e) {
        onFatalError(e);
      } catch (const Object &e) {
        onUnhandledException(e);
      }
    }
  } catch (...) {
    Logger::Error("unknown exception was thrown from psp");
  }

  /*
   * This has to happen before requestEventHandler shutdown hooks,
   * because it can run user code which may need to access other
   * RequestLocal objects (such as the stream registry).
   */
  ext_session_request_shutdown();

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
  if (mode != ErrorThrowMode::Never || errorNeedsLogging(errnum) ||
      RID().hasTrackErrors()) {
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
  s_php_errormsg("php_errormsg"),
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
    VMRegAnchor _;
    auto fp = vmfp();

    if (RID().hasTrackErrors() && fp) {
      // Set $php_errormsg in the parent scope
      Variant msg(ee.getMessage());
      if (fp->func()->isBuiltin()) {
        fp = getPrevVMState(fp);
      }
      assert(fp);
      auto id = fp->func()->lookupVarId(s_php_errormsg.get());
      if (id != kInvalidId) {
        auto local = frame_local(fp, id);
        tvSet(*msg.asTypedValue(), *tvToCell(local));
      } else if ((fp->func()->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
        fp->getVarEnv()->set(s_php_errormsg.get(), msg.asTypedValue());
      }
    }

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
        auto& last = deferred.lvalAt(int64_t{deferred.size() - 1});
        if (last.isDict()) last.asArrRef().set(s_overflow, true);
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

bool ExecutionContext::callUserErrorHandler(const Exception &e, int errnum,
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
      ErrorStateHelper esh(this, ErrorState::ExecutingUserHandler);
      auto const ar = g_context->getFrameAtDepth(0);
      auto const context = RuntimeOption::EnableContextInErrorHandler
        ? getDefinedVariables(ar)
        : empty_array();
      m_deferredErrors = Array::CreateVec();
      SCOPE_EXIT { m_deferredErrors = Array::CreateVec(); };
      if (!same(vm_call_user_func
                (m_userErrorHandlers.back().first,
                 make_packed_array(errnum, String(e.getMessage()),
                     fileAndLine.first, fileAndLine.second, context,
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

bool ExecutionContext::onFatalError(const Exception &e) {
  MM().resetCouldOOM(isStandardRequest());
  RID().resetTimer();

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
  if (auto const ee = dynamic_cast<const ExtendedException *>(&e)) {
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
  String err = e.toString();
  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("\nFatal error: Uncaught %s", err.data());
  }

  if (e.instanceof(SystemLib::s_ThrowableClass)) {
    // user thrown exception
    if (!m_userExceptionHandlers.empty()) {
      if (!same(vm_call_user_func
                (m_userExceptionHandlers.back(),
                 make_packed_array(e)),
                false)) {
        return true;
      }
    }
  } else {
    assert(false);
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
  m_envs.set(name, value);
}

void ExecutionContext::unsetenv(const String& name) {
  m_envs.remove(name);
}

String ExecutionContext::getenv(const String& name) const {
  if (m_envs.exists(name)) {
    return m_envs[name].toString();
  }
  if (is_cli_mode()) {
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

Cell ExecutionContext::lookupClsCns(const NamedEntity* ne,
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
      raise_warning("Uncaught %s", ex.toString().data());
      raise_error("Couldn't find constant %s::%s", cls->data(), cns->data());
    }
    throw;
  }
  if (class_ == nullptr) {
    raise_error(Strings::UNKNOWN_CLASS, cls->data());
  }
  Cell clsCns = class_->clsCnsGet(cns);
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
  auto o = Object::attach(newInstance(const_cast<Class*>(class_)));
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
  tvDecRefGen(invokeFunc(ctor, params, o));
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

Class* ExecutionContext::getContextClass() {
  VMRegAnchor _;
  ActRec* ar = vmfp();
  assert(ar != nullptr);
  if (ar->skipFrame()) ar = getPrevVMStateSkipFrame(ar);
  return ar ? ar->m_func->cls() : nullptr;
}

Class* ExecutionContext::getParentContextClass() {
  if (Class* ctx = getContextClass()) {
    return ctx->parent();
  }
  return nullptr;
}

StringData* ExecutionContext::getContainingFileName() {
  VMRegAnchor _;
  ActRec* ar = vmfp();
  if (ar == nullptr) return staticEmptyString();
  if (ar->skipFrame()) ar = getPrevVMStateSkipFrame(ar);
  if (ar == nullptr) return staticEmptyString();
  Unit* unit = ar->m_func->unit();
  assert(unit->filepath()->isStatic());
  // XXX: const StringData* -> Variant(bool) conversion problem makes this ugly
  return const_cast<StringData*>(unit->filepath());
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

const StaticString s___call("__call");
const StaticString s___callStatic("__callStatic");
const StaticString s_call_user_func("call_user_func");
const StaticString s_call_user_func_array("call_user_func_array");

Array ExecutionContext::getCallerInfo() {
  VMRegAnchor _;
  auto ar = vmfp();
  if (ar->skipFrame()) {
    ar = getPrevVMStateSkipFrame(ar);
    if (!ar) return empty_array();
  }
  while (ar->func()->name()->isame(s_call_user_func.get())
         || ar->func()->name()->isame(s_call_user_func_array.get())) {
    ar = getPrevVMState(ar);
    if (ar == nullptr) {
      return empty_array();
    }
  }

  Offset pc = 0;
  ar = getPrevVMState(ar, &pc);
  while (ar != nullptr) {
    if (!ar->func()->name()->isame(s_call_user_func.get())
        && !ar->func()->name()->isame(s_call_user_func_array.get())) {
      auto const unit = ar->func()->unit();
      int lineNumber;
      if ((lineNumber = unit->getLineNumber(pc)) != -1) {
        auto const cls = ar->func()->cls();
        if (cls != nullptr && !ar->func()->isClosureBody()) {
          return make_map_array(
            s_class, const_cast<StringData*>(cls->name()),
            s_file, const_cast<StringData*>(unit->filepath()),
            s_function, const_cast<StringData*>(ar->func()->name()),
            s_line, lineNumber
          );
        } else {
          return make_map_array(
            s_file, const_cast<StringData*>(unit->filepath()),
            s_function, const_cast<StringData*>(ar->func()->name()),
            s_line, lineNumber
          );
        }
      }
    }
    ar = getPrevVMState(ar, &pc);
  }
  return empty_array();
}

ActRec* ExecutionContext::getFrameAtDepth(int frame) {
  VMRegAnchor _;
  auto fp = vmfp();
  if (UNLIKELY(!fp)) return nullptr;
  auto pc = fp->func()->unit()->offsetOf(vmpc());
  while (frame > 0) {
    fp = getPrevVMState(fp, &pc);
    if (UNLIKELY(!fp)) return nullptr;
    if (UNLIKELY(fp->skipFrame())) continue;
    --frame;
  }
  while (fp->skipFrame()) {
    fp = getPrevVMState(fp, &pc);
    if (UNLIKELY(!fp)) return nullptr;
  }
  if (UNLIKELY(fp->localsDecRefd())) return nullptr;
  auto const curOp = fp->func()->unit()->getOp(pc);
  if (UNLIKELY(curOp == Op::RetC || curOp == Op::RetV ||
               curOp == Op::CreateCont || curOp == Op::Await)) {
    return nullptr;
  }
  assert(!fp->magicDispatch());
  return fp;
}

VarEnv* ExecutionContext::getOrCreateVarEnv(int frame) {
  auto const fp = getFrameAtDepth(frame);
  if (!fp || !(fp->func()->attrs() & AttrMayUseVV)) {
    raise_error("Could not create variable environment");
  }
  if (!fp->hasVarEnv()) {
    fp->setVarEnv(VarEnv::createLocal(fp));
  }
  return fp->getVarEnv();
}

void ExecutionContext::setVar(StringData* name, const TypedValue* v) {
  VMRegAnchor _;
  ActRec *fp = vmfp();
  if (!fp) return;
  if (fp->skipFrame()) fp = getPrevVMStateSkipFrame(fp);
  if (fp) fp->getVarEnv()->set(name, v);
}

void ExecutionContext::bindVar(StringData* name, TypedValue* v) {
  VMRegAnchor _;
  ActRec *fp = vmfp();
  if (!fp) return;
  if (fp->skipFrame()) fp = getPrevVMStateSkipFrame(fp);
  if (fp) fp->getVarEnv()->bind(name, v);
}

Array ExecutionContext::getLocalDefinedVariables(int frame) {
  VMRegAnchor _;
  ActRec *fp = vmfp();
  for (; frame > 0; --frame) {
    if (!fp) break;
    fp = getPrevVMState(fp);
  }
  return getDefinedVariables(fp);
}

bool ExecutionContext::setHeaderCallback(const Variant& callback) {
  if (cellAsVariant(g_context->m_headerCallback).toBoolean()) {
    // return false if a callback has already been set.
    return false;
  }
  cellAsVariant(g_context->m_headerCallback) = callback;
  return true;
}

TypedValue ExecutionContext::invokeUnit(const Unit* unit) {
  checkHHConfig(unit);

  auto const func = unit->getMain(nullptr);
  return invokeFunc(func, init_null_variant, nullptr, nullptr,
                    m_globalVarEnv, nullptr, InvokePseudoMain,
                    !unit->useStrictTypes());
}

void ExecutionContext::syncGdbState() {
  if (RuntimeOption::EvalJit && !RuntimeOption::EvalJitNoGdb) {
    Debug::DebugInfo::Get()->debugSync();
  }
}

void ExecutionContext::pushVMState(Cell* savedSP) {
  if (UNLIKELY(!vmfp())) {
    // first entry
    assert(m_nestedVMs.size() == 0);
    return;
  }

  TRACE(3, "savedVM: %p %p %p %p\n", vmpc(), vmfp(), vmFirstAR(), savedSP);
  auto& savedVM = m_nestedVMs.alloc_back();
  savedVM.pc = vmpc();
  savedVM.fp = vmfp();
  savedVM.firstAR = vmFirstAR();
  savedVM.sp = savedSP;
  savedVM.mInstrState = vmMInstrState();
  savedVM.jitCalledFrame = vmJitCalledFrame();
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

  assert(m_nestedVMs.size() >= 1);

  VMState &savedVM = m_nestedVMs.back();
  vmpc() = savedVM.pc;
  vmfp() = savedVM.fp;
  vmFirstAR() = savedVM.firstAR;
  vmStack().top() = savedVM.sp;
  vmMInstrState() = savedVM.mInstrState;
  vmJitCalledFrame() = savedVM.jitCalledFrame;

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

static void threadLogger(const char* header, const char* msg,
                         const char* ending, void* data) {
  auto* ec = static_cast<ExecutionContext*>(data);
  ec->write(header);
  ec->write(msg);
  ec->write(ending);
  ec->flush();
}

StaticString
  s_php_namespace("<?php namespace "),
  s_hh_namespace("<?hh namespace "),
  s_curly_return(" { return "),
  s_semicolon_curly("; }"),
  s_php_return("<?php return "),
  s_hh_return("<?hh return "),
  s_semicolon(";"),
  s_stdclass("stdclass");

void ExecutionContext::requestInit() {
  assert(SystemLib::s_unit);

  initBlackHole();
  VarEnv::createGlobal();
  vmStack().requestInit();
  ObjectData::resetMaxId();
  ResourceHdr::resetMaxId();
  jit::tc::requestInit();

  if (RuntimeOption::EvalJitEnableRenameFunction) {
    assert(SystemLib::s_anyNonPersistentBuiltins);
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
    if (SystemLib::s_nativeFuncUnit) SystemLib::s_nativeFuncUnit->merge();
    if (SystemLib::s_nativeClassUnit) SystemLib::s_nativeClassUnit->merge();
  } else {
    // System units are merge only, and everything is persistent.
    assert(SystemLib::s_unit->isEmpty());
    assert(!SystemLib::s_hhas_unit || SystemLib::s_hhas_unit->isEmpty());
    assert(!SystemLib::s_nativeFuncUnit ||
           SystemLib::s_nativeFuncUnit->isEmpty());
    assert(!SystemLib::s_nativeClassUnit ||
           SystemLib::s_nativeClassUnit->isEmpty());
  }

  profileRequestStart();

  HHProf::Request::StartProfiling();

#ifdef DEBUG
  Class* cls = NamedEntity::get(s_stdclass.get())->clsList();
  assert(cls);
  assert(cls == SystemLib::s_stdclassClass);
#endif

  if (Logger::UseRequestLog) Logger::SetThreadHook(&threadLogger, this);

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
  MIterTable::clear();

  if (m_globalVarEnv) {
    req::destroy_raw(m_globalVarEnv);
    m_globalVarEnv = nullptr;
  }

  if (!m_lastError.isNull()) {
    clearLastError();
  }

  m_deferredErrors = Array::CreateVec();

  if (Logger::UseRequestLog) Logger::SetThreadHook(nullptr, nullptr);
}

/*
 * Shared implementation for invokeFunc{,Few}().
 *
 * The `doCheckStack' and `doInitArgs' callbacks should return truthy in order
 * to short-circuit the rest of invokeFuncImpl() and return early, else they
 * should return falsey.
 *
 * The `doInitArgs' and `doEnterVM' callbacks take an ActRec* argument
 * corresponding to the reentry frame.
 */
template<class FStackCheck, class FInitArgs, class FEnterVM>
ALWAYS_INLINE
TypedValue ExecutionContext::invokeFuncImpl(const Func* f,
                                            ObjectData* thiz, Class* cls,
                                            uint32_t argc, StringData* invName,
                                            bool useWeakTypes,
                                            FStackCheck doStackCheck,
                                            FInitArgs doInitArgs,
                                            FEnterVM doEnterVM) {
  assert(f);
  // If `f' is a regular function, `thiz' and `cls' must be null.
  assert(IMPLIES(!f->preClass(), f->isPseudoMain() || (!thiz && !cls)));
  // If `f' is a method, either `thiz' or `cls' must be non-null.
  assert(IMPLIES(f->preClass(), thiz || cls));
  // If `f' is a static method, thiz must be null.
  assert(IMPLIES(f->isStaticInPrologue(), !thiz));
  // invName should only be non-null if we are calling __call or __callStatic.
  assert(IMPLIES(invName, f->name()->isame(s___call.get()) ||
                          f->name()->isame(s___callStatic.get())));

  VMRegAnchor _;
  auto const reentrySP = vmStack().top();

  if (thiz != nullptr) thiz->incRefCount();

  TypedValue retval;
  if (doStackCheck(retval)) return retval;

  ActRec* ar = vmStack().allocA();
  ar->setReturnVMExit();
  ar->m_func = f;
  if (thiz) {
    ar->setThis(thiz);
  } else if (cls) {
    ar->setClass(cls);
  } else {
    ar->trashThis();
  }
  ar->initNumArgs(argc);

  if (UNLIKELY(invName != nullptr)) {
    ar->setMagicDispatch(invName);
  } else {
    ar->trashVarEnv();
  }

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

  if (doInitArgs(ar, retval)) return retval;

  if (useWeakTypes) {
    ar->setUseWeakTypes();
  } else {
    setTypesFlag(vmfp(), ar);
  }

  {
    pushVMState(reentrySP);
    SCOPE_EXIT {
      assert_flog(
        vmStack().top() == reentrySP,
        "vmsp() mismatch around reentry: before @ {}, after @ {}",
        reentrySP, vmStack().top()
      );
      popVMState();
    };

    doEnterVM(ar);

    // `retptr' might point somewhere that is affected by {push,pop}VMState(),
    // so don't write to it until after we pop the nested VM state.
    tvCopy(*vmStack().topTV(), retval);
    vmStack().discard();
  }
  return retval;
}

/**
 * Enter VM by calling action(), which invokes a function or resumes
 * an async function. The 'ar' argument points to an ActRec of the
 * invoked/resumed function.
 */
template<class Action>
static inline void enterVMCustomHandler(ActRec* ar, Action action) {
  assert(ar);
  assert(!ar->sfp());
  assert(isReturnHelper(reinterpret_cast<void*>(ar->m_savedRip)));
  assert(ar->m_soff == 0);

  auto ec = &*g_context;
  DEBUG_ONLY int faultDepth = ec->m_faults.size();
  SCOPE_EXIT { assert(ec->m_faults.size() == faultDepth); };

  vmFirstAR() = ar;
  vmJitCalledFrame() = nullptr;

  action();

  while (vmpc()) {
    exception_handler(enterVMAtCurPC);
  }
}

template<class Action>
static inline void enterVM(ActRec* ar, Action action) {
  enterVMCustomHandler(ar, [&] { exception_handler(action); });
}

TypedValue ExecutionContext::invokeFunc(const Func* f,
                                        const Variant& args_,
                                        ObjectData* thiz /* = NULL */,
                                        Class* cls /* = NULL */,
                                        VarEnv* varEnv /* = NULL */,
                                        StringData* invName /* = NULL */,
                                        InvokeFlags flags /* = InvokeNormal */,
                                        bool useWeakTypes /* = false */) {
  const auto& args = *args_.asCell();
  assert(isContainerOrNull(args));

  auto const argc = cellIsNull(&args) ? 0 : getContainerSize(args);
  // If we are inheriting a variable environment, then `args' must be empty.
  assert(IMPLIES(varEnv, argc == 0));

  auto const doCheckStack = [&](TypedValue& retval) {
    // We must do a stack overflow check for leaf functions on re-entry,
    // because we won't have checked that the stack is deep enough for a
    // leaf function /after/ re-entry, and the prologue for the leaf
    // function will not make a check.
    if (f->attrs() & AttrPhpLeafFn ||
        !(f->numParams() + kNumActRecCells <= kStackCheckReenterPadding)) {
      // Check both the native stack and VM stack for overflow.
      checkStack(vmStack(), f,
        kNumActRecCells /* numParams is included in f->maxStackCells */);
    } else {
      // invokeFunc() must always check the native stack for overflow no
      // matter what.
      checkNativeStack();
    }

    // Handle includes of pseudomains.
    if (flags & InvokePseudoMain) {
      assert(f->isPseudoMain());
      assert(cellIsNull(&args) || !getContainerSize(args));

      auto toMerge = f->unit();
      toMerge->merge();
      if (toMerge->isMergeOnly()) {
        retval = *toMerge->getMainReturn();
        return true;
      }
    }
    return false;
  };

  auto const doInitArgs = [&] (ActRec* ar, TypedValue& retval) {
    if (!varEnv) {
      auto const& prepArgs = cellIsNull(&args)
        ? make_tv<KindOfArray>(staticEmptyArray())
        : args;
      auto prepResult = prepareArrayArgs(ar, prepArgs, vmStack(), 0,
                                         flags & InvokeCuf, &retval);
      if (UNLIKELY(!prepResult)) {
        assert(KindOfNull == retval.m_type);
        return true;
      }
    }
    return false;
  };

  auto const doEnterVM = [&] (ActRec* ar) {
    enterVM(ar, [&] {
      enterVMAtFunc(
        ar,
        varEnv ? StackArgsState::Untrimmed : StackArgsState::Trimmed,
        varEnv
      );
    });
  };

  return invokeFuncImpl(f, thiz, cls, argc, invName, useWeakTypes,
                        doCheckStack, doInitArgs, doEnterVM);
}

TypedValue ExecutionContext::invokeFuncFew(const Func* f,
                                           void* thisOrCls,
                                           StringData* invName,
                                           int argc,
                                           const TypedValue* argv,
                                           bool useWeakTypes /* = false */) {
  auto const doCheckStack = [&](TypedValue&) {
    // See comments in invokeFunc().
    if (f->attrs() & AttrPhpLeafFn ||
        !(argc + kNumActRecCells <= kStackCheckReenterPadding)) {
      checkStack(vmStack(), f, argc + kNumActRecCells);
    } else {
      checkNativeStack();
    }
    return false;
  };

  auto const doInitArgs = [&](ActRec* /*ar*/, TypedValue&) {
    for (ssize_t i = 0; i < argc; ++i) {
      const TypedValue *from = &argv[i];
      TypedValue *to = vmStack().allocTV();
      if (LIKELY(from->m_type != KindOfRef || !f->byRef(i))) {
        cellDup(*tvToCell(from), *to);
      } else {
        refDup(*from, *to);
      }
    }
    return false;
  };

  auto const doEnterVM = [&] (ActRec* ar) {
    enterVM(ar, [&] { enterVMAtFunc(ar, StackArgsState::Untrimmed, nullptr); });
  };

  return invokeFuncImpl(f,
                        ActRec::decodeThis(thisOrCls),
                        ActRec::decodeClass(thisOrCls),
                        argc, invName, useWeakTypes,
                        doCheckStack, doInitArgs, doEnterVM);
}

static void prepareAsyncFuncEntry(ActRec* enterFnAr, Resumable* resumable) {
  assert(enterFnAr);
  assert(enterFnAr->func()->isAsync());
  assert(enterFnAr->resumed());
  assert(resumable);

  vmfp() = enterFnAr;
  vmpc() = vmfp()->func()->unit()->at(resumable->resumeOffset());
  assert(vmfp()->func()->contains(vmpc()));
  EventHook::FunctionResumeAwait(enterFnAr);
}

void ExecutionContext::resumeAsyncFunc(Resumable* resumable,
                                       ObjectData* freeObj,
                                       const Cell awaitResult) {
  assert(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assert(tl_regState == VMRegState::CLEAN); };

  auto fp = resumable->actRec();
  // We don't need to check for space for the ActRec (unlike generally
  // in normal re-entry), because the ActRec isn't on the stack.
  checkStack(vmStack(), fp->func(), 0);

  Cell* savedSP = vmStack().top();
  cellDup(awaitResult, *vmStack().allocC());

  // decref after awaitResult is on the stack
  decRefObj(freeObj);

  pushVMState(savedSP);
  SCOPE_EXIT { popVMState(); };

  enterVM(fp, [&] {
    prepareAsyncFuncEntry(fp, resumable);

    const bool useJit = RID().getJit();
    if (LIKELY(useJit && resumable->resumeAddr())) {
      Stats::inc(Stats::VMEnter);
      jit::enterTCAfterPrologue(resumable->resumeAddr());
    } else {
      enterVMAtCurPC();
    }
  });
}

void ExecutionContext::resumeAsyncFuncThrow(Resumable* resumable,
                                            ObjectData* freeObj,
                                            ObjectData* exception) {
  assert(exception);
  assert(exception->instanceof(SystemLib::s_ThrowableClass));
  assert(tl_regState == VMRegState::CLEAN);
  SCOPE_EXIT { assert(tl_regState == VMRegState::CLEAN); };

  auto fp = resumable->actRec();
  checkStack(vmStack(), fp->func(), 0);

  // decref after we hold reference to the exception
  Object e(exception);
  decRefObj(freeObj);

  pushVMState(vmStack().top());
  SCOPE_EXIT { popVMState(); };

  enterVMCustomHandler(fp, [&] {
    prepareAsyncFuncEntry(fp, resumable);

    unwindPhp(exception);
  });
}

ActRec* ExecutionContext::getPrevVMState(const ActRec* fp,
                                         Offset* prevPc /* = NULL */,
                                         TypedValue** prevSp /* = NULL */,
                                         bool* fromVMEntry /* = NULL */) {
  if (fp == nullptr) {
    return nullptr;
  }
  ActRec* prevFp = fp->sfp();
  if (LIKELY(prevFp != nullptr)) {
    if (prevSp) {
      if (UNLIKELY(fp->resumed())) {
        assert(fp->func()->isGenerator());
        *prevSp = (TypedValue*)prevFp - prevFp->func()->numSlotsInFrame();
      } else {
        *prevSp = (TypedValue*)(fp + 1);
      }
    }
    if (prevPc) *prevPc = prevFp->func()->base() + fp->m_soff;
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
  assert(prevFp);
  assert(prevFp->func()->unit());
  if (prevSp) *prevSp = vmstate.sp;
  if (prevPc) {
    *prevPc = prevFp->func()->unit()->offsetOf(vmstate.pc);
  }
  if (fromVMEntry) *fromVMEntry = true;
  return prevFp;
}

/*
  Instantiate hoistable classes and functions.
  If there is any more work left to do, setup a
  new frame ready to execute the pseudomain.

  return true iff the pseudomain needs to be executed.
*/
bool ExecutionContext::evalUnit(Unit* unit, PC& pc, int funcType) {
  vmpc() = pc;
  unit->merge();
  if (unit->isMergeOnly()) {
    Stats::inc(Stats::PseudoMain_Skipped);
    *vmStack().allocTV() = *unit->getMainReturn();
    return false;
  }
  Stats::inc(Stats::PseudoMain_Executed);

  ActRec* ar = vmStack().allocA();
  auto const cls = vmfp()->func()->cls();
  auto const func = unit->getMain(cls);
  assert(!func->isCPPBuiltin());
  ar->m_func = func;
  if (cls) {
    ar->setThisOrClass(vmfp()->getThisOrClass());
    if (ar->hasThis()) ar->getThis()->incRefCount();
  } else {
    ar->trashThis();
  }
  ar->initNumArgs(0);
  assert(vmfp());
  ar->setReturn(vmfp(), pc, jit::tc::ustubs().retHelper);
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

const Variant& ExecutionContext::getEvaledArg(const StringData* val,
                                              const String& namespacedName,
                                              const Unit* funcUnit) {
  auto key = StrNR(val);

  if (m_evaledArgs.get()) {
    const Variant& arg = m_evaledArgs.get()->get(key);
    if (&arg != &uninit_variant) return arg;
  }

  String code;
  int pos = namespacedName.rfind('\\');
  if (pos != -1) {
    auto ns = namespacedName.substr(0, pos);
    code = (funcUnit->isHHFile() ? s_hh_namespace : s_php_namespace) +
      ns + s_curly_return + key + s_semicolon_curly;
  } else {
    code = (funcUnit->isHHFile() ? s_hh_return : s_php_return) +
      key + s_semicolon;
  }
  Unit* unit = compileEvalString(code.get());
  assert(unit != nullptr);
  // Default arg values are not currently allowed to depend on class context.
  auto v = Variant::attach(
    g_context->invokeFunc(unit->getMain(nullptr),
                          init_null_variant, nullptr, nullptr, nullptr, nullptr,
                          InvokePseudoMain)
  );
  Variant &lv = m_evaledArgs.lvalAt(key, AccessFlags::Key);
  lv = v;
  return lv;
}

void ExecutionContext::recordLastError(const Exception &e, int errnum) {
  m_lastError = String(e.getMessage());
  m_lastErrorNum = errnum;
  m_lastErrorPath = String::attach(getContainingFileName());
  m_lastErrorLine = getLine();
  if (auto const ee = dynamic_cast<const ExtendedException *>(&e)) {
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
  assert(handle->isUncounted());
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
  assert(apcExtension::UseUncounted || m_apcHandles.size() == 0);
  if (m_apcHandles.size() > 0) {
    std::vector<APCHandle*> handles;
    handles.swap(m_apcHandles);
    Treadmill::enqueue(
      FreedAPCHandle(std::move(handles), m_apcMemSize)
    );
    APCStats::getAPCStats().addPendingDelete(m_apcMemSize);
  }
}

void ExecutionContext::destructObjects() {
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
    while (!m_liveBCObjs.empty()) {
      ObjectData* obj = *m_liveBCObjs.begin();
      obj->destructForExit(); // Let the instance remove the node.
    }
    m_liveBCObjs.clear();
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
      evalFilename
    );
  }
  return acc->second;
}

StrNR ExecutionContext::createFunction(const String& args,
                                       const String& code) {
  if (UNLIKELY(RuntimeOption::EvalAuthoritativeMode)) {
    // Whole program optimizations need to assume they can see all the
    // code.
    raise_error("You can't use create_function in RepoAuthoritative mode; "
                "use a closure instead");
  }

  VMRegAnchor _;
  auto const ar = GetCallerFrame();
  // It doesn't matter if there's a user function named __lambda_func; we only
  // use this name during parsing, and then change it to an impossible name
  // with a NUL byte before we merge it into the request's func map.  This also
  // has the bonus feature that the value of __FUNCTION__ inside the created
  // function will match Zend. (Note: Zend will actually fatal if there's a
  // user function named __lambda_func when you call create_function. Huzzah!)
  static StringData* oldName = makeStaticString("__lambda_func");
  std::ostringstream codeStr;
  codeStr << (ar->unit()->isHHFile() ? "<?hh" : "<?php")
          << " function " << oldName->data()
          << "(" << args.data() << ") {"
          << code.data() << "}\n";
  std::string evalCode = codeStr.str();
  Unit* unit = compile_string(evalCode.data(), evalCode.size());
  // Move the function to a different name.
  std::ostringstream newNameStr;
  newNameStr << '\0' << "lambda_" << ++m_lambdaCounter;
  StringData* newName = makeStaticString(newNameStr.str());
  unit->renameFunc(oldName, newName);
  m_createdFuncs.push_back(unit);
  unit->merge();

  // At the end of the request we clear the m_createdFunc map, JIT'ing the unit
  // would be a waste of time and TC space.
  unit->setInterpretOnly();

  // Technically we shouldn't have to eval the unit right now (it'll execute
  // the pseudo-main, which should be empty) and could get away with just
  // mergeFuncs. However, Zend does it this way, as proven by the fact that you
  // can inject code into the evaled unit's pseudo-main:
  //
  //   create_function('', '} echo "hi"; if (0) {');
  //
  // We have to eval now to emulate this behavior.
  tvDecRefGen(
      invokeFunc(unit->getMain(nullptr), init_null_variant,
                 nullptr, nullptr, nullptr, nullptr,
                 InvokePseudoMain)
  );

  // __lambda_func will be the only hoistable function.
  // Any functions or closures defined in it will not be hoistable.
  Func* lambda = unit->firstHoistable();
  return lambda->nameStr();
}

ExecutionContext::EvaluationResult
ExecutionContext::evalPHPDebugger(StringData* code, int frame) {
  // The code has "<?php" prepended already
  auto unit = compile_string(code->data(), code->size());
  if (unit == nullptr) {
    raise_error("Syntax error");
    return {true, init_null_variant, "Syntax error"};
  }

  return evalPHPDebugger(unit, frame);
}

ExecutionContext::EvaluationResult
ExecutionContext::evalPHPDebugger(Unit* unit, int frame) {
  always_assert(!RuntimeOption::RepoAuthoritative);

  // Do not JIT this unit, we are using it exactly once.
  unit->setInterpretOnly();

  VMRegAnchor _;

  auto fp = vmfp();
  if (fp) {
    if (fp->skipFrame()) fp = getPrevVMStateSkipFrame(fp);
    for (; frame > 0; --frame) {
      auto prevFp = getPrevVMStateSkipFrame(fp);
      if (!prevFp) {
        // To be safe in case we failed to get prevFp. This would mean we've
        // been asked to eval in a frame which is beyond the top of the stack.
        // This suggests the debugger client has made an error.
        break;
      }
      fp = prevFp;
    }
    if (!fp->hasVarEnv()) {
      fp->setVarEnv(VarEnv::createLocal(fp));
    }
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
  // generally be the return address from a call from that frame's function. If
  // we can't find the target frame (because it lies deeper in the stack), then
  // just use the target frame's func's entry point.
  auto const findSuitablePC = [this](const ActRec* target){
    if (auto fp = vmfp()) {
      if (fp == target) return vmpc();
      while (true) {
        auto prevFp = getPrevVMState(fp);
        if (!prevFp) break;
        if (prevFp == target) return prevFp->func()->getEntry() + fp->m_soff;
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

    // Invoke the given PHP, possibly specialized to match the type of the
    // current function on the stack, optionally passing a this pointer or
    // class used to execute the current function.
    return {false, Variant::attach(
        invokeFunc(unit->getMain(functionClass), init_null_variant,
                   this_, frameClass, fp ? fp->m_varEnv : nullptr, nullptr,
                   InvokePseudoMain)
    ), ""};
  } catch (FatalErrorException &e) {
    errorString << s_fatal.data();
    errorString << " : ";
    errorString << e.getMessage().c_str();
    errorString << "\n";
    stack = ExtendedLogger::StringOfStackTrace(e.getBacktrace());
  } catch (ExitException &e) {
    errorString << s_exit.data();
    errorString << " : ";
    errorString << ExitException::ExitCode;
  } catch (Eval::DebuggerException &e) {
  } catch (Exception &e) {
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
      errorString << e->invokeToString().data();
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
  static Unit* s_debuggerDummy = compile_string("<?php?>", 7);
  // Ensure that the VM stack is completely empty (vmfp() should be null)
  // and that we're not in a nested VM (reentrancy)
  assert(vmfp() == nullptr);
  assert(m_nestedVMs.size() == 0);
  assert(m_nesting == 0);
  assert(vmStack().count() == 0);
  ActRec* ar = vmStack().allocA();
  ar->m_func = s_debuggerDummy->getMain(nullptr);
  ar->initNumArgs(0);
  ar->trashThis();
  ar->setReturnVMExit();
  vmfp() = ar;
  vmpc() = s_debuggerDummy->entry();
  vmFirstAR() = ar;
  vmfp()->setVarEnv(m_globalVarEnv);
  m_globalVarEnv->enterFP(nullptr, vmfp());
}

void ExecutionContext::exitDebuggerDummyEnv() {
  assert(m_globalVarEnv);
  // Ensure that vmfp() is valid
  assert(vmfp() != nullptr);
  // Ensure that vmfp() points to the only frame on the call stack.
  // In other words, make sure there are no VM frames directly below
  // this one and that we are not in a nested VM (reentrancy)
  assert(!vmfp()->sfp());
  assert(m_nestedVMs.size() == 0);
  assert(m_nesting == 0);
  // Teardown the frame we erected by enterDebuggerDummyEnv()
  const Func* func = vmfp()->m_func;
  try {
    vmfp()->setLocalsDecRefd();
    frame_free_locals_no_hook(vmfp());
  } catch (...) {}
  vmStack().ndiscard(func->numSlotsInFrame());
  vmStack().discardAR();
  // After tearing down this frame, the VM stack should be completely empty
  assert(vmStack().count() == 0);
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
