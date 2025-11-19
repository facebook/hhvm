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

#include "hphp/util/optional.h"
#include "hphp/util/hash-map.h"

#include <condition_variable>
#include <cstdint>

namespace HPHP::extern_worker {

// Manages how many subprocesses SubprocessImpl executes concurrently on
// the machine. Samples live RSS data and uses it against the provided memory
// cap of `m_maxMemoryUsage` to estimate how many processes to run.
//
// There are two modes of interaction with the SubprocessScheduler:
// 1. Acquiring/releasing jobs by name.
// 2. Instructing the SubprocessScheduler to collect live samples of RSS usage.
//
// To acquire a job, a caller needs to `tryAcquire` until a non-nullopt usage
// estimate is returned, and needs to `registerSubprocess` the pid for accuracy of
// RSS sampling. The scheduler's usage estimate will not change automatically on
// subprocess termination, `release` must be called to allow more work through.
//
// For sampling, SubprocessScheduler maintains a map of pid -> job name. Whenever
// `sampleRSSData` is called, each pid will be inspected for memory usage and memory
// estimates of the job kind will be updated. This will not affect the existing usage
// estimate for already running jobs, but will update limits for later ones.
//
// To avoid thundering herd problems when a job name without known characteristics is
// scheduled, initial estimates for a job's size start at a very high estimate
// of 20GB.
struct SubprocessScheduler {
  explicit SubprocessScheduler(uint64_t maxMemoryUsage);
  /*
    * Blocks until scheduling the job with `jobName` is
    * estimated to keep the managed subprocess usage under `maxMemoryUsage`.
    *
    * Increases the estimated total memory usage by the returned usage estimate.
    */
  uint64_t acquire(const std::string& jobName);
  /*
    * Release `usageEstimate` amount of memory and mark pid as done. Allows other `acquire`'s
    * to make progress.
    */
  void release(uint64_t pid, const std::string& jobName, uint64_t usageEstimate);
  /*
    * Must be called after tryAcquire - this method required a PID, which we don't yet have at
    * tryAcquire time. For accurate memory tracking, this should be called as soon as we
    * create the subprocess.
    */
  void registerSubprocess(uint64_t pid, const std::string& jobName);
  /*
    * Iterates through all live processes and samples RSS, building more accurate estimates over
    * time.
    */
  void sampleRSSData();

  private:
  uint64_t getRSSEstimate(const std::string& jobName) const;
  void addEstimate(const std::string& jobName, uint64_t memoryUsage);

  uint64_t m_maxMemoryUsage;
  uint64_t m_estimatedMemoryUsage;
  mutable std::mutex m_subprocessSchedulingLock;
  mutable std::condition_variable m_subprocessSchedulingCV;
  // pid to job_name
  mutable std::mutex m_subprocessMapLock;
  hphp_fast_map<uint64_t, std::string> m_runningSubprocesses;
  // job name to (total memory usage, count) pairs.
  hphp_fast_map<std::string, std::tuple<uint64_t, uint64_t>> m_jobToMaxRSSEstimate;
  std::atomic_bool m_samplingFailed{false};
};
} // namespace HPHP
