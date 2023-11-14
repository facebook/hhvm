/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/xbox-request-handler.h"
#include "hphp/runtime/server/satellite-server.h"
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

XboxTransport::XboxTransport(const folly::StringPiece message,
                             const folly::StringPiece reqInitDoc /* = "" */)
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

void XboxTransport::sendImpl(const void* data, int size, int code,
                             bool /*chunked*/, bool eom) {
  m_response.append((const char*)data, size);
  if (code) {
    m_code = code;
  }
  if (eom) {
    onSendEndImpl();
  }
}

void XboxTransport::onSendEndImpl() {
  Lock lock(this);
  if (m_done) {
    return;
  }
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
          return empty_string();
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

static THREAD_LOCAL(std::shared_ptr<XboxServerInfo>, s_xbox_server_info);
static THREAD_LOCAL(std::string, s_xbox_prev_req_init_doc);
static THREAD_LOCAL(XboxRequestHandler, s_xbox_request_handler);

///////////////////////////////////////////////////////////////////////////////

struct XboxWorker
  : JobQueueWorker<XboxTransport*,Server*,true,false,JobQueueDropVMStack>
{
  void doJob(XboxTransport *job) override {
    try {
      // If this job or the previous job that ran on this thread have
      // a custom initial document, make sure we do a reset
      string reqInitDoc = job->getHeader("ReqInitDoc");
      *s_xbox_prev_req_init_doc = reqInitDoc;

      job->onRequestStart(job->getStartTimer());

      auto const handler = createRequestHandler();
      if (auto ctx = job->detachCliContext()) {
        handler->setCliContext(std::move(ctx).value());
      }
      handler->run(job);
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

    s_xbox_request_handler->setLogInfo(RuntimeOption::XboxServerLogInfo);
    s_xbox_request_handler->setServerInfo(*s_xbox_server_info);
    return s_xbox_request_handler.get();
  }

  void destroyRequestHandler() {
    if (!s_xbox_request_handler.isNull()) {
      s_xbox_request_handler.destroy();
    }
  }

  void onThreadExit() override {
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
         RuntimeOption::XboxServerThreadCount,
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
  if (!s_dispatcher) return;

  JobQueueDispatcher<XboxWorker>* dispatcher = nullptr;
  {
    Lock l(s_dispatchMutex);
    if (!s_dispatcher) return;

    dispatcher = s_dispatcher;
    s_dispatcher = nullptr;
  }

  dispatcher->stop();
  delete dispatcher;
}

bool XboxServer::Enabled() {
  return s_dispatcher;
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

///////////////////////////////////////////////////////////////////////////////

struct XboxTask : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(XboxTask)

  XboxTask(const String& message, const String& reqInitDoc = "") {
    m_job = new XboxTransport(message.toCppString(), reqInitDoc.toCppString());
    m_job->incRefCount();
    if (cli_supports_clone()) {
      m_job->setCliContext(cli_clone_context());
    }
  }

  XboxTask(const XboxTask&) = delete;
  XboxTask& operator=(const XboxTask&) = delete;

  ~XboxTask() override {
    m_job->decRefCount();
  }

  XboxTransport *getJob() { return m_job;}

  CLASSNAME_IS("XboxTask");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

private:
  XboxTransport *m_job{nullptr};
};
IMPLEMENT_RESOURCE_ALLOCATION(XboxTask)

///////////////////////////////////////////////////////////////////////////////

OptResource XboxServer::TaskStart(const String& msg,
                               const String& reqInitDoc /* = "" */,
  ServerTaskEvent<XboxServer, XboxTransport> *event /* = nullptr */) {
  static auto xboxOverflowCounter =
    ServiceData::createTimeSeries("xbox_overflow",
                                  { ServiceData::StatsType::COUNT });
  static auto xboxQueuedCounter =
    ServiceData::createTimeSeries("xbox_queued",
                                  { ServiceData::StatsType::COUNT });
  {
    Lock l(s_dispatchMutex);
    if (s_dispatcher &&
        (s_dispatcher->getActiveWorker() <
         RuntimeOption::XboxServerThreadCount ||
         s_dispatcher->getQueuedJobs() <
         RuntimeOption::XboxServerMaxQueueLength)) {
      auto task = req::make<XboxTask>(msg, reqInitDoc);
      XboxTransport *job = task->getJob();
      job->incRefCount(); // paired with worker's decRefCount()
      xboxQueuedCounter->addValue(1);

      Transport *transport = g_context->getTransport();
      if (transport) {
        job->setHost(transport->getHeader("Host"));
      }

      if (event) {
        job->setAsioEvent(event);
        event->setJob(job);
      }

      assertx(s_dispatcher);
      s_dispatcher->enqueue(job);

      return OptResource(std::move(task));
    }
  }

  auto hasXbox = RuntimeOption::XboxServerThreadCount > 0;
  const char* errMsg;
  if (hasXbox && !s_dispatcher) {
    errMsg = "Cannot create Xbox task because Xbox server is shut down";
  } else if (hasXbox) {
    errMsg = "Cannot create new Xbox task because the Xbox queue has "
     "reached maximum capacity";
  } else {
     errMsg = "Cannot create new Xbox task because the Xbox is not enabled";
  }
  if (hasXbox) {
    xboxOverflowCounter->addValue(1);
  }

  throw_exception(SystemLib::AllocExceptionObject(errMsg));
  return OptResource();
}

bool XboxServer::TaskStatus(const OptResource& task) {
  return cast<XboxTask>(task)->getJob()->isDone();
}

int XboxServer::TaskResult(const OptResource& task, int timeout_ms, Variant *ret) {
  return TaskResult(cast<XboxTask>(task)->getJob(), timeout_ms, ret);
}

int XboxServer::TaskResult(XboxTransport *job, int timeout_ms, Variant *ret) {
  int code = 0;
  String response = job->getResults(code, timeout_ms);
  if (ret) {
    if (code == 200) {
      *ret =
        unserialize_from_string(response, VariableUnserializer::Type::Internal);
    } else {
      *ret = response;
    }
  }
  return code;
}

int XboxServer::GetActiveWorkers() {
  Lock l(s_dispatchMutex);
  return s_dispatcher ? s_dispatcher->getActiveWorker() : 0;
}

int XboxServer::GetQueuedJobs() {
  Lock l(s_dispatchMutex);
  return s_dispatcher ? s_dispatcher->getQueuedJobs() : 0;
}

///////////////////////////////////////////////////////////////////////////////
}
