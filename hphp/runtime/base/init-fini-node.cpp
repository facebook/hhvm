/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

InitFiniNode *InitFiniNode::s_nodes[NumNodes];
InitFiniNode::IFDispatcher* InitFiniNode::s_dispatcher;

void InitFiniNode::iterate(InitFiniNode *node) {
  while (node) {
    node->func();
    node = node->next;
  }
}

void InitFiniNode::IFWorker::doJob(std::shared_ptr<IFJob> job) {
  Timer timer(Timer::WallTime, job->node.name);
  job->node.func();
}

void InitFiniNode::ProcessInitConcurrentStart(uint32_t maxWorkers) {
  if (maxWorkers == 0) {
    iterate(When::ProcessInitConcurrent);
    return;
  }
  std::vector<std::shared_ptr<IFJob> > jobs;
  for (const auto* n = node(When::ProcessInitConcurrent); n; n = n->next) {
    jobs.push_back(std::make_shared<IFJob>(*n));
  }
  s_dispatcher = new IFDispatcher(std::move(jobs), maxWorkers);
  s_dispatcher->start();
}

void InitFiniNode::ProcessInitConcurrentWaitForEnd() {
  if (s_dispatcher) {
    s_dispatcher->waitForEnd();
    delete s_dispatcher;
    s_dispatcher = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
