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

#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <folly/Memory.h>

#include "hphp/util/compatibility.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ResourceUsage {
  ResourceUsage() : m_wall{0}, m_cpu{0}, m_rssMb(0) {}
  // Returns total usage for entire process.
  static ResourceUsage sinceEpoch();

  void operator=(const ResourceUsage& rhs);
  ResourceUsage operator-(const ResourceUsage& rhs) const;
  ResourceUsage operator+(const ResourceUsage& rhs) const;

  using TimeUnit = std::chrono::nanoseconds;
  TimeUnit wall() const { return m_wall; }
  TimeUnit cpu() const { return m_cpu; }
  int64_t rssMb() const { return m_rssMb; }
  std::string toString() const;

private:
  ResourceUsage(TimeUnit wall, TimeUnit cpu, int64_t rssMb)
      : m_wall{wall}, m_cpu{cpu}, m_rssMb(rssMb) {}
  TimeUnit m_wall;
  TimeUnit m_cpu;
  int64_t m_rssMb;
};

/**
 * Measures execution of the different parts of HHVM startup and warmup.
 *
 * BootStats should be initialized once, soon after HHVM starts up, by calling
 * the start() static method; it will continue collecting samples until the
 * done() static method is called, once the start up process is complete.
 *
 * There are two ways of adding a sample:
 * - BootStats::mark('foo') adds a sample for foo, using the elapsed time from
 *   start or from the last call to mark();
 * - using an instance of the BootStats::Block class during startup
 *   automatically collects a sample.
 */
struct BootStats {
  // Creates a new instance and starts the timer
  static void start();
  // Returns seconds since epoch when start() was called, or 0 if never called.
  static int64_t startTimestamp() {
    auto s = std::chrono::duration_cast<std::chrono::seconds>(s_start.wall());
    return s.count();
  }
  // Stops the timer and logs information
  static void done();
  // Computes the time elapsed from start or from the previous call to this
  // method and stores it as a sample with the given name
  static void mark(const std::string& name);

  struct Block {
    explicit Block(const std::string& name);
    ~Block();

  private:
    std::string m_name;
    ResourceUsage m_start;
  };

private:
  static void add(const std::string& name, const ResourceUsage value);

  struct Impl;

  static bool s_started;
  static ResourceUsage s_start;
  static std::unique_ptr<BootStats::Impl> s_instance;
};

///////////////////////////////////////////////////////////////////////////////
}
