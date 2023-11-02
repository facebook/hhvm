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

#include "hphp/util/coro.h"

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/task_queue/BlockingQueue.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/synchronization/LifoSem.h>

#include <mutex>
#include <queue>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

using namespace folly;

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

struct ThreadFactory : NamedThreadFactory {
  ThreadFactory(const std::string& prefix,
                std::function<void()> init,
                std::function<void()> fini)
    : NamedThreadFactory(prefix)
    , m_init{std::move(init)}
    , m_fini{std::move(fini)}
  {}

  std::thread newThread(Func&& func) override {
    return NamedThreadFactory::newThread(
      [this, func = std::move(func)] () mutable {
        m_init();
        SCOPE_EXIT { m_fini(); };
        func();
      }
    );
  }

  std::function<void()> m_init;
  std::function<void()> m_fini;
};

//////////////////////////////////////////////////////////////////////

// The heart of the TicketExecutor. A queue which prioritizes
// according to Ticket.
struct TicketQueue : BlockingQueue<CPUThreadPoolExecutor::CPUTask> {
  using T = CPUThreadPoolExecutor::CPUTask;
  using Ticket = TicketExecutor::Ticket;

  // Interface specific to TicketQueue:

  BlockingQueueAddResult add(T item, Ticket ticket) {
    {
      std::lock_guard<std::mutex> _{m_lock};
      m_queue.emplace(std::move(item), ticket);
    }
    return m_sem.post();
  }

  Ticket stamp() { return m_next++; }

  // Implement interface that CPUThreadPoolExecutor expects:

  BlockingQueueAddResult add(T item) override {
    return add(std::move(item), stamp());
  }

  T take() override {
    m_sem.wait();
    return get();
  }

  folly::Optional<T> try_take_for(std::chrono::milliseconds time) override {
    if (!m_sem.try_wait_for(time)) return folly::none;
    return get();
  }

  size_t size() override {
    std::lock_guard<std::mutex> _{m_lock};
    return m_queue.size();
  }
private:
  T get() {
    std::lock_guard<std::mutex> _{m_lock};
    assertx(!m_queue.empty());
    // This is only safe because m_item does not participate in the
    // ordering priority_queue uses.
    auto item = std::move(m_queue.top().m_item);
    m_queue.pop();
    return item;
  }

  struct Pair {
    Pair(T item, Ticket ticket)
      : m_item{std::move(item)}, m_ticket{ticket} {}
    mutable T m_item;
    Ticket m_ticket;
    bool operator<(const Pair& o) const {
      // Note the ordering here. We want *lower* tickets to come
      // first.
      return o.m_ticket < m_ticket;
    }
  };

  std::atomic<Ticket> m_next{0};

  // Just a priority queue protected by a lock. Performance
  // measurements shows this isn't anywhere near a bottleneck and more
  // lock-free data structures are complex.
  std::priority_queue<Pair> m_queue;
  LifoSem m_sem;
  std::mutex m_lock;
};

//////////////////////////////////////////////////////////////////////

// Executor which keeps a reference to its parent TicketExecutor and a
// Ticket. Every call to add() just delegates to the parent, but calls
// addWithTicket() instead.
struct StickyTicketExecutor : Executor {
  ~StickyTicketExecutor() override = default;

  static Executor::KeepAlive<StickyTicketExecutor>
  create(Executor::KeepAlive<TicketExecutor> e,
         TicketExecutor::Ticket t) {
    return makeKeepAlive(new StickyTicketExecutor(std::move(e), t));
  }

  StickyTicketExecutor(const StickyTicketExecutor&) = delete;
  StickyTicketExecutor& operator=(const StickyTicketExecutor&) = delete;
  StickyTicketExecutor(StickyTicketExecutor&&) = delete;
  StickyTicketExecutor& operator=(StickyTicketExecutor&&) = delete;

  void add(Func f) override {
    m_parent->addWithTicket(std::move(f), m_ticket);
  }

protected:
  // KeepAlive interface:
  bool keepAliveAcquire() noexcept override {
    auto const DEBUG_ONLY count =
      m_counter.fetch_add(1, std::memory_order_relaxed);
    assertx(count > 0);
    return true;
  }

  void keepAliveRelease() noexcept override {
    auto const count = m_counter.fetch_sub(1, std::memory_order_acq_rel);
    assertx(count > 0);
    if (count == 1) delete this;
  }

private:
  StickyTicketExecutor(Executor::KeepAlive<TicketExecutor> parent,
                       TicketExecutor::Ticket ticket)
    : m_parent{std::move(parent)}
    , m_ticket{ticket} {}

  std::atomic<ssize_t> m_counter{1};
  Executor::KeepAlive<TicketExecutor> m_parent;
  TicketExecutor::Ticket m_ticket;
};

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

TicketExecutor::TicketExecutor(const std::string& threadPrefix,
                               size_t minThreads,
                               size_t maxThreads,
                               std::function<void()> threadInit,
                               std::function<void()> threadFini,
                               std::chrono::milliseconds threadTimeout)
  : CPUThreadPoolExecutor{
      std::make_pair(maxThreads, minThreads),
      std::make_unique<TicketQueue>(),
      std::make_shared<ThreadFactory>(
        threadPrefix, std::move(threadInit), std::move(threadFini)
      )
    }
{
  setThreadDeathTimeout(threadTimeout);
}

TicketExecutor::~TicketExecutor() {
}

void TicketExecutor::addWithTicket(Func func, Ticket ticket) {
  CPUTask task{std::move(func), std::chrono::milliseconds{0}, nullptr, 0};

  // See CPUThreadPoolExecutor.cpp why this is needed
  auto const mayNeedToAddThreads =
    minThreads_.load(std::memory_order_relaxed) == 0 ||
    (activeThreads_.load(std::memory_order_relaxed) <
     maxThreads_.load(std::memory_order_relaxed));

  Executor::KeepAlive<> ka = mayNeedToAddThreads
    ? getKeepAliveToken(this)
    : folly::Executor::KeepAlive<>{};

  auto queue = static_cast<TicketQueue*>(getTaskQueue());
  auto const result = queue->add(std::move(task), ticket);
  if (mayNeedToAddThreads && !result.reusedThread) ensureActiveThreads();
}

TicketExecutor::Ticket TicketExecutor::stamp() {
  return static_cast<TicketQueue*>(getTaskQueue())->stamp();
}

folly::Executor::KeepAlive<> TicketExecutor::sticky(Ticket ticket) {
  return StickyTicketExecutor::create(this, ticket);
}

//////////////////////////////////////////////////////////////////////

}
