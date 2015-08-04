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

typedef void (*BootTimerCallback)(const std::map<std::string, int64_t>);

/**
 * Times execution of the different parts of HHVM startup and warmup.
 *
 * BootTimer should be initialized once, soon after HHVM starts up, by calling
 * the start() static method; it will continue collecting samples until the
 * done() static method is called, once the start up process is complete.
 *
 * There are two ways of adding a sample:
 * - BootTimer::mark('foo') adds a sample for foo, using the elapsed time from
 *   start or from the last call to mark();
 * - using an instance of the Timer class during start up automatically
 *   collects a sample.
 */
struct BootTimer {
  // Creates a new instance and starts the timer
  static void start();
  // Stops the timer and logs information
  static void done();
  // Computes the time elapsed from start or from the previous call to this
  // method and stores it as a sample with the given name
  static void mark(const std::string& name);

  // Sets the callback to be invoked after BootTimer::done is called. Note
  // that only one callback is supported; subsequent calls will overwrite it.
  static void setDoneCallback(BootTimerCallback cb);

  struct Block {
    explicit Block(const std::string& name);
    ~Block();

  private:
    std::string m_name;
    std::chrono::steady_clock::time_point m_start;
  };

private:
  static void add(const std::string& name,
    const std::chrono::nanoseconds value);

  class Impl;

  static bool s_started;
  static std::chrono::steady_clock::time_point s_start;
  static std::unique_ptr<BootTimer::Impl> s_instance;
  static BootTimerCallback s_doneCallback;
};

///////////////////////////////////////////////////////////////////////////////
}
