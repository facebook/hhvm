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

#pragma once

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/service-data.h"

namespace HPHP {

struct Transport;

struct ServerJob {
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
  ~ServerWorker() override {}

  /**
   * Request handler called by Server.
   */
  void doJob(JobPtr job) override {
    doJobImpl(job, false /*abort*/);
  }
  void abortJob(JobPtr job) override {
    doJobImpl(job, true /*abort*/);
    m_requestsTimedOutOnQueue->addValue(1);
  }

  /**
   * Called when thread enters and exits.
   */
  void onThreadEnter() override {
    assertx(this->m_context);
    m_handler = this->m_context->createRequestHandler();
    m_requestsTimedOutOnQueue =
      ServiceData::createTimeSeries("requests_timed_out_on_queue",
                                    {ServiceData::StatsType::COUNT});
  }

  void onThreadExit() override {
    assertx(this->m_context);
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

    // rpc threads keep things live between requests, but other
    // requests should not have allocated anything yet.
    assertx(vmStack().isAllocated() || tl_heap->empty());

    SCOPE_EXIT { m_handler->teardownRequest(transport); };

    try {
      transport->onRequestStart(job->getStartTimer());
      m_handler->setupRequest(transport);

      if (abort) {
        m_handler->abortRequest(transport);
        return;
      }
      std::string cmd = transport->getCommand();
      cmd = std::string("/") + cmd;

      if (server->shouldHandle(cmd)) {
        m_handler->handleRequest(transport);
        error = false;
      } else {
        transport->sendString("Not Found", 404);
        transport->onSendEnd();
        return;
      }
    } catch (Exception& e) {
      if (Server::StackTraceOnError) {
        errorMsg = e.what();
      } else {
        errorMsg = e.getMessage();
      }
    } catch (std::exception& e) {
      errorMsg = e.what();
    } catch (...) {
      errorMsg = "(unknown exception)";
    }

    if (error) {
      if (RuntimeOption::ServerErrorMessage) {
        transport->sendString(errorMsg, 500);
      } else {
        transport->sendString(Cfg::Server::FatalErrorMessage, 500);
      }
      transport->onSendEnd();
    }
  }

  std::unique_ptr<RequestHandler> m_handler;
  ServiceData::ExportedTimeSeries* m_requestsTimedOutOnQueue;
};

}
