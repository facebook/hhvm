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
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/base/system-profiler.h"
#include "hphp/runtime/ext/std/ext_std_output.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/system/constants.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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

  // We want this to run on every request, instead of just once per thread
  std::string memory_limit = "memory_limit";
  auto hasSystemDefault = IniSetting::ResetSystemDefault(memory_limit);
  if (!hasSystemDefault) {
    auto max_mem = std::to_string(RuntimeOption::RequestMemoryMaxBytes);
    IniSetting::SetUser(memory_limit, max_mem, IniSetting::FollyDynamic());
  }

  // This one is hot so we don't want to go through the ini_set() machinery to
  // change it in error_reporting(). Because of that, we have to set it back to
  // the default on every request.
  hasSystemDefault = IniSetting::ResetSystemDefault("error_reporting");
  if (!hasSystemDefault) {
    RID().setErrorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel);
  }

  VariableSerializer::serializationSizeLimit =
    RuntimeOption::SerializationSizeLimit;
}

template<> void ThreadLocalNoCheck<ExecutionContext>::destroy() {
  if (!isNull()) {
    getNoCheck()->sweep();
    setNull();
  }
}


void ExecutionContext::cleanup() {
  manageAPCHandle();

  // Discard all units that were created via create_function().
  for (auto& v : m_createdFuncs) delete v;

  always_assert(m_activeSims.empty());
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
  } else {
    writeStdout(s, len);
  }
  if (m_implicitFlush) flush();
}

///////////////////////////////////////////////////////////////////////////////
// output buffers

void ExecutionContext::obProtect(bool on) {
  m_protectedLevel = on ? m_buffers.size() : 0;
}

void ExecutionContext::obStart(const Variant& handler /* = null */,
                               int chunk_size /* = 0 */) {
  if (m_insideOBHandler) {
    raise_error("ob_start(): Cannot use output buffering "
                "in output buffering display handlers");
  }
  m_buffers.emplace_back(Variant(handler), chunk_size);
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

bool ExecutionContext::obFlush() {
  assert(m_protectedLevel >= 0);

  if ((int)m_buffers.size() <= m_protectedLevel) {
    return false;
  }

  auto iter = m_buffers.end();
  OutputBuffer& last = *(--iter);

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
      writeStdout(str.data(), str.size());
      throw;
    }
  }

  writeStdout(str.data(), str.size());
  return true;
}

void ExecutionContext::obFlushAll() {
  while (obFlush()) { obEnd();}
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
  if (m_buffers.empty()) {
    fflush(stdout);
  } else if (RuntimeOption::EnableEarlyFlush && m_protectedLevel &&
             (m_transport == nullptr ||
              (m_transport->getHTTPVersion() == "1.1" &&
               m_transport->getMethod() != Transport::Method::HEAD))) {
    StringBuffer &oss = m_buffers.front().oss;
    if (!oss.empty()) {
      if (m_transport) {
        m_transport->sendRaw((void*)oss.data(), oss.size(), 200, false, true);
      } else {
        writeStdout(oss.data(), oss.size());
        fflush(stdout);
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
      newFuncs.appendWithRef(iter.secondRef());
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

void ExecutionContext::popUserExceptionHandler() {
  if (!m_userExceptionHandlers.empty()) {
    m_userExceptionHandlers.pop_back();
  }
}

void ExecutionContext::registerRequestEventHandler(
  RequestEventHandler *handler) {
  assert(handler && handler->getInited());
  m_requestEventHandlers.push_back(handler);
}

static bool requestEventHandlerPriorityComp(RequestEventHandler *a,
                                            RequestEventHandler *b) {
  return a->priority() < b->priority();
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

class ErrorStateHelper {
public:
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
  s_file("file"),
  s_line("line"),
  s_php_errormsg("php_errormsg");

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

  ErrorStateHelper esh(this, newErrorState);
  auto const ee = skipFrame ?
    ExtendedException(ExtendedException::SkipFrame{}, msg) :
    ExtendedException(msg);
  bool handled = false;
  if (callUserHandler) {
    handled = callUserErrorHandler(ee, errnum, false);
  }

  if (!handled) {
    recordLastError(ee, errnum);
  }

  if (g_system_profiler) {
    g_system_profiler->errorCallBack(ee, errnum, msg);
  }

  if (mode == ErrorThrowMode::Always ||
      (mode == ErrorThrowMode::IfUnhandled && !handled)) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerErrorHook(ee, errnum, msg));
    bool isRecoverable =
      errnum == static_cast<int>(ErrorMode::RECOVERABLE_ERROR);
    auto exn = FatalErrorException(msg, ee.getBacktrace(), isRecoverable);
    exn.setSilent(!errorNeedsLogging(errnum));
    throw exn;
  }
  if (!handled) {
    VMRegAnchor _;
    auto fp = vmfp();

    if (RID().hasTrackErrors() && fp) {
      // Set $php_errormsg in the parent scope
      Variant varFrom(ee.getMessage());
      const auto tvFrom(varFrom.asTypedValue());
      if (fp->func()->isBuiltin()) {
        fp = getPrevVMState(fp);
      }
      assert(fp);
      auto id = fp->func()->lookupVarId(s_php_errormsg.get());
      if (id != kInvalidId) {
        auto tvTo = frame_local(fp, id);
        if (tvTo->m_type == KindOfRef) {
          tvTo = tvTo->m_data.pref->tv();
        }
        tvDup(*tvFrom, *tvTo);
      } else if ((fp->func()->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
        fp->getVarEnv()->set(s_php_errormsg.get(), tvFrom);
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
      auto const context = ar ? getDefinedVariables(ar) : empty_array();
      if (!same(vm_call_user_func
                (m_userErrorHandlers.back().first,
                 make_packed_array(errnum, String(e.getMessage()),
                     fileAndLine.first, fileAndLine.second, context,
                     backtrace)),
                false)) {
        return true;
      }
    } catch (...) {
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

  if (e.instanceof(SystemLib::s_ExceptionClass)) {
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

///////////////////////////////////////////////////////////////////////////////

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
  char *value = ::getenv(name.data());
  if (value) {
    return String(value, CopyString);
  }
  if (RuntimeOption::EnvVariables.find(name.c_str()) != RuntimeOption::EnvVariables.end()) {
    return String(RuntimeOption::EnvVariables[name.c_str()].data(), CopyString);
  }
  return String();
}

//////////////////////////////////////////////////////////////////////

}
