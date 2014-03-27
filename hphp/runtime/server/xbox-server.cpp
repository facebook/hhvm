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

#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/base/libevent-http-client.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
#include "hphp/runtime/server/server-task-event.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using std::string;

XboxTransport::XboxTransport(const String& message, const String& reqInitDoc /* = "" */)
    : m_refCount(0), m_done(false), m_code(0), m_event(nullptr) {
  Timer::GetMonotonicTime(m_queueTime);

  m_message.append(message.data(), message.size());
  m_reqInitDoc.append(reqInitDoc.data(), reqInitDoc.size());
  disableCompression(); // so we don't have to decompress during sendImpl()
}

const char *XboxTransport::getUrl() {
  if (!m_reqInitDoc.empty()) {
    return "xbox_process_call_message";
  }
  return RuntimeOption::XboxProcessMessageFunc.c_str();
}

std::string XboxTransport::getHeader(const char *name) {
  if (!strcasecmp(name, "Host")) return m_host;
  if (!strcasecmp(name, "ReqInitDoc")) return m_reqInitDoc;
  return "";
}

void XboxTransport::sendImpl(const void *data, int size, int code,
                             bool chunked) {
  m_response.append((const char*)data, size);
  if (code) {
    m_code = code;
  }
}

void XboxTransport::onSendEndImpl() {
  Lock lock(this);

  m_done = true;
  if (m_event) {
    m_event->finish();
  }
  notify();
}

String XboxTransport::getResults(int &code, int timeout_ms /* = 0 */) {
  {
    Lock lock(this);
    while (!m_done) {
      if (timeout_ms > 0) {
        long long seconds = timeout_ms / 1000;
        long long nanosecs = (timeout_ms - seconds * 1000) * 1000;
        if (!wait(seconds, nanosecs)) {
          code = -1;
          return "";
        }
      } else {
        wait();
      }
    }
  }

  String response(m_response.c_str(), m_response.size(), CopyString);
  code = m_code;

  return response;
}

///////////////////////////////////////////////////////////////////////////////

static IMPLEMENT_THREAD_LOCAL(std::shared_ptr<XboxServerInfo>,
  s_xbox_server_info);
static IMPLEMENT_THREAD_LOCAL(std::string, s_xbox_prev_req_init_doc);

class XboxRequestHandler: public RPCRequestHandler {
public:
  XboxRequestHandler() : RPCRequestHandler(
    (*s_xbox_server_info)->getTimeoutSeconds().count(), Info) {}
  static bool Info;
};

bool XboxRequestHandler::Info = false;

static IMPLEMENT_THREAD_LOCAL(XboxRequestHandler, s_xbox_request_handler);

///////////////////////////////////////////////////////////////////////////////

struct XboxWorker
  : JobQueueWorker<XboxTransport*,Server*,true,false,JobQueueDropVMStack>
{
  virtual void doJob(XboxTransport *job) {
    try {
      // If this job or the previous job that ran on this thread have
      // a custom initial document, make sure we do a reset
      string reqInitDoc = job->getHeader("ReqInitDoc");
      *s_xbox_prev_req_init_doc = reqInitDoc;

      job->onRequestStart(job->getStartTimer());
      createRequestHandler()->handleRequest(job);
      destroyRequestHandler();
      job->decRefCount();
    } catch (...) {
      Logger::Error("RpcRequestHandler leaked exceptions");
    }
  }
private:
  RequestHandler *createRequestHandler() {
    if (!*s_xbox_server_info) {
      *s_xbox_server_info = std::make_shared<XboxServerInfo>();
    }
    if (RuntimeOption::XboxServerLogInfo) XboxRequestHandler::Info = true;
    s_xbox_request_handler->setServerInfo(*s_xbox_server_info);
    s_xbox_request_handler->setReturnEncodeType(
      RPCRequestHandler::ReturnEncodeType::Serialize);
    return s_xbox_request_handler.get();
  }

  void destroyRequestHandler() {
    if (!s_xbox_request_handler.isNull()) {
      s_xbox_request_handler.destroy();
    }
  }

  virtual void onThreadExit() {
    if (!s_xbox_request_handler.isNull()) {
      s_xbox_request_handler.destroy();
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

static JobQueueDispatcher<XboxWorker> *s_dispatcher;
static Mutex s_dispatchMutex;

void XboxServer::Restart() {
  Stop();

  if (RuntimeOption::XboxServerThreadCount > 0) {
    {
      Lock l(s_dispatchMutex);
      s_dispatcher = new JobQueueDispatcher<XboxWorker>
        (RuntimeOption::XboxServerThreadCount,
         RuntimeOption::ServerThreadRoundRobin,
         RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
         RuntimeOption::ServerThreadDropStack,
         nullptr);
    }
    if (RuntimeOption::XboxServerLogInfo) {
      Logger::Info("xbox server started");
    }
    s_dispatcher->start();
  }
}

void XboxServer::Stop() {
  if (s_dispatcher) {
    s_dispatcher->stop();

    Lock l(s_dispatchMutex);
    delete s_dispatcher;
    s_dispatcher = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_code("code"),
  s_response("response"),
  s_error("error"),
  s_localhost("localhost"),
  s_127_0_0_1("127.0.0.1");

static bool isLocalHost(const String& host) {
  return host.empty() || host == s_localhost || host == s_127_0_0_1;
}

bool XboxServer::SendMessage(const String& message,
                             Array& ret,
                             int timeout_ms,
                             const String& host /* = "localhost" */) {
  if (isLocalHost(host)) {
    XboxTransport *job;
    {
      Lock l(s_dispatchMutex);
      if (!s_dispatcher) {
        return false;
      }

      job = new XboxTransport(message);
      job->incRefCount(); // paired with worker's decRefCount()
      job->incRefCount(); // paired with decRefCount() at below
      assert(s_dispatcher);
      s_dispatcher->enqueue(job);
    }

    if (timeout_ms <= 0) {
      timeout_ms = RuntimeOption::XboxDefaultLocalTimeoutMilliSeconds;
    }

    int code = 0;
    String response = job->getResults(code, timeout_ms);
    job->decRefCount(); // i'm done with this job

    if (code > 0) {
      ret.set(s_code, code);
      if (code == 200) {
        ret.set(s_response, unserialize_from_string(response));
      } else {
        ret.set(s_error, response);
      }
      return true;
    }

  } else { // remote

    string url = "http://";
    url += host.data();
    url += '/';
    url += RuntimeOption::XboxProcessMessageFunc;

    int timeoutSeconds = timeout_ms / 1000;
    if (timeoutSeconds <= 0) {
      timeoutSeconds = RuntimeOption::XboxDefaultRemoteTimeoutSeconds;
    }

    string hostStr(host.data());
    std::vector<std::string> headers;
    LibEventHttpClientPtr http =
      LibEventHttpClient::Get(hostStr, RuntimeOption::XboxServerPort);
    if (http->send(url, headers, timeoutSeconds, false,
                   message.data(), message.size())) {
      int code = http->getCode();
      if (code > 0) {
        int len = 0;
        char *response = http->recv(len);
        String sresponse(response, len, AttachString);
        ret.set(s_code, code);
        if (code == 200) {
          ret.set(s_response, unserialize_from_string(sresponse));
        } else {
          ret.set(s_error, sresponse);
        }
        return true;
      }
      // code wasn't correctly set by http client, treat it as not found
      ret.set(s_code, 404);
      ret.set(s_error, "http client failed");
    }
  }

  return false;
}

bool XboxServer::PostMessage(const String& message,
                             const String& host /* = "localhost" */) {
  if (isLocalHost(host)) {
    Lock l(s_dispatchMutex);
    if (!s_dispatcher) {
      return false;
    }

    XboxTransport *job = new XboxTransport(message);
    job->incRefCount(); // paired with worker's decRefCount()
    assert(s_dispatcher);
    s_dispatcher->enqueue(job);
    return true;

  } else { // remote

    string url = "http://";
    url += host.data();
    url += "/xbox_post_message";

    std::vector<std::string> headers;
    std::string hostStr(host.data());
    LibEventHttpClientPtr http =
      LibEventHttpClient::Get(hostStr, RuntimeOption::XboxServerPort);
    if (http->send(url, headers, 0, false, message.data(), message.size())) {
      int code = http->getCode();
      if (code > 0) {
        int len = 0;
        char *response = http->recv(len);
        String sresponse(response, len, AttachString);
        if (code == 200 && same(unserialize_from_string(sresponse), true)) {
          return true;
        }
      }
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

class XboxTask : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(XboxTask)

  XboxTask(const String& message, const String& reqInitDoc = "") {
    m_job = new XboxTransport(message, reqInitDoc);
    m_job->incRefCount();
  }

  ~XboxTask() {
    m_job->decRefCount();
  }

  XboxTransport *getJob() { return m_job;}

  CLASSNAME_IS("XboxTask");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

private:
  XboxTransport *m_job;
};
IMPLEMENT_OBJECT_ALLOCATION(XboxTask)

///////////////////////////////////////////////////////////////////////////////

Resource XboxServer::TaskStart(const String& msg, const String& reqInitDoc /* = "" */,
    ServerTaskEvent<XboxServer, XboxTransport> *event /* = nullptr */) {
  {
    Lock l(s_dispatchMutex);
    if (s_dispatcher &&
        (s_dispatcher->getActiveWorker() <
         RuntimeOption::XboxServerThreadCount ||
         s_dispatcher->getQueuedJobs() <
         RuntimeOption::XboxServerMaxQueueLength)) {
      XboxTask *task = NEWOBJ(XboxTask)(msg, reqInitDoc);
      Resource ret(task);
      XboxTransport *job = task->getJob();
      job->incRefCount(); // paired with worker's decRefCount()

      Transport *transport = g_context->getTransport();
      if (transport) {
        job->setHost(transport->getHeader("Host"));
      }

      if (event) {
        job->setAsioEvent(event);
        event->setJob(job);
      }

      assert(s_dispatcher);
      s_dispatcher->enqueue(job);

      return ret;
    }
  }
  const char* errMsg =
    (RuntimeOption::XboxServerThreadCount > 0 ?
     "Cannot create new Xbox task because the Xbox queue has "
     "reached maximum capacity" :
     "Cannot create new Xbox task because the Xbox is not enabled");

  Object e = SystemLib::AllocExceptionObject(errMsg);
  throw_exception(e);
  return Resource();
}

bool XboxServer::TaskStatus(const Resource& task) {
  XboxTask *ptask = task.getTyped<XboxTask>();
  return ptask->getJob()->isDone();
}

int XboxServer::TaskResult(const Resource& task, int timeout_ms, Variant &ret) {
  XboxTask *ptask = task.getTyped<XboxTask>();
  return TaskResult(ptask->getJob(), timeout_ms, ret);
}

int XboxServer::TaskResult(XboxTransport *job, int timeout_ms, Variant &ret) {
  int code = 0;
  String response = job->getResults(code, timeout_ms);
  if (code == 200) {
    ret = unserialize_from_string(response);
  } else {
    ret = response;
  }
  return code;
}

std::shared_ptr<XboxServerInfo> XboxServer::GetServerInfo() {
  return *s_xbox_server_info;
}

RPCRequestHandler *XboxServer::GetRequestHandler() {
  if (s_xbox_request_handler.isNull()) return nullptr;
  return s_xbox_request_handler.get();
}

///////////////////////////////////////////////////////////////////////////////
}
