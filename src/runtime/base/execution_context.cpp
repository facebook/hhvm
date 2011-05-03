/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/execution_context.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/externals.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/memory/sweepable.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/debug/backtrace.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/base/taint/taint_helper.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/ext/ext_string.h>
#include <util/logger.h>
#include <util/process.h>
#include <util/text_color.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ExecutionContext, g_context);

ExecutionContext::ExecutionContext()
  : m_transport(NULL),
    m_maxMemory(RuntimeOption::RequestMemoryMaxBytes),
    m_maxTime(RuntimeOption::RequestTimeoutSeconds),
    m_cwd(Process::CurrentWorkingDirectory),
    m_out(NULL), m_implicitFlush(false), m_protectedLevel(0),
    m_stdout(NULL), m_stdoutData(NULL),
    m_errorState(ExecutionContext::NoError),
    m_errorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel),
    m_lastErrorNum(0), m_logErrors(false), m_throwAllErrors(false),
    m_vhost(NULL) {
  MemoryManager::TheMemoryManager()->getStats().maxBytes = m_maxMemory;
  m_include_paths = Array::Create();
  for (unsigned int i = 0; i < RuntimeOption::IncludeSearchPaths.size(); ++i) {
    m_include_paths.append(String(RuntimeOption::IncludeSearchPaths[i]));
  }
}

ExecutionContext::~ExecutionContext() {
  obFlushAll();
  for (list<OutputBuffer*>::const_iterator iter = m_buffers.begin();
       iter != m_buffers.end(); ++iter) {
    delete *iter;
  }
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

void ExecutionContext::fiberInit(FiberLocal *src, FiberReferenceMap &refMap) {
  ExecutionContext *ec = dynamic_cast<ExecutionContext*>(src);
  ASSERT(ec);

  m_transport = ec->m_transport;
  if (m_transport) {
    m_transport->incFiberCount();
  }
  m_maxMemory = ec->m_maxMemory;
  m_maxTime = ec->m_maxTime;
  m_cwd = ec->m_cwd.fiberCopy();

  for (unsigned int i = 0; i < ec->m_userErrorHandlers.size(); i++) {
    pair<Variant, int> &handler = ec->m_userErrorHandlers[i];
    m_userErrorHandlers.push_back
      (pair<Variant, int>(handler.first.fiberMarshal(refMap), handler.second));
  }
  for (unsigned int i = 0; i < ec->m_userExceptionHandlers.size(); i++) {
    m_userExceptionHandlers.push_back
      (ec->m_userExceptionHandlers[i].fiberMarshal(refMap));
  }

  m_timezone = ec->m_timezone.fiberCopy();
  m_timezoneDefault = ec->m_timezoneDefault.fiberCopy();
  m_argSeparatorOutput = ec->m_argSeparatorOutput.fiberCopy();

  m_include_paths = ec->m_include_paths.fiberMarshal(refMap);
}

void ExecutionContext::fiberExit(FiberLocal *src, FiberReferenceMap &refMap) {
  ExecutionContext *ec = dynamic_cast<ExecutionContext*>(src);
  ASSERT(ec);

  if (m_transport) {
    m_transport->decFiberCount();
  }

  Array shutdowns = ec->m_shutdowns.fiberUnmarshal(refMap);
  for (int i = 0; i < ShutdownTypeCount; i++) {
    Variant newfuncs = shutdowns[i];
    if (!newfuncs.isNull()) {
      Variant funcs = m_shutdowns[i];
      if (funcs.isNull()) {
        m_shutdowns.set(i, newfuncs);
      } else {
        Array arr = funcs.toArray();
        arr.merge(newfuncs.toArray());
        m_shutdowns.set(i, arr);
      }
    }
  }

  // isNull() is really a sub-optimal way of telling "who's changed".
  if (m_timezone.isNull()) {
    m_timezone = ec->m_timezone.fiberCopy();
  }
  if (m_timezoneDefault.isNull()) {
    m_timezoneDefault = ec->m_timezoneDefault.fiberCopy();
  }
  if (m_argSeparatorOutput.isNull()) {
    m_argSeparatorOutput = ec->m_argSeparatorOutput.fiberCopy();
  }
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
  } else if (m_transport && m_transport->sendDefaultContentType()) {
    mimetype = m_transport->getDefaultContentType();
  }
  return mimetype;
}

void ExecutionContext::setContentType(CStrRef mimetype, CStrRef charset) {
  if (m_transport) {
    String contentType = mimetype;
    contentType += "; ";
    contentType += "charset=";
    contentType += charset;
    m_transport->addHeader("Content-Type", contentType);
    m_transport->setDefaultContentType(false);
  }
}

void ExecutionContext::setRequestMemoryMaxBytes(int64 max) {
  m_maxMemory = max;
  MemoryManager::TheMemoryManager()->getStats().maxBytes = m_maxMemory;
}

///////////////////////////////////////////////////////////////////////////////
// write()

void ExecutionContext::write(CStrRef s) {
#ifdef TAINTED
  if (!getTransport() && !m_out) {
    // We are running a PHP script and we are about to echo to stdout
    taint_warn_if_tainted(s, TAINT_BIT_HTML);
  } else if (getTransport() && m_buffers.size() == 2) {
    // We are responding to a request and we are about to echo to stdout
    taint_warn_if_tainted(s, TAINT_BIT_HTML);
  }
#endif

  write(s.data(), s.size());
}

void ExecutionContext::setStdout(PFUNC_STDOUT func, void *data) {
  m_stdout = func;
  m_stdoutData = data;
}

static void safe_stdout(const  void  *ptr,  size_t  size) {
  if (write(fileno(stdout), ptr, size) < 0) {
    throw FatalErrorException("unable to write to stdout");
  }
}

void ExecutionContext::writeStdout(const char *s, int len) {
  if (m_stdout == NULL) {
    if (Util::s_stdout_color) {
      safe_stdout(Util::s_stdout_color, strlen(Util::s_stdout_color));
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

void ExecutionContext::obStart(CVarRef handler /* = null */) {
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
    m_buffers.back()->oss.reset();
  }
}

bool ExecutionContext::obFlush() {
  ASSERT(m_protectedLevel >= 0);
  if ((int)m_buffers.size() > m_protectedLevel) {
    list<OutputBuffer*>::const_iterator iter = m_buffers.end();
    OutputBuffer *last = *(--iter);
    const int flag = PHP_OUTPUT_HANDLER_START | PHP_OUTPUT_HANDLER_END;
    if (iter != m_buffers.begin()) {
      OutputBuffer *prev = *(--iter);
      if (last->handler.isNull()) {
        prev->oss.absorb(last->oss);
      } else {
        try {
          Variant tout =
            f_call_user_func_array(last->handler,
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
          f_call_user_func_array(last->handler,
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

void ExecutionContext::obFlushAll() {
  while (obFlush()) { obEnd();}
}

bool ExecutionContext::obEnd() {
  ASSERT(m_protectedLevel >= 0);
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
  ASSERT((int)m_buffers.size() >= m_protectedLevel);
  return m_buffers.size() - m_protectedLevel;
}

Array ExecutionContext::obGetStatus(bool full) {
  Array ret = Array::Create();
  list<OutputBuffer*>::const_iterator iter = m_buffers.begin();
  ++iter; // skip over the fake outermost buffer
  int level = 0;
  for (; iter != m_buffers.end(); ++iter, ++level) {
    Array status;
    status.set("level", level);
    if (level < m_protectedLevel) {
      status.set("type", 1);
      status.set("name", "default output handler");
    } else {
      status.set("type", 0);
      status.set("name", (*iter)->handler);
    }

    if (full) {
      ret.append(status);
    } else {
      ret = status;
    }
  }
  return ret;
}

void ExecutionContext::obSetImplicitFlush(bool on) {
  m_implicitFlush = on;
}

Array ExecutionContext::obGetHandlers() {
  Array ret;
  for (list<OutputBuffer*>::const_iterator iter = m_buffers.begin();
       iter != m_buffers.end(); ++iter) {
    ret.append((*iter)->handler);
  }
  return ret;
}

void ExecutionContext::flush() {
  if (m_buffers.empty()) {
    fflush(stdout);
  } else if (RuntimeOption::EnableEarlyFlush && m_protectedLevel &&
             (m_transport == NULL ||
              (m_transport->getHTTPVersion() == "1.1" &&
               m_transport->getMethod() != Transport::HEAD))) {
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

void ExecutionContext::resetCurrentBuffer() {
  if (m_buffers.empty()) {
    m_out = NULL;
  } else {
    m_out = &m_buffers.back()->oss;
  }
}

///////////////////////////////////////////////////////////////////////////////
// program executions

void ExecutionContext::registerShutdownFunction(CVarRef function,
                                                Array arguments,
                                                ShutdownType type) {
  Array callback = CREATE_MAP2("name", function, "args", arguments);
  Variant &funcs = m_shutdowns.lvalAt(type);
  funcs.append(callback);
}

void ExecutionContext::registerTickFunction(CVarRef function,
                                            Array arguments) {
  Array callback = CREATE_MAP2("name", function, "args", arguments);
  m_ticks.append(callback);
  throw NotImplementedException(__func__);
}

void ExecutionContext::unregisterTickFunction(CVarRef function) {
  //m_ticks.remove(function);
  throw NotImplementedException(__func__);
}

Variant ExecutionContext::pushUserErrorHandler(CVarRef function,
                                               int error_types) {
  Variant ret;
  if (!m_userErrorHandlers.empty()) {
    ret = m_userErrorHandlers.back().first;
  }
  m_userErrorHandlers.push_back(std::pair<Variant,int>(function, error_types));
  return ret;
}

Variant ExecutionContext::pushUserExceptionHandler(CVarRef function) {
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
  ASSERT(handler);
  if (m_requestEventHandlerSet.find(handler) ==
      m_requestEventHandlerSet.end()) {
    m_requestEventHandlerSet.insert(handler);
    m_requestEventHandlers.push_back(handler);
  } else {
    ASSERT(false);
  }
}

static bool requestEventHandlerPriorityComp(RequestEventHandler *a,
                                            RequestEventHandler *b) {
  return a->priority() <= b->priority();
}

void ExecutionContext::onRequestShutdown() {
  // Sort handlers by priority so that lower priority values get shutdown
  // first
  sort(m_requestEventHandlers.begin(), m_requestEventHandlers.end(),
       requestEventHandlerPriorityComp);
  for (unsigned int i = 0; i < m_requestEventHandlers.size(); i++) {
    RequestEventHandler *handler = m_requestEventHandlers[i];
    ASSERT(handler->getInited());
    if (handler->getInited()) {
      handler->requestShutdown();
      handler->setInited(false);
    }
  }
  m_requestEventHandlers.clear();
  m_requestEventHandlerSet.clear();
}

void ExecutionContext::executeFunctions(CArrRef funcs) {
  for (ArrayIter iter(funcs); iter; ++iter) {
    Array callback = iter.second();
    f_call_user_func_array(callback["name"], callback["args"]);
  }
}

void ExecutionContext::onShutdownPreSend() {
  if (!m_shutdowns.isNull() && m_shutdowns.exists(ShutDown)) {
    executeFunctions(m_shutdowns[ShutDown]);
    m_shutdowns.remove(ShutDown);
  }
  obFlushAll(); // in case obStart was called without obFlush
}

void ExecutionContext::onShutdownPostSend() {
  ServerStats::SetThreadMode(ServerStats::PostProcessing);
  try {
    try {
      ServerStatsHelper ssh("psp");
      if (!m_shutdowns.isNull()) {
        if (m_shutdowns.exists(PostSend)) {
          executeFunctions(m_shutdowns[PostSend]);
          m_shutdowns.remove(PostSend);
        }
        if (m_shutdowns.exists(CleanUp)) {
          executeFunctions(m_shutdowns[CleanUp]);
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
  ServerStats::SetThreadMode(ServerStats::Idling);
}

void ExecutionContext::onTick() {
  executeFunctions(m_ticks);
}

///////////////////////////////////////////////////////////////////////////////
// error handling

bool ExecutionContext::errorNeedsHandling(int errnum,
                                          bool callUserHandler,
                                          ErrorThrowMode mode) {
  if (m_throwAllErrors) throw errnum;
  if (mode != NeverThrow || (getErrorReportingLevel() & errnum) != 0 ||
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
  ErrorStateHelper(ExecutionContext *context, int state) {
    m_context = context;
    m_originalState = m_context->getErrorState();
    m_context->setErrorState(state);
  }
  ~ErrorStateHelper() {
    m_context->setErrorState(m_originalState);
  }
private:
  ExecutionContext *m_context;
  int m_originalState;
};

void ExecutionContext::handleError(const std::string &msg,
                                   int errnum,
                                   bool callUserHandler,
                                   ErrorThrowMode mode,
                                   const std::string &prefix) {
  int newErrorState = ErrorRaised;
  switch (getErrorState()) {
  case ErrorRaised:
  case ErrorRaisedByUserHandler:
    return;
  case ExecutingUserHandler:
    newErrorState = ErrorRaisedByUserHandler;
    break;
  default:
    break;
  }
  ErrorStateHelper esh(this, newErrorState);
  ExtendedException ee(msg);
  recordLastError(ee, errnum);
  bool handled = false;
  if (callUserHandler) {
    handled = callUserErrorHandler(ee, errnum, false);
  }
  if (mode == AlwaysThrow || (mode == ThrowIfUnhandled && !handled)) {
    try {
      if (!Eval::Debugger::InterruptException(String(msg))) return;
    } catch (const Eval::DebuggerClientExitException &e) {}
    throw FatalErrorException(msg.c_str());
  }
  if (!handled &&
      (RuntimeOption::NoSilencer ||
       (getErrorReportingLevel() & errnum) != 0)) {
    try {
      if (!Eval::Debugger::InterruptException(String(msg))) return;
    } catch (const Eval::DebuggerClientExitException &e) {}

    const char *file = NULL;
    int line = 0;
    if (RuntimeOption::InjectedStackTrace) {
      ArrayPtr bt = ee.getBackTrace();
      if (!bt->empty()) {
        Array top = bt->rvalAt(0).toArray();
        if (top.exists("file")) file = top.rvalAt("file").toString();
        if (top.exists("line")) line = top.rvalAt("line");
      }
    }

    Logger::Log(true, prefix.c_str(), ee, file, line);
  }
}

bool ExecutionContext::callUserErrorHandler(const Exception &e, int errnum,
                                            bool swallowExceptions) {
  switch (getErrorState()) {
  case ExecutingUserHandler:
  case ErrorRaisedByUserHandler:
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
      ArrayPtr arr = ee->getBackTrace();
      if (arr) {
        backtrace = *arr;
        Array top = backtrace.rvalAt(0);
        if (!top.isNull()) {
          errfile = top.rvalAt("file");
          errline = top.rvalAt("line").toInt64();
        }
      }
    }
    if (backtrace.isNull()) {
      backtrace = stackTraceToBackTrace(e.getStackTrace());
    }
    try {
      ErrorStateHelper esh(this, ExecutingUserHandler);
      if (!same(f_call_user_func_array
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

void ExecutionContext::recordLastError(const Exception &e,
                                       int errnum /* = 0 */) {
  m_lastError = String(e.getMessage());
  m_lastErrorNum = errnum;
}

bool ExecutionContext::onFatalError(const Exception &e) {
  recordLastError(e);
  const char *file = NULL;
  int line = 0;
  if (RuntimeOption::InjectedStackTrace) {
    const ExtendedException *ee = dynamic_cast<const ExtendedException *>(&e);
    if (ee) {
      ArrayPtr bt = ee->getBackTrace();
      if (!bt->empty()) {
        Array top = bt->rvalAt(0).toArray();
        if (top.exists("file")) file = top.rvalAt("file").toString();
        if (top.exists("line")) line = top.rvalAt("line");
      }
    }
  }
  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(true, "HipHop Fatal error: ", e, file, line);
  }
  bool handled = false;
  if (RuntimeOption::CallUserHandlerOnFatals) {
    int errnum = ErrorConstants::FATAL_ERROR;
    handled = callUserErrorHandler(e, errnum, true);
  }
  if (!handled && !RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log(true, "HipHop Fatal error: ", e, file, line);
  }
  return handled;
}

bool ExecutionContext::onUnhandledException(Object e) {
  String err = e.toString();
  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("HipHop Fatal error: Uncaught exception %s", err.data());
  }

  if (e.instanceof("Exception")) {
    // user thrown exception
    if (!m_userExceptionHandlers.empty()) {
      f_call_user_func_array(m_userExceptionHandlers.back(),CREATE_VECTOR1(e));
      return true;// no matter what handler returns!
    }
  } else {
    ASSERT(false);
  }
  m_lastError = err;

  if (!RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("HipHop Fatal error: Uncaught exception: %s", err.data());
  }
  return false;
}

void ExecutionContext::setLogErrors(bool on) {
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
      Logger::SetNewOutput(NULL);
    }
  }
}

void ExecutionContext::setErrorLog(CStrRef filename) {
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

void ExecutionContext::debuggerInfo(InfoVec &info) {
  if (m_maxMemory <= 0) {
    Add(info, "Max Memory", "(unlimited)");
  } else {
    Add(info, "Max Memory", FormatSize(m_maxMemory));
  }
  Add(info, "Max Time", FormatTime(m_maxTime * 1000));
}

///////////////////////////////////////////////////////////////////////////////

void ExecutionContext::setenv(CStrRef name, CStrRef value) {
  m_envs.set(name, value);
}

String ExecutionContext::getenv(CStrRef name) const {
  if (m_envs.exists(name)) {
    return m_envs[name];
  }
  char *value = ::getenv(name.data());
  if (value) {
    return String(value, CopyString);
  }
  return String();
}

///////////////////////////////////////////////////////////////////////////////

void ExecutionContext::setIncludePath(CStrRef path) {
  m_include_paths = f_explode(":", path);
}

String ExecutionContext::getIncludePath() const {
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
  ASSERT(type && *type);
  ASSERT(name);
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
  ASSERT(type && *type);
  ASSERT(name);
  ResourceMap &resources = m_objects[type];
  ResourceMap::const_iterator iter = resources.find(name);
  if (iter == resources.end()) {
    return NULL;
  }
  return iter->second;
}

void PersistentObjectStore::remove(const char *type, const char *name) {
  ASSERT(type && *type);
  ASSERT(name);
  ResourceMap &resources = m_objects[type];
  ResourceMap::iterator iter = resources.find(name);
  if (iter != resources.end()) {
    removeObject(iter->second);
    resources.erase(iter);
  }
}

const ResourceMap &PersistentObjectStore::getMap(const char *type) {
  ASSERT(type && *type);
  return m_objects[type];
}

///////////////////////////////////////////////////////////////////////////////
// silencer


Silencer::Silencer() : m_active(false) {
}

Silencer::~Silencer() {
  disable();
}

void Silencer::enable() {
  m_errorReportingValue = g_context->getErrorReportingLevel();
  g_context->setErrorReportingLevel(0);
  m_active = true;
}

void Silencer::disable() {
  if (m_active) {
    if (g_context->getErrorReportingLevel() == 0)
      g_context->setErrorReportingLevel(m_errorReportingValue);
    m_active = false;
  }
}

Variant Silencer::disable(CVarRef v) {
  disable();
  return v;
}

///////////////////////////////////////////////////////////////////////////////
// Fast Method Call

static bool g_methodIndexUseSys ;
void MethodIndexHMap::initialize(bool useSystem) {
  g_methodIndexUseSys = useSystem;
}

MethodIndex MethodIndexHMap::methodIndexExists(CStrRef methodName) {
  unsigned size =
    g_methodIndexUseSys ? g_methodIndexHMapSizeSys : g_methodIndexHMapSize;
  if (!size) return MethodIndex::fail();
  const MethodIndexHMap *map =
    g_methodIndexUseSys ? g_methodIndexHMapSys : g_methodIndexHMap;
  unsigned hash = (unsigned)(methodName->hash() % size);
  while (map[hash].name && strcasecmp(map[hash].name, methodName.data())!=0) {
    hash = hash ? hash - 1 : size - 1;
  }
  if (!map[hash].name) return MethodIndex::fail();
  return map[hash].methodIndex;
}

const char * methodIndexLookupReverse(MethodIndex methodIndex) {
  const unsigned *callIndex = g_methodIndexUseSys
    ? g_methodIndexReverseCallIndexSys : g_methodIndexReverseCallIndex;
  const char **map = g_methodIndexUseSys
    ?  g_methodIndexReverseIndexSys : g_methodIndexReverseIndex;
  unsigned callIndexOffset = callIndex[methodIndex.m_callIndex - 1];
  return map[methodIndex.m_overloadIndex + callIndexOffset - 1];
}

///////////////////////////////////////////////////////////////////////////////
}
