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

#include <runtime/base/server/pagelet_server.h>
#include <runtime/base/server/transport.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/resource_data.h>
#include <util/job_queue.h>
#include <util/lock.h>
#include <util/logger.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class PageletTransport : public Transport, public Synchronizable {
public:
  PageletTransport(CStrRef url, CArrRef headers, CStrRef postData,
                   CStrRef remoteHost)
    : m_refCount(0), m_done(false), m_code(0) {
    m_url.append(url.data(), url.size());
    m_remoteHost.append(remoteHost.data(), remoteHost.size());

    for (ArrayIter iter(headers); iter; ++iter) {
      String header = iter.second();
      int pos = header.find(": ");
      if (pos >= 0) {
        string name = header.substr(0, pos).data();
        string value = header.substr(pos + 2).data();
        m_requestHeaders[name].push_back(value);
      } else {
        Logger::Error("throwing away bad header: %s", header.data());
      }
    }

    if (postData.isNull()) {
      m_get = true;
    } else {
      m_get = false;
      m_postData.append(postData.data(), postData.size());
    }

    disableCompression(); // so we don't have to decompress during sendImpl()
  }

  /**
   * Implementing Transport...
   */
  virtual const char *getUrl() {
    return m_url.c_str();
  }
  virtual const char *getRemoteHost() {
    return m_remoteHost.c_str();
  }
  virtual const void *getPostData(int &size) {
    size = m_postData.size();
    return m_postData.data();
  }
  virtual Method getMethod() {
    return m_get ? Transport::GET : Transport::POST;
  }
  virtual std::string getHeader(const char *name) {
    ASSERT(name && *name);
    HeaderMap::const_iterator iter = m_requestHeaders.find(name);
    if (iter != m_requestHeaders.end()) {
      return iter->second[0];
    }
    return "";
  }
  virtual void getHeaders(HeaderMap &headers) {
    headers = m_requestHeaders;
  }
  virtual void addHeaderImpl(const char *name, const char *value) {
    ASSERT(name && *name);
    ASSERT(value);
    m_responseHeaders[name].push_back(value);
  }
  virtual void removeHeaderImpl(const char *name) {
    ASSERT(name && *name);
    m_responseHeaders.erase(name);
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

  String getResults(Array &headers, int &code) {
    {
      Lock lock(this);
      while (!m_done) wait();
    }

    String response(m_response.c_str(), m_response.size(), CopyString);
    headers = Array::Create();
    for (HeaderMap::const_iterator iter = m_responseHeaders.begin();
         iter != m_responseHeaders.end(); ++iter) {
      for (unsigned int i = 0; i < iter->second.size(); i++) {
        StringBuffer sb;
        sb.append(iter->first);
        sb.append(": ");
        sb.append(iter->second[i]);
        headers.append(sb.detach());
      }
    }
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

private:
  Mutex m_mutex;
  int m_refCount;

  string m_url;
  HeaderMap m_requestHeaders;
  bool m_get;
  string m_postData;
  string m_remoteHost;

  bool m_done;
  HeaderMap m_responseHeaders;
  string m_response;
  int m_code;
};

///////////////////////////////////////////////////////////////////////////////

class PageletWorker : public JobQueueWorker<PageletTransport*> {
public:
  virtual void doJob(PageletTransport *job) {
    try {
      HttpRequestHandler().handleRequest(job);
      job->decRefCount();
    } catch (...) {
      Logger::Error("HttpRequestHandler leaked exceptions");
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

class PageletTask : public ResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(PageletTask);

  PageletTask(CStrRef url, CArrRef headers, CStrRef post_data,
              CStrRef remote_host) {
    m_job = new PageletTransport(url, headers, remote_host, post_data);
    m_job->incRefCount();
  }

  ~PageletTask() {
    m_job->decRefCount();
  }

  PageletTransport *getJob() { return m_job;}

  // overriding ResourceData
  virtual const char *o_getClassName() const { return "PageletTask";}

private:
  PageletTransport *m_job;
};
IMPLEMENT_OBJECT_ALLOCATION(PageletTask);

///////////////////////////////////////////////////////////////////////////////
// implementing PageletServer

static JobQueueDispatcher<PageletTransport*, PageletWorker> *s_dispatcher;

void PageletServer::Restart() {
  if (s_dispatcher) {
    s_dispatcher->stop();
    delete s_dispatcher;
    s_dispatcher = NULL;
  }
  if (RuntimeOption::PageletServerThreadCount > 0) {
    s_dispatcher = new JobQueueDispatcher<PageletTransport*, PageletWorker>
      (RuntimeOption::PageletServerThreadCount, NULL);
    Logger::Info("pagelet server started");
    s_dispatcher->start();
  }
}

Object PageletServer::TaskStart(CStrRef url, CArrRef headers,
                                CStrRef remote_host,
                                CStrRef post_data /* = null_string */) {
  if (RuntimeOption::PageletServerThreadCount <= 0) {
    return null_object;
  }
  PageletTask *task = NEW(PageletTask)(url, headers, remote_host, post_data);
  Object ret(task);
  PageletTransport *job = task->getJob();
  job->incRefCount(); // paired with worker's decRefCount()
  ASSERT(s_dispatcher);
  s_dispatcher->enqueue(job);

  return ret;
}

bool PageletServer::TaskStatus(CObjRef task) {
  PageletTask *ptask = task.getTyped<PageletTask>();
  return ptask->getJob()->isDone();
}

String PageletServer::TaskResult(CObjRef task, Array &headers, int &code) {
  PageletTask *ptask = task.getTyped<PageletTask>();
  return ptask->getJob()->getResults(headers, code);
}

///////////////////////////////////////////////////////////////////////////////
}
