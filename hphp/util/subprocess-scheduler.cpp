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

#include "hphp/util/logger.h"
#include "hphp/util/optional.h"
#include "hphp/util/subprocess-scheduler.h"
#include "hphp/util/trace.h"

#include <folly/FileUtil.h>

#include <fstream>

namespace HPHP::extern_worker {

TRACE_SET_MOD(extern_worker)

SubprocessScheduler::SubprocessScheduler(uint64_t maxMemoryUsage)
  : m_maxMemoryUsage{maxMemoryUsage}
  , m_estimatedMemoryUsage{0} {
    if (maxMemoryUsage == 0) {
      // If not overridden, estimate the cap via 60% of the system's RAM.
      size_t pageCount = sysconf(_SC_PHYS_PAGES);
      size_t pageSize = sysconf(_SC_PAGESIZE);
      m_maxMemoryUsage = pageCount * pageSize * 3 / 5;
    }
    FTRACE(1,
            "SubprocessScheduler: limiting subprocess est. RSS to {} bytes.\n",
            m_maxMemoryUsage);
  }

uint64_t SubprocessScheduler::acquire(const std::string& jobName) {
  do {
    std::lock_guard<std::mutex> _{m_subprocessMapLock};
    uint64_t usageEstimate = getRSSEstimate(jobName);
    if (usageEstimate == 0 ||
        m_estimatedMemoryUsage + usageEstimate <= m_maxMemoryUsage) {
      FTRACE(3, "Scheduling `{}` job ({} running). est. job RSS: {}, est. total memory usage: {}\n",
              jobName, m_runningSubprocesses.size(), usageEstimate, m_estimatedMemoryUsage);
      m_estimatedMemoryUsage += usageEstimate;
      return usageEstimate;
    }
    std::unique_lock<std::mutex> lock(m_subprocessSchedulingLock);
    m_subprocessSchedulingCV.wait(lock);
  } while (true);
}

void SubprocessScheduler::release(uint64_t pid, const std::string& jobName, uint64_t usageEstimate) {
  std::lock_guard<std::mutex> _{m_subprocessMapLock};
  assertx(m_estimatedMemoryUsage >= usageEstimate);
  assertx(m_runningSubprocesses.find(pid) != m_runningSubprocesses.end());
  m_runningSubprocesses.erase(pid);
  m_estimatedMemoryUsage -= usageEstimate;
  m_subprocessSchedulingCV.notify_all();
}

 /*
  * Must be called after acquire - since we need the pid here, we can't provide the
  * parameter in the same method, but should be called as soon as we create the subprocess
  * for accurate memory tracking.
  */
void SubprocessScheduler::registerSubprocess(uint64_t pid, const std::string& jobName) {
  std::lock_guard<std::mutex> _{m_subprocessMapLock};
  m_runningSubprocesses.insert_or_assign(pid, jobName);
}

void SubprocessScheduler::sampleRSSData() {
  if (m_samplingFailed.load(std::memory_order_acquire)) return;
  std::vector<std::pair<uint64_t, std::string>> entriesToSample;
  {
     std::lock_guard<std::mutex> _{m_subprocessMapLock};
     for (auto const& [pid, jobName] : m_runningSubprocesses) {
       entriesToSample.emplace_back(pid, jobName);
     }
  }
  uint64_t pageSize = sysconf(_SC_PAGESIZE);
  for (auto const& [pid, jobName] : entriesToSample) {
    try {
      // Read RSS from /proc/{pid}/statm. From `man 5 proc`, the content
      // is a single line, of the following format:
      // `size resident shared text lib data dt`
      // We care about `resident`, which returns the number of allocated pages.
      std::string contents;
      auto res = folly::readFile(folly::sformat("/proc/{}/statm", pid).c_str(),
                                 contents);
      if (!res) continue;
      std::vector<folly::StringPiece> tokens;
      folly::split(' ', contents, tokens);

      always_assert(tokens.size() == 7);
      uint64_t rssPages = folly::to<int>(tokens[1]);
      uint64_t rssBytes = rssPages * pageSize;
      FTRACE(5, "Sampled RSS for job {} (pid {}): {} bytes\n",
              jobName, pid, rssBytes);
      addEstimate(jobName, rssBytes);
    } catch (const std::exception& e) {
      // If we failed to sample, log once and ensure we stop spending time on sampling
      // in the future.
      Logger::FError("Subprocess scheduler caught exception while reading live process data. \
      Sampling will be disabled, may result in slower execution. Exception: %s\n", e.what());
      m_samplingFailed.store(true, std::memory_order_release);
    }
  }
}

uint64_t SubprocessScheduler::getRSSEstimate(const std::string& jobName) const {
  auto estimateIt = m_jobToMaxRSSEstimate.find(jobName);
  // Start with an estimate of 20G memory usage and 1 sample
  uint64_t estimateMem = 20ul * 1024 * 1024 * 1024;
  uint64_t estimateCount = 1;
  if (estimateIt != m_jobToMaxRSSEstimate.end()) {
    auto [total, count] = estimateIt->second;
    assertx(count > 0);
    assertx(total >= 0);
    estimateMem += total;
    estimateCount += count;
  }
  return estimateMem / estimateCount;
}

void SubprocessScheduler::addEstimate(const std::string& jobName, uint64_t memoryUsage) {
  std::lock_guard<std::mutex> _{m_subprocessMapLock};
  auto entry = m_jobToMaxRSSEstimate.find(jobName);
  if (entry != m_jobToMaxRSSEstimate.end()) {
    uint64_t total = std::get<0>(entry->second);
    uint64_t count = std::get<1>(entry->second);
    total += memoryUsage;
    count++;
    m_jobToMaxRSSEstimate[jobName] = std::make_tuple(total, count);
  } else {
    m_jobToMaxRSSEstimate[jobName] = std::make_tuple(memoryUsage, 1);
  }
}
} // namespace HPHP::extern_worker
