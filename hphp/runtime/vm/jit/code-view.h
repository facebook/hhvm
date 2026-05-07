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

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/util/configs/codecache.h"
#include <folly/concurrency/ConcurrentHashMap.h>

#include <atomic>
#include <condition_variable>
#include <limits>
#include <mutex>

namespace HPHP::jit {

struct SlotState {
  uint64_t activeCount : 8;
  uint64_t deadlineMs : 56;
  bool operator==(const SlotState&) const = default;

  static uint64_t toMs(std::chrono::steady_clock::time_point tp) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      tp.time_since_epoch()).count();
  }
};
static_assert(sizeof(SlotState) == sizeof(uint64_t));

struct CodeCacheViews {
  static constexpr pthread_t kAnchorTid = std::numeric_limits<pthread_t>::max();

  explicit CodeCacheViews(int maxViews);
  CodeCache::View view(TransKind kind, const tc::TransRange* sizes, pthread_t tid,
                       bool threadAffinity, bool codeLocked = false);
  void acquire(pthread_t tid);
  bool tryAcquire(pthread_t tid);
  void release(pthread_t tid, std::chrono::milliseconds dropHint);
  void setFull(pthread_t tid);
  bool isFull(pthread_t tid);
  int numThreadsFull();
  int numViews() const;
  int maxViews() const;

private:
  bool doTryAcquire(pthread_t tid);
  bool reserveSlot();
  folly::ConcurrentHashMap<pthread_t, CodeCache::View> m_views;
  folly::ConcurrentHashMap<pthread_t, SlotState> m_slots;
  folly::ConcurrentHashMap<pthread_t, bool> m_threadsFull{};
  std::atomic<std::chrono::steady_clock::time_point> m_lastStealTime{};
  std::atomic<int> m_numSlots{0};
  std::atomic<int> m_maxViews;
  std::mutex m_cvMutex;
  std::condition_variable m_cv;
};

struct ViewHolder {
  ViewHolder(pthread_t, bool blocking,
             std::chrono::milliseconds dropHint =
                 std::chrono::milliseconds(Cfg::CodeCache::DropHintTimeMs));
  ~ViewHolder();
  bool isValid() const;
  void setDropHint(std::chrono::milliseconds dropHint) {
    m_dropHint = dropHint;
  }

private:
  std::optional<pthread_t> m_tid;
  std::chrono::milliseconds m_dropHint;
};

}
