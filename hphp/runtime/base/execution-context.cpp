/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#define __STDC_LIMIT_MACROS

#include "hphp/runtime/base/execution-context.h"

#include <stdint.h>

#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/text_color.h"
#include "hphp/runtime/base/array_init.h"
#include "hphp/runtime/base/array_iterator.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/server/server_stats.h"
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

IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(ExecutionContext, g_context);

int64_t VMExecutionContext::s_threadIdxCounter = 0;
Mutex VMExecutionContext::s_threadIdxLock;
hphp_hash_map<pid_t, int64_t> VMExecutionContext::s_threadIdxMap;

const StaticString BaseExecutionContext::s_amp("&");

BaseExecutionContext::BaseExecutionContext() :
    m_fp(nullptr), m_pc(nullptr),
    m_transport(nullptr),
    m_cwd(Process::CurrentWorkingDirectory),
    m_out(nullptr), m_implicitFlush(false), m_protectedLevel(0),
    m_stdout(nullptr), m_stdoutData(nullptr),
    m_errorState(ExecutionContext::ErrorState::NoError),
    m_errorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel),
    m_lastErrorNum(0), m_logErrors(false), m_throwAllErrors(false),
    m_vhost(nullptr) {

  setRequestMemoryMaxBytes(RuntimeOption::RequestMemoryMaxBytes);
  m_include_paths = Array::Create();
  for (unsigned int i = 0; i < RuntimeOption::IncludeSearchPaths.size(); ++i) {
    m_include_paths.append(String(RuntimeOption::IncludeSearchPaths[i]));
  }
}

VMExecutionContext::VMExecutionContext() :
    m_preg_backtrace_limit(RuntimeOption::PregBacktraceLimit),
    m_preg_recursion_limit(RuntimeOption::PregRecursionLimit),
    m_lambdaCounter(0), m_nesting(0),
    m_breakPointFilter(nullptr), m_lastLocFilter(nullptr),
    m_dbgNoBreak(false), m_coverPrevLine(-1), m_coverPrevUnit(nullptr),
    m_executingSetprofileCallback(false) {

  // Make sure any fields accessed from the TC are within a byte of
  // ExecutionContext's beginning.
  static_assert(offsetof(ExecutionContext, m_stack) <= 0xff,
                "m_stack offset too large");
  static_assert(offsetof(ExecutionContext, m_fp) <= 0xff,
                "m_fp offset too large");
  static_assert(offsetof(ExecutionContext, m_pc) <= 0xff,
                "m_pc offset too large");
  static_assert(offsetof(ExecutionContext, m_currentThreadIdx) <= 0xff,
                "m_currentThreadIdx offset too large");

  {
    Lock lock(s_threadIdxLock);
    pid_t tid = Process::GetThreadPid();
    if (!mapGet(s_threadIdxMap, tid, &m_currentThreadIdx)) {
      m_currentThreadIdx = s_threadIdxCounter++;
      s_threadIdxMap[tid] = m_currentThreadIdx;
    }
  }
}

BaseExecutionContext::~BaseExecutionContext() {
  obFlushAll();
  for (std::list<OutputBuffer*>::const_iterator iter = m_buffers.begin();
       iter != m_buffers.end(); ++iter) {
    delete *iter;
  }
}

VMExecutionContext::~VMExecutionContext() {
  // Discard any ConstInfo objects that were created to support reflection.
  for (ConstInfoMap::const_iterator it = m_constInfo.begin();
       it != m_constInfo.end(); ++it) {
    delete it->second;
  }

  // Discard all units that were created via create_function().
  for (EvaledUnitsVec::iterator it = m_createdFuncs.begin();
       it != m_createdFuncs.end(); ++it) {
    delete *it;
  }

  delete m_breakPointFilter;
  delete m_lastLocFilter;
}

void BaseExecutionContext::backupSession() {
  m_shutdownsBackup = m_shutdowns;
  m_userErrorHandlersBackup = m_userErrorHandlers;
  m_userExceptionHandlersBackup = m_userExceptionHandlers;
}

void BaseExecutionContext::restoreSession() {
  m_shutdowns = m_shutdownsBackup;
  m_userErrorHandlers = m_userErrorHandlersBackup;
  m_userExceptionHandlers = m_userExceptionHandlersBackup;
}

///////////////////////////////////////////////////////////////////////////////
// system functions

String BaseExecutionContext::getMimeType() const {
  String mimetype;
  if (m_transport) {
    mimetype = m_transport->getMimeType();
  }

  if (strncasecmp(mimetype.data(), "text/", 5) == 0) {
    int pos = mimetype.find(';');
    if (pos != String::npos) {
      mimetype = mimetype.substr(0, pos);
    }
  } else if (m_transport && m_transport->sendDefaultContentType()) {
    mimetype = m_transport->getDefaultContentType();
  }
  return mimetype;
}

std::string BaseExecutionContext::getRequestUrl(size_t szLimit) {
  Transport* t = getTransport();
  std::string ret = t ? t->getUrl() : "";
  if (szLimit != std::string::npos) {
    ret = ret.substr(0, szLimit);
  }
  return ret;
}

void BaseExecutionContext::setContentType(CStrRef mimetype, CStrRef charset) {
  if (m_transport) {
    String contentType = mimetype;
    contentType += "; ";
    contentType += "charset=";
    contentType += charset;
    m_transport->addHeader("Content-Type", contentType.c_str());
    m_transport->setDefaultContentType(false);
  }
}

void BaseExecutionContext::setRequestMemoryMaxBytes(int64_t max) {
  if (max <= 0) {
    max = INT64_MAX;
  }
  m_maxMemory = max;
  MemoryManager::TheMemoryManager()->getStats().maxBytes = m_maxMemory;
}

///////////////////////////////////////////////////////////////////////////////
// write()

void BaseExecutionContext::write(CStrRef s) {
  write(s.data(), s.size());
}

void BaseExecutionContext::setStdout(PFUNC_STDOUT func, void *data) {
  m_stdout = func;
  m_stdoutData = data;
}

static void safe_stdout(const  void  *ptr,  size_t  size) {
  write(fileno(stdout), ptr, size);
}

void BaseExecutionContext::writeStdout(const char *s, int len) {
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

void BaseExecutionContext::write(const char *s, int len) {
  if (m_out) {
    m_out->append(s, len);
  } else {
    writeStdout(s, len);
  }
  if (m_implicitFlush) flush();
}

///////////////////////////////////////////////////////////////////////////////
// output buffers

void BaseExecutionContext::obProtect(bool on) {
  m_protectedLevel = on ? m_buffers.size() : 0;
}

void BaseExecutionContext::obStart(CVarRef handler /* = null */) {
  OutputBuffer *ob = new OutputBuffer();
  ob->handler = handler;
  m_buffers.push_back(ob);
  resetCurrentBuffer();
}

String BaseExecutionContext::obCopyContents() {
  if (!m_buffers.empty()) {
    StringBuffer &oss = m_buffers.back()->oss;
    if (!oss.empty()) {
      return oss.copy();
    }
  }
  return "";
}

String BaseExecutionContext::obDetachContents() {
  if (!m_buffers.empty()) {
    StringBuffer &oss = m_buffers.back()->oss;
    if (!oss.empty()) {
      return oss.detach();
    }
  }
  return "";
}

int BaseExecutionContext::obGetContentLength() {
  if (m_buffers.empty()) {
    return 0;
  }
  return m_buffers.back()->oss.size();
}

void BaseExecutionContext::obClean() {
  if (!m_buffers.empty()) {
    m_buffers.back()->oss.reset();
  }
}

bool BaseExecutionContext::obFlush() {
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
          Variant tout =
            vm_call_user_func(last->handler,
                                   CREATE_VECTOR2(last->oss.detach(), flag));
          prev->oss.append(tout.toString());
          last->oss.reset();
        } catch (...) {
          prev->oss.absorb(last->oss);
        }
      }
      return true;
    }

    if (!last->handler.isNull()) {
      try {
        Variant tout =
          vm_call_user_func(last->handler,
                                 CREATE_VECTOR2(last->oss.detach(), flag));
        String sout = tout.toString();
        writeStdout(sout.data(), sout.size());
        last->oss.reset();
        return true;
      } catch (...) {}
    }

    writeStdout(last->oss.data(), last->oss.size());
    last->oss.reset();
    return true;
  }
  return false;
}

void BaseExecutionContext::obFlushAll() {
  while (obFlush()) { obEnd();}
}

bool BaseExecutionContext::obEnd() {
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

void BaseExecutionContext::obEndAll() {
  while (obEnd()) {}
}

int BaseExecutionContext::obGetLevel() {
  assert((int)m_buffers.size() >= m_protectedLevel);
  return m_buffers.size() - m_protectedLevel;
}

const StaticString
  s_level("level"),
  s_type("type"),
  s_name("name"),
  s_args("args"),
  s_default_output_handler("default output handler");

Array BaseExecutionContext::obGetStatus(bool full) {
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

void BaseExecutionContext::obSetImplicitFlush(bool on) {
  m_implicitFlush = on;
}

Array BaseExecutionContext::obGetHandlers() {
  Array ret;
  for (std::list<OutputBuffer*>::const_iterator iter = m_buffers.begin();
       iter != m_buffers.end(); ++iter) {
    ret.append((*iter)->handler);
  }
  return ret;
}

void BaseExecutionContext::flush() {
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
      oss.reset();
    }
  }
}

void BaseExecutionContext::resetCurrentBuffer() {
  if (m_buffers.empty()) {
    m_out = nullptr;
  } else {
    m_out = &m_buffers.back()->oss;
  }
}

///////////////////////////////////////////////////////////////////////////////
// program executions

void BaseExecutionContext::registerShutdownFunction(CVarRef function,
                                                    Array arguments,
                                                    ShutdownType type) {
  Array callback = CREATE_MAP2(s_name, function, s_args, arguments);
  Variant &funcs = m_shutdowns.lvalAt(type);
  funcs.append(callback);
}

Variant BaseExecutionContext::pushUserErrorHandler(CVarRef function,
                                                   int error_types) {
  Variant ret;
  if (!m_userErrorHandlers.empty()) {
    ret = m_userErrorHandlers.back().first;
  }
  m_userErrorHandlers.push_back(std::pair<Variant,int>(function, error_types));
  return ret;
}

Variant BaseExecutionContext::pushUserExceptionHandler(CVarRef function) {
  Variant ret;
  if (!m_userExceptionHandlers.empty()) {
    ret = m_userExceptionHandlers.back();
  }
  m_userExceptionHandlers.push_back(function);
  return ret;
}

void BaseExecutionContext::popUserErrorHandler() {
  if (!m_userErrorHandlers.empty()) {
    m_userErrorHandlers.pop_back();
  }
}

void BaseExecutionContext::popUserExceptionHandler() {
  if (!m_userExceptionHandlers.empty()) {
    m_userExceptionHandlers.pop_back();
  }
}

void BaseExecutionContext::registerRequestEventHandler
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

void BaseExecutionContext::onRequestShutdown() {
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

void BaseExecutionContext::executeFunctions(CArrRef funcs) {
  ThreadInfo::s_threadInfo->m_reqInjectionData.resetTimer(
    RuntimeOption::PspTimeoutSeconds);

  for (ArrayIter iter(funcs); iter; ++iter) {
    Array callback = iter.second().toArray();
    vm_call_user_func(callback[s_name], callback[s_args].toArray());
  }
}

void BaseExecutionContext::onShutdownPreSend() {
  if (!m_shutdowns.isNull() && m_shutdowns.exists(ShutDown)) {
    executeFunctions(m_shutdowns[ShutDown].toArray());
    m_shutdowns.remove(ShutDown);
  }
  obFlushAll(); // in case obStart was called without obFlush
}

void BaseExecutionContext::onShutdownPostSend() {
  ServerStats::SetThreadMode(ServerStats::ThreadMode::PostProcessing);
  try {
    try {
      ServerStatsHelper ssh("psp", ServerStatsHelper::TRACK_HWINST);
      if (!m_shutdowns.isNull()) {
        if (m_shutdowns.exists(PostSend)) {
          executeFunctions(m_shutdowns[PostSend].toArray());
          m_shutdowns.remove(PostSend);
        }
        if (m_shutdowns.exists(CleanUp)) {
          executeFunctions(m_shutdowns[CleanUp].toArray());
          m_shutdowns.remove(CleanUp);
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
  ServerStats::SetThreadMode(ServerStats::ThreadMode::Idling);
}

///////////////////////////////////////////////////////////////////////////////
// error handling

bool BaseExecutionContext::errorNeedsHandling(int errnum,
                                              bool callUserHandler,
                                              ErrorThrowMode mode) {
  if (m_throwAllErrors) throw errnum;
  if (mode != ErrorThrowMode::Never ||
      (getErrorReportingLevel() & errnum) != 0 ||
      RuntimeOption::NoSilencer) {
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

class ErrorStateHelper {
public:
  ErrorStateHelper(BaseExecutionContext *context,
                   ExecutionContext::ErrorState state) {
    m_context = context;
    m_originalState = m_context->getErrorState();
    m_context->setErrorState(state);
  }
  ~ErrorStateHelper() {
    m_context->setErrorState(m_originalState);
  }
private:
  BaseExecutionContext *m_context;
  ExecutionContext::ErrorState m_originalState;
};

const StaticString
  s_file("file"),
  s_line("line");

void BaseExecutionContext::handleError(const std::string &msg,
                                       int errnum,
                                       bool callUserHandler,
                                       ErrorThrowMode mode,
                                       const std::string &prefix,
                                       bool skipFrame /* = false */) {
  SYNC_VM_REGS_SCOPED();

  ErrorState newErrorState = ErrorState::ErrorRaised;
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
  ExtendedException ee = skipFrame ?
    ExtendedException(ExtendedException::SkipFrame::skipFrame, msg) :
    ExtendedException(msg);
  Array bt = ee.getBackTrace();

  recordLastError(ee, errnum);
  bool handled = false;
  if (callUserHandler) {
    handled = callUserErrorHandler(ee, errnum, false);
  }
  if (mode == ErrorThrowMode::Always ||
      (mode == ErrorThrowMode::IfUnhandled && !handled)) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerErrorHook(msg));
    throw FatalErrorException(msg, bt);
  }
  if (!handled &&
      (RuntimeOption::NoSilencer ||
       (getErrorReportingLevel() & errnum) != 0)) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerErrorHook(msg));
    String file = empty_string;
    int line = 0;
    if (RuntimeOption::InjectedStackTrace) {
      if (!bt.empty()) {
        Array top = bt.rvalAt(0).toArray();
        if (top.exists(s_file)) file = top.rvalAt(s_file).toString();
        if (top.exists(s_line)) line = top.rvalAt(s_line).toInt64();
      }
    }

    Logger::Log(Logger::LogError, prefix.c_str(), ee, file.c_str(), line);
  }
}

bool BaseExecutionContext::callUserErrorHandler(const Exception &e, int errnum,
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
    const ExtendedException *ee = dynamic_cast<const ExtendedException*>(&e);
    if (ee) {
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
      if (!same(vm_call_user_func
                (m_userErrorHandlers.back().first,
                 CREATE_VECTOR6(errnum, String(e.getMessage()), errfile,
                                errline, "", backtrace)),
                false)) {
        return true;
      }
    } catch (...) {
      if (!swallowExceptions) throw;
    }
  }
  return false;
}

void BaseExecutionContext::recordLastError(const Exception &e,
                                           int errnum /* = 0 */) {
  m_lastError = String(e.getMessage());
  m_lastErrorNum = errnum;
}

bool BaseExecutionContext::onFatalError(const Exception &e) {
  recordLastError(e);
  String file = empty_string;
  int line = 0;
  if (RuntimeOption::InjectedStackTrace) {
    const ExtendedException *ee = dynamic_cast<const ExtendedException *>(&e);
    if (ee) {
      Array bt = ee->getBackTrace();
      if (!bt.empty()) {
        Array top = bt.rvalAt(0).toArray();
        if (top.exists(s_file)) file = top.rvalAt(s_file).toString();
        if (top.exists(s_line)) line = top.rvalAt(s_line).toInt32();
      }
    }
  }
  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(Logger::LogError, "HipHop Fatal error: ", e,
                file.c_str(), line);
  }
  bool handled = false;
  if (RuntimeOption::CallUserHandlerOnFatals) {
    int errnum = static_cast<int>(ErrorConstants::ErrorModes::FATAL_ERROR);
    handled = callUserErrorHandler(e, errnum, true);
  }
  if (!handled && !RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(Logger::LogError, "HipHop Fatal error: ", e,
                file.c_str(), line);
  }
  return handled;
}

bool BaseExecutionContext::onUnhandledException(Object e) {
  String err = e.toString();
  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("HipHop Fatal error: Uncaught %s", err.data());
  }

  if (e.instanceof(SystemLib::s_ExceptionClass)) {
    // user thrown exception
    if (!m_userExceptionHandlers.empty()) {
      if (!same(vm_call_user_func
                (m_userExceptionHandlers.back(),
                 CREATE_VECTOR1(e)),
                false)) {
        return true;
      }
    }
  } else {
    assert(false);
  }
  m_lastError = err;

  if (!RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("HipHop Fatal error: Uncaught %s", err.data());
  }
  return false;
}

void BaseExecutionContext::setLogErrors(bool on) {
  if (m_logErrors != on) {
    m_logErrors = on;
    if (m_logErrors) {
      if (!m_errorLog.empty()) {
        FILE *output = fopen(m_errorLog.data(), "a");
        if (output) {
          Logger::SetNewOutput(output);
        }
      }
    } else {
      Logger::SetNewOutput(nullptr);
    }
  }
}

void BaseExecutionContext::setErrorLog(CStrRef filename) {
  m_errorLog = filename;
  if (m_logErrors && !m_errorLog.empty()) {
    FILE *output = fopen(m_errorLog.data(), "a");
    if (output) {
      Logger::SetNewOutput(output);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// IDebuggable

void BaseExecutionContext::debuggerInfo(InfoVec &info) {
  if (m_maxMemory <= 0) {
    Add(info, "Max Memory", "(unlimited)");
  } else {
    Add(info, "Max Memory", FormatSize(m_maxMemory));
  }
  Add(info, "Max Time", FormatTime(ThreadInfo::s_threadInfo.getNoCheck()->
                                   m_reqInjectionData.getTimeout() * 1000));
}

///////////////////////////////////////////////////////////////////////////////

void BaseExecutionContext::setenv(CStrRef name, CStrRef value) {
  m_envs.set(name, value);
}

String BaseExecutionContext::getenv(CStrRef name) const {
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

///////////////////////////////////////////////////////////////////////////////

void BaseExecutionContext::setIncludePath(CStrRef path) {
  m_include_paths = f_explode(":", path);
}

String BaseExecutionContext::getIncludePath() const {
  StringBuffer sb;
  bool first = true;
  for (ArrayIter iter(m_include_paths); iter; ++iter) {
    if (first) {
      first = false;
    } else {
      sb.append(':');
    }
    sb.append(iter.second().toString());
  }
  return sb.detach();
}

///////////////////////////////////////////////////////////////////////////////
// persistent objects

IMPLEMENT_THREAD_LOCAL_NO_CHECK(PersistentObjectStore, g_persistentObjects);

void PersistentObjectStore::removeObject(ResourceData *data) {
  if (data) {
    if (data->decRefCount() == 0) {
      data->release();
    } else {
      SweepableResourceData *sw = dynamic_cast<SweepableResourceData*>(data);
      if (sw) {
        sw->decPersistent();
      }
    }
  }
}

PersistentObjectStore::~PersistentObjectStore() {
  for (ResourceMapMap::const_iterator iter = m_objects.begin();
       iter != m_objects.end(); ++iter) {
    const ResourceMap &resources = iter->second;
    for (ResourceMap::const_iterator iterInner = resources.begin();
         iterInner != resources.end(); ++iterInner) {
      removeObject(iterInner->second);
    }
  }
}

int PersistentObjectStore::size() const {
  int total = 0;
  for (ResourceMapMap::const_iterator iter = m_objects.begin();
       iter != m_objects.end(); ++iter) {
    total += iter->second.size();
  }
  return total;
}

void PersistentObjectStore::set(const char *type, const char *name,
                                ResourceData *obj) {
  assert(type && *type);
  assert(name);
  {
    ResourceMap &resources = m_objects[type];
    ResourceMap::iterator iter = resources.find(name);
    if (iter != resources.end()) {
      if (iter->second == obj) {
        return; // we are setting the same object
      }
      removeObject(iter->second);
      resources.erase(iter);
    }
  }
  if (obj) {
    obj->incRefCount();
    SweepableResourceData *sw = dynamic_cast<SweepableResourceData*>(obj);
    if (sw) {
      sw->incPersistent();
    }
    m_objects[type][name] = obj;
  }
}

ResourceData *PersistentObjectStore::get(const char *type, const char *name) {
  assert(type && *type);
  assert(name);
  ResourceMap &resources = m_objects[type];
  ResourceMap::const_iterator iter = resources.find(name);
  if (iter == resources.end()) {
    return nullptr;
  }
  return iter->second;
}

void PersistentObjectStore::remove(const char *type, const char *name) {
  assert(type && *type);
  assert(name);
  ResourceMap &resources = m_objects[type];
  ResourceMap::iterator iter = resources.find(name);
  if (iter != resources.end()) {
    removeObject(iter->second);
    resources.erase(iter);
  }
}

const ResourceMap &PersistentObjectStore::getMap(const char *type) {
  assert(type && *type);
  return m_objects[type];
}

///////////////////////////////////////////////////////////////////////////////
// silencer


Silencer::Silencer(bool e) : m_active(false) {
  if (e) enable();
}

void Silencer::enable() {
  m_errorReportingValue = g_context->getErrorReportingLevel();
  g_context->setErrorReportingLevel(0);
  m_active = true;
}

void Silencer::disableHelper() {
  if (m_active) {
    if (g_context->getErrorReportingLevel() == 0)
      g_context->setErrorReportingLevel(m_errorReportingValue);
  }
}

Variant Silencer::disable(CVarRef v) {
  disable();
  return v;
}

///////////////////////////////////////////////////////////////////////////////
}
