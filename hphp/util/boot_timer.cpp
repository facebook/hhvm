/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/boot_timer.h"

#include <cassert>

#include "hphp/util/logger.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TimeUsage TimeUsage::sinceEpoch() {
  return TimeUsage(
    std::chrono::duration_cast<Unit>(
      std::chrono::steady_clock::now().time_since_epoch()),
    std::chrono::duration_cast<Unit>(
      std::chrono::microseconds(
        HPHP::Timer::GetRusageMicros(Timer::TotalCPU, Timer::Self))));
}

std::string TimeUsage::toString() const {
  return folly::sformat(
    "{}ms wall, {}ms cpu",
    std::chrono::duration_cast<std::chrono::milliseconds>(wall()).count(),
    std::chrono::duration_cast<std::chrono::milliseconds>(cpu()).count());
}

void TimeUsage::operator=(const TimeUsage& rhs) {
  m_wall = rhs.m_wall;
  m_cpu = rhs.m_cpu;
}

TimeUsage TimeUsage::operator-(const TimeUsage& rhs) const {
  return TimeUsage(m_wall - rhs.m_wall, m_cpu - rhs.m_cpu);
}

TimeUsage TimeUsage::operator+(const TimeUsage& rhs) const {
  return TimeUsage(m_wall + rhs.m_wall, m_cpu + rhs.m_cpu);
}

///////////////////////////////////////////////////////////////////////////////

struct BootTimer::Impl {
  Impl() : m_last(TimeUsage::sinceEpoch()) {
  }

  void add(const std::string& info, TimeUsage value);
  void dumpMarks();
  TimeUsage computeDeltaFromLast();

  friend class BootTimer;

private:
  // Must hold when updating m_last
  std::mutex m_last_guard_;
  TimeUsage m_last;
  std::mutex m_marks_guard_;
  std::map<std::string, TimeUsage> m_marks;
};

void BootTimer::Impl::add(const std::string& info, TimeUsage value) {
  std::lock_guard<std::mutex> lock(m_marks_guard_);
  m_marks[info] = m_marks[info] + value;
}

void BootTimer::Impl::dumpMarks() {
  for (const auto& it : m_marks) {
    Logger::Info(
      folly::sformat("BootTimer: {} = {}", it.first, it.second.toString()));
  }
}

TimeUsage BootTimer::Impl::computeDeltaFromLast() {
  std::lock_guard<std::mutex> lock(m_last_guard_);
  auto now = TimeUsage::sinceEpoch();
  auto ret = now - m_last;
  m_last = now;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

BootTimer::Block::Block(const std::string& name)
    : m_name(name), m_start(TimeUsage::sinceEpoch()) {
  Logger::Info(folly::sformat("BootTimer: {}...", name));
}

BootTimer::Block::~Block() {
  auto total = TimeUsage::sinceEpoch() - m_start;
  Logger::Info(
    folly::sformat(
      "BootTimer: {} block done, took {}", m_name, total.toString()));
  BootTimer::add(m_name, total);
}

///////////////////////////////////////////////////////////////////////////////

bool BootTimer::s_started;
TimeUsage BootTimer::s_start;
std::unique_ptr<BootTimer::Impl> BootTimer::s_instance;
BootTimerCallback BootTimer::s_doneCallback;

void BootTimer::start() {
  BootTimer::s_started = true;
  BootTimer::s_start = TimeUsage::sinceEpoch();
  BootTimer::s_instance = folly::make_unique<BootTimer::Impl>();
}

void BootTimer::done() {
  if (!s_started) return;
  s_started = false;

  auto total = TimeUsage::sinceEpoch() - s_start;
  Logger::Info(
    folly::sformat("BootTimer: all done, took {} total", total.toString()));

  BootTimer::s_instance->add("TOTAL", total);
  BootTimer::s_instance->dumpMarks();

  if (s_doneCallback) {
    s_doneCallback(BootTimer::s_instance->m_marks);
  }
}

void BootTimer::mark(const std::string& info) {
  if (!s_started) return;
  auto elapsed = BootTimer::s_instance->computeDeltaFromLast();
  Logger::Info(
    folly::sformat("BootTimer: {} done, took {}", info, elapsed.toString()));
  BootTimer::s_instance->add(info, elapsed);
}

void BootTimer::add(const std::string& info, const TimeUsage value) {
  if (!s_started) return;
  BootTimer::s_instance->add(info, value);
}

void BootTimer::setDoneCallback(BootTimerCallback cb) {
  s_doneCallback = cb;
}

///////////////////////////////////////////////////////////////////////////////
}
