/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/base/execution_context.h>
#include <cpp/base/type_string.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/comparisons.h>
#include <cpp/base/externals.h>
#include <cpp/base/util/request_local.h>
#include <cpp/base/resource_data.h>
#include <util/logger.h>
#include <util/process.h>
#include <cpp/base/runtime_option.h>
#include <cpp/base/debug/backtrace.h>
#include <cpp/base/memory/sweepable.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static

ThreadLocal<ExecutionContext> g_context;

int RequestEventHandler::priority() const { return 0; }

class RequestData : public RequestEventHandler {
public:
  RequestData() : data(NULL) {}

  struct Data {
    int errorReportingLevel;
    String lastError;
    String timezone;
    String timezoneDefault;
    Array shutdowns;
    Array ticks;
    std::vector<Variant> systemExceptionHandlers;
    std::vector<Variant> userExceptionHandlers;
    String cwd;
  };
  Data *data;

  virtual void requestInit() {
    data = new Data();
    data->errorReportingLevel = 6135; // E_ALL & ~E_NOTICE
    data->cwd = Process::CurrentWorkingDirectory;
  }
  virtual void requestShutdown() {
    delete data;
  }
};
static RequestLocal<RequestData> s_request_data;

String ExecutionContext::getLastError() {
  return s_request_data->data->lastError;
}
int ExecutionContext::getErrorReportingLevel() {
  return s_request_data->data->errorReportingLevel;
}
void ExecutionContext::setErrorReportingLevel(int level) {
  s_request_data->data->errorReportingLevel = level;
}
String ExecutionContext::getTimeZone() {
  return s_request_data->data->timezone;
}
void ExecutionContext::setTimeZone(CStrRef timezone) {
  s_request_data->data->timezone = timezone;
}
String ExecutionContext::getDefaultTimeZone() {
  return s_request_data->data->timezoneDefault;
}
String ExecutionContext::getCwd() {
  return s_request_data->data->cwd;
}
void ExecutionContext::setCwd(CStrRef cwd) {
  s_request_data->data->cwd = cwd;
}

///////////////////////////////////////////////////////////////////////////////

ExecutionContext::ExecutionContext()
  : m_silencer(0), m_null("/dev/null"), m_implicitFlush(false),
    m_protectedLevel(0), m_connStatus(Normal), m_transport(NULL) {
  m_out = &cout;
  m_err = &cerr;
}

ExecutionContext::~ExecutionContext() {
  obFlushAll();
  for (list<OutputBuffer*>::const_iterator iter = m_buffers.begin();
       iter != m_buffers.end(); ++iter) {
    delete *iter;
  }
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

String ExecutionContext::obGetContents() {
  if (m_buffers.empty()) {
    return "";
  }

  // ideally we don't have to make a copy here...
  return String(m_buffers.back()->oss.str());
}

std::string ExecutionContext::getContents() {
  if (m_buffers.empty()) {
    return "";
  }
  return m_buffers.back()->oss.str();
}

int ExecutionContext::obGetContentLength() {
  if (m_buffers.empty()) {
    return 0;
  }
  return m_buffers.back()->oss.str().length();
}

void ExecutionContext::obClean() {
  if (!m_buffers.empty()) {
    m_buffers.back()->oss.str("");
  }
}

bool ExecutionContext::obFlush() {
  ASSERT(m_protectedLevel >= 0);
  if ((int)m_buffers.size() > m_protectedLevel) {
    list<OutputBuffer*>::const_iterator iter = m_buffers.end();
    OutputBuffer *last = *(--iter);
    if (iter != m_buffers.begin()) {
      OutputBuffer *prev = *(--iter);
      if (last->handler.isNull()) {
        prev->oss << last->oss.str();
      } else {
        prev->oss <<
          f_call_user_func_array
          (last->handler,
           CREATE_VECTOR1(String(last->oss.str().c_str(),
                                 AttachLiteral))).toString().data();
      }
      last->oss.str("");
      return true;
    }
    cout << last->oss.str();
    last->oss.str("");
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
    return true;
  }
  return false;
}

void ExecutionContext::obEndAll() {
  while (obEnd()) {}
}

int ExecutionContext::obGetLevel() {
  ASSERT((int)m_buffers.size() >= m_protectedLevel);
  return m_buffers.size() - m_protectedLevel;
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
  if (RuntimeOption::EnableEarlyFlush &&
      (m_transport == NULL || m_transport->getHTTPVersion() == "1.1") &&
      !m_buffers.empty()) {
    std::string content = m_buffers.front()->oss.str();
    if (!content.empty()) {
      m_buffers.front()->oss.str("");
      if (m_transport) {
        m_transport->sendString(content, 200, false, true);
      } else {
        cout << content;
        fflush(stdout);
      }
    }
  }
}

void ExecutionContext::silenceInc() {
  m_silencer++;
  Logger::SetSilenced(isSilenced());
}

void ExecutionContext::silenceDec() {
  ASSERT(m_silencer > 0);
  m_silencer--;
  Logger::SetSilenced(isSilenced());
}

bool ExecutionContext::isSilenced() {
  return !RuntimeOption::NoSilencer && m_silencer > 0;
}

void ExecutionContext::resetCurrentBuffer() {
  if (m_buffers.empty()) {
    m_out = &cout;
  } else {
    m_out = &m_buffers.back()->oss;
  }
}

String ExecutionContext::getMimeType() {
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

///////////////////////////////////////////////////////////////////////////////
// program executions

void ExecutionContext::registerShutdownFunction(CVarRef function,
                                                Array arguments,
                                                ShutdownType type) {
  Array callback = CREATE_MAP2("name", function, "args", arguments);
  Variant &funcs = s_request_data->data->shutdowns.lvalAt(type);
  funcs.append(callback);
}

void ExecutionContext::registerTickFunction(CVarRef function,
                                            Array arguments) {
  Array callback = CREATE_MAP2("name", function, "args", arguments);
  s_request_data->data->ticks.append(callback);
  throw NotImplementedException(__func__);
}

void ExecutionContext::unregisterTickFunction(CVarRef function) {
  //s_request_data->data->ticks.remove(function);
  throw NotImplementedException(__func__);
}

Variant ExecutionContext::pushSystemExceptionHandler(CVarRef function) {
  Variant ret;
  RequestData::Data *data = s_request_data->data;
  if (!data->systemExceptionHandlers.empty()) {
    ret = data->systemExceptionHandlers.back();
  }
  data->systemExceptionHandlers.push_back(function);
  return ret;
}

Variant ExecutionContext::pushUserExceptionHandler(CVarRef function) {
  Variant ret;
  RequestData::Data *data = s_request_data->data;
  if (!data->userExceptionHandlers.empty()) {
    ret = data->userExceptionHandlers.back();
  }
  data->userExceptionHandlers.push_back(function);
  return ret;
}

void ExecutionContext::popSystemExceptionHandler() {
  RequestData::Data *data = s_request_data->data;
  if (!data->systemExceptionHandlers.empty()) {
    data->systemExceptionHandlers.pop_back();
  }
}

void ExecutionContext::popUserExceptionHandler() {
  RequestData::Data *data = s_request_data->data;
  if (!data->userExceptionHandlers.empty()) {
    data->userExceptionHandlers.pop_back();
  }
}

void ExecutionContext::registerRequestEventHandler
(RequestEventHandler *handler) {
  ASSERT(handler);
  m_requestEventHandlers.push_back(handler);
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
}

void ExecutionContext::onShutdownPreSend() {
  Array &funcs = s_request_data->data->shutdowns;
  if (!funcs.isNull() && funcs.exists(ShutDown)) {
    executeFunctions(funcs[ShutDown]);
  }
  obFlushAll(); // in case obStart was called without obFlush
}

void ExecutionContext::onShutdownPostSend() {
  Array &funcs = s_request_data->data->shutdowns;
  if (!funcs.isNull()) {
    if (funcs.exists(PostSend)) {
      executeFunctions(funcs[PostSend]);
    }
    if (funcs.exists(CleanUp)) {
      executeFunctions(funcs[CleanUp]);
    }
  }
}

void ExecutionContext::onTick() {
  executeFunctions(s_request_data->data->ticks);
}

void ExecutionContext::onError(const Exception &e) {
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

  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log("Unhandled error: ", e, errfile.c_str(), errline);
  }

  RequestData::Data *data = s_request_data->data;
  // For errors: if the user handler function returns FALSE then the normal
  // error handler continues. Pass -1LL so that we log all the errors.
  if (!data->systemExceptionHandlers.empty() &&
      !same(f_call_user_func_array
            (data->systemExceptionHandlers.back(),
             CREATE_VECTOR6(-1LL, String(e.getMessage()), errfile, errline, "",
                            backtrace)),
            false)) {
    return;
  }
  data->lastError = String(e.getMessage());

  if (!RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Log("Unhandled error: ", e, errfile.c_str(), errline);
  }
}

void ExecutionContext::onException(Object e) {
  String err = e.toString();
  if (RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("Unhandled exception: %s", err.data());
  }

  RequestData::Data *data = s_request_data->data;
  if (e.instanceof("exception")) {
    // user thrown exception
    if (!data->userExceptionHandlers.empty()) {
      f_call_user_func_array(data->userExceptionHandlers.back(),
                             CREATE_VECTOR1(e));
      return;
    }
  } else {
    ASSERT(false);
  }
  data->lastError = err;

  if (!RuntimeOption::AlwaysLogUnhandledExceptions) {
    Logger::Error("Unhandled exception: %s", err.data());
  }
}

void ExecutionContext::executeFunctions(CArrRef funcs) {
  for (ArrayIter iter(funcs); iter; ++iter) {
    Array callback = iter.second();
    f_call_user_func_array(callback["name"], callback["args"]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// persistent objects

ThreadLocal<PersistentObjectStore> g_persistentObjects;

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

///////////////////////////////////////////////////////////////////////////////
}
