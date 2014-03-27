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

#include "folly/MapUtil.h"

#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/text-color.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/event-hook.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ExecutionContext, g_context);

ExecutionContext::ExecutionContext()
  : m_fp(nullptr)
  , m_pc(nullptr)
  , m_transport(nullptr)
  , m_cwd(Process::CurrentWorkingDirectory)
  , m_out(nullptr)
  , m_implicitFlush(false)
  , m_protectedLevel(0)
  , m_stdout(nullptr)
  , m_stdoutData(nullptr)
  , m_errorState(ExecutionContext::ErrorState::NoError)
  , m_lastErrorNum(0)
  , m_throwAllErrors(false)
  , m_vhost(nullptr)
  , m_globalVarEnv(nullptr)
  , m_lambdaCounter(0)
  , m_nesting(0)
  , m_breakPointFilter(nullptr)
  , m_lastLocFilter(nullptr)
  , m_dbgNoBreak(false)
  , m_coverPrevLine(-1)
  , m_coverPrevUnit(nullptr)
  , m_lastErrorPath("")
  , m_lastErrorLine(0)
  , m_executingSetprofileCallback(false)
{
  // We want this to run on every request, instead of just once per thread
  auto max_mem = std::to_string(RuntimeOption::RequestMemoryMaxBytes);
  IniSetting::Set("memory_limit", max_mem);

  // This one is hot so we don't want to go through the ini_set() machinery to
  // change it in error_reporting(). Because of that, we have to set it back to
  // the default on every request.
  ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData.
    setErrorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel);

  // Make sure any fields accessed from the TC are within a byte of
  // ExecutionContext's beginning.
  static_assert(offsetof(ExecutionContext, m_stack) <= 0xff,
                "m_stack offset too large");
  static_assert(offsetof(ExecutionContext, m_fp) <= 0xff,
                "m_fp offset too large");
  static_assert(offsetof(ExecutionContext, m_pc) <= 0xff,
                "m_pc offset too large");
}

ExecutionContext::~ExecutionContext() {
  // Discard any ConstInfo objects that were created to support reflection.
  for (ConstInfoMap::const_iterator it = m_constInfo.begin();
       it != m_constInfo.end(); ++it) {
    delete it->second;
  }

  // Discard all units that were created via create_function().
  for (auto& v : m_createdFuncs) delete v;

  delete m_breakPointFilter;
  delete m_lastLocFilter;
  obFlushAll();
  for (auto& b : m_buffers) delete b;
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
    mimetype =
        ThreadInfo::s_threadInfo->m_reqInjectionData.getDefaultMimeType();
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
  } else {
    m_stdout(s, len, m_stdoutData);
  }
}

void ExecutionContext::write(const char *s, int len) {
  if (m_out) {
    m_out->append(s, len);
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

void ExecutionContext::obStart(const Variant& handler /* = null */) {
  OutputBuffer *ob = new OutputBuffer();
  ob->handler = handler;
  m_buffers.push_back(ob);
  resetCurrentBuffer();
}

String ExecutionContext::obCopyContents() {
  if (!m_buffers.empty()) {
    StringBuffer &oss = m_buffers.back()->oss;
    if (!oss.empty()) {
      return oss.copy();
    }
  }
  return "";
}

String ExecutionContext::obDetachContents() {
  if (!m_buffers.empty()) {
    StringBuffer &oss = m_buffers.back()->oss;
    if (!oss.empty()) {
      return oss.detach();
    }
  }
  return "";
}

int ExecutionContext::obGetContentLength() {
  if (m_buffers.empty()) {
    return 0;
  }
  return m_buffers.back()->oss.size();
}

void ExecutionContext::obClean() {
  if (!m_buffers.empty()) {
    m_buffers.back()->oss.clear();
  }
}

bool ExecutionContext::obFlush() {
  assert(m_protectedLevel >= 0);
  if ((int)m_buffers.size() > m_protectedLevel) {
    std::list<OutputBuffer*>::const_iterator iter = m_buffers.end();
    OutputBuffer *last = *(--iter);
    const int flag = PHP_OUTPUT_HANDLER_START | PHP_OUTPUT_HANDLER_END;
    if (iter != m_buffers.begin()) {
      OutputBuffer *prev = *(--iter);
      if (last->handler.isNull()) {
        prev->oss.absorb(last->oss);
      } else {
        try {
          Variant tout = vm_call_user_func(
            last->handler, make_packed_array(last->oss.detach(), flag)
          );
          prev->oss.append(tout.toString());
          last->oss.clear();
        } catch (...) {
          prev->oss.absorb(last->oss);
        }
      }
      return true;
    }

    if (!last->handler.isNull()) {
      try {
        Variant tout = vm_call_user_func(
          last->handler, make_packed_array(last->oss.detach(), flag)
        );
        String sout = tout.toString();
        writeStdout(sout.data(), sout.size());
        last->oss.clear();
        return true;
      } catch (...) {}
    }

    writeStdout(last->oss.data(), last->oss.size());
    last->oss.clear();
    return true;
  }
  return false;
}

void ExecutionContext::obFlushAll() {
  while (obFlush()) { obEnd();}
}

bool ExecutionContext::obEnd() {
  assert(m_protectedLevel >= 0);
  if ((int)m_buffers.size() > m_protectedLevel) {
    delete m_buffers.back();
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
  s_default_output_handler("default output handler");

Array ExecutionContext::obGetStatus(bool full) {
  Array ret = Array::Create();
  std::list<OutputBuffer*>::const_iterator iter = m_buffers.begin();
  ++iter; // skip over the fake outermost buffer
  int level = 0;
  for (; iter != m_buffers.end(); ++iter, ++level) {
    Array status;
    status.set(s_level, level);
    if (level < m_protectedLevel) {
      status.set(s_type, 1);
      status.set(s_name, s_default_output_handler);
    } else {
      status.set(s_type, 0);
      status.set(s_name, (*iter)->handler);
    }

    if (full) {
      ret.append(status);
    } else {
      ret = std::move(status);
    }
  }
  return ret;
}

void ExecutionContext::obSetImplicitFlush(bool on) {
  m_implicitFlush = on;
}

Array ExecutionContext::obGetHandlers() {
  Array ret;
  for (auto& ob : m_buffers) {
    auto& handler = ob->handler;
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
    StringBuffer &oss = m_buffers.front()->oss;
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
    m_out = nullptr;
  } else {
    m_out = &m_buffers.back()->oss;
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

Variant ExecutionContext::popShutdownFunction(ShutdownType type) {
  Variant& funcs = m_shutdowns.lvalAt(type);
  if (!funcs.isArray()) {
    return uninit_null();
  }
  return funcs.toArrRef().pop();
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

void ExecutionContext::registerRequestEventHandler
(RequestEventHandler *handler) {
  assert(handler);
  if (m_requestEventHandlerSet.find(handler) ==
      m_requestEventHandlerSet.end()) {
    m_requestEventHandlerSet.insert(handler);
    m_requestEventHandlers.push_back(handler);
  } else {
    assert(false);
  }
}

static bool requestEventHandlerPriorityComp(RequestEventHandler *a,
                                            RequestEventHandler *b) {
  return a->priority() < b->priority();
}

void ExecutionContext::onRequestShutdown() {
  // Sort handlers by priority so that lower priority values get shutdown
  // first
  sort(m_requestEventHandlers.begin(), m_requestEventHandlers.end(),
       requestEventHandlerPriorityComp);
  for (unsigned int i = 0; i < m_requestEventHandlers.size(); i++) {
    RequestEventHandler *handler = m_requestEventHandlers[i];
    assert(handler->getInited());
    if (handler->getInited()) {
      handler->requestShutdown();
      handler->setInited(false);
    }
  }
  m_requestEventHandlers.clear();
  m_requestEventHandlerSet.clear();
}

void ExecutionContext::executeFunctions(const Array& funcs) {
  ThreadInfo::s_threadInfo->m_reqInjectionData.resetTimer(
    RuntimeOption::PspTimeoutSeconds);

  for (ArrayIter iter(funcs); iter; ++iter) {
    Array callback = iter.second().toArray();
    vm_call_user_func(callback[s_name], callback[s_args].toArray());
  }
}

void ExecutionContext::onShutdownPreSend() {
  // in case obStart was called without obFlush
  SCOPE_EXIT { obFlushAll(); };

  if (!m_shutdowns.isNull() && m_shutdowns.exists(ShutDown)) {
    SCOPE_EXIT { m_shutdowns.remove(ShutDown); };
    executeFunctions(m_shutdowns[ShutDown].toArray());
  }
}

extern void ext_session_request_shutdown();

void ExecutionContext::onShutdownPostSend() {
  ServerStats::SetThreadMode(ServerStats::ThreadMode::PostProcessing);
  try {
    try {
      ServerStatsHelper ssh("psp", ServerStatsHelper::TRACK_HWINST);
      if (!m_shutdowns.isNull()) {
        if (m_shutdowns.exists(PostSend)) {
          SCOPE_EXIT { m_shutdowns.remove(PostSend); };
          executeFunctions(m_shutdowns[PostSend].toArray());
        }
        if (m_shutdowns.exists(CleanUp)) {
          SCOPE_EXIT { m_shutdowns.remove(CleanUp); };
          executeFunctions(m_shutdowns[CleanUp].toArray());
        }
      }
    } catch (const ExitException &e) {
      // do nothing
    } catch (const Exception &e) {
      onFatalError(e);
    } catch (const Object &e) {
      onUnhandledException(e);
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
  if (m_throwAllErrors) {
    throw errnum;
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
  auto level = ThreadInfo::s_threadInfo.getNoCheck()->
    m_reqInjectionData.getErrorReportingLevel();
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
  s_line("line");

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

  ErrorStateHelper esh(this, newErrorState);
  auto const ee = skipFrame ?
    ExtendedException(ExtendedException::SkipFrame::skipFrame, msg) :
    ExtendedException(msg);
  recordLastError(ee, errnum);
  bool handled = false;
  if (callUserHandler) {
    handled = callUserErrorHandler(ee, errnum, false);
  }
  if (mode == ErrorThrowMode::Always ||
      (mode == ErrorThrowMode::IfUnhandled && !handled)) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerErrorHook(msg));
    auto exn = FatalErrorException(msg, ee.getBackTrace());
    exn.setSilent(!errorNeedsLogging(errnum));
    throw exn;
  }
  if (!handled && errorNeedsLogging(errnum)) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerErrorHook(ee.getMessage()));
    String file = empty_string;
    int line = 0;
    if (RuntimeOption::InjectedStackTrace) {
      Array bt = ee.getBackTrace();
      if (!bt.empty()) {
        Array top = bt.rvalAt(0).toArray();
        if (top.exists(s_file)) file = top.rvalAt(s_file).toString();
        if (top.exists(s_line)) line = top.rvalAt(s_line).toInt64();
      }
    }
    Logger::Log(Logger::LogError, prefix.c_str(), ee, file.c_str(), line);
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
    int errline = 0;
    String errfile;
    Array backtrace;
    if (auto const ee = dynamic_cast<const ExtendedException*>(&e)) {
      Array arr = ee->getBackTrace();
      if (!arr.isNull()) {
        backtrace = arr;
        Array top = backtrace.rvalAt(0).toArray();
        if (!top.isNull()) {
          errfile = top.rvalAt(s_file);
          errline = top.rvalAt(s_line).toInt64();
        }
      }
    }
    try {
      ErrorStateHelper esh(this, ErrorState::ExecutingUserHandler);
      Array dummyContext = Array::Create();
      if (!same(vm_call_user_func
                (m_userErrorHandlers.back().first,
                 make_packed_array(errnum, String(e.getMessage()), errfile,
                     errline, dummyContext, backtrace)),
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
  int errnum = static_cast<int>(ErrorConstants::ErrorModes::FATAL_ERROR);
  recordLastError(e, errnum);
  String file = empty_string;
  int line = 0;
  bool silenced = false;
  if (RuntimeOption::InjectedStackTrace) {
    if (auto const ee = dynamic_cast<const ExtendedException *>(&e)) {
      silenced = ee->isSilent();
      Array bt = ee->getBackTrace();
      if (!bt.empty()) {
        Array top = bt.rvalAt(0).toArray();
        if (top.exists(s_file)) file = top.rvalAt(s_file).toString();
        if (top.exists(s_line)) line = top.rvalAt(s_line).toInt32();
      }
    }
  }
  // need to silence even with the AlwaysLogUnhandledExceptions flag set
  if (!silenced && RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(Logger::LogError, "\nFatal error: ", e,
                file.c_str(), line);
  }
  bool handled = false;
  if (RuntimeOption::CallUserHandlerOnFatals) {
    handled = callUserErrorHandler(e, errnum, true);
  }
  if (!handled && !silenced && !RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(Logger::LogError, "\nFatal error: ", e,
                file.c_str(), line);
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
    newInt = INT64_MAX;
  }
  if (newInt == INT64_MAX) {
    info.emplace_back("Max Memory", "(unlimited)");
  } else {
    info.emplace_back("Max Memory", IDebuggable::FormatSize(newInt));
  }
  info.emplace_back("Max Time",
    IDebuggable::FormatTime(ThreadInfo::s_threadInfo.getNoCheck()->
                 m_reqInjectionData.getTimeout() * 1000));
}

///////////////////////////////////////////////////////////////////////////////

void ExecutionContext::setenv(const String& name, const String& value) {
  m_envs.set(name, value);
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
