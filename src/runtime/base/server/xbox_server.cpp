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

#include <runtime/base/server/xbox_server.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/rpc_request_handler.h>
#include <runtime/base/server/satellite_server.h>
#include <runtime/base/util/libevent_http_client.h>
#include <runtime/ext/ext_json.h>
#include <util/job_queue.h>
#include <util/lock.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class XboxTransport : public Transport, public Synchronizable {
public:
  XboxTransport(CStrRef message) : m_refCount(0), m_done(false), m_code(0) {
    m_message.append(message.data(), message.size());
    disableCompression(); // so we don't have to decompress during sendImpl()
  }

  /**
   * Implementing Transport...
   */
  virtual const char *getUrl() {
    return RuntimeOption::XboxProcessMessageFunc.c_str();
  }
  virtual const char *getRemoteHost() {
    return "127.0.0.1";
  }
  virtual const void *getPostData(int &size) {
    size = m_message.size();
    return m_message.data();
  }
  virtual Method getMethod() {
    return Transport::POST;
  }
  virtual std::string getHeader(const char *name) {
    if (!strcasecmp(name, "Host")) return m_host;
    return "";
  }
  virtual void getHeaders(HeaderMap &headers) {
    // do nothing
  }
  virtual void addHeaderImpl(const char *name, const char *value) {
    // do nothing
  }
  virtual void removeHeaderImpl(const char *name) {
    // do nothing
  }
  virtual void sendImpl(const void *data, int size, int code,
                        bool compressed) {
    ASSERT(!compressed);
    m_response.append((const char*)data, size);
    if (code) {
      m_code = code;
    }
  }
  virtual void onSendEndImpl() {
    Lock lock(this);
    m_done = true;
    notify();
  }

  // task interface
  bool isDone() {
    return m_done;
  }

  String getResults(int &code, int timeout_ms = 0) {
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

  // ref counting
  void incRefCount() {
    Lock lock(m_mutex);
    ++m_refCount;
  }
  void decRefCount() {
    {
      Lock lock(m_mutex);
      --m_refCount;
    }
    if (m_refCount == 0) {
      delete this;
    }
  }

  void setHost(const std::string &host) { m_host = host;}
private:
  Mutex m_mutex;
  int m_refCount;

  string m_message;

  bool m_done;
  string m_response;
  int m_code;
  string m_host;
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(XboxServerInfo);
class XboxServerInfo : public SatelliteServerInfo {
public:
  XboxServerInfo() : SatelliteServerInfo(Hdf()) {
    m_type = SatelliteServer::KindOfXboxServer;
  }

  void reload() {
    m_maxRequest  = RuntimeOption::XboxServerInfoMaxRequest;
    m_maxDuration = RuntimeOption::XboxServerInfoDuration;
    m_warmupDoc   = RuntimeOption::XboxServerInfoWarmupDoc;
    m_reqInitFunc = RuntimeOption::XboxServerInfoReqInitFunc;
  }
};

static XboxServerInfoPtr s_xbox_server_info;
static IMPLEMENT_THREAD_LOCAL(RPCRequestHandler, s_rpc_request_handler);
///////////////////////////////////////////////////////////////////////////////

class XboxWorker : public JobQueueWorker<XboxTransport*> {
public:
  RequestHandler *createRequestHandler() {
    s_rpc_request_handler->setServerInfo(s_xbox_server_info);
    if (s_rpc_request_handler->needReset() ||
        s_rpc_request_handler->incRequest() >
        s_xbox_server_info->getMaxRequest()) {
      s_rpc_request_handler.reset();
      s_rpc_request_handler->setServerInfo(s_xbox_server_info);
      s_rpc_request_handler->incRequest();
    }
    return s_rpc_request_handler.get();
  }

  virtual void doJob(XboxTransport *job) {
    try {
      createRequestHandler()->handleRequest(job);
      job->decRefCount();
    } catch (...) {
      Logger::Error("RpcRequestHandler leaked exceptions");
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

static JobQueueDispatcher<XboxTransport*, XboxWorker> *s_dispatcher;

void XboxServer::Restart() {
  if (s_dispatcher) {
    s_dispatcher->stop();
    delete s_dispatcher;
    s_dispatcher = NULL;
  }

  s_xbox_server_info = XboxServerInfoPtr(new XboxServerInfo());
  s_xbox_server_info->reload();

  if (RuntimeOption::XboxServerThreadCount > 0) {
    s_dispatcher = new JobQueueDispatcher<XboxTransport*, XboxWorker>
      (RuntimeOption::XboxServerThreadCount, NULL);
    Logger::Info("xbox server started");
    s_dispatcher->start();
  }
}

///////////////////////////////////////////////////////////////////////////////

static bool isLocalHost(CStrRef host) {
  return host.empty() || host == "localhost" || host == "127.0.0.1";
}

bool XboxServer::SendMessage(CStrRef message, Variant &ret, int timeout_ms,
                             CStrRef host /* = "localhost" */) {
  if (isLocalHost(host)) {

    if (RuntimeOption::XboxServerThreadCount <= 0) {
      return false;
    }

    XboxTransport *job = new XboxTransport(message);
    job->incRefCount(); // paired with worker's decRefCount()
    job->incRefCount(); // paired with decRefCount() at below
    ASSERT(s_dispatcher);
    s_dispatcher->enqueue(job);

    if (timeout_ms <= 0) {
      timeout_ms = RuntimeOption::XboxDefaultLocalTimeoutMilliSeconds;
    }

    int code = 0;
    String response = job->getResults(code, timeout_ms);
    job->decRefCount(); // i'm done with this job

    if (code > 0) {
      ret.set("code", code);
      if (code == 200) {
        ret.set("response", f_json_decode(response));
      } else {
        ret.set("error", response);
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

    vector<string> headers;
    LibEventHttpClientPtr http =
      LibEventHttpClient::Get(url, RuntimeOption::XboxServerPort);
    if (http->send(url, headers, timeoutSeconds, false,
                   message.data(), message.size())) {
      int code = http->getCode();
      if (code > 0) {
        int len = 0;
        char *response = http->recv(len);
        String sresponse(response, len, AttachString);
        ret.set("code", code);
        if (code == 200) {
          ret.set("response", f_json_decode(sresponse));
        } else {
          ret.set("error", sresponse);
        }
        return true;
      }
      ASSERT(false); // code wasn't correctly set by http client
    }
  }

  return false;
}

bool XboxServer::PostMessage(CStrRef message,
                             CStrRef host /* = "localhost" */) {
  if (isLocalHost(host)) {

    if (RuntimeOption::XboxServerThreadCount <= 0) {
      return false;
    }

    XboxTransport *job = new XboxTransport(message);
    job->incRefCount(); // paired with worker's decRefCount()
    ASSERT(s_dispatcher);
    s_dispatcher->enqueue(job);
    return true;

  } else { // remote

    string url = "http://";
    url += host.data();
    url += "/xbox_post_message";

    vector<string> headers;
    LibEventHttpClientPtr http =
      LibEventHttpClient::Get(url, RuntimeOption::XboxServerPort);
    if (http->send(url, headers, 0, false, message.data(), message.size())) {
      int code = http->getCode();
      if (code > 0) {
        int len = 0;
        char *response = http->recv(len);
        String sresponse(response, len, AttachString);
        if (code == 200 && same(f_json_decode(sresponse), true)) {
          return true;
        }
      }
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

class XboxTask : public ResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(XboxTask);

  XboxTask(CStrRef message) {
    m_job = new XboxTransport(message);
    m_job->incRefCount();
  }

  ~XboxTask() {
    m_job->decRefCount();
  }

  XboxTransport *getJob() { return m_job;}

  // overriding ResourceData
  virtual const char *o_getClassName() const { return "XboxTask";}

private:
  XboxTransport *m_job;
};
IMPLEMENT_OBJECT_ALLOCATION(XboxTask);

///////////////////////////////////////////////////////////////////////////////

Object XboxServer::TaskStart(CStrRef message) {
  if (RuntimeOption::XboxServerThreadCount <= 0) {
    return null_object;
  }
  XboxTask *task = NEW(XboxTask)(message);
  Object ret(task);
  XboxTransport *job = task->getJob();
  job->incRefCount(); // paired with worker's decRefCount()
  Transport *transport = g_context->getTransport();
  if (transport) {
    job->setHost(transport->getHeader("Host"));
  }
  ASSERT(s_dispatcher);
  s_dispatcher->enqueue(job);

  return ret;
}

bool XboxServer::TaskStatus(CObjRef task) {
  XboxTask *ptask = task.getTyped<XboxTask>();
  return ptask->getJob()->isDone();
}

int XboxServer::TaskResult(CObjRef task, int timeout_ms, Variant &ret) {
  XboxTask *ptask = task.getTyped<XboxTask>();

  int code = 0;
  String response = ptask->getJob()->getResults(code, timeout_ms);
  if (code == 200) {
    ret = f_json_decode(response);
  } else {
    ret = response;
  }
  return code;
}

///////////////////////////////////////////////////////////////////////////////
}
