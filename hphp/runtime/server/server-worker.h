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

#ifndef incl_HPHP_HTTP_SERVER_SERVER_WORKER_H
#define incl_HPHP_HTTP_SERVER_SERVER_WORKER_H

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/service-data.h"

namespace HPHP {

class Transport;

class ServerJob {
public:
  explicit ServerJob();
  virtual ~ServerJob() {}

  const timespec &getStartTimer() const { return start; }
  void stopTimer(const struct timespec& reqStart);

  // ServerWorker requires a function that matches this interface
  // void getRequestStart(struct timespec *reqStart) = 0;

private:
  timespec start;
};

template <typename JobPtr, typename TransportTraits>
struct ServerWorker
  : JobQueueWorker<JobPtr,Server*,true,false,JobQueueDropVMStack>
{
  ServerWorker() {}
  virtual ~ServerWorker() {}

  /**
   * Request handler called by Server.
   */
  virtual void doJob(JobPtr job) {
    doJobImpl(job, false /*abort*/);
  }
  virtual void abortJob(JobPtr job) {
    doJobImpl(job, true /*abort*/);
    m_requestsTimedOutOnQueue->addValue(1);
  }

  /**
   * Called when thread enters and exits.
   */
  virtual void onThreadEnter() {
    assert(this->m_context);
    m_handler = this->m_context->createRequestHandler();
    m_requestsTimedOutOnQueue =
      ServiceData::createTimeseries("requests_timed_out_on_queue",
                                    {ServiceData::StatsType::COUNT});
  }

  virtual void onThreadExit() {
    assert(this->m_context);
    m_handler.reset();
  }

protected:

  void doJobImpl(JobPtr job, bool abort) {
    TransportTraits traits(job, this->m_context, this->m_id);
    Server *server = traits.getServer();
    Transport *transport = traits.getTransport();

    struct timespec reqStart;
    job->getRequestStart(&reqStart);
    job->stopTimer(reqStart);
    bool error = true;
    std::string errorMsg;

    if (abort) {
      m_handler->abortRequest(transport);
      return;
    }

    try {
      std::string cmd = transport->getCommand();
      cmd = std::string("/") + cmd;
      if (server->shouldHandle(cmd)) {
        transport->onRequestStart(job->getStartTimer());
        m_handler->handleRequest(transport);
        error = false;
      } else {
        transport->sendString("Not Found", 404);
        return;
      }
    } catch (Exception &e) {
      if (Server::StackTraceOnError) {
        errorMsg = e.what();
      } else {
        errorMsg = e.getMessage();
      }
    } catch (std::exception &e) {
      errorMsg = e.what();
    } catch (...) {
      errorMsg = "(unknown exception)";
    }

    if (error) {
      if (RuntimeOption::ServerErrorMessage) {
        transport->sendString(errorMsg, 500);
      } else {
        transport->sendString(RuntimeOption::FatalErrorMessage, 500);
      }
    }
  }

  std::unique_ptr<RequestHandler> m_handler;
  ServiceData::ExportedTimeSeries* m_requestsTimedOutOnQueue;
};

}

#endif
