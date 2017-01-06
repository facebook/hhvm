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

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

InitFiniNode *InitFiniNode::s_nodes[NumNodes];
InitFiniNode::IFDispatcher* InitFiniNode::s_dispatcher[NumNodes];

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

void InitFiniNode::concurrentStart(uint32_t maxWorkers, When when) {
  if (maxWorkers == 0) {
    iterate(when);
    return;
  }
  std::vector<std::shared_ptr<IFJob> > jobs;
  for (const auto* n = node(when); n; n = n->next) {
    jobs.push_back(std::make_shared<IFJob>(*n));
  }
  dispatcher(when) = new IFDispatcher(std::move(jobs), maxWorkers);
  dispatcher(when)->start();
}

void InitFiniNode::concurrentWaitForEnd(When when) {
  auto& d = dispatcher(when);
  if (d) {
    d->waitForEnd();
    delete d;
    d = nullptr;
  }
}

void InitFiniNode::ProcessInitConcurrentStart(uint32_t maxWorkers) {
  concurrentStart(maxWorkers, When::ProcessInitConcurrent);
}

void InitFiniNode::ProcessInitConcurrentWaitForEnd() {
  concurrentWaitForEnd(When::ProcessInitConcurrent);
}

void InitFiniNode::WarmupConcurrentStart(uint32_t maxWorkers) {
  concurrentStart(maxWorkers, When::WarmupConcurrent);
}

void InitFiniNode::WarmupConcurrentWaitForEnd() {
  concurrentWaitForEnd(When::WarmupConcurrent);
}

///////////////////////////////////////////////////////////////////////////////
}
