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

#include <runtime/base/server/pagelet_server.h>
#include <runtime/base/server/transport.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/upload.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/resource_data.h>
#include <runtime/ext/ext_server.h>
#include <util/job_queue.h>
#include <util/lock.h>
#include <util/logger.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class PageletTransport : public Transport, public Synchronizable {
public:
  PageletTransport(CStrRef url, CArrRef headers, CStrRef postData,
                   CStrRef remoteHost, const set<string> &rfc1867UploadedFiles,
                   CArrRef files)
    : m_refCount(0), m_done(false), m_code(0) {

    gettime(CLOCK_MONOTONIC, &m_queueTime);
    m_threadType = PageletThread;

    m_url.append(url.data(), url.size());
    m_remoteHost.append(remoteHost.data(), remoteHost.size());

    for (ArrayIter iter(headers); iter; ++iter) {
      Variant key = iter.first();
      String header = iter.second();
      if (key.isString() && !key.toString().empty()) {
        m_requestHeaders[key.toString().data()].push_back(header.data());
      } else {
        int pos = header.find(": ");
        if (pos >= 0) {
          string name = header.substr(0, pos).data();
          string value = header.substr(pos + 2).data();
          m_requestHeaders[name].push_back(value);
        } else {
          Logger::Error("throwing away bad header: %s", header.data());
        }
      }
    }

    if (postData.empty()) {
      m_get = true;
    } else {
      m_get = false;
      m_postData.append(postData.data(), postData.size());
    }

    disableCompression(); // so we don't have to decompress during sendImpl()
    m_rfc1867UploadedFiles = rfc1867UploadedFiles;
    m_files = f_serialize(files);
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
  virtual bool isUploadedFile(CStrRef filename) {
    return m_rfc1867UploadedFiles.find(filename.c_str()) !=
           m_rfc1867UploadedFiles.end();
  }
  virtual bool moveUploadedFile(CStrRef filename, CStrRef destination) {
    if (!isUploadedFile(filename.c_str())) {
      Logger::Error("%s is not an uploaded file.", filename.c_str());
      return false;
    }
    return moveUploadedFileHelper(filename, destination);
  }
  virtual bool getFiles(string &files) {
    files = m_files;
    return true;
  }

  // task interface
  bool isDone() {
    return m_done;
  }

  void addToPipeline(const string &s) {
    Lock lock(this);
    m_pipeline.push_back(s);
    notify();
  }

  bool isPipelineEmpty() {
    Lock lock(this);
    return m_pipeline.empty();
  }

  String getResults(Array &headers, int &code) {
    {
      Lock lock(this);
      while (!m_done && m_pipeline.empty()) wait();
      if (!m_pipeline.empty()) {
        // intermediate results do not have headers and code
        string ret = m_pipeline.front();
        m_pipeline.pop_front();
        return ret;
      }
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
    atomic_inc(m_refCount);
  }
  void decRefCount() {
    ASSERT(m_refCount);
    if (atomic_dec(m_refCount) == 0) {
      delete this;
    }
  }

  timespec getStartTimer() const { return m_queueTime; }
private:
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

  deque<string> m_pipeline; // the intermediate pagelet results
  set<string> m_rfc1867UploadedFiles;
  string m_files; // serialized to use as $_FILES
};

///////////////////////////////////////////////////////////////////////////////

class PageletWorker : public JobQueueWorker<PageletTransport*> {
public:
  virtual void doJob(PageletTransport *job) {
    try {
      job->onRequestStart(job->getStartTimer());
      HttpRequestHandler().handleRequest(job);
      job->decRefCount();
    } catch (...) {
      Logger::Error("HttpRequestHandler leaked exceptions");
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

class PageletTask : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(PageletTask)

  PageletTask(CStrRef url, CArrRef headers, CStrRef post_data,
              CStrRef remote_host,
              const std::set<std::string> &rfc1867UploadedFiles,
              CArrRef files) {
    m_job = new PageletTransport(url, headers, remote_host, post_data,
                                 rfc1867UploadedFiles, files);
    m_job->incRefCount();
  }

  ~PageletTask() {
    m_job->decRefCount();
  }

  PageletTransport *getJob() { return m_job;}

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassName() const { return s_class_name; }

private:
  PageletTransport *m_job;
};
IMPLEMENT_OBJECT_ALLOCATION(PageletTask)

StaticString PageletTask::s_class_name("PageletTask");

///////////////////////////////////////////////////////////////////////////////
// implementing PageletServer

static JobQueueDispatcher<PageletTransport*, PageletWorker> *s_dispatcher;

bool PageletServer::Enabled() {
  return RuntimeOption::PageletServerThreadCount > 0;
}

void PageletServer::Restart() {
  if (s_dispatcher) {
    s_dispatcher->stop();
    delete s_dispatcher;
    s_dispatcher = NULL;
  }
  if (RuntimeOption::PageletServerThreadCount > 0) {
    s_dispatcher = new JobQueueDispatcher<PageletTransport*, PageletWorker>
      (RuntimeOption::PageletServerThreadCount,
       RuntimeOption::PageletServerThreadRoundRobin,
       RuntimeOption::PageletServerThreadDropCacheTimeoutSeconds,
       NULL);
    Logger::Info("pagelet server started");
    s_dispatcher->start();
  }
}

Object PageletServer::TaskStart(CStrRef url, CArrRef headers,
                                CStrRef remote_host,
                                CStrRef post_data /* = null_string */,
                                CArrRef files /* = null_array */) {
  if (RuntimeOption::PageletServerThreadCount <= 0) {
    return null_object;
  }
  PageletTask *task = NEWOBJ(PageletTask)(url, headers, remote_host, post_data,
                                          get_uploaded_files(), files);
  Object ret(task);
  PageletTransport *job = task->getJob();
  job->incRefCount(); // paired with worker's decRefCount()
  ASSERT(s_dispatcher);
  s_dispatcher->enqueue(job);

  return ret;
}

int64 PageletServer::TaskStatus(CObjRef task) {
  PageletTask *ptask = task.getTyped<PageletTask>();
  PageletTransport *job = ptask->getJob();
  if (!job->isPipelineEmpty()) {
    return PAGELET_READY;
  }
  if (job->isDone()) {
    return PAGELET_DONE;
  }
  return PAGELET_NOT_READY;
}

String PageletServer::TaskResult(CObjRef task, Array &headers, int &code) {
  PageletTask *ptask = task.getTyped<PageletTask>();
  return ptask->getJob()->getResults(headers, code);
}

void PageletServer::AddToPipeline(const string &s) {
  ASSERT(!s.empty());
  PageletTransport *job =
    dynamic_cast<PageletTransport *>(g_context->getTransport());
  ASSERT(job);
  job->addToPipeline(s);
}

///////////////////////////////////////////////////////////////////////////////
}
