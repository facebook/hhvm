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
#ifndef incl_HPHP_TREADMILL_INL_H_
#define incl_HPHP_TREADMILL_INL_H_

#include <memory>
#include <utility>

namespace HPHP { namespace Treadmill {

//////////////////////////////////////////////////////////////////////

struct WorkItem;
void enqueueInternal(std::unique_ptr<WorkItem>);

//////////////////////////////////////////////////////////////////////

using GenCount = int64_t;

struct WorkItem {
  WorkItem() = default;
  WorkItem(const WorkItem&) = delete;
  WorkItem& operator=(const WorkItem&) = delete;
  virtual ~WorkItem() {}

  virtual void run() noexcept = 0;

private:
  friend void finishRequest();
  friend void enqueueInternal(std::unique_ptr<WorkItem>);

private:
  // Inherently racy. We get a lower bound on the generation;
  // presumably clients are aware of this, and are creating the
  // trigger for an object that was reachable strictly in the past.
  GenCount m_gen{0};
};

template<class F>
struct WorkItemImpl final : WorkItem {
  explicit WorkItemImpl(F f) : f(f) {}

  void run() noexcept override { f(); }

private:
  F f;
};

template<class F>
void enqueue(F&& f) {
  std::unique_ptr<WorkItemImpl<F>> p(new WorkItemImpl<F>(std::forward<F>(f)));
  enqueueInternal(std::move(p));
}

//////////////////////////////////////////////////////////////////////

}}

#endif
