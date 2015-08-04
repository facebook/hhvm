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
#include "hphp/util/trace.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct BootTimer::Impl {
  Impl() : m_last(std::chrono::steady_clock::now()) {
  }

  void add(const std::string& info, int64_t value);
  void dumpMarks();
  int64_t computeDeltaFromLast();

  friend class BootTimer;

private:
  // Must hold when updating m_last
  std::mutex m_last_guard_;
  std::chrono::steady_clock::time_point m_last;
  std::mutex m_marks_guard_;
  std::map<std::string, int64_t> m_marks;
};

void BootTimer::Impl::add(const std::string& info, int64_t value) {
  std::lock_guard<std::mutex> lock(m_marks_guard_);
  int64_t curr = 0;
  if (m_marks.count(info) > 0) {
    curr = m_marks[info];
  }
  curr += value;
  m_marks[info] = curr;
}

void BootTimer::Impl::dumpMarks() {
  for (const auto& it : m_marks) {
    Logger::Info("BootTimer: %s = %" PRId64 "ns", it.first.c_str(), it.second);
  }
}

int64_t BootTimer::Impl::computeDeltaFromLast() {
  std::lock_guard<std::mutex> lock(m_last_guard_);
  auto now = std::chrono::steady_clock::now();
  auto ret = now - m_last;
  m_last = now;
  return std::chrono::duration_cast<std::chrono::nanoseconds>(ret).count();
}

///////////////////////////////////////////////////////////////////////////////

BootTimer::Block::Block(const std::string& name)
  : m_name(name), m_start(std::chrono::steady_clock::now()) {
  Logger::Info(folly::sformat("BootTimer: {}...", name));
}

BootTimer::Block::~Block() {
  auto total = std::chrono::steady_clock::now() - m_start;
  Logger::Info(
    folly::sformat("BootTimer: {} block done, took {}ns",
      m_name,
      std::chrono::duration_cast<std::chrono::nanoseconds>(total).count()));
  BootTimer::add(m_name, total);
}

///////////////////////////////////////////////////////////////////////////////

bool BootTimer::s_started;
std::chrono::steady_clock::time_point BootTimer::s_start;
std::unique_ptr<BootTimer::Impl> BootTimer::s_instance;
BootTimerCallback BootTimer::s_doneCallback;

void BootTimer::start() {
  BootTimer::s_started = true;
  BootTimer::s_start = std::chrono::steady_clock::now();
  BootTimer::s_instance = folly::make_unique<BootTimer::Impl>();
}

void BootTimer::done() {
  if (!s_started) return;
  s_started = false;

  auto total = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::steady_clock::now() - s_start).count();
  Logger::Info(
    folly::sformat("BootTimer: all done, took {}ns total",
      total));

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
    folly::sformat("BootTimer: {} done, took {}ns", info, elapsed));
  BootTimer::s_instance->add(info, elapsed);
}

void BootTimer::add(const std::string& info,
    const std::chrono::nanoseconds value) {
  if (!s_started) return;
  BootTimer::s_instance->add(info, value.count());
}

void BootTimer::setDoneCallback(BootTimerCallback cb) {
  s_doneCallback = cb;
}

///////////////////////////////////////////////////////////////////////////////
}
