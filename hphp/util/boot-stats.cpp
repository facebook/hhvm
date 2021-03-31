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
#include "hphp/util/boot-stats.h"

#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ResourceUsage ResourceUsage::sinceEpoch() {
  return ResourceUsage(
    std::chrono::duration_cast<TimeUnit>(
      std::chrono::steady_clock::now().time_since_epoch()),
    std::chrono::duration_cast<TimeUnit>(
      std::chrono::microseconds(
        HPHP::Timer::GetRusageMicros(Timer::TotalCPU, Timer::Self))),
    Process::GetMemUsageMb());
}

std::string ResourceUsage::toString() const {
  return folly::sformat(
    "{}ms wall, {}ms cpu, {} MB RSS",
    std::chrono::duration_cast<std::chrono::milliseconds>(wall()).count(),
    std::chrono::duration_cast<std::chrono::milliseconds>(cpu()).count(),
    rssMb());
}

void ResourceUsage::operator=(const ResourceUsage& rhs) {
  m_wall = rhs.m_wall;
  m_cpu = rhs.m_cpu;
  m_rssMb = rhs.m_rssMb;
}

ResourceUsage ResourceUsage::operator-(const ResourceUsage& rhs) const {
  return ResourceUsage(m_wall - rhs.m_wall,
                       m_cpu - rhs.m_cpu,
                       m_rssMb - rhs.m_rssMb);
}

ResourceUsage ResourceUsage::operator+(const ResourceUsage& rhs) const {
  return ResourceUsage(m_wall + rhs.m_wall,
                       m_cpu + rhs.m_cpu,
                       m_rssMb + rhs.m_rssMb);
}

///////////////////////////////////////////////////////////////////////////////

struct BootStats::Impl {
  Impl() : m_last(ResourceUsage::sinceEpoch()) {}

  void add(const std::string& info, ResourceUsage value);
  void dumpMarks();
  ResourceUsage computeDeltaFromLast();

  friend struct BootStats;

private:
  // Must hold when updating m_last
  std::mutex m_last_guard_;
  ResourceUsage m_last;
  std::mutex m_marks_guard_;
  std::map<std::string, ResourceUsage> m_marks;
  std::map<std::string, std::string> m_strs;
  std::map<std::string, int64_t> m_ints;
};

void BootStats::Impl::add(const std::string& info, ResourceUsage value) {
  std::lock_guard<std::mutex> lock(m_marks_guard_);
  m_marks[info] = m_marks[info] + value;
}

void BootStats::Impl::dumpMarks() {
  for (const auto& it : m_marks) {
    Logger::FInfo("BootStats: {} = {}", it.first, it.second.toString());
  }
}

ResourceUsage BootStats::Impl::computeDeltaFromLast() {
  std::lock_guard<std::mutex> lock(m_last_guard_);
  auto now = ResourceUsage::sinceEpoch();
  auto ret = now - m_last;
  m_last = now;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

BootStats::Block::Block(const std::string& name, bool enabled) {
  if (!enabled) return;
  m_name = name;
  m_enabled = true;
  m_start = ResourceUsage::sinceEpoch();
  Logger::FInfo("BootStats: {}...", name);
}

BootStats::Block::~Block() {
  if (!m_enabled) return;
  auto total = ResourceUsage::sinceEpoch() - m_start;
  Logger::FInfo("BootStats: {} block done, took {}", m_name, total.toString());
  BootStats::add(m_name, total);
}

///////////////////////////////////////////////////////////////////////////////

bool BootStats::s_started;
ResourceUsage BootStats::s_start;
std::unique_ptr<BootStats::Impl> BootStats::s_instance;

void BootStats::start() {
  BootStats::s_started = true;
  BootStats::s_start = ResourceUsage::sinceEpoch();
  BootStats::s_instance = std::make_unique<BootStats::Impl>();
}

void BootStats::done() {
  if (!s_started) return;
  s_started = false;

  auto total = ResourceUsage::sinceEpoch() - s_start;
  Logger::FInfo("BootStats: all done, took {} total", total.toString());

  BootStats::s_instance->add("TOTAL", total);
  BootStats::s_instance->dumpMarks();

  if (StructuredLog::enabled()) {
    std::lock_guard<std::mutex> lock(s_instance->m_marks_guard_);
    StructuredLogEntry cols;
    for (auto const& sample : s_instance->m_marks) {
      cols.setInt(sample.first, sample.second.wall().count());
      cols.setInt(sample.first + " CPU", sample.second.cpu().count());
    }
    for (auto const& strCol : s_instance->m_strs) {
      cols.setStr(strCol.first, strCol.second);
    }
    for (auto const& intCol : s_instance->m_strs) {
      cols.setStr(intCol.first, intCol.second);
    }
    StructuredLog::log("hhvm_boot_timer", cols);
    cols.clear();
    for (auto sample : s_instance->m_marks) {
      cols.setInt(sample.first, sample.second.rssMb() * (1 << 20)); // To bytes.
    }
    StructuredLog::log("hhvm_boot_memory", cols);
  }
}

void BootStats::mark(const std::string& info) {
  if (!s_started) return;
  auto elapsed = BootStats::s_instance->computeDeltaFromLast();
  Logger::FInfo("BootStats: {} done, took {}", info, elapsed.toString());
  BootStats::s_instance->add(info, elapsed);
}

void BootStats::add(const std::string& info, const ResourceUsage value) {
  if (!s_started) return;
  BootStats::s_instance->add(info, value);
}

void BootStats::set(const std::string& name, const std::string& value) {
  if (!s_started) return;
  BootStats::s_instance->m_strs[name] = value;
}

void BootStats::set(const std::string& name, int64_t value) {
  if (!s_started) return;
  BootStats::s_instance->m_ints[name] = value;
}

///////////////////////////////////////////////////////////////////////////////
}
