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

#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/upload.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/ext/ext_server.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/service-data.h"
#include "hphp/util/timer.h"

using std::set;
using std::deque;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

PageletTransport::PageletTransport(
    const String& url, CArrRef headers, const String& postData,
    const String& remoteHost, const set<string> &rfc1867UploadedFiles,
    CArrRef files, int timeoutSeconds)
    : m_refCount(0),
      m_timeoutSeconds(timeoutSeconds),
      m_done(false),
      m_code(0),
      m_event(nullptr) {

  Timer::GetMonotonicTime(m_queueTime);
  m_threadType = ThreadType::PageletThread;

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
  m_files = (std::string) f_serialize(files);
}

const char *PageletTransport::getUrl() {
  return m_url.c_str();
}

const char *PageletTransport::getRemoteHost() {
  return m_remoteHost.c_str();
}

uint16_t PageletTransport::getRemotePort() {
  return 0;
}

const void *PageletTransport::getPostData(int &size) {
  size = m_postData.size();
  return m_postData.data();
}

Transport::Method PageletTransport::getMethod() {
  return m_get ? Transport::Method::GET : Transport::Method::POST;
}

std::string PageletTransport::getHeader(const char *name) {
  assert(name && *name);
  HeaderMap::const_iterator iter = m_requestHeaders.find(name);
  if (iter != m_requestHeaders.end()) {
    return iter->second[0];
  }
  return "";
}

void PageletTransport::getHeaders(HeaderMap &headers) {
  headers = m_requestHeaders;
}

void PageletTransport::addHeaderImpl(const char *name, const char *value) {
  assert(name && *name);
  assert(value);
  m_responseHeaders[name].push_back(value);
}

void PageletTransport::removeHeaderImpl(const char *name) {
  assert(name && *name);
  m_responseHeaders.erase(name);
}

void PageletTransport::sendImpl(const void *data, int size, int code,
                      bool chunked) {
  m_response.append((const char*)data, size);
  if (code) {
    m_code = code;
  }
}

void PageletTransport::onSendEndImpl() {
  Lock lock(this);
  m_done = true;
  if (m_event) {
    m_event->finish();
  }
  notify();
}

bool PageletTransport::isUploadedFile(const String& filename) {
  return m_rfc1867UploadedFiles.find(filename.c_str()) !=
         m_rfc1867UploadedFiles.end();
}

bool PageletTransport::moveUploadedFile(const String& filename,
    const String& destination) {
  if (!isUploadedFile(filename.c_str())) {
    Logger::Error("%s is not an uploaded file.", filename.c_str());
    return false;
  }
  return moveUploadedFileHelper(filename, destination);
}

bool PageletTransport::getFiles(string &files) {
  files = m_files;
  return true;
}

bool PageletTransport::isDone() {
  return m_done;
}

void PageletTransport::addToPipeline(const string &s) {
  Lock lock(this);
  m_pipeline.push_back(s);
  if (m_event) {
    m_event->finish();
    m_event = nullptr;
  }
  notify();
}

bool PageletTransport::isPipelineEmpty() {
  Lock lock(this);
  return m_pipeline.empty();
}

bool PageletTransport::getResults(
  Array &results,
  PageletServerTaskEvent* next_event
) {
  {
    Lock lock(this);
    assert(m_done || !m_pipeline.empty());
    while (!m_pipeline.empty()) {
      string &str = m_pipeline.front();
      String response(str.c_str(), str.size(), CopyString);
      results.append(response);
      m_pipeline.pop_front();
    }
    if (m_done) {
      String response(m_response.c_str(), m_response.size(), CopyString);
      results.append(response);
      return true;
    } else {
      m_event = next_event;
      m_event->setJob(this);
      return false;
    }
  }
}

String PageletTransport::getResults(
  Array &headers,
  int &code,
  int64_t timeout_ms
) {
  {
    Lock lock(this);
    while (!m_done && m_pipeline.empty()) {
      if (timeout_ms > 0) {
        long seconds = timeout_ms / 1000;
        long long nanosecs = (timeout_ms % 1000) * 1000000;
        if (!wait(seconds, nanosecs)) {
          code = -1;
          return "";
        }
      } else {
        wait();
      }
    }

    if (!m_pipeline.empty()) {
      // intermediate results do not have headers and code
      string ret = m_pipeline.front();
      m_pipeline.pop_front();
      code = 0;
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
void PageletTransport::incRefCount() {
  ++m_refCount;
}

void PageletTransport::decRefCount() {
  assert(m_refCount.load() > 0);
  if (--m_refCount == 0) {
    delete this;
  }
}

const timespec& PageletTransport::getStartTimer() const {
  return m_queueTime;
}

int PageletTransport::getTimeoutSeconds() const {
  return m_timeoutSeconds;
}

void PageletTransport::setAsioEvent(PageletServerTaskEvent *event) {
  m_event = event;
}

///////////////////////////////////////////////////////////////////////////////

static int64_t to_ms(const timespec& ts) {
  return ts.tv_sec * 1000 + (ts.tv_nsec / 1000000);
}

struct PageletWorker
  : JobQueueWorker<PageletTransport*,Server*,true,false,JobQueueDropVMStack>
{
  virtual void doJob(PageletTransport *job) {
    try {
      job->onRequestStart(job->getStartTimer());
      int timeout = job->getTimeoutSeconds();
      if (timeout > 0) {
        timespec ts;
        Timer::GetMonotonicTime(ts);
        int64_t delta_ms =
          to_ms(job->getStartTimer()) + timeout * 1000 - to_ms(ts);
        if (delta_ms > 500) {
          timeout = (delta_ms + 500) / 1000;
        } else {
          timeout = 1;
        }
      } else {
        timeout = 0;
      }
      HttpRequestHandler(timeout).handleRequest(job);
      job->decRefCount();
    } catch (...) {
      Logger::Error("HttpRequestHandler leaked exceptions");
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

class PageletTask : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(PageletTask)

  PageletTask(const String& url, CArrRef headers, const String& post_data,
              const String& remote_host,
              const std::set<std::string> &rfc1867UploadedFiles,
              CArrRef files, int timeoutSeconds) {
    m_job = new PageletTransport(url, headers, remote_host, post_data,
                                 rfc1867UploadedFiles, files, timeoutSeconds);
    m_job->incRefCount();
  }

  ~PageletTask() {
    m_job->decRefCount();
  }

  PageletTransport *getJob() { return m_job;}

  CLASSNAME_IS("PageletTask");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

private:
  PageletTransport *m_job;
};
IMPLEMENT_OBJECT_ALLOCATION(PageletTask)

///////////////////////////////////////////////////////////////////////////////
// implementing PageletServer

static JobQueueDispatcher<PageletWorker> *s_dispatcher;
static Mutex s_dispatchMutex;

bool PageletServer::Enabled() {
  return s_dispatcher;
}

void PageletServer::Restart() {
  Stop();
  if (RuntimeOption::PageletServerThreadCount > 0) {
    {
      Lock l(s_dispatchMutex);
      s_dispatcher = new JobQueueDispatcher<PageletWorker>
        (RuntimeOption::PageletServerThreadCount,
         RuntimeOption::PageletServerThreadRoundRobin,
         RuntimeOption::PageletServerThreadDropCacheTimeoutSeconds,
         RuntimeOption::PageletServerThreadDropStack,
         nullptr);
    }
    Logger::Info("pagelet server started");
    s_dispatcher->start();
  }
}

void PageletServer::Stop() {
  if (s_dispatcher) {
    s_dispatcher->stop();
    Lock l(s_dispatchMutex);
    delete s_dispatcher;
    s_dispatcher = nullptr;
  }
}

Resource PageletServer::TaskStart(
  const String& url, CArrRef headers,
  const String& remote_host,
  const String& post_data /* = null_string */,
  CArrRef files /* = null_array */,
  int timeoutSeconds /* = -1 */,
  PageletServerTaskEvent *event /* = nullptr*/
) {
  static auto pageletOverflowCounter =
    ServiceData::createTimeseries("pagelet_overflow",
                                  { ServiceData::StatsType::COUNT });
  {
    Lock l(s_dispatchMutex);
    if (!s_dispatcher) {
      return null_resource;
    }
    if (RuntimeOption::PageletServerQueueLimit > 0 &&
        s_dispatcher->getQueuedJobs() >
        RuntimeOption::PageletServerQueueLimit) {
      pageletOverflowCounter->addValue(1);
      return null_resource;
    }
  }
  PageletTask *task = NEWOBJ(PageletTask)(url, headers, remote_host, post_data,
                                          get_uploaded_files(), files,
                                          timeoutSeconds);
  Resource ret(task);
  PageletTransport *job = task->getJob();
  Lock l(s_dispatchMutex);
  if (s_dispatcher) {
    job->incRefCount(); // paired with worker's decRefCount()

    if (event) {
      job->setAsioEvent(event);
      event->setJob(job);
    }

    s_dispatcher->enqueue(job);
    return ret;
  }
  return null_resource;
}

int64_t PageletServer::TaskStatus(CResRef task) {
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

String PageletServer::TaskResult(CResRef task, Array &headers, int &code,
                                 int64_t timeout_ms) {
  PageletTask *ptask = task.getTyped<PageletTask>();
  return ptask->getJob()->getResults(headers, code, timeout_ms);
}

void PageletServer::AddToPipeline(const string &s) {
  assert(!s.empty());
  PageletTransport *job =
    dynamic_cast<PageletTransport *>(g_context->getTransport());
  assert(job);
  job->addToPipeline(s);
}

int PageletServer::GetActiveWorker() {
  Lock l(s_dispatchMutex);
  return s_dispatcher ? s_dispatcher->getActiveWorker() : 0;
}

int PageletServer::GetQueuedJobs() {
  Lock l(s_dispatchMutex);
  return s_dispatcher ? s_dispatcher->getQueuedJobs() : 0;
}

///////////////////////////////////////////////////////////////////////////////
}
