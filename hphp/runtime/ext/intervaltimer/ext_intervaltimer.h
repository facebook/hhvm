/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#ifndef incl_HPHP_EXT_INTERVALTIMER_H_
#define incl_HPHP_EXT_INTERVALTIMER_H_

#include <condition_variable>
#include <mutex>
#include <thread>

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/resource-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct IntervalTimer final {
  enum SampleType {
    IOWaitSample,
    ResumeAwaitSample,
    EnterSample,
    ExitSample,
  };

  static Class* c_Class;
  static const StaticString c_ClassName;

  static void RunCallbacks(SampleType);

  IntervalTimer() {}
  ~IntervalTimer();

  IntervalTimer(const IntervalTimer&) = delete;
  IntervalTimer& operator=(IntervalTimer&) = delete;

  void init(double interval,
            double initial,
            const Variant& callback,
            RequestInjectionData* data);
  void start();
  void stop();
  void run();

private:
  template <typename F> friend void scan(const IntervalTimer&, F&);
  double m_interval;
  double m_initial;
  Variant m_callback;
  RequestInjectionData* m_data{nullptr};
  std::thread m_thread;
  std::condition_variable m_cv;
  std::mutex m_mutex;
  bool m_done{false};
  std::mutex m_signalMutex;
  int m_count{0};   // # of times hit since last surprise check
};

void HHVM_METHOD(IntervalTimer, __construct,
                 double interval,
                 double initial,
                 const Variant& callback);
void HHVM_METHOD(IntervalTimer, start);
void HHVM_METHOD(IntervalTimer, stop);

///////////////////////////////////////////////////////////////////////////////
}
#endif
